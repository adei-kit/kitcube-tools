/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Wed Jul 28 15:21:10 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#include "scintillometer.h"


sci::sci() {
}


sci::~sci() {
}


int sci::readHeader(const char *filename) {
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	noData = 999999;
	
	lenHeader = 0;	// no header
	
	lenDataSet = 171;	// 170 bytes + 1 for '\0' in fgets()
	
	profile_length = 0;
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
		sensor[i].data_format = "<scalar>";
	}
	
	return 0;
}


void sci::setConfigDefaults() {
	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
}


int sci::parseData(char *line, struct timeval *l_tData, double *sensorValue) {
	struct tm timestamp;
	char *puffer;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// read date and time
	puffer = strptime(line + 5,"%Y-%m-%d %T", &timestamp);
	
	if (puffer == NULL) {
		// if "strptime" fails, for SCI it does not automatically mean that there is an error
		// SCI uses an invalid time stamp format at midnight:
		// e.g. 2010-07-26 24:00:00 means 2010-07-27 00:00:00
		// we fix this:
		
		// search for "24:00:00"
		puffer = strstr(line, "24:00:00");
		
		if (puffer == NULL) {
			printf("Szintillometer: Error reading date and time string!\n");
			return -1;
		} else {
			if (debug >= 3)
				printf("Warning: wrong time stamp format in SCI data! Fixing...\n");
			
			// replace 24:00:00 by 23:59:59
			strncpy(puffer, "23:59:59", 8);
			
			// read date and time
			puffer = strptime(line + 5,"%Y-%m-%d %T", &timestamp);
			
			// get seconds since the Epoch and add the one missing second
			l_tData->tv_sec = timegm(&timestamp) + 1;	// FIXME: this function is a non-standard GNU extension, try to avoid it!
		}
	} else {
		// get seconds since the Epoch
		l_tData->tv_sec = timegm(&timestamp);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
	}
	
	// read sensor values
	sscanf(puffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
	       &sensorValue[0], &sensorValue[1], &sensorValue[2], &sensorValue[3],
	       &sensorValue[4], &sensorValue[5], &sensorValue[6], &sensorValue[7],
	       &sensorValue[8], &sensorValue[9], &sensorValue[10], &sensorValue[11],
	       &sensorValue[12], &sensorValue[13], &sensorValue[14], &sensorValue[15],
	       &sensorValue[16], &sensorValue[17], &sensorValue[18]);
	// TODO: error handling
	
	return 0;
}


unsigned int sci::getSensorGroup() {
	unsigned int number;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "RES") {
		number = 1;
		buffer = "time series";
	}
	
	return number;
}
