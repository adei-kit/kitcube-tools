/***************************************************************************
                          mast.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef MAST_H
#define MAST_H


#include <cstdio>
#include <string>
#include <map>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif // of USE_MYSQL

#include <akutil/simpleserver.h>
#include <akutil/procDuration.h>

#include "daqbinarydevice.h"


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  * @todo Move return string to the argument list
  * @todo Create Data table
  * @todo Write data to table
  * @todo Write files with simulated data
  *
  */

class Mast: public DAQBinaryDevice  {
public: 
  /**  */
  Mast();
  /**  */
  ~Mast();

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
	
	unsigned long getSensorGroup();

	const char *getSensorName(const char *longname, unsigned long *aggregation = 0);

	const char *getSensorType(const char *unit);
		
    /** Get time until next sample and it's id */
    void readHeader(const char *header);

    void writeHeader();
	
    void readData(const char *dir, const char *filename);	

	/** Replace the time stamp of the data set by the current time */
	void updateDataSet(unsigned char *buf);
	
private:
	unsigned char *headerRaw;
	
    std::string experimentName;	
	
	unsigned int tSample;
	
	struct timeval tRef;
		
};

#endif
