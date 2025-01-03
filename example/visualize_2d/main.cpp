#include "RTree/aabb.hpp"
#include <RTree.hpp>

#include <fstream>
#include <iostream>
#include <random>

int main(int argc, char** argv)
{
  using point_type = eh::rtree::point_t<double, 2>;
  using aabb_type = eh::rtree::aabb_t<point_type>;
  using rtree_type = eh::rtree::RTree<aabb_type, point_type, int>;

  std::mt19937 mt_engine { std::random_device {}() };

  // normal distribution random generator for radius
  // mu = 0, sigma = 5
  std::normal_distribution<double> normal_distribute(0, 5);

  // uniform distribution in [0,\pi] for theta
  std::uniform_real_distribution<double> uniform_distribute(0, std::atan2(1, 1)
                                                                   * 4);

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
    std::cout << i << "\n";
    double r = normal_distribute(mt_engine);
    double theta = uniform_distribute(mt_engine);

    const double epsilon = 1e-6;
    point_type point = { r * std::cos(theta), r * std::sin(theta) };

    rtree.insert({ point, i + 1 });

    std::string name = "Point" + std::to_string(i + 1) + ".txt";
    std::ofstream output(name);

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
          output << " " << c.first.min_[0] << " " << c.first.min_[1];
          output << " " << c.first.max_[0] << " " << c.first.max_[1];
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
        output << " " << c.first[0] << " " << c.first[1];
      }
    }
    output << "\n";
  }

  return 0;
}