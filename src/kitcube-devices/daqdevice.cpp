/***************************************************************************
                          daqdevice.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "daqdevice.h"

#include <sys/stat.h>


DAQDevice::DAQDevice(){
	
	debug = 0;
	
	appId = 0;
    isFlatFolder = 0;
	moduleType = "Generic";
	moduleNumber = 0;
	this->sensorGroup = "txt";
	sensorGroupNumber = 0;
	headerLine = 1;
	dataSep = "\t\n";
	commentChar = "#";
	
	this->iniGroup = "DAQDevice";
	this->tAlarm = 0; // No alarm limits
	
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
	nSensorCols = 0;
	sensorValue = 0;
	processedData = 0;
	
	initDone = 0;
	useTicks = true; // Store microseconds
	
	dataTablePrefix = "Data";
	
#ifdef USE_MYSQL
	db = 0;
#endif
	
#ifdef USE_PYTHON
	pModule = 0;
	pFunc = 0;
#endif
		
}


DAQDevice::~DAQDevice(){
	FILE *fmark;
	
	// Save current position
	if (fileIndex != 0) {
		if (debug>2)
			printf("Closing data file, save file position in %s\n", filenameMarker.c_str());
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
	
	// Free the Python analysis code
	releasePython();
	
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

void DAQDevice::setAppId(int id){
	this->appId = id;
}

int DAQDevice::getAppId(){
	return(this->appId);
}

void DAQDevice::setConfigDefaults(){
	// Note:
	// The paramters are dependant of the module number that is not known at the creation
	// of the class but first time after reading from the inifile
	//
}



void DAQDevice::readInifileCommon(const char *filename){
	akInifile *ini;
	Inifile::result error;

	std::string value;
	std::string tUnit;
	float tValue;

	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	// Save the name of the inifile
	this->inifile = filename;

	//
	// Set common module independant parameters
	//
	this->project = "kitcube";
	this->dbHost = "localhost";
	this->dbUser = "root";
	this->dbPassword = "";
	//this->dbName = project + "_active"; // s. below
	this->sensorTableName = "Sensorlist";
	this->axisTableName = "Axislist";
	this->moduleTableName = "ModuleList";
	this->statusTableName = "Statuslist";
	
	this->configDir = "./";
	this->dataDir = "./data";
	this->remoteDir = "../kitcube-data/data";
	this->archiveDir = "./data";
	this->rsyncArgs = "";
	
	this->pythonDir = "";
	
	
	if (debug > 2) {
		ini = new akInifile(inifile.c_str(), stdout);
	} else {
		ini = new akInifile(inifile.c_str()); // Skip output
	}
	if (ini->Status()==Inifile::kSUCCESS){
		
		// Read global parameters
		// The parameters should be the same for the whole system
		if (debug > 2)
			printf("[Common]\n");
		ini->SpecifyGroup("Common");
		this->project = ini->GetFirstString("project", project.c_str(), &error);
		this->dbHost = ini->GetFirstString("dbHost", dbHost.c_str(), &error);
		
		this->dbName = project + "_active";
		this->dbName = ini->GetFirstString("dbName", dbName.c_str(), &error);
		this->dbUser = ini->GetFirstString("dbUser", dbUser.c_str(), &error);
		this->dbPassword = ini->GetFirstString("dbPassword", dbPassword.c_str(), &error);
		
		// TODO: Some of the directories are not used any more.
		//       The synchronization is done by external scripts
		this->configDir = ini->GetFirstString("configDir", configDir.c_str(), &error);
		this->dataDir = ini->GetFirstString("dataDir", dataDir.c_str(), &error);
		this->remoteDir = ini->GetFirstString("remoteDir", remoteDir.c_str(), &error);
		this->archiveDir = ini->GetFirstString("archiveDir", archiveDir.c_str(), &error);
		this->rsyncArgs = ini->GetFirstString("rsyncArgs", rsyncArgs.c_str(), &error);
		while (error != Inifile::kFAIL) {
			this->rsyncArgs += " ";
			this->rsyncArgs += ini->GetNextString("", &error);
		}
		
		this->pythonDir = ini->GetFirstString("pythonDir", this->pythonDir.c_str(), &error);
	
		
		// Get default parameters for the modules
		//tValue = ini->GetFirstValue("alarmDelay", 5, &error); 
		value = ini->GetFirstString("alarmDelay", "5", &error);
		if (value == "no") {
			this->tAlarm = 0; 
		} else {	
				sscanf(value.c_str(), "%f", &tValue);
				tUnit = ini->GetNextString("min", &error);
				this->tAlarm = tValue * 60;
				if ((tUnit == "sec") || (tUnit == "s")) this->tAlarm = tValue;
				if (tUnit == "h") this->tAlarm = tValue * 3600;
		}
		// Format of the timestamp
		value = ini->GetFirstString("useTicks", "yes", &error);
		if (value == "no") {
			useTicks = false;
		}
			
	}
	delete ini;
	
}


void DAQDevice::readInifile(const char *filename, const char *group){
	akInifile *ini;
	Inifile::result error;
	char line[256];
	std::string value;
	float tValue;
	std::string tUnit;
	char sValue[20];
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// Read module number independant parameter
	// from the [Common] section
	readInifileCommon(filename);
	
	
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
			this->headerLine = ini->GetFirstValue("headerLine", (int) headerLine, &error);
			this->dataSep = ini->GetFirstString("dataSep", dataSep.c_str(), &error);
			this->commentChar = ini->GetFirstString("commentChar", commentChar.c_str(), &error);
            value = ini->GetFirstString("flatFolder", "no", &error);
            if (value == "yes") {
                isFlatFolder = true;
            }
  
		}
	}
	delete ini;

/* -- moved to readInifileCommon()---
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
	this->statusTableName = "Statuslist";
	
	this->configDir = "./";
	this->dataDir = "./data";
	this->remoteDir = "../kitcube-data/data";
	this->archiveDir = "./data";
	this->rsyncArgs = "";
*/
	
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
	
		
/*	-- moved to readInifileCommon()---
	
		// Read global parameters
		// The parameters should be the same for the whole system
		if (debug > 2)
			printf("[Common]\n");
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
		
*/		
		

		if (debug > 2)
			printf("[%s]\n", iniGroup.c_str());
		// Module specific parameters
		ini->SpecifyGroup(iniGroup.c_str());
		this->moduleNumber = ini->GetFirstValue("moduleNumber", (int) moduleNumber, &error);
		this->sensorGroup = ini->GetFirstString("sensorGroup", sensorGroup.c_str(), &error);
		this->sensorGroupNumber = getSensorGroup();
		
		this->moduleName = ini->GetFirstString("moduleName", moduleName.c_str(), &error);
		this->moduleComment= ini->GetFirstString("moduleComment", moduleComment.c_str(), &error);
		this->datafileTemplate = ini->GetFirstString("datafileTemplate", datafileTemplate.c_str(), &error);
        this->dataSubDir = ini->GetFirstString("dataSubDir", getDataDir(), &error);
        
		// Only for information ?!
		// Seems to be not used
		tValue= ini->GetFirstValue("samplingTime", (float) tSample, &error);
		tUnit = ini->GetNextString("ms", &error);
		this->tSample = tValue;
		if ((tUnit == "sec") || (tUnit == "s")) this->tSample = tValue * 1000;
		if (tUnit == "min") this->tSample = tValue * 60000;
		
		// Specifies the maximum allowed delay of new data
		//tValue = ini->GetFirstValue("alarmDelay", (float) this->tAlarm/60, &error); 
		sprintf(sValue,"%f", (float) this->tAlarm/60);
		value = ini->GetFirstString("alarmDelay", sValue, &error);
		if (value == "no") {
			this->tAlarm = 0; 
		} else {	
			sscanf(value.c_str(), "%f", &tValue);
			tUnit = ini->GetNextString("min", &error);
			this->tAlarm = tValue * 60;
			if ((tUnit == "sec") || (tUnit == "s")) this->tAlarm = tValue;
			if (tUnit == "h") this->tAlarm = tValue * 3600;
		}
				
		
		// Global parameters can be overwritten by the module settings
		this->configDir = ini->GetFirstString("configDir", configDir.c_str(), &error);
		this->dataDir = ini->GetFirstString("dataDir", dataDir.c_str(), &error);
		this->remoteDir = ini->GetFirstString("remoteDir", remoteDir.c_str(), &error);
		this->archiveDir = ini->GetFirstString("archiveDir", archiveDir.c_str(), &error);
		this->datafileMask = ini->GetFirstString("datafileMask", datafileMask.c_str(), &error);
		this->sensorListfile = ini->GetFirstString("sensorList", sensorListfile.c_str(), &error);
		
		
		// Add module number to dataDir + archiveDir
		if (dataDir.at(dataDir.length()-1) != '/') dataDir += "/";
		if (archiveDir.at(archiveDir.length()-1) != '/') archiveDir += "/";
		if (remoteDir.at(remoteDir.length()-1) != '/') remoteDir += "/";
		sprintf(line, "%03d/", moduleNumber);
		this->dataDir += line;
		this->remoteDir += line;
		this->archiveDir += line;
		
		// Read the names of the Python analysis scripts
		this->pythonDir = ini->GetFirstString("pythonDir", this->pythonDir.c_str(), &error);
		this->pythonModule = ini->GetFirstString("python", "", &error);
		this->pythonFunction = ini->GetNextString("", &error);

		
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
		//throw std::invalid_argument("Missing <index> in datafileMask");
	}
	
	statusTableKey = "reader.";
	sprintf(sValue,"%d", moduleNumber);
	statusTableKey += sValue;
	statusTableKey += ".";
	statusTableKey += sensorGroup;
	
	
	// Display configuration for one module
	if (debug > 2) {
		printf("Module: %s (%s, %d) Sensor group %s (%d)\n",
			   moduleName.c_str(), moduleComment.c_str(), moduleNumber,
			   sensorGroup.c_str(), sensorGroupNumber);
		printf("Key: %s\n", statusTableKey.c_str());
	}
	
	
	// TODO: Display database configuration, table names
	
	
	// Load Python, if a name of a module is defined
	if (pythonModule.length() > 0) {
		if (debug > 2) {
			printf("      : Analysis with %s.py, %s()\n", 
				   pythonModule.c_str(), pythonFunction.c_str());
		}
		loadPython();
	}	

}


