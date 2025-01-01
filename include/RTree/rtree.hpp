#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "geometry_traits.hpp"
#include "global.hpp"
#include "iterator.hpp"
#include "static_node.hpp"

#include "rstar_split.hpp"

namespace eh
{
namespace rtree
{

struct DefaultConfig
{
  /// minimum number of entries in a node
  constexpr static size_type MIN_ENTRIES = 4;
  /// maximum number of entries in a node
  constexpr static size_type MAX_ENTRIES = 8;

  /// number of entries to be reinserted when node overflow occurs
  constexpr static size_type REINSERT_COUNT = 3;

  // Node Overflow Splitting Scheme
  using split_algorithm = RStarSplit;
  // using split_algorithm = QuadraticSplit;
};

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          typename Config = DefaultConfig, // other configurations
          template <typename _T> class Allocator = std::allocator // allocator
          >
class RTree
{
public:
  using size_type = ::eh::rtree::size_type;

  using geometry_type = GeometryType;
  using key_type = KeyType;
  using mapped_type = MappedType;
  using value_type = std::pair<key_type, mapped_type>;
  using traits = geometry_traits<geometry_type>;

  using scalar_type = typename geometry_traits<geometry_type>::scalar_type;

  // M
  constexpr static size_type MAX_ENTRIES = Config::MAX_ENTRIES;
  // m <= M/2
  constexpr static size_type MIN_ENTRIES = Config::MIN_ENTRIES;
  static_assert(MIN_ENTRIES <= MAX_ENTRIES / 2, "Invalid MIN_ENTRIES count");
  static_assert(MIN_ENTRIES >= 1, "Invalid MIN_ENTRIES count");

  static_assert(MAX_ENTRIES + 1 - Config::REINSERT_COUNT >= MIN_ENTRIES,
                "Invalid REINSERT_COUNT count");
  static_assert(MAX_ENTRIES + 1 - Config::REINSERT_COUNT <= MAX_ENTRIES,
                "Invalid REINSERT_COUNT count");

  // using stack memory for MaxEntries child nodes. instead of std::vector
  using node_base_type = static_node_base_t<GeometryType,
                                            KeyType,
                                            MappedType,
                                            MIN_ENTRIES,
                                            MAX_ENTRIES>;

  using node_type = static_node_t<GeometryType,
                                  KeyType,
                                  MappedType,
                                  MIN_ENTRIES,
                                  MAX_ENTRIES>;
  using leaf_type = static_leaf_node_t<GeometryType,
                                       KeyType,
                                       MappedType,
                                       MIN_ENTRIES,
                                       MAX_ENTRIES>;

  using node_allocator_type = Allocator<node_type>;
  using leaf_allocator_type = Allocator<leaf_type>;

  using iterator = iterator_t<leaf_type>;
  using const_iterator = iterator_t<leaf_type const>;

  using node_iterator = node_iterator_t<node_type>;
  using const_node_iterator = node_iterator_t<node_type const>;

  using leaf_iterator = node_iterator_t<leaf_type>;
  using const_leaf_iterator = node_iterator_t<leaf_type const>;

protected:
  node_base_type* _root = nullptr;
  int _leaf_level = 0;

  node_allocator_type _node_allocator;
  leaf_allocator_type _leaf_allocator;

  void delete_if()
  {
    if (_root)
    {
      if (_leaf_level == 0)
      {
        // root is leaf node
        _root->as_leaf()->delete_recursive(*this);
        destroy_node(_root->as_leaf());
      }
      else
      {
        _root->as_node()->delete_recursive(_leaf_level, *this);
        destroy_node(_root->as_node());
      }
    }
  }
  void set_null()
  {
    _root = nullptr;
    _leaf_level = 0;
  }

  void init_root()
  {
    if (_root == nullptr)
    {
      _root = construct_node<leaf_type>();
      _leaf_level = 0;
    }
  }

