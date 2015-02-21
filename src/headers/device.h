/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef DEVICE_H_
#define DEVICE_H_
#include "general.h"
#include "utils.h"
#include "physical_entity.h"
#include "component.h"
#include "floorplan.h"
#include "material.h"


class Device:public PhysicalEntity {
	map<string, Material *> materials;
	map<string, Floorplan *> floorplans;
	list<Component *> components;
	vector<SubComponent*> subComponents;
	string power_trace_file;
	double t_amb;
	int componentCount;

public:
	Device(string s, double l, double w, double h);
	void setTemperature(double t);
	double getTemperature();
	void fillGaps();
	void setPowerTraceFile(string s);
	string getPowerTraceFile();
	void addMaterial(Material *);
	Material* getMaterial(string name);
	void addFloorplan(Floorplan *);
	Floorplan* getFloorplan(string name);
	void addCompopnent(Component *);
	Component* getComponent(string name);
	list<Component *> getComponents();
	vector<SubComponent*> getSubComponents();
	SubComponent* getSubComponent(int i);

	int getComponentCount();
	void makeComponentsIndex();
	PhysicalEntity* getPhysicalEntity(int i);

	void componentsOverlap();


	virtual ~Device();
};

#endif /* DEVICE_H_ */
