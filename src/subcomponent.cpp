/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/subcomponent.hpp"

SubComponent::SubComponent(string s, VALUE l, VALUE w, VALUE h, VALUE x,
                           VALUE y, VALUE z, Component *c)
    : PhysicalEntity(s, l, w, h, x, y, z) {
  component = c;
  primary = false;
}

Component *SubComponent::getComponent() { return component; }

void SubComponent::setPrimary() { primary = true; }

bool SubComponent::isPrimary() { return primary; }

SubComponent::~SubComponent() {}
