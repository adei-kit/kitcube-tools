/***************************************************************************
                          wolkenkamera.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "wolkenkamera.h"


wolkenkamera::wolkenkamera():DAQBinaryDevice(){
	
	lenHeader = 0;
	lenDataSet = 237; // In the chm files is no check sum !?
	noData = 99999;
	
}


wolkenkamera::~wolkenkamera(){
}


void wolkenkamera::setConfigDefaults(){
}


const char *wolkenkamera::getDataDir(){
	char line[256];
	
	// TODO: evtl. an zu definierende Verzeichnisstruktur anpassen
	sprintf(line, "wolkenkamera/Daten/");
	buffer = line;
	
	return(buffer.c_str());
}


unsigned int wolkenkamera::getSensorGroup(){
	unsigned int number;
	
	number = 0;
	buffer = "";
	if (sensorGroup == "jpg") {
		number = 1;
		buffer = "standard";
	}
	
	return( number);
}


void wolkenkamera::readHeader(const char *filename){
	int fd;
	const char *headerReadPtr;
	char line[256];
	int n;
	int i;
	int heightOffset;
	char heightUnit[5];
	int len;
	
	
	if (sensorGroup == "nc"){	// read NetCDF file header here
		// Number of sensors
		nSensors = 14;
		printf("Number of sensors: %d\n", nSensors);
		
		// TODO: maybe better really read header of NetCDF file here,
		//       at least because of sensor height,
		//       but comments must be set manually here, due to mixed format of NetCDF file in this case!
		
		// List of sensors
		if (sensor > 0 ) delete [] sensor;
		sensor = new struct sensorType [nSensors];
		
		sensor[0].comment = "number of laser pulses";
		sensor[0].data_format = "<scalar>";
		
		sensor[1].comment = "average time per record";
		sensor[1].data_format = "<scalar>";
		
		sensor[2].comment = "31 Bit ServiceCode";
		sensor[2].data_format = "<scalar>";
		
		sensor[3].comment = "transmission of optics";
		sensor[3].data_format = "<scalar>";
		
		sensor[4].comment = "internal temperature in K*10";
		sensor[4].data_format = "<scalar>";
		
		sensor[5].comment = "external temperature in K*10";
		sensor[5].data_format = "<scalar>";
		
		sensor[6].comment = "detector temperature in K*10";
		sensor[6].data_format = "<scalar>";
		
		sensor[7].comment = "Laser fife time";
		sensor[7].data_format = "<scalar>";
		
		sensor[8].comment = "laser quality index - 255 max";
		sensor[8].data_format = "<scalar>";
		
		sensor[9].comment = "quality of detector signal - 255 max";
		sensor[9].data_format = "<scalar>";
		
		sensor[10].comment = "Daylight correction factor";
		sensor[10].data_format = "<scalar>";
		
		sensor[11].comment = "NN1";
		sensor[11].data_format = "<scalar>";
		
		sensor[12].comment = "Standard Deviation raw signal";
		sensor[12].data_format = "<scalar>";
		
		sensor[13].comment = "Profil";
		sensor[13].type = "profile";
		sensor[13].data_format = "<profile size=\"1024\"> <height unit=\"m\">";
		for (i = 1; i < 1024; i++) {	// TODO: read length from file?
			sprintf(line, "%i ", (i * 15));	// TODO: read values from data file!
			sensor[13].data_format += line;
		}
		sensor[13].data_format += "15360</height> </profile>";
		
		for (i = 0; i < nSensors; i++) {
			sensor[i].height = 110;	// TODO: read sensor height from data file!
		}
	} else {
		
		profile_length = 0;	// TODO/FIXME: do we need this here? this is done in daqdevice constructor, too
		
		// Number of sensors
		nSensors = 8;
		printf("Number of sensors %d\n", nSensors);
		
		// List of sensors
		if (sensor > 0 ) delete [] sensor;
		sensor = new struct sensorType [nSensors];
		
		sensor[0].comment = "Cloud level 1";
		sensor[1].comment = "Cloud level 2";
		sensor[2].comment = "Cloud level 3";
		sensor[3].comment = "Penetration depth 1";
		sensor[4].comment = "Penetration depth 2";
		sensor[5].comment = "Penetration depth 3";
		sensor[6].comment = "Vertical visibility";
		sensor[7].comment = "Detection range";
		
		for (i = 0; i < nSensors; i++) {
			sensor[i].height = heightOffset;
			printf("Sensor %3d: %s, %.1f %s\n", i+1, sensor[i].comment.c_str(), sensor[i].height, heightUnit);
		}
	}
}


void wolkenkamera::writeHeader(){
	// Nothing to do -- there is no header
}


// TODO: Move the parsing part to separate functions and move rest to base class
void wolkenkamera::readData(const char *dir, const char *filename){
	//int j;
	std::string timeString;
	std::string dateString;
	FILE *fmark;
	std::string filenameMarker;
	std::string filenameData;
	struct timeval lastTime;
	unsigned long lastPos;
	unsigned long lastIndex;
	//struct timeval tWrite;
	char line[256];
	
#ifdef USE_MYSQL
	struct timeval t0, t1;
	struct timezone tz;
	//int i;
	//MYSQL_RES *res;
	//MYSQL_RES *resTables;
	//MYSQL_ROW row;
	//MYSQL_ROW table;
	std::string tableName;
	std::string sql;
	char sData[50];
#endif
	
	if (debug >= 1)
		printf("_____wolkenkamera::readData(const char *dir, const char *filename)_____\n");
	
	// Compile file name
	filenameData = dir;
	filenameData += filename;
	//printf("<%s> <%s> <%s>\n", dir, filename, filenameData.c_str());
	
	
	// If number of sensors is unknown read the header first
	if (nSensors == 0)
		readHeader(filenameData.c_str());
	if (sensor[0].name.length() == 0)
		getSensorNames(sensorListfile.c_str());
	
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

	// Get the last time stamp + file pointer from
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dir, moduleNumber, sensorGroupNumber);
	filenameMarker = line;
	if (debug > 1) printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
		fclose(fmark);
	}
	
	
	// if file already read, return
	if (lastPos == 1) {
		fd_eof = true;
		printf("File already read -> nothing to do, returning...\n");
		return;
	}
		
		
	// convert all timestamps into timeval structure
	struct timeval* time_stamp_data;
	time_stamp_data = new struct timeval[no_vals];
	for (int i = 0; i < no_vals; i++) {
		time_stamp_data[i].tv_sec = (int)time_stamps[i];
		time_stamp_data[i].tv_usec = (int)((time_stamps[i] - (double)time_stamp_data[i].tv_sec) * 1000000.);	// TODO: maybe use floor() here
	}
	
	
	// write data to DB
#ifdef USE_MYSQL
	if (db > 0){
		// Write dataset to database
		// Store in the order of appearance
		//printf("Write record to database\n");
		sql = "INSERT INTO `";
		sql += dataTableName + "` (`usec`";
		for (int j = 0; j < nSensors; j++) {
			sql += ",`";
			sql += sensor[j].name;
			sql += "`";
		}
		sql += ") VALUES (";
		sprintf(sData, "%ld", time_stamp_data[i].tv_sec * 1000000 + time_stamp_data[i].tv_usec);
		sql += sData;
		for (int j = 0; j < (nSensors - 1); j++) {
			sprintf(sData, "%f", sensor_values[j][i]);
			sql += ",";
			sql += sData;
		}
		sql += ",'";
		// create profile data here
		for (int k = 0; k < dimensions[1]; k++) {
			sprintf(sData, "%f, ", values_2d[(i * dimensions[1]) + k]);
			sql += sData;
		}
		sql += "')";
		
		//printf("SQL: %s (db = %d)\n", sql.c_str(), db);
		
		gettimeofday(&t0, &tz);
		
		if (mysql_query(db, sql.c_str())){
			fprintf(stderr, "%s\n", sql.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			// If this operation fails do not proceed in the file?!
			printf("Error: Unable to write data to database\n");
			throw std::invalid_argument("Writing data failed");
			break;
		}
		
		gettimeofday(&t1, &tz);
		
		if (debug >= 1)
			printf("DB insert duration: %ld.%ldus\n", t1.tv_sec - t0.tv_sec, t1.tv_usec - t0.tv_usec);
	} else {
		printf("Error: No database availabe\n");
		throw std::invalid_argument("No database");
	}
#endif // of USE_MYSQL
	
	
	printf("fertig\n");
	lastPos = 1;	// file read
	fd_eof = true;
	

	// Write the last valid time stamp / file position
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, lastTime.tv_sec, lastTime.tv_usec, lastPos);
		fclose(fmark);
	}
}


void wolkenkamera::updateDataSet(unsigned char *buf){
	struct timeval t;
	struct timezone tz;
	struct tm *time;
	
	
	// Calculate the interval
	gettimeofday(&t, &tz);
	time = gmtime(&t.tv_sec);
	
	
	// Analyse time stamp, if new day a new file needs to generated
	if (this->filename != getDataFilename()) {
		openNewFile();
	}
	
	// Compile the data set for writing to the data file
	if (debug > 2) printf("Line: %s", buf);
	sprintf((char *) buf+12,"%02d.%02d.%02d;%02d:%02d",
			time->tm_mday, time->tm_mon+1, time->tm_year-100,
			time->tm_hour, time->tm_min);
	buf[26]=';';
	sprintf((char *) buf+233,"%02d", time->tm_sec);
	buf[235]=';';
	if (debug > 2) printf("Line: %s", buf);
	
	if (debug > 1) printf("%02d.%02d.%02d  %02d:%02d:%02d\n",
						  time->tm_mday, time->tm_mon+1, time->tm_year-100,
						  time->tm_hour, time->tm_min, time->tm_sec);
}
