/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef UTILS_H_
#define UTILS_H_
#include "general.h"

class Utils {
public:
	static double KtoC(double temp);
	static bool isDouble(string const& s);
	static void matrixCopy (double **dst, double **src, int row_no, int col_no);
	static double** matrixAlloc (int row_no, int col_no);
	static void matrixDealloc (double **matrix, int row_no);
	static void dumpMatrix (double **matrix, int row_no, int col_no);
	static void dumpVector (double *matrix, int row_no);

	static bool eq(double a, double b);
	static bool neq(double a, double b);
	static bool less(double a, double b);
	static bool le(double a, double b);
	static bool ge(double a, double b);
	static bool greater(double a, double b);


	/*template <class T>
	static T next(T x);

	template <class T, class Distance>
	static T next(T x, Distance n);

	template <class T>
	static T prior(T x);

	template <class T, class Distance>
	static T prior(T x, Distance n);*/
};

#endif /* UTILS_H_ */
