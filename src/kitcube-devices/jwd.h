/***************************************************************************
                          simrandom.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef JWD_H
#define JWD_H


#include <cstdio>
#include <string>
#include <ctime>
#include <map>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif // of USE_MYSQL

#include <akutil/simpleserver.h>
#include <akutil/procDuration.h>


#include "daqdevice.h"



/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  */

class jwd : public DAQDevice {
public:
	/**  */
	jwd();
	/**  */
	~jwd();
	
	void setConfigDefaults();
	
	const char *getDataDir();
	
	void readHeader(const char *header);
	
	void writeHeader();
	
	void parseData(char *line, struct timeval *tData, float *sensorValue);
	
	void writeData();
	
	void copyRemoteData();
	
	const char *getDataFilename();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:


};

#endif
