// kitcude-data.cpp
//
// Generation of test data files for KITCube
// A. Kopmann 4.6.09
//

#include <stdlib.h>

#include "dataserver.h"


int main(int argc, char *argv[]){
	DataServer *data;
	int err;
	std::string iniGroup;
	std::string inifile;
	bool runDaemon;
	int debug;
	char *namePtr;
	std::string applicationName;
	//int posModulename;
	bool isLinkedApp;


	printf("KITCube Data Generator (build %s %s)\n", __DATE__, __TIME__);

	//
	// Default program switches
	//
	inifile = "kitcube.ini";
	runDaemon = false; // Run interactive as default
	debug = 2;
	isLinkedApp = false;
	
	//
	// Parse command line
	//
	
	// Get name of the application
	// Strip the path and the kitcube prefix
	//printf("Arguments: %d\n", argc);
	namePtr = strrchr(argv[0], '/');
	if (namePtr > 0)
		applicationName = namePtr + 1;
	else
		applicationName = argv[0];
	//printf("Running application %s\n", applicationName.c_str()); 
	if (strcmp(applicationName.c_str(),"kitcube-data") != 0) {
		isLinkedApp = true;
		namePtr = strstr(namePtr+1, "kitcube-");
		if (namePtr <  0){
			//printf("Warning: Kitcube prefix is missing in the application file name\n");
			throw std::invalid_argument("Kitcube prefix is missing in the application file name");
		} else {
			iniGroup = namePtr+8;
		}
	}
	
	
	while ((err = getopt(argc, argv, "dhi:m:v:")) > -1){
		// Use a colon to specify these option that require an argument
		
		//if (err > 0) printf("Option: %c %d\n", err, optind);
		//if ((err > 0) && (optarg > 0)) printf("Option Argument: %s\n",optarg);
		
		switch (err) {
		case 'd': // run as daemon
			printf("Run as daemon\n");
			runDaemon = true;
			debug = 0;
			break;
		case 'h': // help
			printf("\t-d\t\tRun as daemon without input from keyboard\n");
			printf("\t-i <inifile>\tSelect the inifile\n");
			if (!isLinkedApp) printf("\t-m <iniGroup>\tSelect the iniGroup name as used in the inifile\n");
			printf("\t-v <level>\tSet level of verbosity\n");	
			exit(0);
			break;
		case 'i': // inifile
			if (optarg > 0) inifile = optarg;
			break;				
		case 'm': // run in console
			if (!isLinkedApp){
				if (optarg > 0) iniGroup = optarg;
			}
			break;
		case 'v': // set debug level (verbosity)
			if (optarg > 0) debug = atoi(optarg);
			break;
		}
	}
	
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
		if (!isLinkedApp)
			iniGroup = argv[1 + nOpts];
	}
	
	
	if (iniGroup.length() >  0)
		printf("Start production of data for iniGroup %s\n", iniGroup.c_str());
	
	
	// Read inifile
	try {
		data = new DataServer();
		data->setDebugLevel(debug);
		data->readInifile(inifile.c_str(), iniGroup.c_str());
		data->runAsDaemon(runDaemon);
		
		// Start simulation of data file
		data->runReadout(stdout);
		
		
		// Close file, terminate
		delete data;
	} catch (std::invalid_argument &err){
		printf("Error: %s\n", err.what());
		fflush(stdout);
	}
	
}
