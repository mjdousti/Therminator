/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include <algorithm>
#include <boost/functional/hash.hpp>
#include <tuple>
#include <unordered_set>
#include <utility>

#include "headers/general.h"
#include "headers/neighbor_components.h"
#include "headers/utils.h"

// Open intervals should be sorted and assessed before the close intervals.
enum class IntervalType { open = 0, close = 1 };

namespace neighbor_components {

std::unordered_set<subcomponent_pair, boost::hash<subcomponent_pair>>
getOverlaps(const std::vector<std::tuple<SubComponent *, VALUE, IntervalType>>
                &subcomponents) {

  std::unordered_set<subcomponent_pair, boost::hash<subcomponent_pair>>
      overlaps;
  std::unordered_set<SubComponent *> current_open;

  for (auto &tuple_component : subcomponents) {
    auto [c, val, type] = tuple_component;

    if (type == IntervalType::open) {
      for (auto overlap : current_open) {
        overlaps.insert(std::make_pair(min(c, overlap), max(c, overlap)));
      }
      current_open.insert(c);
    } else {
      current_open.erase(c);
    }
  }
  return overlaps;
}

bool compareTuples(std::tuple<SubComponent *, VALUE, IntervalType> a,
                   std::tuple<SubComponent *, VALUE, IntervalType> b) {
  VALUE a_val = std::get<1>(a);
  VALUE b_val = std::get<1>(b);
  // First place the open interval to capture the tangential
  // overlap.
  return utils::neq(a_val, b_val) ? a_val < b_val
                                  // first open, then close
                                  : std::get<2>(a) < std::get<2>(b);
}

std::vector<std::tuple<SubComponent *, VALUE, IntervalType>>
sortSubcomponents(std::vector<SubComponent *> subComponents,
                  VALUE func1(SubComponent *), VALUE func2(SubComponent *)) {
  std::vector<std::tuple<SubComponent *, VALUE, IntervalType>>
      subComponents_sorted;

  for (auto c : subComponents) {
    subComponents_sorted.push_back(
        std::tuple<SubComponent *, VALUE, IntervalType>(c, func1(c),
                                                        IntervalType::open));
    subComponents_sorted.push_back(
        std::tuple<SubComponent *, VALUE, IntervalType>(c, func1(c) + func2(c),
                                                        IntervalType::close));
  }
  std::sort(subComponents_sorted.begin(), subComponents_sorted.end(),
            compareTuples);
  return subComponents_sorted;
}

std::vector<subcomponent_pair> getNeighbors(Device *device) {
  auto subComponents = device->getSubComponents();
  auto subComponents_sorted_x = sortSubcomponents(
      subComponents, [](SubComponent *c) -> VALUE { return c->getX(); },
      [](SubComponent *c) -> VALUE { return c->getLength(); }

  );
  auto subComponents_sorted_y = sortSubcomponents(
      subComponents, [](SubComponent *c) -> VALUE { return c->getY(); },
      [](SubComponent *c) -> VALUE { return c->getWidth(); }

  );
  auto subComponents_sorted_z = sortSubcomponents(
      subComponents, [](SubComponent *c) -> VALUE { return c->getZ(); },
      [](SubComponent *c) -> VALUE { return c->getHeight(); }

  );

  auto overlaps_x = getOverlaps(subComponents_sorted_x);
  auto overlaps_y = getOverlaps(subComponents_sorted_y);
  auto overlaps_z = getOverlaps(subComponents_sorted_z);

  std::vector<std::pair<SubComponent *, SubComponent *>> common_overlaps;

  for (auto &c : overlaps_x) {
    if (overlaps_y.find(c) != overlaps_y.end() &&
        overlaps_z.find(c) != overlaps_z.end()) {
      common_overlaps.push_back(c);
    }
  }
  for (auto &c : overlaps_y) {
    if (overlaps_x.find(c) != overlaps_x.end() &&
        overlaps_z.find(c) != overlaps_z.end()) {
      common_overlaps.push_back(c);
    }
  }
  for (auto &c : overlaps_z) {
    if (overlaps_x.find(c) != overlaps_x.end() &&
        overlaps_y.find(c) != overlaps_y.end()) {
      common_overlaps.push_back(c);
    }
  }
  return common_overlaps;
}

} // namespace neighbor_components