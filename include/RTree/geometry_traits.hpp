#pragma once

#include "global.hpp"

namespace eh { namespace rtree {

// default traits
// for custom bound type, must implement functions below
template < typename GeometryType >
struct geometry_traits
{
  using area_type = typename GeometryType::area_type;

  template < typename PointOrBoundType >
  static bool is_inside( GeometryType const& bound, PointOrBoundType const& p )
  {
    return bound.is_inside( p );
  }

  template < typename PointOrBoundType >
  static bool is_overlap( GeometryType const& bound, PointOrBoundType const& p )
  {
    return bound.is_overlap( p );
  }

  template < typename PointOrBoundType >
  static GeometryType merge( GeometryType& bound, PointOrBoundType const& p )
  {
    return bound.merged(p);
  }
  static area_type area( GeometryType const& bound )
  {
    return bound.area();
  }

  // optional; used in quadratic split resolving conflict
  template < typename PointOrBoundType >
  static GeometryType intersection( GeometryType const& bound, PointOrBoundType const& p )
  {
    return bound.intersection( p );
  }
};

}} // namespace eh rtree