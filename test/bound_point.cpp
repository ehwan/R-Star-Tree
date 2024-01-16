#include <gtest/gtest.h>
#include <RTree.hpp>

using ipoint2 = eh::rtree::point_t<int,2>;
using ibound2 = eh::rtree::bound_t< ipoint2 >;

TEST( BoundPTest, Init )
{
  ipoint2 p0 = { 0, 1 };
  ipoint2 p1 = { 3, 4 };

  ibound2 bound = { p0, p1 };
  EXPECT_EQ( bound.min_bound()[0], p0[0] );
  EXPECT_EQ( bound.min_bound()[1], p0[1] );
  EXPECT_EQ( bound.max_bound()[0], p1[0] );
  EXPECT_EQ( bound.max_bound()[1], p1[1] );

  ibound2 bound2 = bound;
  EXPECT_EQ( bound2.min_bound()[0], p0[0] );
  EXPECT_EQ( bound2.min_bound()[1], p0[1] );
  EXPECT_EQ( bound2.max_bound()[0], p1[0] );
  EXPECT_EQ( bound2.max_bound()[1], p1[1] );
}

TEST( BoundPTest, Area )
{
  ibound2 bound( {1,2}, {4,6} );
  EXPECT_EQ( bound.area(), 3*4 );

  ibound2 no_width = { {1,4}, {1,10} };
  EXPECT_EQ( no_width.area(), 0 );

  const ibound2 no_height = { {1,5}, {10,5} };
  EXPECT_EQ( no_height.area(), 0 );
}

TEST( BoundPTest, HitTest )
{
  ibound2 bound0 = { {0,0}, {10,10} };

  ibound2 inside1 = { {0,3}, {5,4} };
  ibound2 inside2 = { {1,2}, {4,6} };
  ibound2 inside3 = { {5,3}, {10,5} };
  ibound2 inside4 = { {1,0}, {4,5} };
  ibound2 inside5 = { {1,4}, {5,10} };
  ibound2 inside6 = bound0;

  EXPECT_TRUE( bound0.is_inside(inside1) );
  EXPECT_TRUE( bound0.is_inside(inside2) );
  EXPECT_TRUE( bound0.is_inside(inside3) );
  EXPECT_TRUE( bound0.is_inside(inside4) );
  EXPECT_TRUE( bound0.is_inside(inside5) );
  EXPECT_TRUE( bound0.is_inside(inside6) );

  EXPECT_TRUE( bound0.is_overlap(inside1) );
  EXPECT_TRUE( bound0.is_overlap(inside2) );
  EXPECT_TRUE( bound0.is_overlap(inside3) );
  EXPECT_TRUE( bound0.is_overlap(inside4) );
  EXPECT_TRUE( bound0.is_overlap(inside5) );
  EXPECT_TRUE( bound0.is_overlap(inside6) );

  // TODO more...
}