void DAQDevice::loadPython(){

#ifdef USE_PYTHON
	// Load the python script?!
	// What happens if the script does not work???
	// In priciple this class will quit, because to processing fails?!
    // 
	PyObject *pName;
	
	// Check if Python has been loaded?!
	if (pModule > 0) {
		// Reload function first 
		releasePython();
	}
	
	Py_Initialize();
	PyRun_SimpleString("import sys");
	
	// TODO: Add the directory of the python scripts here 
    PyRun_SimpleString("sys.path.append('')");    
	
	pName = PyString_FromString(pythonModule.c_str());
    // Error checking of pName left out 
	
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
	
	
    if (pModule != NULL) { 
	    pFunc = PyObject_GetAttrString(pModule, pythonFunction.c_str());
	    // pFunc is a new reference 
		
	    if (pFunc && PyCallable_Check(pFunc)) {
			
			// Function has been loaded
			
		} else {
			if (PyErr_Occurred())  PyErr_Print();
		    fprintf(stderr, "Cannot find function \"%s\"\n", pythonFunction.c_str());
			
//			Py_XDECREF(pFunc);
//			Py_DECREF(pModule);
			
			throw std::invalid_argument("Failed to find Python function in module");			
	    }
		
	} else {
	    PyErr_Print();
	    fprintf(stderr, "Failed to load \"%s\"\n", pythonModule.c_str());
	    
		// No release of pModule needed here
		
		throw std::invalid_argument("Failed to load Python module");
    }
#endif	
	
}


void DAQDevice::releasePython(){

#ifdef USE_PYTHON
	if (pModule > 0){
		if (debug > 2) printf("Release Pyhton script\n");
		
		Py_XDECREF(pFunc);
        Py_DECREF(pModule);

		Py_Finalize();
		
		pModule = 0;
	}
#endif
	
}


void DAQDevice::readDataWithPython(const char *filename){
	// TODO: Extend the function to revceive more than 
	// one dataset from  a data file
	// E.g. handle other complex data files?!
	
	int i;
	int nArgs = 1;
	int nRes;
	PyObject *pArgs, *pValue, *pResult;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
		
	
	// Convert the argument list to python
	pArgs = PyTuple_New(nArgs);
	pValue = PyString_FromString(filename);
	if (!pValue) {
		throw(std::invalid_argument("Cannot convert argument to Python"));
	}
	// pValue reference stolen here: 
	PyTuple_SetItem(pArgs, 0, pValue);
	
    // Call the function
	pResult = PyObject_CallObject(this->pFunc, pArgs);
	Py_DECREF(pArgs);

	if (pResult == NULL) {
		PyErr_Print();
		throw(std::invalid_argument("Call of Python function failed"));
	}

	// Copy the results to the sensors list
	nRes = PyObject_Length(pResult);
	if (nRes != nSensors) {
		throw(std::invalid_argument("Number of results does not match nSensors"));		
	}
	for (i = 0; i<nRes; i++){
		pValue = PyList_GetItem(pResult, i);
		printf("Result of call: %f\n", PyFloat_AsDouble(pValue));
		sensorValue[i] = PyFloat_AsDouble(pValue);
		Py_DECREF(pValue);
	}
	Py_DECREF(pResult);

	// Store data 
	storeSensorData();
	
}



