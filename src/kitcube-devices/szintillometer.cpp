/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Wed Jul 28 15:21:10 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#include "szintillometer.h"


szi::szi(){
	
	moduleType = "JWD";
	moduleNumber = 0;
	this->sensorGroup = "txt";
	
	this->iniGroup = "JWD-dd";
}


szi::~szi(){
}


void szi::readHeader(const char *filename){
	
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


void szi::setConfigDefaults(){
	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
}


void szi::parseData(char *line, struct timeval *l_tData, float *sensorValue){
	struct tm timestamp;
	char* puffer;
	float dummy;
	
	// dummy read data set number
	sscanf(line, "%f", &dummy);
	
	// read date and time
	puffer = strptime(line + 5,"%Y-%m-%d %T", &timestamp);
	if (puffer == NULL) {
		printf("Szintillometer: Error reading date and time string!\n");
		return;
	}
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
	
	// read sensor values
	sscanf(puffer, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
	       &sensorValue[0], &sensorValue[1], &sensorValue[2], &sensorValue[3],
	       &sensorValue[4], &sensorValue[5], &sensorValue[6], &sensorValue[7],
	       &sensorValue[8], &sensorValue[9], &sensorValue[10], &sensorValue[11],
	       &sensorValue[12], &sensorValue[13], &sensorValue[14], &sensorValue[15],
	       &sensorValue[16], &sensorValue[17], &sensorValue[18]);
}


void szi::copyRemoteData(){
	struct timeval t0, t1;
	struct timezone tz;
	int err, pos;
	char line[256];
	std::string output, data_files_wildcard;
	
	
	if (debug > 2)
		printf("_____szi::copyRemoteData()_____\n");
	
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
		rsyncArgs.c_str(), data_files_wildcard.c_str(), remoteDir.c_str(), getDataDir(),
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
		printf("Rsync duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));
}


unsigned int szi::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	if (sensorGroup == "RES") {
		number = 1;
		buffer = "time series";
	}
	
	return number;
}
