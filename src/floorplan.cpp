/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/floorplan.h"

Floorplan::Floorplan(string s) : Entity(s) {
}

SubComponent *Floorplan::getSubComponent(int i) { return subComponents[i]; }
void Floorplan::addSubComponent(SubComponent *t) { subComponents.push_back(t); }

int Floorplan::getSubComponentCount() { return subComponents.size(); }

Floorplan::~Floorplan() {
  while (!subComponents.empty()) {
    delete (subComponents.back());
    subComponents.pop_back();
  }
}
