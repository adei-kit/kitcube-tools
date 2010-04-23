/***************************************************************************
                          daqdevice.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "daqdevice.h"


DAQDevice::DAQDevice(){
	
	debug = 0;
	
	moduleType = "Generic";
	moduleNumber = 0;
	this->sensorGroup = "txt";
	sensorGroupNumber = 0;
	
	this->iniGroup = "DAQDevice";
	
	lenHeader = 0;
	lenDataSet = 0;
	profile_length = 0;
	
	fileIndex = 0;
	nLine = 0;	
	filenameMarker = ".kitcube-data.marker";
	
	axis = 0;
	nAxis = 0;
	sensor = 0;
	nSensors = 0; // Unknown configuration
	sensorValue = 0;
	processedData = 0;
	
#ifdef USE_MYSQL
	db = 0;
#endif
	
}


DAQDevice::~DAQDevice(){
	FILE *fmark;
	
	// Save current position
	if (fileIndex != 0) {
		if (debug>2) printf("Closing data file, save file position in %s\n", filenameMarker.c_str());
		fmark = fopen(filenameMarker.c_str(), "w");
		if (fmark > 0) {
			fprintf(fmark, "%ld\t%d\n", fileIndex, nLine);
			fclose(fmark);
		} else {
			printf("Error writing marker file %s\n", filenameMarker.c_str());
		}
	}
	
	if (axis > 0) {
		delete [] axis;
		nAxis = 0;
		axis = 0;
	}
	if (sensor > 0 ) {
		delete [] sensor;
		nSensors = 0;
		sensor = 0;
	}
	if (sensorValue > 0) {
		delete [] sensorValue;
	    sensorValue = 0;
	}
	
}


int DAQDevice::getNSensors(){
	return(nSensors);
}

unsigned int DAQDevice::getProcessedData(){
	return(processedData);
}

void DAQDevice::setDebugLevel(int level){
	this->debug = level;
}


void DAQDevice::setConfigDefaults(){
	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
}


void DAQDevice::readInifile(const char *filename, const char *group){
	akInifile *ini;
	Inifile::result error;
	std::string value;
	char line[256];
	float tValue;
	std::string tUnit;
	
	printf("_____DAQDevice::readInifile()_____\n");
	
	//
	// Get the module number
	//
	this->inifile = filename;
	if (group > 0) this->iniGroup = group;
	
	//ini = new akInifile(inifile.c_str(), stdout);
	ini = new akInifile(inifile.c_str()); // No output ?!
	if (ini->Status()==Inifile::kSUCCESS){
		error = ini->SpecifyGroup(iniGroup.c_str());
		if (error == Inifile::kSUCCESS){
			this->moduleNumber = ini->GetFirstValue("moduleNumber", (int) moduleNumber, &error);
			this->sensorGroup = ini->GetFirstString("sensorGroup", sensorGroup.c_str(), &error);
			this->sensorGroupNumber = getSensorGroup();
		}
	}
	delete ini;
	
	//
	// Set other module number dependant parameters
	//
	this->project = "kitcube";
	this->dbHost = "localhost";
	this->dbUser = "root";
	this->dbPassword = "";
	//this->dbName = project + "_active"; // s. below
	this->sensorTableName = "Sensorlist";
	this->axisTableName = "Axislist";
	this->moduleTableName = "ModuleList";
	
	this->configDir = "./";
	this->dataDir = "./data";
	this->remoteDir = "../kitcube-data/data";
	this->archiveDir = "./data";
	this->rsyncArgs = "";
	
	// Use module number in template name
	this->moduleName = "GEN";
	this->moduleComment = "Generic";
	
	sprintf(line, "Generic.%s.template", sensorGroup.c_str());
	this->datafileTemplate = line;
	sprintf(line, "Data_<index>.%s", sensorGroup.c_str());
	this->datafileMask = line;
	//printf("datafileMask = %s\n", datafileMask.c_str());
	
	sprintf(line, "Generic.%s.sensors", sensorGroup.c_str());
	this->sensorListfile = line;
	
	this->tSample = 1000; // ms
	
	
	// Override default values by module specific implementation
	setConfigDefaults();
	
	
	// Read from inifile
	// The default group from the class implementation can be overwritten by the function call?!
	if (group > 0) this->iniGroup = group;
	
	
	if (debug > 2) {
		ini = new akInifile(inifile.c_str(), stdout);
	} else {
		ini = new akInifile(inifile.c_str()); // Skip output
	}
	if (ini->Status()==Inifile::kSUCCESS){
		
		// Read global parameters
		// The parameters should be the same for the whole system
		if (debug > 2) printf("[Common]\n");
		ini->SpecifyGroup("Common");
		this->project = ini->GetFirstString("project", project.c_str(), &error);
		this->dbHost = ini->GetFirstString("dbHost", dbHost.c_str(), &error);
		
		this->dbName = project + "_active";
		this->dbName = ini->GetFirstString("dbName", dbName.c_str(), &error);
		this->dbUser = ini->GetFirstString("dbUser", dbUser.c_str(), &error);
		this->dbPassword = ini->GetFirstString("dbPassword", dbPassword.c_str(), &error);
		
		this->configDir = ini->GetFirstString("configDir", configDir.c_str(), &error);
		this->dataDir = ini->GetFirstString("dataDir", dataDir.c_str(), &error);
		this->remoteDir = ini->GetFirstString("remoteDir", remoteDir.c_str(), &error);
		this->archiveDir = ini->GetFirstString("archiveDir", archiveDir.c_str(), &error);
		this->rsyncArgs = ini->GetFirstString("rsyncArgs", rsyncArgs.c_str(), &error);
		while (error != Inifile::kFAIL) {
			this->rsyncArgs += " ";
			this->rsyncArgs += ini->GetNextString("", &error);
		}
		
		// Add module number to dataDir + archiveDir
		if (dataDir.at(dataDir.length()-1) != '/') dataDir += "/";
		if (archiveDir.at(archiveDir.length()-1) != '/') archiveDir += "/";
		if (remoteDir.at(remoteDir.length()-1) != '/') remoteDir += "/";
		sprintf(line, "%03d/", moduleNumber);
		this->dataDir += line;
		this->remoteDir += line;
		this->archiveDir += line;
		
		
		if (debug > 2) printf("[%s]\n", iniGroup.c_str());
		// Module specific parameters
		ini->SpecifyGroup(iniGroup.c_str());
		this->moduleNumber = ini->GetFirstValue("moduleNumber", (int) moduleNumber, &error);
		this->sensorGroup = ini->GetFirstString("sensorGroup", sensorGroup.c_str(), &error);
		this->sensorGroupNumber = getSensorGroup();
		
		this->moduleName = ini->GetFirstString("moduleName", moduleName.c_str(), &error);
		this->moduleComment= ini->GetFirstString("moduleComment", moduleComment.c_str(), &error);
		this->datafileTemplate = ini->GetFirstString("datafileTemplate", datafileTemplate.c_str(), &error);
		tValue= ini->GetFirstValue("samplingTime", (float) tSample, &error);
		tUnit = ini->GetNextString("ms", &error);
		this->tSample = tValue;
		if ((tUnit == "sec") || (tUnit == "s")) this->tSample = tValue * 1000;
		if (tUnit == "min") this->tSample = tValue * 60000;
		
		
		// Global parameters can be overwritten by the module settings
		this->configDir = ini->GetFirstString("configDir", configDir.c_str(), &error);
		this->dataDir = ini->GetFirstString("dataDir", dataDir.c_str(), &error);
		this->remoteDir = ini->GetFirstString("remoteDir", remoteDir.c_str(), &error);
		this->archiveDir = ini->GetFirstString("archiveDir", archiveDir.c_str(), &error);
		this->datafileMask = ini->GetFirstString("datafileMask", datafileMask.c_str(), &error);
		this->sensorListfile = ini->GetFirstString("sensorList", sensorListfile.c_str(), &error);
		
	}
	delete ini;
	
	
	// Add slash at the end of the directories
	//printf("ConfigDir: %c\n", configDir.at(configDir.length()-1));
	if (configDir.at(configDir.length()-1) != '/') configDir += "/";
	if (dataDir.at(dataDir.length()-1) != '/') dataDir += "/";
	if (remoteDir.at(remoteDir.length()-1) != '/') remoteDir += "/";
	if (archiveDir.at(archiveDir.length()-1) != '/') archiveDir += "/";
	
	
	// Check integrity of data fields
	if (datafileMask.find("<index>") == std::string::npos){
		printf("There is no tag <index> in datafileMask=%s specified in inifile %s\n",
			   datafileMask.c_str(), inifile.c_str());
		throw std::invalid_argument("Missing <index> in datafileMask");
	}
	
	
	// Display configuration for one module
	if (debug > 2) {
		printf("Module: %s (%s, %d) Sensor group %s (%d)\n",
			   moduleName.c_str(), moduleComment.c_str(), moduleNumber,
			   sensorGroup.c_str(), sensorGroupNumber);
	}
}


void DAQDevice::readAxis(const char *inifile){
	akInifile *ini;
	Inifile::result error;
	std::string value;
	int i;
	std::string item;
	
	
	if (debug > 3) printf("______DAQDevice::readAxis()______________________\n");
	
	//ini = new akInifile(inifile, stdout);
	ini = new akInifile(inifile);
	if (ini->Status()==Inifile::kSUCCESS){
		
		error = ini->SpecifyGroup("Axis");
		
		if (error == Inifile::kSUCCESS){
			
			// Get the number of defined axis
			nAxis = 0;
			ini->GetFirstString("axis", 0, &error);
			while (error == Inifile::kSUCCESS) {
				ini->GetNextString(0, &error);
				nAxis++;
			}
			if (debug > 3) printf("Number of axis: %d\n", nAxis);
			
			// Allocate axis definition
			if (axis > 0) delete [] axis;
			axis = new struct axisDef [nAxis];
			
			// Read the axis short names
			axis[0].name = ini->GetFirstString("axis", 0, &error);
			for (i = 1; i < nAxis; i++) {
				axis[i].name = ini->GetNextString(0, &error);
			}
			
			// Read description and units
			for (i = 0; i < nAxis; i++) {
				item = axis[i].name + "_name";
				axis[i].desc = ini->GetFirstString(item.c_str(), 0, &error);
				item = axis[i].name + "_unit";
				axis[i].unit = ini->GetFirstString(item.c_str(), 0, &error);
				if (debug > 3) printf("%s - %s (%s)\n", axis[i].name.c_str(), axis[i].desc.c_str(), axis[i].unit.c_str());
				
				axis[i].isNew = true;
			}
		}
	}
	delete ini;
}


void DAQDevice::getSensorNames(const char *sensorListfile){
	FILE *flist;
	int i, j;
	char line[256];
	char *n;
	char *namePtr;
	char name[50];
	//unsigned int pos;
	std::string axisName;
	std::string sLine;
	
	
	if (debug > 3) printf("______DAQDevice::getSensorNames()______________________\n");
	
	// Will need the axis definition for parsing the sensor names
	if (axis == 0) readAxis(this->inifile.c_str());
	
	
	sprintf(line, "%s%s", configDir.c_str(), sensorListfile);
	if (debug > 3) printf("Read sensor names from list %s\n", line);
	
	// Open list file
	flist = fopen(line, "r");
	if (flist == 0){
		printf("Configuration file with sensor list is missing\n");
		printf("Creating template - fill in the sensor names in %s\n", line);
		
		flist = fopen(line, "w");
		if (flist < 0){
			throw std::invalid_argument("Error creating template file");
		}
		for (i = 0; i < nSensors; i++) {
			if (sensor[i].longComment.length() == 0){
				fprintf(flist,"%10d\t%s\t\n", i+1, sensor[i].comment.c_str());
			} else {
				fprintf(flist,"%10d\t%s\t\n", i+1, sensor[i].longComment.c_str());
			}
		}
		fclose(flist);
		
		throw std::invalid_argument("Fill in sensor names in template file");
	}
	
	n = line;
	i = 0;
	while ((n > 0) && (i < nSensors)) {
		// Read names, compare with the names in the data base?!
		n = fgets(line, 256, flist);
		if (n > 0) {
			if (debug > 4) printf("%d: %s", i+1, line);
			// Parse the line <TAB> splits the fields
			// Fields: Number <TAB> Description <TAB> KITCube Sensor name <TAB> Axis name
			namePtr = strchr(line, '\t'); // Field 2
			// Read the description back from file
			// This feature can be used to overwrite the standard name from the header files
			// e.g. replace german names or unsystematic ones?!
			sLine = namePtr + 1;
			sensor[i].comment = sLine.substr(0, sLine.find('\t'));
			if (namePtr > 0) namePtr = strchr(namePtr+1, '\t'); // Field 3
			if (namePtr == 0)
				throw std::invalid_argument("No sensor name found in the *.sensors file");
			sscanf(namePtr, "%s", name);
			sensor[i].name = name;
			
			if (namePtr > 0) namePtr = strchr(namePtr+1, '\t'); // Field 3
			if (namePtr == 0)
				throw std::invalid_argument("No axis found in the *.sensors file");
			sscanf(namePtr, "%s", name);
			axisName = name;
			
			// Find the name of the axis in the axis list
			sensor[i].axis = -1;
			for (j = 0; j < nAxis; j++){
				if (axis[j].name == axisName) sensor[i].axis = j;
			}
			if (sensor[i].axis == -1){
				printf("Analysing sensor %s, axis type %s \n", sensor[i].name.c_str(), axisName.c_str());
				throw std::invalid_argument("Axis type is not defined in the inifile");
			}
			
			if (debug > 2) printf("%d %s %s\n", i+1, sensor[i].name.c_str(), axis[sensor[i].axis].name.c_str());
			i++;
		}
	}
	if (i < nSensors - 1) {
		printf("Found %d sensor names\n", i+1);
		throw std::invalid_argument("Missing definitions in the sensor list");
	}
	
	// TODO: Check if the name are according to the naming conventions
	// E.g. check aggregation type?!
/*	
	for (i = 0; i < nSensors; i++) {
		pos = 0;
		for (j = 0; j < 5; j++) {
			pos = sensor[i].name.find('.', pos);
			if (pos == std::string::npos){
				sprintf(line, "Sensor name #%d doesn't match naming convention: %s\n", i+1, sensor[i].name.c_str());
				throw std::invalid_argument(line);
			}
		}
	}
*/	

