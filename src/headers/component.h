/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef COMPONENT_H_
#define COMPONENT_H_
#include "physical_entity.h"
#include "subcomponent.h"
#include "material.h"
#include "floorplan.h"

struct Resolution{
	int width;
	int length;
	int height;
};

class Component:public PhysicalEntity {
	Material* material;
	Floorplan* floorplan;
	bool power_gen;
	bool lateral_connectivity;
	bool fill;
	Material* fill_type;
	Resolution resolution;
	vector<SubComponent* > subComponents;
public:
	Component(string s, double l, double w, double h, double x, double y, double z);
	void setMaterial(Material *);
	Material *getMaterial();
	void setFloorplan(Floorplan *);
	Floorplan* getFloorplan();
	bool hasFloorPlan();
	void setPowerGen(bool);
	bool isPowerGen();
	void setLateralConnectiviy(bool);
	bool isLaterallyConnected();
	void setResolution(int w, int l, int h);
	void setFill(bool b);
	void setFillType(Material* m);
	bool hasToBeFilled();
	Material* getFillType();
	Resolution getResolution();
	SubComponent* getSubComponent(int i);

	void printDetails();
	int getSubComponentCount();

	virtual ~Component();
};

#endif /* COMPONENT_H_ */
