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
  using scalar_type = T;

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
  static void set_min_point(rect_t& bound, int axis, T value)
  {
    bound.min_[axis] = value;
  }
  static auto set_max_point(rect_t& bound, int axis, T value)
  {
    bound.max_[axis] = value;
  }

  // ===================== MUST IMPLEMENT =====================
};

// implement geometry_traits for my_rect_aabb
template <typename T, unsigned int Size>
struct geometry_traits<Eigen::Vector<T, Size>>
{
  // ===================== MUST IMPLEMENT =====================
  constexpr static int DIM = Size;
  using scalar_type = T;

  // get scalar value of min_point in axis
  static auto min_point(Eigen::Vector<T, Size> const& bound, int axis)
  {
    return bound[axis];
  }
  // get scalar value of max_point in axis
  static auto max_point(Eigen::Vector<T, Size> const& bound, int axis)
  {
    return bound[axis];
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

  /*
  rtree.search_inside(
      rect_t { vec_t { 0.1, 0.1, 0.1 }, vec_t { 1.1, 1.1, 1.1 } },
      [](rtree_t::value_type val)
      {
        std::cout << "Value Inside [0.1,0.1,0.1]x[1.1,1.1,1.1]: " << val.second
                  << "\n";
        // continue search
        return false;
      });
      */

  return 0;
}
