/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "entity.hpp"

class PhysicalEntity : public Entity {
  VALUE length;
  VALUE width;
  VALUE height;
  VALUE x;
  VALUE y;
  VALUE z;

public:
  inline VALUE getLength() { return length; }
  inline VALUE getWidth() { return width; }
  inline VALUE getHeight() { return height; }
  inline VALUE getX() { return x; }
  inline VALUE getY() { return y; }
  inline VALUE getZ() { return z; }

  PhysicalEntity(string name, VALUE l, VALUE w, VALUE h, VALUE x, VALUE y,
                 VALUE z);
};
