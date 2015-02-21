/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef PHYSICALENTITY_H_
#define PHYSICALENTITY_H_

#include "entity.h"

class PhysicalEntity: public Entity {
	double length, width, height;
	double x, y, z;
public:
	double getLength();
	double getWidth();
	double getHeight();
	double getX();
	double getY();
	double getZ();

	PhysicalEntity(string name, double l, double w, double h, double x, double y, double z);
	virtual ~PhysicalEntity();
};

#endif /* PHYSICALENTITY_H_ */
