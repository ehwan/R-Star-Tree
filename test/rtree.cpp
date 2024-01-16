#include <gtest/gtest.h>

#include <RTree.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <random>

namespace er = eh::rtree;

TEST( RTreeTest, QuadraticSplit )
{
  using rtree_type = er::RTree<er::bound_t<int>,int>;
  using node_type = rtree_type::node_type;
  node_type *root = new node_type;
  node_type *parent = new node_type;
  root->add_child( {0,1}, parent );

  for( int i=1; i<=rtree_type::MAX_ENTRIES+1; ++i )
  {
    node_type *data_node = new node_type;
    data_node->data() = i;

    parent->add_child( er::bound_t<int>(i,i+1), data_node );
  }
  ASSERT_EQ( parent->size(), rtree_type::MAX_ENTRIES+1 );

  auto *pair = rtree_type::quadratic_split_t{}( parent );
  ASSERT_TRUE( pair );

  EXPECT_GE( parent->size(), rtree_type::MIN_ENTRIES );
  EXPECT_LE( parent->size(), rtree_type::MAX_ENTRIES );
  EXPECT_GE( pair->size(), rtree_type::MIN_ENTRIES );
  EXPECT_LE( pair->size(), rtree_type::MAX_ENTRIES );
  EXPECT_EQ( parent->size()+pair->size(), rtree_type::MAX_ENTRIES+1 );

  ASSERT_EQ( parent->parent(), root );
  for( auto &c : *parent )
  {
    ASSERT_TRUE( c.second );
    ASSERT_EQ( c.second->parent(), parent );
  }
  ASSERT_EQ( pair->parent(), nullptr );
  for( auto &c : *pair )
  {
    ASSERT_TRUE( c.second );
    ASSERT_EQ( c.second->parent(), pair );
  }


  std::map<int,int> child_exist_map;
  for( auto c : *parent )
  {
    child_exist_map[ c.second->data() ] = 10000;
  }
  for( auto c : *pair )
  {
    child_exist_map[ c.second->data() ] = 10000;
  }
  EXPECT_EQ( child_exist_map.size(), rtree_type::MAX_ENTRIES+1 );
  for( int i=1; i<=rtree_type::MAX_ENTRIES+1; ++i )
  {
    EXPECT_EQ( child_exist_map[i], 10000 );
  }

  root->delete_recursive();
  pair->delete_recursive();
  delete root;
  delete pair;
}

TEST( RTreeTest, Insert )
{
  using rtree_type = er::RTree<er::bound_t<int>,int>;
  using bound_type = rtree_type::bound_type;
  using node_type = rtree_type::node_type;

  rtree_type rtree;
  auto rtree_to_sorted_vec = []( rtree_type const& rtree )->std::vector<int>
  {
    std::vector<int> ret;
    rtree.iterate(
      [&ret]( bound_type bound, int val )
      {
        ret.push_back( val );
        return false;
      }
    );
    std::sort( ret.begin(), ret.end() );
    return ret;
  };

  auto vector_expect = []( std::vector<int> const& v1, std::vector<int> const& v2 )
  {
    EXPECT_EQ( v1.size(), v2.size() );
    for( int i=0; i<v1.size(); ++i )
    {
      EXPECT_EQ( v1[i], v2[i] ) << "v1[i] vs v2[i] with i: " << i;
    }
  };

  std::vector<int> expected;
  for( int i=0; i<100; ++i )
  {
    rtree.insert( {i,i+1}, i );
    expected.push_back( i );
    vector_expect(expected, rtree_to_sorted_vec(rtree));
  }
}

TEST( RTreeTest, double_RTree_range_test )
{
  using rtree_type = er::RTree<er::bound_t<double>,int>;
  using bound_type = rtree_type::bound_type;

  rtree_type tree;

  std::mt19937 mt( std::random_device{}() );
  std::normal_distribution<double> dist( 0, 5 );

  for( int i=0; i<100; ++i )
  {
    double p = dist( mt );
    double end = p + 1e-2;

    tree.insert( {p,end}, i );

    tree.iterate_node(
      [&]( rtree_type::node_type *node )
      {
        if( node->level() >= tree.leaves_level() ){ return; }
        for( auto &c : *node )
        {
          auto a = c.first;
          auto b = c.second->calculate_bound();
          ASSERT_TRUE( c.first.is_inside(c.second->calculate_bound()) ) << a.min_bound() << ", " << a.max_bound() << " : " << b.min_bound() << ", " << b.max_bound();
        }
      }
    );
  }

  // @TODO
  // RTree multi-dimension

}