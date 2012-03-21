/********************************************************************
* Description:
* Author: Andreas Kopmann
* Created at: Thu Nov 15 20:50:20 CEST 2011
*    
* Copyright (c) 2011 Andreas Kopmann  All rights reserved.
*
********************************************************************/


#include "orcaprocess.h"

#include <string.h>


OrcaProcess::OrcaProcess()
{
	nMap = 0; // No channels defined
}


OrcaProcess::~OrcaProcess()
{
}


int OrcaProcess::readHeader(const char *filename)
{
	int i;
	FILE *data_file_ptr;
	char *line_of_data = NULL;
	size_t len = 0;
	
	
	// Format of the ORCA process values archiv file
	// # SampleTime	Pac,1,0 SampleTime	Pac,1,1 SampleTime	Pac,1,2 SampleTime	Pac,1,3
	// 
	// Note: The repeated sampleTime is a bug. There is no column in the data
	//       The header can appear multiple times in the file
	//
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	
	// read 1st line of header to get start time
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return -1;
	}
	
	// close data file
	fclose(data_file_ptr);
	
	// Get the ID's of process channels
	int j;
	char *pCh;
	char *pChRes;
	char buffer[256];
	
	// TODO: Allocate map dynamically (currently it's 32) !!!
	
	i = 0;
	pCh = strtok(line_of_data, "\t");
	if (pCh) pCh = strtok(NULL, "\t");
	while (pCh && (i < nSensors) && (i<32)){
		
		// Strip the trailing "sampletime"
		strcpy(buffer, pCh);
		pChRes = strstr(buffer, " SampleTime");
		if (pChRes){
			*pChRes = 0;
			printf("Warning: Drop wrong text in the column names (ORCA bug)\n");
		}
		if (debug > 3) printf("Searching for column: %s\n", buffer);

		map[i] = -1;
		for (j=0;j<nSensors;j++){
			pChRes = strstr(buffer, sensor[j].comment.c_str());
			if (pChRes == buffer) map[i] = j;
		}
	
		if (debug) 
			printf("Channel %d: %s --> %d\n", i, buffer, map[i]);
	
		if (map[i] == -1) {
			printf("OrcaProcess: Error reading sensor configuration!\n");
			printf("             Sensor name %s not found in sensor definition file", buffer);
			free(line_of_data);
			
			throw(std::invalid_argument("Sensor name not found in definition file"));
			return(1); // TODO: Howto stop the application here???
		}
	
		// Read the next token
		i++;
		pCh = strtok(NULL, "\t\n");

	}
	nMap = i;

		
	// Read first line to get the start time???
	// --> Is this required???
	
	// get seconds since the Epoch
	//start_time.tv_sec = timegm(&start_time_l);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
	start_time.tv_sec = 0;
	
	noData = 99999;
	
	// Can be detemined?!
	lenHeader = 0;	// CAUTION: header contains 11 lines
	
	lenDataSet = 0;	// 198 bytes + 1 for '\0' in fgets();
				// this is only one line of data,
				// a data set consists of "number of height levels" lines
	
	profile_length = 0;	// scalar data
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
		sensor[i].data_format = "<scalar>";
	}
	
	//free(line_of_data);
	
	return 0;
}


void OrcaProcess::setConfigDefaults()
{
}


unsigned int OrcaProcess::getSensorGroup()
{
	unsigned int number;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "dat") {
		number = 1;
		buffer = "Main Data";
	}
	
	return number;
}


int OrcaProcess::parseData(char *line, struct timeval *l_tData, double *sensorValue)
{
	int i;
	char *puffer;
	double value;
	int err;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
		
	
	// Check if there is a new header line
	// Drop # lines
	if (line[0] == '#'){
		if (debug > 3) printf("Found another header\n");
		
		// TODO: Re-read the configuration?!
		//       With this version a change of the configuration 
		//       requires the creation a new file
		
		return -1; 
	}
	
	// Write no data to all other variables
	for (i=0; i <nSensors;i++){
		sensorValue[i] = noData;
	}
	
	// read sensor values
	i = 0;
	
	// Read timestamp
	puffer = strtok(line, "\t");
	if (puffer) {
		err = sscanf(puffer, "%ld", &l_tData->tv_sec);
		if (err == 1) {
			if (debug > 2) printf("Timestamp %ld\n", l_tData->tv_sec);
		}
		
		puffer = strtok(NULL, "\t\n");
	}
	while ((puffer != NULL) && (i<nMap)) {
		err = sscanf(puffer, "%lf", &value);
		
		if (err == 1) {
			sensorValue[map[i]] = value;
		}
		
		if (debug > 2) printf("Sensor %d: %f\n", map[i], value);
		
		// Get next value
		puffer = strtok(NULL, "\t\n");
		i++;
	}
	
	return 0;
}
