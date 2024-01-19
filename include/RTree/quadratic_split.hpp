#pragma once

#include <limits>
#include <utility>
#include <iterator>

#include "global.hpp"

namespace eh { namespace rtree {

// quadratic split algorithm
template < typename BoundType >
struct quadratic_split_t
{
  using bound_type = BoundType;
  using area_type = typename bound_type::area_type;
  constexpr static area_type LOWEST_AREA = std::numeric_limits<area_type>::lowest();

  unsigned int MIN_ENTRIES = 4;
  unsigned int MAX_ENTRIES = 8;

  template < typename NodeType >
  std::pair<typename NodeType::iterator,typename NodeType::iterator>
  pick_seed( NodeType *node ) const
  {
    /*
    PS1. [Calculate inefficiency of grouping entries together.]
    For each pair of entries E1 and E2, compose a rectangle J including E1.I and E2.I. 
    Calculate d = area(J) - area(E1.I) - area(E2.I)

    PS2. [Choose the most wasteful pair.]
    Choose the pair with the largest d.
    */
    typename NodeType::iterator n1 = node->end(), n2;
    area_type max_wasted_area = LOWEST_AREA;

    // choose two nodes that would waste the most area if both were put in the same group
    for( auto ci=node->begin(); ci!=node->end()-1; ++ci )
    {
      for( auto cj=ci+1; cj!=node->end(); ++cj )
      {
        const auto J = ci->first.merged( cj->first );
        const area_type wasted_area = J.area() - ci->first.area() - cj->first.area();

        if( wasted_area > max_wasted_area )
        {
          max_wasted_area = wasted_area;
          n1 = ci;
          n2 = cj;
        }
        // if same wasted area, choose pair with small intersection area
        else if( wasted_area == max_wasted_area )
        {
          if( ci->first.intersection(cj->first).area() < n1->first.intersection(n2->first).area() )
          {
            n1 = ci;
            n2 = cj;
          }
        }
      }
    }
    assert( n1 != node->end() );

    return { n1, n2 };
  }

  template < typename NodeType >
  NodeType* operator()( NodeType *node ) const
  {
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
    std::vector<typename NodeType::value_type> entry1, entry2;

    auto seeds = pick_seed( node );
    entry1.push_back( std::move(*seeds.first) );
    entry2.push_back( std::move(*seeds.second) );
    bound_type bound1 = entry1.front().first;
    bound_type bound2 = entry2.front().first;
    node->_child.erase( seeds.second );
    node->_child.erase( seeds.first );

    while( node->_child.empty() == false )
    {
      /*
      PN1. [Determine cost of putting each entry in each group.]
      For each entry E not yet in a group, 
      calculate d1 = the area increase required in the covering rectangle of Group 1 to include E.I. 
      Calculate d2  similarly for Group 2.

      PN2. [Find entry with greatest preference for one group.]
      Choose any entry with the maximum difference between d1 and d2.
      */

      const auto old_node_count = node->size();
      if( entry1.size() + old_node_count == MIN_ENTRIES )
      {
        entry1.insert(
          entry1.end(),
          std::make_move_iterator(node->begin()),
          std::make_move_iterator(node->end())
        );
        node->_child.clear();
      }else if( entry2.size() + old_node_count == MIN_ENTRIES )
      {
        entry2.insert(
          entry2.end(),
          std::make_move_iterator(node->begin()),
          std::make_move_iterator(node->end())
        );
        node->_child.clear();
      }else {
        typename NodeType::iterator picked = node->end();
        int picked_to = 0;
        area_type maximum_difference = LOWEST_AREA;

        for( auto ci=node->begin(); ci!=node->end(); ++ci )
        {
          const area_type d1 = bound1.merged( ci->first ).area() - bound1.area();
          const area_type d2 = bound2.merged( ci->first ).area() - bound2.area();
          const auto diff = std::abs(d1 - d2);

          if( diff > maximum_difference )
          {
            picked = ci;
            maximum_difference = diff;
            picked_to = d1 < d2 ? 0 : 1;
          }
        }

        if( picked_to == 0 )
        {
          entry1.push_back( std::move(*picked) );
          bound1 = bound1.merged( entry1.back().first );
        }else {
          entry2.push_back( std::move(*picked) );
          bound2 = bound2.merged( entry2.back().first );
        }

        node->_child.erase( picked );
      }
    }

    for( auto &e1 : entry1 )
    {
      node->insert( std::move(e1) );
    }
    NodeType *node_pair = new NodeType;
    for( auto &e2 : entry2 )
    {
      node_pair->insert( std::move(e2) );
    }

    return node_pair;
  }
};

}} // namespace eh rtree