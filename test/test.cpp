#include <gtest/gtest.h>

#include <RTree.hpp>

#include <map>

namespace er = eh::rtree;
using ibound_type = er::bound_t<int>;
using fbound_type = er::bound_t<float>;
using dbound_type = er::bound_t<double>;

TEST( BoundTest, Initialize )
{
  ibound_type bound( 10, 20 );
  EXPECT_EQ( bound.min(), 10 );
  EXPECT_EQ( bound.max(), 20 );
}
TEST( BoundTest, Initialize_f )
{
  fbound_type bound( 10.0f, 20.0f );
  EXPECT_FLOAT_EQ( bound.min(), 10.0f );
  EXPECT_FLOAT_EQ( bound.max(), 20.0f );
}
TEST( BoundTest, IsInside )
{
  ibound_type bound( 10, 20 );
  ibound_type bound_inside1( 11, 19 );
  ibound_type bound_inside2( 11, 20 );
  ibound_type bound_inside3( 15, 16 );

  ibound_type left_close( 5, 11 );
  ibound_type left_far1( 5, 10 );
  ibound_type left_far2( 0, 5 );

  ibound_type right_close( 19, 25 );
  ibound_type right_far1( 20, 25 );
  ibound_type right_far2( 25, 30 );

  int point_in1 = 10;
  int point_in2 = 15;
  int point_in3 = 19;

  int point_out1 = 9;
  int point_out2 = 20;
  int point_out3 = 25;

  EXPECT_TRUE( bound.is_inside(bound) );

  EXPECT_TRUE( bound.is_inside(bound_inside1) );
  EXPECT_TRUE( bound.is_inside(bound_inside2) );
  EXPECT_TRUE( bound.is_inside(bound_inside3) );

  EXPECT_FALSE( bound.is_inside(left_close) );
  EXPECT_FALSE( bound.is_inside(left_far1) );
  EXPECT_FALSE( bound.is_inside(left_far2) );

  EXPECT_FALSE( bound.is_inside(right_close) );
  EXPECT_FALSE( bound.is_inside(right_far1) );
  EXPECT_FALSE( bound.is_inside(right_far2) );

  EXPECT_TRUE( bound.is_inside(point_in1) );
  EXPECT_TRUE( bound.is_inside(point_in2) );
  EXPECT_TRUE( bound.is_inside(point_in3) );

  EXPECT_FALSE( bound.is_inside(point_out1) );
  EXPECT_FALSE( bound.is_inside(point_out2) );
  EXPECT_FALSE( bound.is_inside(point_out3) );
}
TEST( BoundTest, IsInside_f )
{
  fbound_type bound( 10, 20 );
  fbound_type bound_inside1( 11, 19 );
  fbound_type bound_inside2( 11, 20 );
  fbound_type bound_inside3( 15, 16 );

  fbound_type left_close( 5, 11 );
  fbound_type left_far1( 5, 10 );
  fbound_type left_far2( 0, 5 );

  fbound_type right_close( 19, 25 );
  fbound_type right_far1( 20, 25 );
  fbound_type right_far2( 25, 30 );

  float point_in1 = 10;
  float point_in2 = 15;
  float point_in3 = 19;

  float point_out1 = 9;
  float point_out2 = 20;
  float point_out3 = 25;

  EXPECT_TRUE( bound.is_inside(bound) );

  EXPECT_TRUE( bound.is_inside(bound_inside1) );
  EXPECT_TRUE( bound.is_inside(bound_inside2) );
  EXPECT_TRUE( bound.is_inside(bound_inside3) );

  EXPECT_FALSE( bound.is_inside(left_close) );
  EXPECT_FALSE( bound.is_inside(left_far1) );
  EXPECT_FALSE( bound.is_inside(left_far2) );

  EXPECT_FALSE( bound.is_inside(right_close) );
  EXPECT_FALSE( bound.is_inside(right_far1) );
  EXPECT_FALSE( bound.is_inside(right_far2) );

  EXPECT_TRUE( bound.is_inside(point_in1) );
  EXPECT_TRUE( bound.is_inside(point_in2) );
  EXPECT_TRUE( bound.is_inside(point_in3) );

  EXPECT_FALSE( bound.is_inside(point_out1) );
  EXPECT_FALSE( bound.is_inside(point_out2) );
  EXPECT_FALSE( bound.is_inside(point_out3) );
}

