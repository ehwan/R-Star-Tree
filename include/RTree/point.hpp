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

}
} // namespace eh, rtree