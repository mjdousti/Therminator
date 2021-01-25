/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#include "headers/entity.hpp"

Entity::Entity(string s) { setName(s); }
string Entity::getName() { return name; }

void Entity::setName(string s) { name = s; }
