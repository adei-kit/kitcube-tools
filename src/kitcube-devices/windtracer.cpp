/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Mon Aug 16 16:17:30 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#include "windtracer.h"


struct SPackedTime{
	unsigned char nTag;
	unsigned char nMonat;
	unsigned short nJahr;
	unsigned char nStunde;
	unsigned char nMinute;
	unsigned char nSekunde;
	unsigned char nHundertstel;
};


struct BlockDescriptor {
	u_int16_t nId;
	u_int16_t nVersion;
	u_int32_t nBlockLength;
};


/* RecordHeader contains record time stamp and the length of the entire record */
struct RecordHeader {
	struct BlockDescriptor block_desc;
	u_int32_t nRecordLength;
	u_int16_t nYear;
	u_int8_t nMonth;
	u_int8_t nDayOfMonth;
	u_int16_t nHour;
	u_int8_t nMinute;
	u_int8_t nSecond;
	u_int32_t nNanosecond;
};


// Configuration text block is always the first record of any datafile and contains
// information about the configuration of the system
struct ConfigurationRecord {
	struct BlockDescriptor block_desc;
	char *chConfiguration;
};


// Scan Info Block provides information about the scanner, it appears before each Raw Data Record and before a Product Data Record
struct ScanInfo {
	struct BlockDescriptor block_desc;
	float fScanAzimuth_deg;
	float fScanElevation_deg;
	float fAzimuthRate_dps;
	float fElevationRate_dps;
	float fTargetAzimuth_deg;
	float fTargetElevation_deg;
	int32_t nScanEnabled;
	int32_t nCurrentIndex;
	int32_t nAcqScanState;
	int32_t nDriverScanState;
	int32_t nAcqDwellState;
	int32_t nScanPatternType;
	u_int32_t nValidPos;
	int32_t nSSDoneState;
	u_int32_t nErrorFlags;
};


// ProductPulseInfo appears before Product Data Blocks and contains information about the pulse used for that products
struct ProductPulseInfo {
	struct BlockDescriptor block_desc;
	float fAzimuthMin_deg;
	float fAzimuthMean_deg;
	float fAzimuthMax_deg;
	float fElevationMin_deg;
	float fElevationMean_deg;
	float fElevationMax_deg;
	float fMonitorCount;
	float fMonitorFrequency_hz;
	float fMonitorTime;
	float fMonitorPeak;
	float fOverLevel;
	float fUnderLevel;
};



windtracer::windtracer(): DAQBinaryDevice(){
	
	headerRaw = 0;
	
	noData = -9999;
}


windtracer::~windtracer(){
	// Free header memory
	if (headerRaw > 0) delete [] headerRaw;
	
}


void windtracer::setConfigDefaults(){
	
}


const char *windtracer::getDataFilename(){
	char line[256];
	
	// TODO: Create a single source for the filename convention...
	sprintf(line, "M%02d_%02ld.%s", moduleNumber, fileIndex, sensorGroup.c_str());
	buffer = line;
	
	return(buffer.c_str());
}


