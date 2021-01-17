/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once


#include "device.h"
#include "general.h"
#include "subcomponent.h"

#include "../libs/pugixml/pugixml.hpp"

class Parser {
public:
  static int parseCmdLine(int argc, char **argv);
  static Device *parseDevice(const string& design, const string& trace);
};
