#pragma once

#include <algorithm>

#include "global.hpp"

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
  EH_RTREE_DEVICE_HOST point_t()
  {
  }
  EH_RTREE_DEVICE_HOST point_t(point_t const& rhs)
  {
    for (size_type i = 0; i < size(); ++i)
    {
      _data[i] = rhs._data[i];
    }
  }
  template <typename T0, typename... Ts>
  EH_RTREE_DEVICE_HOST point_t(T0 arg0, Ts... args)
  {
    static_assert(sizeof...(args) + 1 == Dim,
                  "Constructor Dimension not match");
    set<0>(arg0, args...);
  }
  template <size_type I = 0, typename T0, typename... Ts>
  EH_RTREE_DEVICE_HOST void set(T0 arg0, Ts... args)
  {
    _data[I] = arg0;
    set<I + 1>(args...);
  }
  template <size_type I>
  EH_RTREE_DEVICE_HOST void set()
  {
  }
  template <typename Iterator>
  EH_RTREE_DEVICE_HOST void assign(Iterator begin, Iterator end)
  {
    size_type i = 0;
    while (begin != end && i < Dim)
    {
      _data[i++] = *begin++;
    }
  }
  EH_RTREE_DEVICE_HOST point_t& operator=(point_t const& rhs)
  {
    for (size_type i = 0; i < size(); ++i)
    {
      _data[i] = rhs._data[i];
    }
    return *this;
  }
  EH_RTREE_DEVICE_HOST constexpr static size_type size()
  {
    return Dim;
  }
  EH_RTREE_DEVICE_HOST scalar_type& operator[](size_type i)
  {
    return _data[i];
  }
  EH_RTREE_DEVICE_HOST scalar_type operator[](size_type i) const
  {
    return _data[i];
  }
  EH_RTREE_DEVICE_HOST scalar_type* data()
  {
    return _data;
  }
  EH_RTREE_DEVICE_HOST scalar_type const* data() const
  {
    return _data;
  }
  EH_RTREE_DEVICE_HOST scalar_type* begin()
  {
    return _data;
  }
  EH_RTREE_DEVICE_HOST scalar_type const* begin() const
  {
    return _data;
  }
  EH_RTREE_DEVICE_HOST scalar_type* end()
  {
    return _data + size();
  }
  EH_RTREE_DEVICE_HOST scalar_type const* end() const
  {
    return _data + size();
  }
};

}
} // namespace eh, rtree