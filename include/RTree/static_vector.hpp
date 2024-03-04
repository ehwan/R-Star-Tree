#pragma once

#include "global.hpp"
#include <cassert>

namespace eh
{
namespace rtree
{

template <typename T, unsigned int N>
class static_vector
{
  static_assert(sizeof(char) == 1, "sizeof(char) != 1");

public:
  using size_type = unsigned int;
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;

protected:
  alignas(T) char _data[N * sizeof(T)];
  size_type _size;

public:
  EH_RTREE_DEVICE_HOST static_vector()
      : _size(0)
  {
  }
  EH_RTREE_DEVICE_HOST ~static_vector()
  {
    for (reference v : *this)
    {
      v.~value_type();
    }
    _size = 0;
  }
  static_vector(const static_vector& other) = delete;
  static_vector(static_vector&& other) = delete;
  static_vector& operator=(const static_vector& other) = delete;
  static_vector& operator=(static_vector&& other) = delete;

  EH_RTREE_DEVICE_HOST size_type size() const
  {
    return _size;
  }
  EH_RTREE_DEVICE_HOST bool empty() const
  {
    return _size == 0;
  }
  EH_RTREE_DEVICE_HOST pointer data()
  {
    return reinterpret_cast<pointer>(_data);
  }
  EH_RTREE_DEVICE_HOST const_pointer data() const
  {
    return reinterpret_cast<const_pointer>(_data);
  }
  EH_RTREE_DEVICE_HOST reference operator[](size_type index)
  {
    return data()[index];
  }
  EH_RTREE_DEVICE_HOST const_reference operator[](size_type index) const
  {
    return data()[index];
  }
  EH_RTREE_DEVICE_HOST reference front()
  {
    assert(_size > 0);
    return data()[0];
  }
  EH_RTREE_DEVICE_HOST const_reference front() const
  {
    assert(_size > 0);
    return data()[0];
  }
  EH_RTREE_DEVICE_HOST reference back()
  {
    assert(_size > 0);
    return data()[_size - 1];
  }
  EH_RTREE_DEVICE_HOST const_reference back() const
  {
    assert(_size > 0);
    return data()[_size - 1];
  }
  EH_RTREE_DEVICE_HOST void push_back(const value_type& value)
  {
    assert(_size < N);
    new (data() + _size) value_type(value);
    ++_size;
  }
  EH_RTREE_DEVICE_HOST void push_back(value_type&& value)
  {
    assert(_size < N);
    new (data() + _size) value_type(static_cast<value_type&&>(value));
    ++_size;
  }
  EH_RTREE_DEVICE_HOST void pop_back()
  {
    assert(_size > 0);
    data()[_size - 1].~value_type();
    --_size;
  }
  EH_RTREE_DEVICE_HOST void clear()
  {
    for (reference v : *this)
    {
      v.~value_type();
    }
    _size = 0;
  }
  EH_RTREE_DEVICE_HOST void resize(size_type new_size)
  {
    assert(new_size <= N);
    if (new_size < _size)
    {
      for (size_type i = new_size; i < _size; ++i)
      {
        data()[i].~value_type();
      }
    }
    else if (new_size > _size)
    {
      for (size_type i = _size; i < new_size; ++i)
      {
        new (data() + i) value_type();
      }
    }
    _size = new_size;
  }

  EH_RTREE_DEVICE_HOST pointer begin()
  {
    return data();
  }
  EH_RTREE_DEVICE_HOST const_pointer begin() const
  {
    return data();
  }
  EH_RTREE_DEVICE_HOST pointer end()
  {
    return data() + _size;
  }
  EH_RTREE_DEVICE_HOST const_pointer end() const
  {
    return data() + _size;
  }
};
}
}