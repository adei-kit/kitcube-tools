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
    
    nMap = 0;
    dynMap = 0;
}


DAQAsciiDevice::~DAQAsciiDevice(){
    if (dynMap > 0) delete [] dynMap;
}


void DAQAsciiDevice::openFile(){ // for writing
	char line[256];
	FILE *fmark;
	FILE *fd;
	std::string msg;
	std::string nameTemplate;
	
	
	if (debug > 2) printf("_____DAQAsciiDevice::openFile()_____\n");
	
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
	
	if (debug > 1) printf("KITCube-Device (type %s): Open datafile \"%s\"\n", moduleType.c_str(), fullFilename.c_str());
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


int DAQAsciiDevice::readHeader(const char *filename){
	int i;
	FILE *data_file_ptr;
	char *line_of_data = NULL;
	size_t len = 0;
	
	// Reading header from a generic CSV file
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);

	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	
	// Definintions:
	// Character list for comment lines
	// Field separators
	
	// Drop comment lines 
	if (debug > 3) printf("Header in line: %d\n", headerLine);
	i = 1;
	while(i < headerLine) {
		// Drop the first nComment lines
		if (read_ascii_line(&line_of_data, &len, data_file_ptr) != -1){
			if (debug > 3) printf("Line: %s", line_of_data);
		}
		i++;
    }

	// Read sensor configuration
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return(-1);
    }
	if (debug > 3) printf("Header: %s", line_of_data);
	
	// close data file
	fclose(data_file_ptr);
	

	// Get the ID's of process channels
	int j;
	char *pCh;
	char *pChRes;
	char *buffer = new char [strlen(line_of_data)+1];
	//char buffer[256]; // increased to 1024?!
	
    // TODO: Allocate map dynamically (currently it's 256) !!!
    //printf("nSensors = %d\n", nSensors);
    if (dynMap != 0) delete [] dynMap;
    dynMap = new int[nSensors];
    
    
	i = 0;
	pCh = strtok(line_of_data, dataSep.c_str());
	if (pCh) pCh = strtok(NULL, dataSep.c_str());
	while (pCh && (i < nSensors)){
		
		// Strip the trailing "sampletime"
        // Remove quotes if available, alternative add quote to the dataSep-List
        int k = 0;
        while ((isspace(pCh[k]) || (pCh[k] == '"')) && (k < (int) strlen(pCh))){
            k++;
        }
        strcpy(buffer, pCh+k);

        k = strlen(buffer)-1;
        while ((isspace(buffer[k]) || (buffer[k] == '"')) && (k>=0)){
            k--;
        }
        buffer[k+1] = 0;
               
		if (debug > 3) printf("Searching for column %s in sensor configuration\n", buffer);
               
		//map[i] = -1;
        dynMap[i] = -1;
		for (j=0;j<nSensors;j++){
			
			pChRes = strstr(buffer, sensor[j].comment.c_str());
			if (pChRes == buffer) {
                dynMap[i] = j;
            }
		}
		
		if (debug > 2)
			printf("Channel %d: %s --> %d\n", i, buffer, dynMap[i]);
		
		if (dynMap[i] == -1) {
			printf("DAQAsciiDevice: Error reading sensor configuration!\n");
			printf("                Sensor name %s not found in sensor definition file", buffer);
			free(line_of_data);
			delete [] buffer;
			
			throw(std::invalid_argument("Sensor name not found in definition file"));
			return(1); // TODO: Howto stop the application here???
		}
		
		// Read the next token
		i++;
		pCh = strtok(NULL, dataSep.c_str());
		
	}
	nMap = i;
	
    if (debug>5){
        printf("Map (n=%d): ", nMap);
        for (i=0;i<nMap;i++){
            printf(" %d", dynMap[i]);
        }
        printf("\n");
    }
    
    
	// Other settings ?!
	noData = 99999;
	
	// Can be detemined?!
	lenHeader = 0;	// CAUTION: header contains 11 lines
	
	lenDataSet = 0;	// 198 bytes + 1 for '\0' in fgets();
	// this is only one line of data,
	// a data set consists of "number of height levels" lines
	
	profile_length = 0;	// scalar data
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
		sensor[i].data_format = "<scalar>";
        sensor[i].size = 1;
	}
    
	free(line_of_data);
	delete [] buffer;
	
	return 0;
	
	
}


