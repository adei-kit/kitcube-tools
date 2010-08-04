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
		nSensors = 9;
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
		
		sensor[3].comment = "GPS date";
		sensor[3].data_format = "<scalar>";
		
		sensor[4].comment = "GPS time";
		sensor[4].data_format = "<scalar>";
		
		sensor[5].comment = "time difference median";
		sensor[5].data_format = "<scalar>";
		
		sensor[6].comment = "time difference max";
		sensor[6].data_format = "<scalar>";
		
		sensor[7].comment = "time difference min";
		sensor[7].data_format = "<scalar>";
		
		sensor[8].comment = "Time correction";
		sensor[8].data_format = "<scalar>";
		
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
	struct tm timestamp;
	char *puffer, *tmp, *saveptr;
	double dummy;
	int msec;
	
	
	// TODO/FIXME: are data timestamps in UTC or local time?
	
	if (sensorGroup == "data") {
		// read date and time
		puffer = strptime(line, "%Y-%m-%d,%T", &timestamp);
		
		// read sensor values of one line of data
		sscanf(puffer, ",%lf,%lf,%lf,%lf,%lf,%lf",
		       &dummy,
		       &sensorValue[0], &sensorValue[1], &sensorValue[2],
		       &sensorValue[3], &sensorValue[4]);
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
		
		// read dummy value
		strtok_r(puffer, ",", &saveptr);
		
		// read GPS latitude
		tmp = strtok_r(NULL, ",\"", &saveptr);
		sensorValue[0] = convert_coordinate(tmp);
		
		// read GPS longitude
		tmp = strtok_r(NULL, ",\"", &saveptr);
		sensorValue[1] = convert_coordinate(tmp);
		
		// read GPS altitude
		tmp = strtok_r(NULL, ",\"", &saveptr);
		if (strcmp(tmp, "nan") == 0) {
			sensorValue[2] = noData;
		} else {
			sensorValue[2] = atof(tmp);
		}
		
		// read GPS date
		tmp = strtok_r(NULL, ",\"", &saveptr);
		
		
		
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
		
		// read sensor values of one line of data
		sscanf(puffer, ",%lf,%lf,%lf,%lf,%lf",
		       &dummy,
		       &sensorValue[0], &sensorValue[1], &sensorValue[2], &sensorValue[3]);
	}
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: function is non-standard GNU extension
}


void dreim::copyRemoteData(){
	struct timeval t0, t1;
	struct timezone tz;
	int err, pos;
	char line[256];
	std::string output, data_files_wildcard;
	
	
	if (debug > 2)
		printf("_____dreim::copyRemoteData()_____\n");
	
	createDirectories((archiveDir + getDataDir()).c_str());
	
	// Transfer only files of the specified sensor group (== file type)
	// The time to find changes in large file increases with the file size
	// Therefore it is necessary to use the rsync option --append. This option
	// will make rsync assume that exisitings files will only be added at the end.
	//
	// TODO: Improve output of the system call. The output is mixed with others
	//       Solution 1: Write output to file,
	//       Solution 2: Use pipes (example can be found in README
	//
	if (debug > 2)
		output = "";
	else
		output = " > /dev/null";
	
	data_files_wildcard = datafileMask;
	pos = data_files_wildcard.find("<index>");
	data_files_wildcard.replace(pos, 7, "*");
	
	sprintf(line, "rsync -avz %s --include='*/' --include='%s' --exclude='*' %s%s  %s%s %s",
			rsyncArgs.c_str(), data_files_wildcard.c_str(),
			remoteDir.c_str(), getDataDir(),
			archiveDir.c_str(), getDataDir(), output.c_str());
	if (debug > 2)
		printf("%s\n", line);
	
	gettimeofday(&t0, &tz);
	err = system(line);
	gettimeofday(&t1, &tz);
	
	if (err != 0) {
		printf("Synchronisation error (rsync)\n");
		//throw std::invalid_argument("Synchronisation error (rsync)");
	}
	
	if (debug > 2)
		printf("Rsync duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec));
}


unsigned int dreim::getSensorGroup(){
	unsigned int number;
	
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


double dreim::convert_coordinate(char *coordinate_string) {
	char *tmp, *saveptr;
	double degree, minute, coordinate;
	
	// get degree value
	tmp = strtok_r(coordinate_string, " ", &saveptr);
	degree = atof(tmp);
	
	// get N/S or E/W
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
	
	//get minutes value
	tmp = strtok_r(NULL, " ", &saveptr);
	minute = atof(tmp);
	
	//
	coordinate += minute / 60.;
	
	return coordinate;
}