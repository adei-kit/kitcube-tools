/***************************************************************************
                          simrandom.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef JWD_H
#define JWD_H

#include "daqasciidevice.h"


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  */

class jwd: public DAQAsciiDevice {
public:
	/**  */
	jwd();
	/**  */
	~jwd();
	
	void setConfigDefaults();
	
	void readHeader(const char *filename);
	
	void writeHeader();
	
	void parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	void writeData();
	
	void copyRemoteData();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:


};

#endif
