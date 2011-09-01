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
	altitudes = 0;
}


hatpro::~hatpro(){
	if (altitudes != 0)
		delete [] altitudes;
}


int hatpro::readHeader(const char *filename) {
	FILE *data_file_ptr;
	char *line_of_data = NULL;
	size_t len = 0;
	char *buf;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	
	// read number of samples in file
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
			free(line_of_data);
			return -1;
		}
		
		buf = strstr(line_of_data, "# Number of Samples");
	}
	number_of_samples = atoi(line_of_data);
	
	
	// read some header information from files containing profile data
	if ( (sensorGroup == "CMP") || (sensorGroup == "HPC") || (sensorGroup == "LPR") || (sensorGroup == "TPB") || (sensorGroup == "TPC") ) {
		// get profile length
		buf = NULL;
		while (buf == NULL) {
			if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
				free(line_of_data);
				return -1;
			}
			
			buf = strstr(line_of_data, "# Number of Altitude Levels");
		}
		profile_length = strtod(line_of_data, NULL);
		
		// get altitudes
		buf = NULL;
		while (buf == NULL) {
			if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
				free(line_of_data);
				return -1;
			}
			
			buf = strstr(line_of_data, "# Altitude Levels");
		}
		
		if (altitudes != 0)
			delete [] altitudes;
		altitudes = new double[profile_length];
		
		buf = strtok(line_of_data, ",");
		for (int i = 0; i < profile_length; i++) {
			altitudes[i] = strtod(buf, NULL);
			buf = strtok(NULL, ",");	// this return altitude values
		}
	}
	
	
	// find out header length
	// read lines until you find the last line of the header
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
			free(line_of_data);
			return -1;
		}
		
		if (sensorGroup == "HKD")
			buf = strstr(line_of_data, "#Ye,Mo,Da");
		else
			buf = strstr(line_of_data, "# Ye , Mo , Da");
	}
	lenHeader = ftell(data_file_ptr);
	
	// find second half of file for "HPC" data file
	if (sensorGroup == "HPC") {
		buf = NULL;
		while (buf == NULL) {
			if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
				free(line_of_data);
				return -1;
			}
			
			buf = strstr(line_of_data, "# Ye , Mo , Da");
		}
		pos_hpc_data = ftell(data_file_ptr);
	}
	
	// close data file
	fclose(data_file_ptr);
	
	
	noData = 999999;
	
	if (sensorGroup == "CBH") {
		lenDataSet = 44;	// 43 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
		
	} else if (sensorGroup == "CMP") {
		lenDataSet = 315;	// 314 bytes + 1 for '\0' in fgets()
		
		sensor[0].type = "";
		sensor[0].height = 0;
		sensor[0].data_format = "<scalar>";
		
		for (int i = 1; i < nSensors; i++) {
			sensor[i].type = "profile";
			sensor[i].height = 0;
			sensor[i].data_format = "<vector>";
		}
	} else if (sensorGroup == "HKD") {
		lenDataSet = 209;	// 208 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
		
	} else if (sensorGroup == "HPC") {
		lenDataSet = 347;	// 346 bytes + 1 for '\0' in fgets()
		
		sensor[0].type = "";
		sensor[0].height = 0;
		sensor[0].data_format = "<scalar>";
		
		for (int i = 1; i < nSensors; i++) {
			sensor[i].type = "profile";
			sensor[i].height = 0;
			sensor[i].data_format = "<vector>";
		}
	} else if (sensorGroup.substr(0, 3) == "IWV") {
		lenDataSet = 61;	// 60 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
	} else if (sensorGroup == "LPR") {
		lenDataSet = 771;	// 770 bytes + 1 for '\0' in fgets()
		
		sensor[0].type = "";
		sensor[0].height = 0;
		sensor[0].data_format = "<scalar>";
		
		for (int i = 1; i < nSensors; i++) {
			sensor[i].type = "profile";
			sensor[i].height = 0;
			sensor[i].data_format = "<vector>";
		}
	} else if (sensorGroup.substr(0, 3) == "LWP") {
		lenDataSet = 62;	// 61 bytes + 1 for '\0' in fgets()
		
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
	} else if (sensorGroup == "STA") {
		lenDataSet = 86;	// 85 bytes + 1 for '\0' in fgets()
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 5;
			sensor[i].data_format = "<scalar>";
		}
	} else if (sensorGroup == "TPB") {
		lenDataSet = 203;	// 202 bytes + 1 for '\0' in fgets()
		
		sensor[0].type = "";
		sensor[0].height = 0;
		sensor[0].data_format = "<scalar>";
		
		for (int i = 1; i < nSensors; i++) {
			sensor[i].type = "profile";
			sensor[i].height = 0;
			sensor[i].data_format = "<vector>";
		}
	} else if (sensorGroup == "TPC") {
		lenDataSet = 347;	// 346 bytes + 1 for '\0' in fgets()
		
		sensor[0].type = "";
		sensor[0].height = 0;
		sensor[0].data_format = "<scalar>";
		
		for (int i = 1; i < nSensors; i++) {
			sensor[i].type = "profile";
			sensor[i].height = 0;
			sensor[i].data_format = "<vector>";
		}
	} else {
		printf("Unknown sensor group!\n");
	}
	
	free(line_of_data);
	
	return 0;
}


