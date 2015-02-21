/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef ENTITY_H_
#define ENTITY_H_
#include "general.h"

class Entity {
	string name;
public:
	Entity(string s);
	string getName();
	void setName(string s);
	virtual ~Entity();
};

#endif /* ENTITY_H_ */
