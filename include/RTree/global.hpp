#pragma once

#include <cstdint>
#include <iostream>

#ifndef NDEBUG
  #ifndef EH_RTREE_ASSERT
    #define EH_RTREE_ASSERT(x, msg)                                       \
      if (!(x))                                                           \
      {                                                                   \
        std::cerr << "Assertion failed: " << msg << "\nfile " << __FILE__ \
                  << "\nline " << __LINE__ << std::endl;                  \
      }
  #endif
  #ifndef EH_RTREE_ASSERT_SILENT
    #define EH_RTREE_ASSERT_SILENT(x)                              \
      if (!(x))                                                    \
      {                                                            \
        std::cerr << "Assertion failed: " << "\nfile " << __FILE__ \
                  << "\nline " << __LINE__ << std::endl;           \
      }
  #endif

#else // NDEBUG

  #define EH_RTREE_ASSERT(x, msg)
  #define EH_RTREE_ASSERT_SILENT(x)
#endif

namespace eh
{
namespace rtree
{

using size_type = std::uint32_t;

}
}
