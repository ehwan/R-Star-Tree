#pragma once

#include <iterator>
#include <type_traits>
#include "global.hpp"

namespace eh { namespace rtree {

// iterates through (only) inserted values
template < typename NodeType >
class level_node_iterator_t
{
  using this_type = level_node_iterator_t;
public:
  using value_type = typename NodeType::value_type;
  using difference_type = typename NodeType::size_type;
  using reference = std::conditional_t<
    std::is_const_v<NodeType>,
    value_type const&,
    value_type&
  >;
  using iterator_category = std::bidirectional_iterator_tag;
  using pointer = std::conditional_t<
    std::is_const_v<NodeType>,
    value_type const*,
    value_type*
  >;

protected:
  NodeType *_node;

public:
  level_node_iterator_t()
    : _node(nullptr)
  {
  }
  level_node_iterator_t( NodeType *node )
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

  reference operator*() const
  {
    return _node->data();
  }
};

}}