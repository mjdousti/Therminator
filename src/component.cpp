/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/component.h"

Component::Component(string s, double l, double w, double h, double x, double y, double z)
:PhysicalEntity(s, l, w, h, x, y, z)
{
	floorplan=NULL;
	lateral_connectivity=false;
	power_gen=false;
	fill=false;
	fill_type=NULL;
	material=NULL;
	// The -1 values are used to make sure that both resolution and floorplan is not used
	resolution.length = -1;
	resolution.width = -1;
	resolution.height = -1;
}
void Component::setMaterial(Material *m){
	material=m;
}

bool Component::hasFloorPlan(){
	if (floorplan==NULL)
		return false;
	else
		return true;
}
Floorplan* Component::getFloorplan(){
	return floorplan;
}
void Component::setFloorplan(Floorplan *f){
	if (resolution.length!=-1 || resolution.width!=-1 || resolution.height!=-1){
		cerr<<"Component "<<getName()<<" can have either resolution or floorplan."<<endl;
		exit(-1);
	}
	floorplan=f;

	if (floorplan==NULL)
		return;

	ostringstream subComponentName;
	for (int i = 0; i < floorplan->getSubComponentCount(); i++) {
		SubComponent *temp = floorplan->getSubComponent(i);
		//Assertion to make sure that the floorplan is valid for the component
		if (temp->getX() + temp->getLength() > getLength() ||
			temp->getY() + temp->getWidth() > getWidth()){

			cerr<<"The tile "<< temp->getName()<<" in the floorplan "<< floorplan->getName()
					<<" does not fit in the component "<<getName()<<"."<<endl;
			exit(-1);
		}
		subComponentName.str("");
		subComponentName.clear();
		subComponentName<<getName()<<"-"<<temp->getName();
		//Note: the X & Y are relative values in the floorplan assuming that the bottom-left corner is designated as (0,0)
		SubComponent* sc=new SubComponent(subComponentName.str(), temp->getLength(), temp->getWidth(), getHeight(),
				getX() + temp->getX(), getY() + temp->getY(), getZ(), this);
		subComponents.push_back(sc);
	}
}
void Component::setPowerGen(bool b){
	power_gen=b;
}
void Component::setLateralConnectiviy(bool b){
	lateral_connectivity=b;
}

void Component::setResolution(int l, int w, int h){
	if (floorplan!=NULL){
		cerr<<"Component "<<getName()<<" can have either resolution or floorplan."<<endl;
		exit(-1);
	}
	if (l<1 || w<1 || h<1){
		cerr<<"Invalid resolution for component "<<getName()<<"."<<endl;
		exit(-1);
	}

	resolution.length=l;
	resolution.width=w;
	resolution.height=h;

	ostringstream subComponentName;
	double length = getLength();
	double width = getWidth();
	double height = getHeight();

	for (int k = 0; k < resolution.height; k++) {
		for (int i = 0; i < resolution.length; i++) {
			for (int j = 0; j < resolution.width; j++) {
				subComponentName.str("");
				subComponentName.clear();
				subComponentName<<getName()<<"("<<i<<","<<j<<","<<k<<")";
				SubComponent* sc=new SubComponent(subComponentName.str(),
						length / resolution.length, width / resolution.width, height / resolution.height,
						getX() + i * (length / resolution.length), getY() + j * (width / resolution.width), getZ() + k * (height/ resolution.height), this);
				//The first component is primary and designates the Component
				if (i==0 && j==0 && k==0)
					sc->setPrimary();
				subComponents.push_back(sc);
			}
		}
	}
}

SubComponent* Component::getSubComponent(int i){
	return subComponents[i];
}

Resolution Component::getResolution(){
	return resolution;
}

int Component::getSubComponentCount(){
	if (floorplan==NULL && resolution.length!=-1 && resolution.width != -1 && resolution.height != -1)
		return resolution.length*resolution.width*resolution.height;
	else if (floorplan!=NULL && resolution.length==-1 && resolution.width == -1 && resolution.height == -1)
		return floorplan->getSubComponentCount();
	else{
		cerr<<"Component "<<getName()<<" can have either resolution or floorplan."<<endl;
		exit(-1);
	}
}
Material *Component::getMaterial(){
	return material;
}

bool Component::isPowerGen(){
	return power_gen;
}

bool Component::isLaterallyConnected(){
	return lateral_connectivity;
}

void Component::setFill(bool b){
	fill=b;
}
bool Component::hasToBeFilled(){
	return fill;
}
void Component::setFillType(Material* m){
	//Just an assertion
	if(m==NULL){
		cerr<<"The fill type material is not valid."<<endl;
		exit(-1);
	}
	fill_type=m;
}

Material* Component::getFillType(){
	if (fill_type==NULL){
		cerr<<"No fill type is set for "<<getName() <<"."<<endl;
		exit (-1);
	}
	return fill_type;
}

void Component::printDetails(){
	cout<<"Component Name: "<<getName()<<endl;
	cout<<"Material type: "<<getMaterial()->getName()<<endl;
	cout<<"Position: ("<<getX()<<","<<getY()<<","<<getZ()<<")"<<endl;
	cout<<"Size: ("<<getLength()<<","<<getWidth()<<","<<getHeight()<<")"<<endl;
	cout<<"Resolution: "<<getResolution().length<<"*"<<getResolution().width<<"*"<<getResolution().height<<endl;
	if (isPowerGen())
		cout<<"Generates power: Yes"<<endl;
	else
		cout<<"Generates power: No"<<endl;
	if (isLaterallyConnected())
		cout<<"Laterally dissipates heat: Yes"<<endl;
	else
		cout<<"Laterally dissipates heat: No"<<endl;
}

Component::~Component() {
	for(vector<SubComponent *>::iterator iterator = subComponents.begin(); iterator != subComponents.end(); iterator++) {
		delete(*iterator);
	}
}