void hatpro::setConfigDefaults(){
}


void hatpro::readData(std::string full_filename){
	char *buf = NULL;
	size_t len = 0;
	FILE *fd_data_file;
	double *local_sensorValue;
	int loop_counter = 0;
	FILE *fmark;
	std::string full_data_file_name;
	struct timeval lastTime;
	unsigned long lastPos;
	unsigned long currPos;
	long lastIndex, tmp_pos1, tmp_pos2;
	struct timeval time_stamp_tv = {0};
	
#ifdef USE_MYSQL
	std::string sql;
	char sData[50];
	char* esc_str;
#endif
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	if ( (sensorGroup == "CBH") || (sensorGroup == "HKD") || (sensorGroup.substr(0, 3) == "IWV") || (sensorGroup.substr(0, 3) == "LWP") || (sensorGroup == "MET") || (sensorGroup == "STA") )
		DAQAsciiDevice::readData(full_filename);
	else {
#ifdef USE_MYSQL
		if (db == 0) {
			openDatabase();
		} else {
			// Automatic reconnect
			if (mysql_ping(db) != 0){
				printf("Error: Lost connection to database - automatic reconnect failed\n");
				throw std::invalid_argument("Database unavailable\n");
			}
		}
#endif
		
		// Allocate memory for sensor values: 1 comes from header, 1 is scalar and only the rest are profile sensors
		local_sensorValue = new double[1 + (nSensors - 2) * profile_length];
		
		// Compile file name
		full_data_file_name = full_filename;
		
		// open data file
		if (debug >= 1)
			printf("Open data file: %s\n", full_data_file_name.c_str());
		fd_data_file = fopen(full_data_file_name.c_str(), "r");
		if (fd_data_file == NULL) {
			printf("Error opening data file %s\n", full_data_file_name.c_str());
			return;
		}
		
		
		// Get the last time stamp + file pointer from the marker file
		lastPos = 0;
		lastTime.tv_sec = 0;
		lastTime.tv_usec = 0;
		
		if (debug >= 1)
			printf("Get marker from %s\n", filenameMarker.c_str());
		fmark = fopen(filenameMarker.c_str(), "r");
		if (fmark > 0) {
			fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
			fclose(fmark);
			
			// Read back the data time stamp of the last call
			time_stamp_tv.tv_sec = lastTime.tv_sec;
			time_stamp_tv.tv_usec = lastTime.tv_usec;
			
			if (debug >= 1)
				printf("Last time stamp was %ld\n", lastTime.tv_sec);
		}
		
		// Find the beginning of the new data
		// if new file, jump to the first data set
		if (lastPos == 0)
			lastPos = lenHeader;
		currPos = lastPos;
		if (debug >= 1)
			printf("Last position in file: %ld\n", lastPos);
		fseek(fd_data_file, lastPos, SEEK_SET);
		
#ifdef USE_MYSQL
		sql = "LOCK TABLES " + dataTableName + " WRITE";
		mysql_query(db, sql.c_str());
#endif
		
		fd_eof = false;
		tmp_pos2 = pos_hpc_data;
		
		for (loop_counter = 0; loop_counter < number_of_samples; loop_counter++) {
			// read one line of ASCII data
			if (read_ascii_line(&buf, &len, fd_data_file) == -1) {
				fd_eof = true;
				break;
			}
			
			first_half = true;	// for parseData
			
			// parse data and check for error in especially in HPC files:
			// after restart of the application the last file is read
			// at the middle comment lines, this is caught here
			if (parseData(buf, &time_stamp_tv, local_sensorValue) == -1) {
				fd_eof = true;
				break;
			}
			
			if (sensorGroup == "HPC") {
				tmp_pos1 = ftell(fd_data_file);
				fseek(fd_data_file, tmp_pos2, SEEK_SET);
				
				// read one line of ASCII data
				if (read_ascii_line(&buf, &len, fd_data_file) == -1) {
					fd_eof = true;
					break;
				}
				
				first_half = false;	// for parseData
				
				// parse data
				parseData(buf, &time_stamp_tv, local_sensorValue + 1 + profile_length);
				
				tmp_pos2 = ftell(fd_data_file);
				fseek(fd_data_file, tmp_pos1, SEEK_SET);
			}
			
			// print sensor values
			if (debug >= 4) {
				printf("%4d: Received %4d bytes --- ", loop_counter, (int) strlen(buf));
				printf("%lds %6ldus --- ", time_stamp_tv.tv_sec, time_stamp_tv.tv_usec);
				for (int k = 0; k < profile_length; k++) {
					printf("%.0f ", altitudes[k]);
				}
				printf("%.0f ", local_sensorValue[0]);
				for (int j = 0; j < (nSensors - 2); j++) {
					for (int k = 0; k < profile_length; k++) {
						printf("%.2f ", local_sensorValue[1+ j * profile_length + k]);
					}
				}
				printf("\n");
			}
#ifdef USE_MYSQL
			//--------------------------------------------------------------
			// store data to DB
			//--------------------------------------------------------------
			sql = "INSERT INTO `" + dataTableName + "` (usec";
			
			// sensor names
			for (int i = 0 ; i < nSensors; i++)
				sql += ", `" + sensor[i].name + "`";
			
			sql += ") VALUES (";
			
			// time stamp
			sprintf(sData, "%ld", time_stamp_tv.tv_sec * 1000000 + time_stamp_tv.tv_usec);
			sql += sData;
			
			// rain flag
			sql += ", ";
			sprintf(sData, "%f", local_sensorValue[0]);
			sql += sData;

			//altitudes
			esc_str = new char[2 * profile_length * sizeof(double) + 1];
			sql += ", '";
			mysql_real_escape_string(db, esc_str, (const char*)altitudes, profile_length * sizeof(double));
			sql += esc_str;
			sql += "'";
			
			// sensor values
			for (int i = 0; i < (nSensors - 2); i++) {
				sql += ", '";
				mysql_real_escape_string(db, esc_str,
							(const char *)(local_sensorValue + 1 + i * profile_length),
							profile_length * sizeof(double));
				sql += esc_str;
				sql += "'";
			}
			delete [] esc_str;
			sql += ")";
			
			// execute SQL statement
			if (mysql_query(db, sql.c_str())) {
				printf("Error inserting data: %s\n", mysql_error(db));
				// TODO: error handling
			}
#endif // of USE_MYSQL
			// Get the position in this file
			currPos = ftell(fd_data_file);
		}
		
#ifdef USE_MYSQL
		sql = "UNLOCK TABLES";
		mysql_query(db, sql.c_str());
#endif
		fd_eof = true;
		
		processedData += currPos - lastPos;
		
		if (debug >= 1)
			printf("Position in file: %ld; processed data: %ld Bytes\n", currPos, currPos - lastPos);
		
		// Write the last valid time stamp / file position
		fmark = fopen(filenameMarker.c_str(), "w");
		if (fmark > 0) {
			fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, time_stamp_tv.tv_sec, time_stamp_tv.tv_usec, currPos);
			fclose(fmark);
		}
		
		fclose(fd_data_file);
		free(buf);
		delete[] local_sensorValue;
	}
}


