/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once


#include "device.hpp"
#include "physical_entity.hpp"
#include "utils.hpp"

#include <iostream>



/**
 * Model specific constant. Changed from 1/2 to 1/3 due to the difference from
 * traditional Elmore Delay scenario.
 */
#define C_FACTOR 0.5


class RCutils {
public:
  static VALUE calcConductanceToAmbient(SubComponent *sc, Device *d);
  static VALUE calcCommonConductance(SubComponent *sc1, SubComponent *sc2);
  static VALUE overallParallelConductivity(VALUE k1, VALUE k2);
  static VALUE calcAmbientResistance(VALUE h, VALUE area);
  static VALUE calcThermalConductivity(VALUE k, VALUE thickness,
                                        VALUE area);

  static VALUE calcSubComponentCapacitance(SubComponent *sc);

  static bool touchesAirInXDir(SubComponent *sc, Device *device);
  static bool touchesAirInYDir(SubComponent *sc, Device *device);
  static bool touchesAirFromTopBot(SubComponent *sc, Device *device);

private:

};
