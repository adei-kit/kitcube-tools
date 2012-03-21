// kitcude-monitor.cpp
//
// Maintaining kitcube-tools for KITCube
// A. Kopmann 2.11.11
//


#include <stdlib.h>

#include "reader.h"
#include "../kitcube-devices/daqdevice.h"

#include <akutil/akinifile.h>
#include <akutil/simplesocket.h>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif 

/** Formatting the module list 
  *
  * Current date  3.11.2011 16:55:12 
  * Date      | Module    | Reader |  Last Data | Alarm | Status   | Ping
  *           | no        | id     |            |       |
  *     1:54  | 100       | 1      |       1:59 |       | Running  | Ok / failed
  *
  * Give always relative times
  * Second command w absolute times?!
  *
  *
  * List of item contains: 
  *   module, moduleName, secData, appId,       sec, 
  *   status, alarmLimit, alarm,    alarmEnable, sensorGroup
  * 
  * 
  */

void displayModules(MYSQL_RES *res, const char *title = "device list"){
	
	MYSQL_ROW row;

	struct timeval t;
	struct timezone tz;
	struct tm *ts;
	long tdb;
	long tdiff;
	std::string alarm;
    int alarmLimit;
	int alarmEnable;
	
	// Get current time
	gettimeofday(&t, &tz);	
	ts = gmtime( &t.tv_sec);
	
	// Display the result
	// TODO: Disply date instead of time, if delay is more than 24h
	// TODO: Add outformat XML
	
	printf("KITcube status --  %-41s  UTC %26s", title, asctime(ts));
	printf("\n");
	printf(" %-58s || %-58s\n", "Device", "Served by application");
	printf(" %4s | %4s | %4s | %10s | %12s | %9s || %6s | %4s | %15s\n",  
		   "No", "dev", "grp", "last data", "delayed by", "status",
		   "Name", "no", "comment");
	while((row = mysql_fetch_row(res))) {
		
		printf(" %4s |%5s |%5s | ", row[0], row[1], row[9]);
		sscanf(row[2], "%ld", &tdb);
		tdiff = t.tv_sec - tdb;
		ts = gmtime( &tdb);
		printf("%4d:%02d:%02d | ", ts->tm_hour, ts->tm_min, ts->tm_sec);
		printf("%6ld:%02ld:%02ld |", tdiff/3600, (tdiff/60)%60, tdiff%60);
		alarm = "";
		alarmLimit = atoi(row[6]);
		alarmEnable = atoi(row[8]);
		if (atoi(row[8]) == 0) alarm = "Disabled"; // Alarm is disabled
		//secData+alarmLimit < UNIX_TIMESTAMP() && alarmLimit > 0 && alarmEnable = 1"
		else if ((tdiff > alarmLimit) && (alarmLimit > 0) && alarmEnable) alarm = "ALARM"; // Has alarm
		printf("%10s || ", alarm.c_str());
		printf("%6s | ", row[11]);
		printf("%4s | ", row[3]);
		sscanf(row[4], "%ld", &tdb);
		tdiff = t.tv_sec - tdb;
		ts = gmtime( &tdb);
		//printf("%4d:%02d:%02d | ", ts->tm_hour, ts->tm_min, ts->tm_sec);
		//printf("%6ld:%02ld:%02ld | ", tdiff/3600, (tdiff/60)%60, tdiff%60);
		// TODO: Check if reader is delayed !!!
		printf("%15s", row[5]);
		printf("\n");
	}
		
}


/** Monitoring the kitcube-tools
  *
  */

