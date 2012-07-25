/********************************************************************
* Description: Soil moisture probes
* Author: Andreas Kopmann, IPE
* Created at: Mon Mar 19 15:04:08 CET 2012
*    
* Copyright (c) 2012 Andreas Kopmann, IPE  All rights reserved.
*
********************************************************************/


#include "sisomop.h"

#include <string>
#include <algorithm>

using namespace std;
using std::replace;


sisomop::sisomop() {
}


sisomop::~sisomop() {
}


unsigned int sisomop::getSensorGroup() {
	unsigned int number;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "txt") {
		number = 5;
		buffer = "Soil moisture";
	}
	
	return number;
}


int sisomop::readHeader(const char *filename) {
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	noData = 999999;
	
	lenHeader = 0;	// 2764 bytes
	
	lenDataSet = 0;	// 109 bytes + 1 for '\0' in fgets()
	
	profile_length = 0;
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
		sensor[i].data_format = "<scalar>";
        sensor[i].size = 1; 
	}

	return 0;
}


void sisomop::setConfigDefaults() {
}

const char *sisomop::getDataDir() {
	char line[256];
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "%s/SISOMOP/", moduleName.c_str());
	buffer = line;
	
	return(buffer.c_str());
}


int sisomop::parseData(char *line, struct timeval *l_tData, double *sensorValue) {
    int i, n;
    std::string buf;
    std::string sValue;
    char *res;   
	struct tm timestamp;
    int id;
    double value;
	size_t pos0, pos1;
	
	if (debug >= 5)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
    // Read three lines to fill the complete data set
    // Ist the data always in the same order ???
    
    buf = line;
    if (debug > 5) printf("%s ... (n=%ld) \n", buf.substr(0,60).c_str(), strlen(line));

	
	// read year and day of year
    // Fromat: 01.03.2012   15:42   Sensor 1        5,250   28289   25,20
    timestamp.tm_sec = 0;
    timestamp.tm_gmtoff = 0;
    res = strptime(line, "%d.%m.%Y%n%H:%M", &timestamp);
    //printf("timestamp: %d %d %d %d %d %d \n", timestamp.tm_year, timestamp.tm_mon,
    //       timestamp.tm_mday,timestamp.tm_hour,timestamp.tm_min,timestamp.tm_sec);
    
    if (res == 0){
        return -1; // not a valid line
    }
    
    l_tData->tv_sec = timegm(&timestamp);
    //printf("DATE: %s %d(res=%d)\n", asctime(&timestamp), l_tData->tv_sec, res);
  

    i = 0; // Tag count
    pos0 = 0;
    pos1 = 0;
    pos1 = buf.find_first_of("\t\n", pos0);

    //buf = "20.03.2012\t20:23\tSensor 2\t\t123\n";
    while (pos1 != string::npos){
        sValue = buf.substr(pos0, pos1-pos0);
        if (debug > 6) printf("TAG %d: <<%s>>\n", i, sValue.c_str());

        // Read senor id / height
        if (i == 2) {
            n = sscanf(sValue.c_str(), "%*s %d", &id);
            //printf("Found sensor id = %d (err = %d) \n", id, n);
            if (n < 1) return -1; // No valid sensor id found
        }
        
        if (i > 2) {
            // Replace decimal point
            replace(sValue.begin(), sValue.end(), ',', '.');
            
            n = sscanf(sValue.c_str(), "%lf", &value); 
            if (n < 1) value = noData;
            sensorValue[3*(i-3)+(id-1)] = value;
 
            if (debug >5) printf("ID = %d  %s %f \n", id, sValue.c_str(), value);
        }
        
        // Read next tag, if available
        i++;
        pos0 = pos1+1;
        pos1 = buf.find_first_of("\t\n", pos0); // Linux?!
        //pos1 = buf.find_first_of("\t\r", pos0); // OSX ??
        
    }
    
 
    if (id <3) return 1; // Get next line
    else return 0; // Data set complete

}

