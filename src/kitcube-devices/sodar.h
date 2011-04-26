/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Tue Mar  1 15:26:41 CET 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#ifndef SODAR_H
#define SODAR_H

#include "daqasciidevice.h"
#include <cmath>


class sodar: public DAQAsciiDevice {
public:
	/**  */
	sodar();
	
	/**  */
	~sodar();
	
	void setConfigDefaults();
	
	int readHeader(const char *filename);
	
	//void writeHeader();
	
	//int parseData(char *line, struct timeval *l_tData, double *sensorValue);
	
	//void writeData();
	
	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
	int create_data_table();
	
	void readData(std::string full_filename);
	
	int parseData(char *buffer, double* sensor_values, int height_no);
	
	int read_ascii_line(char *buffer, int length, FILE *file_ptr);
	
	

private:
	/** */
	double *gap_value;
	
	int convert_gap_values(double* sensor_values);

};

#endif
