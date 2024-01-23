#pragma once

#include <utility>

#include "global.hpp"

namespace eh { namespace rtree {

// split nodes 'as-is'
template < typename TreeType >
struct sequence_split_t
{
  constexpr static unsigned int MIN_ENTRIES = TreeType::MIN_ENTRIES;
  constexpr static unsigned int MAX_ENTRIES = TreeType::MAX_ENTRIES;

  template < typename NodeType >
  NodeType *operator()( NodeType *node, typename NodeType::value_type child, NodeType *new_node ) const
  {
    assert( node->size() == MAX_ENTRIES );
    assert( new_node );
    assert( new_node->size() == 0 );
    for( auto i=node->end()-MIN_ENTRIES; i!=node->end(); ++i )
    {
      new_node->insert( std::move(*i) );
    }
    node->_child.erase( node->end()-MIN_ENTRIES, node->end() );
    node->insert( std::move(child) );

    return new_node;
  }
};

}} // namespace eh rtree