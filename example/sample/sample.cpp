#include <RTree.hpp>
#include <iterator>
#include <type_traits>
#include <iostream>

int main()
{
  // ***************************
  // one-dimension RTree example
  // ***************************

  // bounding box representation type
  using aabb_type = eh::rtree::aabb_t<double>;

  // RTree representation type
  // using aabb_type as bounding box,
  // double as per-data key_type
  // int as per-data data_type
  using rtree_type = eh::rtree::RTree< aabb_type, double, int >;

  // std::map - like member types;
  // value_type = std::pair< key_type, mapped_type >
  using value_type = rtree_type::value_type;
  using iterator = rtree_type::iterator;
  using const_iterator = rtree_type::const_iterator;
  static_assert( 
    std::is_same_v<
      std::iterator_traits<iterator>::iterator_category,
      std::bidirectional_iterator_tag
    >,
    "Iterators are Bidirectional Iterator"
  );

  rtree_type rtree;

  // insert 100 arithmetic sequence of points
  for( int i=0; i<50; ++i )
  {
    double point = i;
    int value = i;
    rtree.insert( value_type(point, value) );
  }

  // rtree.begin(), rtree.end()
  for( value_type value : rtree )
  {
    std::cout << "Value Inserted: [" << value.first << ", " << value.second << "]\n";
  }

  // search for points inside range [10, 20)
  rtree.search_inside(
    aabb_type( 10.0, 20.0-1e-9 ),
    []( value_type const& v )
    {
      std::cout << "Search Result: [" << v.first << ", " << v.second << "]\n";
      // continue searching
      return false;

      // return true to stop searching
    }
  );


  // access node data directly
  rtree_type::node_type* node = rtree.root();

  // if 'node' is on leaf level, must cast it to leaf_type
  if( rtree.leaf_level() == 0 )
  {
    rtree_type::leaf_type* leaf = reinterpret_cast<rtree_type::leaf_type*>(node);
    // can be simplified to:
    // rtree_type::leaf_type* leaf = node->as_leaf();
  }

  // begin(), end() implemented on node|leaf _type
  for( auto child : *node )
  {
    auto child_bounding_box = child.first; // bounding box
    auto *child_node = child.second->as_node(); // child node pointer
  }

  return 0;
}