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
	
	if (nSensors){
		delete [] sensorTypes;
		delete [] sensorBytes;
	}
	
}


int CampbellTOB1::readHeader(const char * filename){
	int err;

    // Read Campbell specific header information
    int i;
	FILE *data_file_ptr;
	char *line_of_data = NULL;
	size_t len = 0;
	int nBytes = 0;
	int frameSize = 0;  // a frame contains several records (only TOB2/3)
	std::string tUnit;
	
	// Reading header from a generic CSV file
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
    
	// open data file
	data_file_ptr = fopen(filename, "r");
	if (data_file_ptr == NULL) {
		printf("Error opening file %s\n", filename);
		return -1;
	}
	if (debug > 2) printf("Length of the header : %3d bytes (ftell: %ld)\n", nBytes, ftell(data_file_ptr));

    // Read sensor configuration
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return(-1);
    }
	nBytes += len;
	if (debug > 2) printf("Length of the header : %3d bytes (ftell: %ld)\n", nBytes, ftell(data_file_ptr));
	
    // Save the header length and calculate the length of a data set
    // lenHeader = ?
    // lenDataSet = ?
    
	if (debug > 3) printf("Header: %s", line_of_data);

    
    
	
    
    // Parse the header information
    //int j;
	char *pCh;
	//char *pChRes;
	//char buffer[256];
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

	tobno = 0;
	if (headerParam[0] == "\"TOB1\"") tobno = 1;
	if (headerParam[0] == "\"TOB2\"") tobno = 2;
	if (headerParam[0] == "\"TOB3\"") tobno = 3;
	
	
    // TODO: Check if first tag is TOB1 and all fields are available
    if (tobno == 0){
        printf("\n");
        printf("Error: In file %s data format %s is not known\n", filename, headerParam[0].c_str());
        printf("\n");
        throw std::invalid_argument("Data format not supported by DAQdevice reader");
    }
    
    
    // Display header information
    if (debug > 2){
        printf("Format     : Campbell binary format TOB%d\n", tobno);
        printf("Station    : %s (type %s, SN %s, OS %s)\n", headerParam[1].c_str(), headerParam[2].c_str(), headerParam[3].c_str(), headerParam[4].c_str());
        printf("DLD        : %s (ID %s)\n", headerParam[5].c_str(), headerParam[6].c_str());
		
		if (tobno == 1)
			printf("Table name : %s\n", headerParam[7].c_str()); // only TOB1
    }
	
	// Read second header line (only TOB2/3)
	if (tobno > 1){
	
		if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
			free(line_of_data);
			return(-1);
		}
		nBytes += len;
		if (debug > 2) printf("Length of the header : %3d bytes (ftell: %ld)\n", nBytes, ftell(data_file_ptr));

		if (debug > 3) printf("Second header line: %s", line_of_data);

		pCh = strtok(line_of_data, dataSep.c_str());
		headerParam[0] = pCh;
		
		i = 1;
		while ((pCh = strtok(NULL, dataSep.c_str())) && (i<8)){
			// Strip quotes
			if (pCh[0] == '"') pCh = pCh +1;
			if (pCh[strlen(pCh)-1] == '"') pCh[strlen(pCh)-1] = 0;

			headerParam[i] = pCh;
			i++;
		}
		
		// Read dataset size
		frameSize = atoi(headerParam[2].c_str());
		
		// Parse sampling time
		unsigned int tValue;
		int pos;
		pos = headerParam[1].find(' ');
		//printf("Separator at pos = %d\n", pos);
		tValue = 0;
		if (pos > 0) tValue = atoi(headerParam[1].substr(0,pos).c_str());
		
		tUnit = headerParam[1].substr(pos+1);
		if (debug > 3) printf("Decoded %s to %d / %s\n",
							  headerParam[1].c_str(), tValue, tUnit.c_str() );
		
		if (tUnit == "MSEC") {
			recordTimeStep.tv_sec = 0;
			recordTimeStep.tv_usec = tValue * 1000;
		}
		else if (tUnit == "SEC") {
			recordTimeStep.tv_sec = tValue;
			recordTimeStep.tv_usec = 0;
		}
		else if (tUnit == "MIN") {
			recordTimeStep.tv_sec = tValue * 60;
			recordTimeStep.tv_usec = 0;
		}
		else
			printf("Warning: Record time step %s unkown\n", headerParam[1].c_str());

		
		// Parse subsecond resulution
		tUnit = headerParam[5].c_str();
		if (tUnit == "Sec100Usec") subSecCounterResolution = 10000;
		else {
			printf("Warning: Please specify SubSecCountResolution is kitcube.ini as %s\n", headerParam[5].c_str());
		}
		
		// TODO: Parse also other parameters

		if (debug > 2){
			printf("Table name       : %s\n", headerParam[0].c_str());
			printf("Dataset size     : %s (%d Bytes)\n", headerParam[2].c_str(), frameSize);
			printf("Record time step : %s (%d sec / %d usec)\n",
				headerParam[1].c_str(), (int) recordTimeStep.tv_sec, (int) recordTimeStep.tv_usec);
			printf("SubSecCounterRes : %s (1/%d sec)\n",
				headerParam[5].c_str(), subSecCounterResolution);
		}
		
	}
	
	//
	// Generate the sample *.sensors file that can be copied to a sensors file?!
	// The used configration is always read from a *.sensors file
	//
	
	// Count the number of sensors defined in the header
	int nSensorsInHeader;
		
	// Read sensor configuration
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
			free(line_of_data);
			return(-1);
	}
	nBytes += len;
	
	if (debug > 3) printf("Sensors: %s", line_of_data);
	char * lineOfSensors = new char [strlen(line_of_data)+1];
	strcpy(lineOfSensors, line_of_data);
		
	// Count the numer of sensors
	pCh = strtok(line_of_data, dataSep.c_str());
		
	i = 1;
	while ((pCh = strtok(NULL, dataSep.c_str()))){
			i++;
	}
	nSensorsInHeader = i;
	
	
	// Allocate sensor list
	sensorNames = new std::string [nSensorsInHeader];
	sensorUnits = new std::string [nSensorsInHeader];
	sensorTypes = new std::string [nSensorsInHeader];
	sensorBytes = new int [nSensorsInHeader];
		
		
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
		nBytes += len;
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
			int nTimestampSensors = 0;

			// Skip timestamp fields
			
			// TODO: It looks like the first three fields are optional
			// but always have the predefined names?! In this case the
			// definition can be moved to this class as well
			
			secCol = -1;
			subSecCol = -1;
			if (tobno == 1){
				for (i=0;i<nSensorsInHeader;i++){
					if (tobno == 1){
						if (sensorNames[i] == secCounterName) {
							nTimestampSensors += 1;
							secCol = i;
						}
						else if (sensorNames[i] == subSecCounterName) {
							nTimestampSensors += 1;
							subSecCol = i;
						}
					}
				}
			}
		
			printf("\n");
			printf("Sample configuration file:\n");
			printf("\tCopy to %s%s\n",configDir.c_str(), sensorListfile.c_str());
			printf("\tReplace the third column by the desired KITcube sensor names\n");
			printf("\n");
				
			printf("Number of sensors: %d\n", nSensorsInHeader - nTimestampSensors);

			
			int skip;
			int nSensorNo = 1;
			for (i=0;i<nSensorsInHeader;i++){
				skip = 0;
				if (tobno == 1){
					if (sensorNames[i] == secCounterName) skip = 1;
					else if (sensorNames[i] == subSecCounterName) skip = 1;
				}
	
				if (skip == 0) {
					printf("%d\t%s\t%s (%s)\t\n", nSensorNo, sensorNames[i].c_str(),
					   sensorNames[i].c_str(), sensorUnits[i].c_str());
					nSensorNo += 1;
				}
			}
			printf("\n\n");
		}
	

	// Skip processing flag of the data
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return(-1);
	}
	nBytes += len;
	if (debug > 3) printf("Processing: %s", line_of_data);


	
	
	// Read data types of the binary format
	if (read_ascii_line(&line_of_data, &len, data_file_ptr) == -1) {
		free(line_of_data);
		return(-1);
	}
	nBytes += len;
	if (debug > 3) printf("Format: %s", line_of_data);
	
	pCh = strtok(line_of_data, dataSep.c_str());
	
	i = 0;
	while (i<nSensorsInHeader){
		if (pCh[0] == '"')
			sensorTypes[i] = pCh+1;
		else
			sensorTypes[i] = pCh;
		sensorTypes[i].erase(sensorTypes[i].find('"'));
		
		pCh = strtok(NULL, dataSep.c_str());
		i++;
	}
	
    // Calculate length of the records
    // Available tpyes are
    // IEEE4, IEEE8,        = 4, 8 bytes
    // FP2,                 = 2 bytes
    // a single sign bit, a two-bit negative decimal exponent, and  a 13-bit mantissa
    // ULONG, LONG,         = 4 bytes
    // BOOL - 0x00, 0xff    = 1 byte
	// SecNano,             = ???
    // ASCII(len) - string  = len bytes

	err = 0;
	recordSize = 0;
	for (i=0;i<nSensorsInHeader;i++){
		if (sensorTypes[i] == "FP2") sensorBytes[i] = 2;
		else if (sensorTypes[i] == "FP4") sensorBytes[i] = 4;
		else if (sensorTypes[i] == "IEEE4") sensorBytes[i] = 4;
		else if (sensorTypes[i] == "IEEE4B") { sensorBytes[i] = 4; sensorTypes[i] = "IEEE4"; }
		else if (sensorTypes[i] == "IEEE8") sensorBytes[i] = 8;
		else if (sensorTypes[i] == "IEEE8B") { sensorBytes[i] = 8; sensorTypes[i] = "IEEE8"; }
		else if (sensorTypes[i] == "ULONG") sensorBytes[i] = 4;
		else if (sensorTypes[i] == "LONG") sensorBytes[i] = 4;
		else {
			printf("Error: Sensor type no %d, %s, %s is unkown\n",
				   i+1, sensorNames[i].c_str(), sensorTypes[i].c_str());
			sensorBytes[i] = 0;
			err = 1;
		}
		
		recordSize += sensorBytes[i];
	}
	
	if (err){
		printf("There undefined sensor types (compare list above)\n");
	}
	

	if (tobno == 1){
		// All records contain timestamp information
		lenDataSet = recordSize;
		subSecCounterResolution = 1000000000;
		
		nRecordsInFrame = 1;
		nProcessedRecords = 0;
	}
	else  {
		// Only a group of records has a timestamp
		lenDataSet = frameSize;
		// subSecCounterResolution (see above)
		// recordTimeStep (see above)
		
		
		// Calculate the number of records in one frame
		if (tobno == 2){
			nRecordsInFrame = (frameSize - 12) / recordSize;
		}
		if (tobno == 3){
			nRecordsInFrame = (frameSize - 16) / recordSize;
		}
	
		nProcessedRecords = 0;
	}
	
    // Save the length of the header and the dataset
    // lenHeader = <end of header block>
	if (debug > 3) printf("Length of the header : %3d bytes (ftell: %ld)\n", nBytes, ftell(data_file_ptr));
	if (debug > 3) printf("Length of one dataset: %3d bytes\n", lenDataSet);
	
	lenHeader = ftell(data_file_ptr);
	nColsInFile = nSensorsInHeader;
	
	
	delete [] lineOfSensors;
	delete [] sensorNames;
	delete [] sensorUnits;
	free(line_of_data);
	
	// close data file
	fclose(data_file_ptr);

	
	return err;
}



