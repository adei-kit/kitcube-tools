/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Wed Jul 28 15:21:10 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/

#ifndef SZI_H
#define SZI_H

#include "daqasciidevice.h"


/** Implementation for the szintillometer
  * 
  */

class szi: public DAQAsciiDevice {
public:
	/**  */
	szi();
	/**  */
	~szi();
	
	void setConfigDefaults();
	
	void readHeader(const char *filename);
	
	//void writeHeader();
	
	void parseData(char *line, struct timeval *l_tData, float *sensorValue);
	
	//void writeData();
	
	void copyRemoteData();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:


};

#endif
