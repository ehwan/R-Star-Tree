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

// query points inside range
void search_range(rtree_type const& rtree, aabb_type const& range);

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

  // access node data directly
  constexpr int ROOT_LEVEL = 0;
  rtree_type::node_type* node = rtree.root();

  // if 'node' is on leaf level, must cast it to leaf_type
  // since leaf_node and node have different member structures, but treated as
  // same 'node-pointer'
  if (rtree.leaf_level() == ROOT_LEVEL)
  {
    rtree_type::leaf_type* leaf
        = reinterpret_cast<rtree_type::leaf_type*>(node);
    // can be simplified to:
    // rtree_type::leaf_type* leaf = node->as_leaf();
  }

  // begin(), end() implemented on node|leaf _type
  // iterates over all child in that node
  for (auto child : *node)
  {
    auto child_bounding_box = child.first; // bounding box
    auto* child_node = child.second->as_node(); // child node pointer
  }

  // leaf's child is value_type
  for (auto& leaf_child : *node->as_leaf())
  {
    auto child_bounding_box = leaf_child.first;
    auto& child_data = leaf_child.second;
  }

  // spatial query in rtree example
  // query points in range [10.5, 20.5]
  search_range(rtree, aabb_type(10.5, 20.5));
  // this print 10 points in range [10.5, 20.5]

  return 0;
}

/*
  Note that 'node' does not have variable of 'level' or 'height'.
  Thoes could be calculated by traversing from node to the root, but it takes
  log(N) time complexity to calculate.
  So it is better to pass 'level' as parameter to recursive function.
  */
void search_node_recursive(rtree_type const& rtree,
                           rtree_type::node_type const* node,
                           aabb_type const& range,
                           int node_level)
{
  if (node_level == rtree.leaf_level())
  {
    // 'node' is on leaf level
    auto* leaf = node->as_leaf();
    for (auto value : *leaf)
    {
      // check if value is in range
      if (rtree_type::traits::is_inside(range, value.first))
      {
        std::cout << "Value Found: [" << value.first << ", " << value.second
                  << "]\n";
      }
    }
  }
  else
  {
    // 'node' is not leaf_node
    for (auto child_node : *node)
    {
      // check if child_node possibly contains value in range
      if (rtree_type::traits::is_overlap(child_node.first, range))
      {
        // search recursively on child node
        search_node_recursive(rtree, child_node.second->as_node(), range,
                              node_level + 1);
      }
    }
  }
}

void search_range(rtree_type const& rtree, aabb_type const& range)
{
  search_node_recursive(rtree, rtree.root(), range, 0);
}