#pragma once

#include <iterator>
#include <type_traits>
#include "global.hpp"

namespace eh { namespace rtree {

// iterates through inserted key-value pairs
template < typename LeafType >
class iterator_t
{
  template < typename _BoundType, typename _KeyType, typename _MappedType >
  friend class RTree;

  using this_type = iterator_t<LeafType>;
  using child_iterator = std::conditional_t<
    std::is_const_v<LeafType>,
    typename LeafType::const_iterator,
    typename LeafType::iterator
  >;

public:
  using value_type = typename child_iterator::value_type;
  using difference_type = std::make_signed_t<typename LeafType::size_type>;

  using reference = typename child_iterator::reference;
  using pointer = typename child_iterator::pointer;

  using iterator_category = std::bidirectional_iterator_tag;

protected:
  pointer _pointer;
  LeafType *_leaf;

public:
  iterator_t()
    : _pointer(nullptr),
      _leaf(nullptr)
  {
  }
  iterator_t( pointer __pointer, LeafType *__leaf )
    : _pointer(__pointer),
      _leaf(__leaf)
  {
  }

  bool operator==( this_type const& rhs ) const
  {
    return _pointer==rhs._pointer;
  }
  bool operator!=( this_type const& rhs ) const
  {
    return _pointer!=rhs._pointer;
  }

  LeafType* node() const
  {
    return _leaf;
  }

  this_type& operator++()
  {
    if( _pointer == &_leaf->_child.back() )
    {
      _leaf = reinterpret_cast<LeafType*>(_leaf->next());
      _pointer = _leaf ? &_leaf->_child.front() : nullptr;
    }else {
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
    if( _pointer == &_leaf->_child.front() )
    {
      _leaf = reinterpret_cast<LeafType*>(_leaf->prev());
      _pointer = _leaf ? &_leaf->_child.back() : nullptr;
    }else {
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
template < typename NodeType >
class node_iterator_t
{
  using this_type = node_iterator_t<NodeType>;

public:
  using value_type = NodeType*;
  using difference_type = std::make_signed_t<typename NodeType::size_type>;

  using reference = value_type&;
  using pointer = NodeType*;

  using iterator_category = std::bidirectional_iterator_tag;

protected:
  NodeType *_node;

public:
  node_iterator_t()
    : _node(nullptr)
  {
  }
  node_iterator_t( NodeType *node )
    : _node(node)
  {
  }
  bool operator==( this_type const& rhs ) const
  {
    return (_node==rhs._node);
  }
  bool operator!=( this_type const& rhs ) const
  {
    return (_node!=rhs._node);
  }

  NodeType* node() const
  {
    return _node;
  }

  this_type& operator++()
  {
    _node = reinterpret_cast<NodeType*>(_node->next());
    return *this;
  }
  this_type operator++(int)
  {
    this_type ret = *this;
    _node = reinterpret_cast<NodeType*>(_node->next());
    return ret;
  }
  this_type& operator--()
  {
    _node = reinterpret_cast<NodeType*>(_node->prev());
    return *this;
  }
  this_type operator--(int)
  {
    this_type ret = *this;
    _node = reinterpret_cast<NodeType*>(_node->prev());
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

}}