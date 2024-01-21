#include "RTree/rtree.hpp"
#include <RTree.hpp>
#include <Eigen/Core>
#include <iostream>

// use custom vector type in RTree class

// link Eigen3::Vector type to geometry_traits
template < typename T, unsigned int Size >
struct my_rect_aabb
{
  using vec_t = Eigen::Vector<T,Size>;

  vec_t min_, max_;

  // point to rect conversion
  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  my_rect_aabb( vec_t const& point )
    : min_(point),
      max_(point)
  {
  }
  my_rect_aabb( vec_t const& min__, vec_t const& max__ )
    : min_(min__),
      max_(max__)
  {
  }
};

namespace eh { namespace rtree {

template < typename T, unsigned int Size >
struct geometry_traits<my_rect_aabb<T,Size>>
{
  using rect_t = my_rect_aabb<T,Size>;
  using vec_t = typename rect_t::vec_t;

  // must define area_type as arithmetic_type
  // s.t., std::numeric_limits<area_type>::max(), lowest() is defined
  using area_type = T;

  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  static bool is_inside( rect_t const& rect, vec_t const& v )
  {
    return (rect.min_.array()<=v.array()).all() && (v.array()<=rect.max_.array()).all();
  }
  static bool is_inside( rect_t const& rect, rect_t const& rect2 )
  {
    return (rect.min_.array()<=rect2.min_.array()).all() && (rect2.max_.array()<=rect.max_.array()).all();
  }

  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  static bool is_overlap( rect_t const& rect, vec_t const& v )
  {
    return is_inside( rect, v );
  }
  static bool is_overlap( rect_t const& rect, rect_t const& rect2 )
  {
    if( (rect2.min_.array() > rect.max_.array()).any() ){ return false; }
    if( (rect.min_.array() > rect2.max_.array()).any() ){ return false; }
    return true;
  }

  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  static rect_t merge( rect_t const& rect, vec_t const& v )
  {
    return { 
      rect.min_.array().min(v.array()), 
      rect.max_.array().max(v.array())
    };
  }
  static rect_t merge( rect_t const& rect, rect_t const& rect2 )
  {
    return { 
      rect.min_.array().min(rect2.min_.array()), 
      rect.max_.array().max(rect2.max_.array())
    };
  }


  static area_type area( rect_t const& rect )
  {
    area_type ret = 1;
    for( unsigned int i=0; i<Size; ++i )
    {
      ret *= ( rect.max_[i] - rect.min_[i] );
    }
    return ret;
  }

  // optional; used in quadratic split resolving conflict
  static rect_t intersection( rect_t const& rect1, rect_t const& rect2 )
  {
    const vec_t ret_min = rect1.min_.array().max( rect2.min_.array() );
    return { ret_min, rect1.max_.array().max(rect2.max_.array()).min( ret_min.array() ) };
  }
};

}}

int main()
{
  using vec_t = Eigen::Vector<double,3>;
  using rect_t = my_rect_aabb<double,3>;
  using rtree_t = eh::rtree::RTree< rect_t, vec_t, int >;
  rtree_t rtree;

  rtree.insert( {vec_t{0,0,0}, 0} );
  rtree.insert( {vec_t{0.5,0.5,0.5}, 1} );
  rtree.insert( {vec_t{0.7,0.7,0.7}, 2} );
  rtree.insert( {vec_t{0.7,1.3,0.7}, 3} );

  rtree.search_inside(
    rect_t{
      vec_t{0.1,0.1,0.1},
      vec_t{1.1,1.1,1.1}
    },
    []( rtree_t::value_type val )
    {
      std::cout << "Value Inside [0.1,0.1,0.1]x[1.1,1.1,1.1]: " << val.second << "\n";
      // continue search
      return false;
    }
  );

  return 0;
}