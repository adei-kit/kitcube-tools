/***************************************************************************
                          daqbinarydevice.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "daqbinarydevice.h"


DAQBinaryDevice::DAQBinaryDevice():DAQDevice(){
	// Clear counters for writing in a loop
	nSamples = 0;
	nTemplate = 0;
}


DAQBinaryDevice::~DAQBinaryDevice(){
}


void DAQBinaryDevice::openFile(){ // for writing
	char line[256];
	FILE *fmark;
	int fd;
	std::string msg;
	std::string nameTemplate;
	std::string fullFilename;
	std::string copy_command;
	
	printf("_____DAQBinaryDevice::openFile_____\n");
	
	// Check if template file is existing and contains header + data
	nameTemplate = configDir + datafileTemplate;
	fd = open(nameTemplate.c_str(), O_RDWR);
	if (fd <= 0) {
		msg = "Template file not found -- " + nameTemplate;
		throw std::invalid_argument(msg.c_str());
	}
	close(fd);
	
	if (sensorGroup != "nc"){
		// Read header, test if it exists and is valid?!
		readHeader(nameTemplate.c_str());	// FIXME/TODO: do we really need this here?!?
	}
	
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
	
	
	// Open binary file for writing the data
	path = dataDir +  getDataDir();
	filename = getDataFilename(); // Warning using global variable for returning data !!!
	fullFilename = path + filename;
	
	printf("KITCube-Device (type %s): Open datafile %s\n", moduleType.c_str(), fullFilename.c_str());
	createDirectories(fullFilename.c_str());	// FIXME: don't use fullFilename here!
	
	if (sensorGroup == "nc"){
		copy_command = "cp ";
		copy_command += nameTemplate;
		copy_command += " ";
		copy_command += fullFilename;
		system(copy_command.c_str());
	} else {
		fd_data = open(fullFilename.c_str(), O_APPEND | O_RDWR);
		if (fd_data <= 0) {
			// The file does not exist. Create file and write header
			printf("Creating new file \n");
			fd_data = open(fullFilename.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (fd_data > 0) {
				writeHeader();
			} else {
				throw std::invalid_argument("Error crreating new file\n");
			}
		}
	}
}


void DAQBinaryDevice::closeFile(){
	if (fd_data > 0) {
		close(fd_data);
		fd_data = 0;
	}
}


void DAQBinaryDevice::writeData(){
	unsigned char *buf;
	int fd_templ;
	int len;
	int lenHeader;
	int n;
	//int i;
	std::string filename;
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	if (sensorGroup == "nc"){
		// Analyse time stamp, if new day a new file needs to copied
		if (this->filename != getDataFilename()) {
			openNewFile();
		}
	} else {
		if (fd_data <= 0)
			return;
		
		// Read header to get the reference time of this file !!!
		//
		
		//
		// Use one sample file to generate continuously data - by repetition of the sample data
		//
		
		// Allocate memory for one data set
		len = this->lenDataSet;
		lenHeader = this->lenHeader;
		buf = new unsigned char  [len];
		
		// Open template file 
		filename = configDir + datafileTemplate;
		fd_templ = open(filename.c_str(), O_RDONLY);
		lseek(fd_templ, lenHeader + len * nTemplate, SEEK_SET); // Go to the required data set
		
		// Read data set 
		n = read(fd_templ, buf, len);
		if (n < len) {
			nTemplate = 0; // Reset template counter and try again
			lseek(fd_templ, lenHeader + len * nTemplate, SEEK_SET); // Go to the required data set
			n = read(fd_templ, buf, len);
			if (n < len) {
				printf("Error: No data set found in template\n");
			}
		}
		close(fd_templ);
		
		// Compile the data set for writing to the data file
		if (debug >= 1)
			printf("Received %4d bytes\n", n);
		
		// Replace time stamp
		updateDataSet(buf);
		
		n = write(fd_data, buf, len);
		//printf("Write %d bytes\n", n);
		
		delete buf;
		
		// Increment the counters
		nSamples++;
		nTemplate++;
	}
}


void DAQBinaryDevice::readData(const char *dir, const char *filename){
	unsigned char *buf;
	int len;
	int n;
	int fd_data_file;
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
	long lastIndex;
	struct timeval timestamp_data;
	//struct timeval tWrite;
	char line[256];
	//char *lPtr;
	
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
	buf = new unsigned char [len];
	
	// Allocate memory for sensor values
	if (profile_length != 0) {
		local_sensorValue = new double [nSensors * profile_length];
	} else {
		local_sensorValue = new double [nSensors];
	}
	
	if (debug >= 1)
		printf("Open data file: %s\n", filenameData.c_str());
	fd_data_file = open(filenameData.c_str(), O_RDONLY);
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
	currPos = lastPos;
	
	// Find the beginning of the new data
	if (debug >= 1)
		printf("Last position in file: %ld\n", lastPos);
	
	lseek(fd_data_file, lastPos, SEEK_SET);
	
#ifdef USE_MYSQL
	sql = "LOCK TABLES " + dataTableName + " WRITE";
	mysql_query(db, sql.c_str());
#endif
	
	n = len;
	int iLoop = 0;
	while ((n == len) && (iLoop < 1000000)) {
		n = read(fd_data_file, buf, len);
		
		if (n == len) {
			// Module specific implementation
			// Might be necessary to
			parseData((char *)buf, &timestamp_data, local_sensorValue);
			
			// print sensor values
			if (debug >= 4) {
				printf("%4d: Received %4d bytes --- ", iLoop, n);
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
				sprintf(sData, "%ld", timestamp_data.tv_sec * 1000000 + timestamp_data.tv_usec);
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
			currPos += n;
		}
		iLoop++;
	}
	
#ifdef USE_MYSQL
	sql = "UNLOCK TABLES";
	mysql_query(db, sql.c_str());
#endif
	
	if (n < len) {
		fd_eof = true;
	} else {
		fd_eof = false;
	}
	
	
	
	processedData += currPos - lastPos;
	
	if (debug >= 1)
		printf("Position in file: %ld; processed data: %ld Bytes\n", currPos, currPos - lastPos);
	
	// Write the last valid time stamp / file position
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, timestamp_data.tv_sec, timestamp_data.tv_usec, currPos);
		fclose(fmark);
	}
	
	close(fd_data_file);
	delete buf;
	delete [] local_sensorValue;
}
