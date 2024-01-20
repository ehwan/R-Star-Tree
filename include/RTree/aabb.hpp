#pragma once

#include <algorithm>

#include "global.hpp"

namespace eh { namespace rtree {

// bounding box representation
template < typename PointType >
struct aabb_t
{
  PointType min_, max_;
};

template < typename ArithmeticType >
struct geometry_traits<aabb_t<ArithmeticType>>
{
  using area_type = ArithmeticType;

  using AABB = aabb_t<ArithmeticType>;

  template < typename PointType >
  static bool is_inside( AABB const& aabb, PointType const& p )
  {
    return aabb.min_ <= p && p <= aabb.max_;
  }
  template < typename PointType >
  static bool is_inside( AABB const& aabb, aabb_t<PointType> const& aabb2 )
  {
    return aabb.min_<=aabb2.min_ && aabb2.max_<=aabb.max_;
  }

  template < typename PointType >
  static bool is_overlap( AABB const& aabb, PointType const& p )
  {
    return is_inside(aabb,p);
  }
  template < typename PointType >
  static bool is_overlap( AABB const& aabb, aabb_t<PointType> const& aabb2 )
  {
    if( aabb2.min_>aabb.max_ ){ return false; }
    if( aabb.min_>aabb2.max_ ){ return false; }
    return true;
  }

  static AABB merge( AABB& aabb, ArithmeticType p )
  {
    return { std::min(aabb.min_,p), std::max(aabb.max_,p) };
  }
  static AABB merge( AABB& aabb, AABB const& aabb2 )
  {
    return { std::min(aabb.min_,aabb2.min_), std::max(aabb.max_,aabb2.max_) };
  }

  static area_type area( AABB const& aabb )
  {
    return aabb.max_ - aabb.min_;
  }

  // optional; used in quadratic split resolving conflict
  static AABB intersection( AABB const& aabb, AABB const& aabb2 )
  {
    const auto ret_min = std::min( aabb.min_, aabb2.min_ );
    return { ret_min, std::max( ret_min, std::min(aabb.max_,aabb2.max_) ) };
  }
};

}} // namespace eh, rtree