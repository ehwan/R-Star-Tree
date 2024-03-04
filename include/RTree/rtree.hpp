#pragma once

/*
References:
Antonin Guttman, R-Trees: A Dynamic Index Structure for Spatial Searching,
University if California Berkeley
*/

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "geometry_traits.hpp"
#include "global.hpp"
#include "iterator.hpp"
#include "quadratic_split.hpp"
#include "sequence_split.hpp"
#include "static_node.hpp"

namespace eh
{
namespace rtree
{

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          unsigned int MinEntry = 4, // m
          unsigned int MaxEntry = 8, // M
          template <typename _T> class Allocator = std::allocator // allocator
          >
class RTree
{
  template <typename _GeometryType,
            typename _KeyType,
            typename _MappedType,
            unsigned int _MinEntry,
            unsigned int _MaxEntry,
            template <typename __T>
            class _Allocator>
  friend class RTree;

public:
  // using stack memory for MaxEntries child nodes. instead of std::vector
  using node_base_type = static_node_base_t<RTree>;
  using node_type = static_node_t<RTree>;
  using leaf_type = static_leaf_node_t<RTree>;

  using size_type = unsigned int;

  using geometry_type = GeometryType;
  using traits = geometry_traits<GeometryType>;
  using key_type = KeyType;
  using mapped_type = MappedType;
  using value_type = pair<key_type, mapped_type>;

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

protected:
  node_base_type* _root = nullptr;
  int _leaf_level = 0;

  allocator_type<node_type> _node_allocator;
  allocator_type<leaf_type> _leaf_allocator;

  void delete_if()
  {
    if (_root)
    {
      if (_leaf_level == 0)
      {
        // root is leaf node
        _root->as_leaf()->delete_recursive(leaf_allocator());
        destroy_node(_root->as_leaf());
      }
      else
      {
        _root->as_node()->delete_recursive(_leaf_level, node_allocator(),
                                           leaf_allocator());
        destroy_node(_root->as_node());
      }
    }
  }
  EH_RTREE_DEVICE_HOST void set_null()
  {
    _root = nullptr;
    _leaf_level = 0;
  }

