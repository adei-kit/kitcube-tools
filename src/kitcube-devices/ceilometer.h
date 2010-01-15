/***************************************************************************
                          ceilometer.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef CEILOMETER_H
#define CEILOMETER_H


#include <cstdio>
#include <string>
#include <map>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif // of USE_MYSQL

#include <akutil/simpleserver.h>
#include <akutil/procDuration.h>

#include "daqbinarydevice.h"




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
	
	/** Implements the data filename convention of the DAQ module. */
	const char *getDataFilename();
	
	
	void replaceItem(const char **header, const char *itemTag, const char *newValue);
	
	const char * getStringItem(const char **header, const char *itemTag);	
	int getNumericItem(const char **header, const char *itemTag);	
		
	//const char *getModuleName(const char*filename, unsigned long *deviceNumber = 0);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned long getSensorGroup();

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
