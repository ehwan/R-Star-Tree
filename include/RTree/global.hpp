#pragma once

namespace eh { namespace rtree {

template < typename GeometryType >
struct geometry_traits;

template < typename TreeType >
struct node_base_t;
template < typename TreeType >
struct node_t;
template < typename TreeType >
struct leaf_node_t;

template < typename LeafType >
struct iterator_t;

template < typename NodeType >
struct node_iterator_t;

template < typename GeometryType, typename KeyType, typename MappedType, unsigned int MinEntry=4, unsigned int MaxEntry=8 >
class RTree;

}}
