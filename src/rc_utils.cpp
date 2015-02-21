/*
 *
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab,
 * University of Southern California. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
*/

#include "headers/rc_utils.h"

#if USE_GPU==1
void RCutils::checkGPUStatus(culaStatus status){
    char buf[256];

    if(!status)
        return;

    culaGetErrorInfoString(status, culaGetErrorInfo(), buf, sizeof(buf));
    cout<<buf<<endl;

    culaShutdown();
    exit(-1);
}

void RCutils::gpuSolve(double **matrix, int N, double* P_vector, double *T_vector){
    int NRHS = 1;

    culaStatus status;

    culaDouble* A = NULL;
    culaInt* IPIV = NULL;


    cout<<"--------------------"<<endl;
    cout<<" Calling GPU Solver"<<endl;
    cout<<"--------------------"<<endl;

    cout<<"Allocating Matrices..."<<endl;
    A = (culaDouble*)malloc(N*N*sizeof(culaDouble));
    IPIV = (culaInt*)malloc(N*sizeof(culaInt));
    if(!A || !IPIV){
        cerr<<"Cannot allocate contiguous memory for solving the linear set of equations using GPU."<<endl;
    	exit(-1);
    }

    cout<<"Initializing CULA..."<<endl;
    status = culaInitialize();
    RCutils::checkGPUStatus(status);

	for (int i = 0; i < N; i++){
	    memmove(&A[i*N], matrix[i], sizeof(double) * N);
	}

    //Making a copy of P_vector into T_vector; This is required per specification
    memcpy(T_vector, P_vector, N*sizeof(culaDouble));

    //memset(IPIV, 0, N*sizeof(culaInt));

    cout<<"Calling the solver..."<<endl;
    //General Solver
//	status = culaDgesv(N, NRHS, A, N, IPIV, T_vector, N);
    //Assuming that A is a positive-definite matrix
    status = culaDposv('U', N, NRHS, A, N, T_vector, N);
    if(status == culaInsufficientComputeCapability){
        cout<<"No Double precision support available."<<endl;
        delete(A);
        delete(IPIV);
        culaShutdown();
        return;
    }
    RCutils::checkGPUStatus(status);


    cout<<"Shutting down CULA..."<<endl<<endl;
    culaShutdown();

    delete(A);
    delete(IPIV);
}
#else
/*
 * LUP decomposition from the pseudocode given in the CLR
 * 'Introduction to Algorithms' textbook. The matrix 'a' is
 * transformed into an in-place lower/upper triangular matrix
 * and the vector'p' carries the permutation vector such that
 * Pa = lu, where 'P' is the matrix form of 'p'.
 */

void RCutils::lupdcmp(double**a, int n, int *p){
	int i, j, k, pivot=0;
	double max = 0;

	/* start with identity permutation	*/
	for (i=0; i < n; i++)
		p[i] = i;

	for (k=0; k < n; k++)	 {
		max = 0;
		for (i = k; i < n; i++)	{
			if (fabs(a[i][k]) > max) {
				max = fabs(a[i][k]);
				pivot = i;
			}
		}
		if (Utils::eq (max, 0)){
			cerr<<"Singular matrix in lupdcmp."<<endl;
			exit (-1);
		}

		/* bring pivot element to position	*/
		swap(p[k], p[pivot]);
		for (i=0; i < n; i++){
			swap(a[k][i], a[pivot][i]);
		}

		for (i=k+1; i < n; i++) {
			a[i][k] /= a[k][k];
			for (j=k+1; j < n; j++)
				a[i][j] -= a[i][k] * a[k][j];
		}
	}
}

/*
 * the matrix a is an in-place lower/upper triangular matrix
 * the following macros split them into their constituents
 */

#define LOWER(a, i, j)		((i > j) ? a[i][j] : 0)
#define UPPER(a, i, j)		((i <= j) ? a[i][j] : 0)


/*
 * LU forward and backward substitution from the pseudocode given
 * in the CLR 'Introduction to Algorithms' textbook. It solves ax = b
 * where, 'a' is an in-place lower/upper triangular matrix. The vector
 * 'x' carries the solution vector. 'p' is the permutation vector.
 */