int main(int argc, char *argv[]){
	int err;
	
	std::string inifile = "kitcube.ini";
    std::string tmpDir = "/home/cube/tmp";
	int timeout = 200; // msec ???
	std::string fullname;
	std::string filename;
	std::string applicationName = "reader";
    std::string line;
	int port = READER_PORT;
	int appId;
	int moduleNo;
	std::string key;
	std::string settings;
	std::string host = "localhost";
	std::string cmd = "stat";

	bool getAppIdFromName = false;
	//bool isLinkedApp = false;
	bool connectReader = false;
	bool printHelp = false;
	bool test = false;
	int debug = 0;
	int rtrn = 0;

    akInifile *ini;
    Inifile::result error;
    

	
	
	while ((err = getopt(argc, argv, "a:hH:i:n:p:tv:")) > -1){
		// Use a colon to specify these option that require an argument
		
		//if (err > 0) printf("Option: %c %d\n", err, optind);
		//if ((err > 0) && (optarg > 0)) printf("Option Argument: %s\n",optarg);
		
		switch (err) {
			case 'a': // application name
				if (optarg > 0) fullname = optarg;
				getAppIdFromName = true;
				connectReader = true;
				break;
			case 'h': // help
				printHelp = true;
				break;
			case 'H': // host name
				if (optarg > 0) host = optarg;
				break;
			case 'n': // application number
				if (optarg > 0) port = READER_PORT + atoi(optarg);
				connectReader = true;
				break;				
			case 'i': // inifile
				if (optarg > 0) inifile = optarg;
				break;
			case 'p': // port number (alternative to application id)
				if (optarg > 0) port = atoi(optarg);
				connectReader = true;
				break;
			case 't': // test display only, no action
				printf("Note: Test mode -- no changes are performed\n\n");
				test = true;
				break;
			case 'v': // set debug level (verbosity)
				if (optarg > 0) debug = atoi(optarg);
				break;
		}
	}
		
	appId = port -  READER_PORT;
	
	// Get the applcation number from inifile
	if (getAppIdFromName){
		char *namePtr;
		
		if (debug) printf("Searching for application id for %s\n", fullname.c_str());
		namePtr = strrchr((char *) fullname.c_str(), '/');
		if (namePtr > 0)
			filename = namePtr + 1;
		else
			filename = fullname;
		applicationName = filename;
/*		
		if (strcmp((char *) filename.c_str(),"kitcube-reader") != 0) {
			isLinkedApp = true;
			namePtr = strstr((char *) filename.c_str(), "kitcube-");
			if (namePtr <  0){
				//printf("Warning: Kitcube prefix is missing in the application file name\n");
				throw std::invalid_argument("Kitcube prefix is missing in the application file name");
			} else {
				applicationName = namePtr+8;
				applicationName[0] = toupper(applicationName[0]);
			}
		}	
*/		
		// Read application id from inifile
		ini = new akInifile(inifile.c_str());
		if (ini->Status()==Inifile::kSUCCESS){
			error = ini->SpecifyGroup(applicationName.c_str());
			if (error == Inifile::kFAIL){
				// TODO: Alternativly terminate the application here
				// it does not make sense to run an application that 
				// has no valid configuration ...
				
				applicationName = "Reader"; // Default
				ini->SpecifyGroup(applicationName.c_str());
				
			}
			if (debug > 2) printf("[%s]\n", applicationName.c_str());
			
			// Get reader id
			appId = ini->GetFirstValue("id", 0, &error);	
			port = READER_PORT + appId;
		}
		
		delete ini;
	}
	if (debug) printf("Application Id = %d \n", appId);
	
	
	// 2b - Handle non-option arguments
	int nOpts;
	// int i;
	
	//for (i=0; i<argc; i++) printf("%s ", argv[i]);
	//printf("First non option is %d\n", optind);
	
	// getopt will sort the argument in the end!!!
	// All options are sortet to the beginning.
	nOpts = optind - 1;
	
	// Parse the remaining arguments
	if (argc - nOpts >= 2 ){
		cmd = argv[1 + nOpts];
	}

	moduleNo = -1;
	if (argc - nOpts >= 3 ){
		moduleNo = atoi(argv[2 + nOpts]);
	}
	
    key = "";	
	if (argc - nOpts >= 3 ){
		key = argv[2 + nOpts];
	}
	
	if (argc - nOpts >= 4 ){
		settings = argv[3 + nOpts];
	}
	
	
	
	// Parse command line
	// kitcube-monitor -h <host> -p<port> -a<appId> <cmd> 
	//
	// Commands: 
	//		quit		Terminate kitcube-reader
	//	
	//		stat		Display status of all devices in the kitcube
	//		alarm		Display only devices that are in alarm state
	//		newalarm	Display only devices that have a new alarm
	//
	
	// Display help
	if (printHelp){
		printf("KITCube Monitor (build %s %s)\n\n", __DATE__, __TIME__);
		
		printf("kitcube-monitor <opts> <cmd>\n");
		printf("\t-i <inifile>\tSelect the inifile\n");
		printf("\t-H <host>\tSelect the iniGroup name as used in the inifile\n");
		printf("\t-p <port>\tSet level of verbosity\n");
		printf("\t-t\t\tOperate in test mode, no data is changed.\n");
		printf("\t-a <appId>\tApplication Id\n");	
		printf("\t<cmd>\tKnown commands: \n");
		printf("\t   quit\t\tTerminate reader process\n");
		printf("\t   status\tDisplay module list\n");
		printf("\t   alarm\tDisplay modules with delyed data transfer\n");
		printf("\t   newalarm\tDisplay changes in the alarm status\n");
		printf("\t   alarmEnable <module>\tEnable the alarm handler\n");
		printf("\t   alarmDisable <module>\tDisable the alarm handler\n");
		printf("\t<cmd>\tCommands for scripting:\n");	
		printf("\t   create <key> <assign>\tCreate new entry\n");		
		printf("\t   set <key> <assign>\t\tSet parameter\n");		
		printf("\t   set <module> <assign>\tSet parameter\n");		
		exit(0);
	}

	
	DAQDevice *dev;
	SimpleSocket *s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	bool accessReader = false;
	bool accessStatuslist = false;
	
	
	// Connect to server
	// Set flag if not available
	//printf("Connecting to kitcube-%s at %s:%d\n", 
	//	   applicationName.c_str(), host.c_str(), port);
	if (connectReader){
		try {
			if (debug) printf("Connecting to kitcube-%s at %s:%d\n", 
							  applicationName.c_str(), host.c_str(), port);
			s = new SimpleSocket( (char *) host.c_str(), port, timeout);
			if (debug) printf("Done\n");
			accessReader = true;
			
			// Commands for reader app
			if (cmd == "quit"){
				s->remoteCall(41);
				printf("Server has been terminated\n"); 
			}

			// Close socket and database connection
			if (accessReader) delete s;
			
		} catch (...) {
			if (debug) printf("No connection to Reader process #%d\n", appId);
		}
	
		
		exit(0);
	}
	
#ifdef USE_MYSQL
    
	// Read tmp directory (used for database availablitity flag)
    ini = new akInifile(inifile.c_str());
    if (ini->Status()==Inifile::kSUCCESS){
        error = ini->SpecifyGroup("Common");
        if (error == Inifile::kSUCCESS){
            tmpDir = ini->GetFirstString("tmpDir", tmpDir.c_str(), &error);	
        }
    }
    delete ini;
    
    
	try {
		// Connect to database 
		// Set flag if not available
		
		dev = new DAQDevice();
        		
		dev->setDebugLevel(debug);
		dev->readInifileCommon(inifile.c_str());

		
		if (debug) printf("Connecting to kitcube status list (%s)\n", inifile.c_str());
		dev->connectDatabase();	
		if (debug) printf("Done\n");
		accessStatuslist = true;
        
		// Commands for status list
		try {
			if (cmd == "alarmEnable") {
				
				if (moduleNo <0) {
					printf( "Module number required\n");
				} else {
					dev->setValue(moduleNo, "alarmEnable=1");
					printf("Alarm monitoring enabled for module %d\n", moduleNo);
				}
				
			} else if (cmd == "alarmDisable") {
				
				if (moduleNo <0) {
					printf( "Module number required\n");
				} else {
					dev->setValue(moduleNo, "alarmEnable=0");
					printf("Alarm monitoring disabled for module %d\n", moduleNo);
					printf("Warning: Don't forget to enable the alarm monitoring later !!!\n");
				}
				
			} else if (cmd == "alarm") {
				
				// Show all devices where no new data is transfered
				res = dev->getStatusList(
						"module, moduleName, secData, appId, sec, status, alarmLimit, alarm, alarmEnable, sensorGroup, skey, appName",
						"secData+alarmLimit < UNIX_TIMESTAMP() && alarmLimit > 0 && alarmEnable = 1");		
				displayModules(res, "ALARM list");
				
				if (mysql_num_rows(res) == 0)
					printf("\n  --- all configured modules are running well --- \n"); 
		
				mysql_free_result(res);
				
			} else if (cmd == "newalarm") {
				
				// Check for alarms 
				bool changes = false;


                // Database is available, clear database persistence flag
                // Check if database was not available
                line = "test -f ";
                line += tmpDir + "/NODATABASE";
                if (system(line.c_str()) == 0){
                
                    printf("\n\n");
                    printf("Database is available again - KITcube status reporting will continue\n");
                    printf("The reports should be carefully checked on effects caused by the absent database\n");                
                    printf("\n\n");
                    
                    changes = true; 
                    rtrn = 1;
     
                    // clear flag   
                    line = "rm -f ";
                    line += tmpDir + "/NODATABASE";
                    //printf("CMD: %s\n", line.c_str());
                    system(line.c_str());
                }
                
                
				// Show all devices where no new data is transfered
				
				res = dev->getStatusList(
					"module, moduleName, secData, appId, sec, status, alarmLimit, alarm, alarmEnable, sensorGroup, skey, appName",
					"secData+alarmLimit < UNIX_TIMESTAMP() && alarmLimit > 0 && alarm = 0 && alarmEnable = 1");		
				
				if (mysql_num_rows(res)){
					
					// Print list of new alarms
					// Set alarm flag in the status list
					displayModules(res, "NEW ALARMS");
					
					// Set the alarm flag
					mysql_data_seek(res, 0);
					while((row = mysql_fetch_row(res))) {
						if (!test) dev->setValue(row[10], "alarm=1");
					}
					
					changes = true;
					rtrn = 1;
				}				
				mysql_free_result(res); 
				
				
				res = dev->getStatusList(
					"module, moduleName, secData, appId, sec, status, alarmLimit, alarm, alarmEnable, sensorGroup, skey, appName",
					"secData+alarmLimit > UNIX_TIMESTAMP() && alarmLimit > 0 && alarm = 1 && alarmEnable = 1");		
				
				if (mysql_num_rows(res)){
					// Print list of cleared alarms
					// Clear alarm flag in the status list
					displayModules(res, "CLEARED ALARMS");
					
					// Clear the alarm flag
					mysql_data_seek(res, 0);
					while((row = mysql_fetch_row(res))) {
						if (!test) dev->setValue(row[10], "alarm=0");
					}
					
					changes = true;
					rtrn = 1;					
				}
				mysql_free_result(res); 
				
				if (!changes) 
					printf("\n  --- no changes in the alarm status of the modules ---\n");
			
				
			} else if (cmd == "set") {
				printf("Entry %s: Set %s\n", key.c_str(), settings.c_str());
				err = dev->setValue(key.c_str(), settings.c_str());
				if (err) {
					printf("Error: Failed to set values -- is the key available?\n");
					rtrn = 1;
				}
			
				
			} else if (cmd == "create") {
				printf("Create entry %s\n", key.c_str());
				dev->createEntry(key.c_str());
				
				
			} else { 
				// Display status (default operation)
				res = dev->getStatusList(
						"module, moduleName, secData, appId, sec, status, alarmLimit, alarm, alarmEnable, sensorGroup, skey, appName");
				displayModules(res);
				mysql_free_result(res); 
			}
			
	} catch (...) {
		//printf("No access to Statuslist\n");
	}
#endif
		
		
#ifdef USE_MYSQL
		if (accessStatuslist) delete dev;
#endif	
		
	}
	catch (std::invalid_argument &err) {
		printf("Error: %s\n", err.what());
        
        // Write persistence flag that the database failure has been notified !!!
		if (cmd == "newalarm") {
            
            // Check if database problem has already been reported
            line = "test ! -f ";
            line += tmpDir + "/NODATABASE";
            //printf("CMD: %s\n", line.c_str());
            //printf("Test = %d\n", system(line.c_str()));
  
            if (system(line.c_str()) == 0) {
                // Report the error 
                
                printf("\n\n");
                printf("Database is unavailable - KITcube status can not be reported\n");
                printf("There might be more things broken than the database\n");                
                printf("\n\n");
                
                rtrn = 1; 
                
            } else {
                
                printf("\n  --- database is still unreachable ---\n");
             
            }
            
            // Set the database flag
            line = "touch ";
            line += tmpDir + "/NODATABASE";
            //printf("CMD: %s\n", line.c_str());
            system(line.c_str());
            
        }
        
	}
	
	printf("\n");
	exit(rtrn);	
}


