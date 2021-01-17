/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "entity.h"
#include "utils.h"

class Material : public Entity {
  VALUE specific_heat, density, normal_conductiviy, planar_conductivity;

public:
  Material(string s, VALUE specific_heat, VALUE density,
           VALUE thermal_conductiviy, VALUE planar_conductivity);
  VALUE getNormalConductivity();
  VALUE getPlanarConductivity();
  bool hasPlanarConductivity();
  VALUE getSpecificHeat();
  VALUE getDensity();
};
