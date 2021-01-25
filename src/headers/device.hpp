/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "component.hpp"
#include "floorplan.hpp"
#include "general.hpp"
#include "material.hpp"
#include "physical_entity.hpp"
#include "utils.hpp"

class Device : public PhysicalEntity {
  unordered_map<string, Material *> materials;
  unordered_map<string, Floorplan *> floorplans;
  list<Component *> components;
  vector<SubComponent *> subComponents;
  string power_trace_file;
  VALUE t_amb;
  int componentCount;

public:
  Device(string s, VALUE l, VALUE w, VALUE h);
  void setTemperature(VALUE t);
  VALUE getTemperature();
  void fillGaps();
  void setPowerTraceFile(string s);
  string getPowerTraceFile();
  void addMaterial(Material *);
  Material *getMaterial(string name);
  void addFloorplan(Floorplan *);
  Floorplan *getFloorplan(string name);
  void addCompopnent(Component *);
  Component *getComponent(string name);
  list<Component *> getComponents();
  vector<SubComponent *> getSubComponents();
  SubComponent *getSubComponent(int i);

  int getComponentCount();
  int getSubComponentCount();
  void makeComponentsIndex();
  PhysicalEntity *getPhysicalEntity(int i);

  void componentsOverlap();

  virtual ~Device();
};
