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
  return 0;
}