TEST( BoundTest, IsOverlap )
{
  ibound_type bound( 10, 20 );
  ibound_type bound_inside1( 11, 19 );
  ibound_type bound_inside2( 11, 20 );
  ibound_type bound_inside3( 15, 16 );

  ibound_type left_close( 5, 11 );
  ibound_type left_far1( 5, 10 );
  ibound_type left_far2( 0, 5 );

  ibound_type right_close( 19, 25 );
  ibound_type right_far1( 20, 25 );
  ibound_type right_far2( 25, 30 );

  int point_in1 = 10;
  int point_in2 = 15;
  int point_in3 = 19;

  int point_out1 = 9;
  int point_out2 = 20;
  int point_out3 = 25;


  EXPECT_TRUE( bound.is_overlap(bound) );
  EXPECT_TRUE( bound.is_overlap(bound_inside1) );
  EXPECT_TRUE( bound.is_overlap(bound_inside2) );
  EXPECT_TRUE( bound.is_overlap(bound_inside3) );

  EXPECT_TRUE( bound.is_overlap(left_close) );
  EXPECT_FALSE( bound.is_overlap(left_far1) );
  EXPECT_FALSE( bound.is_overlap(left_far2) );
  EXPECT_TRUE( bound.is_overlap(right_close) );
  EXPECT_FALSE( bound.is_overlap(right_far1) );
  EXPECT_FALSE( bound.is_overlap(right_far2) );

  EXPECT_TRUE( bound.is_overlap(point_in1) );
  EXPECT_TRUE( bound.is_overlap(point_in2) );
  EXPECT_TRUE( bound.is_overlap(point_in3) );

  EXPECT_FALSE( bound.is_overlap(point_out1) );
  EXPECT_FALSE( bound.is_overlap(point_out2) );
  EXPECT_FALSE( bound.is_overlap(point_out3) );
}
TEST( BoundTest, IsOverlap_f )
{
  fbound_type bound( 10, 20 );
  fbound_type bound_inside1( 11, 19 );
  fbound_type bound_inside2( 11, 20 );
  fbound_type bound_inside3( 15, 16 );

  fbound_type left_close( 5, 11 );
  fbound_type left_far1( 5, 10 );
  fbound_type left_far2( 0, 5 );

  fbound_type right_close( 19, 25 );
  fbound_type right_far1( 20, 25 );
  fbound_type right_far2( 25, 30 );

  float point_in1 = 10;
  float point_in2 = 15;
  float point_in3 = 19;

  float point_out1 = 9;
  float point_out2 = 20;
  float point_out3 = 25;


  EXPECT_TRUE( bound.is_overlap(bound) );
  EXPECT_TRUE( bound.is_overlap(bound_inside1) );
  EXPECT_TRUE( bound.is_overlap(bound_inside2) );
  EXPECT_TRUE( bound.is_overlap(bound_inside3) );

  EXPECT_TRUE( bound.is_overlap(left_close) );
  EXPECT_FALSE( bound.is_overlap(left_far1) );
  EXPECT_FALSE( bound.is_overlap(left_far2) );
  EXPECT_TRUE( bound.is_overlap(right_close) );
  EXPECT_FALSE( bound.is_overlap(right_far1) );
  EXPECT_FALSE( bound.is_overlap(right_far2) );

  EXPECT_TRUE( bound.is_overlap(point_in1) );
  EXPECT_TRUE( bound.is_overlap(point_in2) );
  EXPECT_TRUE( bound.is_overlap(point_in3) );

  EXPECT_FALSE( bound.is_overlap(point_out1) );
  EXPECT_FALSE( bound.is_overlap(point_out2) );
  EXPECT_FALSE( bound.is_overlap(point_out3) );
}

