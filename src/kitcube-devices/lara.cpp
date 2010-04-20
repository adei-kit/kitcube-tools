/***************************************************************************
                          lara.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "lara.h"


Lara::Lara(){
	
	this->moduleType = "Lara";
	this->moduleNumber = 900;
	this->sensorGroup = "stab";
	
	this->iniGroup = "Lara"; 
	
	// Read index from file
	fileIndex = 0;
	nTemplate = 0;
	
	// Data format parameters
	nSamplesInFile = 80;
	
}


Lara::~Lara(){	
}

void Lara::setConfigDefaults(){
	char line[256];
	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation 
	// of the class but first time after reading from the inifile
	// 
	
	this->moduleName = "LARA";
	this->moduleComment = "Lara";
	
	sprintf(line, "Lara.%s.template", sensorGroup.c_str());
	this->datafileTemplate = line;
	sprintf(line, "<index>_*.stab");
	this->datafileMask = line;
	//printf("datafileMask = %s\n", datafileMask.c_str());
	
	sprintf(line, "Lara.%s.sensors", sensorGroup.c_str());
	this->sensorListfile = line;
	
}



const char *Lara::getDataDir(){
		char line[256];
	
		// TODO: Create a single source for the filename convention...
	    sprintf(line, "Lara/");
		buffer = line;
		return(buffer.c_str());
}


const char *Lara::getDataFilename(){
	struct timeval t;
	struct timezone tz;	
	struct tm *date;
	char line[256];
	
	if (debug > 3) printf("fileIndex = %ld, nLine = %d\n", fileIndex, nLine);
	if (nLine == 0) {
		// Start new file		
		// Get the actual day
		gettimeofday(&t, &tz);
		date = gmtime((const time_t *) &t.tv_sec);
		fileIndex = (unsigned long) t.tv_sec; // Store file index
	
	} else {
		// Re-open the last file and continue
		date = gmtime( (const time_t *) &fileIndex );
	}
	if (debug > 3) printf("fileIndex = %ld, nLine = %d\n", fileIndex, nLine);

	// TODO: Create a single source for the filename convention...
	sprintf(line, "%02d%02d%02d-%02d%02d_(LARA1_T2_Run2_200_mbar_flow)_5W_100u_75deg_%ds.%s", 
			date->tm_year-100, date->tm_mon+1, date->tm_mday, date->tm_hour, date->tm_min,
			nSamplesInFile, sensorGroup.c_str());
	buffer = line;
	
	//printf("Lara: Get Datafilename = %s\n", line);
	return(buffer.c_str());
}


int Lara::getFileNumber(char *filename){
	std::string name;
	int posIndex;
	int index;
	std::string filePrefix;
	std::string fileSuffix;
	std::string numString;
	std::string fileString;
	struct tm time;
	//int lenIndex;
	//char *pEnd;
	int err;
	
		
	// Get date from filename
	fileString = filename;
	posIndex = 0;
	errno = 0;
	
	numString = fileString.substr(posIndex, 2);
	err = sscanf(numString.c_str(), "%d", &time.tm_year);
	time.tm_year = time.tm_year + 100;

	if (debug > 5) {
		printf("### Year: %s %d (err = %d) %s\n", 
			   numString.c_str(), time.tm_year, err, filename);
	}

	if (err < 1) throw std::invalid_argument("No valid data filename (year)");
	
	numString = fileString.substr(posIndex+2, 2); 
	err = sscanf(numString.c_str(), "%d", &time.tm_mon); 
	time.tm_mon = time.tm_mon - 1;
	if (err < 1) throw std::invalid_argument("No valid data filename (month)");
	
	numString = fileString.substr(posIndex+4, 2); 
	err = sscanf(numString.c_str(), "%d", &time.tm_mday); 
	if (err < 1) throw std::invalid_argument("No valid data filename (day)");
			
	numString = fileString.substr(posIndex+7, 2);  
	err = sscanf(numString.c_str(), "%d", &time.tm_hour); 
	if (err < 1) throw std::invalid_argument("No valid data filename (hour)");
		
	numString = fileString.substr(posIndex+9, 2); 
	err = sscanf(numString.c_str(), "%d", &time.tm_min); 
	if (err < 1) throw std::invalid_argument("No valid data filename (min)");
		
	time.tm_sec = 0; 
	time.tm_isdst = 0; // ???
	time.tm_gmtoff = 0; // ???
		
	// Convert to unix time stamp
    index = timegm(&time);
	if (debug>3) printf("Using the reference time as index. Index %d   Time %s", 
						index, asctime(&time));
	
    return (index);		
}


unsigned int Lara::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	if (sensorGroup == "stab") {
		number = 1; 
		buffer = "standard";
	}
	
	if (sensorGroup == "run") {
		number = 2; 
		buffer = "run parameter";
	}

	if (sensorGroup == "proc") {
		number = 3; 
		buffer = "results file";
	}

	if (sensorGroup == "SPE") {
		number = 4; 
		buffer = "2D spectrum";
	}
	
	
	return( number);
}


/*
void Lara::closeFile(){
	
	// Closing file...
	DAQDevice::closeFile();	

	// Check if the file is complete 
	// Incomplete file should be removed after beeing closed
	// --> Needs also some work in the reader. 
	//     There might be data processed that will disappear later 
	//     DON'T use this option !
	
	if (nLine < nSamplesInFile){
		char line[256];
		
		printf("Note: Will remove incomplete file\n");
		sprintf(line, "rm \"%s\"", fullFilename.c_str());
		printf("%s\n", line);
		system(line);
	}
	
}
*/


