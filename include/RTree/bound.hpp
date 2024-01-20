#pragma once

#include "global.hpp"

namespace eh { namespace rtree {

// bounding box representation
template < typename PointType >
class bound_t
{
public:
  using point_type = PointType;
  using traits = point_traits<PointType>;
  using area_type = typename traits::area_type;

protected:
  point_type _min, _max;

public:
  bound_t( point_type const& __min, point_type const& __max )
    : _min(__min),
      _max(__max)
  {
  }


  inline auto const& min_bound() const
  {
    return _min;
  }
  inline auto const& max_bound() const
  {
    return _max;
  }

  // area of this bounding box
  area_type area() const
  {
    return traits::area( min_bound(), max_bound() );
  }

  // @TODO 
  // inclusive or exclusive max_bound
  template < typename _PointType >
  bool is_inside( _PointType const& p ) const
  {
    return traits::less_equal( min_bound(), p ) && traits::less_equal( p, max_bound() );
  }
  template < typename _PointType >
  bool is_inside( bound_t<_PointType> const& b ) const
  {
    return traits::less_equal(min_bound(), b.min_bound()) && traits::less_equal(b.max_bound(), max_bound());
  }
  template < typename _PointType >
  bool is_overlap( _PointType const& p ) const
  {
    return is_inside( p );
  }

  // @TODO 
  // inclusive or exclusive max_bound
  template < typename _PointType >
  bool is_overlap( bound_t<_PointType> const& b ) const
  {
    if( !traits::less_equal(b.min_bound(), max_bound()) ){ return false; }
    if( !traits::less_equal(min_bound(), b.max_bound()) ){ return false; }
    return true;
  }

  bound_t merged( point_type const& p ) const
  {
    return { traits::min(p,min_bound()), traits::max(p,max_bound()) };
  }
  bound_t merged( bound_t const& b ) const
  {
    return { traits::min(min_bound(), b.min_bound()), traits::max(max_bound(), b.max_bound()) };
  }

  bound_t intersection( bound_t const& b ) const
  {
    const auto ret_min = traits::max( min_bound(), b.min_bound() );
    return { ret_min, traits::max( ret_min, traits::min(max_bound(),b.max_bound()) ) };
  }
};

}} // namespace eh, rtree