#include <gtest/gtest.h>
#include <RTree.hpp>

// one-dimensional bound_type with arithmetic point type
using ibound_type = eh::rtree::bound_t<int>;
using fbound_type = eh::rtree::bound_t<float>;
using dbound_type = eh::rtree::bound_t<double>;

TEST( BoundTest, Initialize )
{
  ibound_type bound( 10, 20 );
  EXPECT_EQ( bound.min_bound(), 10 );
  EXPECT_EQ( bound.max_bound(), 20 );
}
TEST( BoundTest, Initialize_f )
{
  fbound_type bound( 10.0f, 20.0f );
  EXPECT_FLOAT_EQ( bound.min_bound(), 10.0f );
  EXPECT_FLOAT_EQ( bound.max_bound(), 20.0f );
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
  EXPECT_EQ( merged.min_bound(), 0 );
  EXPECT_EQ( merged.max_bound(), 11 );

  merged = right.merged( left );
  EXPECT_EQ( merged.min_bound(), 0 );
  EXPECT_EQ( merged.max_bound(), 11 );

  left = ibound_type( 0, 6 );
  right = ibound_type( 3, 10 );
  merged = left.merged( right );
  EXPECT_EQ( merged.min_bound(), 0 );
  EXPECT_EQ( merged.max_bound(), 10 );
  merged = right.merged( left );
  EXPECT_EQ( merged.min_bound(), 0 );
  EXPECT_EQ( merged.max_bound(), 10 );


  left = ibound_type( 0, 10 );
  right = ibound_type( 3, 6 );
  merged = left.merged( right );
  EXPECT_EQ( merged.min_bound(), 0 );
  EXPECT_EQ( merged.max_bound(), 10 );
  merged = right.merged( left );
  EXPECT_EQ( merged.min_bound(), 0 );
  EXPECT_EQ( merged.max_bound(), 10 );
}

TEST( BoundTest, Merge_f )
{
  fbound_type left( 0, 3 );
  fbound_type right( 6, 11 );

  fbound_type merged = left.merged(right);
  EXPECT_FLOAT_EQ( merged.min_bound(), 0 );
  EXPECT_FLOAT_EQ( merged.max_bound(), 11 );

  merged = right.merged( left );
  EXPECT_FLOAT_EQ( merged.min_bound(), 0 );
  EXPECT_FLOAT_EQ( merged.max_bound(), 11 );

  left = fbound_type( 0, 6 );
  right = fbound_type( 3, 10 );
  merged = left.merged( right );
  EXPECT_FLOAT_EQ( merged.min_bound(), 0 );
  EXPECT_FLOAT_EQ( merged.max_bound(), 10 );
  merged = right.merged( left );
  EXPECT_FLOAT_EQ( merged.min_bound(), 0 );
  EXPECT_FLOAT_EQ( merged.max_bound(), 10 );


  left = fbound_type( 0, 10 );
  right = fbound_type( 3, 6 );
  merged = left.merged( right );
  EXPECT_FLOAT_EQ( merged.min_bound(), 0 );
  EXPECT_FLOAT_EQ( merged.max_bound(), 10 );
  merged = right.merged( left );
  EXPECT_FLOAT_EQ( merged.min_bound(), 0 );
  EXPECT_FLOAT_EQ( merged.max_bound(), 10 );
}

TEST( BoundTest, Area )
{
  ibound_type bound( 5, 5 );
  EXPECT_EQ( bound.area(), 0 );

  bound = ibound_type( 10, 15 );
  EXPECT_EQ( bound.area(), bound.max_bound()-bound.min_bound() );
}
TEST( BoundTest, Area_f )
{
  fbound_type bound( 5, 5 );
  EXPECT_FLOAT_EQ( bound.area(), 0 );

  bound = fbound_type( 10, 15 );
  EXPECT_FLOAT_EQ( bound.area(), bound.max_bound()-bound.min_bound() );
}

TEST( BoundTest, Intersection )
{
  ibound_type left( 0, 5 );
  ibound_type right( 10, 15 );

  auto inter = left.intersection( left );
  EXPECT_EQ( inter.min_bound(), 0 );
  EXPECT_EQ( inter.max_bound(), 5 );

  inter = left.intersection( right );
  EXPECT_EQ( inter.area(), 0 );
  inter = right.intersection( left );
  EXPECT_EQ( inter.area(), 0 );

  right = ibound_type( 3, 10 );
  inter = left.intersection( right );
  EXPECT_EQ( inter.min_bound(), 3 );
  EXPECT_EQ( inter.max_bound(), 5 );
  inter = right.intersection( left );
  EXPECT_EQ( inter.min_bound(), 3 );
  EXPECT_EQ( inter.max_bound(), 5 );

  left = ibound_type( 0, 10 );
  right = ibound_type( 3, 6 );
  inter = left.intersection( right );
  EXPECT_EQ( inter.min_bound(), right.min_bound() );
  EXPECT_EQ( inter.max_bound(), right.max_bound() );
  inter = right.intersection( left );
  EXPECT_EQ( inter.min_bound(), right.min_bound() );
  EXPECT_EQ( inter.max_bound(), right.max_bound() );
}

TEST( BoundTest, Intersection_f )
{
  fbound_type left( 0, 5 );
  fbound_type right( 10, 15 );

  auto inter = left.intersection( left );
  EXPECT_FLOAT_EQ( inter.min_bound(), 0 );
  EXPECT_FLOAT_EQ( inter.max_bound(), 5 );

  inter = left.intersection( right );
  EXPECT_FLOAT_EQ( inter.area(), 0 );
  inter = right.intersection( left );
  EXPECT_FLOAT_EQ( inter.area(), 0 );

  right = fbound_type( 3, 10 );
  inter = left.intersection( right );
  EXPECT_FLOAT_EQ( inter.min_bound(), 3 );
  EXPECT_FLOAT_EQ( inter.max_bound(), 5 );
  inter = right.intersection( left );
  EXPECT_FLOAT_EQ( inter.min_bound(), 3 );
  EXPECT_FLOAT_EQ( inter.max_bound(), 5 );

  left = fbound_type( 0, 10 );
  right = fbound_type( 3, 6 );
  inter = left.intersection( right );
  EXPECT_FLOAT_EQ( inter.min_bound(), right.min_bound() );
  EXPECT_FLOAT_EQ( inter.max_bound(), right.max_bound() );
  inter = right.intersection( left );
  EXPECT_FLOAT_EQ( inter.min_bound(), right.min_bound() );
  EXPECT_FLOAT_EQ( inter.max_bound(), right.max_bound() );
}