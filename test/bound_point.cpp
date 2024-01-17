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

TEST( BoundPTest, Merge )
{
  ibound2 bound0 = { {0,0}, {10,10} };

  auto merged = bound0.merged( {{-1,-2},{3,5}} );
  EXPECT_EQ( merged.min_bound()[0], -1 );
  EXPECT_EQ( merged.min_bound()[1], -2 );
  EXPECT_EQ( merged.max_bound()[0], 10 );
  EXPECT_EQ( merged.max_bound()[1], 10 );

  merged = bound0.merged( {{5,5}, {12, 14}} );
  EXPECT_EQ( merged.min_bound()[0], 0 );
  EXPECT_EQ( merged.min_bound()[1], 0 );
  EXPECT_EQ( merged.max_bound()[0], 12 );
  EXPECT_EQ( merged.max_bound()[1], 14 );

  merged = bound0.merged( {{-1,3},{11,5}} );
  EXPECT_EQ( merged.min_bound()[0], -1 );
  EXPECT_EQ( merged.min_bound()[1], 0 );
  EXPECT_EQ( merged.max_bound()[0], 11 );
  EXPECT_EQ( merged.max_bound()[1], 10 );

  merged = bound0.merged( {{3,-1},{5,12}} );
  EXPECT_EQ( merged.min_bound()[0], 0 );
  EXPECT_EQ( merged.min_bound()[1], -1 );
  EXPECT_EQ( merged.max_bound()[0], 10 );
  EXPECT_EQ( merged.max_bound()[1], 12 );

  merged = bound0.merged( {{1,2}, {3,4}} );
  EXPECT_EQ( merged.min_bound()[0], 0 );
  EXPECT_EQ( merged.min_bound()[1], 0 );
  EXPECT_EQ( merged.max_bound()[0], 10 );
  EXPECT_EQ( merged.max_bound()[1], 10 );
}

TEST( BoundPTest, IsInside )
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

  EXPECT_FALSE( inside1.is_inside(bound0) );
  EXPECT_FALSE( inside2.is_inside(bound0) );
  EXPECT_FALSE( inside3.is_inside(bound0) );
  EXPECT_FALSE( inside4.is_inside(bound0) );
  EXPECT_FALSE( inside5.is_inside(bound0) );
  EXPECT_TRUE( inside6.is_inside(bound0) );

  ibound2 left_bottom = { {-2,-3}, {3, 4} };
  ibound2 left_top = { {-2, 4}, {3, 12} };
  ibound2 right_top = { {5, 4}, {12, 12} };
  ibound2 right_bottom = { {5, -3}, {12, 4} };

  EXPECT_FALSE( bound0.is_inside(left_bottom) );
  EXPECT_FALSE( bound0.is_inside(left_top) );
  EXPECT_FALSE( bound0.is_inside(right_bottom) );
  EXPECT_FALSE( bound0.is_inside(right_top) );
  
  EXPECT_FALSE( bound0.is_inside(left_bottom.merged(left_top)) );
  EXPECT_FALSE( bound0.is_inside(right_bottom.merged(right_top)) );
}

TEST( BoundPTest, IsOverlap )
{
  ibound2 bound0 = { {0,0}, {10,10} };

  ibound2 inside1 = { {0,3}, {5,4} };
  ibound2 inside2 = { {1,2}, {4,6} };
  ibound2 inside3 = { {5,3}, {10,5} };
  ibound2 inside4 = { {1,0}, {4,5} };
  ibound2 inside5 = { {1,4}, {5,10} };
  ibound2 inside6 = bound0;

  EXPECT_TRUE( bound0.is_overlap(inside1) );
  EXPECT_TRUE( bound0.is_overlap(inside2) );
  EXPECT_TRUE( bound0.is_overlap(inside3) );
  EXPECT_TRUE( bound0.is_overlap(inside4) );
  EXPECT_TRUE( bound0.is_overlap(inside5) );
  EXPECT_TRUE( bound0.is_overlap(inside6) );

  EXPECT_TRUE( inside1.is_overlap(bound0) );
  EXPECT_TRUE( inside2.is_overlap(bound0) );
  EXPECT_TRUE( inside3.is_overlap(bound0) );
  EXPECT_TRUE( inside4.is_overlap(bound0) );
  EXPECT_TRUE( inside5.is_overlap(bound0) );
  EXPECT_TRUE( inside6.is_overlap(bound0) );

  ibound2 left_bottom = { {-2,-3}, {3, 4} };
  ibound2 left_top = { {-2, 4}, {3, 12} };
  ibound2 right_top = { {5, 4}, {12, 12} };
  ibound2 right_bottom = { {5, -3}, {12, 4} };

  EXPECT_TRUE( bound0.is_overlap(left_bottom) );
  EXPECT_TRUE( bound0.is_overlap(left_top) );
  EXPECT_TRUE( bound0.is_overlap(right_bottom) );
  EXPECT_TRUE( bound0.is_overlap(right_top) );
  
  EXPECT_TRUE( bound0.is_overlap(left_bottom.merged(left_top)) );
  EXPECT_TRUE( bound0.is_overlap(right_bottom.merged(right_top)) );
}

