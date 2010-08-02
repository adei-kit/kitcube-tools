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


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  */

class dreim: public DAQAsciiDevice {
public:
	/**  */
	dreim();
	/**  */
	~dreim();
	
	void setConfigDefaults();
	
	void readHeader(const char *filename);
	
	void parseData(char *line, struct timeval *l_tData, float *sensorValue);
	
	void copyRemoteData();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:


};

#endif
