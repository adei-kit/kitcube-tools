/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Thu Apr 21 10:27:20 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#include "radiosonde.h"


radiosonde::radiosonde()
{
}


radiosonde::~radiosonde()
{
}


int radiosonde::readHeader(const char *filename)
{
	FILE *data_file_ptr;
	char line_of_data[64];
	struct tm start_time_l = {0};
	char* puffer;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	
	// read 1st line of header to get start time
	if (read_ascii_line(line_of_data, 64, data_file_ptr) == -1)
		return -1;
		
	// close data file
	fclose(data_file_ptr);
	
	// get start date and time
	puffer = strptime(line_of_data, "Datum : %d.%m.%Y\tStartzeit: %T", &start_time_l);
	if (puffer == NULL) {
		printf("Regenwippe: Error reading date and time string!\n");
		return -1;
	}
	
	// get seconds since the Epoch
	start_time.tv_sec = timegm(&start_time_l);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
	
	
	noData = 99999;
	
	lenHeader = 0x1a1;	// CAUTION: header contains 11 lines
	
	lenDataSet = 199;	// 198 bytes + 1 for '\0' in fgets();
				// this is only one line of data,
				// a data set consists of "number of height levels" lines
	
	profile_length = 0;	// scalar data
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
		sensor[0].data_format = "<scalar>";
	}
	
	return 0;
}


void radiosonde::setConfigDefaults()
{
}


unsigned int radiosonde::getSensorGroup()
{
	unsigned int number;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "txt") {
		number = 1;
		buffer = "Main Data";
	}
	
	return number;
}


int radiosonde::parseData(char *line, struct timeval *l_tData, double *sensorValue)
{
	char *puffer;
	int min, sec;
	int i;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// save start time to data time stamp variable
	*l_tData = start_time;
	
	// read minutes and seconds since the start of the measurement
	if (sscanf(line, "%d:%d", &min, &sec) != 2)
		return -1;
	
	// add to start time
	l_tData->tv_sec += min * 60 + sec;
	
	
	// read sensor values
	puffer = strtok(line + 5, " ");
	i = 0;
	while (puffer != NULL) {
		if (sscanf(puffer, "%lf", sensorValue + i) != 1)
			*(sensorValue + i) = noData;
		puffer = strtok(NULL, " ");
		i++;
	}
	
	// check if all sensor values could be read
	/*if (i != nSensors)
		return -1;*/
	
	return 0;
}


int radiosonde::read_ascii_line(char *buffer, int length, FILE *file_ptr)
{
	if (fgets(buffer, length, file_ptr) == NULL) {
		printf("Error reading from file or EOF reached\n");
		return -1;
	}
	if (strchr(buffer, '\n') == NULL) {
		printf("Error: line read from file is not complete\n");
		return -1;
	}
	
	return 0;
}
