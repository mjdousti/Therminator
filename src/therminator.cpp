/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#include "headers/general.h"
#include "headers/parser.h"
#include "headers/device.h"
#include "headers/model.h"

void print_usage(char * argv0);

int main(int argc, char** argv)
{
	/* Parsing the input parameters */
	//Parser::parseCmdLine(argc,argv);
	clock_t start, end;
	double cpu_time_used;
	start = clock();
	string file_output, file_design, file_trace;
	bool flag_designfile = false;
	bool flag_tracefile = false;
	bool flag_outfile = false;		
	if (argc <= 1 || argv[1] == string("-h") || argv[1] == string("-help") || argv[1] == string("--help"))
	{
		print_usage(argv[0]);
	}
	/* First argument is the program name */
	for (int i = 1; i < argc; i++)
	{
		if (argv[i] == string("-d"))
		{
			flag_designfile = true;
			i++;
			file_design.assign(argv[ i]);
		}else if (argv[i] == string("-p"))
		{
			flag_tracefile = true;
			i++;
			file_trace.assign(argv[ i]);
		}else if (argv[i] == string("-o"))
		{
			flag_outfile = true;
			i++;
			file_output.assign(argv[ i]);
		}else{
			cerr<<"Option "<<argv[i]<<" is not supported."<<endl;
			print_usage(argv[0]);
		}
	}
	if (flag_designfile == false || flag_tracefile == false || flag_outfile == false)
	{
		print_usage(argv[0]);
	}
	
	Device* device=Parser::parseDevice(file_design, file_trace);
	if (device == NULL){
		return -1;
	}

	Model* model=new Model(device);
	model->makeRModel();
//	model->printGMatrix();
	model->makePVector();
//	model->printPVector();
	model->solveT();
	model->printComponentTemp(file_output);

	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//model->printPVector();

	//double* T=model->findT();

	delete(model);
	cout<<endl<<"The program is finished successfully."<<endl;
	cout<<endl<<"It took CPU time "<<cpu_time_used<<endl;
	return 0;

}

void print_usage(char * argv0)
{
	cerr <<"usage: therminator -d <file> -p <file> -o <file>"<<endl;
	cerr <<"Therminator: Thermal Simulator for Portable Devices"<<endl;

	cerr <<" -h\t\tShows this help menu"<<endl;
	cerr <<" -d <file>\tInput design specification file"<<endl;
	cerr <<" -p <file>\tInput power trace file"<<endl;
	cerr <<" -o <file>\tOutput file"<<endl;
	exit(-1);
}
