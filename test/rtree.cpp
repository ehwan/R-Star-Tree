#include <gtest/gtest.h>

#include <RTree.hpp>
#include <map>
#include <vector>
#include <algorithm>

namespace er = eh::rtree;

TEST( RTreeTest, QuadraticSplit )
{
  using rtree_type = er::RTree<er::bound_t<int>,int>;
  using node_type = rtree_type::node_type;
  node_type *root = new node_type;

  for( int i=1; i<=rtree_type::MAX_ENTRIES+1; ++i )
  {
    node_type *data_node = new node_type;
    data_node->data() = i;

    root->add_child( er::bound_t<int>(i,i+1), data_node );
  }
  ASSERT_EQ( root->size(), rtree_type::MAX_ENTRIES+1 );

  auto spliter = rtree_type::split_quadratic_t{};
  auto [a, b] = spliter.pick_seed( root );
  ASSERT_GE( a, root->begin() );
  ASSERT_LT( a, root->end() );
  ASSERT_GE( b, root->begin() );
  ASSERT_LT( b, root->end() );

  auto *pair = rtree_type::split_quadratic_t{}( root );
  ASSERT_TRUE( pair );

  EXPECT_GE( root->size(), rtree_type::MIN_ENTRIES );
  EXPECT_LE( root->size(), rtree_type::MAX_ENTRIES );
  EXPECT_GE( pair->size(), rtree_type::MIN_ENTRIES );
  EXPECT_LE( pair->size(), rtree_type::MAX_ENTRIES );
  EXPECT_EQ( root->size()+pair->size(), rtree_type::MAX_ENTRIES+1 );


  std::map<int,int> child_exist_map;
  for( auto c : *root )
  {
    EXPECT_TRUE( c.second );

    child_exist_map[ c.second->data() ] = 10000;
  }
  for( auto c : *pair )
  {
    EXPECT_TRUE( c.second );

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

  vector_expect( rtree_to_sorted_vec(rtree), {} );

  rtree.insert( {0,1}, 5 );
  vector_expect( rtree_to_sorted_vec(rtree), {5} );

  rtree.insert( {2,3}, 10 );
  vector_expect( rtree_to_sorted_vec(rtree), {5,10} );

  rtree.insert( {3,4}, 0);
  vector_expect( rtree_to_sorted_vec(rtree), {0,5,10} );

  rtree.insert( {3,4}, 0);
  vector_expect( rtree_to_sorted_vec(rtree), {0,0,5,10} );

  rtree.insert( {3,4}, 0);
  vector_expect( rtree_to_sorted_vec(rtree), {0,0,0,5,10} );

  rtree.insert( {3,4}, 20);
  vector_expect( rtree_to_sorted_vec(rtree), {0,0,0,5,10,20} );

  rtree.insert( {3,4}, 20);
  vector_expect( rtree_to_sorted_vec(rtree), {0,0,0,5,10,20,20} );

  rtree.insert( {3,4}, 20);
  vector_expect( rtree_to_sorted_vec(rtree), {0,0,0,5,10,20,20, 20} );

  rtree.insert( {3,4}, 30);
  vector_expect( rtree_to_sorted_vec(rtree), {0,0,0,5,10,20,20,20, 30} );

  rtree.insert( {3,4}, 30);
  vector_expect( rtree_to_sorted_vec(rtree), {0,0,0,5,10,20,20,20, 30, 30} );
}