void RCutils::lusolve(double **a, int n, int *p, double *b, double *x){
	int i, j;
	double *y = new double[n];
	double sum;

	/* forward substitution	- solves ly = pb	*/
	for (i=0; i < n; i++) {
		for (j=0, sum=0; j < i; j++)
			sum += y[j] * LOWER(a, i, j);
		y[i] = b[p[i]] - sum;
	}

	/* backward substitution - solves ux = y	*/
	for (i=n-1; i >= 0; i--) {
		for (j=i+1, sum=0; j < n; j++)
			sum += x[j] * UPPER(a, i, j);
		x[i] = (y[i] - sum) / UPPER(a, i, i);
	}

	delete[] y;
}
#endif
double RCutils::calcThermalConductivity(double k, double thickness, double area){
	if (thickness==0){
		cerr<<"The thickness of an element cannot be zero."<<endl;
		exit(-1);
	}
	return k * area / thickness;
}

double RCutils::calcAmbientResistance(double h, double area){
	return 1/(h * area);
}

double RCutils::overallParallelConductivity(double k1, double k2){
	return (k1 * k2)/( k1 + k2 );
}


double RCutils::calcConductanceToAmbient(SubComponent *sc, Device* device){
	double commonArea;
	double t1=0;
	double k;
	double K=0;
	double Kx=0, Ky=0, Kz=0, Rx, Ry, Rz;
	double h = 10 * 1.15; //	W/m^2/K, from <http://www.engineeringtoolbox.com/convective-heat-transfer-d_430.html>

	if (Utils::eq(sc->getZ() , device->getZ()) ||
		Utils::eq(sc->getZ() + sc->getHeight() , device->getZ() +  device->getHeight())){	//Touches air from top or bottom
		t1 = sc->getHeight() / 2;
		commonArea = sc->getLength() * sc->getWidth();
//		cout<<p1->getName() << " and "<<" have XY common area to ambient: "<<commonArea<<endl;

		k = sc->getComponent()->getMaterial()->getNormalConductivity();

		Kz= RCutils::calcThermalConductivity(k, t1, commonArea);
//		cout<<RCutils::calcThermalConductivity(k1, t1, commonArea)<<endl;
		Rz = RCutils::calcAmbientResistance(h, commonArea);
		Kz = RCutils::overallParallelConductivity(Kz, 1/Rz);
	}
	if (Utils::eq(sc->getX(), device->getX()) ||
		Utils::eq(sc->getX() +  sc->getLength(), device->getX() +  device->getLength())){	//Touches air from the X side
		t1 = sc->getLength() / 2;
		commonArea = sc->getWidth() * sc->getHeight();
//		cout<<p1->getName() << " and "<<" have YZ common area to ambient: "<<commonArea<<endl;

		//Setting the k1 value to the proper value if the planar conductivity differs from the normal conductivity
		if (sc->getComponent()->getMaterial()->hasPlanarConductivity())
			k = sc->getComponent()->getMaterial()->getPlanarConductivity();
		else
			k = sc->getComponent()->getMaterial()->getNormalConductivity();


		Kx = RCutils::calcThermalConductivity(k, t1, commonArea);
		Rx = RCutils::calcAmbientResistance(h, commonArea);
		Kx = RCutils::overallParallelConductivity(Kx, 1/Rx);
	}
	if (Utils::eq(sc->getY(), device->getY()) ||
		Utils::eq(sc->getY() +  sc->getWidth(), device->getY() +  device->getWidth())){		//Touches air from the Y side

		t1 = sc->getWidth() / 2;
		commonArea = sc->getLength() * sc->getHeight();
//		cout<<p1->getName() << " and "<<" have XZ common area to ambient: "<<commonArea<<endl;

		//Setting the k1 value to the proper value if the planar conductivity differs from the normal conductivity
		if (sc->getComponent()->getMaterial()->hasPlanarConductivity())
			k = sc->getComponent()->getMaterial()->getPlanarConductivity();
		else
			k = sc->getComponent()->getMaterial()->getNormalConductivity();

		Ky = RCutils::calcThermalConductivity(k, t1, commonArea);
		Ry = RCutils::calcAmbientResistance(h, commonArea);
		Ky = RCutils::overallParallelConductivity(Ky, 1/Ry);
	}
	return K= Kx + Ky + Kz;
}

