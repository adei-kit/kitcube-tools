//
// C++ Implementation: daqasciidevice
//
// Description: 
//
//
// Author: Norbert Flatinger <norbert.flatinger@kit.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "daqasciidevice.h"


DAQAsciiDevice::DAQAsciiDevice():DAQDevice(){
	// Clear counters for writing in a loop
	nSamples = 0;
	nTemplate = 0;
}


DAQAsciiDevice::~DAQAsciiDevice(){
}


void DAQAsciiDevice::openFile(){ // for writing
	char line[256];
	FILE *fmark;
	FILE *fd;
	std::string msg;
	std::string nameTemplate;
	
	
	printf("_____DAQAsciiDevice::openFile()_____\n");
	
	// Check if template file is existing and contains header + data
	nameTemplate = configDir + datafileTemplate;
	fd = fopen(nameTemplate.c_str(), "r");
	if (fd <= 0) {
		msg = "Template file not found -- " + nameTemplate;
		throw std::invalid_argument(msg.c_str());
	}
	fclose(fd);
	
	// Read header, test if it exists and is valid?!
	readHeader(nameTemplate.c_str());	// FIXME/TODO: do we really need this here?!?

	
	// Read index from file, if the value has not been initialized before
	// The markers for the different modules are independant
	if (fileIndex == 0){
		fileIndex = 1; // Default value
		sprintf(line, "%s.kitcube-data.marker.%03d.%d", dataDir.c_str(), moduleNumber, sensorGroupNumber);
		filenameMarker = line;
		fmark = fopen(filenameMarker.c_str(), "r");
		if (fmark > 0) {
			fscanf(fmark, "%ld%d", &fileIndex, &nLine);
			fclose(fmark);
		}
	}
	
	
	// Open ASCII file for writing the data
	path = dataDir +  getDataDir();
	filename = getDataFilename(); // Warning using global variable for returning data !!!
	fullFilename = path + filename;
	
	printf("KITCube-Device (type %s): Open datafile \"%s\"\n", moduleType.c_str(), fullFilename.c_str());
	createDirectories(fullFilename.c_str());	// FIXME: don't use fullFilename here!
	
	fdata = fopen(fullFilename.c_str(), "a");	// use only "a", so ftell(...) works
	if (fdata <= 0) {
		printf("Error opening data file \"%s\"\n", filename.c_str());
		throw std::invalid_argument("Error opening data file\n");
	}
	
	// Write header if the file was not exisitng before
	if (ftell(fdata) == 0)	// use "a" when opening file, so this works
		writeHeader();
}


void DAQAsciiDevice::closeFile(){
	if (fdata > 0) {
		fclose(fdata);
		fdata = 0;
	}
}


