#include <gtest/gtest.h>
#include <RTree.hpp>
#include <random>

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
TEST( BoundTest, HitTest )
{
  ibound_type bound( 10, 20 );
  for( int i=10; i<=20; ++i )
  {
    EXPECT_TRUE( bound.is_inside(i) ) << i;
    EXPECT_TRUE( bound.is_overlap(i) ) << i;
  }
  EXPECT_FALSE( bound.is_inside(9) );
  EXPECT_FALSE( bound.is_inside(21) );

  std::mt19937 mt;
  std::uniform_int_distribution<int> dist( 0, 30 );
  for( int i=0; i<500; ++i )
  {
    int m = dist(mt);
    int M = dist(mt);
    if( m > M ){ std::swap(m,M); }

    bool inside = (m >= 10 && M <=20);
    bool overlap = false;
    for( int j=m; j<=M; ++j )
    {
      if( bound.is_inside(j) ){ overlap = true; break; }
    }

    ASSERT_EQ( bound.is_inside(ibound_type(m,M)), inside ) << "[" << m << ", " << M << "]";
    ASSERT_EQ( bound.is_overlap(ibound_type(m,M)), overlap ) << "[" << m << ", " << M << "]";
  }
}
TEST( BoundTest, HitTestf )
{
  fbound_type bound( 10, 20 );
  for( int i=11; i<=19; ++i )
  {
    EXPECT_TRUE( bound.is_inside(i) ) << i;
    EXPECT_TRUE( bound.is_overlap(i) ) << i;
  }
  EXPECT_TRUE( bound.is_inside(10+1e-6f) );
  EXPECT_TRUE( bound.is_inside(20-1e-6f) );
  EXPECT_FALSE( bound.is_inside(10-1e-6f) );
  EXPECT_FALSE( bound.is_inside(20+1e-6f) );

  std::mt19937 mt;
  std::uniform_real_distribution<float> dist( 0, 30 );
  for( int i=0; i<500; ++i )
  {
    float m = dist(mt);
    float M = dist(mt);
    if( m > M ){ std::swap(m,M); }

    bool inside = (m >= 10 && M <=20);
    bool overlap = !((M<10)||(m>20));

    ASSERT_EQ( bound.is_inside(fbound_type(m,M)), inside ) << "[" << m << ", " << M << "]";
    ASSERT_EQ( bound.is_overlap(fbound_type(m,M)), overlap ) << "[" << m << ", " << M << "]";
  }
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

TEST( BoundTest, Mergef )
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