/***************************************************************************
                          simrandom.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/



#include "simrandom.h"

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <fstream>

#include <akutil/akinifile.h>


SimRandom::SimRandom(){
	FILE *fmark;
	
	moduleType = "Simulation";
	moduleNumber = 0;
	
	// Read index from file
	fileIndex = 0;
	
}


SimRandom::~SimRandom(){
	FILE *fmark;
	
	// Save current position
	if (fileIndex > 0) {
		fmark = fopen(filenameMarker.c_str(), "w");
		if (fmark > 0) {
			fprintf(fmark, "%ld\n", fileIndex);
			fclose(fmark);
		}
	}
}


void SimRandom::readInifile(const char *filename, const char *group){
  akInifile *ini;
  Inifile::result error;
  std::string value;
  std::string iniGroup;
  char line[256];

  this->inifile = filename;
  this->moduleNumber = 100; // Default value for simulation modules
  this->configDir = "./";
  this->dataDir = "./data";
  this->remoteDir = "../kitcube-data/data";
  this->archiveDir = "./data";	
	
  iniGroup = "SimRandom";	
	if (group > 0) iniGroup = group;
	
  //ini = new akInifile(inifile.c_str(), stdout);
  ini = new akInifile(inifile.c_str());
  if (ini->Status()==Inifile::kSUCCESS){
    error = ini->SpecifyGroup(iniGroup.c_str());
	  if (error == Inifile::kSUCCESS){ 
        this->moduleNumber = ini->GetFirstValue("moduleNumber", (int) moduleNumber, &error);
	  }
  

  sprintf(line, "Mast-<index>.kitcube-data");
  this->datafileMask = line;
	
	  
	// Read global parameters
	// The parameters should be the same for the whole system
	ini->SpecifyGroup("Common");
	this->configDir = ini->GetFirstString("configDir", configDir.c_str(), &error);
	this->dataDir = ini->GetFirstString("dataDir", dataDir.c_str(), &error);
	this->remoteDir = ini->GetFirstString("remoteDir", remoteDir.c_str(), &error);
	this->archiveDir = ini->GetFirstString("archiveDir", archiveDir.c_str(), &error);
	
	
	  
	ini->SpecifyGroup(iniGroup.c_str());
	this->configDir = ini->GetFirstString("configDir", configDir.c_str(), &error);
	this->dataDir =  ini->GetFirstString("dataDir", dataDir.c_str(), &error);
	this->remoteDir =  ini->GetFirstString("remoteDir", remoteDir.c_str(), &error);
	this->archiveDir = ini->GetFirstString("archiveDir", archiveDir.c_str(), &error);
	this->datafileMask = ini->GetFirstString("datafileMask", datafileMask.c_str(), &error);
	  
  }
	
  delete ini;


	// Add slash at the end of the directories
	//printf("ConfigDir: %c\n", configDir.at(configDir.length()-1));	
	if (configDir.at(configDir.length()-1) != '/') configDir += "/";	
	if (dataDir.at(dataDir.length()-1) != '/') dataDir += "/";	
	if (remoteDir.at(remoteDir.length()-1) != '/') remoteDir += "/";	
	if (archiveDir.at(remoteDir.length()-1) != '/') archiveDir += "/";	
	

}


void SimRandom::openFile(){ // for writing
	char line[256];
	FILE *fmark;
	
	if (fileIndex == 0) {
		fileIndex = 1; // Default value
		sprintf(line, "%s/.kitcube-data.marker.%03d", dataDir.c_str(), moduleNumber);
		filenameMarker = line;
		fmark = fopen(filenameMarker.c_str(), "r");
		if (fmark > 0) {
			fscanf(fmark, "%ld", &fileIndex);
			fclose(fmark);
		}
	}
	
	// Open binary file for writing the data
	sprintf(line, "%s/%03d/%s_%02d/Mast-%ld.kitcube-data", dataDir.c_str(), 
			moduleNumber, moduleType.c_str(), moduleNumber, fileIndex);
	this->filename = line;
	
	DAQDevice::openFile();
	
}


void SimRandom::copyRemoteData(){
	int err;
	char line[256];
	
	createDirectories(archiveDir.c_str());
	
	sprintf(line, "rsync -avz  %s/%03d  %s", remoteDir.c_str(), moduleNumber, archiveDir.c_str());
	printf("%s\n", line);
	err = system(line); 
    if (err != 0){
		printf("Synchronisation error (rsync)\n");
		//throw std::invalid_argument("Synchronisation error (rsync)");
	}
	
}



const char *SimRandom::getDataDir(){
		char line[256];
	
		// TODO: Create a single source for the filename convention...
		sprintf(line, "%s%03d/%s_%02d/", archiveDir.c_str(), moduleNumber, moduleType.c_str(), moduleNumber);
		buffer = line;
		return(buffer.c_str());
}



void SimRandom::readHeader(){

	// Not used 
	
	
}



void SimRandom::writeHeader(){
	struct timeval t;
	struct timezone tz;

	if (fdata <=0) return;
    
	gettimeofday(&t, &tz);
	
	fprintf(fdata, "; Header");
	fprintf(fdata, "; Started at %12ld %06d\n", t.tv_sec, t.tv_usec);
	fprintf(fdata, "; Height:  4m\n"); 
	fprintf(fdata, ";\n");

	
	// Set seed for random generator
	srand(t.tv_sec);	
}



void SimRandom::readData(const char *dir, const char *filename){
	FILE *fdata;
	FILE *fmark;
	std::string filenameMarker;
	std::string filenameData;
	struct timeval lastTime;
	unsigned long lastPos;
	unsigned long lastIndex;
	char line[256];
	
	
	// Open the data file 
	// Open binary file for writing the data
	filenameData = dir;
	filenameData += "/";
	filenameData += filename;
	if (debug > 1) printf("Open data file %s\n", filenameData.c_str());
    fdata = fopen(filenameData.c_str(), "r");
    if (fdata <= 0) {
		printf("Error opening data file %s\n", filenameData.c_str());
		return;
	}
	//printf("Position of file %d\n", ftell(fdata));
	
	
	// Get the last time stamp + file pointer from 
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	filenameMarker = dir;
	filenameMarker += "/";
    filenameMarker += ".kitcube-reader.marker";
	if (debug > 1) printf("Get marker from %s\n", filenameMarker.c_str());
    fmark = fopen(filenameMarker.c_str(), "r");
    if (fmark > 0) {
		fscanf(fmark, "%ld %ld %d %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
		fclose(fmark);
	}
   	
	
	// Find the beginning of the new data
	//printf("LastPos: %d\n", lastPos);
	fseek(fdata, lastPos, SEEK_SET);
	
	
	
	// Read until end of the file
	while (!feof(fdata)){
		lastPos = ftell(fdata);
		fgets(line, 255, fdata);
		
		//printf("Position of file %d\n", lastPos);	
		if (debug > 1) printf(".");
	}
	if (debug > 1) printf("\n");
	if (debug > 1) printf("Position of file %ld\n", lastPos);
	
	
	// Write the last valid time stamp / file position
    fmark = fopen(filenameMarker.c_str(), "w");
    if (fmark > 0) {
		fprintf(fmark, "%ld %ld %d %ld\n", lastIndex, lastTime.tv_sec, lastTime.tv_usec, lastPos);
		fclose(fmark);
	}
	
	
	fclose(fdata);
	
}



void SimRandom::writeData(){
	struct timeval t;
	struct timezone tz;	
    float data1, data2, data3;
	
	
	if (fdata <=0) return;

	gettimeofday(&t, &tz);
	
	data1 = ( t.tv_sec)  % 10000 + (float) t.tv_usec / 1000000; // ramp data
	data2 = sin( (2 * 3,14 *  (double)  t.tv_sec  + (double) t.tv_usec / 1000000) / 100) ; // sinus with 100s period
	data3 = (float) rand()/RAND_MAX * 10; // random data 0..10
	fprintf(fdata, "%12ld %06d %12f %12f %12f\n", t.tv_sec, t.tv_usec, data1, data2, data3);
	if (debug > 1) printf("%12ld %06d %12f %12f %12f\n", t.tv_sec, t.tv_usec, data1, data2, data3);
	
}

