/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/floorplan.h"

Floorplan::Floorplan(string s)
:Entity(s)
{
	// TODO Auto-generated constructor stub

}

SubComponent* Floorplan::getSubComponent(int i){
	return subComponents[i];
}
void Floorplan::addSubComponent(SubComponent* t){
	subComponents.push_back(t);
}

int Floorplan::getSubComponentCount(){
	return subComponents.size();
}

Floorplan::~Floorplan() {
	while(!subComponents.empty()){
		delete(subComponents.back());
		subComponents.pop_back();
	}
}

