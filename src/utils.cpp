/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/
 
#include "headers/utils.h"
#define NEGLIGIBLE_EPSILON 1e-10

double Utils::KtoC(double temp){
	return temp - 273.15;
}
bool Utils::eq(double a, double b)
{
	//return fabs(a - b) < numeric_limits<double>::epsilon();
	return fabs(a - b) < NEGLIGIBLE_EPSILON;
}

bool Utils::neq(double a, double b)
{
	return !eq(a,b);
}

bool Utils::less(double a, double b)
{
	return (b-a) > NEGLIGIBLE_EPSILON;
}

bool Utils::greater(double a, double b)
{
	return (a-b) > NEGLIGIBLE_EPSILON;
}

bool Utils::le(double a, double b)
{
	return ((a < b) || eq(a,b));
}

bool Utils::ge(double a, double b)
{
	return ((a > b) || eq(a,b));
}

bool Utils::isDouble(std::string const& s) {
	std::istringstream ss(s);
	double d;
	return (ss >> d) && (ss >> std::ws).eof();
}

void Utils::matrixCopy (double **dst, double **src, int row_no, int col_no){
// There are excellent notes here:
//	  http://stackoverflow.com/questions/2225850/c-c-how-to-copy-a-multidimensional-char-array-without-nested-loops
	for (int i = 0; i < row_no; i++) {
	    memmove(dst[i], src[i], sizeof(double) * col_no);
	}
}

double **Utils::matrixAlloc (int row_no, int col_no){
	double **matrix = new double*[row_no];

	for (int i = 0; i < row_no; i++) {
		matrix[i]=new double [col_no];
		for (int j = 0; j < col_no; j++) {
			matrix[i][j]=0;
		}
	}
	return matrix;
}

void Utils::matrixDealloc (double **matrix, int row_no){
	for (int i = 0; i < row_no; i++) {
		delete[] matrix[i];
	}
	delete[] matrix;
}

void Utils::dumpMatrix (double **matrix, int row_no, int col_no){
	cout<<"[";
	for (int i = 0; i < row_no; i++) {
		for (int j = 0; j < col_no; j++) {
			cout << matrix[i][j] <<" ";
		}
		cout<<";"<<endl;
	}
	cout<<"];"<<endl;
}

void Utils::dumpVector (double *vector, int row_no){
	cout<<"[";
	for (int i = 0; i < row_no; i++) {
		cout << vector[i] <<"; ";
	}
	cout<<"];"<<endl;
}


/*template <class T> T Utils::next(T x) {
	return ++x;
}

template <class T, class Distance>
T Utils::next(T x, Distance n){
    std::advance(x, n);
    return x;
}

template <class T>
T Utils::prior(T x) {
	return --x;
}

template <class T, class Distance>
T Utils::prior(T x, Distance n){
    std::advance(x, -n);
    return x;
}
 */
