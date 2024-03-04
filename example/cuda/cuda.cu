// must define for __host__ __device__ functions
#include <type_traits>
#define EH_RTREE_CUDA
#include <RTree.hpp>
#include <iostream>

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

#include <cuda_runtime.h>

template <typename T>
struct cuda_managed_allocator
{
  using value_type = T;

  value_type* allocate(std::size_t n)
  {
    value_type* result;
    auto err = cudaMallocManaged(&result, n * sizeof(value_type));
    if (err != cudaSuccess)
    {
      std::cerr << cudaGetErrorString(err) << std::endl;
      throw std::bad_alloc();
    }
    if (result == nullptr)
    {
      std::cerr << "Allocation failed\n";
      throw std::bad_alloc();
    }
    if ((intptr_t)(result) % alignof(value_type) != 0)
    {
      std::cerr << "Not aligned\n";
      throw std::bad_alloc();
    }
    std::cout << "Alloc " << n * sizeof(value_type) << " bytes\n";
    return result;
  }
  void deallocate(value_type* p, std::size_t n)
  {
    std::cout << "Dealloc " << n * sizeof(value_type) << " bytes\n";
    cudaFree(p);
  }
};

namespace er = eh::rtree;
using rtree_type
    = er::RTree<er::aabb_t<float>, float, int, 4, 8, cuda_managed_allocator>;

__device__ __host__ void dfs(rtree_type::node_type* root, int leaf_level)
{
  if (leaf_level == 0)
  {
    printf("L: %d %lx\n", root->as_leaf()->size(), root);
    for (auto& c : *root->as_leaf())
    {
      printf("%d\n", c.second);
    }
  }
  else
  {
    printf("%d: %d %lx\n", leaf_level, root->as_node()->size(), root);
    for (auto& c : *root->as_node())
    {
      dfs(c.second->as_node(), leaf_level - 1);
    }
  }
}
__global__ void test_kernel(rtree_type::node_type* root, int leaf_level)
{
  dfs(root, leaf_level);
}

int main(int argc, char** argv)
{
  // 1-dimensional float RTree
  // geometry_traits< aabb_t<> > must be defined as __device__
  rtree_type managed_rtree;

  std::mt19937 generator { std::random_device {}() };
  std::uniform_real_distribution<float> dist01(0, 1);

  const int DATA_COUNT = 100;
  for (int i = 0; i < DATA_COUNT; ++i)
  {
    managed_rtree.insert({ dist01(generator), i });
  }

  dfs(managed_rtree.root(), managed_rtree.leaf_level());
  std::cout << "-------------------------\n";
  cudaDeviceSynchronize();

  test_kernel<<<1, 1>>>(managed_rtree.root(), managed_rtree.leaf_level());
  cudaDeviceSynchronize();

  return 0;
}