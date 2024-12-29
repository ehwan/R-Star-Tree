#pragma once

#include "RTree/global.hpp"
#include <algorithm>

namespace eh
{
namespace rtree
{

// default traits
// for custom bound type, must implement functions below
template <typename GeometryType>
struct geometry_traits
{
  // dimension
  constexpr static int DIM = 1;

  using scalar_type = GeometryType;

  // get scalar value of min_point in axis
  static scalar_type min_point(GeometryType const& g, int axis)
  {
    return g;
  }
  // get scalar value of max_point in axis
  static scalar_type max_point(GeometryType const& g, int axis)
  {
    return g;
  }
  // set scalar value of min_point in axis, only for geometry_type
  static void set_min_point(GeometryType& g, int axis, scalar_type value)
  {
    g = value;
  }
  // set scalar value of max_point in axis, only for geometry_type
  static void set_max_point(GeometryType& g, int axis, scalar_type value)
  {
    g = value;
  }
};

namespace helper
{
template <typename GeometryType>
typename geometry_traits<GeometryType>::scalar_type
min_point(GeometryType const& g, int axis)
{
  EH_RTREE_ASSERT_SILENT(axis >= 0
                         && axis < geometry_traits<GeometryType>::DIM);
  return geometry_traits<GeometryType>::min_point(g, axis);
}
template <typename GeometryType>
typename geometry_traits<GeometryType>::scalar_type
max_point(GeometryType const& g, int axis)
{
  EH_RTREE_ASSERT_SILENT(axis >= 0
                         && axis < geometry_traits<GeometryType>::DIM);
  return geometry_traits<GeometryType>::max_point(g, axis);
}
template <typename GeometryType>
void set_min_point(
    GeometryType& g,
    int axis,
    typename geometry_traits<GeometryType>::scalar_type const& value)
{
  EH_RTREE_ASSERT_SILENT(axis >= 0
                         && axis < geometry_traits<GeometryType>::DIM);
  return geometry_traits<GeometryType>::set_min_point(g, axis, value);
}
template <typename GeometryType>
void set_max_point(
    GeometryType& g,
    int axis,
    typename geometry_traits<GeometryType>::scalar_type const& value)
{
  EH_RTREE_ASSERT_SILENT(axis >= 0
                         && axis < geometry_traits<GeometryType>::DIM);
  return geometry_traits<GeometryType>::set_max_point(g, axis, value);
}

// enlarge g1 to cover both g1 and g2
template <typename Geom1, typename Geom2>
void enlarge_to(Geom1& g1, Geom2 const& g2)
{
  static_assert(geometry_traits<Geom1>::DIM == geometry_traits<Geom2>::DIM,
                "Dimension not match");
  for (int i = 0; i < geometry_traits<Geom1>::DIM; ++i)
  {
    set_min_point(g1, i, std::min(min_point(g1, i), min_point(g2, i)));
    set_max_point(g1, i, std::max(max_point(g1, i), max_point(g2, i)));
  }
}

// area of enlarged bound
template <typename Geom1, typename Geom2>
typename geometry_traits<Geom1>::scalar_type enlarged_area(Geom1 const& g1,
                                                           Geom2 const& g2)
{
  static_assert(geometry_traits<Geom1>::DIM == geometry_traits<Geom2>::DIM,
                "Dimension not match");

  typename geometry_traits<Geom1>::scalar_type ret = 1;
  for (int i = 0; i < geometry_traits<Geom1>::DIM; ++i)
  {
    ret *= std::max(max_point(g1, i), max_point(g2, i))
           - std::min(min_point(g1, i), min_point(g2, i));
  }
  return ret;
}
// area of intersection between two bounds
template <typename Geom1, typename Geom2>
typename geometry_traits<Geom1>::scalar_type intersection_area(Geom1 const& g1,
                                                               Geom2 const& g2)
{
  static_assert(geometry_traits<Geom1>::DIM == geometry_traits<Geom2>::DIM,
                "Dimension not match");
  typename geometry_traits<Geom1>::scalar_type ret = 1;
  for (int i = 0; i < geometry_traits<Geom1>::DIM; ++i)
  {
    if (min_point(g1, i) > max_point(g2, i)
        || max_point(g1, i) < min_point(g2, i))
    {
      return 0;
    }

    ret *= std::min(max_point(g1, i), max_point(g2, i))
           - std::max(min_point(g1, i), min_point(g2, i));
  }
  return ret;
}

// area of bound
template <typename GeometryType>
typename geometry_traits<GeometryType>::scalar_type area(GeometryType const& g)
{
  typename geometry_traits<GeometryType>::scalar_type ret = 1;
  for (int i = 0; i < geometry_traits<GeometryType>::DIM; ++i)
  {
    ret *= max_point(g, i) - min_point(g, i);
  }
  return ret;
}
// sum of all length of bound for all dimension
template <typename GeometryType>
typename geometry_traits<GeometryType>::scalar_type
margin(GeometryType const& g)
{
  typename geometry_traits<GeometryType>::scalar_type ret = 0;
  for (int i = 0; i < geometry_traits<GeometryType>::DIM; ++i)
  {
    ret += max_point(g, i) - min_point(g, i);
  }
  return ret;
}

// distance between center of bounds
// used in reinserting
// returned value is used to sort the reinserted nodes,
// so no need to call sqrt() nor to be super-accurate
template <typename Geom1, typename Geom2>
typename geometry_traits<Geom1>::scalar_type distance_center(Geom1 const& g1,
                                                             Geom2 const& g2)
{
  static_assert(geometry_traits<Geom1>::DIM == geometry_traits<Geom2>::DIM,
                "Dimension not match");
  typename geometry_traits<Geom1>::scalar_type ret = 0;
  for (int i = 0; i < geometry_traits<Geom1>::DIM; ++i)
  {
    typename geometry_traits<Geom1>::scalar_type diff
        = (max_point(g1, i) + min_point(g1, i))
          - (max_point(g2, i) + min_point(g2, i));
    ret += diff * diff;
  }
  return ret;
}

}

}
} // namespace eh rtree