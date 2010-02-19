/***************************************************************************
                          simrandom.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef SIMRANDOM_H
#define SIMRANDOM_H


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
  * 
  */

class SimRandom : public DAQDevice  {
public: 
	/**  */
	SimRandom();
	/**  */
	~SimRandom();
	
	void setConfigDefaults();
		
	const char *getDataDir();
	
	/** Get time until next sample and it's id */
	void readHeader();
	
	void writeHeader();
	
    void parseData(char *line, struct timeval *tData, float *sensorValue);	
	
	void writeData();
	
private:	


};

#endif
