/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef FLOORPLAN_H_
#define FLOORPLAN_H_
#include "entity.h"
#include "subcomponent.h"


class Floorplan:public Entity {
	vector<SubComponent *> subComponents;
public:
	Floorplan(string s);
	void addSubComponent(SubComponent *t);
	SubComponent* getSubComponent(int i);
	int getSubComponentCount();
	virtual ~Floorplan();
};

#endif /* FLOORPLAN_H_ */
