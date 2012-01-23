/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Tue Jun 14 16:02:13 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#include "windcube.h"

#ifdef __linux__
#include <endian.h>
#elif __APPLE__
// Header endian is not available in OSX, while endian swapping functions are
#include "endian-osx.h"
#endif


windcube::windcube()
{
	altitudes = NULL;
	profile_length = 10;

	dataTablePrefix = "Profiles";
}


windcube::~windcube(){
	if (altitudes != NULL)
		delete [] altitudes;

}


int windcube::readHeader(const char *filename) {
	FILE *data_file_ptr;
	char *line_of_data = NULL;
	size_t len = 0;
	char *buf;
	struct tm timestamp = {0};
	
	
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
		if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
			free(line_of_data);
			return -1;
		}
		
		buf = strstr(line_of_data, "ScanAngle");
	}
	buf = strchr(line_of_data, '=');
	elevation_angle = strtod(buf + 1, NULL);
	
	
	// search for variable Altitudes
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
			free(line_of_data);
			return -1;
		}
		
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
	
	
	if (sensorGroup == "rtd") {
		// search for RelativeInitialPosition
		buf = NULL;
		while (buf == NULL) {
			if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
				free(line_of_data);
				return -1;
			}
			
			buf = strstr(line_of_data, "RelativeInitialPosition");
		}
		buf = strchr(line_of_data, '=');
		initial_position = atoi(buf + 1);
		
		// search for InitialDate/Time
		buf = NULL;
		while (buf == NULL) {
			if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
				free(line_of_data);
				return -1;
			}
			
			buf = strstr(line_of_data, "InitialDate/Time");
		}
		buf = strptime(line_of_data, "InitialDate/Time=%d/%m/%Y %T", &timestamp);
		initial_timestamp.tv_sec = timegm(&timestamp);
		initial_timestamp.tv_usec = strtol(buf + 1, NULL, 10);
		
		// search for Gain
		buf = NULL;
		while (buf == NULL) {
			if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
				free(line_of_data);
				return -1;
			}
			
			buf = strstr(line_of_data, "Gain");
		}
		buf = strchr(line_of_data, '=');
		gain = strtod(buf + 1, NULL);
	}
	
	
	// find out header length
	// read lines until you find the variable "WiperCount" resp. "InitialLD2" in the last line of the header
	buf = NULL;
	while (buf == NULL) {
		if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
			free(line_of_data);
			return -1;
		}
		
		if (sensorGroup == "sta") {
			buf = strstr(line_of_data, "WiperCount");
		} else if (sensorGroup == "rtd") {
			buf = strstr(line_of_data, "InitialLD2");
		}
	}
	lenHeader = ftell(data_file_ptr);
	
	
	// close data file
	fclose(data_file_ptr);
	
	
	noData = 999999;
	
	if (sensorGroup == "rtd")
		lenDataSet = 135;
	
	// set default value for height
	sensor[0].type = "profile";
	sensor[0].height = 0;
	sensor[0].data_format = "<vector>";
	
	if (sensorGroup == "sta") {
		sensor[1].type = "";
		sensor[1].height = 0;
		sensor[1].data_format = "<scalar>";
		
		for (int i = 2; i < nSensors; i++) {
			sensor[i].type = "profile";
			sensor[i].height = 0;
			sensor[i].data_format = "<vector>";
		}
	}
	
	if (sensorGroup == "rtd") {
		for (int i = 1; i < 4; i++) {
			sensor[i].type = "";
			sensor[i].height = 0;
			sensor[i].data_format = "<scalar>";
		}
		
		for (int i = 4; i < nSensors; i++) {
			sensor[i].type = "profile";
			sensor[i].height = 0;
			sensor[i].data_format = "<vector>";
		}
	}
	
	
	free(line_of_data);
	
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
	
	if (sensorGroup == "rtd") {
		number = 2;
		buffer = "fast data";
	}
	
	return number;
}


