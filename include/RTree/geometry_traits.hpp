#pragma once

#include "global.hpp"

namespace eh
{
namespace rtree
{

// default traits
// for custom bound type, must implement functions below
template <typename GeometryType>
struct geometry_traits
{
  // ===================== MUST IMPLEMENT =====================
  // dimension
  constexpr static int DIM = 3;

  using area_type = typename GeometryType::area_type;

  template <typename PointOrBoundType>
  static bool is_inside(GeometryType const& bound, PointOrBoundType const& p)
  {
    return bound.is_inside(p);
  }

  template <typename PointOrBoundType>
  static bool is_overlap(GeometryType const& bound, PointOrBoundType const& p)
  {
    return bound.is_overlap(p);
  }

  template <typename PointOrBoundType>
  static GeometryType merge(GeometryType& bound, PointOrBoundType const& p)
  {
    return bound.merged(p);
  }
  static area_type area(GeometryType const& bound)
  {
    return bound.area();
  }

  // get scalar value of min_point in axis
  static auto min_point(GeometryType const& bound, int axis)
  {
    return bound.min_point(axis);
  }
  // get scalar value of max_point in axis
  static auto max_point(GeometryType const& bound, int axis)
  {
    return bound.max_point(axis);
  }
  // sum of all length of bound for all dimension
  static auto margin(GeometryType const& bound)
  {
    return bound.margin();
  }
  template <typename PointOrBoundType>
  static GeometryType intersection(GeometryType const& bound,
                                   PointOrBoundType const& p)
  {
    return bound.intersection(p);
  }

  // distance between center of bounds
  // used in reinserting
  // returned value is used to sort the reinserted nodes,
  // so no need to call sqrt() nor to be super-accurate
  static auto distance_center(GeometryType const& bound1,
                              GeometryType const& bound2)
  {
    return bound1.distance_center(bound2);
  }
  // ===================== MUST IMPLEMENT =====================
};

}
} // namespace eh rtree