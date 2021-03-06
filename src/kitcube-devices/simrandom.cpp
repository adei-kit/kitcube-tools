/***************************************************************************
                          simrandom.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "simrandom.h"


SimRandom::SimRandom(){
		
	moduleType = "Simulation";
	moduleNumber = 0;
	this->sensorGroup = "txt";
	
	this->iniGroup = "Simulation"; 

	
	// Read index from file
	fileIndex = 0;
	
	lenHeader = 0x39;	// CAUTION: header has several lines!
	
	lenDataSet = 64;	// 58 plus kleiner Puffer, hier OK, da fgets nach "\n" stoppt
	
}


SimRandom::~SimRandom(){
}


void SimRandom::setConfigDefaults(){
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
	
	sprintf(line, "SimRandom.%s.sensors", sensorGroup.c_str());
	this->sensorListfile = line;
	
}


const char *SimRandom::getDataFilename(){
	char tmp_line[16];
	int pos_index;
	
	
	// print file index to buffer
	sprintf(tmp_line, "%ld", fileIndex);
	
	// replace <index> in datafile mask with file index
	buffer = datafileMask;
	pos_index = buffer.find("<index>");
	buffer.replace(pos_index, 7, tmp_line);
	
	return buffer.c_str();
}


const char *SimRandom::getDataDir(){
		char line[256];
	
		sprintf(line, "SimRandom/");
		buffer = line;
		return(buffer.c_str());
}



int SimRandom::readHeader(const char *){
	// Not used 
	
	
	return(0); // Ok
}



void SimRandom::writeHeader(){
	struct timeval t;
	struct timezone tz;

	if (fdata <=0) return;
    
	gettimeofday(&t, &tz);
	
	fprintf(fdata, "; Header");
	fprintf(fdata, "; Started at %12ld %06ld\n", t.tv_sec, (long) t.tv_usec);
	fprintf(fdata, "; Height:  4m\n"); 
	fprintf(fdata, ";\n");

	
	// Set seed for random generator
	srand(t.tv_sec);	
}


int SimRandom::parseData(char *line, struct timeval *l_tData, double *sensorValue){
    //unsigned long tNew;
	//unsigned long nShift;
	
	// read nSensor values
	// TODO: Implement for variable number nSensor 
	sscanf(line, "%12ld %06ld %12lf %12lf %12lf", &l_tData->tv_sec, (long *) &l_tData->tv_usec,
		   &sensorValue[0], &sensorValue[1], &sensorValue[2]);
		
	//printf("Time stamp: %ld  %ld  %f--- ", tRef.tv_sec, tData->tv_sec, sensorValue[0]);
	return 0;
}


void SimRandom::writeData(){
	struct timeval t;
	struct timezone tz;	
    float data1, data2, data3;
	
	
	if (fdata <=0) return;

	gettimeofday(&t, &tz);
	
	data1 = ( t.tv_sec)  % 10000 + (float) t.tv_usec / 1000000; // ramp data
	data2 = sin( (2 * 3.14 *  (double)  t.tv_sec  + (double) t.tv_usec / 1000000) / 100) ; // sinus with 100s period
	data3 = (float) rand()/RAND_MAX * 10; // random data 0..10
	fprintf(fdata, "%12ld %06ld %12f %12f %12f\n", t.tv_sec, (long) t.tv_usec, data1, data2, data3);
	fflush(fdata);
	if (debug > 1) printf("#%03d %12ld %06ld %12f %12f %12f\n", moduleNumber, t.tv_sec, (long) t.tv_usec, data1, data2, data3);
	
}

