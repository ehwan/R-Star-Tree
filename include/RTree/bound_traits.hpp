#pragma once

#include "global.hpp"

namespace eh { namespace rtree {

// default traits
// for custom bound type, must implement functions below
template < typename BoundType >
struct bound_traits
{
  using bound_type = BoundType;
  using area_type = typename BoundType::area_type;

  template < typename PointOrBoundType >
  static bool is_inside( bound_type const& bound, PointOrBoundType const& p )
  {
    return bound.is_inside( p );
  }

  template < typename PointOrBoundType >
  static bool is_overlap( bound_type const& bound, PointOrBoundType const& p )
  {
    return bound.is_overlap( p );
  }

  template < typename PointOrBoundType >
  static bound_type merge( bound_type& bound, PointOrBoundType const& p )
  {
    return bound.merged(p);
  }
  static area_type area( bound_type const& bound )
  {
    return bound.area();
  }

  // optional; used in quadratic split resolving conflict
  template < typename PointOrBoundType >
  static bound_type intersection( bound_type const& bound, PointOrBoundType const& p )
  {
    return bound.intersection( p );
  }
};

}} // namespace eh rtree