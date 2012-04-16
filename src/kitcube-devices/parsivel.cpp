//
// C++ Implementation: parsivel
//
// Description: 
//
//
// Author: Andreas Kopmann <andreas.kopmann@kit.edu>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "parsivel.h"


parsivel::parsivel(){
}


parsivel::~parsivel(){
}


unsigned int parsivel::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	if (sensorGroup == "dat") {
		number = 1;
		buffer = "Parsivel Daten";
	}
	
	return number;
}


int parsivel::readHeader(const char *filename) {
	char line[256];
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	lenHeader = 0;	// no header
	noData = 9999999;
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
        sensor[i].size = 1;
	}
	
	sensor[0].comment = "Weather code";
	sensor[0].data_format = "<scalar>";
	
	sensor[1].comment = "Accumulated rain amount (mm)";
	sensor[1].data_format = "<scalar>";
	
	sensor[2].comment = "Rain intensity (mm/h)";
	sensor[2].data_format = "<scalar>";
	
	sensor[3].comment = "Radar reflectivity (dBZ)";
	sensor[3].data_format = "<scalar>";
	
	sensor[4].comment = "Sichtweite";
	sensor[4].data_format = "<scalar>";
	
	sensor[5].comment = "Signal amplitude laser";
	sensor[5].data_format = "<scalar>";

  	sensor[6].comment = "Total number of drops";
	sensor[6].data_format = "<scalar>";  
	
	sensor[7].comment = "drop velocities (m/s)";
	sensor[7].data_format = "<profile size=\"32\"> <unknown unit=\"no unit\">";
	for (int i = 1; i < 32; i++) {
		sprintf(line, "%i ", i);
		sensor[0].data_format += line;
	}
	sensor[7].data_format += "32</unknown> </profile>";
	sensor[7].type = "profile";
    sensor[7].size = 32;
	
	sensor[8].comment = "drop density (1/m^3/mm)";
	sensor[8].data_format = "<profile size=\"32\"> <unknown unit=\"no unit\">";
	for (int i = 1; i < 32; i++) {
		sprintf(line, "%i ", i);
		sensor[0].data_format += line;
	}
	sensor[8].data_format += "32</unknown> </profile>";
	sensor[8].type = "profile";
    sensor[8].size = 32;
	
	sensor[9].comment = "Number of drops";
	sensor[9].data_format = "<2D size=\"32*32\">";
	sensor[9].type = "profile";
    sensor[9].size = 32*32;
	
	if (debug >= 1) {
		for (int i = 0; i < nSensors; i++) {
			printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
		}
	}
	
	return 0;
}


int parsivel::parseData(char *line, struct timeval *l_tData, double *sensorValue){
    std::string buf;
	int i;
    //double value;
	//int err;
    char *res;
    struct tm timestamp_tm;
	double *pSensor;
    double *pRain;
    
	
	if (debug >= 4)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
    
    
    pRain = sensorValue + 6; // Number of rain drops in interval
    

    buf = line;
    if (debug > 5) printf("%s ... (n=%ld) \n", buf.substr(0,60).c_str(), strlen(line));
    
    
    // Read time stamp
    if (buf.substr(0,3) == "#0#"){
        res = strptime(line, "#0#,%d.%m.%Y %T", &timestamp_tm);
        timestamp_tm.tm_gmtoff = 0;
        l_tData->tv_sec = timegm(&timestamp_tm);
        //printf("DATE: %s %d(res=%d)\n", asctime(&timestamp_tm), l_tData->tv_sec, res);
    }

    if (buf.substr(0,3) == "#1#"){
        res = strtok(line, ",");
        for (i=0;i<6;i++){
            res = strtok(NULL, ",");
            sscanf(res, "%lf", &(sensorValue[i]));            
            // Replace -9.999 with the nodata flag
            if (sensorValue[i] == -9.999) sensorValue[i] = noData; 
            
            if (debug > 5) printf("%s %d: %f \n", res, i, sensorValue[i]);             
        } 
        if (debug > 5) printf("\n");
    }

    if (buf.substr(0,3) == "#2#"){
        res = strtok(line, ",");
        for (i=0;i<6;i++){
            res = strtok(NULL, ",");
            if (i==5){
                // Only the totel number of drop is in the second field
                sscanf(res, "%lf", &(sensorValue[6]));            
                if (debug > 5) printf("%s %d: %f \n", res, 6, sensorValue[6]);  
            }
        } 
        if (debug > 5) printf("\n");
    }
    
    if (buf.substr(0,3) == "#3#"){
        pSensor = sensorValue + 7;
        
        *pSensor = noData;
        if (*pRain > 0){
            res = strtok(line, ",");
            for (i=0;i<32;i++){
                res = strtok(NULL, ",");
                sscanf(res, "%lf", &(pSensor[i]));
                if (debug > 5) printf("%f \t", pSensor[i]);            
            } 
            if (debug > 5) printf("\n");
        }
    }    

    if (buf.substr(0,3) == "#4#"){
        pSensor = sensorValue + 7 + 32;

        *pSensor = noData;
        if (*pRain > 0){        
            res = strtok(line, ",");
            for (i=0;i<32;i++){
                res = strtok(NULL, ",");
                sscanf(res, "%lf", &(pSensor[i]));
                if (debug > 5) printf("%f \t", pSensor[i]);   
            }
        if (debug > 5) printf("\n");
        } 
    }    
    
    // Check if it's the last line
    if (buf.substr(0,3) == "#5#"){
        pSensor = sensorValue + 7 + 32 + 32;
        
        *pSensor = noData;
        if (*pRain > 0){        
            res = strtok(line, ",");
            for (i=0;i<32*32;i++){
                res = strtok(NULL, ",");
                sscanf(res, "%lf", &(pSensor[i]));
                if (debug > 5) printf("%f\t", pSensor[i]);            
            }
            if (debug > 5) printf("\n");
        }
        
        // The dataset is now complete
        return(0);
    }
		
    // Read the next line
	return 1;
}



void parsivel::writeData(){
	FILE *file;
	char line[4150];
	time_t time_in_sec;
	struct tm *date;
	int day_min, i;
	std::string data_set, template_filename;
	
	
	if (fdata <= 0) return;
	
	// Analyse time stamp, if new day a new file needs to generated
	if (filename != getDataFilename()) {
		openNewFile();
	}
	
	// Read the template header
	if (datafileTemplate.length() == 0) throw std::invalid_argument("No template file given");
    
	template_filename = configDir + datafileTemplate;
	printf("Reading data from template file %s\n", template_filename.c_str());
	
	// open template file
	file = fopen(template_filename.c_str(), "r");
	
	// get minute of day
	time(&time_in_sec);
	date = gmtime((const time_t *) &time_in_sec);
	day_min = date->tm_hour * 60 + date->tm_min;
	
	// throw away last (day_min - 1) data sets
	for (i = 0; i < (day_min * 6); i++) {
		fgets(line, 4150, file);
	}
	
	// read actual data set
	for (i = 0; i < 6; i++) {
		fgets(line, 4150, file);
		data_set = line;
		
		if (i == 0) {
			// replace date
			sprintf(line, "%02d.%02d.%d", date->tm_mday, date->tm_mon + 1, (date->tm_year + 1900));
			data_set.replace(4, 10, line);
		}
		
		fprintf(fdata, "%s", data_set.c_str());
		fflush(fdata);
        
		if (debug > 1)
			printf("%s", data_set.c_str());
	}
	
	fclose(file);
}


