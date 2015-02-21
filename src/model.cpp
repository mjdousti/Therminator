/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/model.h"


Model::Model(Device *d){
	//P & G matrices should be made later
	T_vector_made = P_vector_made = G_matrix_made = false;

	device=d;
	elements_no=device->getComponentCount();
	cout<<"Component count: "<<elements_no<<endl;

	G_matrix= Utils::matrixAlloc(elements_no, elements_no);

	//Allocating space for the power vector
	P_vector = new double [elements_no];
	//Allocating space for the temperature vector
	T_vector = new double [elements_no];
	//Initializing the vectors to zero
	for (int i = 0; i < elements_no; i++) {
		P_vector[i]=T_vector[i]=0.0;
	}
}

void Model::makePVector(){
	string temp;
	map<int, string> powerMappingTraceOrder;
	vector<SubComponent *> subComponents = device->getSubComponents();
	SubComponent* sc;
	int i=0;
	int power_consumers = 0;	//This designates the number of power consumers we expect to find in the power trace file
	//It is used to check if the power trace has enough info

	//Storing the device subcomponents order in a map, i.e., powerMappingDeviceOrder in order to access them while filling the P vector
	for(vector<SubComponent *>::iterator it1 = subComponents.begin(); it1 != subComponents.end(); it1++){
		sc=(*it1);
		//avoid using (i,j) concatenated with the names in the high-res components
		if (sc->isPrimary()){
			powerMappingDeviceOrder.insert(pair<string, int> (sc->getComponent()->getName(), i));
		}else{
			powerMappingDeviceOrder.insert(pair<string, int> (sc->getName(), i));
		}
		if (sc->getComponent()->isPowerGen() && (sc->getComponent()->hasFloorPlan() || sc->isPrimary())){
			power_consumers++;
		}
		i++;
	}
	//Opening the power trace file for reading
	string powerTraceFileAddr = "./" + device->getPowerTraceFile();
	ifstream powerTraceFile(powerTraceFileAddr.c_str(), ifstream::in);
	if (!powerTraceFile.is_open()){
		cerr<<"Could not open "<<powerTraceFileAddr<<" for parsing as the power trace."<<endl;
		exit(-1);
	}

	//skipping from comment
	powerTraceFile>>temp;
	while (temp.compare("#")==0){
		getline(powerTraceFile, temp);
		if (powerTraceFile.eof()){
			cerr<<"The file does not contain valid power trace."<<endl;
			powerTraceFile.close();
			exit (-1);
		}else
			powerTraceFile>>temp;
	}
	//Reading the column titles
	i=0;
	while(!Utils::isDouble(temp)){
		powerMappingTraceOrder.insert(pair<int, string> (i, temp));
		i++;
		if (powerTraceFile.eof())
			break;
		else
			powerTraceFile>>temp;
	}
	//If the number of titles and the power consumer subcomponents do not match, exit.
	if (power_consumers != i){
		cerr<<"The number of (sub)component power trace in "<<powerTraceFileAddr<< " do not match with the XML description."
				<<" The expected sub(component) count was "<<power_consumers<<" but "<<i<<" subcomponents are provided."<<endl;
		powerTraceFile.close();
		exit (-1);
	}

	i=0;
	int subComponentIndex;
	//Adding the power consumption of subcomponents to the P vector
	while(Utils::isDouble(temp)){
		if (powerMappingDeviceOrder.find(powerMappingTraceOrder[i])==powerMappingDeviceOrder.end()){
			cerr<<"Component "<<powerMappingTraceOrder[i]<<" is listed in the trace file but not found in the design file."<<endl;
			powerTraceFile.close();
			exit (-1);
		}	
		subComponentIndex = powerMappingDeviceOrder[powerMappingTraceOrder[i]];
		//If the subcomponent has a resolution, assume the uniform heat distribution for that component
		if (device->getSubComponent(subComponentIndex)->isPrimary()){
			for (int j = subComponentIndex;
					j < subComponentIndex + device->getSubComponent(subComponentIndex)->getComponent()->getSubComponentCount(); j++) {
				P_vector[j]+= atof(temp.c_str()) / device->getSubComponent(subComponentIndex)->getComponent()->getSubComponentCount();
			}
		}else{
			P_vector[subComponentIndex] += atof(temp.c_str());
		}
		i++;
		if (powerTraceFile.eof())
			break;
		else
			powerTraceFile>>temp;
	}
	//If the power values and the power consumer subcomponents do not match, exit.
	if (power_consumers != i){
		cerr<<"The number of (sub)component power trace in "<<powerTraceFileAddr<< " do not match with the XML description."
				<<" The expected sub(component) count was "<<power_consumers<<" but "<<i<<" subcomponents are provided."<<endl;
		powerTraceFile.close();
		exit (-1);
	}

	powerTraceFile.close();
	P_vector_made=true;
}

