/***************************************************************************
                          ceilometer.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef WOLKENKAMERA_H
#define WOLKENKAMERA_H

#include "daqbinarydevice.h"


/** Implementation for the ceilometer DAQ device.
  * 
  * @todo Move return string to the argument list
  *
  */

class wolkenkamera: public DAQBinaryDevice  {
public:
	/**  */
	wolkenkamera();

	/**  */
	~wolkenkamera();

	/** Set default configuration */
	void setConfigDefaults();
	
	/** Read parameter from inifile */
	//void readInifile(const char *inifile, const char *group = 0);

	/** Returns the path relative to the base path to the data dir */
	const char *getDataDir();
	
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
	/** Get time until next sample and it's id */
	void readHeader(const char *header);

	void writeHeader();
	
	void readData(const char *dir, const char *filename);
	
	/** Replace time stamp in the data set by the current time */
	void updateDataSet(unsigned char *buf);
	
	int getFileNumber(char* filename);
	
private:
	unsigned char *headerRaw;
	
	std::string experimentName;
	
	unsigned long tSample;
	
	struct timeval tRef;
	
};

#endif
