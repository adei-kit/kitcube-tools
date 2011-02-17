/********************************************************************
* Description: GPS device for humidity measurement
* Author: Norbert Flatinger, IPE
* Created at: Mon Feb 14 15:04:08 CET 2011
*    
* Copyright (c) 2011 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/

#ifndef GPS_H
#define GPS_H

#include "daqasciidevice.h"


/** Implementation for the gps
  * 
  */

class gps: public DAQAsciiDevice {
public:
	/**  */
	gps();
	
	/**  */
	~gps();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	//void writeHeader();
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	//void writeData();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:

};

#endif
