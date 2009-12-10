/***************************************************************************
                          daqbinarydevice.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef DAQBINARYDEVICE_H
#define DAQBINARYDEVICE_H


#include <cstdio>
#include <string>
#include <map>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif // of USE_MYSQL

#include <akutil/simpleserver.h>
#include <akutil/procDuration.h>

#include "daqdevice.h"


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * The data files contain any kind of binary format.
  */

class DAQBinaryDevice : public DAQDevice {
public:
	/**  */
	DAQBinaryDevice();
	/**  */
	~DAQBinaryDevice();

	/** Open file for writing data */
	virtual void openFile();
	
	/** Close file for reading */
	virtual void closeFile();
	
	/** Write simulated data set */
	void writeData();
	
protected:
	/** File descriptor for reading binary data files */
	int fd_data;

	/** Number of samples written by writeData() */
	unsigned long nSamples;
	
	/** Number of the current data set in the template file used by writeData() */
	unsigned long nTemplate;

};

#endif
