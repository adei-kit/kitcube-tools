/***************************************************************************
                          wolkenkamera.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "wolkenkamera.h"


wolkenkamera::wolkenkamera():DAQBinaryDevice(){
	
	lenHeader = 0;
	lenDataSet = 237; // In the chm files is no check sum !?
	noData = 99999;
	
}


wolkenkamera::~wolkenkamera(){
}


void wolkenkamera::setConfigDefaults(){
}


const char *wolkenkamera::getDataDir(){
	char line[256];
	
	// TODO: evtl. an zu definierende Verzeichnisstruktur anpassen
	sprintf(line, "CC2-pics/");
	buffer = line;
	
	return(buffer.c_str());
}


unsigned int wolkenkamera::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	if (sensorGroup == "jpg") {
		number = 1;
		buffer = "standard";
	}
	
	return( number);
}


void wolkenkamera::readHeader(const char *filename){
	int i;
	
	
	// Number of sensors
	nSensors = 1;
	
	// List of sensors
	if (sensor > 0 ) delete [] sensor;
	sensor = new struct sensorType [nSensors];
	
	sensor[0].comment = "sky picture";
	sensor[0].data_format = "<binary>";
	sensor[0].type = "profile";
	
	profile_length = 0;	// TODO/FIXME: do we need this here? this is done in daqdevice constructor, too
	
	for (i = 0; i < nSensors; i++) {
		sensor[i].height = 4;
		printf("Sensor %3d: %s, %.1f\n", i+1, sensor[i].comment.c_str(), sensor[i].height);
	}
}


void wolkenkamera::writeHeader(){
	// Nothing to do -- there is no header
}


// TODO: Move the parsing part to separate functions and move rest to base class
void wolkenkamera::readData(const char *dir, const char *filename){
	//int j;
	std::string timeString;
	std::string dateString;
	FILE *fmark;
	std::string filenameMarker;
	std::string filenameData;
	struct timeval lastTime;
	unsigned long lastPos;
	unsigned long lastIndex;
	//struct timeval tWrite;
	char line[256];
	
#ifdef USE_MYSQL
	struct timeval t0, t1;
	struct timezone tz;
	//int i;
	//MYSQL_RES *res;
	//MYSQL_RES *resTables;
	//MYSQL_ROW row;
	//MYSQL_ROW table;
	std::string tableName;
	std::string sql;
	char sData[50];
#endif
	
	if (debug >= 1)
		printf("_____wolkenkamera::readData(const char *dir, const char *filename)_____\n");
	
	// Compile file name
	filenameData = dir;
	filenameData += filename;
	//printf("<%s> <%s> <%s>\n", dir, filename, filenameData.c_str());
	
	
	// If number of sensors is unknown read the header first
	if (nSensors == 0)
		readHeader(filenameData.c_str());
	if (sensor[0].name.length() == 0)
		getSensorNames(sensorListfile.c_str());
	
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

	// Get the last time stamp + file pointer from
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dir, moduleNumber, sensorGroupNumber);
	filenameMarker = line;
	if (debug > 1) printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
		fclose(fmark);
	}
	
	
	// if file already read, return
	if (lastPos == 1) {
		fd_eof = true;
		printf("File already read -> nothing to do, returning...\n");
		return;
	}
		
		
	// convert all timestamps into timeval structure
	struct timeval* time_stamp_data;
/*	time_stamp_data = new struct timeval[no_vals];
	for (int i = 0; i < no_vals; i++) {
		time_stamp_data[i].tv_sec = (int)time_stamps[i];
		time_stamp_data[i].tv_usec = (int)((time_stamps[i] - (double)time_stamp_data[i].tv_sec) * 1000000.);	// TODO: maybe use floor() here
	}
*/	
	
	// write data to DB
#ifdef USE_MYSQL
	if (db > 0){
		// Write dataset to database
		// Store in the order of appearance
		//printf("Write record to database\n");
		sql = "INSERT INTO `";
		sql += dataTableName + "` (`usec`";
		for (int j = 0; j < nSensors; j++) {
			sql += ",`";
			sql += sensor[j].name;
			sql += "`";
		}
		sql += ") VALUES (";
//		sprintf(sData, "%ld", time_stamp_data[i].tv_sec * 1000000 + time_stamp_data[i].tv_usec);
		sql += sData;
		for (int j = 0; j < (nSensors - 1); j++) {
//			sprintf(sData, "%f", sensor_values[j][i]);
			sql += ",";
			sql += sData;
		}
		sql += ",'";
		// create profile data here
/*		for (int k = 0; k < dimensions[1]; k++) {
			sprintf(sData, "%f, ", values_2d[(i * dimensions[1]) + k]);
			sql += sData;
		}*/
		sql += "')";
		
		//printf("SQL: %s (db = %d)\n", sql.c_str(), db);
		
		gettimeofday(&t0, &tz);
		
		if (mysql_query(db, sql.c_str())){
			fprintf(stderr, "%s\n", sql.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			// If this operation fails do not proceed in the file?!
			printf("Error: Unable to write data to database\n");
			throw std::invalid_argument("Writing data failed");
//			break;
		}
		
		gettimeofday(&t1, &tz);
		
		if (debug >= 1)
			printf("DB insert duration: %ld.%ldus\n", t1.tv_sec - t0.tv_sec, t1.tv_usec - t0.tv_usec);
	} else {
		printf("Error: No database availabe\n");
		throw std::invalid_argument("No database");
	}
#endif // of USE_MYSQL
	
	
	printf("fertig\n");
	lastPos = 1;	// file read
	fd_eof = true;
	

	// Write the last valid time stamp / file position
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, lastTime.tv_sec, lastTime.tv_usec, lastPos);
		fclose(fmark);
	}
}


