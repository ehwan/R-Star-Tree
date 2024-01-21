#pragma once

#include <limits>
#include <utility>

#include "global.hpp"

namespace eh { namespace rtree {

// split nodes 'as-is'
template < typename TreeType >
struct sequence_split_t
{
  using geometry_type = typename TreeType::geometry_type;
  using area_type = typename geometry_type::area_type;
  constexpr static area_type LOWEST_AREA = std::numeric_limits<area_type>::lowest();

  constexpr static unsigned int MIN_ENTRIES = TreeType::MIN_ENTRIES;
  constexpr static unsigned int MAX_ENTRIES = TreeType::MAX_ENTRIES;

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