#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "geometry_traits.hpp"
#include "global.hpp"
#include "iterator.hpp"
#include "static_node.hpp"

#include "quadratic_split.hpp"
#include "rstar_split.hpp"

namespace eh
{
namespace rtree
{

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          size_type MinEntry = 4u, // m
          size_type MaxEntry = 8u, // M
          template <typename _T> class Allocator = std::allocator // allocator
          >
class RTree
{
  // Node Overflow Splitting Scheme
  // using splitter_t = quadratic_split_t<RTree>;
  using splitter_t = rstar_split_t<RTree>;

public:
  // using stack memory for MaxEntries child nodes. instead of std::vector
  using node_base_type = static_node_base_t<GeometryType,
                                            KeyType,
                                            MappedType,
                                            MinEntry,
                                            MaxEntry>;
  using node_type
      = static_node_t<GeometryType, KeyType, MappedType, MinEntry, MaxEntry>;
  using leaf_type = static_leaf_node_t<GeometryType,
                                       KeyType,
                                       MappedType,
                                       MinEntry,
                                       MaxEntry>;

  using size_type = ::eh::rtree::size_type;

  using geometry_type = GeometryType;
  using traits = geometry_traits<GeometryType>;
  using key_type = KeyType;
  using mapped_type = MappedType;
  using value_type = std::pair<key_type, mapped_type>;

  template <typename __T>
  using allocator_type = Allocator<__T>;

  // type for area
  using area_type = typename geometry_traits<geometry_type>::area_type;
  constexpr static area_type MAX_AREA = std::numeric_limits<area_type>::max();
  constexpr static area_type LOWEST_AREA
      = std::numeric_limits<area_type>::lowest();

  // M
  constexpr static size_type MAX_ENTRIES = MaxEntry;
  // m <= M/2
  constexpr static size_type MIN_ENTRIES = MinEntry;
  static_assert(MIN_ENTRIES <= MAX_ENTRIES / 2, "Invalid MIN_ENTRIES count");
  static_assert(MIN_ENTRIES >= 1, "Invalid MIN_ENTRIES count");

  using iterator = iterator_t<leaf_type>;
  using const_iterator = iterator_t<leaf_type const>;

  using node_iterator = node_iterator_t<node_type>;
  using const_node_iterator = node_iterator_t<node_type const>;

  using leaf_iterator = node_iterator_t<leaf_type>;
  using const_leaf_iterator = node_iterator_t<leaf_type const>;

  template <typename TA, typename TB>
  static bool is_overlap(TA const& a, TB const& b)
  {
    return traits::is_overlap(a, b);
  }
  template <typename TargetBoundary, typename QueryPoint>
  static bool is_inside(TargetBoundary const& target, QueryPoint const& query)
  {
    return traits::is_inside(target, query);
  }

protected:
  node_base_type* _root = nullptr;
  int _leaf_level = 0;

  /*
  ReInsertion Scheme
  When node overflow occurs
  (i.e, trying to add more than MAX_ENTRIES into single node),
  the R*-Tree uses a reinsertion strategy to redistribute the
  entries in the overflowing node.

  `_reinsert_nodes` child nodes of the overflowing node are removed and
  reinserted
  */
  size_type _reinsert_nodes = 0;

  allocator_type<node_type> _node_allocator;
  allocator_type<leaf_type> _leaf_allocator;

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
    assert(target_level <= _leaf_level);
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
      area_type min_area_enlarge = MAX_AREA;
      typename node_type::iterator chosen = n->end();

