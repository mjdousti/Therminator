/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef SUBCOMPONENT_H_
#define SUBCOMPONENT_H_

#include "physical_entity.h"

class Component;

class SubComponent:public PhysicalEntity {
	Component* component;
	bool primary;
public:
	Component* getComponent();
	SubComponent(string s, double l, double w, double h, double x, double y, double z, Component* c);
	void setPrimary();
	bool isPrimary();
	virtual ~SubComponent();
};

#endif /* SUBCOMPONENT_H_ */
