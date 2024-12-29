#pragma once

#include <cmath>
#include <limits>
#include <utility>

#include "geometry_traits.hpp"
#include "global.hpp"

namespace eh
{
namespace rtree
{

// quadratic split algorithm
struct QuadraticSplit
{
  template <typename NodeType>
  static NodeType* split(NodeType* node,
                         typename NodeType::value_type new_child,
                         NodeType* node_pair)
  {
    using geometry_type = typename NodeType::geometry_type;
    using traits = geometry_traits<geometry_type>;

    EH_RTREE_ASSERT_SILENT(node->size() == NodeType::MAX_ENTRIES);
    EH_RTREE_ASSERT_SILENT(node_pair);
    EH_RTREE_ASSERT_SILENT(node_pair->size() == 0);

    /*
    QS1. [Pick first entry for each group.]
    Apply Algorithm PickSeeds to choose two entries to be the first elements of
    the groups. Assign each to a group.

    QS2. [Check if done.]
    If all entries have been assigned, stop.
    If one group has so few entries that all the rest must
    be assigned to it in order for it to have the minimum number m, assign them
    and stop.

    QS3. [Select entry to assign.]
    Invoke Algorithm PickNext to choose the next entry to assign.
    Add it to the group whose covering rectangle will have to be enlarged least
    to accommodate it. Resolve ties by adding the entry to the group with
    smaller area, then to the one with fewer entries, then to either. Repeat
    from QS2.
    */

    /*
    ************************** Peek Seeds **************************
    PS1. [Calculate inefficiency of grouping entries together.]
    For each pair of entries E1 and E2, compose a rectangle J including E1.I and
    E2.I. Calculate d = area(J) - area(E1.I) - area(E2.I)

    PS2. [Choose the most wasteful pair.]
    Choose the pair with the largest d.
    */
    {
      int n1 = 0, n2 = 0;
      typename traits::scalar_type max_wasted_area
          = std::numeric_limits<typename traits::scalar_type>::lowest();

      // choose two nodes that would waste the most area if both were put in the
      // same group for each pair (i,j) such that ( i in *node.child AND j in
      // *node.child AND i < j )
      for (int i = 0; i < node->size() - 1; ++i)
      {
        for (int j = i + 1; j < node->size(); ++j)
        {
          const typename traits::scalar_type J
              = helper::enlarged_area(node->at(i).first, node->at(j).first);
          const typename traits::scalar_type wasted_area
              = J - helper::area(node->at(i).first)
                - helper::area(node->at(j).first);

          if (wasted_area > max_wasted_area)
          {
            max_wasted_area = wasted_area;
            n1 = i;
            n2 = j;
          }
          // if same wasted area, choose pair with small intersection area
          else if (wasted_area == max_wasted_area)
          {
            if (helper::intersection_area(node->at(i).first, node->at(j).first)
                < helper::intersection_area(node->at(n1).first,
                                            node->at(n2).first))
            {
              n1 = i;
              n2 = j;
            }
          }
        }
      }

      // for each pair (i,j) such that ( i is new_child AND j in *node.child )
      for (int j = 0; j < node->size(); ++j)
      {
        const typename traits::scalar_type J
            = helper::enlarged_area(new_child.first, node->at(j).first);
        const typename traits::scalar_type wasted_area
            = J - helper::area(new_child.first)
              - helper::area(node->at(j).first);

        if (wasted_area > max_wasted_area)
        {
          max_wasted_area = wasted_area;
          n1 = -1;
          n2 = j;
        }
        // if same wasted area, choose pair with small intersection area
        else if (wasted_area == max_wasted_area)
        {
          if (n1 == -1)
          {
            if (helper::intersection_area(new_child.first, node->at(j).first)
                < helper::intersection_area(new_child.first,
                                            node->at(n2).first))
            {
              n1 = -1;
              n2 = j;
            }
          }
          else
          {
            if (helper::intersection_area(new_child.first, node->at(j).first)
                < helper::intersection_area(node->at(n1).first,
                                            node->at(n2).first))
            {
              n1 = -1;
              n2 = j;
            }
          }
        }
      }

      if (n1 == -1)
      {
        // insert
        // n1 -> node_pair[0]
        // n2 -> node[0]
        node_pair->insert(std::move(new_child));
        if (n2 != 0)
        {
          node->swap(0, n2);
        }
      }
      else
      {
        // Note.
        // n1 is still valid after erase() since n1 < n2
        auto n2_data = std::move(node->at(n2));
        node->erase(node->begin() + n2);
        node_pair->insert(std::move(n2_data));
        if (n1 != 0)
        {
          node->swap(0, n1);
        }
        node->insert(std::move(new_child));
      }
    }
    // ************************* Peek Seeds End *************************

    typename NodeType::size_type count1 = 1;
    geometry_type bound1 = node->at(0).first;
    geometry_type bound2 = node_pair->at(0).first;

    while (count1 + node_pair->size() < NodeType::MAX_ENTRIES + 1)
    {
      /*
      PN1. [Determine cost of putting each entry in each group.]
      For each entry E not yet in a group,
      calculate d1 = the area increase required in the covering rectangle of
      Group 1 to include E.I. Calculate d2  similarly for Group 2.

      PN2. [Find entry with greatest preference for one group.]
      Choose any entry with the maximum difference between d1 and d2.
      */
      const typename NodeType::size_type node_left
          = NodeType::MAX_ENTRIES + 1 - (count1 + node_pair->size());
      if (count1 + node_left == NodeType::MIN_ENTRIES)
      {
        break;
      }
      else if (node_pair->size() + node_left == NodeType::MIN_ENTRIES)
      {
        for (typename NodeType::size_type i = 0; i < node_left; ++i)
        {
          auto back_data = std::move(node->back());
          node->pop_back();
          node_pair->insert(std::move(back_data));
        }
        break;
      }
      else
      {
        int picked = 0;
        int picked_to = 0;
        typename traits::scalar_type maximum_difference
            = std::numeric_limits<typename traits::scalar_type>::lowest();

        for (typename NodeType::size_type i = count1; i < node->size(); ++i)
        {
          const typename traits::scalar_type d1
              = helper::enlarged_area(bound1, node->at(i).first)
                - helper::area(bound1);
          const typename traits::scalar_type d2
              = helper::enlarged_area(bound2, node->at(i).first)
                - helper::area(bound2);
          const auto diff = std::abs(d1 - d2);

          if (diff > maximum_difference)
          {
            picked = i;
            maximum_difference = diff;
            picked_to = d1 < d2 ? 0 : 1;
          }
        }

        if (picked_to == 0)
        {
          helper::enlarge_to(bound1, node->at(picked).first);
          if (picked != count1)
          {
            node->swap(count1, picked);
          }
          ++count1;
        }
        else
        {
          helper::enlarge_to(bound2, node->at(picked).first);
          auto picked_data = std::move(node->at(picked));
          node->erase(node->begin() + picked);
          node_pair->insert(std::move(picked_data));
        }
      }
    }
    return node_pair;
  }
};

}
} // namespace eh rtree