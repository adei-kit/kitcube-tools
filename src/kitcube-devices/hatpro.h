/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Mon Jul  4 16:00:22 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#ifndef HATPRO_H
#define HATPRO_H

#include "daqasciidevice.h"


class hatpro: public DAQAsciiDevice {
public:
	/**  */
	hatpro();
	/**  */
	~hatpro();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();

private:
	int read_ascii_line(char *buffer, int length, FILE *file_ptr);
};

#endif
