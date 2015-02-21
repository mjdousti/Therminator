/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef MATERIAL_H_
#define MATERIAL_H_
#include "entity.h"
#include "utils.h"

class Material:public Entity {
	double specific_heat, density, normal_conductiviy, planar_conductivity;
public:
	Material(string s, double specific_heat, double density, double thermal_conductiviy, double planar_conductivity);
	double getNormalConductivity();
	double getPlanarConductivity();
	bool hasPlanarConductivity();
	double getSpecificHeat();
	double getDensity();
	virtual ~Material();
};

#endif /* MATERIAL_H_ */