int DAQAsciiDevice::parseData(char* line, struct timeval* l_tData, double *sensorValue){
	
	// Reading data from a generic CSV file

	int i;
	char *puffer, *pSubsec, *pRes;
	double value;
	int err;	
	struct tm time_stamp_data;
    
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	

	// TODO: Don't read comment lines
	if (strchr(commentChar.c_str(), line[0]) > 0){
		if (debug > 3) printf("Drop: %s", line);
		return 1;
	}
	
	// Write no data to all other variables
	for (i=0; i <nSensors;i++){
		sensorValue[i] = noData;
	}
	
	// read sensor values
	i = 0;
	
	// Read timestamp
	puffer = strtok(line, dataSep.c_str());
	if (puffer) {
        // Remove quotes if available
        if (puffer[0] == '"') puffer = puffer +1;
        if (puffer[strlen(puffer)-1] == '"') puffer[strlen(puffer)-1] = 0;
        
        if (debug > 5)
            printf("Timestamp %s (format: %s)\n", puffer, timestampFormat.c_str());
        
        if (timestampFormat.length() == 0){
            // Read timestamp in seconds
            err = sscanf(puffer, "%ld", &l_tData->tv_sec);
            if (err == 1) {
                if (debug > 3) printf("Timestamp %ld\n", l_tData->tv_sec);
            }
        } else {
            // Parse time stamp format
            memset(&time_stamp_data, 0, sizeof(time_stamp_data));
            pRes = strptime(puffer, timestampFormat.c_str(), &time_stamp_data);
            if (pRes == 0){
                if (debug > 5) printf("Error: Conversion of timestamp %s failed\n", puffer);
                return -1; // No data found
            }
                
                
            if (debug > 3) printf("Date      : %02d.%02d.%4d  %02d:%02d:%02d\n",
                   time_stamp_data.tm_mday, time_stamp_data.tm_mon+1, time_stamp_data.tm_year+1900,
                   time_stamp_data.tm_hour, time_stamp_data.tm_min, time_stamp_data.tm_sec);
            
            l_tData->tv_sec = timegm(&time_stamp_data);
            l_tData->tv_usec = 0;

            // Check if there are usecs given and add them !!!
            pSubsec = strchr(puffer, '.');
            if ( pSubsec > 0){
                err = sscanf(pSubsec, "%lf", &value);
                if (err == 1){
                    l_tData->tv_usec = value * 1000000;
                    if (debug > 3) printf("Subsec string: %s / %lf / %06ld\n", pSubsec, value, l_tData->tv_usec);
                }
            }
            
            if (debug > 3) printf("Timestamp %ld.%06ld\n", l_tData->tv_sec, l_tData->tv_usec);
        }
            
            
		
		puffer = strtok(NULL, dataSep.c_str());
	}
	while ((puffer != NULL) && (i<nMap)) {
		err = sscanf(puffer, "%lf", &value);
		
		if (err == 1) {
			sensorValue[dynMap[i]] = value;
		}
		
		if (debug > 3) printf("Sensor %3d: %20.9f\n", dynMap[i], value);
		
		// Get next value
		puffer = strtok(NULL, dataSep.c_str());
		i++;
	}
	
	return 0;
	
}