  // search for appropriate node in target_level to insert bound
  node_type* choose_insert_target(geometry_type const& bound, int target_level)
  {
    EH_RTREE_ASSERT_SILENT(target_level <= _leaf_level);
    /*
    CLl. [Initialize.]
    Set N to be the root node.

    CL2. [Leaf check.]
    If N is a leaf, return N.

    CL3. [Choose subtree.]
    If N is not a leaf, let F be the entry in N whose rectangle F.I needs least
    enlargement to include E.I. Resolve ties by choosing the entry with the
    rectangle of smallest area.

    CL4. [Descend until a leaf is reached.]
    Set N to be the child node pointed to by F.p and repeat from CL2.
    */

    node_type* n = _root->as_node();
    for (int level = 0; level < target_level; ++level)
    {
      scalar_type min_area_enlarge = std::numeric_limits<scalar_type>::max();
      typename node_type::iterator chosen = n->end();

      for (typename node_type::iterator ci = n->begin(); ci != n->end(); ++ci)
      {
        const auto area_enlarge
            = helper::enlarged_area(ci->first, bound) - helper::area(ci->first);
        if (area_enlarge < min_area_enlarge)
        {
          min_area_enlarge = area_enlarge;
          chosen = ci;
        }
        else if (area_enlarge == min_area_enlarge)
        {
          if (helper::area(ci->first) < helper::area(chosen->first))
          {
            chosen = ci;
          }
        }
      }
      EH_RTREE_ASSERT_SILENT(chosen != n->end());
      n = chosen->second->as_node();
    }
    return n;
  }

  // insert child to given parent
  template <typename NodeType>
  void insert_node(NodeType* parent,
                   typename NodeType::value_type new_child,
                   bool reinsert)
  {
    NodeType* pair = nullptr;
    if (parent->size() == MAX_ENTRIES)
    {
      // overflow treatment
      if (reinsert && parent->as_node() != root()
          && parent->parent()->size() > 1
          && MAX_ENTRIES + 1 - Config::REINSERT_COUNT >= MIN_ENTRIES
          && MAX_ENTRIES + 1 - Config::REINSERT_COUNT <= MAX_ENTRIES)
      {
        this->reinsert(parent, std::move(new_child));
      }
      else
      {
        pair = split(parent, std::move(new_child));
      }
    }
    else
    {
      parent->insert(std::move(new_child));
    }
    rebound(parent);

    // split occured
    // insert new node pair to parent
    if (pair)
    {
      if (parent == _root)
      {
        node_type* new_root = construct_node<node_type>();
        new_root->insert({ parent->calculate_bound(), parent });
        new_root->insert({ pair->calculate_bound(), pair });
        _root = new_root;
        ++_leaf_level;
      }
      else
      {
        insert_node(parent->parent(), { pair->calculate_bound(), pair }, true);
      }
    }
  }

  /*
    Overflow treatment:
      either split or reinsert
  */

  /*
    splitting scheme:
      quadratic split
      r*-tree split
  */

  // 'node' contains MAX_ENTRIES nodes;
  // trying to add additional child 'child'
  // split into two nodes
  // so that two nodes' child count is in range [ MIN_ENTRIES, MAX_ENTRIES ]
  template <typename NodeType>
  NodeType* split(NodeType* node, typename NodeType::value_type child)
  {
    NodeType* pair = construct_node<NodeType>();
    // @TODO another split scheme
    Config::split_algorithm::split(node, std::move(child), pair);
    return pair;
  }

  /*
  ReInsertion Scheme
  When node overflow occurs
  (i.e, trying to add more than MAX_ENTRIES into single node),
  the R*-Tree uses a reinsertion strategy to redistribute the
  entries in the overflowing node.

  `_reinsert_nodes` child nodes of the overflowing node are removed and
  reinserted
  */

