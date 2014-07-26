//
// C++ Implementation: campbelltoa5
//
// Description: 
//
//
// Author: Andreas Kopmann <andreas.kopmann@kit.edu>, (C) 2014
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "campbelltoa5.h"

#include <stdio.h>
#include <string.h>

#include <string>


CampbellTOA5::CampbellTOA5(){

	// Standard definitions - the coresponding inifile parameters should not be used 
	this->dataSep = ",";
    this->headerLine = 2;
	this->lenHeaderLines = 4;
	this->timestampFormat = "%Y-%m-%d %H:%M:%S";

}

CampbellTOA5::~CampbellTOA5(){
}


int CampbellTOA5::readHeader(const char * filename){
	int err;

    // Read Campbell specific header information
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

    // Read sensor configuration
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return(-1);
    }
	if (debug > 3) printf("Header: %s", line_of_data);
	
    
    
    // Parse the header information
    int j;
	char *pCh;
	char *pChRes;
	char buffer[256];
    std::string headerParam[8];
    std::string *sensorNames;
    std::string *sensorUnits;
    
	pCh = strtok(line_of_data, dataSep.c_str());
    headerParam[0] = pCh;
    
    i = 1;
    while ((pCh = strtok(NULL, dataSep.c_str())) && (i<8)){
        // TODO: strip quotes
        headerParam[i] = pCh;
        i++;
    }
    
    // TODO: Check if first tag is TOA5 and all fields are available
    if (headerParam[0] != "\"TOA5\""){
        printf("\n");
        printf("Error: In file %s data format %s is not known\n", filename, headerParam[0].c_str());
        printf("\n");
        throw std::invalid_argument("Data format not supported by DAQdevice reader");
    }
    
    
    // TODO: Display header information
    // Device type
    // Comment
    // Data table name
    // Number of sensors
    // First time stamp?
    
    if (debug > 2){
        //printf("Format     : %s\n", headerParam[0].c_str());
        printf("Station    : %s (type %s, SN %s, OS %s)\n", headerParam[1].c_str(), headerParam[2].c_str(), headerParam[3].c_str(), headerParam[4].c_str());
        printf("DLD        : %s (ID %s)\n", headerParam[5].c_str(), headerParam[6].c_str());
        printf("Table name : %s\n", headerParam[7].c_str());
    }
    
    
    // Write a sample configuration file
    int nSensorsInHeader;
    
    // Read sensor configuration
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return(-1);
    }
	if (debug > 3) printf("Sensors: %s", line_of_data);
    char * lineOfSensors = new char [strlen(line_of_data)];
    strcpy(lineOfSensors, line_of_data);
    
    // Count the numer of sensors
    pCh = strtok(line_of_data, dataSep.c_str());
    
    i = 1;
    while ((pCh = strtok(NULL, dataSep.c_str()))){
        i++;
    }
    nSensorsInHeader = i;
    if (debug > 1) {
        printf("Sample configuration file: \n\n");
        printf("Number of sensors: %d\n", nSensorsInHeader-1);
    }
        
    // Allocate sensor list
    sensorNames = new std::string [nSensorsInHeader];
    sensorUnits = new std::string [nSensorsInHeader];
    
    
    // Read sensors names
    strcpy(line_of_data, lineOfSensors);
    pCh = strtok(line_of_data, dataSep.c_str());
    i = 0;
    while (i<nSensorsInHeader){
        if (pCh[0] == '"')
            sensorNames[i] = pCh+1;
        else
            sensorNames[i] = pCh;
        sensorNames[i].erase(sensorNames[i].find('"'));
        
        pCh = strtok(NULL, dataSep.c_str());
        i++;
    }
    

    // Read units
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return(-1);
    }
	if (debug > 3) printf("Units: %s", line_of_data);
    
    pCh = strtok(line_of_data, dataSep.c_str());
    
    i = 0;
    while (i<nSensorsInHeader){
        if (pCh[0] == '"')
            sensorUnits[i] = pCh+1;
        else
            sensorUnits[i] = pCh;
        sensorUnits[i].erase(sensorUnits[i].find('"'));

        pCh = strtok(NULL, dataSep.c_str());
        i++;
    }
    

    if (debug > 1){
        // Don't print the time stamp
        for (i=1;i<nSensorsInHeader;i++){
            printf("%d\t%s\t%s (%s)\t\n", i, sensorNames[i].c_str(),
                   sensorNames[i].c_str(), sensorUnits[i].c_str());
        }
        printf("\n\n");
    }
    

    
    // close data file
    delete [] lineOfSensors;
    delete [] sensorNames;
    delete [] sensorUnits;
    free(line_of_data);
	fclose(data_file_ptr);
    

    
    // Get the sensor list
	err = DAQAsciiDevice::readHeader(filename);
	return err;

}




