#pragma once

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

#include "geometry_traits.hpp"
#include "global.hpp"

namespace eh
{
namespace rtree
{

// rstar-tree split algorithm
struct RStarSplit
{

  template <typename NodeType>
  static NodeType* split(NodeType* node,
                         typename NodeType::value_type new_child,
                         NodeType* node_pair)
  {
    using geometry_type = typename NodeType::geometry_type;
    using traits = geometry_traits<geometry_type>;
    using scalar_type = typename traits::scalar_type;

    EH_RTREE_ASSERT_SILENT(node->size() == NodeType::MAX_ENTRIES);
    EH_RTREE_ASSERT_SILENT(node_pair);
    EH_RTREE_ASSERT_SILENT(node_pair->size() == 0);

    // MAX_ENTRIES+1 nodes
    std::vector<typename NodeType::value_type> entries;
    entries.reserve(NodeType::MAX_ENTRIES + 1);
    for (int i = 0; i < NodeType::MAX_ENTRIES; ++i)
    {
      entries.emplace_back(std::move(node->at(i)));
    }
    entries.emplace_back(std::move(new_child));
    node->clear();

    int choose_axis = -1;
    scalar_type min_overlap = std::numeric_limits<scalar_type>::max();
    scalar_type min_margin = std::numeric_limits<scalar_type>::max();

    // choose axis
    for (int axis = 0; axis < traits::DIM; ++axis)
    {
      // sort along axis
      std::sort(entries.begin(), entries.end(),
                [axis](typename NodeType::value_type const& a,
                       typename NodeType::value_type const& b)
                {
                  if (helper::min_point(a.first, axis)
                      < helper::min_point(b.first, axis))
                    return true;
                  else if (helper::min_point(a.first, axis)
                           == helper::min_point(b.first, axis))
                    return helper::max_point(a.first, axis)
                           < helper::max_point(b.first, axis);
                  else
                    return false;
                });

      // split MAX_ENTRIES+1 nodes into two groups;
      // [0, k) and [k, MAX_ENTRIES]
      for (int k = NodeType::MIN_ENTRIES;
           k <= NodeType::MAX_ENTRIES - NodeType::MIN_ENTRIES + 1; ++k)
      {
        // calculate bounding box
        geometry_type mbr1 = entries[0].first;
        for (int i = 1; i < k; ++i)
        {
          helper::enlarge_to(mbr1, entries[i].first);
        }

        geometry_type mbr2 = entries[k].first;
        for (int i = k + 1; i <= NodeType::MAX_ENTRIES; ++i)
        {
          helper::enlarge_to(mbr2, entries[i].first);
        }

        // compute S
        const scalar_type margin = helper::margin(mbr1) + helper::margin(mbr2);
        const scalar_type overlap = helper::intersection_area(mbr1, mbr2);

        if (choose_axis == -1 || margin < min_margin
            || (margin == min_margin && overlap < min_overlap))
        {
          choose_axis = axis;
          min_overlap = overlap;
          min_margin = margin;
        }
      }
    }
    EH_RTREE_ASSERT_SILENT(choose_axis != -1);
    // choose axis end

    // choose split index
    int choose_index = -1;
    min_overlap = std::numeric_limits<scalar_type>::max();
    scalar_type min_area = std::numeric_limits<scalar_type>::max();

    if (choose_axis < traits::DIM - 1)
    {
      int axis = choose_axis;
      // sort along axis
      std::sort(entries.begin(), entries.end(),
                [axis](typename NodeType::value_type const& a,
                       typename NodeType::value_type const& b)
                {
                  if (helper::min_point(a.first, axis)
                      < helper::min_point(b.first, axis))
                    return true;
                  else if (helper::min_point(a.first, axis)
                           == helper::min_point(b.first, axis))
                    return helper::max_point(a.first, axis)
                           < helper::max_point(b.first, axis);
                  else
                    return false;
                });
    }

    // split MAX_ENTRIES+1 nodes into two groups;
    // [0, k) and [k, MAX_ENTRIES]
    for (int k = NodeType::MIN_ENTRIES;
         k <= NodeType::MAX_ENTRIES - NodeType::MIN_ENTRIES + 1; ++k)
    {
      // calculate bounding box
      geometry_type mbr1 = entries[0].first;
      for (int i = 1; i < k; ++i)
      {
        helper::enlarge_to(mbr1, entries[i].first);
      }

      geometry_type mbr2 = entries[k].first;
      for (int i = k + 1; i <= NodeType::MAX_ENTRIES; ++i)
      {
        helper::enlarge_to(mbr2, entries[i].first);
      }

      // compute S
      const auto area_sum = helper::area(mbr1) + helper::area(mbr2);
      const auto overlap = helper::intersection_area(mbr1, mbr2);

      if (choose_index == -1 || overlap < min_overlap
          || (overlap == min_overlap && area_sum < min_area))
      {
        choose_index = k;
        min_overlap = overlap;
        min_area = area_sum;
      }
    }
    EH_RTREE_ASSERT_SILENT(choose_index >= NodeType::MIN_ENTRIES);
    EH_RTREE_ASSERT_SILENT(choose_index <= NodeType::MAX_ENTRIES
                                               - NodeType::MIN_ENTRIES + 1);

    // split nodes
    for (int i = 0; i < choose_index; ++i)
    {
      node->insert(std::move(entries[i]));
    }
    for (int i = choose_index; i <= NodeType::MAX_ENTRIES; ++i)
    {
      node_pair->insert(std::move(entries[i]));
    }
    return node_pair;
  }
};
}
} // namespace eh rtree