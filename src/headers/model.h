/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef MODEL_H_
#define MODEL_H_
#include "general.h"
#include "device.h"
#include "rc_utils.h"
#include "utils.h"
#include <iterator>

class Model {
	Device *device;
	double** G_matrix;
	double* P_vector, *T_vector;
	bool P_vector_made, G_matrix_made, T_vector_made;
	int elements_no;
	map<string, int> powerMappingDeviceOrder;
public:
	void makeRModel();
	void makePVector();
	void solveT();
	void printSubComponentTemp();
	void printComponentTemp(string file_output);
	void printGMatrix();
	void printPVector();
	void printTVector();
	Model(Device * device);
	virtual ~Model();
};

#endif /* MODEL_H_ */