void DAQDevice::readAxis(const char *inifile){
	akInifile *ini;
	Inifile::result error;
	std::string value;
	int i;
	std::string item;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
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
			if (debug > 3)
				printf("%-20s : %d\n", "Number of axis", nAxis);
			
			// Allocate axis definition
			if (axis > 0)
				delete [] axis;
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
				if (debug > 3)
					printf("%s - %s (%s)\n", axis[i].name.c_str(), axis[i].desc.c_str(), axis[i].unit.c_str());
				
				axis[i].isNew = true;
			}
		}
	}
	delete ini;
}


void DAQDevice::getSensorNames(const char *sensor_list_file_name) {
	FILE *sensor_list_file;
	int i, j;
	char *line = NULL;
	size_t len = 0;
	char *tmp;
	std::string axisName;
	std::string full_sensorlist_filename;
	int n;
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
    // Print module names
	printf("#%03d %s/%s -- \"%s\" from [%s]:\n", moduleNumber, moduleName.c_str(), 
		   	sensorGroup.c_str(), moduleComment.c_str(), iniGroup.c_str());
	
	
	// Will need the axis definition for parsing the sensor names
	if (axis == 0)
		readAxis(this->inifile.c_str());
	
	full_sensorlist_filename =  configDir + sensor_list_file_name;
	
	if (debug > 2)
		printf("Read sensor names from list file: %s\n", full_sensorlist_filename.c_str());
	
	// open sensor list file
	sensor_list_file = fopen(full_sensorlist_filename.c_str(), "r");
	
	// if there is no sensor list file, create a template file
	if (sensor_list_file == NULL) {
		printf("\033[31mError: Configuration file with sensor list is missing\033[0m\n");
		printf("Creating template file: %s\n", full_sensorlist_filename.c_str());
		
		sensor_list_file = fopen(full_sensorlist_filename.c_str(), "w");
		if (sensor_list_file == NULL) {
			throw std::invalid_argument("Error creating template file");
		}
		
		fprintf(sensor_list_file, "Number of sensors: <n>\n");
		fprintf(sensor_list_file, "<sensor number>\t<comment>\t<sensor name>\t<axis>\n");
		
		fclose(sensor_list_file);
		
		throw std::invalid_argument("Fill in sensor names in template file");
	}
	
	// read number of sensors from sensor list file
	// this has to be done to be able to create the sensorType structs before actually parsing the file
	read_ascii_line(&line, &len, sensor_list_file);
	n = sscanf(line, "Number of sensors: %d %d\n", &nSensors, &nSensorCols);
	if (n == 0) {
		throw std::invalid_argument("The first line with the numer of sensors is wrong");
	}
	if (n == 1) {
		nSensorCols = nSensors;
	}
	if (n == 2){
		if (nSensorCols > nSensors)
			throw std::invalid_argument("The numer of sensors entries in the data table can't be larger than the number of sensors");
	}
	
	if (debug >= 1){
		printf("%-20s : %d\n", "Number of sensors", nSensors);
		if (nSensorCols < nSensors)
			printf("%-20s : %d\n", "Number of sensor cols", nSensorCols);
	}
	
	// create sensor list
	if (sensor > 0)
		delete [] sensor;
	sensor = new struct sensorType [nSensors];
	
	//----------------------------------------------------------------------
	// read sensor descriptions, parse the lines and fill sensor list
	// format: number <TAB> comment <TAB> KITCube sensor name <TAB> axis name
	//----------------------------------------------------------------------
	for (i = 0; i < nSensors; i++) {
		read_ascii_line(&line, &len, sensor_list_file);
        if (debug > 4) printf("Line: %s\n", line);
		
		// TODO: check for missing entries in line
		
		// get number
		tmp = strtok(line, "\t");
		
		// get comment
		tmp = strtok(NULL, "\t");
		sensor[i].comment = tmp;
		
		// get sensor name
		tmp = strtok(NULL, "\t");
		sensor[i].name = tmp;
		
		// get axis
		tmp = strtok(NULL, "\n");
		axisName = tmp;
		
		// Find the name of the axis in the axis list
		sensor[i].axis = -1;
		for (j = 0; j < nAxis; j++){
			if (axis[j].name == axisName)
				sensor[i].axis = j;
		}
		
		if (sensor[i].axis == -1){
			printf("Analysing sensor %s, axis type %s \n", sensor[i].name.c_str(), axisName.c_str());
			throw std::invalid_argument("Axis type is not defined in the inifile");
		}
		
		if (debug >= 1) {
			printf("Sensor %3d           : %s\t%s (%s)\t%s\n",
			       i + 1, sensor[i].name.c_str(), axis[sensor[i].axis].name.c_str(), axis[sensor[i].axis].unit.c_str(), sensor[i].comment.c_str());
		}
	}	
	
	fclose(sensor_list_file);
	free(line);

	// Display also other information on the DAQ module
	if (debug >= 1) {
		if (tAlarm)
			printf("%-20s : %02d:%02d min\n", "Alarm delay", tAlarm/60, tAlarm%60);
		else
			printf("%-20s : disabled\n", "Alarm delay");
		printf("\n");
		
		if (pModule > 0){
			printf("%-20s : %s.py, %s()\n", "Python script", 
				   pythonModule.c_str(), pythonFunction.c_str());
		}
	}	

}


void DAQDevice::saveFilePosition(long lastIndex, unsigned long currPos, struct timeval &timestamp){
	FILE *fmark;

	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);

	// Write the last valid time stamp / file position
	fmark = fopen(filenameMarker.c_str(), "w");
	if (fmark > 0) {
		fprintf(fmark, "%ld %ld %ld %ld\n", lastIndex, timestamp.tv_sec, (long) timestamp.tv_usec, currPos);
		fclose(fmark);
	}

	
#ifdef USE_MYSQL
	// Update the status table in the database	
	struct timeval t;
	struct timezone tz;
	std::string sql;
	char sData[50];

	// Get curent time
	gettimeofday(&t, &tz);	
	
	// TODO: UPDATE ...
	// UPDATE SET usec = current time, usecData = timestamp WHERE module = this->moduleNumber
	sql = "UPDATE `";
	sql += statusTableName + "` SET `sec` = ";
	sprintf(sData, "%ld", t.tv_sec);
	sql += sData;
	sql += ", `secData` = ";
	sprintf(sData, "%ld", timestamp.tv_sec);
	sql += sData;
	sql += ", `status`= 'Running' ";
	sql += " WHERE `skey` = '";
	sql += statusTableKey;
	sql += "' "; 
	
	
	if (mysql_query(db, sql.c_str())){
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// If this operation fails do not proceed in the file?!
		printf("Error: Unable to write register module in status list\n");
		throw std::invalid_argument("Writing data failed");
	}	
	
	//printf("SQL: %s\n", sql.c_str());	
	//printf("mysql_affected_rows: %lld\n", mysql_affected_rows(db));	
#endif
	
	
}


