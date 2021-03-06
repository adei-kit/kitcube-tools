/***************************************************************************
                          wolkenkamera.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "wolkenkamera.h"

#include <libgen.h>



wolkenkamera::wolkenkamera():DAQBinaryDevice(){
	
	lenHeader = 0;
	lenDataSet = 237; // In the chm files is no check sum !?
	noData = 99999;
	
	// There are non scalar variables to be stored
	//dataTablePrefix = "Profiles";
}


wolkenkamera::~wolkenkamera(){
}


void wolkenkamera::setConfigDefaults(){
}


const char *wolkenkamera::getDataDir(){
	char line[256];
	
	// TODO: evtl. an zu definierende Verzeichnisstruktur anpassen
	sprintf(line, "%s/data/", moduleName.c_str());
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


int wolkenkamera::readHeader(const char *filename) {
	int i;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	if (sensorGroup == "jpg"){	// read jpg file and write file name?!
	
        if (nSensors < 4) {
			printf("Error: Sensor defintion does not match to the implementation\n");
			throw std::invalid_argument("Sensor definition too short\n");
        }
        
		sensor[0].comment = "sky picture";
		sensor[0].data_format = "<binary>";
		sensor[0].type = "profile";

		sensor[1].comment = "mean pixel brightness";
		sensor[1].data_format = "<scalar>";
		
		sensor[2].comment = "number of black pixel";
		sensor[2].data_format = "<scalar>";

		sensor[3].comment = "number of white pixel";
		sensor[3].data_format = "<scalar>";
        
		profile_length = 0;	// TODO/FIXME: do we need this here? this is done in daqdevice constructor, too
		
	}
	
	for (i = 0; i < nSensors; i++) {
		sensor[i].height = 4;
		//printf("Sensor %3d: %s, %.1f\n", i+1, sensor[i].comment.c_str(), sensor[i].height);
	}
	
	return 0;
}


void wolkenkamera::writeHeader(){
	// Nothing to do -- there is no header
}



// TODO: Move the parsing part to separate functions and move rest to base class
void wolkenkamera::readData(std::string full_filename){
	//int j;
	std::string timeString;
	std::string dateString;
	//FILE *fmark;
	std::string filenameData;
	std::string filename;	
	struct timeval lastTime;
	unsigned long lastPos;
	long lastIndex;
	//struct timeval tWrite;
	struct tm time_stamp_data;
    double *local_sensorValue;
	
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

    
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// Compile file name
	filenameData = full_filename;
	//printf("<%s> <%s> <%s>\n", dir, filename, filenameData.c_str());
	
	
	// If number of sensors is unknown read the header first
	if (nSensors == 0)
		readHeader(filenameData.c_str());
	
#ifdef USE_MYSQL
    if ((db == 0) || (mysql_ping(db) != 0))
        openDatabase();

#endif

	// Get the last time stamp + file pointer from
	loadFilePosition(lastIndex, lastPos, lastTime);
	
	
	// if file already read, return
	if (lastPos == 1) {
		fd_eof = true;
		if (debug > 1) printf("File already read -> nothing to do, returning...\n");
		
		saveFilePosition(lastIndex, lastPos, lastTime); // Update the entry in the status tab
		return;
	}
		
	// extract timestamp from filename
	// Strip path
    
    // Get the pattern from the inifile
    size_t pos; 
    size_t len;
    std::string mask;
    
    pos = this->datafileMask.find("<index>");
    len = 7;
    //printf("Template: %s (pos = %d)\n", datafileMask.c_str(), pos);
    
    if (pos == std::string::npos) {
        printf("Error: <index> not found\n");
        throw std::invalid_argument("Tag <index> is missing in datafile mask\n");
    }
    mask = this->datafileMask;
    //mask.replace(pos, len, "%Y-%m-%d_%H-%M-%S");
    mask.replace(pos, len, datafileIndexFormat.c_str());
    //printf("Mask = %s\n", mask.c_str());
    
    
    filename = basename((char*)full_filename.c_str());		
	//strptime(filename.c_str(), "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg", &time_stamp_data);
	strptime(filename.c_str(), mask.c_str(), &time_stamp_data);
	
	if (debug > 3){	
	    printf("Filename  : %s\n", filename.c_str());
	    printf("Date      : %02d.%02d.%4d  %02d:%02d:%02d\n", 
			   time_stamp_data.tm_mday, time_stamp_data.tm_mon+1, time_stamp_data.tm_year+1900,
			   time_stamp_data.tm_hour, time_stamp_data.tm_min, time_stamp_data.tm_sec);
		printf("Timestamp : %lds\n", timegm(&time_stamp_data));
	}

	
	if (sensorGroup == "jpg"){

        local_sensorValue = new double [nSensors];
        for (int j=0; j<nSensors;j++){
            local_sensorValue[j] = noData;
        }        
        
        // Calculate the image properties
        // Stores directly in sensors field
        readDataWithPython(full_filename.c_str(), local_sensorValue+1);
        
	// write data to DB
#ifdef USE_MYSQL
	if (db > 0){
		// Write dataset to database
		// Store in the order of appearance
		//printf("Write record to database\n");
		sql = "INSERT INTO `";
		sql += dataTableName + "` (`usec`";
        sql += ",`";
        sql += sensor[0].name;
        sql += "`";        
		for (int j = 1; j < nSensors; j++) {
            if (local_sensorValue[j] != noData){
			   sql += ",`";
			   sql += sensor[j].name;
			   sql += "`";
            }
		}
		sql += ") VALUES (";
		sprintf(sData, "%ld", timegm(&time_stamp_data) * 1000000);
		sql += sData;
        sql += ",";
        sprintf(sData, "'%s'", filename.c_str());
        sql += sData;
		for (int j = 1; j < nSensors; j++) {
            if (local_sensorValue[j] != noData){
  			   sql += ",";
			   sprintf(sData, "'%f'", local_sensorValue[j]);
			   sql += sData;
            }
		}
		sql += ")";
		
		if (debug > 4) printf("SQL: %s (db = %p)\n", sql.c_str(), db);
		
		gettimeofday(&t0, &tz);
		
		if (mysql_query(db, sql.c_str())){
			fprintf(stderr, "%s\n", sql.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			// If this operation fails do not proceed in the file?!
			printf("Error: Unable to write data to database\n");
			throw std::invalid_argument("Writing data failed");
		}
		
		gettimeofday(&t1, &tz);
		
		if (debug >= 5)
			printf("DB insert duration: %ld.%ldus\n", t1.tv_sec - t0.tv_sec, (long) t1.tv_usec - t0.tv_usec);
	} else {
		printf("Error: No database availabe\n");
		throw std::invalid_argument("No database");
	}
#endif // of USE_MYSQL
	
        
        delete [] local_sensorValue;
	} 
	
		
	// file read
	fd_eof = true;
	lastPos = 1;
	
	// Write the last valid time stamp / file position
	lastTime.tv_sec = timegm(&time_stamp_data);
	lastTime.tv_usec = 0;
	saveFilePosition(lastIndex, lastPos, lastTime);
	if (debug > 3) 
		printf("Last timestamp: %ld sec %d usec\n", lastTime.tv_sec, lastTime.tv_usec);

	
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


long wolkenkamera::getFileNumber(char* filename){
	std::string filename_prefix;
	std::string filename_suffix;
	std::string filename_string;
	size_t pos_index, pos_prefix, pos_suffix, length_prefix, length_suffix, filename_length;
	size_t find_position;
	long index;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	if (debug > 2)
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
			if (debug >= 3)
				printf("\033[35mPrefix not found or not at the beginning of file name!\033[0m\n");
			return 0;
		}
	}
	
	// if there is a suffix, check for suffix at end of filename and delete it
	if (filename_suffix.size() != 0) {
		pos_suffix = filename_string.find(filename_suffix);
		filename_length = filename_string.length();
		if ((pos_suffix != std::string::npos) && ((pos_suffix + length_suffix) == filename_length)) {
			filename_string.erase(pos_suffix, length_suffix);
		} else {
			if (debug >= 3)
				printf("\033[35mSuffix not found or not at end of file name!\033[0m\n");
			return 0;
		}
	}
	
/*
	// remove "-" from date and time string in filename
	find_position = filename_string.find("-", 0);
	while (find_position != std::string::npos) {
		filename_string.erase(find_position, 1);
		find_position = filename_string.find("-", 0);
	}
	
	// we assume, that after the removal of prefix and suffix, there are only numbers left
	// FIXME/TODO: check, if this is really only a number
	index = atol(filename_string.c_str());
*/	

	//std::string datafileIndexFormat = "%Y-%m-%d_%H-%M-%S";
	struct tm timestamp = {0};
	char *puffer;
	printf("Format string is //%s//\n", datafileIndexFormat.c_str());
	puffer = strptime(filename_string.c_str(), datafileIndexFormat.c_str(), &timestamp);
        //puffer = strptime(filename_string.c_str(), "%Y-%m-%d", &timestamp);
	if (puffer == NULL) {
	       printf("getFileNumber: Error reading date and time string in file %s / index %s\n", 
	                                          filename, filename_string.c_str());
	       index = 0; // skip this file ?!        
	}		       
	//printf("Timestamp %d %d %d (err=%ld)\n", timestamp.tm_mday, timestamp.tm_mon, timestamp.tm_year, puffer);	
	index = timegm(&timestamp);


	if (debug > 2)
		printf("Index is: %ld\n", index);
	
	return index;
}


