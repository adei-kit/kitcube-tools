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
	
	/** Write simulated data set */
//	virtual void writeData();
	
	virtual void readData(const char *dir, const char *filename);
	
protected:
	/** File descriptor for reading binary data files */
	int fd_data;
	
	/** Number of samples written by writeData() */
	unsigned long nSamples;
	
	/** Number of the current data set in the template file used by writeData() */
	unsigned long nTemplate;
};

#endif
