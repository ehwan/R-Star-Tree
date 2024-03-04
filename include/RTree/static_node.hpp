#pragma once

#include <cassert>
#include <cstdint>
#include <iterator>
#include <utility>

#include "geometry_traits.hpp"
#include "global.hpp"
#include "static_vector.hpp"

namespace eh
{
namespace rtree
{

template <typename TreeType>
struct static_node_t;

template <typename TreeType>
struct static_leaf_node_t;

template <typename TreeType>
struct static_node_base_t
{
  using node_base_type = static_node_base_t;
  using node_type = static_node_t<TreeType>;
  using leaf_type = static_leaf_node_t<TreeType>;

  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;

  node_type* _parent = nullptr;
  size_type _index_on_parent;

  // parent node's pointer
  EH_RTREE_DEVICE_HOST node_type* parent() const
  {
    return _parent;
  }
  EH_RTREE_DEVICE_HOST bool is_root() const
  {
    return _parent == nullptr;
  }

  EH_RTREE_DEVICE_HOST auto& entry()
  {
    return parent()->at(_index_on_parent);
  }
  EH_RTREE_DEVICE_HOST auto const& entry() const
  {
    return parent()->at(_index_on_parent);
  }

  EH_RTREE_DEVICE_HOST inline node_type* as_node()
  {
    return reinterpret_cast<node_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline node_type const* as_node() const
  {
    return reinterpret_cast<node_type const*>(this);
  }
  EH_RTREE_DEVICE_HOST inline leaf_type* as_leaf()
  {
    return reinterpret_cast<leaf_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline leaf_type const* as_leaf() const
  {
    return reinterpret_cast<leaf_type const*>(this);
  }

  // get next node on same level
  // this would return node across different parent
  // if it is last node, return nullptr
  EH_RTREE_DEVICE_HOST node_base_type* next()
  {
    // if n is root
    if (_parent == nullptr)
    {
      return nullptr;
    }

    // if n is last node on parent
    // return parent's next's 0th child node
    if (_index_on_parent == parent()->size() - 1)
    {
      node_type* n = parent()->next();
      if (n == nullptr)
      {
        return nullptr;
      }
      return n->front().second;
    }
    else
    {
      // else; return next node in same parent
      return parent()->at(_index_on_parent + 1).second;
    }
  }
  EH_RTREE_DEVICE_HOST node_base_type const* next() const
  {
    // if n is root
    if (this->_parent == nullptr)
    {
      return nullptr;
    }

    // if n is last node on parent
    // return parent's next's 0th child node
    if (this->_index_on_parent == parent()->size() - 1)
    {
      node_type const* n = parent()->next();
      if (n == nullptr)
      {
        return nullptr;
      }
      return n->front().second;
    }
    else
    {
      // else; return next node in same parent
      return parent()->at(_index_on_parent + 1).second;
    }
  }
  // get prev node on same level
  // this would return node across different parent
  // if it is first node, return nullptr
  EH_RTREE_DEVICE_HOST node_base_type* prev()
  {
    // if n is root
    if (this->_parent == nullptr)
    {
      return nullptr;
    }

    // if n is last node on parent
    // return parent's next's 0th child node
    if (this->_index_on_parent == 0)
    {
      node_type* n = parent()->prev();
      if (n == nullptr)
      {
        return nullptr;
      }
      return n->back().second;
    }
    else
    {
      // else; return next node in same parent
      return parent()->at(_index_on_parent - 1).second;
    }
  }
  EH_RTREE_DEVICE_HOST node_base_type const* prev() const
  {
    // if n is root
    if (this->_parent == nullptr)
    {
      return nullptr;
    }

    // if n is last node on parent
    // return parent's next's 0th child node
    if (this->_index_on_parent == 0)
    {
      node_type const* n = parent()->prev();
      if (n == nullptr)
      {
        return nullptr;
      }
      return n->back().second;
    }
    else
    {
      // else; return next node in same parent
      return parent()->at(_index_on_parent - 1).second;
    }
  }
};

template <typename TreeType>
struct static_node_t
{
  using parent_type = static_node_base_t<TreeType>;
  using node_base_type = parent_type;
  using node_type = static_node_t;
  using leaf_type = static_leaf_node_t<TreeType>;
  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;
  using value_type = pair<geometry_type, node_base_type*>;

  using iterator = value_type*;
  using const_iterator = value_type const*;

  // c-style class inheritance;
  // for CUDA CPU<->GPU memory layout mismatch
  parent_type _parent_class;
  static_vector<value_type, TreeType::MAX_ENTRIES> _child;

  static_node_t() = default;
  static_node_t(static_node_t const&) = delete;
  static_node_t& operator=(static_node_t const&) = delete;
  static_node_t(static_node_t&&) = delete;
  static_node_t& operator=(static_node_t&&) = delete;

  // add child node with bounding box
  void insert(value_type child)
  {
    assert(size() < TreeType::MAX_ENTRIES);
    child.second->_parent = this;
    child.second->_index_on_parent = size();
    _child.push_back(static_cast<value_type&&>(child));
  }
  void erase(node_base_type* node)
  {
    assert(node->_parent == this);
    assert(size() > 0);
    if (node->_index_on_parent < size() - 1)
    {
      back().second->_index_on_parent = node->_index_on_parent;
      at(node->_index_on_parent) = static_cast<value_type&&>(back());
    }
    node->_parent = nullptr;
    pop_back();
  }
  void erase(iterator pos)
  {
    erase(pos->second);
  }

  void clear()
  {
    _child.clear();
  }

  // swap two different child node (i, j)
  void swap(size_type i, size_type j)
  {
    assert(i != j);
    assert(i < size());
    assert(j < size());

    std::swap(at(i), at(j));
    at(i).second->_index_on_parent = i;
    at(j).second->_index_on_parent = j;
  }
  void pop_back()
  {
    assert(size() > 0);
    _child.pop_back();
  }

  // child count
  EH_RTREE_DEVICE_HOST size_type size() const
  {
    return _child.size();
  }
  EH_RTREE_DEVICE_HOST bool empty() const
  {
    return _child.empty();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST iterator begin()
  {
    return _child.begin();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST const_iterator begin() const
  {
    return _child.begin();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST iterator end()
  {
    return _child.end();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST const_iterator end() const
  {
    return _child.end();
  }

  EH_RTREE_DEVICE_HOST value_type& at(size_type i)
  {
    assert(i >= 0);
    assert(i < size());
    return _child[i];
  }
  EH_RTREE_DEVICE_HOST value_type const& at(size_type i) const
  {
    assert(i >= 0);
    assert(i < size());
    return _child[i];
  }
  EH_RTREE_DEVICE_HOST value_type& operator[](size_type i)
  {
    return at(i);
  }
  EH_RTREE_DEVICE_HOST value_type const& operator[](size_type i) const
  {
    return at(i);
  }
  EH_RTREE_DEVICE_HOST value_type& front()
  {
    assert(size() > 0);
    return at(0);
  }
  EH_RTREE_DEVICE_HOST value_type const& front() const
  {
    assert(size() > 0);
    return at(0);
  }
  EH_RTREE_DEVICE_HOST value_type& back()
  {
    assert(size() > 0);
    return at(size() - 1);
  }
  EH_RTREE_DEVICE_HOST value_type const& back() const
  {
    assert(size() > 0);
    return at(size() - 1);
  }

  EH_RTREE_DEVICE_HOST value_type* data()
  {
    return _child.data();
  }
  EH_RTREE_DEVICE_HOST value_type const* data() const
  {
    return _child.data();
  }

  // union bouinding box of children
  EH_RTREE_DEVICE_HOST geometry_type calculate_bound() const
  {
    assert(empty() == false);
    geometry_type merged = at(0).first;
    for (size_type i = 1; i < size(); ++i)
    {
      merged = geometry_traits<geometry_type>::merge(merged, at(i).first);
    }
    return merged;
  }

  // delete its child recursively
  template <typename NodeAllocator, typename LeafAllocator>
  void delete_recursive(int leaf_level,
                        NodeAllocator&& node_allocator,
                        LeafAllocator&& leaf_allocator)
  {
    if (leaf_level == 1)
    {
      // child is leaf node
      for (auto& c : *this)
      {
        c.second->as_leaf()->delete_recursive(leaf_allocator);
        c.second->as_leaf()->~leaf_type();
        leaf_allocator.deallocate(c.second->as_leaf(), 1);
      }
    }
    else
    {
      for (auto& c : *this)
      {
        c.second->as_node()->delete_recursive(leaf_level - 1, node_allocator,
                                              leaf_allocator);
        c.second->as_node()->~node_type();
        node_allocator.deallocate(c.second->as_node(), 1);
      }
    }
  }
  template <typename NodeAllocator, typename LeafAllocator>
  node_type* clone_recursive(int leaf_level,
                             NodeAllocator&& node_allocator,
                             LeafAllocator&& leaf_allocator) const
  {
    node_type* new_node = node_allocator.allocate(1);
    new (new_node) node_type();
    if (leaf_level == 1)
    {
      // child is leaf node
      for (auto& c : *this)
      {
        new_node->insert(value_type {
            c.first,
            c.second->as_leaf()->clone_recursive(leaf_allocator)->as_base() });
      }
    }
    else
    {
      for (auto& c : *this)
      {
        new_node->insert(value_type {
            c.first, c.second->as_node()
                         ->clone_recursive(leaf_level - 1, node_allocator,
                                           leaf_allocator)
                         ->as_base() });
      }
    }
    return new_node;
  }
  EH_RTREE_DEVICE_HOST size_type size_recursive(int leaf_level) const
  {
    size_type ret = 0;
    if (leaf_level == 1)
    {
      // child is leaf node
      for (auto& c : *this)
      {
        ret += c.second->as_leaf()->size_recursive();
      }
    }
    else
    {
      for (auto& c : *this)
      {
        ret += c.second->as_node()->size_recursive(leaf_level - 1);
      }
    }
    return ret;
  }

  EH_RTREE_DEVICE_HOST node_type* next()
  {
    return _parent_class.next()->as_node();
  }
  EH_RTREE_DEVICE_HOST node_type const* next() const
  {
    return _parent_class.next()->as_node();
  }
  EH_RTREE_DEVICE_HOST node_type* prev()
  {
    return _parent_class.prev()->as_node();
  }
  EH_RTREE_DEVICE_HOST node_type const* prev() const
  {
    return _parent_class.prev()->as_node();
  }

  // parent node's pointer
  EH_RTREE_DEVICE_HOST node_type* parent() const
  {
    return _parent_class.parent();
  }
  EH_RTREE_DEVICE_HOST bool is_root() const
  {
    return _parent_class.is_root();
  }

  EH_RTREE_DEVICE_HOST auto& entry()
  {
    return parent()->at(_parent_class._index_on_parent);
  }
  EH_RTREE_DEVICE_HOST auto const& entry() const
  {
    return parent()->at(_parent_class._index_on_parent);
  }

  EH_RTREE_DEVICE_HOST inline node_type* as_node()
  {
    return reinterpret_cast<node_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline node_type const* as_node() const
  {
    return reinterpret_cast<node_type const*>(this);
  }
  EH_RTREE_DEVICE_HOST inline leaf_type* as_leaf()
  {
    return reinterpret_cast<leaf_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline leaf_type const* as_leaf() const
  {
    return reinterpret_cast<leaf_type const*>(this);
  }
  EH_RTREE_DEVICE_HOST inline node_base_type* as_base()
  {
    return reinterpret_cast<node_base_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline node_base_type const* as_base() const
  {
    return reinterpret_cast<node_base_type const*>(this);
  }
};

template <typename TreeType>
struct static_leaf_node_t
{
  using parent_type = static_node_base_t<TreeType>;
  using node_base_type = parent_type;
  using node_type = static_node_t<TreeType>;
  using leaf_type = static_leaf_node_t;
  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;
  using value_type = pair<key_type, mapped_type>;

  using iterator = value_type*;
  using const_iterator = value_type const*;

  // c-style class inheritance;
  // for CUDA CPU<->GPU memory layout mismatch
  parent_type _parent_class;
  static_vector<value_type, TreeType::MAX_ENTRIES> _child;

  static_leaf_node_t() = default;
  static_leaf_node_t(static_leaf_node_t const&) = delete;
  static_leaf_node_t& operator=(static_leaf_node_t const&) = delete;
  static_leaf_node_t(static_leaf_node_t&&) = delete;
  static_leaf_node_t& operator=(static_leaf_node_t&&) = delete;

  // add child node with bounding box
  void insert(value_type child)
  {
    assert(size() < TreeType::MAX_ENTRIES);
    _child.push_back(static_cast<value_type&&>(child));
  }
  void erase(value_type* pos)
  {
    assert(size() > 0);
    assert(std::distance(data(), pos) < size());
    if (std::distance(data(), pos) < size() - 1)
    {
      *pos = static_cast<value_type&&>(back());
    }
    pop_back();
  }

  void clear()
  {
    _child.clear();
  }

  // swap two different child node (i, j)
  void swap(size_type i, size_type j)
  {
    assert(i != j);
    assert(i < size());
    assert(j < size());

    std::swap(at(i), at(j));
  }
  void pop_back()
  {
    assert(size() > 0);
    _child.pop_back();
  }

  // child count
  EH_RTREE_DEVICE_HOST size_type size() const
  {
    return _child.size();
  }
  EH_RTREE_DEVICE_HOST bool empty() const
  {
    return _child.empty();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST iterator begin()
  {
    return _child.begin();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST const_iterator begin() const
  {
    return _child.begin();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST iterator end()
  {
    return _child.end();
  }
  // child iterator
  EH_RTREE_DEVICE_HOST const_iterator end() const
  {
    return _child.end();
  }

  EH_RTREE_DEVICE_HOST value_type& at(size_type i)
  {
    assert(i >= 0);
    assert(i < size());
    return _child[i];
  }
  EH_RTREE_DEVICE_HOST value_type const& at(size_type i) const
  {
    assert(i >= 0);
    assert(i < size());
    return _child[i];
  }
  EH_RTREE_DEVICE_HOST value_type& operator[](size_type i)
  {
    return at(i);
  }
  EH_RTREE_DEVICE_HOST value_type const& operator[](size_type i) const
  {
    return at(i);
  }
  EH_RTREE_DEVICE_HOST value_type& front()
  {
    assert(size() > 0);
    return at(0);
  }
  EH_RTREE_DEVICE_HOST value_type const& front() const
  {
    assert(size() > 0);
    return at(0);
  }
  EH_RTREE_DEVICE_HOST value_type& back()
  {
    assert(size() > 0);
    return at(size() - 1);
  }
  EH_RTREE_DEVICE_HOST value_type const& back() const
  {
    assert(size() > 0);
    return at(size() - 1);
  }
  EH_RTREE_DEVICE_HOST value_type* data()
  {
    return _child.data();
  }
  EH_RTREE_DEVICE_HOST value_type const* data() const
  {
    return _child.data();
  }

  // union bouinding box of children
  EH_RTREE_DEVICE_HOST geometry_type calculate_bound() const
  {
    assert(empty() == false);
    geometry_type merged = at(0).first;
    for (int i = 1; i < size(); ++i)
    {
      merged = geometry_traits<geometry_type>::merge(merged, at(i).first);
    }
    return merged;
  }

  // delete its child recursively
  template <typename LeafAllocator>
  void delete_recursive(LeafAllocator&& leaf_allocator)
  {
  }
  template <typename LeafAllocator>
  leaf_type* clone_recursive(LeafAllocator&& leaf_allocator) const
  {
    leaf_type* new_node = leaf_allocator.allocate(1);
    new (new_node) leaf_type();
    for (auto& c : *this)
    {
      new_node->insert(c);
    }
    return new_node;
  }
  EH_RTREE_DEVICE_HOST size_type size_recursive() const
  {
    return size();
  }

  EH_RTREE_DEVICE_HOST leaf_type* next()
  {
    return _parent_class.next()->as_leaf();
  }
  EH_RTREE_DEVICE_HOST leaf_type const* next() const
  {
    return _parent_class.next()->as_leaf();
  }
  EH_RTREE_DEVICE_HOST leaf_type* prev()
  {
    return _parent_class.prev()->as_leaf();
  }
  EH_RTREE_DEVICE_HOST leaf_type const* prev() const
  {
    return _parent_class.prev()->as_leaf();
  }

  // parent node's pointer
  EH_RTREE_DEVICE_HOST node_type* parent() const
  {
    return _parent_class.parent();
  }
  EH_RTREE_DEVICE_HOST bool is_root() const
  {
    return _parent_class.is_root();
  }

  EH_RTREE_DEVICE_HOST auto& entry()
  {
    return parent()->at(_parent_class._index_on_parent);
  }
  EH_RTREE_DEVICE_HOST auto const& entry() const
  {
    return parent()->at(_parent_class._index_on_parent);
  }

  EH_RTREE_DEVICE_HOST inline node_type* as_node()
  {
    return reinterpret_cast<node_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline node_type const* as_node() const
  {
    return reinterpret_cast<node_type const*>(this);
  }
  EH_RTREE_DEVICE_HOST inline leaf_type* as_leaf()
  {
    return reinterpret_cast<leaf_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline leaf_type const* as_leaf() const
  {
    return reinterpret_cast<leaf_type const*>(this);
  }
  EH_RTREE_DEVICE_HOST inline node_base_type* as_base()
  {
    return reinterpret_cast<node_base_type*>(this);
  }
  EH_RTREE_DEVICE_HOST inline node_base_type const* as_base() const
  {
    return reinterpret_cast<node_base_type const*>(this);
  }
};

}
} // namespace eh rtree