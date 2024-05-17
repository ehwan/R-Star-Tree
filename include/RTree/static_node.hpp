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

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          size_type MinEntry = 4, // m
          size_type MaxEntry = 8 // M
          >
struct static_node_t;

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          size_type MinEntry = 4, // m
          size_type MaxEntry = 8 // M
          >
struct static_leaf_node_t;

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          size_type MinEntry = 4, // m
          size_type MaxEntry = 8 // M
          >
struct static_node_base_t
{
  using node_base_type = static_node_base_t;
  using node_type
      = static_node_t<GeometryType, KeyType, MappedType, MinEntry, MaxEntry>;
  using leaf_type = static_leaf_node_t<GeometryType,
                                       KeyType,
                                       MappedType,
                                       MinEntry,
                                       MaxEntry>;

  using size_type = ::eh::rtree::size_type;
  using geometry_type = GeometryType;
  using key_type = KeyType;
  using mapped_type = MappedType;

  node_type* _parent = nullptr;
  size_type _index_on_parent;

  // parent node's pointer
  node_type* parent() const
  {
    return _parent;
  }
  bool is_root() const
  {
    return _parent == nullptr;
  }

  auto& entry()
  {
    return parent()->at(_index_on_parent);
  }
  auto const& entry() const
  {
    return parent()->at(_index_on_parent);
  }

  inline node_type* as_node()
  {
    return reinterpret_cast<node_type*>(this);
  }
  inline node_type const* as_node() const
  {
    return reinterpret_cast<node_type const*>(this);
  }
  inline leaf_type* as_leaf()
  {
    return reinterpret_cast<leaf_type*>(this);
  }
  inline leaf_type const* as_leaf() const
  {
    return reinterpret_cast<leaf_type const*>(this);
  }

  int level_recursive() const
  {
    if (parent() == nullptr)
    {
      return 0;
    }
    return parent()->level_recursive() + 1;
  }

