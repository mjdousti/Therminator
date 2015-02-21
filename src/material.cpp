/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/material.h"

Material::Material(string s, double specific_heat, double density, double normal_conductiviy, double planar_conductivity)
:Entity(s)
{
	this->specific_heat=specific_heat;
	this->density=density;
	this->normal_conductiviy=normal_conductiviy;
	if (Utils::eq(planar_conductivity, 0))
		this->planar_conductivity=-1;
	else
		this->planar_conductivity=planar_conductivity;
}
double Material::getNormalConductivity(){
	return normal_conductiviy;
}
bool Material::hasPlanarConductivity(){
	if (Utils::eq(planar_conductivity, -1))
		return false;
	else
		return true;
}
double Material::getPlanarConductivity(){
	return planar_conductivity;
}
double Material::getSpecificHeat(){
	return specific_heat;
}
double Material::getDensity(){
	return density;
}

Material::~Material() {
	// TODO Auto-generated destructor stub
}

