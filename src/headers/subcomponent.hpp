/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "physical_entity.hpp"

class Component;

class SubComponent : public PhysicalEntity {
  Component *component;
  bool primary;

public:
  Component *getComponent();
  SubComponent(string s, VALUE l, VALUE w, VALUE h, VALUE x, VALUE y,
               VALUE z, Component *c);
  void setPrimary();
  bool isPrimary();
  virtual ~SubComponent();
};
