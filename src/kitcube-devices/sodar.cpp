/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Tue Mar  1 15:27:32 CET 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#include "sodar.h"


sodar::sodar(){
}


sodar::~sodar(){
}


int sodar::readHeader(const char *filename) {
	FILE *data_file_ptr;
	char line_of_data[128];
	char *check_ptr;
	int no_of_comment, no_of_variables, no_of_heights;
	char *gap_value_str;
	int no_of_gap_values;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	
	// read 1st line and check for "FORMAT-1" string
	if (read_ascii_line(line_of_data, 127, data_file_ptr) == -1)
		return -1;
	
	if (strstr(line_of_data, "FORMAT-1") == NULL) {
		printf("Error file %s is not a \"FORMAT-1\"\n", filename);
		return -1;
	}
	
	// read 2 dummy lines (1 time stamp line + 1 type of instrument line)
	for (int i = 0; i < 2; i++) {
		if (read_ascii_line(line_of_data, 127, data_file_ptr) == -1)
			return -1;
	}
	
	// read number of comment lines, variables and height levels
	if (read_ascii_line(line_of_data, 127, data_file_ptr) == -1)
		return -1;
	
	sscanf(line_of_data, "%d %d %d", &no_of_comment, &no_of_variables, &no_of_heights);
	
	// read lines until you find the comment "variable definitions"
	// ich suche das, damit ich in der Daten Datei keine Zeilen abzaehlen muss ;-)
	check_ptr = NULL;
	while (check_ptr == NULL) {
		if (read_ascii_line(line_of_data, 127, data_file_ptr) == -1)
			return -1;
		
		check_ptr = strstr(line_of_data, "variable definitions");
	}
	
	// TODO/FIXME: the following 2 code blocks only work, because in the variable
	//             definition list z is 1st and T and error are the last 2
	
	// read 2 dummy lines (1 comment line + height variable (z) line)
	for (int i = 0; i < 2; i++) {
		if (read_ascii_line(line_of_data, 127, data_file_ptr) == -1)
			return -1;
	}
	
	// read gap values for all variables besides z and T and the error variable
	no_of_gap_values = nSensors - 2;	// TODO/FIXME: this works only here!
	gap_value = new double[no_of_gap_values];
	for (int i = 0; i < no_of_gap_values; i++) {
		if (read_ascii_line(line_of_data, 127, data_file_ptr) == -1)
			return -1;
		
		gap_value_str = strrchr(line_of_data, '#');
		gap_value[i] = atof(gap_value_str + 1);
	}
	
	// close data file
	fclose(data_file_ptr);
	
	noData = 999999;
	
	lenHeader = 0x343;	// CAUTION: header contains 32 lines
	
	lenDataSet = 76;	// 75 bytes + 1 for '\0' in fgets();
				// this is only one line of data,
				// a data set consists of "number of height levels" lines
	
	profile_length = no_of_heights;	// profile
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
		sensor[0].data_format = "<vector>";
	}
	
	return 0;
}


void sodar::setConfigDefaults() {
}


unsigned int sodar::getSensorGroup() {
	unsigned int number;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "MND") {
		number = 1;
		buffer = "Main Data";
	}
	
	return number;
}


int sodar::create_data_table() {
#ifdef USE_MYSQL
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::string sql_stmt;
	
	
	// search for tables with names like "dataTableName"
	result = mysql_list_tables(db, dataTableName.c_str());
	if (result == NULL) {
		printf("Error retrieving table list: %s\n", mysql_error(db));
		// TODO: error handling
	}
	
	// fetch row from result set
	row = mysql_fetch_row(result);
	
	// free memory for result set
	mysql_free_result(result);
	
	// if there is no row, meaning no table, create it
	if (row == NULL) {
		printf("Creating data table %s...\n", dataTableName.c_str());
		
		// build SQL statement
		sql_stmt = "CREATE TABLE `" + dataTableName + "` ";
		sql_stmt += "(`id` bigint auto_increment, `usec` bigint default '0', ";
		for (int i = 0; i < nSensors; i++)
			sql_stmt += "`" + sensor[i].name + "` blob, ";
		sql_stmt += "PRIMARY KEY (`id`), INDEX(`usec`)) TYPE=MyISAM";
		
		// execute SQL statement
		if (mysql_query(db, sql_stmt.c_str())) {
			printf("Error creating data table %s: %s\n", dataTableName.c_str(), mysql_error(db));
			// TODO: error handling
		}
	}
#endif
	return 0;
}