void DAQAsciiDevice::readData(const char *dir, const char *filename){
	char *buf;
	int len;
	//int n;
	FILE *fd_data_file;
	int j, k;
	//char *sensorString;
	double *local_sensorValue;
	//int err;
	//std::string timeString;
	//std::string dateString;
	//unsigned long timestamp;
	
	
	FILE *fmark;
	std::string filenameMarker;
	std::string filenameData;
	struct timeval lastTime;
	unsigned long lastPos;
	unsigned long currPos;
	unsigned long lastIndex;
	struct timeval timestamp_data;
	//struct timeval tWrite;
	char line[256];
	char *lPtr;
	
#ifdef USE_MYSQL
	//MYSQL_RES *res;
	//MYSQL_RES *resTables;
	//MYSQL_ROW row;
	//MYSQL_ROW table;
	std::string tableName;
	std::string sql;
	char sData[50];
	struct timeval t0, t1;
	struct timezone tz;
	int i;
#endif
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// Compile file name
	filenameData = dir;
	filenameData += filename;
	//printf("<%s> <%s> <%s>\n", dir, filename, filenameData.c_str());	
	
	
	// If number of sensors is unknown read the header first
	//if (nSensors == 0) readHeader(filenameData.c_str());
	// For every file the header should be read ?!
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
	
	// Allocate memory for one data set
	len = lenDataSet;
	buf = new char [len];
	
	// Allocate memory for sensor values
	if (profile_length != 0) {
		local_sensorValue = new double [nSensors * profile_length];
	} else {
		local_sensorValue = new double [nSensors];
	}
	
	if (debug >= 1)
		printf("Open data file: %s\n", filenameData.c_str());
	fd_data_file = fopen(filenameData.c_str(), "r");
	if (fd_data_file <= 0) {
		printf("Error opening data file %s\n", filenameData.c_str());
		return;
	}
	
	
	// Get the last time stamp + file pointer from 
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dir, moduleNumber, sensorGroupNumber);
	filenameMarker = line;
	if (debug >= 1)
		printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
		fclose(fmark);
		
		// Read back the data time stamp of the last call
		timestamp_data.tv_sec = lastTime.tv_sec;
		timestamp_data.tv_usec = lastTime.tv_usec;
		
		if (debug >= 1)
			printf("Last time stamp was %ld\n", lastTime.tv_sec);
	}
	
	if (lastPos == 0)
		lastPos = lenHeader; // Move to the the first data
	
	
	// Find the beginning of the new data
	if (debug >= 1)
		printf("Last position in file: %ld\n", lastPos);
	
	fseek(fd_data_file, lastPos, SEEK_SET);
	
	lPtr = (char *) 1;
	int iLoop = 0;
	while ((lPtr > 0) && (iLoop < 1000000)) {
		lPtr = fgets(buf, len, fd_data_file);
		
		if (lPtr > 0){
			// Module specific implementation
			// Might be necessary to
			parseData(buf, &timestamp_data, local_sensorValue);
			
			// print sensor values
			if (debug >= 4) {
				printf("%4d: Received %4d bytes --- ", iLoop, (int) strlen(buf));
				printf("%lds %6ldus --- ", timestamp_data.tv_sec, timestamp_data.tv_usec);
				if (profile_length != 0) {
					for (j = 0; j < nSensors; j++) {
						for (k = 0; k < profile_length; k++) {
							printf("%10.3f ", local_sensorValue[j * profile_length + k]);
						}
					}
				} else {
					for (j = 0; j < nSensors; j++) {
						printf("%f ", local_sensorValue[j]);
					}
				}
				printf("\n");
			}
#ifdef USE_MYSQL
			if (db > 0){
				// Write dataset to database
				// Store in the order of appearance	
				//printf("Write record to database\n");
				
				sql = "INSERT INTO `";
				sql += dataTableName + "` (`usec`";
				for (i = 0 ; i < nSensors; i++) {
					if (local_sensorValue[i] != noData) {
						sql += ",`";
						sql += sensor[i].name;
						sql += "`";
					}
				}
				sql += ") VALUES (";
				sprintf(sData, "%ld", timestamp_data.tv_sec * 1000000 +  timestamp_data.tv_usec);
				sql += sData;
				if (profile_length != 0) {
					for (i = 0; i < nSensors; i++) {
						sql += ", '";
						for (k = 0; k < profile_length; k++) {
							sprintf(sData, "%f, ", local_sensorValue[i * profile_length + k]);
							sql += sData;
						}
						sql += "'";
					}
				} else {
					for (i = 0; i < nSensors; i++) {
						if (local_sensorValue[i] != noData) {
							sql += ",";
							sprintf(sData, "%f", local_sensorValue[i]);
							sql += sData;
						}
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
				if (debug >= 5)
					printf("DB insert duration: %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec));
			} else {
				printf("Error: No database availabe\n");
				throw std::invalid_argument("No database");
			}
#endif // of USE_MYSQL
			
		}
		iLoop++;
	}
	
	if (lPtr == 0) {
		fd_eof = true;
	} else {
		fd_eof = false;
	}
	
	// Get the position in this file
	currPos = ftell(fd_data_file);
	processedData += currPos - lastPos;
	
	if (debug >= 1)
		printf("Position in file: %ld; processed data: %ld Bytes\n", currPos, currPos - lastPos);
	
	// Write the last valid time stamp / file position
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, timestamp_data.tv_sec, timestamp_data.tv_usec, currPos);
		fclose(fmark);
	}
	
	fclose(fd_data_file);
	delete buf;
	delete [] local_sensorValue;
}
