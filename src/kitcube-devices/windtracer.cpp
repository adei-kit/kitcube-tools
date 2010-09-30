/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Mon Aug 16 16:17:30 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#include "windtracer.h"


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


void windtracer::readHeader(const char *filename){
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
	// read the configuration record, each file starts with this!
	//----------------------------------------------------------------------
	
	// read record header
	n = read(fd, &record_header, sizeof(record_header));	// FIXME: that's dangerous, because the order of the struct components is NOT fixed
	if (n == -1) {
		printf("Error in read function!!!\n");
		// TODO: error handling
	} else if (n == sizeof(record_header)) {
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
		// file not completly transfered, try again
		return;
	}
	
	// read the configuration record block descriptor
	n = read(fd, &config_record.block_desc, sizeof(struct BlockDescriptor));	// FIXME: that's dangerous, because the order of the struct components is NOT fixed
	if (n == -1) {
		printf("Error in read function!!!\n");
		// TODO: error handling
	} else if (n == sizeof(struct BlockDescriptor)) {
		if (config_record.block_desc.nId == CONFIG_DATA_BLOCK_ID) {
			if (debug >= 3) {
				printf("Block ID: %#X\n", config_record.block_desc.nId);
				printf("Block length: %d\n", config_record.block_desc.nBlockLength);
			}
		} else {
			printf("Error: wrong data block ID at beginning of file!\n");
			// TODO: error handling
		}
	} else {
		// file not completly transfered, try again
		return;
	}
	
	// get memory for config text block
	config_text_block_length = config_record.block_desc.nBlockLength - sizeof(struct BlockDescriptor);
	if (debug >= 3)
		printf("Config text block length: %d\n", config_text_block_length);
	config_record.chConfiguration = new char [config_text_block_length];
	
	// read the configuration text block
	n = read (fd, config_record.chConfiguration, config_text_block_length);
	if (n == -1) {
		printf("Error in read function!!!\n");
		// TODO: error handling
	} else if (n == config_text_block_length) {
		if (debug >= 3)
			printf("Content:\n%s", config_record.chConfiguration);
	} else {
		// file not completly transfered, try again
		return;
	}
	
	close(fd);

	// store header length
	// do NOT do this earlier to indicate the complete header could be read sucessfully
	lenHeader = record_header.nRecordLength;
	
	
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
	
	ptr = strstr(config_record.chConfiguration, "P_MONITOR_FFT_SIZE");
	monitor_fft_size = atoi(ptr + sizeof("P_MONITOR_FFT_SIZE"));
	
	if (debug >= 3) {
		printf("Range gates: %d\n", range_gates);
		printf("Sample frequency: %f\n", sample_frequency);
		printf("Raw data offset meters: %f\n", raw_data_offset_meters);
		printf("Raw data sample count: %d\n", raw_data_sample_count);
		printf("Samples per gate: %d\n", samples_per_gate);
		printf("Gates to merge: %d\n", gates_to_merge);
		printf("Monitor FFT size: %d\n", monitor_fft_size);
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
	
	nSensors = 1;
	
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
	int loop_counter = 0;
	unsigned long record_header_position;
	ssize_t n;
	struct RecordHeader record_header;
	struct ScanInfo scan_info;
	struct ProductPulseInfo pulse_info;
	float *velocity = 0, *snr = 0, *spectral_width = 0, *backscatter = 0, *spectral_data = 0;
	struct BlockDescriptor block_desc;
	
	
	if(debug >= 1)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// Compile file name
	full_data_filename = dir;
	full_data_filename += filename;


	// If number of sensors is unknown read the header first
	if (nSensors == 0)
		readHeader(full_data_filename.c_str());
	
	// if header could not be read, leave function here
	if (lenHeader == 0) {
		fd_eof = true;
		return;
	}
	
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
	if (fd_data_file == -1) {
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
	
	// get memory for the data
	// TODO: check for success!!!
	velocity = new float [range_gates];
	snr = new float [range_gates];
	spectral_width = new float [range_gates];
	backscatter = new float [range_gates];
	spectral_data = new float [monitor_fft_size/2];
	
	//----------------------------------------------------------------------
	// read data
	//----------------------------------------------------------------------
	fd_eof = false;
	while (loop_counter < 100) {
		//--------------------------------------------------------------
		// loop over data records
		//--------------------------------------------------------------
		
		record_header_position = current_position;
		
		// read record header
		n = read(fd_data_file, &record_header, sizeof(record_header));	// FIXME: that's dangerous, because the order of the struct components is NOT fixed
		if (n < sizeof(record_header)) {
			// file not completely transfered, try again next time
			fd_eof = true;
			
			break;	// leave outer while loop over data records
		}
		current_position += sizeof(record_header);
		
		// check record ID
		switch (record_header.block_desc.nId) {
		case PRODUCT_VELOCITY_RECORD_ID:
			// read scan info data block
			n = read(fd_data_file, &scan_info, sizeof(scan_info));
			if (n < sizeof(scan_info)) {
				// file not completley transfered, try again next time
				fd_eof = true;
				
				break;	// leave switch for record ID
			}
			current_position += sizeof(scan_info);
			
			// read product pulse info
			n = read(fd_data_file, &pulse_info, sizeof(pulse_info));
			if (n < sizeof(pulse_info)) {
				// file not completely transfered, try again next time
				fd_eof = true;
				
				break;	// leave switch for record ID
			}
			current_position += sizeof(pulse_info);
			
			// read data, solange man nicht ausserhalb der record length ist
			while (current_position < record_header_position + record_header.nRecordLength) {
				//--------------------------------------
				// loop over data blocks
				//--------------------------------------
				
				// read block descriptor
				n = read(fd_data_file, &block_desc, sizeof(block_desc));
				if (n < sizeof(block_desc)) {
					// file not completely transfered, try again next time
					fd_eof = true;
					
					break;	// leave inner while loop over data blocks
				}
				
				// check data block ID
				switch (block_desc.nId) {
				case PRODUCT_VELOCITY_DATA_BLOCK_ID:
					n = read(fd_data_file, velocity, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completely transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_SNR_DATA_BLOCK_ID:
					n = read(fd_data_file, snr, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completly transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_SPECTRAL_WIDTH_DATA_BLOCK_ID:
					n = read(fd_data_file, spectral_width, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completly transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_BACKSCATTER_DATA_BLOCK_ID:
					n = read(fd_data_file, backscatter, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completly transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_MONITOR_SPECTRAL_DATA_BLOCK_ID:
					n = read(fd_data_file, spectral_data, monitor_fft_size / 2 * sizeof(float));
					if (n < monitor_fft_size / 2 * sizeof(float))
						// file not completly transfered, try again
						fd_eof = true;
					
					break;
					
				default:
					lseek(fd_data_file, block_desc.nBlockLength - sizeof(block_desc), SEEK_CUR);
					
					break;
				}
				
				if (fd_eof == true)
					break;	// leave inner while loop over data blocks
				
				current_position += block_desc.nBlockLength;
			}
			
			break;
			
		case PRODUCT_FILTERED_VELOCITY_RECORD_ID:
			// read scan info data block
			n = read(fd_data_file, &scan_info, sizeof(scan_info));
			if (n < sizeof(scan_info)) {
				// file not completley transfered, try again next time
				fd_eof = true;
				
				break;	// leave switch for record ID
			}
			current_position += sizeof(scan_info);
			
			// read product pulse info
			n = read(fd_data_file, &pulse_info, sizeof(pulse_info));
			if (n < sizeof(pulse_info)) {
				// file not completely transfered, try again next time
				fd_eof = true;
				
				break;	// leave switch for record ID
			}
			current_position += sizeof(pulse_info);
			
			// read data
			while (current_position < record_header_position + record_header.nRecordLength) {
				//--------------------------------------
				// loop over data blocks
				//--------------------------------------
				
				// read block descriptor
				n = read(fd_data_file, &block_desc, sizeof(block_desc));
				if (n < sizeof(block_desc)) {
					// file not completely transfered, try again next time
					fd_eof = true;
					
					break;	// leave inner while loop over data blocks
				}
				
				// check data block ID
				switch (block_desc.nId) {
				case PRODUCT_FILTERED_VELOCITY_DATA_BLOCK_ID:
					n = read(fd_data_file, velocity, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completely transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_FILTERED_SNR_DATA_BLOCK_ID:
					n = read(fd_data_file, snr, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completly transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_FILTERED_SPECTRAL_WIDTH_DATA_BLOCK_ID:
					n = read(fd_data_file, spectral_width, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completly transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_FILTERED_BACKSCATTER_DATA_BLOCK_ID:
					n = read(fd_data_file, backscatter, range_gates * sizeof(float));
					if (n < range_gates * sizeof(float))
						// file not completly transfered, try again
						fd_eof = true;
					
					break;
					
				case PRODUCT_MONITOR_SPECTRAL_DATA_BLOCK_ID:
					printf("PRODUCT_MONITOR_SPECTRAL_DATA_BLOCK_ID found at %ld B\n", current_position);
					
					//break;
					
				default:
					lseek(fd_data_file, block_desc.nBlockLength - sizeof(block_desc), SEEK_CUR);
					
					break;
				}
				
				if (fd_eof == true)
					break;	// leave inner while loop over data blocks
				
				current_position += block_desc.nBlockLength;
			}
			
			break;
			
		default:
			printf("No PRODUCT_VELOCITY_RECORD_ID or PRODUCT_FILTERED_VELOCITY_RECORD_ID found -> ignoring\n");
			lseek(fd_data_file, record_header.nRecordLength - record_header.block_desc.nBlockLength, SEEK_CUR);
			current_position += record_header.nRecordLength - record_header.block_desc.nBlockLength;
			
			break;
		}
		
		if (fd_eof == true)
			break;	// leave outer while loop over data records
		

		if (debug >= 1)
			printf("Timestamp: %02d.%02d.%d, %02d:%02d:%02d,%09d\n",
				record_header.nDayOfMonth, record_header.nMonth, record_header.nYear,
				record_header.nHour, record_header.nMinute, record_header.nSecond, record_header.nNanosecond);
		
		//--------------------------------------------------------------
		// TODO: store to DB
		//--------------------------------------------------------------
		
		// save position in file
		fmark = fopen(filenameMarker.c_str(), "w");
		if (fmark > 0) {
			fprintf(fmark, "%ld %ld %ld %ld\n",
				lastIndex, last_data_timestamp.tv_sec, last_data_timestamp.tv_usec, current_position);
			fclose(fmark);
		}
		
		loop_counter++;
	}
	
	delete [] velocity;
	velocity = 0;
	
	delete [] snr;
	snr = 0;
	
	delete [] spectral_width;
	spectral_width = 0;
	
	delete [] backscatter;
	backscatter = 0;
	
	delete [] spectral_data;
	spectral_data = 0;
	
	close(fd_data_file);
}