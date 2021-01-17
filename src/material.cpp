/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/material.h"

Material::Material(string s, VALUE specific_heat, VALUE density,
                   VALUE normal_conductiviy, VALUE planar_conductivity)
    : Entity(s) {
  this->specific_heat = specific_heat;
  this->density = density;
  this->normal_conductiviy = normal_conductiviy;
  if (utils::eq(planar_conductivity, 0))
    this->planar_conductivity = -1;
  else
    this->planar_conductivity = planar_conductivity;
}
VALUE Material::getNormalConductivity() { return normal_conductiviy; }
bool Material::hasPlanarConductivity() {
  if (utils::eq(planar_conductivity, -1))
    return false;
  else
    return true;
}
VALUE Material::getPlanarConductivity() { return planar_conductivity; }
VALUE Material::getSpecificHeat() { return specific_heat; }
VALUE Material::getDensity() { return density; }
