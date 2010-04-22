//
// C++ Interface: parsivel
//
// Description: 
//
//
// Author: Norbert Flatinger <norbert.flatinger@kit.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PARSIVEL_H
#define PARSIVEL_H

#include "daqasciidevice.h"


/** Implementation for the parsivel device
  */

class parsivel: public DAQAsciiDevice {
public:
	/**  */
	parsivel();
	
	/**  */
	~parsivel();
	
	//void setConfigDefaults();
	
	void readHeader(const char *filename);
	
	//void writeHeader();
	
	//void parseData(char *line, struct timeval *tData, float *sensorValue);
	
	void writeData();
	
	//void copyRemoteData();
	
	const char *getDataFilename();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
private:

};

#endif
