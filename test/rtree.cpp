#include <gtest/gtest.h>

#include <RTree.hpp>
#include <map>

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