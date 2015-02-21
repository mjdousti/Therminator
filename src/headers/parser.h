/*
 * 
 * Copyright (C) 2014 Mohammad Javad Dousti, Qing Xie, and Massoud Pedram, SPORT lab, 
 * University of Southern California. All rights reserved.
 * 
 * Please refer to the LICENSE file for terms of use.
 * 
*/

#ifndef PARSER_H_
#define PARSER_H_
#include "general.h"
#include "subcomponent.h"
#include "device.h"
//#include <boost/program_options.hpp>
#include "../libs/pugixml/pugixml.hpp"

class Parser {
public:
	Parser();
	virtual ~Parser();

	static int parseCmdLine(int argc, char** argv);
	static Device* parseDevice(string design, string trace);
};

#endif /* PARSER_H_ */
