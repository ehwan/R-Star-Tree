#include "RTree/point.hpp"
#include <gtest/gtest.h>
#include <RTree.hpp>

using eh::rtree::point_t;

TEST( PointTest, Init )
{
  point_t<int,2> p( 2, 3 );
  EXPECT_EQ( p[0], 2 );
  EXPECT_EQ( p[1], 3 );

  point_t<int,2> p2 = p;
  EXPECT_EQ( p2[0], 2 );
  EXPECT_EQ( p2[1], 3 );
}

TEST( PointTest, Assign )
{
  point_t<int,3> p;

  std::vector<int> v3 = { 1, 2, 3 };
  std::vector<int> v2 = { 4, 5 };
  std::vector<int> v1 = { 6 };

  p.assign( v3.begin(), v3.end() );
  EXPECT_EQ( p[0], 1 );
  EXPECT_EQ( p[1], 2 );
  EXPECT_EQ( p[2], 3 );

  p.assign( v2.begin(), v2.end() );
  EXPECT_EQ( p[0], 4 );
  EXPECT_EQ( p[1], 5 );
  EXPECT_EQ( p[2], 3 );

  p.assign( v1.begin(), v1.end() );
  EXPECT_EQ( p[0], 6 );
  EXPECT_EQ( p[1], 5 );
  EXPECT_EQ( p[2], 3 );

  p.set( 7, 8, 9 );
  EXPECT_EQ( p[0], 7 );
  EXPECT_EQ( p[1], 8 );
  EXPECT_EQ( p[2], 9 );

  point_t<int,3> p2 = { 1, 2, 3 };
  p = p2;
  EXPECT_EQ( p[0], 1 );
  EXPECT_EQ( p[1], 2 );
  EXPECT_EQ( p[2], 3 );
}

TEST( PointTest, Iterator )
{
  point_t<int, 4> p;

  int i = 0;
  for( auto &x : p )
  {
    x = i++;
  }
  EXPECT_EQ( p[0], 0 );
  EXPECT_EQ( p[1], 1 );
  EXPECT_EQ( p[2], 2 );
  EXPECT_EQ( p[3], 3 );
}