  // 'node' contains MAX_ENTRIES nodes;
  // trying to add additional child 'child'
  // pick `reinsert_nodes` children from node and reinsert them
  void reinsert(node_type* node, typename node_type::value_type child)
  {
    /*
    Algorithm Reinsert
      1. For all M+l entries of a node N, compute the distance
          between the centers of their rectangles and the center
          of the bounding rectangle of N
      2. Sort the entries m decreasing order of their distances
          computed in (1)
      3. Remove the first p entries from N and adjust the
          bounding rectangle of N
      4. In the sort, defined in (2), starting with the maximum distance
         (= far reinsert) or minimum distance (= close reinsert),
         invoke Insert to reinsert the entries
    */

    const int node_realtive_level_from_leaf
        = leaf_level() - node->level_recursive();

    geometry_type node_bound = node->calculate_bound();
    helper::enlarge_to(node_bound, child.first);
    std::vector<typename node_type::value_type> children;
    children.reserve(MAX_ENTRIES + 1);
    for (typename node_type::value_type& c : *node)
    {
      children.emplace_back(std::move(c));
    }
    children.emplace_back(std::move(child));
    EH_RTREE_ASSERT_SILENT(children.size() == MAX_ENTRIES + 1);
    node->clear();

    std::sort(children.begin(), children.end(),
              [&](typename node_type::value_type const& a,
                  typename node_type::value_type const& b)
              {
                return helper::distance_center(node_bound, a.first)
                       < helper::distance_center(node_bound, b.first);
              });
    for (size_type i = 0; i < MAX_ENTRIES + 1 - Config::REINSERT_COUNT; ++i)
    {
      node->insert(std::move(children[i]));
    }
    rebound(node);
    for (size_type i = MAX_ENTRIES + 1 - Config::REINSERT_COUNT;
         i <= MAX_ENTRIES; ++i)
    {
      typename node_type::value_type& c = children[i];
      node_type* chosen = choose_insert_target(
          c.first, leaf_level() - node_realtive_level_from_leaf);
      insert_node(chosen, std::move(c), false);
    }
  }
  void reinsert(leaf_type* node, typename leaf_type::value_type child)
  {
    geometry_type node_bound = node->calculate_bound();
    helper::enlarge_to(node_bound, child.first);
    std::vector<typename leaf_type::value_type> children;
    children.reserve(MAX_ENTRIES + 1);
    for (typename leaf_type::value_type& c : *node)
    {
      children.emplace_back(std::move(c));
    }
    children.emplace_back(std::move(child));
    node->clear();

    std::sort(children.begin(), children.end(),
              [&](typename leaf_type::value_type const& a,
                  typename leaf_type::value_type const& b)
              {
                return helper::distance_center(node_bound, a.first)
                       < helper::distance_center(node_bound, b.first);
              });
    for (size_type i = 0; i < MAX_ENTRIES + 1 - Config::REINSERT_COUNT; ++i)
    {
      node->insert(std::move(children[i]));
    }
    rebound(node);
    for (size_type i = MAX_ENTRIES + 1 - Config::REINSERT_COUNT;
         i <= MAX_ENTRIES; ++i)
    {
      typename leaf_type::value_type& c = children[i];
      leaf_type* chosen
          = choose_insert_target(c.first, leaf_level())->as_leaf();
      insert_node(chosen, std::move(c), false);
    }
  }

public:
  void insert(value_type new_val)
  {
    leaf_type* chosen
        = choose_insert_target(new_val.first, _leaf_level)->as_leaf();
    insert_node(chosen, std::move(new_val), true);
  }
  template <typename... Args>
  void emplace(Args&&... args)
  {
    insert(value_type(std::forward<Args>(args)...));
  }

