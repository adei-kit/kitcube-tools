/***************************************************************************
                          simrandom.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "regenwippe.h"


regenwippe::regenwippe(){
	sensor_value_old = 0;
}


regenwippe::~regenwippe(){
}


int regenwippe::readHeader(const char *filename) {
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	noData = 999999;
	
	lenHeader = 0;	// no header
	
	lenDataSet = 32;	// inklusive Puffer fuer char Input Array; fgets stoppt nach "\n"
	
	profile_length = 0;	// no profile, scalar data
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
        sensor[i].size = 1;        
	}
	
	sensor[0].comment = "Rain amount";
	sensor[0].data_format = "<scalar>";
	
	if (debug >= 1) {
		for (int i = 0; i < nSensors; i++) {
			printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
		}
	}
	
	return 0;
}


void regenwippe::setConfigDefaults(){

	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
}


int regenwippe::parseData(char *line, struct timeval *l_tData, double *sensorValue){
	struct tm timestamp;
	char* puffer;
	int sensor_value_new;
	
	
	// TODO/FIXME: are data timestamps in UTC or local time?
	
	// read date and time
	puffer = strptime(line,"%d.%m.%Y %T", &timestamp);
	if (puffer == NULL) {
		printf("Regenwippe: Error reading date and time string!\n");
		return -1;
	}
	
	// read sensor values
	sscanf(puffer, "%d", &sensor_value_new);
	if (sensor_value_new < sensor_value_old) {	// overflow
		sensorValue[0] = 999999;
	} else {
		sensorValue[0] = (double)(sensor_value_new - sensor_value_old) * 0.2;
	}
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
	
	sensor_value_old = sensor_value_new;
	
	return 0;
}


void regenwippe::writeData(){
	FILE *file;
	char line[lenDataSet];
	time_t time_in_sec;
	struct tm *date;
	int day_min, i;
	std::string data_set, template_filename;
	size_t n;
	
	
	if (fdata <= 0)
		return;
	
	// Analyse time stamp, if new day a new file needs to generated
	if (filename != getDataFilename()) {
		openNewFile();
	}
	
	if (datafileTemplate.length() == 0)
		throw std::invalid_argument("No template file given");
	
	
	// open template file
	template_filename = configDir + datafileTemplate;
	file = fopen(template_filename.c_str(), "r");
	
	if (debug >= 1)
		printf("Reading data from template file: %s\n", template_filename.c_str());
	
	// get minute of day
	time(&time_in_sec);
	date = gmtime((const time_t *) &time_in_sec);
	day_min = date->tm_hour * 60 + date->tm_min;
	
	// read and throw away the last (day_min - 1) data sets
	for (i = 0; i < day_min; i++) {
		fgets(line, lenDataSet, file);
	}
	
	// read actual data set
	fgets(line, lenDataSet, file);
	
	fclose(file);
	
	
	// replace date and time in data set
	data_set = line;
	n = strftime(line, 20, "%d.%m.%Y %T", date);
	if (n != 19) {
		printf("Regenwippe: Error writing date and time string!\n");
		return;
	}
	data_set.replace(0, 19, line);
	
	// write data set
	fprintf(fdata, "%s", data_set.c_str());
	fflush(fdata);
	
	if (debug > 1)
		printf("%s", data_set.c_str());
}


unsigned int regenwippe::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "dat") {
		number = 1;
		buffer = "time series";
	}
	
	return number;
}
