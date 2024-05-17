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
  // ==================== MUST implement ====================
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
  // ==================== MUST implement ====================

  // ==================== for R-star-Tree ====================
  // dimension
  constexpr static int DIM = 3;

  static auto min_point(GeometryType const& bound, int axis)
  {
    return bound.min_point(axis);
  }
  static auto max_point(GeometryType const& bound, int axis)
  {
    return bound.max_point(axis);
  }
  // sum of all length of bound for all dimension
  static auto margin(GeometryType const& bound)
  {
    return bound.margin();
  }
  // also used in quadratic split resolving conflict [optional]
  template <typename PointOrBoundType>
  static GeometryType intersection(GeometryType const& bound,
                                   PointOrBoundType const& p)
  {
    return bound.intersection(p);
  }

  // ==================== for R-star-Tree ====================
};

}
} // namespace eh rtree