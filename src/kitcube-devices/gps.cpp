/********************************************************************
* Description: GPS device for humidity measurement
* Author: Norbert Flatinger, IPE
* Created at: Mon Feb 14 15:04:08 CET 2011
*    
* Copyright (c) 2011 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#include "gps.h"


gps::gps() {
}


gps::~gps() {
}


int gps::readHeader(const char *filename) {
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	noData = 999999;
	
	lenHeader = 0xACC;	// 2764 bytes
	
	lenDataSet = 0x110;	// 109 bytes + 1 for '\0' in fgets()
	
	profile_length = 0;
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
		sensor[i].data_format = "<scalar>";
        sensor[i].size = 1; 
	}

	return 0;
}


void gps::setConfigDefaults() {
}


int gps::parseData(char *line, struct timeval *l_tData, double *sensorValue) {
	struct tm timestamp = {0};
	char *puffer;
	int seconds_of_day;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// check if we have a data set line
	if (strstr(line, "KITK") == NULL)
		return -1;
	
	// read year and day of year
	puffer = strptime(line + 9, "%y:%j:", &timestamp);
	
	
	// read seconds of day and sensor values
	sscanf(puffer, "%d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
	       &seconds_of_day, &sensorValue[0], &sensorValue[1], &sensorValue[2],
	       &sensorValue[3], &sensorValue[4], &sensorValue[5], &sensorValue[6],
	       &sensorValue[7], &sensorValue[8], &sensorValue[9], &sensorValue[10],
	       &sensorValue[11], &sensorValue[12], &sensorValue[13]);
	// TODO: error handling
	
	// check quality flag
	if (strstr(line, "DELETED") != NULL)
		sensorValue[14] = -1;
	else
		sensorValue[14] = 0;
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
	
	// add seconds of day and 7,5 minutes due to averaging
	l_tData->tv_sec += seconds_of_day + 450;
	
	return 0;
}


unsigned int gps::getSensorGroup() {
	unsigned int number;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "TRO") {
		number = 1;
		buffer = "time series";
	}
	
	return number;
}
