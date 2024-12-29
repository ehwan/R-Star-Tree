#pragma once

#include <algorithm>

#include "geometry_traits.hpp"

namespace eh
{
namespace rtree
{

template <typename ScalarType, unsigned int Dim>
class point_t
{
public:
  using size_type = unsigned int;
  using scalar_type = ScalarType;
  using value_type = ScalarType;

protected:
  scalar_type _data[Dim];

public:
  point_t()
  {
  }
  point_t(point_t const& rhs)
  {
    for (size_type i = 0; i < size(); ++i)
    {
      _data[i] = rhs._data[i];
    }
  }
  template <typename T0, typename... Ts>
  point_t(T0 arg0, Ts... args)
  {
    static_assert(sizeof...(args) + 1 == Dim,
                  "Constructor Dimension not match");
    set<0>(arg0, args...);
  }
  template <size_type I = 0, typename T0, typename... Ts>
  void set(T0 arg0, Ts... args)
  {
    _data[I] = arg0;
    set<I + 1>(args...);
  }
  template <size_type I>
  void set()
  {
  }
  template <typename Iterator>
  void assign(Iterator begin, Iterator end)
  {
    size_type i = 0;
    while (begin != end && i < Dim)
    {
      _data[i++] = *begin++;
    }
  }
  point_t& operator=(point_t const& rhs)
  {
    for (size_type i = 0; i < size(); ++i)
    {
      _data[i] = rhs._data[i];
    }
    return *this;
  }
  constexpr static size_type size()
  {
    return Dim;
  }
  scalar_type& operator[](size_type i)
  {
    return _data[i];
  }
  scalar_type operator[](size_type i) const
  {
    return _data[i];
  }
  scalar_type* data()
  {
    return _data;
  }
  scalar_type const* data() const
  {
    return _data;
  }
  scalar_type* begin()
  {
    return _data;
  }
  scalar_type const* begin() const
  {
    return _data;
  }
  scalar_type* end()
  {
    return _data + size();
  }
  scalar_type const* end() const
  {
    return _data + size();
  }
};

// bounding box representation
template <typename PointType>
struct aabb_t
{
  PointType min_, max_;

  aabb_t(PointType const& p)
      : min_(p)
      , max_(p)
  {
  }
  aabb_t(PointType const& min__, PointType const& max__)
      : min_(min__)
      , max_(max__)
  {
  }
};

template <typename ArithmeticType>
struct geometry_traits<aabb_t<ArithmeticType>>
{
  using scalar_type = ArithmeticType;

  using AABB = aabb_t<ArithmeticType>;

  template <typename PointType>
  static bool is_inside(AABB const& aabb, PointType const& p)
  {
    return aabb.min_ <= p && p <= aabb.max_;
  }
  template <typename PointType>
  static bool is_inside(AABB const& aabb, aabb_t<PointType> const& aabb2)
  {
    return aabb.min_ <= aabb2.min_ && aabb2.max_ <= aabb.max_;
  }

  template <typename PointType>
  static bool is_overlap(AABB const& aabb, PointType const& p)
  {
    return is_inside(aabb, p);
  }
  template <typename PointType>
  static bool is_overlap(AABB const& aabb, aabb_t<PointType> const& aabb2)
  {
    if (aabb2.min_ > aabb.max_)
    {
      return false;
    }
    if (aabb.min_ > aabb2.max_)
    {
      return false;
    }
    return true;
  }

  static AABB merge(AABB const& aabb, ArithmeticType p)
  {
    return { std::min(aabb.min_, p), std::max(aabb.max_, p) };
  }
  static AABB merge(AABB const& aabb, AABB const& aabb2)
  {
    return { std::min(aabb.min_, aabb2.min_), std::max(aabb.max_, aabb2.max_) };
  }

  static scalar_type area(AABB const& aabb)
  {
    return aabb.max_ - aabb.min_;
  }

  // ==================== for R-star-Tree ====================
  // dimension
  constexpr static int DIM = 1;

  static auto min_point(AABB const& bound, int axis)
  {
    return bound.min_;
  }
  static auto max_point(AABB const& bound, int axis)
  {
    return bound.max_;
  }
  static void set_min_point(AABB& bound, int axis, ArithmeticType value)
  {
    bound.min_ = value;
  }
  static void set_max_point(AABB& bound, int axis, ArithmeticType value)
  {
    bound.max_ = value;
  }
};

// traits for multi-dimension point
template <typename T, unsigned int Dim>
struct geometry_traits<aabb_t<point_t<T, Dim>>>
{
  using scalar_type = T;

  using Point = point_t<T, Dim>;
  using AABB = aabb_t<Point>;

  template <typename PointType>
  static bool is_inside(AABB const& aabb, PointType const& p)
  {
    return less_equal(aabb.min_, p) && less_equal(p, aabb.max_);
  }
  template <typename PointType>
  static bool is_inside(AABB const& aabb, aabb_t<PointType> const& aabb2)
  {
    return less_equal(aabb.min_, aabb2.min_)
           && less_equal(aabb.max_, aabb.max_);
    return aabb.min_ <= aabb2.min_ && aabb2.max_ <= aabb.max_;
  }

  template <typename PointType>
  static bool is_overlap(AABB const& aabb, PointType const& p)
  {
    return is_inside(aabb, p);
  }
  template <typename PointType>
  static bool is_overlap(AABB const& aabb, aabb_t<PointType> const& aabb2)
  {
    if (!less_than(aabb2.min_, aabb.max_))
    {
      return false;
    }
    if (!less_than(aabb.min_, aabb2.max_))
    {
      return false;
    }
    return true;
  }

  static bool less_than(Point const& lhs, Point const& rhs)
  {
    for (unsigned int i = 0; i < Dim; ++i)
    {
      if (lhs[i] >= rhs[i])
      {
        return false;
      }
    }
    return true;
  }
  static bool less_equal(Point const& lhs, Point const& rhs)
  {
    for (unsigned int i = 0; i < Dim; ++i)
    {
      if (lhs[i] > rhs[i])
      {
        return false;
      }
    }
    return true;
  }
  static Point min(Point const& lhs, Point const& rhs)
  {
    Point ret;
    for (unsigned int i = 0; i < Dim; ++i)
    {
      ret[i] = std::min(lhs[i], rhs[i]);
    }
    return ret;
  }

  static Point max(Point const& lhs, Point const& rhs)
  {
    Point ret;
    for (unsigned int i = 0; i < Dim; ++i)
    {
      ret[i] = std::max(lhs[i], rhs[i]);
    }
    return ret;
  }

  // ==================== for R-star-Tree ====================
  // dimension
  constexpr static int DIM = Dim;

  static auto min_point(AABB const& bound, int axis)
  {
    return bound.min_[axis];
  }
  static auto max_point(AABB const& bound, int axis)
  {
    return bound.max_[axis];
  }
  static void set_min_point(AABB& bound, int axis, T value)
  {
    bound.min_[axis] = value;
  }
  static void set_max_point(AABB& bound, int axis, T value)
  {
    bound.max_[axis] = value;
  }
};

template <typename T, unsigned int Dim>
struct geometry_traits<point_t<T, Dim>>
{
  using scalar_type = T;
  constexpr static int DIM = Dim;

  static auto min_point(point_t<T, Dim> const& bound, int axis)
  {
    return bound[axis];
  }
  static auto max_point(point_t<T, Dim> const& bound, int axis)
  {
    return bound[axis];
  }
};

}
} // namespace eh, rtree