// Move to base class?
void windtracer::replaceItem(const char **header, const char *itemTag, const char *newValue){
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


const char *windtracer::getStringItem(const char **header, const char *itemTag){
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


int windtracer::getNumericItem(const char **header, const char *itemTag){
	int value;
	const char *ptr;
	
	ptr = getStringItem(header, itemTag);
	value = atoi(ptr);
	
	return(value);
}


unsigned int windtracer::getSensorGroup(){
	unsigned int number;

	
	if (debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	number = 0;
	buffer = "";
	
	if (sensorGroup == "data") {
		number = 1;
		buffer = "base data";
	}
	
	return( number);
}


const char *windtracer::getSensorName(const char *longName, unsigned long *aggregation){
	const char *ptr;
	unsigned long type;
	
	
	buffer = longName;
	type = 0;
	
	ptr = strstr(longName, "mittel");
	if (ptr > 0){
		std::string result(longName, ptr - longName);
		buffer = result;
		type = 1;
	}
	
	ptr = strstr(longName, "max");
	if (ptr > 0){
		std::string result(longName, ptr - longName);
		buffer = result;
		type = 2;
	}
	
	ptr = strstr(longName, "min");
	if (ptr > 0){
		std::string result(longName, ptr - longName);
		buffer = result;
		type = 3;
	}
	
	ptr = strstr(longName, "sigma");
	if (ptr > 0){
		std::string result(longName, ptr - longName);
		buffer = result;
		type = 4;
	}
	
	
	if (aggregation > 0)
		*aggregation = type;
	
	// Remove with characters at the end of the name
	if (buffer.at(buffer.length()-1) == ' ')
		buffer.erase(buffer.length()-1, buffer.length()-1);
	
	//buffer.erase(buffer.end(),buffer.end());
	return(buffer.c_str());
}


void windtracer:: readHeader(const char *filename){
	int fd;
	char line[256];
	int n;
	struct RecordHeader record_header;
	struct ConfigurationRecord config_record;
	u_int32_t config_text_block_length;
	char *ptr;
	int raw_data_sample_count, samples_per_gate, gates_to_merge;
	int samples_between_gate_centers;
	double sample_frequency, raw_data_offset_meters, range_per_sample, range_between_gate_centers;
	double range_per_gate, first_range, corrected_first_range;
	double *range_gate_center, *range_gate_start, *range_gate_end;
	
	
	// ID of the device
	// NOT in the header
	// Get it from the filename?!
	// --> use parser function in reader...
	
	if(debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		sprintf(line, "Error opening file %s", filename);
		throw std::invalid_argument(line);
	}
	
	
	//----------------------------------------------------------------------
	// read the configuration record
	//----------------------------------------------------------------------
	
	// read record header, always 24 bytes long
	n = read(fd, &record_header, 24);	// FIXME: that's dangerous, because the order of the struct components is NOT fixed
	if (n == -1) {
		printf("Error in read function!!!\n");
		// TODO: error handling
	} else if (n == 24) {
		if (record_header.block_desc.nId == CONFIG_RECORD_ID) {
			if (debug >= 3) {
				printf("Record ID: %#X\n", record_header.block_desc.nId);
				printf("Record header length: %d\n", record_header.block_desc.nBlockLength);
				printf("Record length total: %d\n", record_header.nRecordLength);
				printf("Timestamp: %02d.%02d.%d, %02d:%02d:%02d,%d\n",
				record_header.nDayOfMonth, record_header.nMonth, record_header.nYear,
				record_header.nHour, record_header.nMinute, record_header.nSecond, record_header.nNanosecond);
			}
		} else {
			printf("Error: wrong record ID at beginning of file!\n");
			// TODO: error handling
		}
	} else {
		// TODO: file not completly transfered, try again
	}
	
	// store header length
	lenHeader = record_header.nRecordLength;
	
	// read the configuration record block descriptor, always 8 bytes long
	n = read (fd, &config_record.block_desc, 8);	// FIXME: that's dangerous, because the order of the struct components is NOT fixed
	if (n == -1) {
		printf("Error in read function!!!\n");
		// TODO: error handling
	} else if (n < 8) {
		// TODO: file not completly transfered, try again
	}
	
	// get memory for config text block
	config_text_block_length = config_record.block_desc.nBlockLength - 8;
	if (debug >= 3)
		printf("Config text block length: %d\n", config_text_block_length);
	config_record.chConfiguration = new char [config_text_block_length];
	
	// read the configuration text block
	n = read (fd, config_record.chConfiguration, config_text_block_length);
	if (n == -1) {
		printf("Error in read function!!!\n");
		// TODO: error handling
	} else if (n == config_text_block_length) {
		if (config_record.block_desc.nId == CONFIG_DATA_BLOCK_ID) {
			if (debug >= 3) {
				printf("Block ID: %#X\n", config_record.block_desc.nId);
				printf("Block length: %d\n", config_record.block_desc.nBlockLength);
				printf("Content: %s", config_record.chConfiguration);
			}
		} else {
			printf("Error: wrong record ID at beginning of file!\n");
		}
	} else {
		// TODO: file not completly transfered, try again
	}
	
	close(fd);

	
	//----------------------------------------------------------------------
	// get some variables from the config text block
	//----------------------------------------------------------------------
	ptr = strstr(config_record.chConfiguration, "P_RANGE_GATES");
	range_gates = atoi(ptr + sizeof("P_RANGE_GATES"));
	
	ptr = strstr(config_record.chConfiguration, "Q_SAMPLING_RATE");
	sample_frequency = atof(ptr + sizeof("Q_SAMPLING_RATE"));
	
	ptr = strstr(config_record.chConfiguration, "Q_RAW_DATA_OFFSET_METERS");
	raw_data_offset_meters = atof(ptr + sizeof("Q_RAW_DATA_OFFSET_METERS"));
	
	ptr = strstr(config_record.chConfiguration, "Q_RAW_SIGNAL_BLOCK_SAMPLE_COUNT");
	raw_data_sample_count = atoi(ptr + sizeof("Q_RAW_SIGNAL_BLOCK_SAMPLE_COUNT"));
	
	ptr = strstr(config_record.chConfiguration, "P_SAMPLES_PER_GATE");
	samples_per_gate = atoi(ptr + sizeof("P_SAMPLES_PER_GATE"));
	
	ptr = strstr(config_record.chConfiguration, "P_GATES_TO_MERGE");
	gates_to_merge = atoi(ptr + sizeof("P_GATES_TO_MERGE"));
	
	if (debug >= 3) {
		printf("Range gates: %d\n", range_gates);
		printf("Sample frequency: %f\n", sample_frequency);
		printf("Raw data offset meters: %f\n", raw_data_offset_meters);
		printf("Raw data sample count: %d\n", raw_data_sample_count);
		printf("Samples per gate: %d\n", samples_per_gate);
		printf("Gates to merge: %d\n", gates_to_merge);
	}
	
	
	//----------------------------------------------------------------------
	// calculate range gates
	//----------------------------------------------------------------------
	
	// calculate intermediate variables/values
	range_per_sample = 150000000. / sample_frequency;
	samples_between_gate_centers = (raw_data_sample_count - samples_per_gate) / (range_gates - 1);
	range_between_gate_centers = samples_between_gate_centers * range_per_sample;
	range_per_gate = samples_per_gate * range_per_sample;
	first_range = raw_data_offset_meters + (samples_per_gate / 2) * range_per_sample;
	corrected_first_range = first_range + ((gates_to_merge - 1) / 2) * range_per_gate;
	
	// get memory for range gates
	range_gate_start = new double [range_gates];
	range_gate_center = new double [range_gates];
	range_gate_end = new double [range_gates];
	
	// calculate range gates
	for (int i = 0; i < range_gates; i++) {
		range_gate_center[i] = corrected_first_range + range_between_gate_centers * i;
		
		range_gate_start[i] = range_gate_center[i] - range_per_gate / 2.;
		range_gate_end[i] = range_gate_center[i] + range_per_gate / 2.;
	}
	
	if (debug >= 3) {
		for (int i = 0; i < range_gates; i++) {
			printf("Range gate values (%d): %f %f %f\n",
			       i, range_gate_start[i], range_gate_center[i], range_gate_end[i]);
		}
	}
	
	printf("Hello world\n");
}


void windtracer::writeHeader(){
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


void windtracer::parseData(char *line, struct timeval *l_tData, double *sensorValue){
	float *local_sensorValue;
	struct SPackedTime *time;
	struct tm tm_zeit;

	
	if (sizeof(float) != 4) {
		printf("Size of 'float' is not 4! So not reading any data!\n");
		return;
	}
	
	// Data format:
	// long Tickcount : Hundertstel seit Messbeginn (wird nicht benutzt)
	// float Sensorwerte: Messdaten, Anzahl steht im Header
	// struct SPackedTime: Zeitstempel-Struktur
	
	local_sensorValue =  (float *)(line + 4);	// TODO/FIXME: that is dangerous, as you don't know the size of "float"
	time = (struct SPackedTime *)(line + 4 + 4 * nSensors);	// TODO/FIXME: that's dangerous, as the order of the struct components is NOT fixed
	
	
	// read data
	for (int i = 0; i < nSensors; i++) {
		sensorValue[i] = local_sensorValue[i];
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
	l_tData->tv_sec = timegm(&tm_zeit);
	l_tData->tv_usec = time->nHundertstel * 10000;
}


void windtracer::updateDataSet(unsigned char *buf){
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


void windtracer::readData(const char *dir, const char *filename)
{
	std::string full_data_filename;
	int fd_data_file;
	unsigned long last_position;
	struct timeval last_data_timestamp;
	char line[256];
	FILE *fmark;
	unsigned long lastIndex;
	unsigned long current_position;
	int n;
	struct RecordHeader record_header;
	struct ScanInfo scan_info;
	struct ProductPulseInfo pulse_info;
	float *velocity, *snr, *spectral_width, *backscatter;
	struct BlockDescriptor block_desc;

	
	if(debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// Compile file name
	full_data_filename = dir;
	full_data_filename += filename;


	// If number of sensors is unknown read the header first
	if (nSensors == 0)
		readHeader(full_data_filename.c_str());
//	if (sensor[0].name.length() == 0)
//		getSensorNames(sensorListfile.c_str());

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
	
	if (debug >= 1)
		printf("Open data file: %s\n", full_data_filename.c_str());
	fd_data_file = open(full_data_filename.c_str(), O_RDONLY);
	if (fd_data_file <= 0) {
		printf("Error opening data file %s\n", full_data_filename.c_str());
		return;
	}
	
	// Get the last time stamp + file pointer from
	last_position = 0;
	last_data_timestamp.tv_sec = 0;
	last_data_timestamp.tv_usec = 0;
	
	sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dir, moduleNumber, sensorGroupNumber);
	filenameMarker = line;
	if (debug >= 1)
		printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &last_data_timestamp.tv_sec, &last_data_timestamp.tv_usec, &last_position);
		fclose(fmark);
		
		if (debug >= 1)
			printf("Last time stamp was %ld\n", last_data_timestamp.tv_sec);
	}
	
	// Find the beginning of the new data
	if (last_position == 0)	// if new file, jump to the first dataset
		last_position = lenHeader;
	if (debug >= 1)
		printf("Last position in file: %ld\n", last_position);
	lseek(fd_data_file, last_position, SEEK_SET);
	
	current_position = last_position;
	
	
	//----------------------------------------------------------------------
	// read data
	//----------------------------------------------------------------------
	
	// read record header, always 24 bytes long
	n = read(fd_data_file, &record_header, 24);	// FIXME: that's dangerous, because the order of the struct components is NOT fixed
	if (n < 24) {
		// TODO: file not completly transfered, try again
	}
	current_position += 24;
	
	switch (record_header.block_desc.nId) {
		case PRODUCT_VELOCITY_RECORD_ID:
			// read scan info data block
			n = read(fd_data_file, &scan_info, sizeof(scan_info));
			if (n < sizeof(scan_info)) {
				// TODO: file not completly transfered, try again
			}
			current_position += sizeof(scan_info);
			
			// read product pulse info
			n = read(fd_data_file, &pulse_info, sizeof(pulse_info));
			if (n < sizeof(pulse_info)) {
				// TODO: file not completly transfered, try again
			}
			current_position += sizeof(pulse_info);
			
			// read data, solange man nicht ausserhalb der record length ist
			while (current_position < last_position + record_header.nRecordLength) {
				n = read(fd_data_file, &block_desc, sizeof(block_desc));
				if (n < sizeof(block_desc)) {
					// TODO: file not completly transfered, try again
				}
				
				switch (block_desc.nId) {
					case PRODUCT_VELOCITY_DATA_BLOCK_ID:
						velocity = new float [range_gates];
						n = read(fd_data_file, velocity, range_gates * sizeof(float));
						if (n < range_gates * sizeof(float)) {
							// TODO: file not completly transfered, try again
						}
						
						break;
					case PRODUCT_SNR_DATA_BLOCK_ID:
						snr = new float [range_gates];
						n = read(fd_data_file, snr, range_gates * sizeof(float));
						if (n < range_gates * sizeof(float)) {
							// TODO: file not completly transfered, try again
						}
						
						break;
					case PRODUCT_SPECTRAL_WIDTH_DATA_BLOCK_ID:
						spectral_width = new float [range_gates];
						n = read(fd_data_file, spectral_width, range_gates * sizeof(float));
						if (n < range_gates * sizeof(float)) {
							// TODO: file not completly transfered, try again
						}
						
						break;
					case PRODUCT_BACKSCATTER_DATA_BLOCK_ID:
						backscatter = new float [range_gates];
						n = read(fd_data_file, backscatter, range_gates * sizeof(float));
						if (n < range_gates * sizeof(float)) {
							// TODO: file not completly transfered, try again
						}
						
						break;
					default:
						lseek(fd_data_file, block_desc.nBlockLength - sizeof(block_desc), SEEK_CUR);
						
						break;
				}
				
				current_position += block_desc.nBlockLength;
			}
			
			break;
		case PRODUCT_FILTERED_VELOCITY_RECORD_ID:
			// read scan info data block
			n = read(fd_data_file, &scan_info, sizeof(scan_info));
			
			// read product pulse info
			n = read(fd_data_file, &pulse_info, sizeof(pulse_info));
			
			// read data
			
			
			break;
		default:
			printf("No PRODUCT_VELOCITY_RECORD_ID or PRODUCT_FILTERED_VELOCITY_RECORD_ID found -> ignoring\n");
			break;
	}
}