void sodar::readData(std::string full_filename){
	char *buf;
	FILE *fd_data_file;
	double *local_sensorValue;
	int loop_counter = 0;
	struct tm time_stamp_tm = {0};
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
	
	// Allocate memory for sensor values
	local_sensorValue = new double[nSensors * profile_length];
	
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
	if (lastPos == 0)	// if new file, jump to the first data set
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
		// read the line of ASCII data containing the time stamp
		if (read_ascii_line(buf, lenDataSet, fd_data_file) == -1) {
			fd_eof = true;
			break;
		}
		
		// get time stamp in struct tm
		strptime(buf, "%F %T", &time_stamp_tm);
		
		// convert time stamp to struct timeval
		time_stamp_tv.tv_sec = timegm(&time_stamp_tm); // FIXME: this function is a non-standard GNU extension, try to avoid it!
		
		// read 1 line of comment
		if (read_ascii_line(buf, lenDataSet, fd_data_file) == -1) {
			fd_eof = true;
			break;
		}
		
		// read and parse sensor value lines
		for (int j = 0; j < profile_length; j++) {
			// read line
			if (read_ascii_line(buf, lenDataSet, fd_data_file) == -1) {
				fd_eof = true;
				break;
			}
			
			// parse data
			parseData(buf, local_sensorValue, j);
		}
		
		// FIXME: If we come to the end of the data file in the above
		//        for-loop and therefore break out of it, we don't have
		//        to manually break out of the while-loop. This is done
		//        "automatically" by the following code block
		
		// read 1 empty line after data block
		if (read_ascii_line(buf, lenDataSet, fd_data_file) == -1) {
			fd_eof = true;
			break;
		}
		
		// search for gap values and replace them with NAN
		convert_gap_values(local_sensorValue);
		
		// print sensor values
		if (debug >= 4) {
			printf("%4d: Received %4d bytes --- ", loop_counter, (int) strlen(buf));
			printf("%lds %6ldus --- ", time_stamp_tv.tv_sec, time_stamp_tv.tv_usec);
			for (int j = 0; j < nSensors; j++) {
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
		sql = "INSERT INTO `" + dataTableName + "` (`usec`";
		for (int i = 0 ; i < nSensors; i++)
			sql += ", `" + sensor[i].name + "`";
		sql += ") VALUES (";
		
		// time stamp
		sprintf(sData, "%ld", time_stamp_tv.tv_sec * 1000000 + time_stamp_tv.tv_usec);
		sql += sData;
		
		// sensor values
		esc_str = new char[2 * profile_length * sizeof(double) + 1];
		for (int i = 0; i < nSensors; i++) {
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


int sodar::parseData(char *buffer, double* sensor_values, int height_no) {
	double dummy;
	
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	sscanf(buffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
	       sensor_values + height_no,
	       sensor_values + profile_length + height_no,
	       sensor_values + 2 * profile_length + height_no,
	       sensor_values + 3 * profile_length + height_no,
	       sensor_values + 4 * profile_length + height_no,
	       sensor_values + 5 * profile_length + height_no,
	       sensor_values + 6 * profile_length + height_no,
	       sensor_values + 7 * profile_length + height_no,
	       &dummy,
	       sensor_values + 8 * profile_length + height_no);
	// TODO: error handling
	
	return 0;
}


int sodar::read_ascii_line(char *buffer, int length, FILE *file_ptr) {
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


int sodar::convert_gap_values(double* sensor_values) {
	// replace gap values by NAN
	// 1st "sensor" is height, which has no gap values; so jump to 2nd "sensor"
	for (int i = 0; i < nSensors - 2; i++) {
		for (int j = 0; j < profile_length; j++) {
			if (*(sensor_values + (i + 1) * profile_length + j) == gap_value[i])
				*(sensor_values + (i + 1) * profile_length + j) = nan("123");
		}
	}
	
	return 0;
}