void DAQDevice::loadFilePosition(long &lastIndex, unsigned long &lastPos, struct timeval &timestamp){
	FILE *fmark;
    struct timeval lastTime;
	
	lastPos = 0;
	lastTime.tv_sec = 0;
	lastTime.tv_usec = 0;
	
	if (debug > 1)
		printf("Get marker from %s\n", filenameMarker.c_str());
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark > 0) {
		fscanf(fmark, "%ld %ld %ld %ld", &lastIndex,  &lastTime.tv_sec, (long *) &lastTime.tv_usec, &lastPos);
		fclose(fmark);
		
		// Read back the data time stamp of the last call
		timestamp.tv_sec = lastTime.tv_sec;
		timestamp.tv_usec = lastTime.tv_usec;
		
		if (debug > 1)
			printf("Last time stamp was %ld\n", lastTime.tv_sec);
	}	

	return;
}


int DAQDevice::read_ascii_line(char **buffer, size_t *length, FILE *file_ptr) {
	if (getline(buffer, length, file_ptr) == -1) {
		if (debug > 3) printf("Error reading from file or EOF reached\n");
		return -1;
	}
	if (strchr(*buffer, '\n') == NULL) {
		if (debug > 3) printf("Error: line read from file is not complete\n");
		return -1;
	}
	
	return 0;
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
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	if (debug > 2)
		printf("Creating directory '%s'\n", path);
	
	// Check if the base directory exists
	pathname = path;
	pos0 = 0;
	pos1 = 0;
	i = 0;
	while ((pos1 != std::string::npos) && (i < 10)) {	// FIXME: why the limitation to 10 subdirectories?
		pos1 = pathname.find("/", pos0);
		if (pos1 != std::string::npos) {
			dir = pathname.substr(0, pos1);
			if (debug > 3)
				printf("Create directory %s (%ld, %ld)\n", dir.c_str(), pos0, pos1);
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


const char *DAQDevice::getDataDir(){
	char line[256];
	
	sprintf(line, "%s/", moduleName.c_str());
	buffer = line;
	
	return(buffer.c_str());
}


const char *DAQDevice::getDataFilename(){
	time_t time_in_sec;
	struct tm *date;
	char tmp_line[16];
	int pos_index;
	
	
	// Get the actual day
	time(&time_in_sec);	// get seconds since the Epoch
	date = gmtime((const time_t *) &time_in_sec);
	
	// print date string of file name to buffer
	sprintf(tmp_line, "%02d%02d%02d", date->tm_year - 100, date->tm_mon + 1, date->tm_mday);
	
	// replace <index> in datafile mask with date string
	buffer = datafileMask;
	pos_index = buffer.find("<index>");
	buffer.replace(pos_index, 7, tmp_line);
	
	return buffer.c_str();
}


int DAQDevice::create_data_table_name(std::string & data_table_name)
{
	char *line;
	int err;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	err = asprintf(&line, "%s_%03d_%s_%s", dataTablePrefix.c_str(),
				   moduleNumber, moduleName.c_str(), sensorGroup.c_str());
	if (err == -1) {
		printf("Error: not enough space for string!\n");
		return -1;
	} else {
		if (debug > 2) printf("Data table name: \t%s\n", line);
		data_table_name = line;
		free(line);
		return 0;
	}
	
}


void DAQDevice::connectDatabase() {
#ifdef USE_MYSQL
	std::string sql;

	// allocate MYSQL object
	// TODO: only, if it does not exist yet!
	db = mysql_init(NULL);
	if (db == NULL) {
		printf("Error allocating MYSQL object!\n");
		// TODO: error handling
	}
	
	// Enable automatic reconnect
	my_bool re_conn = 1;
	mysql_options(db, MYSQL_OPT_RECONNECT, &re_conn);
	
	// connect to database
	//printf("Database: \t\t%s@%s\n", dbUser.c_str(), dbHost.c_str());
	if (!mysql_real_connect(db, dbHost.c_str(), dbUser.c_str(), dbPassword.c_str(), NULL, 0, NULL, 0)) {
		printf("Failed to connect to database: %s\n", mysql_error(db));
		// TODO: error handling
        throw std::invalid_argument("Failed to connect to database");
	}
	
	// **********************************************************************
	// select the default database
	// create it, if it doesn't exist
	// **********************************************************************
	if (mysql_select_db(db, dbName.c_str())) {
		printf("Error selecting DB '%s' for use: %s\n", dbName.c_str(), mysql_error(db));
		
		printf("Creating DB '%s'...\n", dbName.c_str());
		sql = "CREATE DATABASE " + dbName;
		if (mysql_query(db, sql.c_str())) {
			printf("Error creating new DB '%s': %s\n", dbName.c_str(), mysql_error(db));
		}
		
		if (mysql_select_db(db, dbName.c_str())) {
			printf("Error selecting DB '%s' for use: %s\n", dbName.c_str(), mysql_error(db));
			// TODO: error handling
			return;
		}
	}
	
	return;
#endif	
}



void DAQDevice::openDatabase() {
#ifdef USE_MYSQL
	int i;
	MYSQL_RES *res;
	MYSQL_ROW row;
	std::string sql;
	const char *wild;
	char line[256];
	std::string cmd;
	int nNewSensors;
	int nNewAxis;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// TODO: error handling	
	// TODO:  Check if the data is available in the class?!
	
	// Connect to the database "<project>_active"
	// Create database, if not existing
	connectDatabase();
	
/*
	
	// allocate MYSQL object
	// TODO: only, if it does not exist yet!
	db = mysql_init(NULL);
	if (db == NULL) {
		printf("Error allocating MYSQL object!\n");
		// TODO: error handling
	}
	
	// Enable automatic reconnect
	my_bool re_conn = 1;
	mysql_options(db, MYSQL_OPT_RECONNECT, &re_conn);
	
	// connect to database
	printf("Database: \t\t%s@%s\n", dbUser.c_str(), dbHost.c_str());
	if (!mysql_real_connect(db, dbHost.c_str(), dbUser.c_str(), dbPassword.c_str(), NULL, 0, NULL, 0)) {
		printf("Failed to connect to database: %s\n", mysql_error(db));
		// TODO: error handling
	}
	
	// **********************************************************************
	//select the default database
	//create it, if it doesn't exist
	// **********************************************************************
	if (mysql_select_db(db, dbName.c_str())) {
		printf("Error selecting DB '%s' for use: %s\n", dbName.c_str(), mysql_error(db));
		
		printf("Creating DB '%s'...\n", dbName.c_str());
		sql = "CREATE DATABASE " + dbName;
		if (mysql_query(db, sql.c_str())) {
			printf("Error creating new DB '%s': %s\n", dbName.c_str(), mysql_error(db));
		}
		
		if (mysql_select_db(db, dbName.c_str())) {
			printf("Error selecting DB '%s' for use: %s\n", dbName.c_str(), mysql_error(db));
			// TODO: error handling
			return;
		}
	}
	
*/
	
	//----------------------------------------------------------------------
	// read list of databases, only kitcube* are relevant
	//----------------------------------------------------------------------
	// send SQL query
/*	sql = "SHOW databases";
	if (mysql_query(db, sql.c_str())) {
		printf("SQL command '%s' failed: %s\n", sql.c_str(), mysql_error(db));
		// TODO: error handling
	}
	
	// retrieve results
	res = mysql_store_result(db);
	if (res == NULL) {
		printf("Error retrieving results: %s\n", mysql_error(db));
		// TODO: error handling
	}*/
	
	wild = (project + "%").c_str();
	res = mysql_list_dbs(db, wild);
	if (res == NULL) {
		printf("Error retrieving database list: %s\n", mysql_error(db));
		// TODO: error handling
	}
	
	if (debug > 2) printf("Project \"%s\" database list: \t", project.c_str());
	
	// search for kitcube* database names
	while ((row = mysql_fetch_row(res)) != NULL) {
		if (strstr(row[0], project.c_str()) == row[0])
			if (debug > 2) printf("%s ", row[0]);
	}
	if (debug > 2) printf("\n");
	
	mysql_free_result(res);	

	
	// Create axis table
	// Read axis definition from inifile
	if (axis == 0)
		readAxis(this->inifile.c_str());
	
	// Check if axis table is available
	// Get list of cols in data table
	sql = "SHOW COLUMNS FROM " + axisTableName;
	if (mysql_query(db, sql.c_str())) {
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create table
		printf("Creating axis table\n");
		cmd = "CREATE TABLE `";
		cmd += axisTableName;
		cmd += "` ( `id` int(10) auto_increment, ";
		cmd += "    `name` varchar(4), ";
		cmd += "    `comment` text, ";
		cmd += "    `unit` text, ";
		cmd += "    `min`  float(10), ";
		cmd += "    `max`  float(10), ";
		cmd += "PRIMARY KEY (`id`), INDEX(`name`) ) ENGINE=MyISAM";
		
		//printf("SQL: %s\n", cmd.c_str());
		
		if (mysql_query(db, cmd.c_str())) {
			fprintf(stderr, "%s\n", cmd.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			throw std::invalid_argument("Creation of axis table failed");
		}
	}
	res = mysql_store_result(db);
	mysql_free_result(res);
	
	// Query all axis
	sql = "SELECT id, name FROM " + axisTableName;
	if (mysql_query(db, sql.c_str())) {
		fprintf(stderr, "%s\n", sql.c_str());
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
				if (debug > 2)
					printf("The axis %s is already defined in the axis list (id=%d)\n",
					       axis[i].name.c_str(), axis[i].id);
				break;
			}
		}
		//for (i=0;i<mysql_num_fields(res); i++){
		//	printf("%s", row[i]);
		//}
	}
	mysql_free_result(res);
	if (debug > 2) printf("\n");
	
	// Add the new axis definitions to the database
	nNewAxis = 0;
	for (i = 0; i < nAxis; i++) {
		if (axis[i].isNew) {
			if (debug) printf("Adding axis %s -- %s (%s) to the axis list\n",
				   axis[i].name.c_str(), axis[i].desc.c_str(), axis[i].unit.c_str());
			sql = "INSERT INTO " + axisTableName + "(`name`, `comment`, `unit`) VALUES ('" +
			      axis[i].name + "', '" + axis[i].desc + "', '" + axis[i].unit + "')";
			if (mysql_query(db, sql.c_str())) {
				fprintf(stderr, "%s\n", sql.c_str());
				fprintf(stderr, "%s\n", mysql_error(db));
				
				throw std::invalid_argument("Cannot create new axis entry");
			}
			
			// Get the id of the new sensor entry
			axis[i].id = mysql_insert_id(db);
			nNewAxis++;
		}
	}
	if ((debug) && (nNewAxis>0))
		printf("New axis definitions: %d\n", nNewAxis);
	
	
	/***********************************************************************
	 * create data table, if it doesn't exist
	 **********************************************************************/
	// Create Database tables
	create_data_table_name(dataTableName);
	create_data_table();
	
	
	// TODO: Check the sensor configuration
	
	
	// Create sensor list table
	// Get list of cols in data table
	sql ="SHOW COLUMNS FROM " + sensorTableName;
	if (mysql_query(db, sql.c_str())) {
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
		cmd += "PRIMARY KEY (`id`), INDEX(`name`) ) ENGINE=MyISAM"; 
		
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
	
	if ((debug) && (nNewSensors > 0)){ 
		printf("New sensors: \t\t%d\n", nNewSensors);
		printf("\n");
	}
	
	
	// Create status table
	// Get list of cols in data table
	sql ="SHOW COLUMNS FROM " + statusTableName;
	if (mysql_query(db, sql.c_str())) {
		//fprintf(stderr, "%s\n", sql);
		//fprintf(stderr, "%s\n", mysql_error(db));
		
		// Create table
		if (debug) printf("Creating status table\n");
		cmd = "CREATE TABLE `";
		cmd += statusTableName;
		cmd += "` (`id` int(10) auto_increment, ";
		cmd += "`skey` text, ";						// unique key: TODO add unique statement?!
		cmd += "`sec` bigint default '0', ";		// ts entry by reader
		cmd += "`appName` text, ";					// name of the reporting application
		cmd += "`appId` int(10), ";					// id of the application (used by reader)
		cmd += "`module` int(10), ";			    // no of module
		cmd += "`moduleName` text, ";				// name of the module
		cmd += "`sensorGroup` text, ";				// sensor group of the module
		cmd += "`sensorName` text, ";				// sensor name 

		cmd += "`secData` bigint default '0', ";	// ts of last data
		cmd += "`valueData` double, ";				// value last data

		cmd += "`alarmEnable` int(10) default '1', "; // enable / disable alarm handling
		cmd += "`alarmLimit` int(10) default '0', ";  // delay of alarm
		cmd += "`alarmLow` double, ";				// lower alarm limit
		cmd += "`alarmHigh` double, ";				// upper alarm limit
		cmd += "`alarmTime` int(10) default '0', ";	// limit

		cmd += "`secAlarm` bigint default '0', ";	// ts of alarm condition (report after alarmTime)
		cmd += "`alarm` int(10) default '0', ";		// flag: alarm has been reported 
		cmd += "`status` text, ";					// ???			
		cmd += "`comment` text, ";		
		cmd += "PRIMARY KEY (`id`), INDEX(`appId`, `module`) ) ENGINE=MyISAM"; 
		
		// if alarmLimit = 0 there will be no alarm for a module
		// enable / disable is a user function - intended for operator
		// TODO: secSync complements the information for a whole module?!
		//		But rsync is hard to control, because knowledge of file 
		//		convention or data format is required?!
		//
		
		//printf("SQL: %s\n", cmd.c_str());
		
		if (mysql_query(db, cmd.c_str())){
			fprintf(stderr, "%s\n", cmd.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			throw std::invalid_argument("Creation of table for status list failed");
		}
	}
	res = mysql_store_result(db);
	mysql_free_result(res);
	
	
	// Register in the status list
	registerStatusTab("Starting");
	
#endif // USE_MYSQL
}


int DAQDevice::create_data_table() {
	/***********************************************************************
	 * create data table, if it doesn't exist
	 **********************************************************************/
#ifdef USE_MYSQL
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::string sql_stmt;
	
	
	// search for tables with names like "dataTableName"
	result = mysql_list_tables(db, dataTableName.c_str());
	if (result == NULL) {
		printf("Error retrieving table list: %s\n", mysql_error(db));
		// TODO: error handling
	}
	
	// fetch row from result set
	row = mysql_fetch_row(result);
	
	// free memory for result set
	mysql_free_result(result);
	
	// if there is no row, meaning no table, create it
	if (row == NULL) {
		if (debug) printf("Creating data table %s...\n", dataTableName.c_str());
		
		// build SQL statement
		sql_stmt = "CREATE TABLE `" + dataTableName + "` ";
		if (useTicks)
			sql_stmt += "(`id` bigint auto_increment, `usec` bigint default '0', ";
		else
			sql_stmt += "(`id` bigint auto_increment, `sec` bigint default '0', ";
		for (int i = 0; i < nSensorCols; i++)
			if (sensor[i].type == "profile") {
				sql_stmt += "`" + sensor[i].name + "` blob, ";
			} else {
				sql_stmt += "`" + sensor[i].name + "` double, ";
			}
		if (useTicks)
			sql_stmt += "PRIMARY KEY (`id`), INDEX(`usec`) ) ENGINE=MyISAM";
		else
			sql_stmt += "PRIMARY KEY (`id`), INDEX(`sec`) ) ENGINE=MyISAM";
		
		// execute SQL statement
		if (mysql_query(db, sql_stmt.c_str())) {
			printf("Error creating data table %s: %s\n", dataTableName.c_str(), mysql_error(db));
			// TODO: error handling
		}
	}
#endif
	return 0;
}


void DAQDevice::closeDatabase(){
#ifdef USE_MYSQL
	if (db > 0){
		// Register in the status list
		registerStatusTab("Stopped");
		
		// Close database
		mysql_close(db);
		db = 0;
	}
#endif
}


int DAQDevice::readHeader(const char *header) {
	return -1;
}


void DAQDevice::readHeader(){
	readHeader((path+filename).c_str());
}


void DAQDevice::writeHeader(){
}

int DAQDevice::parseData(char* line, struct timeval* l_tData, double *sensorValue){
	printf("Define parseData in higher level class!\n");
	
	return -1;
}


void DAQDevice::registerStatusTab(const char *status, const char *comment){
#ifdef USE_MYSQL
	std::string sql;
	char sData[50];
	struct timeval t;
	struct timezone tz;
	bool rows;
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
		
	// Get curent time
	gettimeofday(&t, &tz);	

	if (debug > 2) printf("Register module in table %s\n", statusTableName.c_str());
	
	// Check if the entry is available
	// Insert new entry
	// INSERT INTO table (sec = current time, secData = timestamp WHERE module = this->moduleNumber	
	sql = "SELECT * FROM `";
	sql += statusTableName; 	
	sql += "` WHERE `skey` = '";
	sql += statusTableKey;
	sql += "' ";
	
	if (mysql_query(db, sql.c_str())){
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// If this operation fails do not proceed in the file?!
		printf("Error: Unable to read from status list\n");
		throw std::invalid_argument("Reading status list failed");
	}	
	
	MYSQL_RES *res;
	res = mysql_store_result(db);
	rows = mysql_num_rows(res);
	mysql_free_result(res);
	
	
	//printf("SQL: %s\n", sql.c_str());	
	//printf("Rows %d\n", rows);
	
	
	if (rows == 0){	
		// Insert new entry
		// INSERT INTO table (usec = current time, usecData = timestamp WHERE module = this->moduleNumber	
		sql = "INSERT INTO `";
		sql += statusTableName + "` (`skey`, `sec`, `module`, `sensorGroup`";
		sql += ") VALUES ('";
		sql += statusTableKey;
		sql += "',";
		sprintf(sData, "%ld", tData.tv_sec);
		sql += sData;
		sql += ",";
		sprintf(sData, "%d", moduleNumber);
		sql += sData;
		sql += ",\"";
		sql += sensorGroup;
		sql += "\")";
		
		if (mysql_query(db, sql.c_str())){
			fprintf(stderr, "%s\n", sql.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			// If this operation fails do not proceed in the file?!
			printf("Error: Unable to write register module in status list\n");
			throw std::invalid_argument("Writing data failed");
		}
	}
	
	// Update the entry 
	// UPDATE SET usec = current time, usecData = timestamp WHERE module = this->moduleNumber	
	sql = "UPDATE `";
	sql += statusTableName + "` SET `sec` = ";
	sprintf(sData, "%ld", t.tv_sec);
	sql += sData;
	sql += ",`moduleName`= '";
	sql += moduleName;
	sql += "'";
	sql += ", `appName` = 'Reader'";
	sql += ", `appId` = ";
	sprintf(sData, "%d", appId);
	sql += sData;
	if (status > 0){
		sql += ", `status`= '";
		sql += status;
		sql += "'";
	}
	if (comment > 0){
		sql += ", `comment`='";
		sql += comment;
		sql += "'";
	}
	sql += ",`alarmLimit`= ";
	sprintf(sData, "%d", tAlarm);
	sql += sData;
	sql += " WHERE `skey` = '";
	sql += statusTableKey;
	sql += "' ";

	if (mysql_query(db, sql.c_str())){
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// If this operation fails do not proceed in the file?!
		printf("Error: Unable to write register module in status list\n");
		throw std::invalid_argument("Writing data failed");
	}	

	//printf("SQL: %s\n", sql.c_str());	
	//printf("mysql_affected_rows: %lld\n", mysql_affected_rows(db));
	
	return;
#endif
}


void DAQDevice::storeSensorData(){
#ifdef USE_MYSQL
	std::string sql;
	char sData[50];
	struct timeval t0, t1;
	struct timezone tz;
#endif
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
			
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

	
	// Use the function setNdata, updateTimeStamp, updateData to 
	// fill the sensor string
		
	// Display sensor data
	if (debug > 1) {
		printf("%25s --- ", moduleName.c_str());
		printf("%ld s %ld us ---- ", tData.tv_sec, (long) tData.tv_usec);
		for (int i = 0; i < nSensors; i++)
			printf("%f ", sensorValue[i]);
		printf("\n");
	}
	
#ifdef USE_MYSQL
	if (db > 0){
		// Write dataset to database
		// Store in the order of appearance
		//printf("Write record to database\n");
		
		sql = "INSERT INTO `";
		if (useTicks)
			sql += dataTableName + "` (`usec`";
		else
			sql += dataTableName + "` (`sec`";
		for (int i = 0; i < nSensors; i++) {
			sql += ",`";
			sql += sensor[i].name;
			sql += "`";
		}
		sql +=") VALUES (";
		if (useTicks)
			sprintf(sData, "%ld", tData.tv_sec * 1000000 + tData.tv_usec);
		else
			sprintf(sData, "%ld", tData.tv_sec);
		sql += sData;
		for (int i = 0; i < nSensors; i++) {
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
		
		if (debug >= 5)
			printf("DB insert duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));
	} else {
		printf("Error: No database availabe\n");
		throw std::invalid_argument("No database");
	}
#endif // of USE_MYSQL
}


void DAQDevice::readData(std::string full_filename){
	printf("Define readData(...) in higher class!\n");
		
	// TODO: Is it possible to have a generic readData function here???
	//	Try to use only simpler call back functions here?!
	
}


void DAQDevice::updateDataSet(unsigned char *buf){
}


void DAQDevice::writeData(){
}


long DAQDevice::getFileNumber(char* filename) {
	std::string filename_prefix;
	std::string filename_suffix;
	std::string filename_string;
	size_t pos_index, pos_prefix, pos_suffix, length_prefix, length_suffix, filename_length, pos;
	long index;
	
	
	if (debug > 2) {
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
		printf("... from filename '%s'\n", filename);
	}
	
	
	// Write the index of the file to a list
	// Process in this list with the next index
	// The read pointer of the last file will be kept
	
	// Find <index> in data template
	pos_index = datafileMask.find("<index>");
	// FIXME: this is done in DAQDevice::readInifile already. Why repeat it here? What to do in case of error?
	if (pos_index == std::string::npos) {
		printf("Error: There is no tag <index> in datafileMask '%s' specified in inifile %s\n",
			datafileMask.c_str(), inifile.c_str());

        if (datafileMask == filename) return 1;
        else return 0;
        
	} else {
        filename_prefix = datafileMask.substr(0, pos_index);
        length_prefix = filename_prefix.length();
        filename_suffix = datafileMask.substr(pos_index + 7);
        length_suffix = filename_suffix.length();
	}
        
	if (debug >= 4)
		printf("Position of <index> in %s is: %ld -- Prefix is: %s, suffix is: %s\n",
		       datafileMask.c_str(), pos_index, filename_prefix.c_str(), filename_suffix.c_str());
	
	filename_string = filename;
	
	// If there is a prefix, check for prefix at beginning of file name and delete it
	if (filename_prefix.size() != 0) {
		pos_prefix = filename_string.find(filename_prefix);
		if (pos_prefix == 0) {
			filename_string.erase(pos_prefix, length_prefix);
		} else {
			if (debug >= 3)
				printf("Prefix not found or not at the beginning of file name!\n");
			return 0;
		}
	}
	
	// If there is a suffix, check for suffix at end of filename and delete it
    // TODO: Allow extra .gz at the end of the suffix if compressed data files are allowed
    //          Add new flag to the inifile!!!
    // 
	if (filename_suffix.size() != 0) {
		pos_suffix = filename_string.find(filename_suffix);
		filename_length = filename_string.length();
		if ((pos_suffix != std::string::npos) && ((pos_suffix + length_suffix) == filename_length)) {
			filename_string.erase(pos_suffix, length_suffix);
		} else {
			if (debug >= 3)
				printf("Suffix not found or not at end of file name!\n");
			return 0;
		}
	}
	
	// remove "_" from file names
	pos = filename_string.find('_');
	while(pos != std::string::npos) {
		filename_string.erase(pos, 1);
		pos = filename_string.find('_');
	}
	
	// we assume, that after the removal of prefix and suffix, there are only numbers left
	// FIXME/TODO: check, if this is really only a number
	index = atol(filename_string.c_str());
	
	if (debug >= 3)
		printf("File number is: %ld\n", index);
	
	return index;
}


int DAQDevice::get_file_list(std::string directory)
{
	DIR *dir;
	struct dirent *dir_entry;
	long index;

	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	if (debug > 2)
		printf("... from directory '%s'\n", directory.c_str());
	
	
	dir = opendir(directory.c_str());	
	if (dir == 0){
		printf("Directory %s is not existing\n", directory.c_str());
		throw(std::invalid_argument("Opendir failed"));
	}
	
	// read all directory entries
	while ((dir_entry = readdir(dir)) != NULL) {
		if (dir_entry->d_type == DT_DIR) {
			//
			// if we have a real directory (not "." or ".."), call this function recursively
			//
			if ( strcmp(dir_entry->d_name, ".") && strcmp(dir_entry->d_name, "..") ) {
				// add a '/' if necessary
				if (directory.rfind('/') == (directory.size() - 1) )
					get_file_list(directory + dir_entry->d_name);
				else
					get_file_list(directory + "/" + dir_entry->d_name);
			}
		} else if (dir_entry->d_type == DT_REG) {
			//
			// if we have a file, get its number and, if it's the right on, save name and number
			//
			index = getFileNumber(dir_entry->d_name);
			
			// if the file is one of the desired
			if (index != 0) {
				// add a '/' if necessary
				if (directory.rfind('/') == (directory.size() - 1) )
					dateien.insert(std::pair<long, std::string>(index, directory + dir_entry->d_name));
				else
					dateien.insert(std::pair<long, std::string>(index, directory + "/" + dir_entry->d_name));
			}
		}
	}
	
	closedir(dir);
	
	return 0;
}


void DAQDevice::getNewFiles() {
	std::string dataDir;
	char line[256];
	int i;
	int err;
	// Handler for marker file
	FILE *fmark;
	struct timeval lastTime;
	unsigned int lastPos;
	long lastIndex;
	std::map<long, std::string>::iterator dateien_pos;
	
	
	// Go through the list and find the next entry
	// Is there function that gives the distance?
	//
	// Sort the files?! Is there a function that returns a sorted list?
	
	
	// Get the last file (from pseudo inifile?)
	// Define template for the data file name or the folder name?!
	//

	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	processedData = 0; // Counter for the processed bytes
	
	dataDir = archiveDir + dataSubDir;
	
	
	// clear file list
	dateien.clear();
	
	// get new list of files
	get_file_list(dataDir);
	
	// print list of data files found
	if (debug > 2) {
		dateien_pos = dateien.begin();
		printf("\nList of data files in %s:\n", dataDir.c_str());
		printf("Number          List-Index Filename\n");
		for (i = 0; i < (int) dateien.size(); i++) {
			printf("%6d %19ld %s\n", i, dateien_pos->first, dateien_pos->second.c_str());
			dateien_pos++;
		}
		printf("\n");
	}
	
	
	// Read the last file from the last time the directory was processed
	err = 0;
	lastIndex = 1;
	sprintf(line, "%s.kitcube-reader.marker.%03d.%d", dataDir.c_str(), moduleNumber, sensorGroupNumber);
	filenameMarker = line;
	fmark = fopen(filenameMarker.c_str(), "r");
	if (fmark != NULL) {
		err = fscanf(fmark, "%ld %ld %ld %d", &lastIndex,  &lastTime.tv_sec, (long *) &lastTime.tv_usec, &lastPos);
		fclose(fmark);
	} else {
		// Create new marker file
		if (debug)
			printf("No marker file found -- try to create a new one  %s\n", filenameMarker.c_str());
		fmark = fopen(filenameMarker.c_str(), "w");
		if (fmark != NULL) {
			fprintf(fmark, "%ld %d %d %d\n", lastIndex, 0, 0, 0);
			fclose(fmark);
		}	
		
		lastTime.tv_sec = 0;
		lastTime.tv_usec = 0;
		lastPos = 0;
	}	
	//printf("Marker %s (err=%d, errno=%d): %d\n", filenameMarker.c_str(), err, errno, lastIndex);
	
	// Read the data from the files
	try {
		dateien_pos = dateien.begin();
		
		for (i = 0; i < (int) dateien.size(); i++) {
            
			if (dateien_pos->first == lastIndex) {
				// continue reading file from last call				
				if ((debug > 1) || (debug && !initDone))
					printf("#%03d: Continue reading file no. %03d, index %06ld, name %s:\n",
					       moduleNumber, i, dateien_pos->first, dateien_pos->second.c_str());
				
				// read header if first call of this function or if it failed below
				err = 0;
				if (initDone == 0)
					err = readHeader(dateien_pos->second.c_str());
				
				// if successful read data else cancel handling files
				if (err == 0) {
					initDone = 1;
					readData(dateien_pos->second);
				} else
					break;
			}
			
			if (dateien_pos->first > lastIndex) {
				// read new file from the beginning
                // Post handling of the old file ?!
                // TODO: E.g. Compress file
                //      Always compress, if module defines compress flag
                // 
				
				// Remove the pointers of the last file
				fmark = fopen(filenameMarker.c_str(), "w");
				if (fmark != NULL) {
					// Preserve the time stamp
					fprintf(fmark, "%ld %ld %ld %d\n",
						dateien_pos->first, lastTime.tv_sec, (long) lastTime.tv_usec, 0);
					fclose(fmark);
				}
				
                // TODO: Uncompress the file if necessary directly before handling
                
				if (debug){
					printf("#%03d:    Begin reading file no. %03d, index %06ld, name %s:\n",
					       moduleNumber, i, dateien_pos->first, dateien_pos->second.c_str());
				}
				
				// read header for each new file
				initDone = 0;
				err = readHeader(dateien_pos->second.c_str());
				
				// if successful read data else cancel handling files
				if (err == 0) {
					initDone = 1;
					readData(dateien_pos->second);
				} else
					break;
			}
			
			// Check if file has been completely read
			if (dateien_pos->first >= lastIndex) {
				if (!reachedEOF()) {
					if (debug >= 1) {
						printf("EOF not reached - continue with %s, position %d in next call\n",
							dateien_pos->second.c_str(), lastPos);	// FIXME: this gives wrong numbers, as lastPos gets updated in readData(...)
					}
					break;
				}
			}
			
			dateien_pos++;
		}
	} catch (std::invalid_argument &err){
		printf("Error: %s\n", err.what());
		// Reset database connection
		closeDatabase();
	}
	
	// Note:
	// The marker file is also updated in readData() with the lastest position in file pointer
	//
	
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


//=========== Working with the status list ============


#ifdef USE_MYSQL
MYSQL_RES *DAQDevice::getStatusList(const char *param, const char *cond){
	MYSQL_RES *res;
	std::string sql;
	

	sql = "SELECT "; 
	sql += param; 
	sql += " FROM `";
	sql += statusTableName; 
	sql += "`";
	if (cond > 0) { sql += " WHERE "; sql += cond; }
	sql += " ORDER BY `module`";
	
	if (mysql_query(db, sql.c_str())){
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// If this operation fails do not proceed in the file?!
		printf("Error: Unable to read from status list\n");
		throw std::invalid_argument("Reading status list failed");
	}	
	
	res = mysql_store_result(db);	
	return(res);
}


/*
void DAQDevice::displayStatusList(){
	
}


void DAQDevice::displayAlarmList(){

	// Display only a list if there is at least a single device 
	
}

 
*/ 


void DAQDevice::createEntry(const char *key){
	std::string sql;
	bool rows;
	
	
	sql = "SELECT * FROM `";
	sql += statusTableName;
	sql += "` WHERE `skey` = '";
	sql += key;
	sql += "'";
	
	if (mysql_query(db, sql.c_str())){
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// If this operation fails do not proceed in the file?!
		printf("Error: Unable to access database\n");
		throw std::invalid_argument("Create entry failed");
	}	 
	
	MYSQL_RES *res;
	res = mysql_store_result(db);
	rows = 0;
	if (res != NULL) {
		rows = mysql_num_rows(res);
		mysql_free_result(res);
	}
	
	// Create entry if not existing
	// TODO: Use unique key!?
	if (rows == 0){

		sql = "INSERT INTO `";
		sql += statusTableName;
		sql += "` ( `skey` ) VALUES ('";
		sql += key;
		sql += "') ";
		
		if (mysql_query(db, sql.c_str())){
			fprintf(stderr, "%s\n", sql.c_str());
			fprintf(stderr, "%s\n", mysql_error(db));
			
			// If this operation fails do not proceed in the file?!
			printf("Error: Unable to access database\n");
			throw std::invalid_argument("Create entry failed");
		}	 		
		
	}
	
	return;
}



int DAQDevice::setValue(const char *key, const char *assign){
	std::string sql;
	int rows;

	
	sql = "UPDATE `";
	sql += statusTableName + "` SET ";
	sql += assign;	
	sql += " WHERE `skey` = '";
	sql += key;
	sql += "'";
	
	//printf("SQL: %s\n", sql.c_str());
	
	if (mysql_query(db, sql.c_str())){
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// If this operation fails do not proceed in the file?!
		printf("Error: Unable to change parameter in status list\n");
		throw std::invalid_argument("Set parameter failed");
	}	 
			
	rows = mysql_affected_rows(db);
	if (debug > 3) printf("Affected rows = %d\n", rows);

	if (rows == 0) return 1;	
		
	return 0;
}

int DAQDevice::setValue(int module, const char *assign){
	std::string sql;
	char sValue[10];
	int rows;

	
	sql = "UPDATE `";
	sql += statusTableName + "` SET ";
	sql += assign;	
	sql += " WHERE `module` = ";
	sprintf(sValue, "%d", module);
	sql += sValue;
	
	if (mysql_query(db, sql.c_str())){
		fprintf(stderr, "%s\n", sql.c_str());
		fprintf(stderr, "%s\n", mysql_error(db));
		
		// If this operation fails do not proceed in the file?!
		printf("Error: Unable to change parameter  in status list\n");
		throw std::invalid_argument("Set parameter failed");
	}	 
	
	rows = mysql_affected_rows(db);
	if (debug > 3) printf("Affected rows = %d\n", rows);

	if (rows == 0) return 1;	
			
	return 0;
}


#endif


