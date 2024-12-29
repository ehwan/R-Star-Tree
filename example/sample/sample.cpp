#include <RTree.hpp>
#include <iostream>
#include <iterator>
#include <type_traits>

// bounding box representation type
using aabb_type = eh::rtree::aabb_t<double>;

// RTree representation type
// using aabb_type as bounding box,
// double as per-data key_type
// int as per-data data_type
using rtree_type = eh::rtree::RTree<aabb_type, double, int>;

// std::map - like member types;
// value_type = std::pair< key_type, mapped_type >
using value_type = rtree_type::value_type;
using iterator = rtree_type::iterator;
using const_iterator = rtree_type::const_iterator;
static_assert(std::is_same<std::iterator_traits<iterator>::iterator_category,
                           std::bidirectional_iterator_tag>::value,
              "Iterators are Bidirectional Iterator");

int main()
{
  // ***************************
  // one-dimension RTree example
  // ***************************

  rtree_type rtree;

  // insert 100 arithmetic sequence of points
  for (int i = 0; i < 50; ++i)
  {
    double point = i;
    int value = i;
    rtree.insert(value_type(point, value));
  }

  // rtree.begin(), rtree.end()
  // iterates over all values in the tree
  for (value_type value : rtree)
  {
    std::cout << "Value Inserted: [" << value.first << ", " << value.second
              << "]\n";
  }

  auto geometry_filter = [](aabb_type const& bound) -> int
  {
    // if bound does not touch [10,20] skip
    if (bound.min_ > 20 || bound.max_ < 10)
      return 0;
    return 1;
  };
  auto data_functor = [](std::pair<double, int> value) -> bool
  {
    std::cout << "Value Found: [" << value.first << ", " << value.second
              << "]\n";
    return false;
  };

  rtree.rebalance();

  rtree.search(geometry_filter, data_functor);
}