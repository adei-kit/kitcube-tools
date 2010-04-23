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


void parsivel::readHeader(const char *filename){
	printf("parsivel::readHeader: no header to read!\n");
	
	lenHeader = 0;	// no header
}


const char *parsivel::getDataFilename(){
	time_t time_in_sec;
	struct tm *date;
	char line[256];
	int posIndex;
	std::string temp;
	
	
	// Get the actual day
	time(&time_in_sec);	// get seconds since the Epoch
	date = gmtime((const time_t *) &time_in_sec);
	
	// print date string of file name to buffer
	sprintf(line, "%02d%02d%02d", date->tm_year - 100, date->tm_mon + 1, date->tm_mday);
	
	// replace <index> in datafile mask with date string
	temp = datafileMask;
	posIndex = temp.find("<index>");
	temp.replace(posIndex, 7, line);
	
	return(temp.c_str());
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
			sprintf(line, "%02d.%02d.%02d", date->tm_mday, date->tm_mon + 1, (date->tm_year + 1900));
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


void parsivel::readData(const char *dir, const char *filename){
	fd_eof = false;
}