//
// C++ Implementation: campbelltob1
//
// Description: 
//
//
// Author: Andreas Kopmann <andreas.kopmann@kit.edu>, (C) 2014
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "campbelltob1.h"

#include <stdio.h>
#include <string.h>

#include <string>


CampbellTOB1::CampbellTOB1(){

	// Standard definitions - the coresponding inifile parameters should not be used 
	this->dataSep = ",";
    this->headerLine = 2;

    this->lenHeader = 0;
    this->lenDataSet = 0;
    
}


CampbellTOB1::~CampbellTOB1(){
}


int CampbellTOB1::readHeader(const char * filename){
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
    
    // Save the header length and calculate the length of a data set
    // lenHeader = ?
    // lenDataSet = ?
    
	if (debug > 3) printf("Header: %s", line_of_data);

    
    
	// close data file
	fclose(data_file_ptr);

    
    
    // Parse the header information
    int j;
	char *pCh;
	char *pChRes;
	char buffer[256];
    std::string headerParam[8];

    
	pCh = strtok(line_of_data, dataSep.c_str());
    headerParam[0] = pCh;
    
    i = 1;
    while ((pCh = strtok(NULL, dataSep.c_str())) && (i<8)){
        // TODO: strip quotes
        headerParam[i] = pCh;
        i++;
    }

    // TODO: Check if first tag is TOB1 and all fields are available
    if (headerParam[0] != "\"TOB3\""){
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
    
    // TODO: Get the sensor list
	// -> Copy from DAQAsciiDevice
    
    
    // TODO: Calculate length of the records
    // Available tpyes are
    // IEEE4, IEEE8,        = 4, 8 bytes
    // FP2,                 = 2 bytes
    // a single sign bit, a two-bit negative decimal exponent, and  a 13-bit mantissa
    // ULONG, LONG,         = 4 bytes
    // SecNano,             = ???
    // BOOL - 0x00, 0xff    = 1 byte
    // ASCII(len) - string  = len bytes
    
    // Required are at least: ULONG, IEEE4
    // If not all values are know the data can not be decoded !!!
    
    
    
    // TODO: Save the length of the header
    // lenHeader = <end of header block>
    
    
	return err;

}



int CampbellTOB1::parseData(char *line, struct timeval *l_tData, double *sensorValue) {
	float *local_sensorValue;
	struct SPackedTime *time;
	struct tm tm_zeit;
	
	
	if(debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	if (sizeof(float) != 4) {
		printf("Size of 'float' is not 4! So not reading any data!\n");
		return -1;
	}
	
	//----------------------------------------------------------------------
	// Data format:
	// ============
	// (u_)int32_t Tickcount: Hundertstel seit Messbeginn = Referenz-Zeitstempel,
	//                        s. readHeader(...) (wird nicht benutzt)
	// float Sensorwerte:     Messdaten, Anzahl steht im Header
	// struct SPackedTime:    Zeitstempel-Struktur
	//----------------------------------------------------------------------
	
	local_sensorValue =  (float *)(line + 4);	// TODO/FIXME: that is dangerous, as you don't know the size of "float"
	time = (struct SPackedTime *)(line + 4 + 4 * nSensors);	// TODO/FIXME: that's dangerous, as the order of the struct components is NOT fixed
	
	
	// read data and check for NaN
	for (int i = 0; i < nSensors; i++) {
		if (local_sensorValue[i] == local_sensorValue[i])	// is number
			sensorValue[i] = local_sensorValue[i];
		else	// is NaN
			sensorValue[i] = noData;
	}
	
	
	// read timestamp
	// TODO/FIXME: that's dangerous, as the order of the struct components is NOT fixed
	tm_zeit.tm_mday = time->nTag;
	tm_zeit.tm_mon = time->nMonat - 1;
	tm_zeit.tm_year = time->nJahr - 1900;
	tm_zeit.tm_hour = time->nStunde;
	tm_zeit.tm_min = time->nMinute;
	tm_zeit.tm_sec = time->nSekunde;
	
	// Calculate the time stamp
	l_tData->tv_sec = timegm(&tm_zeit);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
	l_tData->tv_usec = time->nHundertstel * 10000;
	
	return 0;
}




