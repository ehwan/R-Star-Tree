#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
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
template <typename TreeType>
struct rstar_split_t
{
  using geometry_type = typename TreeType::geometry_type;
  using traits = geometry_traits<geometry_type>;
  using area_type = typename geometry_traits<geometry_type>::area_type;
  constexpr static area_type LOWEST_AREA
      = std::numeric_limits<area_type>::lowest();

  constexpr static unsigned int MIN_ENTRIES = TreeType::MIN_ENTRIES;
  constexpr static unsigned int MAX_ENTRIES = TreeType::MAX_ENTRIES;

  template <typename NodeType>
  NodeType* operator()(NodeType* node,
                       typename NodeType::value_type new_child,
                       NodeType* node_pair) const
  {
    assert(node->size() == MAX_ENTRIES);
    assert(node_pair);
    assert(node_pair->size() == 0);

    // MAX_ENTRIES+1 nodes
    std::vector<typename NodeType::value_type> entries;
    entries.reserve(MAX_ENTRIES + 1);
    for (int i = 0; i < MAX_ENTRIES; ++i)
    {
      entries.emplace_back(std::move(node->at(i)));
    }
    entries.emplace_back(std::move(new_child));
    node->clear();

    int choose_axis = -1;
    area_type min_overlap = std::numeric_limits<area_type>::max();
    area_type min_margin = std::numeric_limits<area_type>::max();

    // choose axis
    for (int axis = 0; axis < traits::DIM; ++axis)
    {
      // sort along axis
      std::sort(entries.begin(), entries.end(),
                [axis](const auto& a, const auto& b)
                {
                  if (traits::min_point(a.first, axis)
                      < traits::min_point(b.first, axis))
                    return true;
                  else if (traits::min_point(a.first, axis)
                           == traits::min_point(b.first, axis))
                    return traits::max_point(a.first, axis)
                           < traits::max_point(b.first, axis);
                  else
                    return false;
                });

      // split MAX_ENTRIES+1 nodes into two groups;
      // [0, k) and [k, MAX_ENTRIES]
      for (int k = MIN_ENTRIES; k <= MAX_ENTRIES - MIN_ENTRIES + 1; ++k)
      {
        // calculate bounding box
        geometry_type mbr1 = entries[0].first;
        for (int i = 1; i < k; ++i)
        {
          mbr1 = traits::merge(mbr1, entries[i].first);
        }

        geometry_type mbr2 = entries[k].first;
        for (int i = k + 1; i <= MAX_ENTRIES; ++i)
        {
          mbr2 = traits::merge(mbr2, entries[i].first);
        }

        // compute S
        const auto margin = traits::margin(mbr1) + traits::margin(mbr2);
        const auto overlap = traits::area(traits::intersection(mbr1, mbr2));

        if (choose_axis == -1 || margin < min_margin
            || (margin == min_margin && overlap < min_overlap))
        {
          choose_axis = axis;
          min_overlap = overlap;
          min_margin = margin;
        }
      }
    }
    assert(choose_axis != -1);
    // choose axis end

    // choose split index
    int choose_index = -1;
    min_overlap = std::numeric_limits<area_type>::max();
    area_type min_area = std::numeric_limits<area_type>::max();

    if (choose_axis < traits::DIM - 1)
    {
      int axis = choose_axis;
      // sort along axis
      std::sort(entries.begin(), entries.end(),
                [axis](const auto& a, const auto& b)
                {
                  if (traits::min_point(a.first, axis)
                      < traits::min_point(b.first, axis))
                    return true;
                  else if (traits::min_point(a.first, axis)
                           == traits::min_point(b.first, axis))
                    return traits::max_point(a.first, axis)
                           < traits::max_point(b.first, axis);
                  else
                    return false;
                });
    }

    // split MAX_ENTRIES+1 nodes into two groups;
    // [0, k) and [k, MAX_ENTRIES]
    for (int k = MIN_ENTRIES; k <= MAX_ENTRIES - MIN_ENTRIES + 1; ++k)
    {
      // calculate bounding box
      geometry_type mbr1 = entries[0].first;
      for (int i = 1; i < k; ++i)
      {
        mbr1 = traits::merge(mbr1, entries[i].first);
      }

      geometry_type mbr2 = entries[k].first;
      for (int i = k + 1; i <= MAX_ENTRIES; ++i)
      {
        mbr2 = traits::merge(mbr2, entries[i].first);
      }

      // compute S
      const auto area_sum = traits::area(mbr1) + traits::area(mbr2);
      const auto overlap = traits::area(traits::intersection(mbr1, mbr2));

      if (choose_index == -1 || overlap < min_overlap
          || (overlap == min_overlap && area_sum < min_area))
      {
        choose_index = k;
        min_overlap = overlap;
        min_area = area_sum;
      }
    }
    assert(choose_index >= MIN_ENTRIES);
    assert(choose_index <= MAX_ENTRIES - MIN_ENTRIES + 1);

    // split nodes
    for (int i = 0; i < choose_index; ++i)
    {
      node->insert(std::move(entries[i]));
    }
    for (int i = choose_index; i <= MAX_ENTRIES; ++i)
    {
      node_pair->insert(std::move(entries[i]));
    }
    return node_pair;
  }
};
}
} // namespace eh rtree