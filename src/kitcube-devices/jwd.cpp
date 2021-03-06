/***************************************************************************
                          simrandom.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "jwd.h"


jwd::jwd(){
	
	moduleType = "JWD";
	moduleNumber = 0;
	this->sensorGroup = "txt";
	
	this->iniGroup = "JWD-dd";
}


jwd::~jwd(){
}


int jwd::readHeader(const char *filename) {
	char line[256];
	
	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	noData = 999999;
	
	if (sensorGroup == "dd") {
		lenHeader = 0xd4;	// CAUTION: header has several lines!
		
		lenDataSet = 128;	// 106 plus kleiner Puffer, hier OK, da fgets nach "\n" stoppt
		
		profile_length = 0;
		
		// set default value for height
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 0;
            sensor[i].size = 1;
		}
		
		sensor[0].comment = "Rain intensity";
		sensor[0].data_format = "<scalar>";
		
		sensor[1].comment = "Spectral number density";
		sensor[1].data_format = "<scalar>";
		
		sensor[2].comment = "Radar reflectivity";
		sensor[2].data_format = "<scalar>";
		
		sensor[3].comment = "unknown";
		sensor[3].data_format = "<scalar>";
		
		sensor[4].comment = "unknown";
		sensor[4].data_format = "<scalar>";
		
		sensor[5].comment = "unknown";
		sensor[5].data_format = "<scalar>";
		
		sensor[6].comment = "unknown";
		sensor[6].data_format = "<scalar>";
		
		sensor[7].comment = "Accumulated rain amount";
		sensor[7].data_format = "<scalar>";
		
		if (debug) {
			for (int i = 0; i < nSensors; i++) {
				printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
			}
		}
	} else if (sensorGroup == "rd") {
		lenHeader = 0;	// no header
		
		lenDataSet = 128;	// 85 plus kleiner Puffer, hier OK, da fgets nach "\n" stoppt
		
		profile_length = 20;
		
		for (int i = 0; i < nSensors; i++) {
			sensor[i].height = 0;
		}
		
		sensor[0].comment = "unknown";
		sensor[0].type = "profile";
		sensor[0].data_format = "<profile size=\"20\"> <unknown unit=\"no unit\">";
		for (int i = 1; i < profile_length; i++) {
			sprintf(line, "%i ", i);
			sensor[0].data_format += line;
		}
		sensor[0].data_format += "20</unknown> </profile>";
        sensor[0].size = 20; 
		
		if (debug) {
			for (int i = 0; i < nSensors; i++) {
				printf("Sensor %3d: %s\n", i+1, sensor[i].comment.c_str());
			}
		}
	} else {
		printf("Unknown sensor group!\n");
	}
	
	return 0;
}


void jwd::setConfigDefaults(){
	char line[256];
	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation 
	// of the class but first time after reading from the inifile
	// 
	
	this->moduleName = "Sim";
	this->moduleComment = "Simulation";
		
	this->datafileTemplate = ""; // No template file required

	sprintf(line, "sim_<index>.%s", sensorGroup.c_str());
	this->datafileMask = line;
	//printf("datafileMask = %s\n", datafileMask.c_str());
	
	sprintf(line, "jwd.%s.sensors", sensorGroup.c_str());
	this->sensorListfile = line;
	
}


int jwd::parseData(char *line, struct timeval *l_tData, double *sensorValue){
	int yday, mod;
	char date[7] = {0}, time[5] = {0};
	struct tm timestamp;
	std::string profile_data;
	
	
	// TODO/FIXME: are data timestamps in UTC or local time?
	
	if (sensorGroup == "dd") {
		// read date, time and sensor values of one line of data
		sscanf(line, "%s %i %s %i %lf %lf %lf %lf %lf %lf %lf %lf",
			date, &yday, time, &mod,
			&sensorValue[0], &sensorValue[1], &sensorValue[2], &sensorValue[3],
			&sensorValue[4], &sensorValue[5], &sensorValue[6], &sensorValue[7]);
	} else if (sensorGroup == "rd") {
		// read date and time of one line of data
		sscanf(line, "%s %i %s", date, &yday, time);
		
		profile_data.assign(line + 21, 3);
		mod = atoi(profile_data.c_str());
		
		// read sensor values of one line of data
		for (int i = 0; i < profile_length; i++) {
			profile_data.assign(line + 24 + i * 3, 3);
			sensorValue[i] = atoi(profile_data.c_str());
		}
	}
	
	// calculate timestamp in seconds:
	
	// evaluate date...
	std::string year(date, 2);
	std::string month(date + 2, 2);
	std::string day(date + 4, 2);
	
	// ...and fill tm structure
	timestamp.tm_mday = atoi(day.c_str());
	timestamp.tm_mon = atoi(month.c_str()) - 1;
	timestamp.tm_year = atoi(year.c_str()) + 100;
	
	// evaluate time...
	std::string hour(time, 2);
	std::string min(time + 2, 2);
	std::string sec("0");
	
	// ...and fill tm structure
	timestamp.tm_hour = atoi(hour.c_str());
	timestamp.tm_min = atoi(min.c_str());
	timestamp.tm_sec = atoi(sec.c_str());
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: function is non-standard GNU extension
	
	return 0;
}


void jwd::writeData(){
	FILE *file;
	char line[128];
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
	
	// throw away 2 header lines for dd* data file
	if (sensorGroup == "dd") {
		for (i = 0; i < 2; i++) {
			fgets(line, 128, file);
		}
	}
	// rd* data files don't have a header
	
	// get minute of day
	time(&time_in_sec);
	date = gmtime((const time_t *) &time_in_sec);
	day_min = date->tm_hour * 60 + date->tm_min;
	
	// throw away last (day_min - 1) data sets
	for (i = 0; i < day_min; i++) {
		fgets(line, 128, file);
	}
	
	// read actual data set
	fgets(line, 128, file);
	
	fclose(file);
	
	// replace date and year_day
	data_set = line;
	// print date string of file name to buffer
	sprintf(line, "%02d%02d%02d", date->tm_year - 100, date->tm_mon + 1, date->tm_mday);
	if (sensorGroup == "dd") {
		data_set.replace(1, 6, line);
	} else if (sensorGroup == "rd") {
		data_set.replace(0, 6, line);
	}
	sprintf(line, "%3d", date->tm_yday);
	if (sensorGroup == "dd") {
		data_set.replace(10, 3, line);
	} else if (sensorGroup == "rd") {
		data_set.replace(9, 3, line);
	}

	fprintf(fdata, "%s", data_set.c_str());
	fflush(fdata);
	
	if (debug > 1)
		printf("%s", data_set.c_str());
}


void jwd::writeHeader(){
	std::string filename;
	FILE *template_file;
	char header_line[128];
	int i;


	if (sensorGroup == "dd") {
		// Read the template header
		if (datafileTemplate.length() == 0) throw std::invalid_argument("No template file given");
	
		filename = configDir + datafileTemplate;
		printf("Reading header from template file %s\n", filename.c_str());
		
		// open template file
		template_file = fopen(filename.c_str(), "r");
		
		// work on 2 header lines here
		for (i = 0; i < 2; i++) {
			// read header
			fgets(header_line, 128, template_file);
			
			// write header
			fprintf(fdata, "%s", header_line);
		}
	
		// close template file
		fclose(template_file);
	}
	// rd* data files do not have a header
}


unsigned int jwd::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "dd") {
		number = 1;
		buffer = "time series";
	}
	
	if (sensorGroup == "rd") {
		number = 2;
		buffer = "profile";
	}
	
	return number;
}