  void erase(iterator pos)
  {
    leaf_type* leaf = pos._leaf;
    leaf->erase(pos._pointer);

    if (leaf == _root)
    {
      return;
    }

    // it's children will be reinserted later
    struct erase_reinsert_node_info_t
    {
      int relative_level_from_leaf;
      node_base_type* parent;
    };
    std::vector<erase_reinsert_node_info_t> reinsert_nodes;

    node_type* node = leaf->parent();
    if (leaf->size() < MIN_ENTRIES)
    {
      // delete node from node's parent
      node->erase(leaf);

      // insert node to set
      reinsert_nodes.push_back({ 0, leaf });
    }
    else
    {
      leaf->entry().first = leaf->calculate_bound();
    }
    for (int level = _leaf_level - 1; level > 0; --level)
    {
      node_type* parent = node->parent();
      if (node->size() < MIN_ENTRIES)
      {
        // delete node from node's parent
        parent->erase(node);
        // insert node to set
        reinsert_nodes.push_back({ _leaf_level - level, node });
      }
      else
      {
        node->entry().first = node->calculate_bound();
      }
      node = parent;
    }

    // root adjustment
    if (_leaf_level > 0)
    {
      if (_root->as_node()->size() == 1)
      {
        node_base_type* child = _root->as_node()->at(0).second;
        _root->as_node()->erase(child);
        destroy_node(_root->as_node());
        _root = child;
        --_leaf_level;
      }
    }

    // reinsert entries
    // sustain the relative level from leaf
    for (erase_reinsert_node_info_t reinsert : reinsert_nodes)
    {
      // leaf node
      if (reinsert.relative_level_from_leaf == 0)
      {
        for (typename leaf_type::value_type& c : *(reinsert.parent->as_leaf()))
        {
          insert(std::move(c));
        }
        destroy_node(reinsert.parent->as_leaf());
      }
      else
      {
        for (typename node_type::value_type& c : *(reinsert.parent->as_node()))
        {
          node_type* chosen = choose_insert_target(
              c.first, _leaf_level - reinsert.relative_level_from_leaf);
          insert_node(chosen, c, true);
        }
        destroy_node(reinsert.parent->as_node());
      }
    }
  }

  size_type size() const
  {
    if (_leaf_level == 0)
    {
      return _root->as_leaf()->size_recursive();
    }
    else
    {
      return _root->as_node()->size_recursive(_leaf_level);
    }
  }

  void clear()
  {
    delete_if();
    set_null();
    init_root();
  }

  // adjust bound from node `N` to root recursively
  void rebound(node_type* N)
  {
    while (N->parent())
    {
      N->entry().first = N->calculate_bound();
      N = N->parent();
    }
  }
  // adjust bound from node `leaf` to root recursively
  void rebound(leaf_type* leaf)
  {
    if (leaf->parent())
    {
      leaf->entry().first = leaf->calculate_bound();
      rebound(leaf->parent());
    }
  }
  // adjust bound from the iterator to root recursively
  void rebound(iterator pos)
  {
    rebound(pos._leaf);
  }

  RTree()
  {
    init_root();
  }

  // @TODO
  // mapped_type copy-assignable
  RTree(RTree const& rhs)
  {
    if (rhs._leaf_level == 0)
    {
      _root = rhs._root->as_leaf()->clone_recursive(*this);
      _leaf_level = 0;
    }
    else
    {
      _root = rhs._root->as_node()->clone_recursive(rhs._leaf_level, *this);
      _leaf_level = rhs._leaf_level;
    }
  }
  template <typename Iterator>
  RTree(Iterator begin, Iterator end)
  {
    init_root();
    for (; begin != end; ++begin)
    {
      insert(*begin);
    }
  }

