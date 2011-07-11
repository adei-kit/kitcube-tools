/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Mon Jul  4 16:00:22 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#include "hatpro.h"


hatpro::hatpro(){
}


hatpro::~hatpro(){
}


int hatpro::readHeader(const char *filename) {
	FILE *data_file_ptr;
	char line_of_data[2048];
	char *buf;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	
	// find out header length
	// read lines until you find the last line of the header
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(line_of_data, 2048, data_file_ptr) == -1)
			return -1;
		
		if (sensorGroup == "HKD")
			buf = strstr(line_of_data, "#Ye,Mo,Da");
		else
			buf = strstr(line_of_data, "# Ye , Mo , Da");
		
	}
	lenHeader = ftell(data_file_ptr);
	
	// close data file
	fclose(data_file_ptr);
	
	
	noData = 999999;
	
	if (sensorGroup == "LWP") {
		lenDataSet = 62;	// 61 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
	} else if (sensorGroup == "IWV") {
		lenDataSet = 61;	// 60 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
	} else if (sensorGroup == "MET") {
		lenDataSet = 59;	// 58 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
	} else if (sensorGroup == "CBH") {
		lenDataSet = 44;	// 41 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
		
	} else if (sensorGroup == "HKD") {
		lenDataSet = 209;	// 208 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
		
	} else if (sensorGroup == "sonic") {
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


void hatpro::setConfigDefaults(){
}


int hatpro::parseData(char *line, struct timeval *l_tData, double *sensorValue){
	struct tm timestamp;
	char *puffer;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// read date and time
	puffer = strptime(line, "%y , %m , %d , %H , %M , %S", &timestamp);
	
	
	// read some dummy
	puffer = strtok(puffer - 1, ",");
	// read sensor values
	for (int i = 0; i < nSensors; i++) {
		puffer = strtok(NULL, ",\r\n");
		sensorValue[i] = strtod(puffer, NULL);
	}
	
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: function is non-standard GNU extension
	
	return 0;
}


unsigned int hatpro::getSensorGroup(){
	unsigned int number;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "LWP") {
		number = 1;
		buffer = "time series";
	}
	
	if (sensorGroup == "IWV") {
		number = 2;
		buffer = "profile";
	}
	
	if (sensorGroup == "MET") {
		number = 3;
		buffer = "profile";
	}
	
	if (sensorGroup == "TPC") {
		number = 4;
		buffer = "profile";
	}
	
	if (sensorGroup == "TPB") {
		number = 5;
		buffer = "profile";
	}
	
	if (sensorGroup == "HPC") {
		number = 6;
		buffer = "profile";
	}
	
	if (sensorGroup == "LPR") {
		number = 7;
		buffer = "profile";
	}
	
	if (sensorGroup == "STA") {
		number = 8;
		buffer = "profile";
	}
	
	if (sensorGroup == "CBH") {
		number = 9;
		buffer = "profile";
	}
	
	if (sensorGroup == "CMP") {
		number = 10;
		buffer = "profile";
	}
	
	if (sensorGroup == "HKD") {
		number = 11;
		buffer = "profile";
	}
	
	return number;
}


int hatpro::read_ascii_line(char *buffer, int length, FILE *file_ptr) {
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


long hatpro::getFileNumber(char* filename)
{
	std::string filename_prefix;
	std::string filename_suffix;
	std::string filename_string;
	size_t pos_index, pos_prefix, pos_suffix, length_prefix, length_suffix, filename_length, pos;
	long index;
	
	
	if (debug >= 3) {
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
		printf("... from filename '%s'\n", filename);
	}
	
	
	// Find <index> in data template
	pos_index = datafileMask.find("<index>");
	// FIXME: this is done in DAQDevice::readInifile already. Why repeat it here? What to do in case of error?
	if (pos_index == std::string::npos) {
		printf("Error: There is no tag <index> in datafileMask '%s' specified in inifile %s\n",
			datafileMask.c_str(), inifile.c_str());
	}
	
	//filename_prefix = datafileMask.substr(0, pos_index);
	//length_prefix = filename_prefix.length();
	filename_suffix = datafileMask.substr(pos_index + 7);
	length_suffix = filename_suffix.length();
	
	if (debug >= 4)
		printf("Position of <index> in %s is: %ld -- Prefix is: %s, suffix is: %s\n",
		       datafileMask.c_str(), pos_index, filename_prefix.c_str(), filename_suffix.c_str());
	
	filename_string = filename;
	
	// find "_" as and of prefix in filename and removewhole prefix
	length_prefix = filename_string.find("_") + 1;
	if (length_prefix != std::string::npos) {
		filename_string.erase(0, length_prefix);
	} else {
		if (debug >= 3)
			printf("No '_' as end of prefix found in filename!\n");
		return 0;
	}
	
	// if there is a suffix, check for suffix at end of filename and delete it
	if (filename_suffix.size() != 0) {
		pos_suffix = filename_string.find(filename_suffix);
		filename_length = filename_string.length();
		if ((pos_suffix != std::string::npos) && ((pos_suffix + length_suffix) == filename_length)) {
			filename_string.erase(pos_suffix, length_suffix);
		} else {
			if (debug >= 3)
				printf("Suffix not found or not at end of file name!\n");
			return 0;
		}
	}
	
	// remove "_" from file names
	pos = filename_string.find('_');
	while(pos != std::string::npos) {
		filename_string.erase(pos, 1);
		pos = filename_string.find('_');
	}
	
	// we assume, that after the removal of prefix and suffix, there are only numbers left
	// FIXME/TODO: check, if this is really only a number
	index = atol(filename_string.c_str());
	
	if (debug >= 3)
		printf("File number is: %ld\n", index);
	
	return index;
}
