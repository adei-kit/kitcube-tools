/***************************************************************************
                          simrandom.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef REGENWIPPE_H
#define REGENWIPPE_H

#include "daqasciidevice.h"


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  */

class regenwippe: public DAQAsciiDevice {
public:
	/**  */
	regenwippe();
	/**  */
	~regenwippe();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	void writeData();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:
    /** Last timestamp */
    long timestamp_old;
    /** Last sensor data */
	int sensor_value_old;
    
    /** Counter for lines to show after unclear data */
    int nShowData;

};

#endif