void wolkenkamera::updateDataSet(unsigned char *buf){
	struct timeval t;
	struct timezone tz;
	struct tm *time;
	
	
	// Calculate the interval
	gettimeofday(&t, &tz);
	time = gmtime(&t.tv_sec);
	
	
	// Analyse time stamp, if new day a new file needs to generated
	if (this->filename != getDataFilename()) {
		openNewFile();
	}
	
	// Compile the data set for writing to the data file
	if (debug > 2) printf("Line: %s", buf);
	sprintf((char *) buf+12,"%02d.%02d.%02d;%02d:%02d",
			time->tm_mday, time->tm_mon+1, time->tm_year-100,
			time->tm_hour, time->tm_min);
	buf[26]=';';
	sprintf((char *) buf+233,"%02d", time->tm_sec);
	buf[235]=';';
	if (debug > 2) printf("Line: %s", buf);
	
	if (debug > 1) printf("%02d.%02d.%02d  %02d:%02d:%02d\n",
						  time->tm_mday, time->tm_mon+1, time->tm_year-100,
						  time->tm_hour, time->tm_min, time->tm_sec);
}


int wolkenkamera::getFileNumber(char* filename){
	std::string filename_prefix;
	std::string filename_suffix;
	std::string filename_string;
	size_t pos_index, pos_prefix, pos_suffix, length_prefix, length_suffix, filename_length;
	int index;
	
	
	if (debug >= 1)
		printf("\n_____wolkenkamera::getFileNumber(char* filename)_____\n");
	if (debug >= 2)
		printf("From file %s\n", filename);
	
	// Write the index of the file to a list
	// Process in this list with the next index
	// The read pointer of the last file will be kept
	
	// Find <index> in data template
	pos_index = datafileMask.find("<index>");
	// FIXME: this is done in DAQDevice::readInifile already. Why repeat it here? What to do in case of error?
	if (pos_index == std::string::npos) {
		printf("Error: There is no tag <index> in datafileMask '%s' specified in inifile %s\n",
			datafileMask.c_str(), inifile.c_str());
	}
	
	filename_prefix = datafileMask.substr(0, pos_index);
	length_prefix = filename_prefix.length();
	filename_suffix = datafileMask.substr(pos_index + 7);
	length_suffix = filename_suffix.length();
	
	if (debug >= 4)
		printf("Position of <index> in %s is: %ld -- Prefix is: %s, suffix is: %s\n",
		       datafileMask.c_str(), pos_index, filename_prefix.c_str(), filename_suffix.c_str());
	
	filename_string = filename;
	
	// if there is a prefix, check for prefix at beginning of file name and delete it
	if (filename_prefix.size() != 0) {
		pos_prefix = filename_string.find(filename_prefix);
		if (pos_prefix == 0) {
			filename_string.erase(pos_prefix, length_prefix);
		} else {
			throw std::invalid_argument("Prefix not found or not at the beginning of file name");
		}
	}
	
	// if there is a suffix, check for suffix at end of filename and delete it
	if (filename_suffix.size() != 0) {
		pos_suffix = filename_string.find(filename_suffix);
		filename_length = filename_string.length();
		if ((pos_suffix != std::string::npos) && ((pos_suffix + length_suffix) == filename_length)) {
			filename_string.erase(pos_suffix, length_suffix);
		} else {
			throw std::invalid_argument("Suffix not found or not at end of file name");
		}
	}
	
	while (filename_string.find("-", 0) != std::string::npos) {
		//filename_string.erase(, 1);
	}
	
	// we assume, that after the removal of prefix and suffix, there are only numbers left
	// FIXME/TODO: check, if this is really only a number
	index = atoi(filename_string.c_str());
	
	if (debug >= 2)
		printf("Index is: %d\n", index);
	
	return index;
}