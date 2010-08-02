/***************************************************************************
                          simrandom.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef SIMRANDOM_H
#define SIMRANDOM_H

#include "daqasciidevice.h"

#include <math.h>


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  *
  */

class SimRandom: public DAQAsciiDevice  {
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
	
	void parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	void writeData();
	
	const char *getDataFilename();
	
private:

};

#endif
