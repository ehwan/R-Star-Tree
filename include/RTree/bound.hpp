#pragma once

#include "RTree/point.hpp"
#include <limits>
#include <algorithm>

namespace eh { namespace rtree {

// bounding box representation
template < typename PointType >
class bound_t;

// arithmetic type specialization for one-dimension
template < typename PointType >
class bound_t
{
public:
  using point_type = PointType;
  using area_type = point_type;

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
    return ( max_bound() - min_bound() );
  }
  template < typename _PointType >
  bool is_inside( _PointType const& p ) const
  {
    return (min_bound() <= p) && (p < max_bound());
  }
  template < typename _PointType >
  bool is_inside( bound_t<_PointType> const& b ) const
  {
    return (min_bound() <= b.min_bound()) && (b.max_bound() <= max_bound());
  }
  template < typename _PointType >
  bool is_overlap( _PointType const& p ) const
  {
    return is_inside( p );
  }
  template < typename _PointType >
  bool is_overlap( bound_t<_PointType> const& b ) const
  {
    if( !(b.min_bound() < max_bound()) ){ return false; }
    if( !(min_bound() < b.max_bound()) ){ return false; }
    return true;
  }

  bound_t merged( bound_t const& b ) const
  {
    return { std::min(_min,b.min_bound()), std::max(_max,b.max_bound()) };
  }

  bound_t intersection( bound_t const& b ) const
  {
    const auto ret_min = std::max( min_bound(), b.min_bound() );
    return { ret_min, std::max( ret_min, std::min(max_bound(),b.max_bound()) ) };
  }
};

template < typename ScalarType, unsigned int Dim >
class bound_t< point_t<ScalarType,Dim> >
{
public:
  using point_type = point_t<ScalarType,Dim>;
  using area_type = ScalarType;

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
    area_type ret = 1;
    for( typename point_type::size_type i=0; i<_min.size(); ++i )
    {
      ret *= (_max[i] - _min[i]);
    }
    return ret;
  }
  template < typename _PointType >
  bool is_inside( _PointType const& p ) const
  {
    return (min_bound() <= p) && (p < max_bound());
  }
  template < typename _PointType >
  bool is_inside( bound_t<_PointType> const& b ) const
  {
    return (min_bound() <= b.min_bound()) && (b.max_bound() <= max_bound());
  }
  template < typename _PointType >
  bool is_overlap( _PointType const& p ) const
  {
    return is_inside( p );
  }
  template < typename _PointType >
  bool is_overlap( bound_t<_PointType> const& b ) const
  {
    if( !(b.min_bound() < max_bound()) ){ return false; }
    if( !(min_bound() < b.max_bound()) ){ return false; }
    return true;
  }

  bound_t merged( bound_t const& b ) const
  {
    return { eh::rtree::min(_min,b.min_bound()), eh::rtree::max(_max,b.max_bound()) };
  }

  bound_t intersection( bound_t const& b ) const
  {
    const auto ret_min = eh::rtree::max( min_bound(), b.min_bound() );
    return { ret_min, eh::rtree::max( ret_min, eh::rtree::min(max_bound(),b.max_bound()) ) };
  }
};

}} // namespace eh, rtree