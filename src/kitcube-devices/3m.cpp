/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Mon Aug  2 13:49:30 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#include "3m.h"


dreim::dreim(){
}


dreim::~dreim(){
}


int dreim::readHeader(const char *filename) {
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	noData = 999999;
	
	if (sensorGroup == "data") {
		lenHeader = 0x7f;	// 127 bytes
		
		lenDataSet = 55;	// 54 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
		
	} else if (sensorGroup == "gps") {
		lenHeader = 0x80;	// 128 bytes
		
		lenDataSet = 128;	// 106 - 109 bytes plus small buffer; here OK, as fgets stops after "\n"
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
		
	} else if (sensorGroup == "sonic") {
		lenHeader = 0x4a;	// 74 bytes
		
		lenDataSet = 64;	// 45 -51 bytes plus small buffer; here OK, as fgets stops after "\n"
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
		
	} else {
		printf("Unknown sensor group!\n");
	}
	
	return 0;
}


void dreim::setConfigDefaults(){
}


void dreim::parseData(char *line, struct timeval *l_tData, double *sensorValue){
	struct tm timestamp, gps_timestamp;
	char *puffer, *saveptr;
	int msec;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// TODO/FIXME: are data timestamps in UTC or local time?
	
	// read date and time
	puffer = strptime(line, "%Y-%m-%d,%T", &timestamp);
	
	// get ms part of time stamp, if there is one
	if (*puffer == '.') {
		// read ms part of time stamp
		sscanf(puffer, ".%d", &msec);
		
		l_tData->tv_usec = msec * 1000;
		
		// set pointer to "begin of data" as if there was no ms part
		puffer = strchr(puffer, ',');
	} else if (*puffer == ',') {
		l_tData->tv_usec = 0;
	} else {
		printf("Error: unknown data format!\n");
	}
	
	// read dummy value "period"
	puffer = strtok_r(puffer, ",", &saveptr);
	
	if (sensorGroup == "data") {
		// read the 5 sensor values
		for (int i = 0; i < 5; i++) {
			puffer = strtok_r(NULL, ",\r\n", &saveptr);
			if (strcmp(puffer, "nan") == 0) {
				sensorValue[i] = noData;
			} else {
				sscanf(puffer, "%lf", &sensorValue[i]);
			}
		}
	} else if (sensorGroup == "gps") {
		// read GPS latitude and longitude
		for (int i = 0; i < 2; i++) {
			puffer = strtok_r(NULL, ",\"", &saveptr);
			if (strcmp(puffer, "nan") == 0) {
				sensorValue[i] = noData;
			} else {
				sensorValue[i] = convert_coordinate(puffer);
			}
		}
		
		// read GPS altitude
		puffer = strtok_r(NULL, ",", &saveptr);
		if (strcmp(puffer, "nan") == 0) {
			sensorValue[2] = noData;
		} else {
			sensorValue[2] = atof(puffer);
		}
		
		// read GPS timestamp
		puffer = strtok_r(NULL, ",", &saveptr);
		if (strcmp(puffer, "nan") == 0) {
			sensorValue[3] = noData;
			puffer = strtok_r(NULL, ",", &saveptr);
		} else {
			puffer = strptime(puffer, "%Y-%m-%d", &gps_timestamp);
			puffer = strtok_r(NULL, ",", &saveptr);
			if (strcmp(puffer, "nan") == 0) {
				sensorValue[3] = noData;
			} else {
				puffer = strptime(puffer, "%T", &gps_timestamp);
				sensorValue[3] = timegm(&gps_timestamp);	// FIXME: function is non-standard GNU extension
			}
		}
		
		// read time difference median, max, min and time correction
		for (int i = 4; i < 8; i++) {
			puffer = strtok_r(NULL, ",\r\n", &saveptr);
			if (strcmp(puffer, "nan") == 0) {
				sensorValue[i] = noData;
			} else {
				sscanf(puffer, "%lf", &sensorValue[i]);
			}
		}
	} else if (sensorGroup == "sonic") {
		// read the 4 sensor values
		for (int i = 0; i < 4; i++) {
			puffer = strtok_r(NULL, ",\r\n", &saveptr);
			if (strcmp(puffer, "nan") == 0) {
				sensorValue[i] = noData;
			} else {
				sscanf(puffer, "%lf", &sensorValue[i]);
			}
		}
	}
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: function is non-standard GNU extension
}


unsigned int dreim::getSensorGroup(){
	unsigned int number;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "data") {
		number = 1;
		buffer = "time series";
	}
	
	if (sensorGroup == "gps") {
		number = 2;
		buffer = "profile";
	}
	
	if (sensorGroup == "sonic") {
		number = 3;
		buffer = "profile";
	}
	
	return number;
}


// convert coordinate of format "48 N 31.3385" to double number
double dreim::convert_coordinate(char *coordinate_string) {
	char *tmp, *saveptr;
	double degree, minute, coordinate;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// get degree value
	tmp = strtok_r(coordinate_string, " ", &saveptr);
	degree = atof(tmp);
	
	// determine sign by evaluating get N/E or S/W
	tmp = strtok_r(NULL, " ", &saveptr);
	switch (*tmp) {
	case 'N':
	case 'E':
		coordinate = degree;
		break;
	case 'S':
	case 'W':
		coordinate = -degree;
		break;
	default:
		printf("Error: invalid coordinates!\n");
	}
	
	// get minutes value
	tmp = strtok_r(NULL, " ", &saveptr);
	minute = atof(tmp);
	
	// calculate coordinate number
	coordinate += minute / 60.;
	
	return coordinate;
}