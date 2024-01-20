#include <gtest/gtest.h>

#include <RTree.hpp>
#include <vector>
#include <algorithm>
#include <random>

namespace er = eh::rtree;

TEST( RTreeTest, Insert )
{
  using rtree_type = er::RTree<er::aabb_t<int>,er::aabb_t<int>,int>;
  using traits = rtree_type::traits;
  using bound_type = rtree_type::bound_type;
  using node_type = rtree_type::node_type;


  std::mt19937 mt( std::random_device{}() );
  std::uniform_int_distribution<int> dist( -1000, 1000 );
  rtree_type rtree;

  for( int i=0; i<1000; ++i )
  {
    int min_ = dist(mt);
    int max_ = dist(mt);
    if( max_ < min_ ){ std::swap(min_,max_); }
    rtree.insert( {{min_,max_}, i} );

    // entries count check
    for( int level=0; level<rtree.leaves_level(); ++level )
    {
      for( auto ni=rtree.begin(level); ni!=rtree.end(level); ++ni )
      {
        if( level != 0 )
        {
          ASSERT_GE( ni->size(), rtree_type::MIN_ENTRIES ) << "level " << level << " below MIN_ENTRIES";
        }
        ASSERT_LE( ni->size(), rtree_type::MAX_ENTRIES ) << "level " << level << " above MAX_ENTRIES";
      }
    }
    for( auto ni=rtree.leaf_begin(); ni!=rtree.leaf_end(); ++ni )
    {
      if( rtree.leaves_level() != 0 )
      {
        ASSERT_GE( ni->size(), rtree_type::MIN_ENTRIES ) << "leaf below MIN_ENTRIES";
      }
      ASSERT_LE( ni->size(), rtree_type::MAX_ENTRIES ) << "leaf above MAX_ENTRIES";
    }
    // entries bound check
    for( int level=0; level<rtree.leaves_level(); ++level )
    {
      for( auto ni=rtree.begin(level); ni!=rtree.end(level); ++ni )
      {
        auto *node = *ni;
        for( auto &c : *node )
        {
          ASSERT_EQ( c.second->parent(), node );
          auto a = c.first;
          if( level+1==rtree.leaves_level() )
          {
            auto b = reinterpret_cast<rtree_type::leaf_type*>(c.second)->calculate_bound();
            ASSERT_TRUE( traits::is_inside(a,b) ) << "i: " << i << ", level: " << level;
          }else {
            auto b = reinterpret_cast<node_type*>(c.second)->calculate_bound();
            ASSERT_TRUE( traits::is_inside(a,b) ) << "i: " << i << ", level: " << level;
          }
        }
      }
    }

    // all data valid check
    std::vector<bool> valid( i+1, false );
    for( auto x : rtree )
    {
      ASSERT_FALSE( valid[x.second] ) << "i: " << i << ", " << "x: " << x.second << " already exist";
      valid[x.second] = true;
    }
    for( int x=0; x<=i; ++x )
    {
      ASSERT_TRUE( valid[x] ) << "i: " << i << ", " << "x: " << x << " not exist";
    }
  }
}

TEST( RTreeTest, Erase )
{
  using rtree_type = er::RTree<er::aabb_t<int>,er::aabb_t<int>,int>;
  using traits = rtree_type::traits;
  using bound_type = rtree_type::bound_type;
  using node_type = rtree_type::node_type;


  std::mt19937 mt( std::random_device{}() );
  std::uniform_int_distribution<int> dist( -1000, 1000 );
  rtree_type rtree;

  for( int i=0; i<1000; ++i )
  {
    int min_ = dist(mt);
    int max_ = dist(mt);
    if( max_ < min_ ){ std::swap(min_,max_); }
    rtree.insert( {{min_,max_}, i} );
  }

  std::vector<bool> data_inserted( 1000, true );
  for( int i=0; i<1000; ++i )
  {
    const int cur_size = 1000-i;
    int erase_index = std::uniform_int_distribution<int>( 0, cur_size-1 )( mt );
    auto it = rtree.begin();
    std::advance( it, erase_index );
    ASSERT_GE( it->second, 0 );
    ASSERT_LE( it->second, 1000 );
    ASSERT_TRUE( data_inserted[it->second] ) << it->second << ": already deleted";
    data_inserted[it->second] = false;
    rtree.erase( it );

    // entries count check
    for( int level=0; level<rtree.leaves_level(); ++level )
    {
      for( auto ni=rtree.begin(level); ni!=rtree.end(level); ++ni )
      {
        if( level != 0 )
        {
          ASSERT_GE( ni->size(), rtree_type::MIN_ENTRIES ) << "level " << level << " below MIN_ENTRIES";
        }
        ASSERT_LE( ni->size(), rtree_type::MAX_ENTRIES ) << "level " << level << " above MAX_ENTRIES";
      }
    }
    for( auto ni=rtree.leaf_begin(); ni!=rtree.leaf_end(); ++ni )
    {
      if( rtree.leaves_level() != 0 )
      {
        ASSERT_GE( ni->size(), rtree_type::MIN_ENTRIES ) << "leaf below MIN_ENTRIES";
      }
      ASSERT_LE( ni->size(), rtree_type::MAX_ENTRIES ) << "leaf above MAX_ENTRIES";
    }

    // entries bound check
    for( int level=0; level<rtree.leaves_level(); ++level )
    {
      for( auto ni=rtree.begin(level); ni!=rtree.end(level); ++ni )
      {
        auto *node = *ni;
        for( auto &c : *node )
        {
          ASSERT_EQ( c.second->parent(), node );
          auto a = c.first;
          if( level+1==rtree.leaves_level() )
          {
            auto b = reinterpret_cast<rtree_type::leaf_type*>(c.second)->calculate_bound();
            ASSERT_TRUE( traits::is_inside(a,b) ) << "i: " << i << ", level: " << level;
          }else {
            auto b = reinterpret_cast<node_type*>(c.second)->calculate_bound();
            ASSERT_TRUE( traits::is_inside(a,b) ) << "i: " << i << ", level: " << level;
          }
        }
      }
    }
    ASSERT_EQ( std::distance(rtree.begin(),rtree.end()), cur_size-1 ) << "distance(tree.iterator) is not equal to " << i+1;
  }
  for( int i=0; i<1000; ++i )
  {
    ASSERT_FALSE( data_inserted[i] ) << "data " << i << " exist after deletion";
  }
}