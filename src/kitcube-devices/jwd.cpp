/***************************************************************************
                          simrandom.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/



#include "jwd.h"

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <fstream>

#include <akutil/akinifile.h>


jwd::jwd(){
	int i;
	
	moduleType = "JWD";
	moduleNumber = 0;
	this->sensorGroup = "txt";
	
	this->iniGroup = "JWD-dd";
	
	// Read index from file
	fileIndex = 0;
	
	lenHeader = 0xd4;	// CAUTION: header has several lines!
	
	// List of sensors
	nSensors = 8;
	sensor = new struct sensorType [nSensors];
	
	for (i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
	}
	
	sensor[0].comment = "Rain intensity";
	sensor[1].comment = "Spectral number density";
	sensor[2].comment = "Radar reflectivity";
	sensor[3].comment = "unknown";
	sensor[4].comment = "unknown";
	sensor[5].comment = "unknown";
	sensor[6].comment = "unknown";
	sensor[7].comment = "Accumulated rain amount";
	
	if (debug) {
		for (i = 0; i < nSensors; i++) {
			printf("Sensor %3d: %s\n", i+1, sensor[i].comment.c_str());
		}
	}
}


jwd::~jwd(){
}


void jwd::setConfigDefaults(){
	char line[256];
	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation 
	// of the class but first time after reading from the inifile
	// 
	
	this->moduleName = "Sim";
	this->moduleComment = "Simulation";
		
	this->datafileTemplate = ""; // No template file required

	sprintf(line, "sim_<index>.%s", sensorGroup.c_str());
	this->datafileMask = line;
	//printf("datafileMask = %s\n", datafileMask.c_str());
	
	sprintf(line, "jwd.%s.sensors", sensorGroup.c_str());
	this->sensorListfile = line;
	
}


/*
const char *jwd::getDataFilename(){
	char line[256];
	
	if (debug > 3) printf("fileIndex = %ld, nLine = %d\n", fileIndex, nLine);
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "sim_%ld.%s", fileIndex, sensorGroup.c_str());
	buffer = line;
	
	//printf("Lara: Get Datafilename = %s\n", line);
	return(buffer.c_str());
}
*/


const char *jwd::getDataDir(){
	char line[256];
	
	sprintf(line, "%s/", moduleName.c_str());
	buffer = line;
	return(buffer.c_str());
}



void jwd::readHeader(){
	// Not used
}



void jwd::writeHeader(){
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


void jwd::parseData(char *line, struct timeval *tData, float *sensorValue){
	int yday, mod;
	char date[7] = {0}, time[5] = {0};
	struct tm timestamp;
	
	// TODO/FIXME: are data timestamps in UTC or local time?
	
	// read date, time and sensor values of one line of data
	sscanf(line, "%s %i %s %i %f %f %f %f %f %f %f %f",
		date, &yday, time, &mod,
		&sensorValue[0], &sensorValue[1], &sensorValue[2], &sensorValue[3],
		&sensorValue[4], &sensorValue[5], &sensorValue[6], &sensorValue[7]);
	
	
	// calculate timestamp in seconds:
	
	// evaluate date...
	std::string year(date, 2);
	std::string month(date + 2, 2);
	std::string day(date + 4, 2);
	
	// ...and file tm structure
	timestamp.tm_mday = atoi(day.c_str());
	timestamp.tm_mon = atoi(month.c_str()) -1;
	timestamp.tm_year = atoi(year.c_str()) + 100;
	
	// evaluate time...
	std::string hour(time, 2);
	std::string min(time + 2, 2);
	std::string sec("0");
	
	// ...and fill tm structure
	timestamp.tm_hour = atoi(hour.c_str());
	timestamp.tm_min = atoi(min.c_str());
	timestamp.tm_sec = atoi(sec.c_str());
	
	// get seconds since the Epoch
	tData->tv_sec = timegm(&timestamp);
}


void jwd::writeData(){
	struct timeval t;
	struct timezone tz;
	float data1, data2, data3;
	
	
	if (fdata <=0) return;

	gettimeofday(&t, &tz);
	
	data1 = ( t.tv_sec)  % 10000 + (float) t.tv_usec / 1000000; // ramp data
	data2 = sin( (2 * 3,14 *  (double)  t.tv_sec  + (double) t.tv_usec / 1000000) / 100) ; // sinus with 100s period
	data3 = (float) rand()/RAND_MAX * 10; // random data 0..10
	fprintf(fdata, "%12ld %06d %12f %12f %12f\n", t.tv_sec, t.tv_usec, data1, data2, data3);
	fflush(fdata);
	if (debug > 1) printf("%12ld %06d %12f %12f %12f\n", t.tv_sec, t.tv_usec, data1, data2, data3);
	
}


void jwd::copyRemoteData(){
	struct timeval t0, t1;
	struct timezone tz;
	int err, pos;
	char line[256];
	std::string output, data_files_wildcard;
	
	
	if (debug > 2) printf("_____jwd::copyRemoteData()_____\n");
	
	createDirectories((archiveDir + getDataDir()).c_str());
	
	// Transfer only files of the specified sensor group (== file type)
	// The time to find changes in large file increases with the file size
	// Therefore it is necessary to use the rsync option --append. This option
	// will make rsync assume that exisitings files will only be added at the end.
	//
	// TODO: Improve output of the system call. The output is mixed with others
	//       Solution 1: Write output to file,
	//       Solution 2: Use pipes (example can be found in README
	//
	if (debug > 2)
		output = "";
	else
		output = " > /dev/null";
	
	data_files_wildcard = datafileMask;
	pos = data_files_wildcard.find("<index>");
	data_files_wildcard.replace(pos, 7, "*");
	
	sprintf(line, "rsync -avz %s --include='*/' --include='%s' --exclude='*' %s%s  %s%s %s",
			rsyncArgs.c_str(), data_files_wildcard.c_str(),
			remoteDir.c_str(), getDataDir(),
			archiveDir.c_str(), getDataDir(), output.c_str());
	if (debug > 2) printf("%s\n", line);
	
	gettimeofday(&t0, &tz);
	err = system(line);
	gettimeofday(&t1, &tz);
	
	if (err != 0) {
		printf("Synchronisation error (rsync)\n");
		//throw std::invalid_argument("Synchronisation error (rsync)");
	}
	
	if (debug > 2) printf("Rsync duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));
}
