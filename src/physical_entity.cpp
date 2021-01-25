/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/physical_entity.hpp"

PhysicalEntity::PhysicalEntity(string s, VALUE l, VALUE w, VALUE h, VALUE x,
                               VALUE y, VALUE z)
    : Entity(s) {
  length = l;
  width = w;
  height = h;
  this->x = x;
  this->y = y;
  this->z = z;
}