  template <typename GeometryType_,
            typename KeyType_,
            typename MappedType_,
            typename Config_,
            template <typename _T>
            class Allocator_>
  RTree(RTree<GeometryType_, KeyType_, MappedType_, Config_, Allocator_> const&
            rhs)
  {
    for (value_type const& v : rhs)
    {
      insert(v);
    }
  }
  template <typename GeometryType_,
            typename KeyType_,
            typename MappedType_,
            typename Config_,
            template <typename _T>
            class Allocator_>
  RTree(RTree<GeometryType_, KeyType_, MappedType_, Config_, Allocator_>&& rhs)
  {
    for (value_type& v : rhs)
    {
      insert(std::move(v));
    }
  }
  // @TODO
  // mapped_type copy-assignable
  RTree& operator=(RTree const& rhs)
  {
    delete_if();
    if (rhs._leaf_level == 0)
    {
      _root = rhs._root->as_leaf()->clone_recursive(*this);
      _leaf_level = 0;
    }
    else
    {
      _root = rhs._root->as_node()->clone_recursive(rhs._leaf_level, *this);
      _leaf_level = rhs._leaf_level;
    }
    return *this;
  }
  RTree(RTree&& rhs)
  {
    _root = rhs._root;
    _leaf_level = rhs._leaf_level;
    rhs.set_null();
    rhs.init_root();
  }
  RTree& operator=(RTree&& rhs)
  {
    delete_if();
    _root = rhs._root;
    _leaf_level = rhs._leaf_level;
    rhs.set_null();
    rhs.init_root();
    return *this;
  }
  template <typename GeometryType_,
            typename KeyType_,
            typename MappedType_,
            typename Config_,
            template <typename _T>
            class Allocator_>
  RTree& operator=(
      RTree<GeometryType_, KeyType_, MappedType_, Config_, Allocator_> const&
          rhs)
  {
    clear();
    for (value_type const& v : rhs)
    {
      insert(v);
    }
    return *this;
  }
  template <typename GeometryType_,
            typename KeyType_,
            typename MappedType_,
            typename Config_,
            template <typename _T>
            class Allocator_>
  RTree& operator=(
      RTree<GeometryType_, KeyType_, MappedType_, Config_, Allocator_>&& rhs)
  {
    clear();
    for (value_type& v : rhs)
    {
      insert(std::move(v));
    }
    return *this;
  }
  ~RTree()
  {
    delete_if();
  }

  /// fetch iterator for the elements in the tree
  iterator begin()
  {
    node_type* n = _root->as_node();
    for (int level = 0; level < _leaf_level; ++level)
    {
      n = n->at(0).second->as_node();
    }
    if (n->as_leaf()->empty())
    {
      return {};
    }
    return { &n->as_leaf()->at(0), n->as_leaf() };
  }
  /// fetch iterator for the elements in the tree
  const_iterator cbegin() const
  {
    node_type const* n = _root->as_node();
    for (int level = 0; level < _leaf_level; ++level)
    {
      n = n->at(0).second->as_node();
    }
    if (n->as_leaf()->empty())
    {
      return {};
    }
    return { &n->as_leaf()->at(0), n->as_leaf() };
  }
  /// fetch iterator for the elements in the tree
  const_iterator begin() const
  {
    return cbegin();
  }

  /// fetch iterator for the elements in the tree
  iterator end()
  {
    return {};
  }
  /// fetch iterator for the elements in the tree
  const_iterator cend() const
  {
    return {};
  }
  /// fetch iterator for the elements in the tree
  const_iterator end() const
  {
    return {};
  }

  /// fetch iterator for the non-leaf nodes on the given level
  node_iterator node_begin(int level)
  {
    EH_RTREE_ASSERT(level >= 0, "level must be greater than or equal to 0");
    EH_RTREE_ASSERT(level < _leaf_level,
                    "level must be pointing non-leaf node");
    if (level > _leaf_level)
    {
      return {};
    }
    node_type* n = _root->as_node();
    for (int l = 0; l < level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n };
  }
  /// fetch iterator for the non-leaf nodes on the given level
  const_node_iterator node_begin(int level) const
  {
    EH_RTREE_ASSERT(level >= 0, "level must be greater than or equal to 0");
    EH_RTREE_ASSERT(level < _leaf_level,
                    "level must be pointing non-leaf node");
    if (level > _leaf_level)
    {
      return {};
    }
    node_type const* n = _root->as_node();
    for (int l = 0; l < level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n };
  }
  /// fetch iterator for the non-leaf nodes on the given level
  const_node_iterator node_cbegin(int level) const
  {
    EH_RTREE_ASSERT(level >= 0, "level must be greater than or equal to 0");
    EH_RTREE_ASSERT(level < _leaf_level,
                    "level must be pointing non-leaf node");
    return begin(level);
  }

