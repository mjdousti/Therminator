/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#define VALUE float

// Source: https://stackoverflow.com/a/3767883
// Example: ASSERT(x < 10, "x was " << x);
#define ASSERT(condition, message)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << "Assertion `" #condition "` failed in " << __FILE__         \
                << " line " << __LINE__ << ": " << message << "\n";            \
      std::terminate();                                                        \
    }                                                                          \
  } while (false)

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>


/* Boost libraries */
#include <boost/algorithm/string.hpp>
#include <boost/numeric/odeint.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::unordered_map;
using std::list;
using std::min;
using std::max;