void Lara::readHeader(char *filename){
	std::string dir; 
	std::string fileString;
	std::string numString;
	//struct tm time;
	int posIndex;
	//int posFilename;
	int i;
	char *pEnd;
	time_t timeFromFilename;
	//char line[256];
	//struct timeval lastTime;
	//unsigned long lastPos;
	//unsigned long lastIndex;
	char *pFilename;
	
	errno = 0;
	
	if (debug > 0) printf("______Reading header_____________________\n");	
	
	//
	// There is no header for this format
    //
	
	// Read reference time
	// Get date from filename	
	try {
		pFilename = strrchr(filename, '/');
		if (pFilename == 0) pFilename = (char *) filename;
		else pFilename = pFilename +1;
		fileString = pFilename;	
		
		timeFromFilename = getFileNumber(pFilename);
		
	} catch (std::invalid_argument) {
		printf("Note: The file %s doesn't follow the Lara naming scheme\n", pFilename);
	}
	
	//if (tRef.tv_sec + nSamplesInFile > timeFromFilename) {
	//	tRef.tv_sec = tRef.tv_sec + nSamplesInFile;
	//	if (debug>2) printf("Note: Reference time adjusted to aviod overlapping time stamps\n");
	//} else {
		tRef.tv_sec = timeFromFilename;
		tRef.tv_usec = 0;
	//}

	
	// Define sensors if not available 
	
	if (sensorGroup == "stab") {
		
		if (nSensors == 0){			
			// Number of sensors
			// TODO: Move to the end of the inifile reader?!
			nSensors = 8;
			printf("Number of sensors %d\n", nSensors);
			
			// List of sensors
			if (sensor > 0 ) delete [] sensor;
			sensor = new struct sensorType [nSensors];
			
			sensor[0].comment = "Time offset";
			sensor[1].comment = "Photo diode signal";
			sensor[2].comment = "Laser power";
			sensor[3].comment = "Laser position x";
			sensor[4].comment = "Laser position y";
			sensor[5].comment = "Temperature 1"; // PT1000 
			sensor[6].comment = "Temperature 2"; // PT1000
			sensor[7].comment = "Temperature 3"; // PT1000
			
			for (i=0;i<nSensors;i++){
				printf("Sensor %3d: %s\n", i+1, sensor[i].comment.c_str());
			}
			
		}	
		
	}
	
	if (sensorGroup == "run"){
		
		//
		//  Read parameter from command line and store in a separate table
		//
		if (nSensors == 0) {
			
			// Create sensor definition - only once
			// TODO: Move to the end of the inifile reader?!
			nSensors = 5;
			if (sensorValue > 0 ) delete [] sensor;
			sensorValue = new float [nSensors];
			
			printf("Number of sensors %d\n", nSensors);
			
			// List of sensors
			if (sensor > 0 ) delete [] sensor;
			sensor = new struct sensorType [nSensors];
			
			sensor[0].comment = "Run number";
			sensor[1].comment = "Laser power";
			sensor[2].comment = "Slit width";
			sensor[3].comment = "CCD Temperature";
			sensor[4].comment = "N Samples";
			
			for (i=0;i<nSensors;i++){
				printf("Sensor %3d: %s\n", i+1, sensor[i].comment.c_str());
			}	
		}
		
		// Time stamp
		tData.tv_sec = tRef.tv_sec;
		tData.tv_usec = tRef.tv_usec;
		
		
		// Read run parameters from the filename
		for (i=0;i<nSensors;i++){
			sensorValue[i] = 0;
		}
		
		posIndex = fileString.find("Run");
		if (posIndex >= 0) {
			numString = fileString.substr(posIndex+3);
			sensorValue[0] = strtol(numString.c_str(), &pEnd, 0); 
		}
		
		posIndex = numString.find(")_");
		if (posIndex >= 0) {
			numString = numString.substr(posIndex+2);
			sensorValue[1] = strtol(numString.c_str(), &pEnd, 0); 
		}
		
		for (i=2;i<nSensors;i++){
			posIndex = numString.find("_");
			if (posIndex >= 0) {
				numString = numString.substr(posIndex+1);
				sensorValue[i] = strtol(numString.c_str(), &pEnd, 0); 
			}
		}
		
	}
	
}


