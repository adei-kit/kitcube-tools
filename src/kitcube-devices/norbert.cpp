/********************************************************************
* Description:
* Author: Norbert Flatinger, norbert.flatinger@kit.edu
* Created at: Wed Dec  2 2009
* Computer: ipeflatinger1
* System: Linux 2.6.26-2-686 on i686
*
* Copyright (c) 2009 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#include "norbert.h"

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


Norbert::Norbert():DAQBinaryDevice(){
	
	moduleType = "NF";
	moduleNumber = 190;	// Norbert default number
	sensorGroup = "dat";

	iniGroup = "Norbert";
	
	lenHeader = 0x33;	// CAUTION: header has 2 lines!
	lenDataSet = 14;
	
	headerRaw = 0;		// unsigned char*
}


Norbert::~Norbert(){
	// Free header memory
	if (headerRaw > 0) delete [] headerRaw;
}


void Norbert::setConfigDefaults(){
	char line[256];

	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
	
	moduleName = "Norbert";
	moduleComment = "Norbert's";
	
	sprintf(line, "Norbert.%s.template", sensorGroup.c_str());
	datafileTemplate = line;

	sprintf(line, "20<index>.%s", sensorGroup.c_str());
	datafileMask = line;
	//printf("datafileMask = %s\n", datafileMask.c_str());
	
	sprintf(line, "Norbert.%s.sensors", sensorGroup.c_str());
	this->sensorListfile = line;
}

 
const char *Norbert::getDataDir(){
	char line[256];
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "%s/Daten/", moduleName.c_str());
	buffer = line;
	return(buffer.c_str());
}


const char *Norbert::getDataFilename(){
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

	//printf("Norbert: Get Datafilename = %s\n", line);
	return(buffer.c_str());
}



int Norbert::getFileNumber(char *filename){
	std::string name;
	size_t posIndex;
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
	// FIXME: this is done in DAQDevice::readInifile already. Why repeat it here? What to do in case of error?
	posIndex = datafileMask.find("<index>");
	if (posIndex == std::string::npos) {
		printf("Error: There is no tag <index> in datafileMask=%s specified in inifile %s\n",
			datafileMask.c_str(), inifile.c_str());
	}

	// extract filename prefix and suffix out of datafileMask
	filePrefix = datafileMask.substr(0, posIndex);
	fileSuffix = datafileMask.substr(posIndex + 7);
	if (debug > 3) printf("Position of <index> in %s is  %d -- data file prefix/suffix: %s / %s (debug = %d)\n",
				datafileMask.c_str(), posIndex, filePrefix.c_str(), fileSuffix.c_str(), debug);

	// calculate actual length of <index> in filename
	fileString = filename;
	lenIndex = fileString.length() - filePrefix.length() - fileSuffix.length();
	
	// Check for starting tag
	if (fileString.find(filePrefix) != 0) {
		throw std::invalid_argument("Prefix not found");
	}
	if (fileString.find(fileSuffix) < 0) {
		throw std::invalid_argument("Suffix not found");
	}
	
	
	// Get year
	numString = fileString.substr(posIndex, 2);
	time.tm_year = atoi(numString.c_str()) + 100;
	
	numString = fileString.substr(posIndex + 2, 2);
	time.tm_mon = atoi(numString.c_str()) - 1;
	
	numString = fileString.substr(posIndex + 4, 2);
	time.tm_mday = atoi(numString.c_str());
	
	if (lenIndex > 6){
		if (fileString.find("_") != posIndex + 6){
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
	index = timegm(&time);

	if (debug > 3)
		printf("File index %s\n", asctime(&time));
	
	return (index);
}


void Norbert::replaceItem(const char **header, const char *itemTag, const char *newValue){
	bool findTag;
	char *ptr;
	char *startChar;
	//char *endChar;
	int i;
	int len;
	
	
	// TODO: Add length of the header to avoid searching outside the header !!!
	
	findTag = false;
	i = 0;
	while ( (!findTag) && (i < 20)) {
		ptr = strcasestr(*header,  itemTag);
		if (ptr > 0){
			startChar = strstr(ptr, ":\t");
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


const char *Norbert::getStringItem(const char **header, const char *itemTag){
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


int Norbert::getNumericItem(const char **header, const char *itemTag){
	int value;
	const char *ptr;
	
	ptr = getStringItem(header, itemTag);
	value = atoi(ptr);
	
	return(value);
}


unsigned long Norbert::getSensorGroup(){
	unsigned long number;
	
	number = 0;
	buffer = "";

	if (sensorGroup == "dat") {
		number = 1;
		buffer = "online";
	}

	printf("Sensor Group ID: %ld\n", number);
	
	return number;
}


void Norbert::readHeader(const char *filename){
	int fd;
	const char *headerReadPtr;
	char line[256];
	int n;
	int len;
	
	
	printf("_____Reading header information_____________________\n");
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		sprintf(line, "Error opening file %s", filename);
		throw std::invalid_argument(line);
	}
	
	// Read the complete header
	// No need for character code conversions
	len = lenHeader;
	headerRaw = new unsigned char [ len ];	// Is stored in the class variables
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

	// Reference time == time stamp of first entry
	std::string dateString;
	std::string timeString;
	unsigned long timestamp;

	headerReadPtr = (const char *) headerRaw + 0x28;	// position of date string in Header
	dateString.assign(headerReadPtr, 10);

/*
	headerReadPtr = (const char *) headerRaw + 21;	// position of time string in header
	headerRaw[26] = 0;
	timeString = headerReadPtr;
	timeString += ":";
	headerRaw[235] = 0;
	timeString += (const char *) headerRaw + 233;	
	//timeString += ":00"; // Add missing seconds
*/
	timeString = "empty";

	printf("Reference time stamp: \t[%s] [%s]\n", dateString.c_str(), timeString.c_str());
	
	timestamp = getTimestamp(dateString.c_str(), timeString.c_str());	// should work with timeString "empty", too
	tRef.tv_sec = timestamp;
	tRef.tv_usec = 0;
	
	
	// Number of sensors
	nSensors = 1;
	printf("Number of sensors %d\n", nSensors);
	
	// List of sensors
	if (sensor > 0 ) delete [] sensor;
	sensor = new struct sensorType [nSensors];
	
	sensor[0].comment = "Triangle Signal";

}



