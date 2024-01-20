#pragma once

namespace eh { namespace rtree {

template < typename PointType >
struct point_traits;

template < typename BoundType >
struct bound_traits;

template < typename BoundType, typename KeyType, typename MappedType >
class node_base_t;
template < typename BoundType, typename KeyType, typename MappedType >
class node_t;
template < typename BoundType, typename KeyType, typename MappedType >
class leaf_node_t;

template < typename LeafType >
class iterator_t;

template < typename NodeType >
class node_iterator_t;

template < typename BoundType, typename KeyType, typename MappedType >
class RTree;

}}