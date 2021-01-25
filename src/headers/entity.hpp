/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "general.hpp"

class Entity {
  string name;

public:
  Entity(string s);
  string getName();
  void setName(string s);
};
