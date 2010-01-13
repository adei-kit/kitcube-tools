/***************************************************************************
                          ceilometer.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/



#include "ceilometer.h"

#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <fstream>
#include <nl_types.h>
#include <string>
#include <stdexcept>


#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif


#include <akutil/akinifile.h>



Ceilometer::Ceilometer():DAQBinaryDevice(){
	
	this->moduleType = "Ceilometer";
	this->moduleNumber = 20; // Ceilometer default number
	this->sensorGroup = "chm";

	this->iniGroup = "Ceilometer"; 
	
	this->lenHeader = 0;
	this->lenDataSet = 237; // In the chm files is no check sum !?
    this->noData = 99999;
	
	headerRaw = 0;
}


Ceilometer::~Ceilometer(){
	
	// Free header memory
	if (headerRaw > 0) delete [] headerRaw;

}


void Ceilometer::setConfigDefaults(){
	char line[256];

	// Note:
	// The paramters are dependant of the module number that is not known at the creation 
	// of the class but first time after reading from the inifile
	// 
	
	this->moduleName = "CM";
	this->moduleComment = "Ceilometer";
	
	sprintf(line, "Ceilometer.%s.template", sensorGroup.c_str());
	this->datafileTemplate = line;
	sprintf(line, "20<index>.%s", sensorGroup.c_str());
	this->datafileMask = line;
	//printf("datafileMask = %s\n", datafileMask.c_str());
	
	sprintf(line, "Ceilometer.%s.sensors", sensorGroup.c_str());
	this->sensorListfile = line;

}

 
const char *Ceilometer::getDataDir(){
	char line[256];
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "Ceilometer/Daten/");
	buffer = line;
	return(buffer.c_str());
}


const char *Ceilometer::getDataFilename(){
	struct timeval t;
	struct timezone tz;	
	struct tm *date;
	char line[256];

	
	// Get the actual day
	gettimeofday(&t, &tz);
	date = gmtime((const time_t *) &t.tv_sec);
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "%04d%02d%02d.%s", date->tm_year+1900, date->tm_mon+1, date->tm_mday, sensorGroup.c_str());
	buffer = line;

	//printf("Ceilometer: Get Datafilename = %s\n", line);
	return(buffer.c_str());
}



int Ceilometer::getFileNumber(char *filename){
	std::string name;
	int posIndex;
	int index;
	std::string filePrefix;
	std::string fileSuffix;
	std::string numString;
	std::string fileString;
	struct tm time;
	int lenIndex;
	
	
	// Write the index of the file to a list 
	// Process in this list with the next index
	// The read pointer of the last file will be kept
	
	// Find <index> in data template
	posIndex = datafileMask.find("<index>");
	if (posIndex == -1) {
		printf("Error: There is no tag <index> in datafileMask=%s specified in inifile %s\n",
			datafileMask.c_str(), inifile.c_str());
	}
	filePrefix = datafileMask.substr(0,posIndex);
	fileSuffix = datafileMask.substr(posIndex+7);
	if (debug >3) printf("Position of <index> in %s is  %d -- data file prefix/suffix %s / %s (debug = %d)\n",
				datafileMask.c_str(), posIndex, filePrefix.c_str(), fileSuffix.c_str(), debug);
	
	fileString = filename;
	lenIndex = fileString.length() - filePrefix.length() - fileSuffix.length();
	
	// Check for starting tag
	if (fileString.find(filePrefix) !=0){
		throw std::invalid_argument("Prefix not found");
	}
	if (fileString.find(fileSuffix) <0){
		throw std::invalid_argument("Suffix not found");
	}
	
	
	// Get year
	numString = fileString.substr(posIndex, 2);
	time.tm_year = atoi(numString.c_str()) + 100;
	
	numString = fileString.substr(posIndex+2, 2);
	time.tm_mon = atoi(numString.c_str()) - 1;
	
	numString = fileString.substr(posIndex+4, 2);
	time.tm_mday = atoi(numString.c_str());
	
	if (lenIndex > 6){
		if (fileString.find("_") != posIndex+6){
			throw std::invalid_argument("Separator between date and time not found");
		}
		
		numString = fileString.substr(posIndex+7, 2);
		time.tm_hour = atoi(numString.c_str());
	
		numString = fileString.substr(posIndex+9, 2);
		time.tm_min = atoi(numString.c_str()); 

		numString = fileString.substr(posIndex+11, 2);
		time.tm_sec = atoi(numString.c_str());
		
	} else {
		time.tm_hour = 0;
		time.tm_min = 0;
		time.tm_sec = 0;
		time.tm_gmtoff = 0;
	}

	
	// Convert to unix time stamp
	if (debug>3) printf("File index %s\n", asctime(&time));
	index = timegm(&time);
	
	return (index);
}


void Ceilometer::replaceItem(const char **header, const char *itemTag, const char *newValue){
	bool findTag;
	char *ptr;
	char *startChar;
	char *endChar;
	int i;
	int len;
	
	
	// TODO: Add length of the header to avoid searching outside the header !!!
	
	findTag = false;
	i = 0;
	while ( (!findTag) && (i < 20)) {
		ptr = strcasestr(*header,  itemTag);
		if (ptr > 0){
			startChar = strstr(ptr, ": "); 
			if (startChar > 0) {
				
				// Replace the data in the header
				// TODO: Move the rest of the header?!
				//       Check if the value has the same length?!
				len = strlen(newValue);
				strncpy(startChar+2, newValue,len);
				// TODO: End is not found properly?!	
			}
		}
		i++;
	}
	
	*header = startChar+2 + len;
}




const char *Ceilometer::getStringItem(const char **header, const char *itemTag){
	bool findTag;
	char *ptr;
	char *startChar;
	char *endChar;
	int i;
	int len;
	
	
	// TODO: Add length of the header to avoid searching outside the header !!!
	
	findTag = false;
	i = 0;
	while ( (!findTag) && (i < 20)) {
		ptr = strcasestr(*header,  itemTag);
		if (ptr > 0){
			startChar = strstr(ptr, ": "); 
			if (startChar > 0) {
			 			
			  // Find the end of the line
			  // TODO: End is not found properly?!	
				endChar = strstr(ptr, "\n");
				if (endChar > 0){
					//printf("getStringItem:  %02x %02x %02x %02x --- ", *(endChar-2), *(endChar-1), endChar[0], endChar[1]);
					
					len = endChar - startChar - 3;
					std::string tag(startChar+2, len); 
					buffer = tag;
			        findTag = true;
				}
			}
		}
		i++;
	}
	
	*header = endChar+1;
	return (buffer.c_str());
}


int Ceilometer::getNumericItem(const char **header, const char *itemTag){
	int value;
	const char *ptr;
		
	ptr = getStringItem(header, itemTag);
	value = atoi(ptr);
	
	return(value);
}


unsigned long Ceilometer::getSensorGroup(){
	unsigned long number;
	
	number = 0;
	buffer = "";
	if (sensorGroup == "chm") {
		number = 1; 
		buffer = "standard";
	}

	if (sensorGroup == "dat") {
		number = 2; 
		buffer = "online";
	}
	
	if (sensorGroup == "nc") {
		number = 3; 
		buffer = "raw data";
	}
	
	return( number);
}




void Ceilometer::readHeader(const char *filename){
	int fd; 
	const char *headerReadPtr;
	char line[256];
	int n;
	int i;
	int heightOffset;
	char heightUnit[5];
	int len; 
	
	
	// There is no header for this format
	// This means all output is static all the time?!
	// Some header informations are hidden in the data (e.g. height)
	// --> So read the first data set
		
	printf("_____Reading header information_____________________\n");
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		sprintf(line, "Error opening file %s", filename);
		throw std::invalid_argument(line);
	}
	
	// Read the complete header 
	// No need for character code conversions
	// Read the first data set - there is no header for these sensor group 
	len = this->lenDataSet;
	headerRaw = new unsigned char [ len ]; // Is stored in the class variables
	n = read(fd, headerRaw, len);
	printf("Bytes read %d from file %s\n", n, filename);
	
	close(fd);
	
	if (n<len) {
		// There is no header defined in the data format. Instead the data from the 
		// First data set is read -- of course when starting with a new file there is no
		// data set available. So it makes no sense to complain here!
		//throw std::invalid_argument("No header found in data file");		
		return;
	}
	
	
	//
	// Read parameters
	//
	printf("Module: \t\t%s, ID %03d, Group %s ID %d\n", moduleName.c_str(), moduleNumber, 
		   sensorGroup.c_str(), sensorGroupNumber);

	
	// Sampling time
	
	// Height (offset)
	headerReadPtr = (const char *) headerRaw + 72;
	sscanf(headerReadPtr, "%d", &heightOffset);
	printf("Height = %d\n", heightOffset);
	

	// Unit (m/ft)
	// In case of feet the values need to be converted to m
	*heightUnit = 0;
	headerReadPtr =  (const char *) headerRaw + 77;
	strncpy(heightUnit, headerReadPtr, 2);
	if (heightUnit[1] == ' ') heightUnit[1] = 0;
	else heightUnit[2] = 0;
	printf("Unit = <%s>\n", heightUnit);

	// Device ID + fabrication date 
	
	// Software version DAQ + Processing
	
	
	// Reference time == time stamp of first entry 
	std::string timeString;
	std::string dateString;
	unsigned long timestamp;
	headerReadPtr = (const char *) headerRaw + 12; // Date
	headerRaw[20] = 0;
	dateString = headerReadPtr;
	dateString.insert(6,"20"); // Insert full year number in string
	headerReadPtr = (const char *) headerRaw + 21; // Time
	headerRaw[26] = 0;
	timeString = headerReadPtr;	
	timeString += ":";
	headerRaw[235] = 0;
	timeString += (const char *) headerRaw + 233;	
	//timeString += ":00"; // Add missing seconds
	printf("Reference time stamp: \t[%s] [%s]\n", dateString.c_str(), timeString.c_str());
	
	timestamp = getTimestamp(dateString.c_str(), timeString.c_str());
	tRef.tv_sec = timestamp;
	tRef.tv_usec = 0;
	
	
	// Number of sensors
	nSensors = 8;
	printf("Number of sensors %d\n", nSensors);
	
	// List of sensors
	if (sensor > 0 ) delete [] sensor;
	sensor = new struct sensorType [nSensors];
	
	sensor[0].comment = "Cloud level 1";
	sensor[1].comment = "Cloud level 2";
	sensor[2].comment = "Cloud level 3";
	sensor[3].comment = "Penetration depth 1";
	sensor[4].comment = "Penetration depth 2";
	sensor[5].comment = "Penetration depth 3";
	sensor[6].comment = "Vertical visibility";
	sensor[7].comment = "Detection range";

	for (i=0;i<nSensors;i++){
		sensor[i].height	= heightOffset;
	}

	for (i=0;i<nSensors;i++){
		sensor[i].height	= heightOffset;
		printf("Sensor %3d: %s, %.1f %s\n", i+1, sensor[i].comment.c_str(), 
			   sensor[i].height, heightUnit);
	}
}



void Ceilometer::writeHeader(){
	// Nothing to do -- there is no header
}


// TODO: Move the parsing part to separate functions and move rest to base class
void Ceilometer::readData(const char *dir, const char *filename){
	struct timeval t0, t1;
	struct timezone tz;

	unsigned char *buf;
	int len;
	int n;
	int fd;
	int i, j;
	char *sensorString;
	float *sensorValue;
	int err;
	int sensorPtr[] = {27, 33, 39, 45, 50, 55, 60, 66, 72};
	std::string timeString;
	std::string dateString;
	unsigned long timestamp;
	
	
	FILE *fmark;
	std::string filenameMarker;
	std::string filenameData;
	struct timeval lastTime;
	unsigned long lastPos;
	unsigned long lastIndex;
	struct timeval tData;
	struct timeval tWrite;
	char line[256];

#ifdef USE_MYSQL
	MYSQL_RES *res;
	MYSQL_RES *resTables;
	MYSQL_ROW row;
	MYSQL_ROW table;
	std::string tableName;
	std::string sql;
	char sData[50];
#endif
	
	// Data format:
	// Integer values for the heights
	// If no cloud is found than NODT is send
	// If signal strenght is not high enough "NaN"
	// Negative error codes

	
	// Compile file name
	filenameData = dir;
	filenameData += filename;
	//printf("<%s> <%s> <%s>\n", dir, filename, filenameData.c_str());

	
	// If number of sensors is unknown read the header first
	if (nSensors == 0) readHeader(filenameData.c_str());
	if (sensor[0].name.length() == 0) getSensorNames(sensorListfile.c_str());
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
	
	if (debug > 0) printf("_____Reading data_____________________\n");	
	
	// Allocate memory for one data set
	len = 237;
	buf = new unsigned char [len];
	sensorValue = new float [nSensors];
	
	if (debug > 1) printf("Open data file %s\n", filenameData.c_str());
	fd = open(filenameData.c_str(), O_RDONLY);
	
	
	// Get the last time stamp + file pointer from
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dir, moduleNumber, sensorGroupNumber);
	filenameMarker =line;
	if (debug > 1) printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %d %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
		fclose(fmark);
	}

	if (lastPos == 0) lastPos = 0; // Move the the first data
	
	// Find the beginning of the new data
	if (debug > 1) printf("LastPos: %ld\n", lastPos);
	lseek(fd, lastPos, SEEK_SET);
	
	
	n = len;
	int iLoop = 0;
	while ((n == len) && (iLoop< 1000)) {
		n = read(fd, buf, len);
		
		if (n == len){
			
			//
			// TODO: Check for valid data format
			//       If not stop with this file and continue with the next one
			//
			if (strstr((const char *) buf, "X1TA") != (char *) (buf+1)) {
				printf("No valid ceilometer data found -- will continue with next file\n");
				
				// Continue with the next file
				break; // or n = 0
			}
			
			if (debug > 1) printf("%4d: Received %4d bytes ---  ", iLoop, n);
			
			
			// TODO: Put in a separate function...
			// Read the time stamp
			buf[20] = 0;
			dateString = (char *) (buf + 12);
			dateString.insert(6,"20"); // Insert full year number in string
			buf[26] = 0;
			timeString = (char *) (buf + 21);
			timeString += ":";
			buf[235] = 0;
			timeString += (char *) (buf + 233);
			//timeString += ":00"; // Add missing seconds
			if (debug > 1) printf("[%s] [%s]", dateString.c_str(), timeString.c_str());

			
			timestamp = getTimestamp(dateString.c_str(), timeString.c_str());
			tData.tv_sec = timestamp;
			tData.tv_usec = 0;
			
			if (debug > 1) printf(" %ld  %d  ---- ", tData.tv_sec, tData.tv_usec);
			
			// Read data values
			//printf("%s\n", buf);
			if (debug > 1) printf("Sensors:");
			for (j = 0; j < nSensors; j++) {
				sensorString = (char *) (buf + sensorPtr[j]);
				//buf[sensorPtr[1]-1] = 0;
				sensorValue[j] = noData;
				err = sscanf(sensorString, "%f", &sensorValue[j]);
				
				if (debug > 1) printf("%5.0f ", sensorValue[j]);
			}
			if (debug > 1) printf("\n");
			//if (debug > 1) printf("Sensor %s = %.0f  (err=%d)\n",sensorString, sensorValue[0], err);
	
			
#ifdef USE_MYSQL	
			if (db > 0){
				// Write dataset to database
				// Store in the order of appearance
				//printf("Write record to database\n");
		
				sql = "INSERT INTO `";
				sql += dataTableName + "` (`sec`,`usec`";
				for (i=0; i<nSensors; i++){
					if (sensorValue[i] != noData) {
						sql += ",`";
						sql += sensor[i].name;
						sql += "`";
					}
				}
				sql +=") VALUES (";
				sprintf(sData, "%ld, %d", tData.tv_sec, tData.tv_usec);
				sql += sData;
				for (i=0; i<nSensors; i++){
					if (sensorValue[i] != noData) {
						sprintf(sData, "%f", sensorValue[i]);
						sql += ",";
						sql += sData;
					}
				}
				sql += ")";
				
				//printf("SQL: %s (db = %d)\n", sql.c_str(), db);

				gettimeofday(&t0, &tz);
			
				if (mysql_query(db, sql.c_str())){
					fprintf(stderr, "%s\n", sql.c_str());
					fprintf(stderr, "%s\n", mysql_error(db));
					
					// If this operation fails do not proceed in the file?!
					printf("Error: Unable to write data to database\n");
					throw std::invalid_argument("Writing data failed");
					break;
				}
				
				gettimeofday(&t1, &tz);
				printf("DB insert duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));
	
			} else {
				printf("Error: No database availabe\n");
				throw std::invalid_argument("No database");
			}
#endif // of USE_MYSQL
			
			lastPos += n;
		}
		iLoop++;
	}
	
	if (n < len) {
		fd_eof = true;
	} else {
		fd_eof = false;
	}

	if (debug > 1) printf("\n");
	if (debug > 1) printf("Position of file %ld\n", lastPos);
	
	
	// Write the last valid time stamp / file position
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %d %ld\n", lastIndex, lastTime.tv_sec, lastTime.tv_usec, lastPos);
		fclose(fmark);
	}
	
	
	close(fd);
	delete buf;
	delete [] sensorValue;
}


void Ceilometer::updateDataSet(unsigned char *buf){
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

