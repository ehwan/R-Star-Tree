#pragma once

#include <limits>
#include <utility>

#include "global.hpp"

namespace eh { namespace rtree {

// split nodes 'as-is'
template < typename BoundType >
struct sequence_split_t
{
  using bound_type = BoundType;
  using area_type = typename bound_type::area_type;
  constexpr static area_type LOWEST_AREA = std::numeric_limits<area_type>::lowest();

  unsigned int MIN_ENTRIES = 4;
  unsigned int MAX_ENTRIES = 8;

  template < typename NodeType >
  NodeType *operator()( NodeType *node ) const
  {
    NodeType *new_node = new NodeType;
    for( auto i=node->end()-MIN_ENTRIES; i!=node->end(); ++i )
    {
      new_node->insert( std::move(*i) );
    }
    node->_child.erase( node->end()-MIN_ENTRIES, node->end() );

    return new_node;
  }
};

}} // namespace eh rtree