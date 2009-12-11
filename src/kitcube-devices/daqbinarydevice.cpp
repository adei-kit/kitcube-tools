/***************************************************************************
                          daqbinarydevice.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/



#include "daqbinarydevice.h"

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <fstream>

#include <akutil/akinifile.h>





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
	char dataName[256];
	int fd;
	std::string msg;
	std::string nameTemplate;
	std::string fullFilename;
	
	//printf("_____DAQBinaryDevice::openFile__________\n");	
	
	// Check if template file is existing and contains header + data
	nameTemplate = configDir + datafileTemplate;
	fd = open(nameTemplate.c_str(), O_RDWR);	// FIXME: file gets not closed again!
	if (fd <= 0) {
		msg = "Template file not found -- " + nameTemplate;
		throw std::invalid_argument(msg.c_str());
	}
	
	// Read header, test if it exists and is valid?!
	readHeader(nameTemplate.c_str());
	
	
	// Read index from file, if the value has not been initialized before
	// The markers for the different modules are independant
	if (fileIndex == 0){
		fileIndex = 1; // Default value
		sprintf(line, "%s.kitcube-data.marker.%03d.%d", dataDir.c_str(), moduleNumber, sensorGroupNumber);
		filenameMarker = line;
		fmark = fopen(filenameMarker.c_str(), "r");
		if (fmark > 0) {
			fscanf(fmark, "%ld", &fileIndex);
			fclose(fmark);
		}
	}
	
	
	// Open binary file for writing the data
	path = dataDir +  getDataDir();
	filename = getDataFilename(); // Warning using global variable for returning data !!!
	fullFilename = path + filename;
	
	printf("KITCube-Device (type %s): Open datafile %s\n", moduleType.c_str(), fullFilename.c_str());
	createDirectories(fullFilename.c_str());
	
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
	int i;
	std::string filename;
	
	if (fd_data <=0) return;

	// Read header to get the reference time of this file !!!
	//
	
	if (debug > 2) printf("_____DAQBinaryDevice::writeData()______________\n");
	
	//
	// Use one sample file to generate continuously data - by repetition of the sample data
	//
	
	// Allocate memory for one data set
	len = this->lenDataSet;    // (4 + nSensors * sizeof(float) + 8);
	lenHeader = this->lenHeader; // 0x10000; // Length of the header data
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
		if (n< len) {
			printf("Error: No data set found in template\n");
		}
	}
	close(fd_templ);
	
	// Compile the data set for writing to the data file
	if (debug > 1) printf("Received %4d bytes\n", n);
	
	// Replace time stamp
	updateDataSet(buf);
	
	n = write(fd_data, buf, len);
	//printf("Write %d bytes\n", n);
	
	delete buf;
	
	// Increment the counters
	nSamples++;
	nTemplate++;
	
}
