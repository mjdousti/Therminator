/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/physical_entity.h"

PhysicalEntity::PhysicalEntity(string s, double l, double w, double h, double x, double y, double z)
:Entity(s)
{
	length=l;
	width=w;
	height=h;
	this->x=x;
	this->y=y;
	this->z=z;
}
double PhysicalEntity::getLength(){
	return length;
}
double PhysicalEntity::getWidth(){
	return width;
}
double PhysicalEntity::getHeight(){
	return height;
}
double PhysicalEntity::getX(){
	return x;
}
double PhysicalEntity::getY(){
	return y;
}
double PhysicalEntity::getZ(){
	return z;
}

PhysicalEntity::~PhysicalEntity() {
	// TODO Auto-generated destructor stub
}