void Model::makeRModel(){
	int i=0, j=0;
	SubComponent *sc1, *sc2;
	vector<SubComponent *> subComponents = device->getSubComponents();

	for(vector<SubComponent *>::iterator it1 = subComponents.begin(); it1 != subComponents.end(); it1++) {
		sc1 = (*it1);
		//cout<<i<<": "<<sc1->getName()<<" with height of "<<sc1->getHeight()<<endl;
		vector<SubComponent *>::iterator it1_next=it1;
		it1_next++;
		j=i;
		for(vector<SubComponent *>::iterator it2 =  it1_next; it2 != subComponents.end(); it2++) {
			j++;
			sc2 = (*it2);
			double commonConductance=RCutils::calcCommonConductance(sc1, sc2);
			if (Utils::neq(commonConductance, 0)){
				G_matrix[i][j]=G_matrix[j][i] = -commonConductance;
//				cout<< sc1->getName() <<" is adjacent to "<<sc2->getName()<<endl;
			}
		}
		//Adding the conductances to air for items in the borders of the device
		double K1 = RCutils::calcConductanceToAmbient(sc1, device);
		if (Utils::neq(K1,0)){
			//			cout<<sc1->getName()<<" with k "<<k1<<endl;
			//			cout<<"Changing the diagonal "<<i<<" by "<<K1<<endl;
			G_matrix[i][i]+= K1;
			P_vector[i]+=device->getTemperature() * K1;
//			cout<<"Updating the P_vector of "<<sc1->getName()<<" by "<< device->getTemperature() * K1<<endl;
//			cout<<device->getTemperature()<<endl;
//			cout<<"K1="<<K1<<endl;

		}
		//		cout<<i<<": "<<sc1->getName()<<endl;
		i++;
	}

	//Calculating the diagonal elements
	for (int i = 0; i < elements_no; i++) {
		for (int j = 0; j < elements_no; j++) {
			if (i!=j)
				G_matrix[i][i]-= G_matrix[i][j];
		}
	}
	G_matrix_made=true;
}

void Model::solveT(){
	int *p = new int [elements_no];
	if (!P_vector_made){
		cerr<<"P vector is not initialized."<<endl;
		exit (-1);
	}else if (!G_matrix_made){
		cerr<<"G matrix is not initialized."<<endl;
		exit (-1);
	}
#if USE_GPU==1
	RCutils::gpuSolve(G_matrix, elements_no, P_vector, T_vector);
#else
	double** LU=Utils::matrixAlloc(elements_no, elements_no);
	Utils::matrixCopy(LU, G_matrix, elements_no, elements_no);
	RCutils::lupdcmp(LU, elements_no, p);
	RCutils::lusolve(LU, elements_no, p, P_vector, T_vector);
	Utils::matrixDealloc(LU, elements_no);
#endif
	T_vector_made =true;

	delete[] p;
}
void Model::printComponentTemp(string file_output){
	std::ofstream Tout (file_output.c_str(), std::ofstream::out);
	SubComponent *sc=NULL;
	if (!T_vector_made){
		cerr<<"The temperature is not calculated."<<endl;
		return;
	}
	double temp_avg=0;
	double temp_max = 0;
	int count=0, temp_width;
	Component *current_component=NULL;
	for (int i = 0; i < elements_no; ++i) {
			sc = device->getSubComponent(i);
//			cout<<device->getSubComponent(i)->getName()<<":\t"<<Utils::KtoC(T_vector[i])<<" C"<<endl;
	}
	for (int i = 0; i < elements_no; ++i) {
		sc = device->getSubComponent(i);
		if (sc->getComponent() != current_component){
			if (current_component!=NULL){	//not the first component
				cout<<device->getSubComponent(i-1)->getComponent()->getName()<<":\t"<<Utils::KtoC(temp_max)<<" C\t"<<Utils::KtoC(temp_avg/count)<<" C"<<endl;
			}
			Tout<<device->getSubComponent(i)->getComponent()->getName()<<":\n";
			Tout<<Utils::KtoC(T_vector[i])<<"\t";
			current_component =sc->getComponent();
			temp_avg = T_vector[i];
			temp_max = T_vector[i];
			count = 1;
		}else{
			temp_avg += T_vector[i];
			if (T_vector[i]>temp_max){
				temp_max = T_vector[i];
			}
			count++;;
			Tout<<Utils::KtoC(T_vector[i])<<"\t";
			temp_width = device->getSubComponent(i)->getComponent()->getResolution().width;
			if (count%temp_width == 0){
				Tout<<endl;
			}
		}
	}
	cout<<sc->getComponent()->getName()<<":\t"<<Utils::KtoC(temp_max)<<" C\t"<<Utils::KtoC(temp_avg/count)<<" C"<<endl;
	Tout.close();
}
void Model::printSubComponentTemp(){
	SubComponent *sc;
	if (!T_vector_made){
		cerr<<"The temperature is not calculated."<<endl;
		return;
	}

	for (int i = 0; i < elements_no; ++i) {
		sc = device->getSubComponent(i);
		cout<<sc->getName()<<":\t"<<Utils::KtoC(T_vector[i])<<" C"<<endl;
	}
}
void Model::printTVector(){
	if (!T_vector_made){
		cerr<<"T vector is not initialized."<<endl;
		return;
	}
	cout<<"T=";
	Utils::dumpVector(T_vector, elements_no);
}

void Model::printGMatrix(){
	if (!G_matrix_made){
		cerr<<"G matrix is not initialized."<<endl;
		return;
	}
	cout<<"G=";
	Utils::dumpMatrix(G_matrix, elements_no, elements_no);
}
void Model::printPVector(){
	if (!P_vector_made){
		cerr<<"P vector is not initialized."<<endl;
		return;
	}
	cout<<"P=";
	Utils::dumpVector(P_vector, elements_no);
}

Model::~Model(){
	delete(device);
	Utils::matrixDealloc(G_matrix, elements_no);
	delete[] P_vector;
	delete[] T_vector;
}
