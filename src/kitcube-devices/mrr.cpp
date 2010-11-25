//
// C++ Implementation: mrr
//
// Description: 
//
//
// Author: Norbert Flatinger <norbert.flatinger@kit.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "mrr.h"


mrr::mrr(){
}


mrr::~mrr(){
}


int mrr::readHeader(const char *filename) {
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	if (sensorGroup == "dat") {
		lenHeader = 0x2F6;	// CAUTION: header has 10 lines
	}
	
	return 0;
}


void mrr::writeHeader(){
	std::string filename;
	FILE *template_file;
	char header_line[256];
	int i;
	
	
	printf("\n_____mrr::writeHeader()_____\n");
	
	if (sensorGroup == "dat") {
		// Read the template header
		if (datafileTemplate.length() == 0) throw std::invalid_argument("No template file given");
	
		filename = configDir + datafileTemplate;
		printf("Reading header from template file: %s\n", filename.c_str());
		
		// open template file
		template_file = fopen(filename.c_str(), "r");
		
		// work on 10 header lines here
		for (i = 0; i < 10; i++) {
			// read header
			fgets(header_line, 256, template_file);
			
			// write header
			fprintf(fdata, "%s", header_line);
		}
	
		// close template file
		fclose(template_file);
	}
}


void mrr::writeData(){
	FILE *template_file;
	char line[256];
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
	template_file = fopen(template_filename.c_str(), "r");
	
	// throw away 10 header lines for *.dat data file
	if (sensorGroup == "dat") {
		for (i = 0; i < 10; i++) {
			fgets(line, 256, template_file);
		}
	}
	
	// get minute of day
	time(&time_in_sec);
	date = gmtime((const time_t *) &time_in_sec);
	day_min = date->tm_hour * 60 + date->tm_min;
	
	// throw away last (day_min - 1) data sets
	for (i = 0; i < (day_min * 31); i++) {
		fgets(line, 256, template_file);
	}
	
	// read and write the actual data set
	for (i = 0; i < 31; i++) {
		fgets(line, 256, template_file);
		data_set = line;
		
		if (i == 0) {
			// replace date and time
			sprintf(line, "%d%02d%02d%02d%02d%02d", (date->tm_year + 1900), date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
			data_set.replace(2, 14, line);
		}
		
		fprintf(fdata, "%s", data_set.c_str());
		fflush(fdata);
	
		if (debug > 1)
			printf("%s", data_set.c_str());
	}
	
	fclose(template_file);
}


/* - parsivel files (p<index>.dat) get handled by parsivel class, so they have
     sensor group number 1
   - int01_<index>.dat gets sensor group number 2
*/
unsigned int mrr::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	if (sensorGroup == "dat") {
		number = 2;
		buffer = "Mikro Regenradar 1D Profile";
	}
	
	return number;
}


void mrr::readData(const char *dir, const char *filename){
	fd_eof = false;
}
