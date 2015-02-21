/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/subcomponent.h"

SubComponent::SubComponent(string s, double l, double w, double h, double x, double y, double z, Component* c)
:PhysicalEntity(s, l, w, h, x, y, z)
{
	component=c;
	primary = false;
}

Component* SubComponent::getComponent(){
	return component;
}

void SubComponent::setPrimary(){
	primary =true;
}

bool SubComponent::isPrimary(){
	return primary;
}

SubComponent::~SubComponent() {
}

