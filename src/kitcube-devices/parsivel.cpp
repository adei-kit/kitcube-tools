//
// C++ Implementation: parsivel
//
// Description: 
//
//
// Author: Norbert Flatinger <norbert.flatinger@kit.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "parsivel.h"


parsivel::parsivel(){
}


parsivel::~parsivel(){
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
	
	sensor[6].comment = "drop velocities (m/s)";
	sensor[6].data_format = "<profile size=\"32\"> <unknown unit=\"no unit\">";
	for (int i = 1; i < 32; i++) {
		sprintf(line, "%i ", i);
		sensor[0].data_format += line;
	}
	sensor[6].data_format += "32</unknown> </profile>";
	sensor[6].type = "profile";
	
	sensor[7].comment = "drop density (1/m^3/mm)";
	sensor[7].data_format = "<profile size=\"32\"> <unknown unit=\"no unit\">";
	for (int i = 1; i < 32; i++) {
		sprintf(line, "%i ", i);
		sensor[0].data_format += line;
	}
	sensor[7].data_format += "32</unknown> </profile>";
	sensor[7].type = "profile";
	
	sensor[8].comment = "Number of drops";
	sensor[8].data_format = "<2D size=\"32*32\">";
	sensor[8].type = "profile";
	
	if (debug >= 1) {
		for (int i = 0; i < nSensors; i++) {
			printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
		}
	}
	
	return 0;
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


void parsivel::readData(std::string full_filename){
	std::string filenameData;
	float* local_sensorValue;
	unsigned long lastPos;
	struct timeval lastTime;
	char line[4150];
	FILE *fmark;
	long lastIndex;
	struct timeval timestamp_data;
	int iLoop;
	char *lPtr, *date_time;
	
	
	if (debug >= 1)
		printf("_____parsivel::readData(const char *dir, const char *filename)_____\n");
	
	// Compile file name
	filenameData = full_filename;
	
	// If number of sensors is unknown read the header first
	if (nSensors == 0)
		readHeader(filenameData.c_str());
	
#ifdef USE_MYSQL
	if (db == 0) {
		openDatabase();
	} else {
		// Automatic reconnect
		if (mysql_ping(db) != 0){
			printf("Error: Lost connection to database - automatic reconnect failed\n");
			throw std::invalid_argument("Database unavailable\n");
		}
	}
#endif
	
	// number of sensors here: 6 + 32 + 32 + 32 * 32
	local_sensorValue = new float [nSensors - 3 + 32 + 32 + 32 * 32];
	
	if (debug > 3) printf("Open data file %s\n", filenameData.c_str());
	fdata = fopen(filenameData.c_str(), "r");
	if (fdata <= 0) {
		printf("Error opening data file %s\n", filenameData.c_str());
		return;
	}
	
	// Get the last time stamp + file pointer from 
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	if (debug > 3) printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
		fclose(fmark);
		
		// Read back the data time stamp of the last call
		timestamp_data.tv_sec = lastTime.tv_sec;
		timestamp_data.tv_usec = lastTime.tv_usec;
		
		if (debug > 4) printf("Last time stamp was %ld\n", lastTime.tv_sec);
	}
	
	if (lastPos == 0)
		lastPos = lenHeader; // Move to the the first data
	
	// Find the beginning of the new data
	if (debug > 4)
		printf("LastPos: %ld\n", lastPos);
	fseek(fdata, lastPos, SEEK_SET);
	
	iLoop = 0;
	lPtr = (char *) 1;
	while ((lPtr > 0) && (iLoop < 100)) {
		
		lPtr = fgets(line, 4150, fdata);
		
		if (lPtr != NULL) {
			if (strlen(line) != 24) {
				// bail out
			} else{
				// read date and time
				date_time = strtok(line, ",");
				date_time = strtok(NULL, ",");
			}
			
		}
		
		
		
	}
}
