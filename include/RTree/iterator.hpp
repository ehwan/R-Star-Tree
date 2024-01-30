#pragma once

#include "global.hpp"
#include <iterator>
#include <type_traits>

namespace eh
{
namespace rtree
{

// iterates through inserted key-value pairs
template <typename LeafType>
struct iterator_t
{
  using this_type = iterator_t<LeafType>;
  using child_iterator = std::conditional_t<std::is_const<LeafType>::value,
                                            typename LeafType::const_iterator,
                                            typename LeafType::iterator>;

  using value_type = typename std::iterator_traits<child_iterator>::value_type;
  using difference_type = std::make_signed_t<typename LeafType::size_type>;

  using reference = typename std::iterator_traits<child_iterator>::reference;
  using pointer = typename std::iterator_traits<child_iterator>::pointer;

  using iterator_category = std::bidirectional_iterator_tag;

  pointer _pointer;
  LeafType* _leaf;

  iterator_t()
      : _pointer(nullptr)
      , _leaf(nullptr)
  {
  }
  iterator_t(pointer __pointer, LeafType* __leaf)
      : _pointer(__pointer)
      , _leaf(__leaf)
  {
  }

  bool operator==(this_type const& rhs) const
  {
    return _pointer == rhs._pointer;
  }
  bool operator!=(this_type const& rhs) const
  {
    return _pointer != rhs._pointer;
  }

  LeafType* node() const
  {
    return _leaf;
  }

  this_type& operator++()
  {
    if (_pointer == &_leaf->at(_leaf->size() - 1))
    {
      _leaf = _leaf->next();
      _pointer = _leaf ? &_leaf->at(0) : nullptr;
    }
    else
    {
      ++_pointer;
    }
    return *this;
  }
  this_type operator++(int)
  {
    this_type ret = *this;
    operator++();
    return ret;
  }
  this_type& operator--()
  {
    if (_pointer == &_leaf->at(0))
    {
      _leaf = _leaf->prev();
      _pointer = _leaf ? &_leaf->at(_leaf->size() - 1) : nullptr;
    }
    else
    {
      --_pointer;
    }
    return *this;
  }
  this_type operator--(int)
  {
    this_type ret = *this;
    operator--();
    return ret;
  }

  reference operator*() const
  {
    return *_pointer;
  }
  pointer operator->() const
  {
    return _pointer;
  }
};

// iterates through same-level nodes
template <typename NodeType>
struct node_iterator_t
{
  using this_type = node_iterator_t<NodeType>;

  using value_type = NodeType*;
  using difference_type = std::make_signed_t<typename NodeType::size_type>;

  using reference = value_type&;
  using pointer = NodeType*;

  using iterator_category = std::bidirectional_iterator_tag;

  NodeType* _node;

  node_iterator_t()
      : _node(nullptr)
  {
  }
  node_iterator_t(NodeType* node)
      : _node(node)
  {
  }
  bool operator==(this_type const& rhs) const
  {
    return (_node == rhs._node);
  }
  bool operator!=(this_type const& rhs) const
  {
    return (_node != rhs._node);
  }

  NodeType* node() const
  {
    return _node;
  }

  this_type& operator++()
  {
    _node = _node->next();
    return *this;
  }
  this_type operator++(int)
  {
    this_type ret = *this;
    _node = _node->next();
    return ret;
  }
  this_type& operator--()
  {
    _node = _node->prev();
    return *this;
  }
  this_type operator--(int)
  {
    this_type ret = *this;
    _node = _node->prev();
    return ret;
  }

  reference operator*()
  {
    return _node;
  }
  pointer operator->() const
  {
    return _node;
  }
};

}
}