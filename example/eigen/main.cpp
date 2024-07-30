#include "RTree/rtree.hpp"
#include <Eigen/Core>
#include <RTree.hpp>
#include <iostream>

// use custom vector type in RTree class
// link Eigen3::Vector type to geometry_traits

// define AABB type with Eigen::Vector
template <typename T, unsigned int Size>
struct my_rect_aabb
{
  using vec_t = Eigen::Vector<T, Size>;

  vec_t min_, max_;

  // point to rect implicit conversion
  // only if you use point
  // ( geometry object that does not have volume in N-dimension ) as key_type
  my_rect_aabb(vec_t const& point)
      : min_(point)
      , max_(point)
  {
  }
  my_rect_aabb(vec_t const& min__, vec_t const& max__)
      : min_(min__)
      , max_(max__)
  {
  }
};

namespace eh
{
namespace rtree
{

// implement geometry_traits for my_rect_aabb
template <typename T, unsigned int Size>
struct geometry_traits<my_rect_aabb<T, Size>>
{
  using rect_t = my_rect_aabb<T, Size>;
  using vec_t = typename rect_t::vec_t;

  // ===================== MUST IMPLEMENT =====================
  constexpr static int DIM = Size;

  // must define area_type as arithmetic_type
  // such that, std::numeric_limits<area_type>::max(), lowest() is defined
  using area_type = T;

  // only if you use point
  // ( geometry object that does not have volume in N-dimension ) as key_type
  static bool is_inside(rect_t const& rect, vec_t const& v)
  {
    return (rect.min_.array() <= v.array()).all()
           && (v.array() <= rect.max_.array()).all();
  }
  static bool is_inside(rect_t const& rect, rect_t const& rect2)
  {
    return (rect.min_.array() <= rect2.min_.array()).all()
           && (rect2.max_.array() <= rect.max_.array()).all();
  }

  // only if you use point
  // ( geometry object that does not have volume in N-dimension ) as key_type
  static bool is_overlap(rect_t const& rect, vec_t const& v)
  {
    return is_inside(rect, v);
  }
  static bool is_overlap(rect_t const& rect, rect_t const& rect2)
  {
    if ((rect2.min_.array() > rect.max_.array()).any())
    {
      return false;
    }
    if ((rect.min_.array() > rect2.max_.array()).any())
    {
      return false;
    }
    return true;
  }

  // only if you use point
  // ( geometry object that does not have volume in N-dimension ) as key_type
  static rect_t merge(rect_t const& rect, vec_t const& v)
  {
    return { rect.min_.array().min(v.array()),
             rect.max_.array().max(v.array()) };
  }
  static rect_t merge(rect_t const& rect, rect_t const& rect2)
  {
    return { rect.min_.array().min(rect2.min_.array()),
             rect.max_.array().max(rect2.max_.array()) };
  }

  static area_type area(rect_t const& rect)
  {
    area_type ret = 1;
    for (unsigned int i = 0; i < Size; ++i)
    {
      ret *= (rect.max_[i] - rect.min_[i]);
    }
    return ret;
  }

  // get scalar value of min_point in axis
  static auto min_point(rect_t const& bound, int axis)
  {
    return bound.min_[axis];
  }
  // get scalar value of max_point in axis
  static auto max_point(rect_t const& bound, int axis)
  {
    return bound.max_[axis];
  }
  // sum of all length of bound for all dimension
  static auto margin(rect_t const& bound)
  {
    area_type ret = 0;
    for (unsigned int i = 0; i < Size; ++i)
    {
      ret += bound.max_[i] - bound.min_[i];
    }
    return ret;
  }

  static rect_t intersection(rect_t const& rect1, rect_t const& rect2)
  {
    const vec_t ret_min = rect1.min_.array().max(rect2.min_.array());
    return { ret_min,
             rect1.max_.array().min(rect2.max_.array()).max(ret_min.array()) };
  }

  // distance between center of bounds
  // used in reinserting
  // returned value is used to sort the reinserted nodes,
  // so no need to call sqrt() nor to be super-accurate
  static auto distance_center(rect_t const& rect1, rect_t const& rect2)
  {
    return (rect1.min_ + rect1.max_ - rect2.min_ - rect2.max_).squaredNorm();
  }

  // ===================== MUST IMPLEMENT =====================
};

}
}

int main()
{
  using vec_t = Eigen::Vector<double, 3>;
  using rect_t = my_rect_aabb<double, 3>;
  // pass your AABB type (and vec_t for key_type) to template argument
  using rtree_t = eh::rtree::RTree<rect_t, vec_t, int>;
  //                                ^ AABB   ^ key  ^ data
  //                                         <------------> value_type
  rtree_t rtree;

  rtree.insert({ vec_t { 0, 0, 0 }, 0 });
  rtree.insert({ vec_t { 0.5, 0.5, 0.5 }, 1 });
  rtree.insert({ vec_t { 0.7, 0.7, 0.7 }, 2 });
  rtree.insert({ vec_t { 0.7, 1.3, 0.7 }, 3 });

  rtree.search_inside(
      rect_t { vec_t { 0.1, 0.1, 0.1 }, vec_t { 1.1, 1.1, 1.1 } },
      [](rtree_t::value_type val)
      {
        std::cout << "Value Inside [0.1,0.1,0.1]x[1.1,1.1,1.1]: " << val.second
                  << "\n";
        // continue search
        return false;
      });

  return 0;
}