void DAQAsciiDevice::readData(std::string full_filename){
	int i;
	char *buf = NULL;
	size_t len = 0;
	int n;
	FILE *fd_data_file;
	int j, k;
    int nSensorElements;
	double *local_sensorValue;
	
	std::string filenameData;
	unsigned long lastPos;
	unsigned long currPos;
	long lastIndex;
	struct timeval timestamp_data = {0};
    int newline;
    double *pSensor;
	
#ifdef USE_MYSQL
	std::string tableName;
	std::string sql;
	char sData[50];
	struct timeval t0, t1;
	struct timezone tz;
    char* esc_str;
#endif
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// Compile file name
	filenameData = full_filename;
	//printf("<%s> <%s> <%s>\n", dir, filename, filenameData.c_str());	
	
#ifdef USE_MYSQL
    if ((db == 0) || (mysql_ping(db) != 0))
        openDatabase();    
#endif
		
	// Allocate memory for sensor values
    // TODO: calculate the length of the data set
    //       What to do with other data formats that are not double arrays?
    //       e.g. cloud camera -- link to data file?
    //       Native e.g. netcdf formats?!
	if (profile_length != 0) {
		nSensorElements = nSensors * profile_length;
	} else {
        nSensorElements = 0;
        for (i=0;i<nSensors;i++) nSensorElements += sensor[i].size;
    }
    
    local_sensorValue = new double [nSensorElements];        
	
	
	if (debug > 1)
		printf("Open data file: %s\n", filenameData.c_str());
	fd_data_file = fopen(filenameData.c_str(), "r");
	if (fd_data_file <= 0) {
		printf("Error opening data file %s\n", filenameData.c_str());
		return;
	}
	
	
	// Get the last time stamp + file pointer from
	loadFilePosition(lastIndex, lastPos, timestamp_data);
 
    // Skip the header
	if ((lastPos == 0) && (lenHeader > 0)){
        // The header is given in bytes
		lastPos = lenHeader;
        
        if (debug > 4) printf("Skip header of %d bytes\n", lenHeader);
        
    }
	currPos = lastPos;
	
	// Find the beginning of the new data
	if (debug > 1)
		printf("Last position in file: %ld\n", lastPos);
	
	fseek(fd_data_file, lastPos, SEEK_SET);
	
    
    // Skip the header, the header is given in lines
	if ((lastPos == 0) && (lenHeaderLines > 0)){
        if (debug > 4) printf("Skip header of %d lines\n", lenHeaderLines);
        
        n = 0;
        i = 0;
        while((n==0) && (i<lenHeaderLines)){
            n = read_ascii_line(&buf, &len, fd_data_file);
            i++;
        }
        
    }
    
#ifdef USE_MYSQL
	sql = "LOCK TABLES " + dataTableName + " WRITE";
	mysql_query(db, sql.c_str());
#endif
	
	n = 0;
	int iLoop = 0;
    int maxLoop = 1000000;
    if (debug > 2) maxLoop = 10;
	while ((n == 0) && (iLoop < maxLoop)) {
		// read one or more line of ASCII data
        newline = 1;
        // Clear the sensor array before reading new data
        for (i=0;i<nSensorElements;i++){
            local_sensorValue[i]=noData;
        }
        while ((n == 0) && (newline == 1)){
            n = read_ascii_line(&buf, &len, fd_data_file);
		    if (n == 0) 
                newline = parseData(buf, &timestamp_data, local_sensorValue);
        } 
        
		if (n == 0) {
			// No data in this line
			if(newline < 0) continue;
			
			// print sensor values
            // TODO: Use the size field of the sensor array to decode the data
			if (debug >= 3) {
				printf("%4d: Received %4d bytes --- ", iLoop, (int) strlen(buf));
				printf("%lds %6ldus --- ", timestamp_data.tv_sec, (long) timestamp_data.tv_usec);
				if (profile_length != 0) {
					for (j = 0; j < nSensors; j++) {
						for (k = 0; k < profile_length; k++) {
							printf("%10.3f ", local_sensorValue[j * profile_length + k]);
						}
					}
				} else {
                    pSensor = local_sensorValue;
					for (j = 0; j < nSensors; j++) {
						printf("%f ", *pSensor);
                        pSensor += sensor[j].size;                   
					}
				}
				printf("\n");
			}
#ifdef USE_MYSQL
			if (db > 0) {
              if (timestamp_data.tv_sec > 0){

				// Write dataset to database
				// Store in the order of appearance	
				//printf("Write record to database\n");
 				
				sql = "INSERT INTO `";
				if (useTicks)
					sql += dataTableName + "` (`usec`";
				else
					sql += dataTableName + "` (`sec`";

                pSensor = local_sensorValue;
				for (i = 0 ; i < nSensors; i++) {
					if (*pSensor != noData) {
						sql += ",`";
						sql += sensor[i].name;
						sql += "`";
					}
                    pSensor += sensor[i].size;
				}
				sql += ") VALUES (";
				if (useTicks)
					sprintf(sData, "%ld", timestamp_data.tv_sec * 1000000 + timestamp_data.tv_usec);
				else
					sprintf(sData, "%ld", timestamp_data.tv_sec);
				sql += sData;
                
                // TODO: Save the data according to the size defined in sensors list
                //       What is the difference to the BLOBs stored in binaryDevice ???
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
                    pSensor = local_sensorValue;
					for (i = 0; i < nSensors; i++) {
						if (*pSensor != noData) {
                            if (sensor[i].size == 1){
                                sql += ",";
                                sprintf(sData, "%f", *pSensor);
                                sql += sData;
                            } else if (sensor[i].size > 1) {
                                    //sql += ", '";
                                    //for (k = 0; k < sensor[i].size; k++) {
                                    //    sprintf(sData, "%f, ", *pSensor++);
                                    //    sql += sData;
                                    //}
                                    //sql += "'"; 
                                    esc_str = new char[2 * sensor[i].size * sizeof(double) + 1];
                                    sql += ", compress('";
                                    mysql_real_escape_string(db, esc_str, (const char*) pSensor, sensor[i].size * sizeof(double));
                                    sql += esc_str;
                                    sql += "')";
                                    delete [] esc_str;
                            }
						}
                        pSensor += sensor[i].size;                        
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
					break;
				}
				
				gettimeofday(&t1, &tz);
				if (debug >= 5)
					printf("DB insert duration: %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec));
              }
            } else {
				printf("Error: No database availabe\n");
				throw std::invalid_argument("No database");
			}
#endif // of USE_MYSQL
			// Get the position in this file
			currPos = ftell(fd_data_file);
		}
		iLoop++;
	}
	
#ifdef USE_MYSQL
	sql = "UNLOCK TABLES";
	mysql_query(db, sql.c_str());
#endif
	
	if (n == -1) {
		fd_eof = true;
	} else {
		fd_eof = false;
	}
	
	processedData += currPos - lastPos;
	
	if (debug > 1)
		printf("Position in file: %ld; processed data: %ld Bytes\n", currPos, currPos - lastPos);

	
	// Write the last valid time stamp / file position
    if (timestamp_data.tv_sec > 0){
	   saveFilePosition(lastIndex, currPos, timestamp_data);
	}
	
	fclose(fd_data_file);
	free(buf);
	delete[] local_sensorValue;
    
}
