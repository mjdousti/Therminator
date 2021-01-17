/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "floorplan.h"
#include "material.h"
#include "physical_entity.h"
#include "subcomponent.h"


struct Resolution {
  int width;
  int length;
  int height;
};
/**
 * Stores component data
 */
class Component : public PhysicalEntity {
  Material *material;
  Floorplan *floorplan;
  bool power_gen;
  bool lateral_connectivity;
  bool fill;
  Material *fill_type;
  Resolution resolution;
  vector<SubComponent *> subComponents;

public:
  Component(string s, VALUE l, VALUE w, VALUE h, VALUE x, VALUE y,
            VALUE z);
  void setMaterial(Material *);
  Material *getMaterial();
  void setFloorplan(Floorplan *);
  Floorplan *getFloorplan();
  bool hasFloorPlan();
  void setPowerGen(bool);
  bool isPowerGen();
  void setLateralConnectiviy(bool);
  bool isLaterallyConnected();
  void setResolution(int w, int l, int h);
  void setFill(bool b);
  void setFillType(Material *m);
  bool hasToBeFilled();
  Material *getFillType();
  Resolution getResolution();
  SubComponent *getSubComponent(int i);

  void printDetails();
  int getSubComponentCount();

  virtual ~Component();
};
