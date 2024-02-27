#pragma once

#if defined(__NVCC__) && defined(EH_RTREE_CUDA)
  #define EH_RTREE_DEVICE __device__
  #define EH_RTREE_HOST __host__
  #define EH_RTREE_DEVICE_HOST __device__ __host__
#else
  #define EH_RTREE_DEVICE
  #define EH_RTREE_HOST
  #define EH_RTREE_DEVICE_HOST
#endif

namespace eh
{
namespace rtree
{

}
}