  void init_root()
  {
    if (_root == nullptr)
    {
      _root = leaf_allocator().allocate(1)->as_base();
      new (_root->as_leaf()) leaf_type;
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
  void broadcast_new_bound(node_type* N)
  {
    while (N->parent())
    {
      N->entry().first = N->calculate_bound();
      N = N->parent();
    }
  }
  void broadcast_new_bound(leaf_type* leaf)
  {
    if (leaf->parent())
    {
      leaf->entry().first = leaf->calculate_bound();
      broadcast_new_bound(leaf->parent());
    }
  }

  // insert node to given parent
  void insert_node(geometry_type const& bound,
                   node_base_type* node,
                   node_type* parent)
  {
    node_type* pair = nullptr;
    if (parent->size() == MAX_ENTRIES)
    {
      pair = split(parent, typename node_type::value_type { bound, node });
    }
    else
    {
      parent->insert({ bound, node });
    }
    broadcast_new_bound(parent);
    if (pair)
    {
      if (parent->as_base() == _root)
      {
        node_type* new_root = node_allocator().allocate(1);
        new (new_root) node_type;
        new_root->insert({ parent->calculate_bound(), parent->as_base() });
        new_root->insert({ pair->calculate_bound(), pair->as_base() });
        _root = new_root->as_base();
        ++_leaf_level;
      }
      else
      {
        insert_node(pair->calculate_bound(), pair->as_base(), parent->parent());
      }
    }
  }

  // using splitter_t = sequence_split_t<RTree>;
  using splitter_t = quadratic_split_t<RTree>;

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

public:
  void insert(value_type new_val)
  {
    leaf_type* chosen
        = choose_insert_target(new_val.first, _leaf_level)->as_leaf();
    leaf_type* pair = nullptr;
    if (chosen->size() == MAX_ENTRIES)
    {
      pair = split(chosen, std::move(new_val));
    }
    else
    {
      chosen->insert(std::move(new_val));
    }
    broadcast_new_bound(chosen);
    if (pair)
    {
      if (chosen->parent() == nullptr)
      {
        node_type* new_root = construct_node<node_type>();
        new_root->insert({ chosen->calculate_bound(), chosen->as_base() });
        new_root->insert({ pair->calculate_bound(), pair->as_base() });
        _root = new_root->as_base();
        ++_leaf_level;
      }
      else
      {
        insert_node(pair->calculate_bound(), pair->as_base(), chosen->parent());
      }
    }
  }

  void erase(iterator pos)
  {
    leaf_type* leaf = pos._leaf;
    leaf->erase(pos._pointer);

    if (leaf == _root->as_leaf())
    {
      return;
    }

    // relative level from leaf, node's children will be reinserted later.
    std::vector<pair<int, node_base_type*>> reinsert_nodes;

    node_type* node = leaf->parent();
    if (leaf->size() < MIN_ENTRIES)
    {
      // delete node from node's parent
      node->erase(leaf->as_base());

      // insert node to set
      reinsert_nodes.emplace_back(0, leaf->as_base());
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
        parent->erase(node->as_base());
        // insert node to set
        reinsert_nodes.emplace_back(_leaf_level - level, node->as_base());
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
    for (auto reinsert : reinsert_nodes)
    {
      // leaf node
      if (reinsert.first == 0)
      {
        for (auto& c : *reinsert.second->as_leaf())
        {
          insert(std::move(c));
        }
        destroy_node(reinsert.second->as_leaf());
      }
      else
      {
        for (auto& c : *reinsert.second->as_node())
        {
          node_type* chosen
              = choose_insert_target(c.first, _leaf_level - reinsert.first);
          insert_node(c.first, c.second, chosen);
        }
        destroy_node(reinsert.second->as_node());
      }
    }
  }

  EH_RTREE_DEVICE_HOST size_type size() const
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
  }

  RTree(RTree const& rhs)
  {
    if (rhs._leaf_level == 0)
    {
      _root
          = rhs._root->as_leaf()->clone_recursive(leaf_allocator())->as_base();
      _leaf_level = 0;
    }
    else
    {
      _root = rhs._root->as_node()
                  ->clone_recursive(rhs._leaf_level, node_allocator(),
                                    leaf_allocator())
                  ->as_base();
      _leaf_level = rhs._leaf_level;
    }
  }
  RTree& operator=(RTree const& rhs)
  {
    delete_if();
    if (rhs._leaf_level == 0)
    {
      _root
          = rhs._root->as_leaf()->clone_recursive(leaf_allocator())->as_base();
      _leaf_level = 0;
    }
    else
    {
      _root = rhs._root->as_node()
                  ->clone_recursive(rhs._leaf_level, node_allocator(),
                                    leaf_allocator())
                  ->as_base();
      _leaf_level = rhs._leaf_level;
    }
    return *this;
  }

  // copy construct from different allocator
  template <template <typename> class _Allocator>
  RTree(RTree<GeometryType,
              KeyType,
              MappedType,
              MinEntry,
              MaxEntry,
              _Allocator> const& rhs)
  {
    if (rhs._leaf_level == 0)
    {
      _root
          = rhs._root->as_leaf()->clone_recursive(leaf_allocator())->as_base();
      _leaf_level = 0;
    }
    else
    {
      _root = rhs._root->as_node()
                  ->clone_recursive(rhs._leaf_level, node_allocator(),
                                    leaf_allocator())
                  ->as_base();
      _leaf_level = rhs._leaf_level;
    }
  }
  template <template <typename> class _Allocator>
  RTree& operator=(RTree<GeometryType,
                         KeyType,
                         MappedType,
                         MinEntry,
                         MaxEntry,
                         _Allocator> const& rhs)
  {
    delete_if();
    if (rhs._leaf_level == 0)
    {
      _root
          = rhs._root->as_leaf()->clone_recursive(leaf_allocator())->as_base();
      _leaf_level = 0;
    }
    else
    {
      _root = rhs._root->as_node()
                  ->clone_recursive(rhs._leaf_level, node_allocator(),
                                    leaf_allocator())
                  ->as_base();
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
  ~RTree()
  {
    delete_if();
  }

  EH_RTREE_DEVICE_HOST iterator begin()
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
  EH_RTREE_DEVICE_HOST const_iterator cbegin() const
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
  EH_RTREE_DEVICE_HOST const_iterator begin() const
  {
    return cbegin();
  }

  EH_RTREE_DEVICE_HOST iterator end()
  {
    return {};
  }
  EH_RTREE_DEVICE_HOST const_iterator cend() const
  {
    return {};
  }
  EH_RTREE_DEVICE_HOST const_iterator end() const
  {
    return {};
  }

  EH_RTREE_DEVICE_HOST node_iterator begin(int level)
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
  EH_RTREE_DEVICE_HOST const_node_iterator begin(int level) const
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
  EH_RTREE_DEVICE_HOST const_node_iterator cbegin(int level) const
  {
    return begin(level);
  }

  EH_RTREE_DEVICE_HOST node_iterator end(int level)
  {
    return {};
  }
  EH_RTREE_DEVICE_HOST const_node_iterator end(int level) const
  {
    return {};
  }
  EH_RTREE_DEVICE_HOST const_node_iterator cend(int level) const
  {
    return {};
  }

  EH_RTREE_DEVICE_HOST leaf_iterator leaf_begin()
  {
    node_type* n = _root->as_node();
    for (int l = 0; l < _leaf_level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
  }
  EH_RTREE_DEVICE_HOST const_leaf_iterator leaf_begin() const
  {
    node_type const* n = _root->as_node();
    for (int l = 0; l < _leaf_level; ++l)
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
  }
  EH_RTREE_DEVICE_HOST const_leaf_iterator leaf_cbegin() const
  {
    return leaf_begin();
  }
  EH_RTREE_DEVICE_HOST leaf_iterator leaf_end()
  {
    return {};
  }
  EH_RTREE_DEVICE_HOST const_leaf_iterator leaf_end() const
  {
    return {};
  }
  EH_RTREE_DEVICE_HOST const_leaf_iterator leaf_cend() const
  {
    return {};
  }

  EH_RTREE_DEVICE_HOST node_type* root()
  {
    return _root->as_node();
  }
  EH_RTREE_DEVICE_HOST node_type const* root() const
  {
    return _root->as_node();
  }

  EH_RTREE_DEVICE_HOST int leaf_level() const
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
    node_type* ret = node_allocator().allocate(1);
    new (ret) node_type;
    return ret;
  }
  template <typename NodeType>
  typename std::enable_if<std::is_same<NodeType, leaf_type>::value,
                          NodeType*>::type
  construct_node()
  {
    leaf_type* ret = leaf_allocator().allocate(1);
    new (ret) leaf_type;
    return ret;
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
  EH_RTREE_DEVICE_HOST static bool
  search_overlap_wrapper(_NodeType* node,
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
  EH_RTREE_DEVICE_HOST static bool
  search_inside_wrapper(_NodeType* node,
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
  EH_RTREE_DEVICE_HOST static bool
  search_overlap_leaf_wrapper(_LeafType* node,
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
  EH_RTREE_DEVICE_HOST static bool
  search_inside_leaf_wrapper(_LeafType* node,
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
  EH_RTREE_DEVICE_HOST void search_inside(_GeometryType const& search_range,
                                          Functor functor)
  {
    search_inside_wrapper(root()->as_node(), _leaf_level, search_range,
                          functor);
  }
  template <typename _GeometryType, typename Functor>
  EH_RTREE_DEVICE_HOST void search_overlap(_GeometryType const& search_range,
                                           Functor functor)
  {
    search_overlap_wrapper(root()->as_node(), _leaf_level, search_range,
                           functor);
  }
  template <typename _GeometryType, typename Functor>
  EH_RTREE_DEVICE_HOST void search_inside(_GeometryType const& search_range,
                                          Functor functor) const
  {
    search_inside_wrapper(root()->as_node(), _leaf_level, search_range,
                          functor);
  }
  template <typename _GeometryType, typename Functor>
  EH_RTREE_DEVICE_HOST void search_overlap(_GeometryType const& search_range,
                                           Functor functor) const
  {
    search_overlap_wrapper(root()->as_node(), _leaf_level, search_range,
                           functor);
  }
};

}
} // namespace eh rtree