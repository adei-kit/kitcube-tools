/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Tue Jun 14 16:02:13 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#include "windcube.h"


windcube::windcube()
{
	altitudes = NULL;
	profile_length = 9;
}


windcube::~windcube(){
	if (altitudes != NULL)
		delete [] altitudes;

}


int windcube::readHeader(const char *filename) {
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
	
	
	// search for variable ScanAngle
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(line_of_data, 2048, data_file_ptr) == -1)
			return -1;
		
		buf = strstr(line_of_data, "ScanAngle");
	}
	
	buf = strchr(line_of_data, '=');
	elevation_angle = strtod(buf + 1, NULL);
	
	
	// search for variable Altitudes
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(line_of_data, 2048, data_file_ptr) == -1)
			return -1;
		
		buf = strstr(line_of_data, "Altitudes");
	}
	
	if (altitudes != NULL)
		delete [] altitudes;
	altitudes = new double[profile_length];
	
	strtok(line_of_data, "\t");	// this returns "Altitudes(m)="
	for (int i = 0; i < profile_length; i++) {
		buf = strtok(NULL, "\t\r\n");	// this return altitude values
		altitudes[i] = strtod(buf, NULL);
	}
	
	
	// find out header length
	// read lines until you find the variable "WiperCount" in the last line of the header
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(line_of_data, 2048, data_file_ptr) == -1)
			return -1;
		
		buf = strstr(line_of_data, "WiperCount");
	}
	lenHeader = ftell(data_file_ptr);
	
	
	// close data file
	fclose(data_file_ptr);
	
	
	noData = 999999;
	
	lenDataSet = 2048;	// unknown bytes + 1 for '\0' in fgets();
				// longest line found yet was ~1500
	
	// set default value for height
	sensor[0].type = "profile";
	sensor[0].height = 0;
	sensor[0].data_format = "<vector>";
	
	sensor[1].type = "";
	sensor[1].height = 0;
	sensor[1].data_format = "<scalar>";
	
	for (int i = 2; i < nSensors; i++) {
		sensor[i].type = "profile";
		sensor[i].height = 0;
		sensor[i].data_format = "<vector>";
	}
	
	return 0;
}


void windcube::setConfigDefaults() {
}


unsigned int windcube::getSensorGroup() {
	unsigned int number;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "sta") {
		number = 1;
		buffer = "average data";
	}
	
	return number;
}


void windcube::readData(std::string full_filename){
	char *buf;
	FILE *fd_data_file;
	double *local_sensorValue;
	int loop_counter = 0;
	FILE *fmark;
	std::string full_data_file_name;
	struct timeval lastTime;
	unsigned long lastPos;
	unsigned long currPos;
	long lastIndex;
	struct timeval time_stamp_tv = {0};
	
#ifdef USE_MYSQL
	std::string sql;
	char sData[50];
	char* esc_str;
#endif
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
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
	
	// Allocate memory for one line of data
	buf = new char[lenDataSet];
	
	// Allocate memory for sensor values, 2 sensors come from header
	local_sensorValue = new double[(nSensors - 2) * profile_length];
	
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
	
	while (loop_counter < 1000000) {
		// read one line of ASCII data
		if (read_ascii_line(buf, lenDataSet, fd_data_file) == -1) {
			fd_eof = true;
			break;
		}
		
		// parse data
		parseData(buf, &time_stamp_tv, local_sensorValue);
		
		// print sensor values
		if (debug >= 4) {
			printf("%4d: Received %4d bytes --- ", loop_counter, (int) strlen(buf));
			printf("%lds %6ldus --- ", time_stamp_tv.tv_sec, time_stamp_tv.tv_usec);
			for (int k = 0; k < profile_length; k++) {
				printf("%6.2f ", altitudes[k]);
			}
			printf("%5.2f ", elevation_angle);
			for (int j = 0; j < (nSensors - 2); j++) {
				for (int k = 0; k < profile_length; k++) {
					printf("%10.3f ", local_sensorValue[j * profile_length + k]);
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
		
		//altitudes
		esc_str = new char[2 * profile_length * sizeof(double) + 1];
		sql += ", '";
		mysql_real_escape_string(db, esc_str, (const char*)altitudes, profile_length * sizeof(double));
		sql += esc_str;
		sql += "'";
		
		// elevation angle
		sql += ", ";
		sprintf(sData, "%f", elevation_angle);
		sql += sData;
		
		// sensor values
		for (int i = 0; i < (nSensors - 2); i++) {
			sql += ", '";
			mysql_real_escape_string(db, esc_str,
						 (const char *)(local_sensorValue + i * profile_length),
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
		
		loop_counter++;
	}
	
#ifdef USE_MYSQL
	sql = "UNLOCK TABLES";
	mysql_query(db, sql.c_str());
#endif
	
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
	delete[] buf;
	delete[] local_sensorValue;
}


int windcube::parseData(char *line, struct timeval *l_tData, double* sensor_values) {
	char *buffer, **buf;
	struct tm time_stamp_tm;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// get time stamp in struct tm
	buffer = strptime(line, "%d/%m/%Y %T", &time_stamp_tm);
	if (buffer == NULL) {
		printf("Windcube: Error reading date and time string!\n");
		return -1;
	}
	
	// convert time stamp to struct timeval
	l_tData->tv_sec = timegm(&time_stamp_tm); // FIXME: this function is a non-standard GNU extension, try to avoid it!
	
	// read data
	buf = &buffer;
	strtod(buffer, buf);	// WiperCount, not used
	strtod(buffer, buf);	// Tm, not used
	for (int i = 0; i < profile_length; i++) {
		sensor_values[i] = strtod(buffer, buf);	// Vhm
		sensor_values[i + profile_length] = strtod(buffer, buf);	// dVh
		strtod(buffer, buf);	// VhMax, not used
		strtod(buffer, buf);	// VhMin, not used
		sensor_values[i + 2 * profile_length] = strtod(buffer, buf);	// Azim
		sensor_values[i + 3 * profile_length] = strtod(buffer, buf);	// um
		strtod(buffer, buf);	// du, not used
		sensor_values[i + 4 * profile_length] = strtod(buffer, buf);	// vm
		strtod(buffer, buf);	// dv, not used
		sensor_values[i + 5 * profile_length] = strtod(buffer, buf);	// wm
		sensor_values[i + 6 * profile_length] = strtod(buffer, buf);	// dw
		sensor_values[i + 7 * profile_length] = strtod(buffer, buf);	// CNRm
		strtod(buffer, buf);	// dCNR, not used
		strtod(buffer, buf);	// CNRmax, not used
		strtod(buffer, buf);	// CNRmin, not used
		strtod(buffer, buf);	// sigmaFreqm, not used
		strtod(buffer, buf);	// dsigmaFreq, not used
		strtod(buffer, buf);	// Avail, not used
	}
	
	return 0;
}


int windcube::read_ascii_line(char *buffer, int length, FILE *file_ptr) {
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


int windcube::create_data_table_name(std::string & data_table_name)
{
	char *line;
	int err;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
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
