/***************************************************************************
                          simrandom.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "regenwippe.h"


regenwippe::regenwippe(){
	
}


regenwippe::~regenwippe(){
}


void regenwippe::readHeader(const char *filename){
	
	noData = 999999;
	
	lenHeader = 0;	// no header
	
	lenDataSet = 32;	// inklusive Puffer fuer char Input Array; fgets stoppt nach "\n"
	
	profile_length = 0;	// no profile, scalar data
	
	// List of sensors
	nSensors = 2;
	sensor = new struct sensorType [nSensors];
	
	// set default value for height
	for (int i = 0; i < nSensors; i++) {
		sensor[i].height = 0;
	}
	
	sensor[0].comment = "Counts";
	sensor[0].data_format = "<scalar>";
	
	sensor[1].comment = "Rain intensity";
	sensor[1].data_format = "<scalar>";
	
	if (debug >= 1) {
		for (int i = 0; i < nSensors; i++) {
			printf("Sensor %3d: %s\n", i + 1, sensor[i].comment.c_str());
		}
	}
}


void regenwippe::setConfigDefaults(){

	
	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
}


void regenwippe::parseData(char *line, struct timeval *l_tData, float *sensorValue){
	struct tm timestamp;
	char* puffer;
	
	
	// TODO/FIXME: are data timestamps in UTC or local time?
	
	// read date and time
	puffer = strptime(line,"%d.%m.%Y %T", &timestamp);
	if (puffer == NULL) {
		printf("Regenwippe: Error reading date and time string!\n");
		return;
	}
	
	// read sensor values
	sscanf(puffer, "%f", &sensorValue[0]);
	
	// get seconds since the Epoch
	l_tData->tv_sec = timegm(&timestamp);	// FIXME: this function is a non-standard GNU extension, try to avoid it!
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
	n = strftime(line, 19, "%d.%m.%Y %T", date);
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


void regenwippe::copyRemoteData(){
	struct timeval t0, t1;
	struct timezone tz;
	int err, pos;
	char line[256];
	std::string output, data_files_wildcard;
	
	
	if (debug > 2) printf("_____regenwippe::copyRemoteData()_____\n");
	
	createDirectories((archiveDir + getDataDir()).c_str());
	
	// Transfer only files of the specified sensor group (== file type)
	// The time to find changes in large file increases with the file size
	// Therefore it is necessary to use the rsync option --append. This option
	// will make rsync assume that exisitings files will only be added at the end.
	//
	// TODO: Improve output of the system call. The output is mixed with others
	//       Solution 1: Write output to file,
	//       Solution 2: Use pipes (example can be found in README
	//
	if (debug > 2)
		output = "";
	else
		output = " > /dev/null";
	
	data_files_wildcard = datafileMask;
	pos = data_files_wildcard.find("<index>");
	data_files_wildcard.replace(pos, 7, "*");
	
	sprintf(line, "rsync -avz %s --include='*/' --include='%s' --exclude='*' %s%s  %s%s %s",
			rsyncArgs.c_str(), data_files_wildcard.c_str(),
			remoteDir.c_str(), getDataDir(),
			archiveDir.c_str(), getDataDir(), output.c_str());
	if (debug > 2) printf("%s\n", line);
	
	gettimeofday(&t0, &tz);
	err = system(line);
	gettimeofday(&t1, &tz);
	
	if (err != 0) {
		printf("Synchronisation error (rsync)\n");
		//throw std::invalid_argument("Synchronisation error (rsync)");
	}
	
	if (debug > 2) printf("Rsync duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));
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
