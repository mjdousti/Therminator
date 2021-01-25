/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "entity.hpp"
#include "subcomponent.hpp"

class Floorplan : public Entity {
  vector<SubComponent *> subComponents;

public:
  Floorplan(string s);
  void addSubComponent(SubComponent *t);
  SubComponent *getSubComponent(int i);
  int getSubComponentCount();
  virtual ~Floorplan();
};