double RCutils::calcCommonConductance(SubComponent *sc1, SubComponent *sc2){
	double commonArea;
	double t1=0, t2=0;
	double k1, k2;

	// common area in the Y & Z planes
	double commonX, commonY, commonZ;

	commonX= min(sc1->getX()+sc1->getLength() , sc2->getX()+sc2->getLength()) - max(sc1->getX() , sc2->getX());
	commonY= min(sc1->getY()+sc1->getWidth() , sc2->getY()+sc2->getWidth()) - max(sc1->getY() , sc2->getY());
	commonZ= min(sc1->getZ()+sc1->getHeight() , sc2->getZ()+sc2->getHeight()) - max(sc1->getZ() , sc2->getZ());


	if ((Utils::eq(sc1->getX() + sc1->getLength() , sc2->getX()) ||
		Utils::eq(sc2->getX() + sc2->getLength() , sc1->getX())) && commonZ>0 && commonY>0){
		commonArea = commonY * commonZ;
		 t1 = sc1->getLength() / 2;
		 t2 = sc2->getLength() / 2;

		 //using planar thermal conductivity if the material has different value for it
		 if (sc1->getComponent()->getMaterial()->hasPlanarConductivity()){
			 k1 = sc1->getComponent()->getMaterial()->getPlanarConductivity();
		 }else{
			 k1 = sc1->getComponent()->getMaterial()->getNormalConductivity();
		 }
		 if (sc2->getComponent()->getMaterial()->hasPlanarConductivity()){
			 k2 = sc2->getComponent()->getMaterial()->getPlanarConductivity();
		 }else{
			 k2 = sc2->getComponent()->getMaterial()->getNormalConductivity();
		 }
//		 cout<<"Common area (YZ) btn "<<p1->getName()<<" and "<<p2->getName()<<" is "<<commonArea<<endl;
	}else if ((Utils::eq(sc1->getY() + sc1->getWidth() , sc2->getY()) ||
			Utils::eq(sc2->getY() + sc2->getWidth() , sc1->getY())) && commonX>0 && commonZ>0){
		commonArea = commonX * commonZ;
		 t1 = sc1->getWidth() / 2;
		 t2 = sc2->getWidth() / 2;

		 //using planar thermal conductivity if the material has different value for it
		 if (sc1->getComponent()->getMaterial()->hasPlanarConductivity()){
			 k1 = sc1->getComponent()->getMaterial()->getPlanarConductivity();
		 }else{
			 k1 = sc1->getComponent()->getMaterial()->getNormalConductivity();
		 }
		 if (sc2->getComponent()->getMaterial()->hasPlanarConductivity()){
			 k2 = sc2->getComponent()->getMaterial()->getPlanarConductivity();
		 }else{
			 k2 = sc2->getComponent()->getMaterial()->getNormalConductivity();
		 }
//		 cout<<"Common area (XZ) btn "<<p1->getName()<<" and "<<p2->getName()<<" is "<<commonArea<<endl;
	}else if ((Utils::eq(sc1->getZ() + sc1->getHeight() , sc2->getZ()) ||
			Utils::eq(sc2->getZ() + sc2->getHeight(), sc1->getZ())) && commonX>0 && commonY>0){
		commonArea = commonX * commonY;
		t1 = sc1->getHeight() / 2;
		t2 = sc2->getHeight() / 2;

		//using normal conductivity since it is in the vertical direction
		 k1 = sc1->getComponent()->getMaterial()->getNormalConductivity();
		 k2 = sc2->getComponent()->getMaterial()->getNormalConductivity();

//		 cout<<"Common area (XY) btn "<<p1->getName()<<" and "<<p2->getName()<<" is "<<commonArea<<endl;
	}else{
		commonArea = 0;
		return 0;
	}
	//cout<<p1->getName() << " and "<<p2->getName() <<" have common area: "<<commonArea<<endl;

	double K1 = RCutils::calcThermalConductivity(k1, t1, commonArea);
	double K2 = RCutils::calcThermalConductivity(k2, t2, commonArea);

	return RCutils::overallParallelConductivity(K1,K2);
}