void Lara::parseData(char *line, struct timeval *tData, float *sensorValue){
    unsigned long tNew;
	unsigned long nShift;
	
	// read nSensor values
	// TODO: Implement for variable number nSensor 
	sscanf(line, "%f%f%f%f%f%f%f%f",
		   &sensorValue[0], &sensorValue[1], &sensorValue[2], &sensorValue[3],
		   &sensorValue[4], &sensorValue[5], &sensorValue[6], &sensorValue[7]);
	
	// Check reference time - shift in order to aviod over lap
	if (debug > 3) printf("Last time %ld -- new time %ld\n", tData->tv_sec, tRef.tv_sec + (int) sensorValue[0]);
    tNew = tRef.tv_sec + (int) sensorValue[0];
	if (tNew <= tData->tv_sec){
		nShift = tData->tv_sec - tNew +1;
		tRef.tv_sec = tRef.tv_sec + nShift;

		if (debug > 1) printf("Note: Shifting reference time by %ldsec in order to avoid overlaping time stamps\n", nShift);
	}
	
	// Update time stamp
	tData->tv_sec = tRef.tv_sec + (int) sensorValue[0];
	tData->tv_usec = 0;
	
	//printf("Time stamp: %ld  %ld  %f--- ", tRef.tv_sec, tData->tv_sec, sensorValue[0]);
	
}



