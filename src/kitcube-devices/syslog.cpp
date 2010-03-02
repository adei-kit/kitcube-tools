/***************************************************************************
                          syslogSysLog.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/



#include "syslog.h"

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



SysLog::SysLog():DAQDevice(){
	
	this->moduleType = "SysLog";
	this->moduleNumber = 99; // SysLog default number
	this->sensorGroup = "log";

	this->iniGroup = "SysLog"; 
	
	this->lenHeader = 0;
	this->lenDataSet = 0; 
    this->noData = 0;
		
	this->sensorValue = 0;
}


SysLog::~SysLog(){
}



void SysLog::setNData(int n){
	
	if (n>0) {
		nSensors = n;
		
		if (sensor > 0 ) delete [] sensor;
		sensor = new struct sensorType [nSensors];
		
		if (sensorValue > 0 ) delete [] sensorValue;
		sensorValue = new float[n];
	}
	
}


void SysLog::setConfig(int ch, const char *name){
	
	if (ch < nSensors){
		sensor[ch].comment = name;
		sensor[ch].height = 0;
	}	
}


void SysLog::updateTimestamp(struct timeval *t){
	
	tData.tv_sec = t->tv_sec;
	tData.tv_usec = t->tv_usec;
}


void SysLog::updateData(int ch, float data){
	
	if (ch< nSensors){
		sensorValue[ch] = data;
	}
}


void SysLog::setConfigDefaults(){
	char line[256];

	// Note:
	// The paramters are dependant of the module number that is not known at the creation 
	// of the class but first time after reading from the inifile
	// 
	
	this->moduleName = "System";
	this->moduleComment = "SysLog";
		
	sprintf(line, "SysLog.sensors");
	this->sensorListfile = line;
}