  /// fetch iterator for the non-leaf nodes on the given level
  node_iterator node_end(int level)
  {
    EH_RTREE_ASSERT(level >= 0, "level must be greater than or equal to 0");
    EH_RTREE_ASSERT(level < _leaf_level,
                    "level must be pointing non-leaf node");
    return {};
  }
  /// fetch iterator for the non-leaf nodes on the given level
  const_node_iterator node_end(int level) const
  {
    EH_RTREE_ASSERT(level >= 0, "level must be greater than or equal to 0");
    EH_RTREE_ASSERT(level < _leaf_level,
                    "level must be pointing non-leaf node");
    return {};
  }
  /// fetch iterator for the non-leaf nodes on the given level
  const_node_iterator node_cend(int level) const
  {
    EH_RTREE_ASSERT(level >= 0, "level must be greater than or equal to 0");
    EH_RTREE_ASSERT(level < _leaf_level,
                    "level must be pointing non-leaf node");
    return {};
  }

  /// fetch iterator for the leaf nodes
  leaf_iterator leaf_begin()
  {
    node_type* n = _root->as_node();
    for (int l = 0; l < _leaf_level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
  }
  /// fetch iterator for the leaf nodes
  const_leaf_iterator leaf_begin() const
  {
    node_type const* n = _root->as_node();
    for (int l = 0; l < _leaf_level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
  }
  /// fetch iterator for the leaf nodes
  const_leaf_iterator leaf_cbegin() const
  {
    return leaf_begin();
  }
  /// fetch iterator for the leaf nodes
  leaf_iterator leaf_end()
  {
    return {};
  }
  /// fetch iterator for the leaf nodes
  const_leaf_iterator leaf_end() const
  {
    return {};
  }
  /// fetch iterator for the leaf nodes
  const_leaf_iterator leaf_cend() const
  {
    return {};
  }

  /// get the root node
  node_type* root()
  {
    return _root->as_node();
  }
  /// get the root node
  node_type const* root() const
  {
    return _root->as_node();
  }

  /// get the level where leaf nodes are
  int leaf_level() const
  {
    return _leaf_level;
  }

  node_allocator_type& node_allocator()
  {
    return _node_allocator;
  }
  leaf_allocator_type& leaf_allocator()
  {
    return _leaf_allocator;
  }

  template <typename NodeType>
  typename std::enable_if<std::is_same<NodeType, node_type>::value,
                          NodeType*>::type
  construct_node()
  {
    return new (node_allocator_type().allocate(1)) NodeType;
  }
  template <typename NodeType>
  typename std::enable_if<std::is_same<NodeType, leaf_type>::value,
                          NodeType*>::type
  construct_node()
  {
    return new (leaf_allocator_type().allocate(1)) NodeType;
  }
  void destroy_node(node_type* node)
  {
    node->~node_type();
    node_allocator_type().deallocate(node, 1);
  }
  void destroy_node(leaf_type* node)
  {
    node->~leaf_type();
    leaf_allocator_type().deallocate(node, 1);
  }

public:
  struct flatten_node_t
  {
    /// offset in global dense buffer
    size_type offset;

    /// the number of children
    size_type size;

    /// parent node index
    size_type parent;
  };
  struct flatten_result_t
  {
    /// leaf node's level
    size_type leaf_level;

    /// root node index; must be 0
    size_type root;

    /// node data ( include leaf nodes )
    std::vector<flatten_node_t> nodes;

    /// global dense buffer of children_boundingbox
    std::vector<geometry_type> children_bound;
    /// global dense buffer of children index
    std::vector<size_type> children;

    /// real data
    std::vector<mapped_type> data;
  };

protected:
  size_type
  flatten_recursive_leaf(flatten_result_t& res,
                         leaf_type const* node,
                         size_type parent_index,
                         std::integral_constant<bool, true> move_data) const
  {
    const size_type this_index = res.nodes.size();
    res.nodes.emplace_back();
    res.nodes[this_index].parent = parent_index;
    res.nodes[this_index].offset = res.children.size();
    res.nodes[this_index].size = node->size();

    for (size_type child_index = 0; child_index < node->size(); ++child_index)
    {
      res.children.push_back(0);
      res.children_bound.push_back(node->at(child_index).first);
    }

    for (size_type child_index = 0; child_index < node->size(); ++child_index)
    {
      const size_type data_index = res.data.size();
      res.data.emplace_back(std::move(node->at(child_index).second));
      res.children[res.nodes[this_index].offset + child_index] = data_index;
    }
    return this_index;
  }

  size_type
  flatten_recursive_leaf(flatten_result_t& res,
                         leaf_type const* node,
                         size_type parent_index,
                         std::integral_constant<bool, false> move_data) const
  {
    const size_type this_index = res.nodes.size();
    res.nodes.emplace_back();
    res.nodes[this_index].parent = parent_index;
    res.nodes[this_index].offset = res.children.size();
    res.nodes[this_index].size = node->size();

    for (size_type child_index = 0; child_index < node->size(); ++child_index)
    {
      res.children.push_back(0);
      res.children_bound.push_back(node->at(child_index).first);
    }

    for (size_type child_index = 0; child_index < node->size(); ++child_index)
    {
      const size_type data_index = res.data.size();
      res.data.emplace_back(node->at(child_index).second);
      res.children[res.nodes[this_index].offset + child_index] = data_index;
    }
    return this_index;
  }

  template <bool Move = false>
  size_type flatten_recursive(flatten_result_t& res,
                              node_type const* node,
                              size_type parent_index,
                              int level) const
  {
    if (level == leaf_level())
    {
      return flatten_recursive_leaf(res, node->as_leaf(), parent_index,
                                    std::integral_constant<bool, Move> {});
    }
    else
    {
      // add this node to buffer
      const size_type this_index = res.nodes.size();
      res.nodes.emplace_back();
      res.nodes[this_index].parent = parent_index;
      res.nodes[this_index].offset = res.children.size();
      res.nodes[this_index].size = node->size();

      for (size_type child_index = 0; child_index < node->size(); ++child_index)
      {
        res.children.push_back(0);
        res.children_bound.push_back(node->at(child_index).first);
      }

      for (size_type child_index = 0; child_index < node->size(); ++child_index)
      {
        res.children[res.nodes[this_index].offset + child_index]
            = flatten_recursive<Move>(res,
                                      node->at(child_index).second->as_node(),
                                      this_index, level + 1);
      }
      return this_index;
    }
  }

public:
  /// flatten the tree into a single dense buffer
  template <bool Move = false>
  flatten_result_t flatten() const
  {
    flatten_result_t res;
    res.leaf_level = leaf_level();
    res.root = 0;
    flatten_recursive<Move>(res, root(), 0, 0);
    return res;
  }

  /// flatten the tree into a single dense buffer;
  /// move the data
  flatten_result_t flatten_move() const
  {
    return flatten<true>();
  }

protected:
  template <typename GeometryFilter, typename DataFunctor>
  bool search_recursive(GeometryFilter& geometry_filter,
                        DataFunctor& data_functor,
                        node_base_type const* node,
                        int level) const
  {
    if (level == leaf_level())
    {
      for (value_type const& element : *node->as_leaf())
      {
        if (data_functor(element))
        {
          return true;
        }
      }
    }
    else
    {
      for (typename node_type::value_type const& child : *node->as_node())
      {
        switch (geometry_filter(child.first))
        {
        case -1:
          return true;
        case 1:
          if (search_recursive(geometry_filter, data_functor, child.second,
                               level + 1))
          {
            return true;
          }
        default:;
        }
      }
    }
    return false;
  }

  template <typename GeometryFilter, typename DataFunctor>
  bool search_recursive(GeometryFilter& geometry_filter,
                        DataFunctor& data_functor,
                        node_base_type* node,
                        int level)
  {
    if (level == leaf_level())
    {
      for (value_type& element : *node->as_leaf())
      {
        if (data_functor(element))
        {
          return true;
        }
      }
    }
    else
    {
      for (typename node_type::value_type& child : *node->as_node())
      {
        switch (geometry_filter(child.first))
        {
        case -1:
          return true;
        case 1:
          if (search_recursive(geometry_filter, data_functor, child.second,
                               level + 1))
          {
            return true;
          }
        default:;
        }
      }
    }
    return false;
  }

  template <typename GeometryFilter, typename ConstItFunctor>
  bool search_iterator_recursive(GeometryFilter& geometry_filter,
                                 ConstItFunctor& it_functor,
                                 node_base_type const* node,
                                 int level) const
  {
    if (level == leaf_level())
    {
      if (!node->as_leaf()->empty())
      {
        for (auto it = const_iterator(&node->as_leaf()->at(0), node->as_leaf());
             it != const_iterator(); ++it)
        {
          if (it_functor(it))
          {
            return true;
          }
        }
      }
    }
    else
    {
      for (typename node_type::value_type const& child : *node->as_node())
      {
        switch (geometry_filter(child.first))
        {
        case -1:
          return true;
        case 1:
          if (search_recursive(geometry_filter, it_functor, child.second,
                               level + 1))
          {
            return true;
          }
        default:;
        }
      }
    }
    return false;
  }

  template <typename GeometryFilter, typename ItFunctor>
  bool search_iterator_recursive(GeometryFilter& geometry_filter,
                                 ItFunctor& it_functor,
                                 node_base_type* node,
                                 int level)
  {
    if (level == leaf_level())
    {
      if (!node->as_leaf()->empty())
      {
        for (auto it = iterator(&node->as_leaf()->at(0), node->as_leaf());
             it != iterator(); ++it)
        {
          if (it_functor(it))
          {
            return true;
          }
        }
      }
    }
    else
    {
      for (typename node_type::value_type& child : *node->as_node())
      {
        switch (geometry_filter(child.first))
        {
        case -1:
          return true;
        case 1:
          if (search_recursive(geometry_filter, it_functor, child.second,
                               level + 1))
          {
            return true;
          }
        default:;
        }
      }
    }
    return false;
  }

public:
  template <typename GeometryFilter, typename ConstDataFunctor>
  void search(GeometryFilter&& geometry_filter,
              ConstDataFunctor&& data_functor) const
  {
    search_recursive(geometry_filter, data_functor, root(), 0);
  }

  template <typename GeometryFilter, typename DataFunctor>
  void search(GeometryFilter&& geometry_filter, DataFunctor&& data_functor)
  {
    search_recursive(geometry_filter, data_functor, root(), 0);
  }

  template <typename GeometryFilter, typename ItFunctor>
  void search_iterator(GeometryFilter&& geometry_filter,
                       ItFunctor&& it_functor) const
  {
    search_iterator_recursive(geometry_filter, it_functor, root(), 0);
  }

  template <typename GeometryFilter, typename ItFunctor>
  void search_iterator(GeometryFilter&& geometry_filter, ItFunctor&& it_functor)
  {
    search_iterator_recursive(geometry_filter, it_functor, root(), 0);
  }

  /// Rebalance the tree.
  /// This function reinserts all the elements in the tree, so that its bounding
  /// box distribution is more balanced.
  void rebalance()
  {
    RTree rtree = RTree(std::make_move_iterator(begin()),
                        std::make_move_iterator(end()));
    *this = std::move(rtree);
  }
};

}
} // namespace eh rtree