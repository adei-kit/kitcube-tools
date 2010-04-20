/********************************************************************
* Description: Header file for simulation device
* Author: Norbert Flatinger, norbert.flatinger@kit.edu
* Created at: Wed Dec  2 2009
* Computer: ipeflatinger1
* System: Linux 2.6.26-2-686 on i686
*
* Copyright (c) 2009 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#ifndef NORBERT_H
#define NORBERT_H

#include "daqbinarydevice.h"


/** Implementation for the norbert simulation device.
  */

class Norbert: public DAQBinaryDevice {
public:
	/** Constructor */
	Norbert();

	/** Destructor */
	~Norbert();

	/** Set default configuration */
	void setConfigDefaults();
	
	/** Read parameter from inifile */
	//void readInifile(const char *inifile, const char *group = 0);

	/** Returns the path relative to the base path to the data dir */
	const char *getDataDir();
	
	/** Implements the data filename convention of the DAQ module. */
	const char *getDataFilename();
	
	void replaceItem(const char **header, const char *itemTag, const char *newValue);
	
	const char * getStringItem(const char **header, const char *itemTag);	
	int getNumericItem(const char **header, const char *itemTag);	
	
	//const char *getModuleName(const char*filename, unsigned long *deviceNumber = 0);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

	/** Re-implement the calculation of the file numbering scheme. 
	  * The ceilometer uses date and times in the filename 
	  */
	int getFileNumber(char *filename);
	
	/** Get time until next sample and it's id */
	void readHeader(const char *header);

	void writeHeader();
	
	void readData(const char *dir, const char *filename);	

	/** Replace time stamp in the data set by the current time */
	void updateDataSet(unsigned char *buf);


private:
	unsigned char *headerRaw;

	std::string experimentName;	
	
	unsigned long tSample;
	
	struct timeval tRef;
	
};

#endif
