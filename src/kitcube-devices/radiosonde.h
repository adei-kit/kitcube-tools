/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Thu Apr 21 10:27:20 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#ifndef RADIOSONDE_H
#define RADIOSONDE_H

#include "daqasciidevice.h"


class radiosonde: public DAQAsciiDevice {
public:
	/**  */
	radiosonde();
	
	/**  */
	~radiosonde();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
private:
	struct timeval start_time;
};
#endif