int CampbellTOB1::parseData(char *line, struct timeval *l_tData, double *sensorValue) {
	//float *local_sensorValue;
	//struct SPackedTime *time;
	//struct tm tm_zeit;
	
	int i;
	int nDataLeft;
	
	if(debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	if (sizeof(float) != 4) {
		printf("Size of 'float' is not 4! So not reading any data!\n");
		return -1;
	}
	
	
	//
	// Note: The timestamp of the TOB1 format seems to be based on 1.1.1990
	// This means there is a shift by 631148400 sec !!!
	//
	int timeBase = 631152000;

	// Parse datasets
	char *recPtr = line;
	
	// Parse header (only TOB2+3)
	// Table name: "Extra"
	// non-timestamped record interval :  e.g. "50 MSEC"
	// data frame size: "984" - use this instead of the calculated one ???
	// intended table size: "1900800"
	// validation stamp: "64308"
	// frame time resolution "Sec100Usec" ???
	
	// The TOB2+3 formats do not contain a timestamp for every dataset
	// The times need to be calcuated !!!
	// This also means that the procedure of parseData calls need to be modified !!!
	
	// The function parseData is called for every recored.
	// the number of remaining NOT parsed data needs to be returned
	
	
	// Parse frame header (only TOB2+3)
	if (tobno > 1){
		if (nProcessedRecords == 0) {
			uint32_t *data;

			//
			// Decode timestamp from the frame header
			//
		
			data = (uint32_t *)recPtr;
			frameTimestamp.tv_sec = *data + timeBase;
			if (debug > 3) printf("Seconds: %10d %10x\n", *data, *data);
		
			// Go to next value
			recPtr += 4;

			data = (uint32_t *)recPtr;
			frameTimestamp.tv_usec = ((long long) *data) * 1000000 / subSecCounterResolution;
			if (debug > 3) printf("Subseconds: %10d %10x\n", *data, *data);

			// Go to next value
			recPtr += 4;
		
			if (tobno == 3){
				data = (uint32_t *)recPtr;
				if (debug > 3) printf("Counter: %10d %10x\n", *data, *data);
		
				// Go to next value
				recPtr += 4;
			}
			
		}
		
		// Calculate the record time stamp
		l_tData->tv_sec = (long long) frameTimestamp.tv_sec
						+ nProcessedRecords * recordTimeStep.tv_sec
						+ ((frameTimestamp.tv_usec + nProcessedRecords * recordTimeStep.tv_usec) /1000000);
		l_tData->tv_usec = ((long long) frameTimestamp.tv_usec
						+ nProcessedRecords * recordTimeStep.tv_usec) %1000000;
	
		
		
		
	}
	
	
	// Parse datasets
	int sensorNo = 0;
	for (i=0;i<nColsInFile;i++) {
		if (sensorTypes[i] == "ULONG"){
			uint32_t *data;
			
			data = (uint32_t *)recPtr;
			if (debug > 4) printf("Sensor %3d: %10d %10x\n", i+1, *data, *data);
			
			
			if ((tobno == 1) && (secCol == i)) {
				// Save second counter
				l_tData->tv_sec = *data + timeBase;
			}
			else if ((tobno == 1) && (subSecCol == i)) {
				// Save second counter
				l_tData->tv_usec = ((long long) *data) * 1000000 /subSecCounterResolution;
			}
			else {
				sensorValue[sensorNo] = *data;
				sensorNo += 1;
			}
			
			// Go to next value
			recPtr += sensorBytes[i];
		}
		
		else if (sensorTypes[i] == "LONG") {
			int32_t *data;
			
			data = (int32_t *)recPtr;
			if (debug > 4) printf("Sensor %3d: %10d %10x\n", i+1, *data, *data);

			sensorValue[sensorNo] = *data;
			sensorNo += 1;

			// Go to next value
			recPtr += sensorBytes[i];
		}
		
		else if (sensorTypes[i] == "IEEE4") {
			float *data;
	
			data = (float *)recPtr;
			if (debug > 4) printf("Sensor %3d: %10f\n", i+1, *data);
			
			sensorValue[sensorNo] = *data;
			sensorNo += 1;
	
			
			// Go to next value
			recPtr += sensorBytes[i];
		}
		
		else if (sensorTypes[i] == "IEEE8") {
			double *data;
			
			data = (double *)recPtr;
			if (debug > 4) printf("Sensor %3d: %10f\n", i+1, *data);
			
			sensorValue[sensorNo] = *data;
			sensorNo += 1;
			
			
			// Go to next value
			recPtr += sensorBytes[i];
		}

		else if (sensorTypes[i] == "FP2") {
			float data;
			int16_t dataInt;
			char *dataIntPtr;
			char *fPtr;
			fPtr = (char *) &data;

			
			// Form the manual: FP2 data are two-byte big-endian values.
			// Currently all PC are Intel-based and thus little-endian.
			// Needs to be changed here before conversion
			dataIntPtr = (char *) &dataInt;
			dataIntPtr[0] = recPtr[1];
			dataIntPtr[1] = recPtr[0];
			
			int sign     = ((0x8000 & dataInt) >> 15 );
			int exponent = ((0x6000 & dataInt) >> 13 );
			int mantissa = ((0x1FFF & dataInt)       );
			
			switch(exponent) {
				case 0: data=(float)mantissa;
					break;
				case 1: data=(float)mantissa*1e-1;
					break;
				case 2: data=(float)mantissa*1e-2;
					break;
				default: data=(float)mantissa*1e-3;
			}
			if (sign == 1) data = -data;
			
			if (debug > 4) printf("Sensor %3d: %10f\n", i+1, data);
			
			if (debug >5) {
				unsigned short *dataShort;
				dataShort = (unsigned short *) recPtr;
				printf("FP2: raw %02x %02x  %04x -- sign=%d, exp=%d, mantissa=%d -- data %f\n", recPtr[0], recPtr[1], *dataShort, sign, exponent, mantissa, data);
			}
			
			sensorValue[sensorNo] = data;
			sensorNo += 1;
			
			// Go to next value
			recPtr += sensorBytes[i];
		}
		
		else if (sensorTypes[i] == "FP4") {
			double data;
			int32_t *dataInt;
			
			dataInt = (int32_t *) recPtr;
			
			// TODO: Test with sample data
			int sign     = ((0x80000000 & *dataInt) >> 31 );
			int exponent = ((0x7F000000 & *dataInt) >> 24 );
			int mantissa = ((0x00FFFFFF & *dataInt)       );
			data =(double)mantissa/16777216.0*(double)pow(2.0,(double)(exponent-64));
			if (sign == 1) data = - data;
			
			//printf("Sensor %3d: %10f\n", i+1, *data);
			if (debug > 4) printf("Sensor %3d: %10f %08x (FP4 - not tested)\n", i+1, data, *dataInt);
				
			sensorValue[sensorNo] = data;
			sensorNo += 1;
				
			// Go to next value
			recPtr += sensorBytes[i];
		}
		
		else {
			printf("Implementation for format %s is missing -- skipped\n", sensorTypes[i].c_str());
		}

		
	}
		   
	
	
	
/*
	
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
	
*/
	
	// Calculate the number of remaning data
	nProcessedRecords += 1;
	
	if (nProcessedRecords < nRecordsInFrame) {
		if (tobno == 1) nDataLeft = 0;
		if (tobno > 2) nDataLeft = (nRecordsInFrame - nProcessedRecords) * recordSize + 4;
	}
	else {
		// Start over with the next frame
		nProcessedRecords = 0;
		nDataLeft = 0;
	}
	
	return nDataLeft;
}




