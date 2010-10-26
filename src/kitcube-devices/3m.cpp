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


void dreim::readHeader(const char *filename){
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	noData = 999999;
	
	if (sensorGroup == "data") {
		lenHeader = 0x7f;
		
		lenDataSet = 55;
		
		profile_length = 0;
		
		// List of sensors
		nSensors = 5;
		sensor = new struct sensorType [nSensors];
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
		}
		
		sensor[0].comment = "air temperature";
		sensor[0].data_format = "<scalar>";
		
		sensor[1].comment = "relative humidity";
		sensor[1].data_format = "<scalar>";
		
		sensor[2].comment = "air pressure at surface";
		sensor[2].data_format = "<scalar>";
		
		sensor[3].comment = "battery voltage";
		sensor[3].data_format = "<scalar>";
		
		sensor[4].comment = "precipitation sum";
		sensor[4].data_format = "<scalar>";
		
		if (debug) {
			for (int i = 0; i < nSensors; i++) {
				printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
			}
		}
	} else if (sensorGroup == "gps") {
		lenHeader = 128;
		
		lenDataSet = 128;	// inkl. kleiner Puffer, hier OK, da fgets nach "\n" stoppt
		
		profile_length = 0;
		
		// List of sensors
		nSensors = 8;
		sensor = new struct sensorType [nSensors];
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
		}
		
		sensor[0].comment = "Latitude";
		sensor[0].data_format = "<scalar>";
		
		sensor[1].comment = "Longitude";
		sensor[1].data_format = "<scalar>";
		
		sensor[2].comment = "Altitude";
		sensor[2].data_format = "<scalar>";
		
		sensor[3].comment = "GPS timestamp";
		sensor[3].data_format = "<scalar>";
		
		sensor[4].comment = "time difference median";
		sensor[4].data_format = "<scalar>";
		
		sensor[5].comment = "time difference max";
		sensor[5].data_format = "<scalar>";
		
		sensor[6].comment = "time difference min";
		sensor[6].data_format = "<scalar>";
		
		sensor[7].comment = "Time correction";
		sensor[7].data_format = "<scalar>";
		
		if (debug) {
			for (int i = 0; i < nSensors; i++) {
				printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
			}
		}
	} else if (sensorGroup == "sonic") {
		lenHeader = 74;
		
		lenDataSet = 64;	// including some extra bytes!
		
		profile_length = 0;
		
		// list of sensors
		nSensors = 4;
		sensor = new struct sensorType [nSensors];
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
		}
		
		sensor[0].comment = "wind speed, east";
		sensor[0].data_format = "<scalar>";
		
		sensor[1].comment = "wind speed, north";
		sensor[1].data_format = "<scalar>";
		
		sensor[2].comment = "vertical wind speed";
		sensor[2].data_format = "<scalar>";
		
		sensor[3].comment = "virtual temperature";
		sensor[3].data_format = "<scalar>";
		
		if (debug) {
			for (int i = 0; i < nSensors; i++) {
				printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
			}
		}
	} else {
		printf("Unknown sensor group!\n");
	}
}


void dreim::setConfigDefaults(){
}


void dreim::parseData(char *line, struct timeval *l_tData, double *sensorValue){
	struct tm timestamp, gps_timestamp;
	char *puffer, *saveptr;
	double dummy;
	int msec;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// TODO/FIXME: are data timestamps in UTC or local time?
	
	if (sensorGroup == "data") {
		// read date and time
		puffer = strptime(line, "%Y-%m-%d,%T", &timestamp);
		
		// read dummy value "period"
		puffer = strtok(puffer, ",");
		
		// read the 5 sensor values
		for (int i = 0; i < 5; i++) {
			puffer = strtok(NULL, ",\r\n");
			if (strcmp(puffer, "nan") == 0) {
				sensorValue[i] = noData;
			} else {
				sscanf(puffer, "%lf", &sensorValue[i]);
			}
		}
	} else if (sensorGroup == "gps") {
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
		
		// read GPS latitude
		puffer = strtok_r(NULL, ",\"", &saveptr);
		if (strcmp(puffer, "nan") == 0) {
			sensorValue[0] = noData;
		} else {
			sensorValue[0] = convert_coordinate(puffer);
		}
		
		// read GPS longitude
		puffer = strtok_r(NULL, ",\"", &saveptr);
		if (strcmp(puffer, "nan") == 0) {
			sensorValue[1] = noData;
		} else {
			sensorValue[1] = convert_coordinate(puffer);
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
			puffer = strtok_r(NULL, ",", &saveptr);
			if (strcmp(puffer, "nan") == 0) {
				sensorValue[i] = noData;
			} else {
				sscanf(puffer, "%lf", &sensorValue[i]);
			}
		}
	} else if (sensorGroup == "sonic") {
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
		
		// read dummy value "period" and sensor values of one line of data
		sscanf(puffer, ",%lf,%lf,%lf,%lf,%lf",
		       &dummy,
		       &sensorValue[0], &sensorValue[1], &sensorValue[2], &sensorValue[3]);
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