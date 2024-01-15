#include <gtest/gtest.h>

#include <RTree.hpp>

namespace er = eh::rtree;

TEST( BoundTest, Initialize )
{
  er::bound_t bound( 10, 20 );
  EXPECT_EQ( bound.min(), 10 );
  EXPECT_EQ( bound.max(), 20 );
}
TEST( BoundTest, IsInside )
{
  er::bound_t bound( 10, 20 );
  er::bound_t bound_inside1( 11, 19 );
  er::bound_t bound_inside2( 11, 20 );
  er::bound_t bound_inside3( 15, 16 );

  er::bound_t left_close( 5, 11 );
  er::bound_t left_far1( 5, 10 );
  er::bound_t left_far2( 0, 5 );

  er::bound_t right_close( 19, 25 );
  er::bound_t right_far1( 20, 25 );
  er::bound_t right_far2( 25, 30 );

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

TEST( BoundTest, IsOverlap )
{
  er::bound_t bound( 10, 20 );
  er::bound_t bound_inside1( 11, 19 );
  er::bound_t bound_inside2( 11, 20 );
  er::bound_t bound_inside3( 15, 16 );

  er::bound_t left_close( 5, 11 );
  er::bound_t left_far1( 5, 10 );
  er::bound_t left_far2( 0, 5 );

  er::bound_t right_close( 19, 25 );
  er::bound_t right_far1( 20, 25 );
  er::bound_t right_far2( 25, 30 );

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

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}