TEST( BoundTest, Merge )
{
  ibound_type left( 0, 3 );
  ibound_type right( 6, 11 );

  ibound_type merged = left.merged(right);
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 11 );

  merged = right.merged( left );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 11 );

  merged = left.merged( 3 );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 3 );
  merged = left.merged( 0 );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 3 );

  merged = left.merged( -1 );
  EXPECT_EQ( merged.min(), -1 );
  EXPECT_EQ( merged.max(), 3 );

  merged = left.merged( 5 );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 5 );

  left = ibound_type( 0, 6 );
  right = ibound_type( 3, 10 );
  merged = left.merged( right );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 10 );
  merged = right.merged( left );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 10 );


  left = ibound_type( 0, 10 );
  right = ibound_type( 3, 6 );
  merged = left.merged( right );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 10 );
  merged = right.merged( left );
  EXPECT_EQ( merged.min(), 0 );
  EXPECT_EQ( merged.max(), 10 );

  left = ibound_type();
  merged = left.merged( right );
  EXPECT_EQ( merged.min(), right.min() );
  EXPECT_EQ( merged.max(), right.max() );
  merged = right.merged( left );
  EXPECT_EQ( merged.min(), right.min() );
  EXPECT_EQ( merged.max(), right.max() );
}

TEST( BoundTest, Merge_f )
{
  fbound_type left( 0, 3 );
  fbound_type right( 6, 11 );

  fbound_type merged = left.merged(right);
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 11 );

  merged = right.merged( left );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 11 );

  merged = left.merged( 3 );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 3 );
  merged = left.merged( 0 );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 3 );

  merged = left.merged( -1 );
  EXPECT_FLOAT_EQ( merged.min(), -1 );
  EXPECT_FLOAT_EQ( merged.max(), 3 );

  merged = left.merged( 5 );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 5 );

  left = fbound_type( 0, 6 );
  right = fbound_type( 3, 10 );
  merged = left.merged( right );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 10 );
  merged = right.merged( left );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 10 );


  left = fbound_type( 0, 10 );
  right = fbound_type( 3, 6 );
  merged = left.merged( right );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 10 );
  merged = right.merged( left );
  EXPECT_FLOAT_EQ( merged.min(), 0 );
  EXPECT_FLOAT_EQ( merged.max(), 10 );

  left = fbound_type();
  merged = left.merged( right );
  EXPECT_FLOAT_EQ( merged.min(), right.min() );
  EXPECT_FLOAT_EQ( merged.max(), right.max() );
  merged = right.merged( left );
  EXPECT_FLOAT_EQ( merged.min(), right.min() );
  EXPECT_FLOAT_EQ( merged.max(), right.max() );
}

TEST( BoundTest, Area )
{
  ibound_type bound( 5, 5 );
  EXPECT_EQ( bound.area(), 0 );

  bound = ibound_type( 10, 15 );
  EXPECT_EQ( bound.area(), bound.max()-bound.min() );
}
TEST( BoundTest, Area_f )
{
  fbound_type bound( 5, 5 );
  EXPECT_FLOAT_EQ( bound.area(), 0 );

  bound = fbound_type( 10, 15 );
  EXPECT_FLOAT_EQ( bound.area(), bound.max()-bound.min() );
}

