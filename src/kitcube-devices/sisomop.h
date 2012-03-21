/********************************************************************
* Description: Soil moisture probes
* Author: Andreas Kopmann, IPE
* Created at: Mon Mar 19 15:04:08 CET 2012
*    
* Copyright (c) 2012 Andreas Kopmann, IPE  All rights reserved.
*
********************************************************************/

#ifndef SISOMOP_H
#define SISOMPO_H

#include "daqasciidevice.h"


/** Implementation for the soil moisture sensors 
  * (Simple Soil Moisture Probe (SISOMOP) )
  * 
  */

class sisomop: public DAQAsciiDevice {
public:
	/**  */
	sisomop();
	
	/**  */
	~sisomop();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:

};

#endif
