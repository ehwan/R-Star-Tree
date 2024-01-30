#include <RTree.hpp>

#include <iostream>
#include <random>

int main(int argc, char** argv)
{
  using bound_type = eh::rtree::aabb_t<double>;
  using rtree_type = eh::rtree::RTree<bound_type, double, int>;

  std::mt19937 mt_engine { std::random_device {}() };

  // normal distribution random generator
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
    double point = normal_distribute(mt_engine);

    rtree.insert({ point, i + 1 });
  }

  // print tree structures to stdout
  std::ostream& output = std::cout;

  output << rtree.leaf_level() << "\n";

  for (int level = 0; level < rtree.leaf_level(); ++level)
  {
    int count = 0;
    for (auto ni = rtree.begin(level); ni != rtree.end(level); ++ni)
    {
      rtree_type::node_type* node = *ni;
      count += node->size();
    }
    output << count;

    for (auto ni = rtree.begin(level); ni != rtree.end(level); ++ni)
    {
      rtree_type::node_type* node = *ni;

      for (rtree_type::node_type::value_type& c : *node)
      {
        output << " " << c.first.min_ << " " << c.first.max_;
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
      output << " " << c.first;
    }
  }
  output << "\n";

  return 0;
}