int hatpro::parseData(char *line, struct timeval *l_tData, double *sensorValue){
	struct tm timestamp;
	char *puffer;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// read date and time
	puffer = strptime(line, "%y , %m , %d , %H , %M , %S", &timestamp);
	// error checking for HPC files: at restart of the application the last
	// file is read at the middle comment lines, this is caught here
	if (puffer == NULL) {
		printf("HATPRO: Error reading date and time string!\n");
		return -1;
	}
	
	if ( (sensorGroup == "CMP") || (sensorGroup == "HPC") || (sensorGroup == "LPR") || (sensorGroup == "TPB") || (sensorGroup == "TPC") ) {
		// read some dummy
		puffer = strtok(puffer - 1, ",");
		
		// read rain flag
		puffer = strtok(NULL, ",");
		sensorValue[0] = strtod(puffer, NULL);
		
		// read sensor values
		for (int j = 0; j < profile_length; j++) {
			puffer = strtok(NULL, ",\r\n");
			if (first_half)
				sensorValue[1 + j] = strtod(puffer, NULL);
			else
				sensorValue[j] = strtod(puffer, NULL);
		}
	} else {
		// read some dummy
		puffer = strtok(puffer - 1, ",");
		
		// read sensor values
		for (int i = 0; i < nSensors; i++) {
			puffer = strtok(NULL, ",\r\n");
			sensorValue[i] = strtod(puffer, NULL);
		}
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
	
	if (sensorGroup == "CBH") {
		number = 1;
		buffer = "time series";
	}
	
	if (sensorGroup == "CMP") {
		number = 2;
		buffer = "profile";
	}
	
	if (sensorGroup == "HKD") {
		number = 3;
		buffer = "profile";
	}
	
	if (sensorGroup == "HPC") {
		number = 4;
		buffer = "profile";
	}
	
	if (sensorGroup == "IWV_PPI") {
		number = 51;
		buffer = "profile";
	}
	
	if (sensorGroup == "IWV_RHI") {
		number = 52;
		buffer = "profile";
	}
	
	if (sensorGroup == "IWV_VER") {
		number = 53;
		buffer = "profile";
	}
	
	if (sensorGroup == "IWV_VOL") {
		number = 54;
		buffer = "profile";
	}
	
	if (sensorGroup == "LPR") {
		number = 6;
		buffer = "profile";
	}
	
	if (sensorGroup == "LWP_PPI") {
		number = 71;
		buffer = "profile";
	}
	
	if (sensorGroup == "LWP_RHI") {
		number = 72;
		buffer = "profile";
	}
	
	if (sensorGroup == "LWP_VER") {
		number = 73;
		buffer = "profile";
	}
	
	if (sensorGroup == "LWP_VOL") {
		number = 74;
		buffer = "profile";
	}
	
	if (sensorGroup == "MET") {
		number = 8;
		buffer = "profile";
	}
	
	if (sensorGroup == "STA") {
		number = 9;
		buffer = "profile";
	}
	
	if (sensorGroup == "TPB") {
		number = 10;
		buffer = "profile";
	}
	
	if (sensorGroup == "TPC") {
		number = 11;
		buffer = "profile";
	}
	
	return number;
}


long hatpro::getFileNumber(char* filename)
{
	std::string filename_prefix;
	std::string filename_suffix;
	std::string filename_string;
	size_t pos_index, pos_prefix, pos_suffix, length_prefix, length_suffix, pos;
	long index;
	
	
	if (debug >= 3) {
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
		printf("... from filename '%s'\n", filename);
	}
	
	
	if ( (sensorGroup.substr(0, 3) == "IWV") || (sensorGroup.substr(0, 3) == "LWP") )
		return DAQDevice::getFileNumber(filename);
	else {
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
		
		// find "_" as end of prefix in filename and remove whole prefix
		pos_prefix = filename_string.find("_");
		if (pos_prefix != std::string::npos) {
			length_prefix = pos_prefix + 1;
			filename_string.erase(0, length_prefix);
		} else {
			if (debug >= 3)
				printf("No '_' as end of prefix found in filename!\n");
			return 0;
		}
		
		// find "." as beginning of suffix and compare it to given suffix, then remove it
		pos_suffix = filename_string.find(".");
		if ((pos_suffix != std::string::npos) && (filename_string.substr(pos_suffix) == filename_suffix)) {
			filename_string.erase(pos_suffix);
		} else {
			if (debug >= 3)
				printf("Suffix not found or not at end of file name!\n");
			return 0;
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
}


int hatpro::create_data_table_name(std::string & data_table_name)
{
	char *line;
	int err;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	if( (sensorGroup == "CBH") || (sensorGroup == "HKD") || (sensorGroup.substr(0, 3) == "IWV") || (sensorGroup.substr(0, 3) == "LWP") || (sensorGroup == "MET") || (sensorGroup == "STA") )
		return DAQDevice::create_data_table_name(dataTableName);
	else {
		err = asprintf(&line, "Profiles_%03d_%s_%s", moduleNumber, moduleName.c_str(), sensorGroup.c_str());
		if (err == -1) {
			printf("Error: not enough space for string!\n");
			return -1;
		} else {
			printf("Data table name: \t%s\n", line);
			data_table_name = line;
			free(line);
			return 0;
		}
	}
}
