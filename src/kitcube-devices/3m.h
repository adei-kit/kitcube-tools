/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Mon Aug  2 13:42:15 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#ifndef DREIM_H
#define DREIM_H

#include "daqasciidevice.h"


class dreim: public DAQAsciiDevice {
public:
	/**  */
	dreim();
	/**  */
	~dreim();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:
	double convert_coordinate(char *coordinate_string);

};

#endif
