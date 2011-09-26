/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Tue Jun 14 16:02:13 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#ifndef WINDCUBE_H
#define WINDCUBE_H

#include "daqasciidevice.h"

#include <endian.h>


class windcube: public DAQAsciiDevice {
public:
	/**  */
	windcube();
	
	/**  */
	~windcube();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
	void readData(std::string full_filename);
	
	int parseData(char *buffer,struct timeval *l_tData, double* sensor_values);
	
private:
	/** */
	double elevation_angle;
	
	double *altitudes;
	
	int initial_position;
	
	struct timeval initial_timestamp;
	
	double gain;
	
	int create_data_table_name(std::string & data_table_name);
};

#endif