      for (auto ci = n->begin(); ci != n->end(); ++ci)
      {
        const auto area_enlarge = traits::area(traits::merge(ci->first, bound))
                                  - traits::area(ci->first);
        if (area_enlarge < min_area_enlarge)
        {
          min_area_enlarge = area_enlarge;
          chosen = ci;
        }
        else if (area_enlarge == min_area_enlarge)
        {
          if (traits::area(ci->first) < traits::area(chosen->first))
          {
            chosen = ci;
          }
        }
      }
      assert(chosen != n->end());
      n = chosen->second->as_node();
    }
    return n;
  }
  // adjust bound from node `N` to root recursively
  void broadcast_new_bound(node_type* N)
  {
    while (N->parent())
    {
      N->entry().first = N->calculate_bound();
      N = N->parent();
    }
  }
  // adjust bound from node `leaf` to root recursively
  void broadcast_new_bound(leaf_type* leaf)
  {
    if (leaf->parent())
    {
      leaf->entry().first = leaf->calculate_bound();
      broadcast_new_bound(leaf->parent());
    }
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
          && MAX_ENTRIES + 1 - _reinsert_nodes >= MIN_ENTRIES
          && MAX_ENTRIES + 1 - _reinsert_nodes <= MAX_ENTRIES)
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
    broadcast_new_bound(parent);

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
    splitter_t spliter;
    spliter(node, std::move(child), pair);
    return pair;
  }

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
    const size_type reinsert_count = _reinsert_nodes;

    geometry_type node_bound = node->calculate_bound();
    node_bound = traits::merge(node_bound, child.first);
    std::vector<typename node_type::value_type> children;
    children.reserve(MAX_ENTRIES + 1);
    for (auto& c : *node)
    {
      children.emplace_back(std::move(c));
    }
    children.emplace_back(std::move(child));
    assert(children.size() == MAX_ENTRIES + 1);
    node->clear();

    std::sort(children.begin(), children.end(),
              [&](typename node_type::value_type const& a,
                  typename node_type::value_type const& b)
              {
                return traits::distance_center(node_bound, a.first)
                       < traits::distance_center(node_bound, b.first);
              });
    for (size_type i = 0; i < MAX_ENTRIES + 1 - reinsert_count; ++i)
    {
      node->insert(std::move(children[i]));
    }
    broadcast_new_bound(node);
    for (size_type i = MAX_ENTRIES + 1 - reinsert_count; i <= MAX_ENTRIES; ++i)
    {
      auto& c = children[i];
      node_type* chosen = choose_insert_target(
          c.first, leaf_level() - node_realtive_level_from_leaf);
      insert_node(chosen, std::move(c), false);
    }
  }
  void reinsert(leaf_type* node, typename leaf_type::value_type child)
  {
    const size_type reinsert_count = _reinsert_nodes;

    geometry_type node_bound = node->calculate_bound();
    node_bound = traits::merge(node_bound, child.first);
    std::vector<typename leaf_type::value_type> children;
    children.reserve(MAX_ENTRIES + 1);
    for (auto& c : *node)
    {
      children.emplace_back(std::move(c));
    }
    children.emplace_back(std::move(child));
    node->clear();

    std::sort(children.begin(), children.end(),
              [&](typename leaf_type::value_type const& a,
                  typename leaf_type::value_type const& b)
              {
                return traits::distance_center(node_bound, a.first)
                       < traits::distance_center(node_bound, b.first);
              });
    for (size_type i = 0; i < MAX_ENTRIES + 1 - reinsert_count; ++i)
    {
      node->insert(std::move(children[i]));
    }
    broadcast_new_bound(node);
    for (size_type i = MAX_ENTRIES + 1 - reinsert_count; i <= MAX_ENTRIES; ++i)
    {
      auto& c = children[i];
      leaf_type* chosen
          = choose_insert_target(c.first, leaf_level())->as_leaf();
      insert_node(chosen, std::move(c), false);
    }
  }

