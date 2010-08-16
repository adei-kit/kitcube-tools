/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Wed Jul 28 15:21:10 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#include "scintillometer.h"


sci::sci(){
	
	moduleType = "JWD";
	moduleNumber = 0;
	this->sensorGroup = "txt";
	
	this->iniGroup = "JWD-dd";
}


sci::~sci(){
}


void sci::readHeader(const char *filename){
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	noData = 999999;
	
	lenHeader = 0;	// no header
	
	lenDataSet = 0xAB;
	
	profile_length = 0;
	
	// List of sensors
	nSensors = 19;
	sensor = new struct sensorType [nSensors];
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
	}
	
	sensor[0].comment = "air pressure";
	sensor[0].data_format = "<scalar>";
	
	sensor[1].comment = "air temperature";
	sensor[1].data_format = "<scalar>";
	
	sensor[2].comment = "temperature difference";
	sensor[2].data_format = "<scalar>";
	
	sensor[3].comment = "path length";
	sensor[3].data_format = "<scalar>";
	
	sensor[4].comment = "instrument height";
	sensor[4].data_format = "<scalar>";
	
	sensor[5].comment = "logarithm of the amplitude in ch 1";
	sensor[5].data_format = "<scalar>";
	
	sensor[6].comment = "logarithm of the amplitude in ch 2";
	sensor[6].data_format = "<scalar>";
	
	sensor[7].comment = "correlation of the logs of the amps in ch 1 and 2";
	sensor[7].data_format = "<scalar>";
	
	sensor[8].comment = "error free data periods";
	sensor[8].data_format = "<scalar>";
	
	sensor[9].comment = "structure function constant of refractive index fluctuations";
	sensor[9].data_format = "<scalar>";
	
	sensor[10].comment = "inner scale of refractive fluctuations";
	sensor[10].data_format = "<scalar>";
	
	sensor[11].comment = "structure function constant of temperature fluctuations";
	sensor[11].data_format = "<scalar>";
	
	sensor[12].comment = "kinetic energy dissipation";
	sensor[12].data_format = "<scalar>";
	
	sensor[13].comment = "sensible heat flux, unstable density stratifiction";
	sensor[13].data_format = "<scalar>";
	
	sensor[14].comment = "sensible heat flux, stable density stratification";
	sensor[14].data_format = "<scalar>";
	
	sensor[15].comment = "momentum flux, unstable density stratification";
	sensor[15].data_format = "<scalar>";
	
	sensor[16].comment = "momentum flux, stable density stratification";
	sensor[16].data_format = "<scalar>";
	
	sensor[17].comment = "Monin Obukhov length, unstable density stratification";
	sensor[17].data_format = "<scalar>";
	
	sensor[18].comment = "Monin Obukhov length, stable density stratification";
	sensor[18].data_format = "<scalar>";
	
	if (debug) {
		for (int i = 0; i < nSensors; i++) {
			printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
		}
	}
}


void sci::setConfigDefaults(){
	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
}


void sci::parseData(char *line, struct timeval *l_tData, double *sensorValue){
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
			return;
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
}


unsigned int sci::getSensorGroup(){
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
