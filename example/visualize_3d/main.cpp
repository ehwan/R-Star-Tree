#include "RTree/aabb.hpp"
#include <RTree.hpp>

#include <iostream>
#include <random>

int main(int argc, char** argv)
{
  using point_type = eh::rtree::point_t<double, 3>;
  using aabb_type = eh::rtree::aabb_t<point_type>;
  using rtree_type = eh::rtree::RTree<aabb_type, point_type, int>;

  std::mt19937 mt_engine { std::random_device {}() };

  // normal distribution random generator for each dimension
  // mu = 0, sigma = 5
  std::normal_distribution<double> normal_distribute(0, 5);

  if (argc < 2)
  {
    std::cerr << "Invalid Arguments:\n";
    std::cerr << argv[0] << " (Number of Points)\n";
    return 0;
  }

  const int N = std::atoi(argv[1]);

  rtree_type rtree;
  for (int i = 0; i < N; ++i)
  {
    double x, y, z;
    x = normal_distribute(mt_engine);
    y = normal_distribute(mt_engine);
    z = normal_distribute(mt_engine);

    const double epsilon = 1e-6;
    point_type point = { x, y, z };

    rtree.insert({ point, i + 1 });
  }

  // print tree structures to stdout
  std::ostream& output = std::cout;

  output << rtree.leaf_level() << "\n";

  for (int level = 0; level < rtree.leaf_level(); ++level)
  {
    int count = 0;
    for (auto ni = rtree.node_begin(level); ni != rtree.node_end(level); ++ni)
    {
      rtree_type::node_type* node = *ni;
      count += node->size();
    }
    output << count;

    for (auto ni = rtree.node_begin(level); ni != rtree.node_end(level); ++ni)
    {
      rtree_type::node_type* node = *ni;

      for (rtree_type::node_type::value_type& c : *node)
      {
        output << " " << c.first.min_[0] << " " << c.first.min_[1] << " "
               << c.first.min_[2];
        output << " " << c.first.max_[0] << " " << c.first.max_[1] << " "
               << c.first.max_[2];
      }
    }
    output << "\n";
  }
  int count = 0;
  for (auto ni = rtree.leaf_begin(); ni != rtree.leaf_end(); ++ni)
  {
    rtree_type::leaf_type* leaf = *ni;
    count += leaf->size();
  }
  output << count;
  for (auto ni = rtree.leaf_begin(); ni != rtree.leaf_end(); ++ni)
  {
    rtree_type::leaf_type* leaf = *ni;
    for (rtree_type::leaf_type::value_type& c : *leaf)
    {
      output << " " << c.first[0] << " " << c.first[1] << " " << c.first[2];
    }
  }
  output << "\n";

  return 0;
}