  // get next node on same level
  // this would return node across different parent
  // if it is last node, return nullptr
  node_base_type* next()
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
  node_base_type const* next() const
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
  node_base_type* prev()
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
  node_base_type const* prev() const
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

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          size_type MinEntry, // m
          size_type MaxEntry // M
          >
struct static_node_t
    : public static_node_base_t<GeometryType,
                                KeyType,
                                MappedType,
                                MinEntry,
                                MaxEntry>
{
  using parent_type = static_node_base_t<GeometryType,
                                         KeyType,
                                         MappedType,
                                         MinEntry,
                                         MaxEntry>;
  using node_base_type = parent_type;
  using node_type = static_node_t;
  using leaf_type = static_leaf_node_t<GeometryType,
                                       KeyType,
                                       MappedType,
                                       MinEntry,
                                       MaxEntry>;
  using size_type = typename parent_type::size_type;
  using geometry_type = GeometryType;
  using key_type = KeyType;
  using mapped_type = MappedType;
  using value_type = std::pair<geometry_type, node_base_type*>;

  using iterator = value_type*;
  using const_iterator = value_type const*;

  static_vector<value_type, MaxEntry> _children;

  static_node_t() = default;
  static_node_t(static_node_t const&) = delete;
  static_node_t& operator=(static_node_t const&) = delete;
  static_node_t(static_node_t&&) = delete;
  static_node_t& operator=(static_node_t&&) = delete;

  // add child node with bounding box
  void insert(value_type child)
  {
    assert(size() < MaxEntry);
    child.second->_parent = this;
    child.second->_index_on_parent = size();
    _children.emplace_back(std::move(child));
  }
  void erase(node_base_type* node)
  {
    assert(node->_parent == this);
    assert(size() > 0);
    if (node->_index_on_parent < size() - 1)
    {
      back().second->_index_on_parent = node->_index_on_parent;
      at(node->_index_on_parent) = std::move(back());
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
    _children.clear();
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
    _children.pop_back();
  }

  // child count
  size_type size() const
  {
    return _children.size();
  }
  bool empty() const
  {
    return _children.empty();
  }
  // child iterator
  iterator begin()
  {
    return _children.begin();
  }
  // child iterator
  const_iterator begin() const
  {
    return _children.begin();
  }
  // child iterator
  iterator end()
  {
    return _children.end();
  }
  // child iterator
  const_iterator end() const
  {
    return _children.end();
  }

  value_type& at(size_type i)
  {
    assert(i >= 0);
    assert(i < size());
    return _children.at(i);
  }
  value_type const& at(size_type i) const
  {
    assert(i >= 0);
    assert(i < size());
    return _children.at(i);
  }
  value_type& operator[](size_type i)
  {
    return at(i);
  }
  value_type const& operator[](size_type i) const
  {
    return at(i);
  }
  value_type& front()
  {
    assert(size() > 0);
    return at(0);
  }
  value_type const& front() const
  {
    assert(size() > 0);
    return at(0);
  }
  value_type& back()
  {
    assert(size() > 0);
    return at(size() - 1);
  }
  value_type const& back() const
  {
    assert(size() > 0);
    return at(size() - 1);
  }

  value_type* data()
  {
    return _children.data();
  }
  value_type const* data() const
  {
    return _children.data();
  }

  // union bouinding box of children
  geometry_type calculate_bound() const
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
  template <typename TreeType>
  void delete_recursive(int leaf_level, TreeType& tree)
  {
    if (leaf_level == 1)
    {
      // child is leaf node
      for (auto& c : *this)
      {
        c.second->as_leaf()->delete_recursive(tree);
        tree.destroy_node(c.second->as_leaf());
      }
    }
    else
    {
      for (auto& c : *this)
      {
        c.second->as_node()->delete_recursive(leaf_level - 1, tree);
        tree.destroy_node(c.second->as_node());
      }
    }
  }
  template <typename TreeType>
  node_type* clone_recursive(int leaf_level, TreeType& tree) const
  {
    node_type* new_node = tree.template construct_node<node_type>();
    if (leaf_level == 1)
    {
      // child is leaf node
      for (auto& c : *this)
      {
        new_node->insert(
            { c.first, c.second->as_leaf()->clone_recursive(tree) });
      }
    }
    else
    {
      for (auto& c : *this)
      {
        new_node->insert({ c.first, c.second->as_node()->clone_recursive(
                                        leaf_level - 1, tree) });
      }
    }
    return new_node;
  }
  size_type size_recursive(int leaf_level) const
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

  node_type* next()
  {
    return parent_type::next()->as_node();
  }
  node_type const* next() const
  {
    return parent_type::next()->as_node();
  }
  node_type* prev()
  {
    return parent_type::prev()->as_node();
  }
  node_type const* prev() const
  {
    return parent_type::prev()->as_node();
  }
};

template <typename GeometryType, // bounding box representation
          typename KeyType, // key type, either bounding box or point
          typename MappedType, // mapped type, user defined
          size_type MinEntry, // m
          size_type MaxEntry // M
          >
struct static_leaf_node_t
    : public static_node_base_t<GeometryType,
                                KeyType,
                                MappedType,
                                MinEntry,
                                MaxEntry>
{
  using parent_type = static_node_base_t<GeometryType,
                                         KeyType,
                                         MappedType,
                                         MinEntry,
                                         MaxEntry>;
  using node_base_type = parent_type;
  using node_type
      = static_node_t<GeometryType, KeyType, MappedType, MinEntry, MaxEntry>;
  using leaf_type = static_leaf_node_t;
  using size_type = typename parent_type::size_type;
  using geometry_type = GeometryType;
  using key_type = KeyType;
  using mapped_type = MappedType;
  using value_type = std::pair<key_type, mapped_type>;

  using iterator = value_type*;
  using const_iterator = value_type const*;

  static_vector<value_type, MaxEntry> _children;

  static_leaf_node_t() = default;
  static_leaf_node_t(static_leaf_node_t const&) = delete;
  static_leaf_node_t& operator=(static_leaf_node_t const&) = delete;
  static_leaf_node_t(static_leaf_node_t&&) = delete;
  static_leaf_node_t& operator=(static_leaf_node_t&&) = delete;

  // add child node with bounding box
  void insert(value_type child)
  {
    assert(size() < MaxEntry);
    _children.emplace_back(std::move(child));
  }
  void erase(value_type* pos)
  {
    assert(size() > 0);
    assert(std::distance(data(), pos) < size());
    if (std::distance(data(), pos) < size() - 1)
    {
      *pos = std::move(back());
    }
    pop_back();
  }

  void clear()
  {
    _children.clear();
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
    _children.pop_back();
  }

  // child count
  size_type size() const
  {
    return _children.size();
  }
  bool empty() const
  {
    return _children.empty();
  }
  // child iterator
  iterator begin()
  {
    return _children.begin();
  }
  // child iterator
  const_iterator begin() const
  {
    return _children.begin();
  }
  // child iterator
  iterator end()
  {
    return _children.end();
  }
  // child iterator
  const_iterator end() const
  {
    return _children.end();
  }

  value_type& at(size_type i)
  {
    assert(i >= 0);
    assert(i < size());
    return _children.at(i);
  }
  value_type const& at(size_type i) const
  {
    assert(i >= 0);
    assert(i < size());
    return _children.at(i);
  }
  value_type& operator[](size_type i)
  {
    return at(i);
  }
  value_type const& operator[](size_type i) const
  {
    return at(i);
  }
  value_type& front()
  {
    assert(size() > 0);
    return at(0);
  }
  value_type const& front() const
  {
    assert(size() > 0);
    return at(0);
  }
  value_type& back()
  {
    assert(size() > 0);
    return at(size() - 1);
  }
  value_type const& back() const
  {
    assert(size() > 0);
    return at(size() - 1);
  }
  value_type* data()
  {
    return _children.data();
  }
  value_type const* data() const
  {
    return _children.data();
  }

  // union bouinding box of children
  geometry_type calculate_bound() const
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
  template <typename TreeType>
  void delete_recursive(TreeType& tree)
  {
  }
  template <typename TreeType>
  leaf_type* clone_recursive(TreeType& tree) const
  {
    leaf_type* new_node = tree.template construct_node<leaf_type>();
    for (auto& c : *this)
    {
      new_node->insert(c);
    }
    return new_node;
  }
  size_type size_recursive() const
  {
    return size();
  }

  leaf_type* next()
  {
    return parent_type::next()->as_leaf();
  }
  leaf_type const* next() const
  {
    return parent_type::next()->as_leaf();
  }
  leaf_type* prev()
  {
    return parent_type::prev()->as_leaf();
  }
  leaf_type const* prev() const
  {
    return parent_type::prev()->as_leaf();
  }
};

}
} // namespace eh rtree