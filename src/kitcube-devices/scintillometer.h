/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Wed Jul 28 15:21:10 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/

#ifndef SCI_H
#define SCI_H

#include "daqasciidevice.h"


/** Implementation for the scintillometer
  * 
  */

class sci: public DAQAsciiDevice {
public:
	/**  */
	sci();
	/**  */
	~sci();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	//void writeHeader();
	
	void parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	//void writeData();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:

};

#endif
