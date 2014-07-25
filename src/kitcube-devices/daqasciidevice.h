//
// C++ Interface: daqasciidevice
//
// Description: 
//
//
// Author: Norbert Flatinger <norbert.flatinger@kit.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef DAQASCIIDEVICE_H
#define DAQASCIIDEVICE_H

#include "daqdevice.h"


class DAQAsciiDevice: public DAQDevice {
public:
	/**  */
	DAQAsciiDevice();
	
	/**  */
	~DAQAsciiDevice();
	
	/** Open file for writing data */
	virtual void openFile();
	
	/** Close file for reading */
	virtual void closeFile();

	/** Get time until next sample and it's id */
	virtual int readHeader(const char *header);
	
	virtual int parseData(char* line, struct timeval* l_tData, double *sensorValue);
		
	virtual void readData(std::string full_filename);
	
	/** Write simulated data set */
	//	virtual void writeData();
	
	
protected:
	/** File descriptor for reading binary data files */
	int fd_data;
	
	/** Number of samples written by writeData() */
	unsigned long nSamples;
	
	/** Number of the current data set in the template file used by writeData() */
	unsigned long nTemplate;
	
	/** Number of channels in the data files */
	int nMap;
	
	/** Mapping of the sensors */
    int * dynMap;
	
};

#endif