void Lara::readData(const char *dir, char *filename){

	if (sensorGroup == "stab") {
		DAQAsciiDevice::readData(dir, filename);
	}	
	
	if (sensorGroup == "run"){
		char line[256];
		std::string filenameMarker;
		FILE *fmark;
		unsigned long lastIndex;
		struct timeval lastTime;
		int handled;
		
		// It is needed to read the header for every file
		// As the run parameters need to be extracted from the file name
		// No need to open in this case !!!
		
		// Check if there is a new file 
		// Get the last time stamp + file pointer from 		
		sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dir, moduleNumber, sensorGroupNumber);
		filenameMarker =line;
		if (debug > 1) printf("Get marker from %s\n", filenameMarker.c_str());
		fmark = fopen(filenameMarker.c_str(), "r");
		if (fmark > 0) {
			fscanf(fmark, "%ld %ld %ld %d", &lastIndex, &lastTime.tv_sec, &lastTime.tv_usec, &handled);
			fclose(fmark);
		}
		
		// Handle every file only once 
		if ((handled == 0) && (getFileNumber((char *) filename) >= lastIndex)){			
			readHeader(filename);		
			DAQDevice::storeSensorData();	
		
			// Write back the marker 
			// It is necessary to write back a marker (in lastpos) that this file has been handled
			fmark = fopen(filenameMarker.c_str(), "w");
			if (fmark > 0) {
				fprintf(fmark, "%ld %ld %ld %d\n", lastIndex, lastTime.tv_sec, lastTime.tv_usec, 1);
				fclose(fmark);
			}			

			// The file / entry is always processed completely 
			fd_eof = true; 			
			
		} else {
			printf("File %s has been already processed (lastIndex = %ld)\n", filename, lastIndex);
		}		
		
	}
	
	
	if (sensorGroup == "proc") {
		// Store the profile data here
		
		// Not handled 
		
	}	

	if (sensorGroup == "SPE") {
		// Not handled
	}		
	
}


void Lara::writeData(){	
	char line[256];
	char sData[256];
	char *lPtr;
	char *dPtr;
	
	FILE *ftempl;
	int len;
	int lenHeader;
	//int n;
	//int i;
	std::string filename;
	
	if (fdata <=0) return;
	
	// Close file if the length of the file is reached
	// Open the next one - is there any wait time?
	
	
	// Read header to get the reference time of this file !!!
	// 
	
	if (debug > 2) printf("_____Lara::writeData()______________\n");
	
	//
	// Use one sample file to generate continuously data - by repetition of the sample data
	//
	
	// Allocate memory for one data set
	len = this->lenDataSet;    // (4 + nSensors * sizeof(float) + 8);
	lenHeader = this->lenHeader; // 0x10000; // Length of the header data
	//buf = new unsigned char  [len];
	
	// Open template file 
	filename = configDir + datafileTemplate;
	//fd_templ = open(filename.c_str(), O_RDONLY);
	ftempl = fopen(filename.c_str(), "r");
	
	
	fseek(ftempl, nTemplate, SEEK_SET); // Go to the required data set
	//lseek(fd_templ, lenHeader + len * nTemplate, SEEK_SET); // Go to the required data set
	
	// Read data set 
	//n = read(fd_templ, buf, len);
    lPtr = fgets(line, 255, ftempl);	
	if (lPtr == NULL) {
		// Reset template counter and try again
		nTemplate = 0; 
		fseek(ftempl, nTemplate, SEEK_SET);
		//lseek(fd_templ, lenHeader + len * nTemplate, SEEK_SET); // Go to the required data set
		//n = read(fd_templ, buf, len);
		lPtr = fgets(line, 255, ftempl);	
		if (lPtr == NULL) {
			printf("Error: No data set found in template\n");
		}
	}
	// Store the last file position in the template
	nTemplate = ftell(ftempl);
	
	//close(fd_templ);
	fclose(ftempl);
	
		
	// Exchange the index of the line
	// Every nSamplesInFile (default = 250) lines a new file needs to be created
	// After the run there is an analysis break (8sec)
	if (nLine >= nSamplesInFile) {
		// Recreate a new file after a pause 
		if (nLine > nSamplesInFile + 8) {
			nLine = 0; // Needs to be before opening the new file!
			openNewFile();
		}
	} 
	
	if (nLine < nSamplesInFile) {
		// Compile the data set for writing to the data file
		if (debug > 1) printf("Writing line %d\n", nLine+1);
		
		// Write data
		sprintf(sData, "%.6f", (float) nLine+1);
		dPtr = sData + strlen(sData);
		lPtr = strstr(line, "\t");
		strcpy(dPtr, lPtr);

		fprintf(fdata, "%s", sData);
		fflush(fdata);
	}
	
	
	// Increment the counters
	nLine++;
	//nSamples++;
	//nTemplate++;
		
}






