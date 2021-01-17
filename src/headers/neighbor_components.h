/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include <utility>
#include <vector>

#include "device.h"

typedef std::pair<SubComponent *, SubComponent *> subcomponent_pair;

namespace neighbor_components {

std::vector<subcomponent_pair> getNeighbors(Device *device);

} // namespace neighbor_components