void Norbert::writeHeader(){
	// muss jetzt Header schreiben :-(
	struct timezone tz;
	const char *headerReadPtr;
	struct tm *t;
	char time[20];
	char date[20];
	int n;
	std::string filename;
	int len;
	

	if (fd_data <=0) throw std::invalid_argument("Data file not open");

	// Read one sample data and replace the starting time by the original time
	len = this->lenHeader;
/*
	// Read the template header
	if (datafileTemplate.length() == 0) throw std::invalid_argument("No template file given");	
	filename = configDir + datafileTemplate;
	printf("Reading header from template file %s\n", filename.c_str());
	readHeader(filename.c_str());
	headerReadPtr = (const char *) headerRaw;
	
	if (headerRaw == 0) {
		printf("Error: No template header found\n");
		throw std::invalid_argument("No template header found");
	}
*/
	headerReadPtr = (const char *) headerRaw;

	// Replace the date by the actual date
	gettimeofday(&tRef, &tz);
	t = gmtime( &tRef.tv_sec);
	sprintf(time, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
	sprintf(date, "%02d.%02d.%4d", t->tm_mday, t->tm_mon+1, t->tm_year+1900);
	
	//replaceItem(&headerReadPtr, "Referenzzeit", time);
	replaceItem(&headerReadPtr, "Datum", date);
	printf("%s", headerRaw);

	// write header
	n = write(fd_data, headerRaw, len);
	printf("Write header of %d bytes\n", n);
}


// TODO: Move the parsing part to separate functions and move rest to base class
void Norbert::readData(const char *dir, const char *filename){
	unsigned char *buf;
	int len;
	int n;
	int fd;
	int j;
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
	//struct timeval tWrite;
	char line[256];

#ifdef USE_MYSQL
	struct timeval t0, t1;
	struct timezone tz;
	int i;
	//MYSQL_RES *res;
	//MYSQL_RES *resTables;
	//MYSQL_ROW row;
	//MYSQL_ROW table;
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
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
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
			
			if (debug > 1) printf(" %ld  %ld  ---- ", tData.tv_sec, tData.tv_usec);
			
			// Read data values
			//printf("%s\n", buf);
			if (debug > 1) printf("Sensors:");
			for (j=0;j<nSensors; j++){
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
				sprintf(sData, "%ld, %ld", tData.tv_sec, tData.tv_usec);
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
	
	if (n < len) { fd_eof = true; }
	else { fd_eof = false; }

	if (debug > 1) printf("\n");
	if (debug > 1) printf("Position of file %ld\n", lastPos);
	
	
	// Write the last valid time stamp / file position
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, lastTime.tv_sec, lastTime.tv_usec, lastPos);
		fclose(fmark);
	}
	
	close(fd);
	delete buf;
	delete [] sensorValue;
}


void Norbert::updateDataSet(unsigned char *buf){
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
	if (debug > 2)
		printf("Line before: %s", buf);

	// write actual timestamp to buffer
	sprintf((char *) buf,"%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
	buf[8]='\t';

	if (debug > 2)
		printf("Line after : %s", buf);
	
	if (debug > 1)
		printf("%02d.%02d.%02d  %02d:%02d:%02d\n",
			time->tm_mday, time->tm_mon+1, time->tm_year-100,
			time->tm_hour, time->tm_min, time->tm_sec);
	
}