TEST( BoundTest, Intersection )
{
  ibound_type left( 0, 5 );
  ibound_type right( 10, 15 );

  auto inter = left.intersection( left );
  EXPECT_EQ( inter.min(), 0 );
  EXPECT_EQ( inter.max(), 5 );

  inter = left.intersection( right );
  EXPECT_EQ( inter.area(), 0 );
  inter = right.intersection( left );
  EXPECT_EQ( inter.area(), 0 );

  right = ibound_type( 3, 10 );
  inter = left.intersection( right );
  EXPECT_EQ( inter.min(), 3 );
  EXPECT_EQ( inter.max(), 5 );
  inter = right.intersection( left );
  EXPECT_EQ( inter.min(), 3 );
  EXPECT_EQ( inter.max(), 5 );

  left = ibound_type( 0, 10 );
  right = ibound_type( 3, 6 );
  inter = left.intersection( right );
  EXPECT_EQ( inter.min(), right.min() );
  EXPECT_EQ( inter.max(), right.max() );
  inter = right.intersection( left );
  EXPECT_EQ( inter.min(), right.min() );
  EXPECT_EQ( inter.max(), right.max() );
}

TEST( BoundTest, Intersection_f )
{
  fbound_type left( 0, 5 );
  fbound_type right( 10, 15 );

  auto inter = left.intersection( left );
  EXPECT_FLOAT_EQ( inter.min(), 0 );
  EXPECT_FLOAT_EQ( inter.max(), 5 );

  inter = left.intersection( right );
  EXPECT_FLOAT_EQ( inter.area(), 0 );
  inter = right.intersection( left );
  EXPECT_FLOAT_EQ( inter.area(), 0 );

  right = fbound_type( 3, 10 );
  inter = left.intersection( right );
  EXPECT_FLOAT_EQ( inter.min(), 3 );
  EXPECT_FLOAT_EQ( inter.max(), 5 );
  inter = right.intersection( left );
  EXPECT_FLOAT_EQ( inter.min(), 3 );
  EXPECT_FLOAT_EQ( inter.max(), 5 );

  left = fbound_type( 0, 10 );
  right = fbound_type( 3, 6 );
  inter = left.intersection( right );
  EXPECT_FLOAT_EQ( inter.min(), right.min() );
  EXPECT_FLOAT_EQ( inter.max(), right.max() );
  inter = right.intersection( left );
  EXPECT_FLOAT_EQ( inter.min(), right.min() );
  EXPECT_FLOAT_EQ( inter.max(), right.max() );
}

TEST( RTreeTest, QuadraticSplit )
{
  using node_type = er::node_t<ibound_type,int>;
  node_type *root = new node_type;

  for( int i=1; i<=er::RTree::MAX_ENTRIES+1; ++i )
  {
    node_type *data_node = new node_type;
    data_node->data() = i;

    root->add_child( ibound_type(i,i+1), data_node );
  }
  ASSERT_EQ( root->size(), er::RTree::MAX_ENTRIES+1 );

  auto spliter = er::RTree::split_quadratic_t{};
  auto [a, b] = spliter.pick_seed( root );
  ASSERT_GE( a, root->begin() );
  ASSERT_LT( a, root->end() );
  ASSERT_GE( b, root->begin() );
  ASSERT_LT( b, root->end() );

  auto *pair = er::RTree::split_quadratic_t{}( root );
  ASSERT_TRUE( pair );

  EXPECT_GE( root->size(), er::RTree::MIN_ENTRIES );
  EXPECT_LE( root->size(), er::RTree::MAX_ENTRIES );
  EXPECT_GE( pair->size(), er::RTree::MIN_ENTRIES );
  EXPECT_LE( pair->size(), er::RTree::MAX_ENTRIES );
  EXPECT_EQ( root->size()+pair->size(), er::RTree::MAX_ENTRIES+1 );


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
  EXPECT_EQ( child_exist_map.size(), er::RTree::MAX_ENTRIES+1 );
  for( int i=1; i<=er::RTree::MAX_ENTRIES+1; ++i )
  {
    EXPECT_EQ( child_exist_map[i], 10000 );
  }

  root->delete_recursive();
  pair->delete_recursive();
  delete root;
  delete pair;
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}