TEST( BoundPTest, Intersection )
{
  ibound2 bound0 = { {0,0}, {10,10} };

  ibound2 inside1 = { {0,3}, {5,4} };
  ibound2 inside2 = { {1,2}, {4,6} };
  ibound2 inside3 = { {5,3}, {10,5} };
  ibound2 inside4 = { {1,0}, {4,5} };
  ibound2 inside5 = { {1,4}, {5,10} };
  ibound2 inside6 = bound0;

  auto inter = bound0.intersection( inside1 );
  EXPECT_EQ( inter.min_bound()[0], inside1.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], inside1.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], inside1.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], inside1.max_bound()[1] );
  inter = bound0.intersection( inside2 );
  EXPECT_EQ( inter.min_bound()[0], inside2.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], inside2.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], inside2.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], inside2.max_bound()[1] );
  inter = bound0.intersection( inside3 );
  EXPECT_EQ( inter.min_bound()[0], inside3.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], inside3.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], inside3.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], inside3.max_bound()[1] );
  inter = bound0.intersection( inside4 );
  EXPECT_EQ( inter.min_bound()[0], inside4.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], inside4.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], inside4.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], inside4.max_bound()[1] );
  inter = bound0.intersection( inside5 );
  EXPECT_EQ( inter.min_bound()[0], inside5.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], inside5.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], inside5.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], inside5.max_bound()[1] );
  inter = bound0.intersection( inside6 );
  EXPECT_EQ( inter.min_bound()[0], inside6.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], inside6.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], inside6.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], inside6.max_bound()[1] );

  ibound2 left_bottom = { {-2,-3}, {3, 4} };
  ibound2 left_top = { {-2, 4}, {3, 12} };
  ibound2 right_top = { {5, 4}, {12, 12} };
  ibound2 right_bottom = { {5, -3}, {12, 4} };

  inter = bound0.intersection( left_bottom );
  EXPECT_EQ( inter.min_bound()[0], 0 );
  EXPECT_EQ( inter.min_bound()[1], 0 );
  EXPECT_EQ( inter.max_bound()[0], left_bottom.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], left_bottom.max_bound()[1] );

  inter = bound0.intersection( left_top );
  EXPECT_EQ( inter.min_bound()[0], 0 );
  EXPECT_EQ( inter.min_bound()[1], left_top.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], left_top.max_bound()[0] );
  EXPECT_EQ( inter.max_bound()[1], 10 );

  inter = bound0.intersection( right_bottom );
  EXPECT_EQ( inter.min_bound()[0], right_bottom.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], 0 );
  EXPECT_EQ( inter.max_bound()[0], 10 );
  EXPECT_EQ( inter.max_bound()[1], right_bottom.max_bound()[1] );

  inter = bound0.intersection( right_top );
  EXPECT_EQ( inter.min_bound()[0], right_top.min_bound()[0] );
  EXPECT_EQ( inter.min_bound()[1], right_top.min_bound()[1] );
  EXPECT_EQ( inter.max_bound()[0], 10 );
  EXPECT_EQ( inter.max_bound()[1], 10 );

  EXPECT_EQ( bound0.intersection( {{-10,-10},{-5,-5}} ).area(), 0 );
}