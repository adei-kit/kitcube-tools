/***************************************************************************
                          ceilometer.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef CEILOMETER_H
#define CEILOMETER_H

#include "daqbinarydevice.h"

#include <netcdfcpp.h>


/** Implementation for the ceilometer DAQ device.
  * 
  * @todo Move return string to the argument list
  *
  */

class Ceilometer: public DAQBinaryDevice  {
public:
	/**  */
	Ceilometer();

	/**  */
	~Ceilometer();

	/** Set default configuration */
	void setConfigDefaults();
	
	/** Read parameter from inifile */
	//void readInifile(const char *inifile, const char *group = 0);

	/** Returns the path relative to the base path to the data dir */
	const char *getDataDir();
	
	void replaceItem(const char **header, const char *itemTag, const char *newValue);
	
	const char * getStringItem(const char **header, const char *itemTag);	
	int getNumericItem(const char **header, const char *itemTag);	
		
	//const char *getModuleName(const char*filename, unsigned long *deviceNumber = 0);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
	/** Get time until next sample and it's id */
	int readHeader(const char *header);

	void writeHeader();
	
	void readData(std::string full_filename);

	void readNetcdf(std::string full_filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
    
    
	/** Replace time stamp in the data set by the current time */
	void updateDataSet(unsigned char *buf);
	
	
private:
	unsigned char *headerRaw;
	
	std::string experimentName;
	
	unsigned long tSample;
	
	struct timeval tRef;
	
};

#endif
