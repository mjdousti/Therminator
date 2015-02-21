/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef RCUTILS_H_
#define RCUTILS_H_
#include "utils.h"
#include "physical_entity.h"
#include "device.h"

#if USE_GPU==1
#include <cula_lapack.h>
#endif


class RCutils {
public:
#if USE_GPU==1
	static void checkGPUStatus(culaStatus status);
	static void gpuSolve(double **LU, int elements_no, double* P_vector, double *T_vector);
#else
	static void lupdcmp(double**a, int n, int *p);
	static void lusolve(double **a, int n, int *p, double *b, double *x);
#endif
	static double calcConductanceToAmbient(SubComponent *sc, Device* d);
	static double calcCommonConductance(SubComponent *sc1, SubComponent *sc2);
	static double overallParallelConductivity(double k1, double k2);
	static double calcThermalConductivity(double k, double thickness, double area);
	static double calcAmbientResistance(double h, double area);
};

#endif /* RCUTILS_H_ */
