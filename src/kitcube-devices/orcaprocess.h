/********************************************************************
* Description:
* Author: Andreas Kopmann
* Created at: Thu Nov 15 20:50:20 CEST 2011
*    
* Copyright (c) 2011 Andreas Kopmann  All rights reserved.
*
********************************************************************/


#ifndef ORCAPROCESS_H
#define ORCAPROCESS_H

#include "daqasciidevice.h"


class OrcaProcess: public DAQAsciiDevice {
public:
	/**  */
	OrcaProcess();
	
	/**  */
	~OrcaProcess();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
private:
	struct timeval start_time;
	
	/** Number of channels in the PAC data files */
	int nMap;
	
	/** Mapping of the sensors */
	int map[32];	
	
};
#endif
