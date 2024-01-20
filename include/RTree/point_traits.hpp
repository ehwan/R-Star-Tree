#pragma once

namespace eh { namespace rtree {

// default traits
template < typename PointType >
struct point_traits
{
  using point_type = PointType;
  using area_type = PointType;

  template < typename Lhs, typename Rhs >
  static bool less_than( Lhs const& lhs, Rhs const& rhs )
  {
    return lhs < rhs;
  }
  template < typename Lhs, typename Rhs >
  static bool less_equal( Lhs const& lhs, Rhs const& rhs )
  {
    return lhs <= rhs;
  }
  template < typename Lhs, typename Rhs >
  static decltype(auto) min( Lhs const& lhs, Rhs const& rhs )
  {
    return lhs<rhs ? lhs : rhs;
  }
  template < typename Lhs, typename Rhs >
  static decltype(auto) max( Lhs const& lhs, Rhs const& rhs )
  {
    return lhs>rhs ? lhs : rhs;
  }

  template < typename Min_, typename Max_ >
  static area_type area( Min_ const& min_, Max_ const& max_ )
  {
    return max_ - min_;
  }
};

}} // namespace eh rtree