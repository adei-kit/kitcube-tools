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
	char *line_of_data = NULL;
	size_t len = 0;
	struct tm start_time_l = {0};
	char* puffer;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	
    // TODO: Find out if it's old or new format
    // The new format does not contain any header the reference time
    // need to be estimated from the file name
    // 
    
    
	// read 1st line of header to get start time
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return -1;
	}
	
	// close data file
	fclose(data_file_ptr);
	
	// get start date and time
	puffer = strptime(line_of_data, "Datum : %d.%m.%Y\tStartzeit: %T", &start_time_l);
	if (puffer == NULL) {
        
        // Check if's the new data format wo header
        if (strstr(line_of_data, "Zeit") == line_of_data) {
            // Parse date from filename
            long int index;
            char *ptrName; 
            
            puffer = NULL;
            ptrName = strrchr((char *) filename, '/');
            if (ptrName == NULL) 
                ptrName = (char *) filename;
            else 
                ptrName++;
            index = getFileNumber(ptrName);
            
            char strIndex[20];
            sprintf(strIndex, "%ld", index);
            puffer = strptime(strIndex, "%Y%m%d%H", &start_time_l);
            
            //printf("%ld --> %02d.%02d.%4d %02d:%02d:%02d \n", index, 
            //       start_time_l.tm_mday, start_time_l.tm_mon+1, start_time_l.tm_year+1900,
            //       start_time_l.tm_hour, start_time_l.tm_min, start_time_l.tm_sec);
        }          
       
	}

	if (puffer == NULL) {
        printf("Radiosonde: Error reading date and time string!\n");
        free(line_of_data);
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
		sensor[i].data_format = "<scalar>";
        sensor[i].size = 1;
	}
	
	free(line_of_data);
	
	return 0;
}


void radiosonde::setConfigDefaults()
{
}


unsigned int radiosonde::getSensorGroup()
{
	unsigned int number;
	
	
	if (debug > 2)
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
    l_tData->tv_sec = start_time.tv_sec;
    l_tData->tv_usec = 0;
	
	// read minutes and seconds since the start of the measurement
	if (sscanf(line, "%d:%d", &min, &sec) != 2)
		return -1;
	
	// add to start time
	l_tData->tv_sec += min * 60 + sec;
	
	
	// read sensor values
	puffer = strtok(line + 8, " \t");
	i = 0;
	while (puffer != NULL) {
		if (sscanf(puffer, "%lf", sensorValue + i) != 1){
            *(sensorValue + i) = noData;
        }
		puffer = strtok(NULL, " \t\r\n");
		i++;
	}
	
	// check if all sensor values could be read
	/*if (i != nSensors)
		return -1;*/
	
	return 0;
}