public:
  // set the number of reinserted nodes
  void reinsert_nodes(size_type count)
  {
    // max >= max+1-count >= min
    // count >= 1
    // count <= max - min + 1
    _reinsert_nodes = std::min(count, MAX_ENTRIES - MIN_ENTRIES + 1);
  }
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
        for (auto& c : *(reinsert.parent->as_leaf()))
        {
          insert(std::move(c));
        }
        destroy_node(reinsert.parent->as_leaf());
      }
      else
      {
        for (auto& c : *(reinsert.parent->as_node()))
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

  RTree()
  {
    init_root();
    reinsert_nodes(static_cast<size_type>(0.3 * MAX_ENTRIES));
  }

  // @TODO
  // mapped_type copy-assignable
  RTree(RTree const& rhs)
  {
    _reinsert_nodes = rhs._reinsert_nodes;
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
    _reinsert_nodes = rhs._reinsert_nodes;
    return *this;
  }
  RTree(RTree&& rhs)
  {
    _reinsert_nodes = rhs._reinsert_nodes;
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
    _reinsert_nodes = rhs._reinsert_nodes;
    rhs.set_null();
    rhs.init_root();
    return *this;
  }
  ~RTree()
  {
    delete_if();
  }

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
  const_iterator begin() const
  {
    return cbegin();
  }

  iterator end()
  {
    return {};
  }
  const_iterator cend() const
  {
    return {};
  }
  const_iterator end() const
  {
    return {};
  }

  node_iterator begin(int level)
  {
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
  const_node_iterator begin(int level) const
  {
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
  const_node_iterator cbegin(int level) const
  {
    return begin(level);
  }

  node_iterator end(int level)
  {
    return {};
  }
  const_node_iterator end(int level) const
  {
    return {};
  }
  const_node_iterator cend(int level) const
  {
    return {};
  }

  leaf_iterator leaf_begin()
  {
    node_type* n = _root->as_node();
    for (int l = 0; l < _leaf_level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
  }
  const_leaf_iterator leaf_begin() const
  {
    node_type const* n = _root->as_node();
    for (int l = 0; l < _leaf_level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
  }
  const_leaf_iterator leaf_cbegin() const
  {
    return leaf_begin();
  }
  leaf_iterator leaf_end()
  {
    return {};
  }
  const_leaf_iterator leaf_end() const
  {
    return {};
  }
  const_leaf_iterator leaf_cend() const
  {
    return {};
  }

  node_type* root()
  {
    return _root->as_node();
  }
  node_type const* root() const
  {
    return _root->as_node();
  }

  int leaf_level() const
  {
    return _leaf_level;
  }

  auto& node_allocator()
  {
    return _node_allocator;
  }
  auto& leaf_allocator()
  {
    return _leaf_allocator;
  }

  template <typename NodeType>
  typename std::enable_if<std::is_same<NodeType, node_type>::value,
                          NodeType*>::type
  construct_node()
  {
    return new (node_allocator().allocate(1)) NodeType;
  }
  template <typename NodeType>
  typename std::enable_if<std::is_same<NodeType, leaf_type>::value,
                          NodeType*>::type
  construct_node()
  {
    return new (leaf_allocator().allocate(1)) NodeType;
  }
  void destroy_node(node_type* node)
  {
    node->~node_type();
    node_allocator().deallocate(node, 1);
  }
  void destroy_node(leaf_type* node)
  {
    node->~leaf_type();
    leaf_allocator().deallocate(node, 1);
  }

protected:
  template <typename _NodeType, typename _GeometryType, typename Functor>
  static bool search_overlap_wrapper(_NodeType* node,
                                     int leaf,
                                     _GeometryType const& search_range,
                                     Functor functor)
  {
    if (leaf == 0)
    {
      if (search_overlap_leaf_wrapper(node->as_leaf(), search_range, functor))
      {
        return true;
      }
    }
    else
    {
      for (auto& c : *node)
      {
        if (traits::is_overlap(c.first, search_range) == false)
        {
          continue;
        }
        if (search_overlap_wrapper(c.second->as_node(), leaf - 1, search_range,
                                   functor))
        {
          return true;
        }
      }
    }
    return false;
  }

  template <typename _NodeType, typename _GeometryType, typename Functor>
  static bool search_inside_wrapper(_NodeType* node,
                                    int leaf,
                                    _GeometryType const& search_range,
                                    Functor functor)
  {
    if (leaf == 0)
    {
      if (search_inside_leaf_wrapper(node->as_leaf(), search_range, functor))
      {
        return true;
      }
    }
    else
    {
      for (auto& c : *node)
      {
        if (traits::is_overlap(c.first, search_range) == false)
        {
          continue;
        }
        if (search_inside_wrapper(c.second->as_node(), leaf - 1, search_range,
                                  functor))
        {
          return true;
        }
      }
    }
    return false;
  }
  template <typename _LeafType, typename _GeometryType, typename Functor>
  static bool search_overlap_leaf_wrapper(_LeafType* node,
                                          _GeometryType const& search_range,
                                          Functor functor)
  {
    for (auto& c : *node)
    {
      if (traits::is_overlap(c.first, search_range) == false)
      {
        continue;
      }
      if (functor(c))
      {
        return true;
      }
    }
    return false;
  }

  template <typename _LeafType, typename _GeometryType, typename Functor>
  static bool search_inside_leaf_wrapper(_LeafType* node,
                                         _GeometryType const& search_range,
                                         Functor functor)
  {
    for (auto& c : *node)
    {
      if (traits::is_inside(search_range, c.first) == false)
      {
        continue;
      }
      if (functor(c))
      {
        return true;
      }
    }
    return false;
  }

public:
  template <typename _GeometryType, typename Functor>
  void search_inside(_GeometryType const& search_range, Functor functor)
  {
    search_inside_wrapper(root()->as_node(), _leaf_level, search_range,
                          functor);
  }
  template <typename _GeometryType, typename Functor>
  void search_overlap(_GeometryType const& search_range, Functor functor)
  {
    search_overlap_wrapper(root()->as_node(), _leaf_level, search_range,
                           functor);
  }
  template <typename _GeometryType, typename Functor>
  void search_inside(_GeometryType const& search_range, Functor functor) const
  {
    search_inside_wrapper(root()->as_node(), _leaf_level, search_range,
                          functor);
  }
  template <typename _GeometryType, typename Functor>
  void search_overlap(_GeometryType const& search_range, Functor functor) const
  {
    search_overlap_wrapper(root()->as_node(), _leaf_level, search_range,
                           functor);
  }

  struct flatten_node_t
  {
    // offset in global dense buffer
    size_type offset;

    // the number of children
    size_type size;

    // parent node index
    size_type parent;
  };
  struct flatten_result_t
  {
    // leaf node's level
    size_type leaf_level;

    // root node index; must be 0
    size_type root;

    // node data ( include leaf nodes )
    std::vector<flatten_node_t> nodes;

    // global dense buffer of children_boundingbox
    std::vector<geometry_type> children_bound;
    // global dense buffer of children index
    std::vector<size_type> children;

    // real data
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
  template <bool Move = false>
  flatten_result_t flatten() const
  {
    flatten_result_t res;
    res.leaf_level = leaf_level();
    res.root = 0;
    flatten_recursive<Move>(res, root(), 0, 0);
    return res;
  }
};

}
} // namespace eh rtree