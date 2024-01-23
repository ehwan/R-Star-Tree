#pragma once

#include <limits>
#include <utility>
#include <iterator>
#include <cassert>
#include <vector>

#include "global.hpp"
#include "geometry_traits.hpp"

namespace eh { namespace rtree {

// quadratic split algorithm
template < typename TreeType >
struct quadratic_split_t
{
  using geometry_type = typename TreeType::geometry_type;
  using traits = geometry_traits<geometry_type>;
  using area_type = typename geometry_traits<geometry_type>::area_type;
  constexpr static area_type LOWEST_AREA = std::numeric_limits<area_type>::lowest();

  constexpr static unsigned int MIN_ENTRIES = TreeType::MIN_ENTRIES;
  constexpr static unsigned int MAX_ENTRIES = TreeType::MAX_ENTRIES;

  template < typename ValueType >
  std::pair<ValueType,ValueType>
  pick_seed( std::vector<ValueType> &nodes ) const
  {
    /*
    PS1. [Calculate inefficiency of grouping entries together.]
    For each pair of entries E1 and E2, compose a rectangle J including E1.I and E2.I. 
    Calculate d = area(J) - area(E1.I) - area(E2.I)

    PS2. [Choose the most wasteful pair.]
    Choose the pair with the largest d.
    */
    int n1 = 0, n2 = 0;
    area_type max_wasted_area = LOWEST_AREA;

    // choose two nodes that would waste the most area if both were put in the same group
    for( int i=0; i<nodes.size()-1; ++i )
    {
      for( int j=i+1; j<nodes.size(); ++j )
      {
        const auto J = traits::merge(nodes[i].first, nodes[j].first);
        const area_type wasted_area = traits::area(J) - traits::area(nodes[i].first) - traits::area(nodes[j].first);

        if( wasted_area > max_wasted_area )
        {
          max_wasted_area = wasted_area;
          n1 = i;
          n2 = j;
        }
        // if same wasted area, choose pair with small intersection area
        else if( wasted_area == max_wasted_area )
        {
          if( traits::area(traits::intersection(nodes[i].first,nodes[j].first)) <
              traits::area(traits::intersection(nodes[n1].first,nodes[n2].first)) )
          {
            n1 = i;
            n2 = j;
          }
        }
      }
    }

    auto ret1 = std::move( nodes[n1] );
    auto ret2 = std::move( nodes[n2] );
    if( n1 > n2 )
    {
      std::swap( n1, n2 );
    }
    if( n2 < nodes.size()-1 )
    {
      nodes[n2] = std::move( nodes.back() );
    }
    nodes.pop_back();
    if( n1 < nodes.size()-1 )
    {
      nodes[n1] = std::move( nodes.back() );
    }
    nodes.pop_back();

    return { std::move(ret1), std::move(ret2) };
  }

  template < typename NodeType >
  NodeType* operator()( NodeType *node, typename NodeType::value_type child, NodeType *node_pair ) const
  {
    assert( node->size() == MAX_ENTRIES );
    assert( node_pair );
    assert( node_pair->size() == 0 );
    /*
    QS1. [Pick first entry for each group.]
    Apply Algorithm PickSeeds to choose two entries to be the first elements of the groups. 
    Assign each to a group.

    QS2. [Check if done.]
    If all entries have been assigned, stop. 
    If one group has so few entries that all the rest must
    be assigned to it in order for it to have the minimum number m, assign them and stop.

    QS3. [Select entry to assign.]
    Invoke Algorithm PickNext to choose the next entry to assign. 
    Add it to the group whose covering rectangle will have to be enlarged least to accommodate it.
    Resolve ties by adding the entry to the group with smaller area,
    then to the one with fewer entries, then to either. 
    Repeat from QS2.
    */
    std::vector<typename NodeType::value_type> childs = 
      { std::make_move_iterator(node->begin()), std::make_move_iterator(node->end()) };
    node->clear();
    childs.emplace_back( std::move(child) );

    auto seeds = pick_seed( childs );
    geometry_type bound1 = seeds.first.first;
    geometry_type bound2 = seeds.second.first;

    node->insert( std::move(seeds.first) );
    node_pair->insert( std::move(seeds.second) );

    while( childs.empty() == false )
    {
      /*
      PN1. [Determine cost of putting each entry in each group.]
      For each entry E not yet in a group, 
      calculate d1 = the area increase required in the covering rectangle of Group 1 to include E.I. 
      Calculate d2  similarly for Group 2.

      PN2. [Find entry with greatest preference for one group.]
      Choose any entry with the maximum difference between d1 and d2.
      */
      if( node->size() + childs.size() == MIN_ENTRIES )
      {
        for( auto &c : childs )
        {
          node->insert( std::move(c) );
        }
        childs.clear();
      }else if( node_pair->size() + childs.size() == MIN_ENTRIES )
      {
        for( auto &c : childs )
        {
          node_pair->insert( std::move(c) );
        }
        childs.clear();
      }else
      {
        int picked = 0;
        int picked_to = 0;
        area_type maximum_difference = LOWEST_AREA;

        for( int i=0; i<childs.size(); ++i )
        {
          const area_type d1 = 
            traits::area( traits::merge(bound1,childs[i].first) ) - traits::area(bound1);
          const area_type d2 = 
            traits::area( traits::merge(bound2,childs[i].first) ) - traits::area(bound2);
          const auto diff = std::abs(d1 - d2);

          if( diff > maximum_difference )
          {
            picked = i;
            maximum_difference = diff;
            picked_to = d1 < d2 ? 0 : 1;
          }
        }

        if( picked_to == 0 )
        {
          bound1 = traits::merge( bound1, childs[picked].first );
          node->insert( std::move(childs[picked]) );
        }else {
          bound2 = traits::merge( bound2, childs[picked].first );
          node_pair->insert( std::move(childs[picked]) );
        }

        if( picked < childs.size()-1 )
        {
          childs[picked] = std::move( childs.back() );
        }
        childs.pop_back();
      }
    }
    return node_pair;
  }
};

}} // namespace eh rtree