/*	
	// Replace the module name
	// This obviously only works if the first part of the name contains the module?!
	// Not really necessary as the name can also be given in the sensors files...
	for (i = 0; i < nSensors; i++) {
		sensor[i].name.replace(0,sensor[i].name.find('.'), moduleName);
		//printf("%d %s\n", i+1, sensor[i].name.c_str());
	}
*/
	
	// TODO: Check for double names
	
	
	// Close list file
	fclose(flist);
	
	if (debug > 3) {
		for (i = 0; i < nSensors; i++) {
			printf("Sensor %4d: %s %s (%s)\n", i+1, sensor[i].name.c_str(),
				sensor[i].comment.c_str(), axis[sensor[i].axis].unit.c_str());
		}
	}
}


void DAQDevice::getSamplingTime(struct timeval *time){
	time->tv_sec = tSample / 1000;
	time->tv_usec = (tSample%1000) * 1000;
}


unsigned int DAQDevice::getModuleNumber(){
	return(this->moduleNumber);
}


unsigned int DAQDevice::getSensorGroup(){
	return(0);
}


const char* DAQDevice::getArchiveDir(){
	return(this->archiveDir.c_str());
}


void::DAQDevice::createDirectories(const char *path){
	std::string pathname;
	std::string dir;
	size_t pos0, pos1;
	int i;
	
	
	if (debug >= 1) {
		printf("\n_____DAQDevice::createDirectories(const char *path)_____\n");
		printf("Creating directory: %s\n", path);
	}
	
	// Check if the base directory exists
	pathname = path;
	pos0 = 0;
	pos1 = 0;
	i = 0;
	while ((pos1 != std::string::npos) && (i < 10)) {	// FIXME: why the limitation to 10 subdirectories?
		pos1 = pathname.find("/", pos0);
		if (pos1 != std::string::npos) {
			dir = pathname.substr(0, pos1);
			if (debug > 2)
				printf("Create directory %s (%d, %d)\n", dir.c_str(), pos0, pos1);
			if (dir.length() > 0)	// FIXME: do we really need this check?
				mkdir(dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
			pos0 = pos1 + 1;
			i++;
		}
	}
}


void DAQDevice::openFile(const char *filename){

	// Define filename for reading?!
	this->filename = filename;
}


void DAQDevice::openFile(){ // for writing
	printf("Define openFile() in higher class!\n");
}


void DAQDevice::openNewFile(){
	closeFile();
	fileIndex++;
	this->filename = getDataFilename();
	openFile();
}


void DAQDevice::closeFile(){
	printf("Define closeFile() in higher class!\n");
}


void DAQDevice::copyRemoteData(){
	struct timeval t0, t1;
	struct timezone tz;
	int err;
	char line[256];
	std::string output;
	
	
	if (debug >= 1)
		printf("\n_____DAQDevice::copyRemoteData()_____\n");
	
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
	if (debug >= 2)
		output = "";
	else
		output = " > /dev/null";
	sprintf(line, "rsync -avz %s --include='*/' --include='*.%s' --exclude='*' %s%s  %s%s %s",
			rsyncArgs.c_str(), sensorGroup.c_str(),
			remoteDir.c_str(), getDataDir(),
			archiveDir.c_str(), getDataDir(), output.c_str());
	if (debug >= 2) printf("%s\n", line);
	
	gettimeofday(&t0, &tz);
	err = system(line);
	gettimeofday(&t1, &tz);
	
	if (err != 0) {
		printf("Synchronisation error (rsync)\n");
		//throw std::invalid_argument("Synchronisation error (rsync)");
	}
	
	if (debug >= 2)
		printf("Rsync duration: %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));
}


const char *DAQDevice::getDataDir(){
	char line[256];
	
	sprintf(line, "%s/", moduleName.c_str());
	buffer = line;
	
	return(buffer.c_str());
}


const char *DAQDevice::getDataFilename(){
	std::string name;
	int posIndex;
	//int index;
	std::string filePrefix;
	std::string fileSuffix;
	char line[256];
	
	// Write the index of the file to a list 
	// Process in this list with the next index
	// The read pointer of the last file will be kept
	
	// Find <index> in data template
	posIndex = datafileMask.find("<index>");
	if (posIndex == -1) {
		printf("Error: There is no tag <index> in datafileMask=%s specified in inifile %s\n",
			datafileMask.c_str(), inifile.c_str());
	}
	if (debug > 3) printf("Position of <index> in %s is  %d\n", datafileMask.c_str(), posIndex);
	filePrefix = datafileMask.substr(0, posIndex);
	fileSuffix = datafileMask.substr(posIndex+7,datafileMask.length()-posIndex-7);
	if (debug > 3) printf("Prefix is %s, suffix %s\n", filePrefix.c_str(), fileSuffix.c_str());
	
	sprintf(line, "%s%ld%s", filePrefix.c_str(), fileIndex, fileSuffix.c_str());
	
	buffer = line;
	return(buffer.c_str());
}


void DAQDevice::openDatabase(){
	//printf("=== Create data table === \n");
#ifdef USE_MYSQL
	//printf("Create data table\n");
	
	int i;
	MYSQL_RES *res;
	//MYSQL_RES *resTables;
	MYSQL_ROW row;
	//MYSQL_ROW table;
	char sql[256];
	char line[256];
	std::string cmd;
	int nNewSensors;
	int nNewAxis;
	
	
	// Display arguments
	for (i = 0; i < nSensors; i++) {
		//printf("Col %3d: %s\n", i+1, colNames[i].c_str());
	}
	
	// Create Database tables
	sprintf(line, "Data_%03d_%s_%s", moduleNumber, moduleName.c_str(), sensorGroup.c_str());
	printf("Data table name: \t%s\n", line);
	this->dataTableName = line;
	
	
	//TODO:  Check if the data is available in the class?!
	
	
	// Open database
	db = mysql_init(NULL);
	
	// Connect to database
	printf("Database: \t\t%s@%s\n", dbUser.c_str(), dbHost.c_str());
	if (!mysql_real_connect(db, dbHost.c_str(), dbUser.c_str(), dbPassword.c_str(), 0, 0, NULL, 0)) {
		printf("%s\n", mysql_error(db));
		// Connection failed
		// Set db to zero to indicate missing connection
		db = 0;
		
		return;
	}
	
	// Enable automatic reconnect
	my_bool re_conn = 1;
	mysql_options(db, MYSQL_OPT_RECONNECT, &re_conn);
	
	
	// Read list of databases?!
	// Only kitcube.* are relevant
	
	
	sprintf(sql,"SHOW databases");
	if (mysql_query(db, sql)){
		fprintf(stderr, "%s\n", sql);
		fprintf(stderr, "%s\n", mysql_error(db));
	}
	
	res = mysql_store_result(db);
	
	// output fields 1 and 2 of each row
	//fprintf(fout, "Result: %d x %d\n",
	//            mysql_num_rows(res),
	//            mysql_num_fields(res));
	printf("Project \"%s\" database list: \t", project.c_str());
	while ((row = mysql_fetch_row(res)) != NULL) {
		for (i = 0; i < mysql_num_fields(res); i++) {
			if (strstr(row[i], project.c_str()) == row[i])
				printf("%s", row[i]);
		}
	}
	mysql_free_result(res);
	printf("\n");
	
	
	// Select active database
	// Print the tables of the database
	sprintf(sql,"USE %s", dbName.c_str());
	if (mysql_query(db, sql)){
		//fprintf(stderr, "%s\n", sql);
		//fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create database and try again
		printf("Create database %s\n", dbName.c_str());
		sprintf(sql,"CREATE DATABASE %s", dbName.c_str());
		if (mysql_query(db, sql)){
			fprintf(stderr, "%s\n", sql);
			fprintf(stderr, "%s\n", mysql_error(db));
		}
		
		sprintf(sql,"USE %s", dbName.c_str());
		if (mysql_query(db, sql)){
			fprintf(stderr, "%s\n", sql);
			fprintf(stderr, "%s\n", mysql_error(db));
			
			return;
		}
	}
	
	
	// Create axis table
	// Read axis definition from inifile
	if (axis == 0) readAxis(this->inifile.c_str());
	
	// Check if axis table is available
	// Get list of cols in data table
	sprintf(sql,"SHOW COLUMNS FROM `%s`", axisTableName.c_str());
	if (mysql_query(db, sql)){
		fprintf(stderr, "%s\n", sql);
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create table
		printf("Creating axis table\n");
		std::string cmd = "CREATE TABLE `";
		cmd += axisTableName;
		cmd += "` ( `id` int(10) auto_increment, ";
		cmd += "    `name` varchar(4), ";
		cmd += "    `comment` text, ";
		cmd += "    `unit` text, ";
		cmd += "    `min`  float(10), ";
		cmd += "    `max`  float(10), ";
		cmd += "PRIMARY KEY (`id`), INDEX(`name`) ) TYPE=MyISAM";
		
		//printf("SQL: %s\n", cmd.c_str());
		
		if (mysql_query(db, cmd.c_str())){
			fprintf(stderr, "%s\n", cmd.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			throw std::invalid_argument("Creation of axis table failed");
		}
	}
	res = mysql_store_result(db);
	mysql_free_result(res);
	
	// Query all axis
	sprintf(sql,"SELECT id,name FROM `%s`", axisTableName.c_str());
	if (mysql_query(db, sql)){
		fprintf(stderr, "%s\n", sql);
		fprintf(stderr, "%s\n", mysql_error(db));
		
		throw std::invalid_argument("Can not read axis table");
	}
	
	
	// Find missing axis definitions
	res = mysql_store_result(db);
	while ((row = mysql_fetch_row(res)) != NULL) {
		for (i = 0; i < nAxis; i++) {
			if (axis[i].name == row[1]){
				axis[i].isNew = false;
				axis[i].id = atoi(row[0]);
				if (debug > 2) printf("The axis %s is already defined in the axis list (id=%d)\n",
									  axis[i].name.c_str(), axis[i].id);
				break;
			}
		}
		//for (i=0;i<mysql_num_fields(res); i++){
		//	printf("%s", row[i]);
		//}
	}
	mysql_free_result(res);
	printf("\n");
	
	// Add the new axis definitions to the database
	nNewAxis = 0;
	for (i = 0; i < nAxis; i++) {
		if (axis[i].isNew) {
			printf("Adding axis %s -- %s (%s) to the axis list\n",
				   axis[i].name.c_str(), axis[i].desc.c_str(), axis[i].unit.c_str());
			sprintf(sql,"INSERT INTO `%s` (`name`,`comment`,`unit`) VALUES ('%s','%s','%s')",
					axisTableName.c_str(),
					axis[i].name.c_str(), axis[i].desc.c_str(), axis[i].unit.c_str());
			if (mysql_query(db, sql)){
				fprintf(stderr, "%s\n", sql);
				fprintf(stderr, "%s\n", mysql_error(db));
				
				throw std::invalid_argument("Cannot create new axis entry");
			}
			
			// Get the id of the new sensor entry
			axis[i].id = mysql_insert_id(db);
			nNewAxis++;
		}
	}
	printf("New axis definitions: %d\n", nNewAxis);
	
	
	// Get list of cols in data table
	sprintf(sql,"SHOW COLUMNS FROM `%s`", dataTableName.c_str());
	if (mysql_query(db, sql)){
		//fprintf(stderr, "%s\n", sql);
		//fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create table
		printf("Creating data table\n");
		std::string cmd = "CREATE TABLE `";
		cmd += dataTableName;
		cmd += "` ( `id` int(12) auto_increment, ";
		cmd += "    `sec` int(12)  default '0', ";
		cmd += "    `usec` int(10)  default '0', ";
		for (i = 0; i < nSensors; i++)
			if (sensor[i].type == "profile") {
				cmd += "`" + sensor[i].name + "` TEXT, ";
			} else {
				cmd += "`" + sensor[i].name + "` float(10), ";
			}
		cmd += "PRIMARY KEY (`id`), INDEX(`sec`) ) TYPE=MyISAM";
		
		//printf("SQL: %s\n", cmd.c_str());
		
		if (mysql_query(db, cmd.c_str())){
			fprintf(stderr, "%s\n", cmd.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			throw std::invalid_argument("Creation of data table failed");
		}
	}
	res = mysql_store_result(db);
	mysql_free_result(res);
	
	
	// TODO: Check the sensor configuration
	
	
	// Create sensor list table
	// Get list of cols in data table
	sprintf(sql,"SHOW COLUMNS FROM `%s`", sensorTableName.c_str());
	if (mysql_query(db, sql)) {
		//fprintf(stderr, "%s\n", sql);
		//fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create table
		printf("Creating sensor table\n");
		cmd = "CREATE TABLE `";
		cmd += sensorTableName;
		cmd += "` (`id` int(10) auto_increment, ";
		cmd += "`name` varchar(30) unique, ";
		cmd += "`module` int(10) unsigned default '0', ";
		cmd += "`comment` text, ";
		cmd += "`axis` int(10), ";
		cmd += "`height` float(10)  default '0', ";
		cmd += "`data_format` text, ";
		cmd += "PRIMARY KEY (`id`), INDEX(`name`) ) TYPE=MyISAM";
		
		//printf("SQL: %s\n", cmd.c_str());
		
		if (mysql_query(db, cmd.c_str())){
			fprintf(stderr, "%s\n", cmd.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			throw std::invalid_argument("Creation of table for sensor list failed");
		}
	}
	res = mysql_store_result(db);
	mysql_free_result(res);
	
	
	// TODO: Check if the sensor names are in the list
	// Store in sensor table
	// Bug: Is not stored in the first time?! At this time the database is not open...
	nNewSensors = 0;
	for (i = 0; i < nSensors; i++) {
		cmd = "INSERT INTO ";
		cmd += "`" + sensorTableName + "` ";
		cmd += "(`name`, `module`, `comment`, `axis`, `height`, `data_format`) VALUES (";
		cmd += "'" + sensor[i].name + "', ";
		sprintf(line, "'%d', ", moduleNumber);
		cmd += line;
		cmd += "'" + sensor[i].name + " " + sensor[i].comment + "', ";
		sprintf(line, "'%d', '%f', ", axis[sensor[i].axis].id, sensor[i].height);
		cmd += line;
		cmd += "'" + sensor[i].data_format + "')";
		
		//printf("SQL: %s (db = %d)\n", cmd.c_str(), db);
		
		if (mysql_query(db, cmd.c_str())){
			//fprintf(stderr, "%s\n", cmd.c_str());
			//fprintf(stderr, "%s\n", mysql_error(db));
			
			// TODO: If this operation fails do not proceed in the file?!
			//break;
		} else {
			nNewSensors++;
		}
	}
	printf("New sensors: \t\t%d\n", nNewSensors);
	printf("\n");
#endif // USE_MYSQL
}


void DAQDevice::closeDatabase(){
#ifdef USE_MYSQL
	if (db > 0){
	  // Close database
	  mysql_close(db);
	  db = 0;
	}
#endif
}


void DAQDevice::readHeader(const char *header){
}


void DAQDevice::readHeader(){
	readHeader((path+filename).c_str());
}


void DAQDevice::writeHeader(){
}

void DAQDevice::parseData(char *line, struct timeval *tData, float *sensorValue){
}


void DAQDevice::storeSensorData(){
	//unsigned char *buf;
	//int len;
	//int n;
	//int fd;
	int j;
	//char *sensorString;
	//int err;
	std::string timeString;
	std::string dateString;
	//unsigned long timestamp;
	
	
	std::string filenameMarker;
	std::string filenameData;
	//struct timeval lastTime;
	//unsigned long lastPos;
	//unsigned long lastIndex;
	//struct timeval tWrite;
	//char line[256];
	
#ifdef USE_MYSQL
	//MYSQL_RES *res;
	//MYSQL_RES *resTables;
	//MYSQL_ROW row;
	//MYSQL_ROW table;
	std::string tableName;
	std::string sql;
	char sData[50];
	struct timeval t0, t1;
	struct timezone tz;
	int i;
#endif
	
	if (sensor[0].name.length() == 0) getSensorNames(sensorListfile.c_str());		
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
	
	if (debug > 2) printf("_____Store sensor data___%s_____________________\n", moduleName.c_str());	
	
	
	// Use the function setNdata, updateTimeStamp, updateData to 
	// fill the sensor string
		
	// Display sensor data
	if (debug > 1) {
		printf(" %ld  %ld  ---- ", tData.tv_sec, tData.tv_usec);
		for (j=0;j<nSensors; j++){				
			printf("%5.3f ", sensorValue[j]);	
		}
		printf("\n");
	}
	
	
#ifdef USE_MYSQL	
	if (db > 0){
		// Write dataset to database
		// Store in the order of appearance	
		//printf("Write record to database\n");
		
		sql = "INSERT INTO `";
		sql += dataTableName + "` (`sec`,`usec`";
		for (i=0; i<nSensors; i++){
			sql += ",`";
			sql += sensor[i].name;
			sql += "`";
		}
		sql +=") VALUES (";
		sprintf(sData, "%ld, %ld", tData.tv_sec, tData.tv_usec);
		sql += sData;
		for (i = 0; i < nSensors; i++) {
			
			sprintf(sData, "%f", sensorValue[i]);
			sql += ",";
			sql += sData;
		}
		sql += ")";
		
		//printf("SQL: %s (db = %d)\n", sql.c_str(), db);
		
		gettimeofday(&t0, &tz);
		
		if (mysql_query(db, sql.c_str())){
			fprintf(stderr, "%s\n", sql.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			// If this operation fails do not proceed in the file?!
			printf("Error: Unable to write data to database\n");
			throw std::invalid_argument("Writing data failed");
		}	
		
		gettimeofday(&t1, &tz);
		printf("DB insert duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));
		
	} else {
		printf("Error: No database availabe\n");
		throw std::invalid_argument("No database");
	}
#endif // of USE_MYSQL
	

}


void DAQDevice::readData(const char *dir, const char *filename){
	printf("Define readData(...) in higher class!\n");
}


void DAQDevice::updateDataSet(unsigned char *buf){
}


void DAQDevice::writeData(){
}


unsigned long DAQDevice::getIndex(char* filename, char* firstTag, char* lastTag,  int len, char* next){
	unsigned long index;
	char indexTag[20];
	int indexTagLen;
	char *ptr, *firstChar, *lastChar;
	std::string restOfName;
	int err;
	
	
	// Find the start of the index
	if (*firstTag != 0){	// take empty prefix into account
		ptr = strcasestr(filename, firstTag);
		if (ptr != filename) {
			//printf("The file %s doesnot match the DAQ-naming convention\n", filename);
			throw std::invalid_argument("Starting tag not found");
		}
		firstChar = filename + strlen(firstTag);
	} else {
		firstChar = filename;
	}
	//printf("Start of index field: %s\n", firstChar);
	
	
	// Use the length if available
	if (len > 0) {
		indexTagLen = len;
		lastChar = firstChar + indexTagLen -1;
	} else {
		// Find the end string
		if (*lastTag != 0){	// take empty suffix into account
			ptr = strcasestr(filename, lastTag);
			if (ptr == 0) {
				//printf("The file %s doesnot match the DAQ-naming convention (missing field termination)\n", filename);
				throw std::invalid_argument("Termintation tag not found");
			}
		} else {
			// Find the next letter in the string
			ptr = firstChar;
			while (isdigit(*ptr)){ptr++;}
		}
		lastChar = ptr - 1;
		indexTagLen = lastChar - firstChar + 1;
	}
	
	// Read the number
	if (indexTagLen > 20){
		//printf("Index length exceeded (%d > 20)\n", indexTagLen);
		throw std::invalid_argument("Length of numeric field exceeded (>20)");
	}
	strncpy(indexTag, firstChar, indexTagLen);
	indexTag[indexTagLen] = 0; // Without there the termination is missing?!
	//printf("Numerical field:  %s %c (len=%d)\n", firstChar, indexTag, indexTagLen);
	
	
	// Get the index
	index = 0;
	err = sscanf(indexTag, "%ld", &index);
	if (err == 0){
		throw std::invalid_argument("No numerical value read from the field");
	}
	
	// Remove the field and return the filename without the index field
	//restOfName = lastChar + 1;
	//strcpy(firstChar, restOfName.c_str());
	
	if (next > 0){
		next = lastChar + 1;
	}
	
	return index;
}


int DAQDevice::getFileNumber(char* filename){
	//std::string name;
	size_t posIndex;
	int index;
	std::string filePrefix;
	std::string fileSuffix;
	
	
	if (debug >= 1) {
		printf("\n_____DAQDevice::getFileNumber(char* filename)_____\n");
		printf("From file %s\n", filename);
	}
	// Write the index of the file to a list
	// Process in this list with the next index
	// The read pointer of the last file will be kept
	
	// Find <index> in data template
	posIndex = datafileMask.find("<index>");
	if (posIndex == std::string::npos) {
		printf("Error: There is no tag <index> in datafileMask=%s specified in inifile %s\n",
			datafileMask.c_str(), inifile.c_str());
	}
	
	if (debug >= 2)
		printf("Position of <index> in %s is: %d\n", datafileMask.c_str(), posIndex);
	
	filePrefix = datafileMask.substr(0, posIndex);
	fileSuffix = datafileMask.substr(posIndex + 7);
	
	if (debug >= 2)
		printf("Prefix is: %s, suffix is: %s\n", filePrefix.c_str(), fileSuffix.c_str());
	
	index = getIndex(filename, (char*) filePrefix.c_str(), (char*) fileSuffix.c_str());
	//printf("Index: %d\n",  index);
	
	return index;
}


void DAQDevice::getNewFiles(){
	std::string dataDir;
	std::string lastFile;
	DIR *din;
	struct dirent *file;
	int nFiles;
	char line[256];
	unsigned int index;
	
	// Linked list of directory entries
	std::string *listName;
	unsigned int *listIndex;
	unsigned int *listNext;
	int nList;
	int i, j;
	int next;
	int err;
	
	// Handler for marker file
	FILE *fmark;
	std::string filenameMarker;
	struct timeval lastTime;
	unsigned int lastPos;
	unsigned int lastIndex;
	
	
	// Go through the list and find the next entry
	// Is there function that gives the distance?
	//
	// Sort the files?! Is there a function that returns a sorted list?
	
	
	// Get the last file (from pseudo inifile?)
	// Define template for the data file name or the folder name?!
	//
	// TODO: Get the template name from device class
	//datafileMask = "Mast-<index>.dat";
	//datafileMask = "M12_<index>.DAR";
	//dataDir = "./"; // Open current dir
	//dataDir = "./data/"; // Open current dir

	if (debug >= 1) {
		printf("\n_____DAQDevice::getNewFiles()_____\n");
		printf("Reading from %s\n", dataDir.c_str());
	}
	
	
	processedData = 0; // Counter for the processed bytes
	
	dataDir = archiveDir + getDataDir();
	
	// Get all alphabetical following files
	din  = opendir(dataDir.c_str());
	if (din == 0) {
		printf("Directory %s is not available - waiting for data to arrive\n", dataDir.c_str());
		return;
	}
	
	// How many entries are in the dir?
	nFiles = 0;
	while ((file = readdir(din)) != NULL) {nFiles++;};
	
	// Initialize the file list
	// The lists starts at 0 and ends 0 again
	listName = new std::string [nFiles + 1];
	listIndex = new unsigned int[nFiles + 1];
	listNext = new unsigned int [nFiles + 1];
	
	listName[0] = "START";
	listIndex [0] = 0;
	listNext[0]= 1;
	
	listName[1] = "END";
	listIndex[1] = 0xffffffff;
	listNext[1] = 0;
	
	nList = 2;
	
	
	rewinddir(din);
	
	while ((file = readdir(din)) != NULL) {
		
		if (debug >= 2) printf("File type: %2d, file name: %s\n", file->d_type, file->d_name);
		
		if (file->d_type == DT_REG){
			try {
				// Get the number of the file that can be used to order the files
				// Which one to read first
				index = getFileNumber(file->d_name);
				if (debug > 5) printf("Index: %d\n", index);
				
				// Insert in the list.
				
				// TODO: Check first, if the current files should go to the end
				// --> The following code is NOT working !!!
				//if (index > listIndex[nList-1]){
				//	//printf("Add to end of list\n");
				//	i = nList-1;
				//} else {
				
				
				// Read the index of the following item in order to find the right slot for the current file
				i = 0;
				j = 0;
				while ((index > listIndex[listNext[i]]) && (j < nList - 1)){
					i = listNext[i];
					j++;
				}
				
				//}
				
				// Fill in the new entry in the list
				listName[nList] = file->d_name;
				listIndex[nList] = index;
				listNext[nList] = listNext[i];
				listNext[i] = nList;
				nList++;
				
				//next = 0;
				//for(i=0;i<nList;i++){
				//	//printf(" %d  %d %d %s\n", i, listIndex[i], listNext[i], listName[i].c_str());
				//	printf(" %d  %d  %s\n", next, listIndex[next], listName[next].c_str());
				//	next = listNext[next];
				//}
				
			} catch (std::invalid_argument &err) {
				//printf("Error in filelist: %s\n", err.what());
			}
		}
	}
	
	closedir(din);
	
	
	if (debug > 3){
		printf("\nList of data files in %s:\n", dataDir.c_str());
		printf(" %6s  %12s %6s %s\n", "No", "Index", "Next", "Filename");
		for (i = 0; i < nList; i++) {
			printf(" %6d  %12u %6d %s\n", i, listIndex[i], listNext[i], listName[i].c_str());
		}
		printf("\n");
	}
	
	
	// Read the last file from the last time the directory was processed
	err = 0;
	lastIndex = 1;
	sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dataDir.c_str(), moduleNumber, sensorGroupNumber);
	filenameMarker = line;
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		err = fscanf(fmark, "%d %ld %ld %d", &lastIndex,  &lastTime.tv_sec, &lastTime.tv_usec, &lastPos);
		fclose(fmark);
	} else {
		// Create new marker file
		printf("No marker file found -- try to create a new one  %s\n", filenameMarker.c_str());
		fmark = fopen(filenameMarker.c_str(), "w");
		if (fmark > 0) {
			fprintf(fmark, "%d %d %d %d\n", lastIndex, 0, 0, 0);
			fclose(fmark);
		}	
		
		lastTime.tv_sec = 0;
		lastTime.tv_usec = 0;
		lastPos = 0;
	}	
	//printf("Marker %s (err=%d, errno=%d): %d\n", filenameMarker.c_str(), err, errno, lastIndex);
	
	// Read the data from the files
	try {
		next = 0;
		for (i = 0; i < nList - 1; i++) {
			//printf(" %d  %d %d %s\n", i, listIndex[i], listNext[i], listName[i].c_str());
			//printf("Reading %d  %d  %s %d\n", next, listIndex[next], listName[next].c_str(), lastIndex);
			
			if (listIndex[next] == lastIndex){
				if (debug) printf("Reading file %d, index %d ___ %s (continued)\n", next, listIndex[next], listName[next].c_str());
				readData(dataDir.c_str(), listName[next].c_str());
			}
			
			if (listIndex[next] > lastIndex){
				// Remove the pointers of the last file
				fmark = fopen(filenameMarker.c_str(), "w");
				if (fmark > 0) {
					//fprintf(fmark, "%d %d %d %d\n", listIndex[next], 0, 0, 0);
					// Preserve the time stamp
					fprintf(fmark, "%d %ld %ld %d\n", listIndex[next], lastTime.tv_sec, lastTime.tv_usec, 0);
					fclose(fmark);
				}
				

				if (debug == 0) printf("%s%s\n",  dataDir.c_str(), listName[next].c_str());
				if (debug) printf("Reading file %d, index %d ___ %s\n", next, listIndex[next], listName[next].c_str());
				readData(dataDir.c_str(), listName[next].c_str());
			}
			
			// Check if file has been completely read
			if (listIndex[next] >= lastIndex){
				if (!reachedEOF()){
					if (debug) {
						printf("EOF not reached - continue with %s, position %d in next call\n",
							listName[next].c_str(), lastPos);	// FIXME: this gives wrong numbers, as lastPos gets updated in readData(...)
					}
					break;
				}
			}
			
			next = listNext[next];
		}
	} catch (std::invalid_argument &err){
		printf("Error: %s\n", err.what());
		// Reset database connection
		closeDatabase();
	}
	
	// Note: 
	// The marker file is also updated in readData() with the lastest position in file pointer
	//
	
	delete [] listName;
	delete [] listIndex;
	delete [] listNext;
}


unsigned long DAQDevice::getTimestamp(const char *date, const char *time){
	const char *ptr1, *ptr2;
	struct tm timestamp;
	// char buf[20];
	
	ptr1 = strstr(date, ".");
	if (ptr1 > 0) {
		std::string day(date, ptr1-date);
		//printf("Day %s\n", day.c_str());
		timestamp.tm_mday = atoi(day.c_str());
		ptr1++;
		
		ptr2 = strstr(ptr1, ".");
		if (ptr2 > 0) {
			std::string month(ptr1, ptr2-ptr1);
			//printf("Month %s\n", month.c_str());
			timestamp.tm_mon = atoi(month.c_str()) -1;
			ptr2++;
			
			std::string year(ptr2, strlen(ptr2));
			//printf("Year %s\n", year.c_str());
			timestamp.tm_year = atoi(year.c_str()) - 1900;
			
		}
	}
	
	
	ptr1 = strstr(time, ":");
	if (ptr1 > 0) {
		std::string hour(time, ptr1-time);
		//printf("Hour %s\n", hour.c_str());
		timestamp.tm_hour = atoi(hour.c_str());
		ptr1++;
		
		ptr2 = strstr(ptr1, ":");
		if (ptr2 > 0) {
			std::string min(ptr1, ptr2-ptr1);
			//printf("Minute %s\n", min.c_str());
			timestamp.tm_min = atoi(min.c_str());
			ptr2++;
			
			std::string sec(ptr2, strlen(ptr2));
			//printf("Second %s\n", sec.c_str());
			timestamp.tm_sec = atoi(sec.c_str());
		}
	}
	
	
	//printf("%d %d %d   %d %d %d  -- %d errno %d\n", timestamp.tm_mday, timestamp.tm_mon+1, timestamp.tm_year+ 1900,
	//	   timestamp.tm_hour, timestamp.tm_min, timestamp.tm_sec,
	//	   timegm(&timestamp), errno);
	
	// Is the time always given in UTC?
	return( timegm(&timestamp));
}


bool DAQDevice::reachedEOF(){
	return(fd_eof);
}
