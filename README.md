# RTree
**Currently Working in Progress.**

C++ template RTree header only library.

## References
 Guttman, A. (1984). "R-Trees: A Dynamic Index Structure for Spatial Searching" (PDF). Proceedings of the 1984 ACM SIGMOD international conference on Management of data – SIGMOD '84. p. 47.

## Dependencies
 **No dependencies required** for core library.

 Unit Tests are using [Google Test](https://github.com/google/googletest), examples are using [Eigen](https://eigen.tuxfamily.org/).

## Sample Codes

example code in `example/sample/sample.cpp`

```cpp
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

  // search for points in range [10, 20)
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

  // ----------------------------
  // accessing node data directly
  // ----------------------------
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

  // leaf's child is value_type
  for( auto &leaf_child : *node->as_leaf() )
  {
    auto child_bounding_box = leaf_child.first;
    auto &child_data = leaf_child.second;
  }

  return 0;
}
```

## Using your own linear-algebra types

To use custom linear algebra types ( eg. Eigen::Vector3f ), you must implement `eh::rtree::geometry_traits`.

example below ( `example/eigen/main.cpp` ) shows how to link `Eigen::Vector<>` types to `RTree<>`.

```cpp
// use custom vector type in RTree class

// link Eigen3::Vector type to geometry_traits
template < typename T, unsigned int Size >
struct my_rect_aabb
{
  using vec_t = Eigen::Vector<T,Size>;

  vec_t min_, max_;

  // point to rect conversion
  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  my_rect_aabb( vec_t const& point )
    : min_(point),
      max_(point)
  {
  }
  my_rect_aabb( vec_t const& min__, vec_t const& max__ )
    : min_(min__),
      max_(max__)
  {
  }
};

namespace eh { namespace rtree {

template < typename T, unsigned int Size >
struct geometry_traits<my_rect_aabb<T,Size>>
{
  using rect_t = my_rect_aabb<T,Size>;
  using vec_t = typename rect_t::vec_t;

  // must define area_type as arithmetic_type
  // s.t., std::numeric_limits<area_type>::max(), lowest() is defined
  using area_type = T;

  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  static bool is_inside( rect_t const& rect, vec_t const& v )
  {
    return (rect.min_.array()<=v.array()).all() && (v.array()<=rect.max_.array()).all();
  }
  static bool is_inside( rect_t const& rect, rect_t const& rect2 )
  {
    return (rect.min_.array()<=rect2.min_.array()).all() && (rect2.max_.array()<=rect.max_.array()).all();
  }

  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  static bool is_overlap( rect_t const& rect, vec_t const& v )
  {
    return is_inside( rect, v );
  }
  static bool is_overlap( rect_t const& rect, rect_t const& rect2 )
  {
    if( (rect2.min_.array() > rect.max_.array()).any() ){ return false; }
    if( (rect.min_.array() > rect2.max_.array()).any() ){ return false; }
    return true;
  }

  // only if you use point ( geometry object that does not have volume in N-dimension ) as key_type
  static rect_t merge( rect_t const& rect, vec_t const& v )
  {
    return { 
      rect.min_.array().min(v.array()), 
      rect.max_.array().max(v.array())
    };
  }
  static rect_t merge( rect_t const& rect, rect_t const& rect2 )
  {
    return { 
      rect.min_.array().min(rect2.min_.array()), 
      rect.max_.array().max(rect2.max_.array())
    };
  }


  static area_type area( rect_t const& rect )
  {
    area_type ret = 1;
    for( unsigned int i=0; i<Size; ++i )
    {
      ret *= ( rect.max_[i] - rect.min_[i] );
    }
    return ret;
  }

  // optional; used in quadratic split resolving conflict
  static rect_t intersection( rect_t const& rect1, rect_t const& rect2 )
  {
    const vec_t ret_min = rect1.min_.array().max( rect2.min_.array() );
    return { ret_min, rect1.max_.array().max(rect2.max_.array()).min( ret_min.array() ) };
  }
};

}}

int main()
{
  using vec_t = Eigen::Vector<double,3>;
  using rect_t = my_rect_aabb<double,3>;
  using rtree_t = eh::rtree::RTree< rect_t, vec_t, int >;
  rtree_t rtree;

  rtree.insert( {vec_t{0,0,0}, 0} );
  rtree.insert( {vec_t{0.5,0.5,0.5}, 1} );
  rtree.insert( {vec_t{0.7,0.7,0.7}, 2} );
  rtree.insert( {vec_t{0.7,1.3,0.7}, 3} );

  rtree.search_inside(
    rect_t{
      vec_t{0.1,0.1,0.1},
      vec_t{1.1,1.1,1.1}
    },
    []( rtree_t::value_type val )
    {
      std::cout << "Value Inside [0.1,0.1,0.1]x[1.1,1.1,1.1]: " << val.second << "\n";
      // continue search
      return false;
    }
  );

  return 0;
}
```


## Visualization Examples
### 1-dimensional R-Tree structure visualization
`example/visualize_1d`

![](example/visualize_1d/images/N300Quadratic.png)

The x-axis illustrates different levels within the R-tree, and the y-axis displays the size of bounding boxes (in one dimension) for each node.

On the far right, there are green dots representing input points (N = 300). These points are generated from a normal distribution with a mean ($\mu$) of 0 and a standard deviation ($\sigma$) of 5.


### 2-dimensional R-Tree structure visualization
`example/visualize_2d`

![](example/visualize_2d/images/N300Quadratic.png)

The bounding boxes on the graph indicate the coverage range of each node. Additionally, the thickness and color of these bounding boxes are about their respective levels. 'blue', 'orange', and 'black' are used to represent levels 2, 1, and 0, respectively.

Purple dots represent input points (N = 300), generated from a normal distribution with an origin of (0,0), a mean ($\mu$) of 0, and a standard deviation ($\sigma$) of 5.


### Spatial Indexing and Raycasting

[This Project](https://github.com/ehwan/RayTracing)

![](example/teapot_diffusive_reflection.png)