//
// C++ Interface: mrr
//
// Description: 
//
//
// Author: Norbert Flatinger <norbert.flatinger@kit.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef MRR_H
#define MRR_H

#include "daqasciidevice.h"


/** Implementation for the mrr device
  */

class mrr: public DAQAsciiDevice {
public:
	/**  */
	mrr();
	
	/**  */
	~mrr();
	
	//void setConfigDefaults();
	
	void readHeader(const char *filename);
	
	void writeHeader();
	
	//void parseData(char *line, struct timeval *tData, float *sensorValue);
	
	void writeData();
	
	//void copyRemoteData();
	
	const char *getDataFilename();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
	void readData(const char *dir, const char *filename);
private:

};

#endif