void windcube::readData(std::string full_filename){
	char *buf = NULL;
	size_t len = 0;
	FILE *fd_data_file;
	int fd_datafile;
	double *local_sensorValue, azimuth_angle;
	int loop_counter = 0;
	//FILE *fmark;
	std::string full_data_file_name;
	//struct timeval lastTime;
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
	
	if (sensorGroup == "sta") {
		// Allocate memory for sensor values, 2 sensors come from header
		local_sensorValue = new double[(nSensors - 2) * profile_length];
	}
	if (sensorGroup == "rtd") {
		// Allocate memory for one data set
		buf = new char [lenDataSet];
	
		// Allocate memory for sensor values: 2 come from header, 1 is calculated, 1 is scalar and only the rest are profile sensors
		local_sensorValue = new double[1 + (nSensors - 4) * profile_length];
	}
	
	// Compile file name
	full_data_file_name = full_filename;
	
	// open data file
	if (debug >= 1)
		printf("Open data file: %s\n", full_data_file_name.c_str());
	if (sensorGroup == "sta") {
		fd_data_file = fopen(full_data_file_name.c_str(), "r");
		if (fd_data_file == NULL) {
			printf("Error opening data file %s\n", full_data_file_name.c_str());
			return;
		}
	}
	if (sensorGroup == "rtd") {
		fd_datafile = open(full_data_file_name.c_str(), O_RDONLY);
		if (fd_datafile == -1) {
			printf("Error opening data file %s\n", full_data_file_name.c_str());
			return;
		}
	}
	
	
	// Get the last time stamp + file pointer from the marker file
	loadFilePosition(lastIndex, lastPos, time_stamp_tv);

/*	
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	if (debug >= 1)
		printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, (long *) &lastTime.tv_usec, &lastPos);
		fclose(fmark);
		
		// Read back the data time stamp of the last call
		time_stamp_tv.tv_sec = lastTime.tv_sec;
		time_stamp_tv.tv_usec = lastTime.tv_usec;
		
		if (debug >= 1)
			printf("Last time stamp was %ld\n", lastTime.tv_sec);
	}
*/	
 
	// Find the beginning of the new data
	// if new file, jump to the first data set
	if (lastPos == 0)
		lastPos = lenHeader;
	currPos = lastPos;
	if (debug >= 1)
		printf("Last position in file: %ld\n", lastPos);
	if (sensorGroup == "sta")
		fseek(fd_data_file, lastPos, SEEK_SET);
	if (sensorGroup == "rtd")
		lseek(fd_datafile, lastPos, SEEK_SET);
	
#ifdef USE_MYSQL
	sql = "LOCK TABLES " + dataTableName + " WRITE";
	mysql_query(db, sql.c_str());
#endif
	
	fd_eof = false;
	
	while (loop_counter < 1000000) {
		if (sensorGroup == "sta") {
			// read one line of ASCII data
			if (read_ascii_line(&buf, &len, fd_data_file) == -1) {
				fd_eof = true;
				break;
			}
		}
		
		if (sensorGroup == "rtd") {
			// read one data set of binary data
			if (read(fd_datafile, buf, lenDataSet) != lenDataSet) {
				fd_eof = true;
				break;
			}
			
			// calculate azimuth angle of measurement:
			// starting at RelativeInitialPosition and adding 90 degrees
			// for each acquisition
			azimuth_angle = ((currPos - lenHeader) / lenDataSet * 90  + initial_position) % 360;
		}
		
		// parse data
		parseData(buf, &time_stamp_tv, local_sensorValue);
		
		// print sensor values
		if (debug >= 4) {
			printf("%4d: Received %4d bytes --- ", loop_counter, (int) strlen(buf));
			printf("%lds %6ldus --- ", time_stamp_tv.tv_sec, (long) time_stamp_tv.tv_usec);
			for (int k = 0; k < profile_length; k++) {
				printf("%6.2f ", altitudes[k]);
			}
			printf("%5.2f ", elevation_angle);
			if (sensorGroup == "sta") {
				for (int j = 0; j < (nSensors - 2); j++) {
					for (int k = 0; k < profile_length; k++) {
						printf("%10.3f ", local_sensorValue[j * profile_length + k]);
					}
				}
			}
			if (sensorGroup == "rtd") {
				printf("%5.2f ", azimuth_angle);
				printf("%.0f ", local_sensorValue[0]);
				for (int j = 0; j < (nSensors - 4); j++) {
					for (int k = 0; k < profile_length; k++) {
						printf("%10.3f ", local_sensorValue[1 + j * profile_length + k]);
					}
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
		if (sensorGroup == "sta") {
			// sensor values
			for (int i = 0; i < (nSensors - 2); i++) {
				sql += ", '";
				mysql_real_escape_string(db, esc_str,
							(const char *)(local_sensorValue + i * profile_length),
							profile_length * sizeof(double));
				sql += esc_str;
				sql += "'";
			}
		}
		
		if (sensorGroup == "rtd") {
			// azimuth angle
			sql += ", ";
			sprintf(sData, "%f", azimuth_angle);
			sql += sData;
			
			// wiper flag
			sql += ", ";
			sprintf(sData, "%f", local_sensorValue[0]);
			sql += sData;
			
			// sensor values
			for (int i = 0; i < (nSensors - 4); i++) {
				sql += ", '";
				mysql_real_escape_string(db, esc_str,
							(const char *)(local_sensorValue + 1 + i * profile_length),
							profile_length * sizeof(double));
				sql += esc_str;
				sql += "'";
			}
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
		if (sensorGroup == "sta")
			currPos = ftell(fd_data_file);
		if (sensorGroup == "rtd")
			currPos += lenDataSet;
		
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
	saveFilePosition(lastIndex, currPos, time_stamp_tv);

/*	
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, time_stamp_tv.tv_sec, (long) time_stamp_tv.tv_usec, currPos);
		fclose(fmark);
	}
*/	
 
	if (sensorGroup == "sta")
		fclose(fd_data_file);
	if (sensorGroup == "rtd") 
		close(fd_datafile);
	free(buf);
	delete[] local_sensorValue;
}


int windcube::parseData(char *line, struct timeval *l_tData, double* sensor_values) {
	char *buffer, **buf;
	struct tm time_stamp_tm;
	u_int32_t time_diff;
	u_int8_t wiper_flag;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	if (sensorGroup == "sta") {
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
	}
	
	if (sensorGroup == "rtd") {
		// get time stamp
		time_diff = be32toh(*(u_int32_t *)line);
		l_tData->tv_sec = initial_timestamp.tv_sec + time_diff/10;
		l_tData->tv_usec = initial_timestamp.tv_usec + (time_diff % 10) * 100000;
		
		// get wiper flag
		wiper_flag = *(line + 6);
		sensor_values[0] = wiper_flag;
		
		// get sensor values (CNR and RWS)
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < profile_length; i++)
				sensor_values[i + 1 + j * profile_length] =
				(int16_t)be16toh(*(int16_t *)(line + 15 + j * profile_length * 2 + i * 2)) / gain;
		}
		
		// get sensor values (RWSD)
		for (int i = 0; i < profile_length; i++)
			sensor_values[i + 1 + 2 * profile_length] =
			(int16_t)be16toh(*(int16_t *)(line + 15 + 2 * profile_length * 2 + i * 2)) / (10 * gain);
	}
	
	return 0;
}

/*
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
*/
