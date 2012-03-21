/***************************************************************************
                          mast.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "mast.h"


#define ICONV_DONE() (r>=0)
#define ICONV_INVAL() (r<0) && (errno==EILSEQ))
#define ICONV_OVER() (r<0) && (errno==E2BIG))
#define ICONV_TRUNC() (r<0) && (errno==EINVAL))


struct SPackedTime {
	unsigned char nTag;
	unsigned char nMonat;
	unsigned short nJahr;
	unsigned char nStunde;
	unsigned char nMinute;
	unsigned char nSekunde;
	unsigned char nHundertstel;
};


Mast::Mast():DAQBinaryDevice() {
	
	this->lenHeader = 0x10000; // 64k Header block
	this->lenDataSet = 0; // Depends on the number of sensors - updated in readHeader
	
	headerRaw = 0;
	
	noData = -9999;
}


Mast::~Mast() {
	// Free header memory
	if (headerRaw > 0) delete [] headerRaw;
	
}


void Mast::setConfigDefaults() {
	
}


const char *Mast::getDataDir() {
	char line[256];
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "%s/DATA/", moduleName.c_str());
	buffer = line;
	
	return(buffer.c_str());
}


const char *Mast::getDataFilename() {
	char line[256];
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "M%02d_%02ld.%s", moduleNumber, fileIndex, sensorGroup.c_str());
	buffer = line;
	
	return(buffer.c_str());
}


// Move to base class?
void Mast::replaceItem(const char **header, const char *itemTag, const char *newValue) {
	bool findTag;
	const char *ptr;
	const char *startChar;
	//char *endChar;
	int i;
	int len;
	
	
	// TODO: Add length of the header to avoid searching outside the header !!!
	
	findTag = false;
	i = 0;
	while ((!findTag) && (i < 20)) {
		ptr = strcasestr(*header,  itemTag);
		if (ptr > 0){
			startChar = strstr(ptr, ": ");
			if (startChar > 0) {
				
				// Replace the data in the header
				// TODO: Move the rest of the header?!
				//       Check if the value has the same length?!
				len = strlen(newValue);
				strncpy((char *)startChar+2, newValue,len);
				// TODO: End is not found properly?!	
			}
		}
		i++;
	}
	
	*header = startChar + 2 + len;
}


const char *Mast::getStringItem(const char **header, const char *itemTag) {
	bool findTag;
	const char *ptr;
	const char *startChar;
	const char *endChar;
	int i;
	int len;
	
	
	// TODO: Add length of the header to avoid searching outside the header !!!
	
	findTag = false;
	i = 0;
	while ( (!findTag) && (i < 20)) {
		ptr = strcasestr(*header,  itemTag);
		if (ptr > 0) {
			startChar = strstr(ptr, ": ");
			if (startChar > 0) {
			 
				// Find the end of the line
				// TODO: End is not found properly?!
				endChar = strstr(ptr, "\n");
				if (endChar > 0) {
					//printf("getStringItem:  %02x %02x %02x %02x --- ", *(endChar-2), *(endChar-1), endChar[0], endChar[1]);
					
					len = endChar - startChar - 3;
					std::string tag(startChar + 2, len);
					buffer = tag;
					findTag = true;
				}
			}
		}
		i++;
	}
	
	*header = endChar + 1;
	return (buffer.c_str());
}


int Mast::getNumericItem(const char **header, const char *itemTag) {
	int value;
	const char *ptr;
	
	ptr = getStringItem(header, itemTag);
	value = atoi(ptr);
	
	return(value);
}


unsigned int Mast::getSensorGroup() {
	unsigned int number;

	switch (sensorGroup.at(2)) {
	case 'L': 
	case 'l': // slow / 1Hz data
		number = 1;
		buffer = "slow sensors (1Hz)";
		break;
	case 'S': 
	case 's': // fast / 20Hz data
		number = 2;
		buffer = "fast sensors (20Hz)";
		break;
	case 'R':
	case 'r': // 10min mean data (contains groups l + s)
		number = 3;
		buffer = "10min mean values";
		break;
	case 'X': 
	case 'x': // 10min mean calculated data (based on r data)
		number = 4;
		buffer = "10min calculates values";
		break;
		
	default:
		number = 0;
	}
	
	return( number);
}


int Mast::readHeader(const char *filename) {
	int fd;
	unsigned char *header;
	const char *headerReadPtr;
	std::string timeString;
	std::string dateString;
	int n;
	int i;
	int num_sensors, avg_time;
	std::string heightString;
	
	
	if(debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// Get the sensor names from the configuration file 
	if (nSensors == 0) getSensorNames(sensorListfile.c_str());
	
	
	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("Error opening file %s", filename);
		return -1;
	}
	
	// Read the complete header
	headerRaw = new unsigned char [lenHeader]; // Is stored in the class variables
	header = new unsigned char [lenHeader + 100];
	
	n = read(fd, headerRaw, lenHeader);
	
	close(fd);
	
	if (n < lenHeader) {
		return -1;
	}
	
#ifdef HAVE_ICONV_H
	//
	// Convert character encoding
	//
	iconv_t ic;
	size_t nIn, nOut;
	char *in;
	char *out;
	int r;
	
	// Windows --> Mac
	//ic = iconv_open("ISO-8859-1", "ISO-8859-1"); // Test
	ic = iconv_open("UTF-8", "ISO-8859-1");
	if (ic < 0) {
		printf("Conversion codes not available\n");
	}

	nIn = n;
	in = (char *) headerRaw;
	nOut = lenHeader + 400;
	out = (char *) header;

	//printf("nIn = %d, nOut = %d\n", nIn, nOut);
	r = iconv(ic, &in, &nIn, &out, &nOut);
	//printf("nIn = %d, nOut = %d\n", nIn, nOut);
	
	if (r < 0) {
		printf("Error during iconv (errno = %d)\n", errno);
		if (errno == EILSEQ)
			printf("Character not available in output or input sream (%d %d %d)\n",
			       in[0], in[1], in[2]);
	}
	iconv_close(ic);
#else
	memcpy(header, headerRaw, lenHeader);
#endif
	
	//----------------------------------------------------------------------
	// Read parameters from header
	//----------------------------------------------------------------------
	headerReadPtr = (const char *) header;
	
	if (debug >= 3) {
		// averaging intervall
		avg_time = getNumericItem(&headerReadPtr, "Mittelung");
		
		// reference timestamp - the ticks are relative to this time stamp (see parseData(...))
		// reference time
		timeString = getStringItem(&headerReadPtr, "Referenzzeit");
		// reference date
		dateString = getStringItem(&headerReadPtr, "Referenzdatum");
		
		// Experiment name
		experimentName = getStringItem(&headerReadPtr, "Versuchskennung");
	}
	
	
	// number of sensors
	num_sensors = getNumericItem(&headerReadPtr, "Anzahl");
	
	
	if (debug >= 3) {
		printf("Module:				%s, ID %03d, Group %s ID %d\n",
		       moduleName.c_str(), moduleNumber, sensorGroup.c_str(), sensorGroupNumber);
		printf("Averaging intervall (min):	%d\n", avg_time);
		printf("Reference time stamp:		[%s] [%s]\n", dateString.c_str(), timeString.c_str());
		printf("Experiment name:		[%s]\n", experimentName.c_str());
		printf("Number of sensors:		%d\n", num_sensors);
	}
	
	
	if (num_sensors != nSensors) {
		printf("\033[31mError: wrong number of sensors in data file: found %d, should be %d\033[0m\n",
		       num_sensors, nSensors);
		return -1;
	}
	
	// update length of data sets according to data format, see parseData
	lenDataSet = 4 + nSensors * 4 + 8;
	
	for (i = 0; i < nSensors; i++) {
		// read sensor description, that's only nice to have...
		sensor[i].longComment = getStringItem(&headerReadPtr, "Kanalbeschreibung");
		
		// read sensor value unit, that's only nice to have...
		sensor[i].unit = getStringItem(&headerReadPtr, "Kanalgruppe");
		
		// read sensor height
		heightString = getStringItem(&headerReadPtr, "Montagehoehe");
		sscanf(heightString.c_str(), "%f", &sensor[i].height);
	}
	
	delete [] header;
	
	// TODO: Move this to a separte function
	//       Reading the header should initialize all structures but take no action
	
	// Check if Data table is available 
	// If not create the database table
	
	// Column naming
	//<phys type><module number:2><device number:1><sensor number in list:3>.<Mean|Max|Min|SD>
	// Might be necessary to have 6 digits here?
	//
	
	// Write to database
	// Check if datatable is already existing?! Only once if the class is created !
	// The name of the data table is given by the name of the device number
	// Only data of one file can go to a table - it's not possible to merge at this level
	// Data table name:  Data_<module number><daq group number>_<module name><daqgroup>
	
	printf("\n");
	
	return 0;
}


void Mast::writeHeader() {
	struct timezone tz;
	const char *headerReadPtr;
	struct tm *tm_ptr;
	char time[20];
	char date[20];
	int n;
	std::string filename;
	int len; 
	

	if (fd_data <= 0)
		throw std::invalid_argument("Data file not open");
		
	// Read one sample data and replace the starting time by the original time
	len = this->lenHeader;
	
	// Read the template header
	if (datafileTemplate.length() == 0)
		throw std::invalid_argument("No template file given");	
	filename = configDir + datafileTemplate;
	printf("Reading header from template file %s\n", filename.c_str());
	readHeader(filename.c_str());
	headerReadPtr = (const char *) headerRaw;
	
	if (headerRaw == 0) {
		printf("Error: No template header found\n");
		throw std::invalid_argument("No template header found");
	}
	
	// Replace the start time by the actual time
	gettimeofday(&tRef, &tz);
	tm_ptr = gmtime(&tRef.tv_sec);
	sprintf(time, "%02d:%02d:%02d", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
	sprintf(date, "%02d.%02d.%4d", tm_ptr->tm_mday, tm_ptr->tm_mon+1, tm_ptr->tm_year+1900);
	
	replaceItem(&headerReadPtr, "Referenzzeit", time);
	replaceItem(&headerReadPtr, "Referenzdatum", date);
	//printf("%s\n", headerRaw);

	// Write 64k header
	n = write(fd_data, headerRaw, len);
	printf("Write header of %d bytes\n", n);
}


int Mast::parseData(char *line, struct timeval *l_tData, double *sensorValue) {
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


void Mast::updateDataSet(unsigned char *buf) {
	struct timeval tv;
	struct timezone tz;
	struct SPackedTime *time;
	float *local_sensorValue;
	struct tm *tm_zeit;
	int32_t *tickcount;
	char puffer[32];
	
	
	tickcount = (int32_t *) buf;
	local_sensorValue = (float *) (buf + 4);
	time = (struct SPackedTime *) (buf + 4 + 4 * nSensors);
	
	
	gettimeofday(&tv, &tz);
	
	tm_zeit = gmtime(&tv.tv_sec);
	
	time->nTag = tm_zeit->tm_mday;
	time->nMonat = tm_zeit->tm_mon + 1;
	time->nJahr = tm_zeit->tm_year + 1900;
	time->nStunde = tm_zeit->tm_hour;
	time->nMinute = tm_zeit->tm_min;
	time->nSekunde = tm_zeit->tm_sec;
	
	time->nHundertstel = tv.tv_usec / 10000;
	
	
	if (debug > 1) {
		printf("Tickcount: %12d Sensors: %f %f %f %f ",
		       *tickcount,  local_sensorValue[0], local_sensorValue[1], local_sensorValue[2], local_sensorValue[3]);
		strftime(puffer, 20, "%d.%m.%Y %T", tm_zeit);
		printf("%s,%d\n", puffer, time->nHundertstel);
	}
}
