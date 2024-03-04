#pragma once

#if defined(__NVCC__) && defined(EH_RTREE_CUDA)
  #include <cuda/std/utility>
#else
  #include <utility>
#endif

namespace eh
{
namespace rtree
{

#if defined(__NVCC__) && defined(EH_RTREE_CUDA)
  #define EH_RTREE_DEVICE __device__
  #define EH_RTREE_HOST __host__
  #define EH_RTREE_DEVICE_HOST __device__ __host__
template <typename A, typename B>
using pair = cuda::std::pair<A, B>;

#else
  #define EH_RTREE_DEVICE
  #define EH_RTREE_HOST
  #define EH_RTREE_DEVICE_HOST
template <typename A, typename B>
using pair = std::pair<A, B>;
#endif

}
}
