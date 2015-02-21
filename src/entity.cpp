/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/entity.h"

Entity::Entity(string s) {
	setName(s);

}
string Entity::getName(){
	return name;
}

void Entity::setName(string s){
	name = s;
}

Entity::~Entity() {
	// TODO Auto-generated destructor stub
}

