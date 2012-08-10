// kitcude-reader.cpp
//
// Reading only new data from an increasing data file for KITCube
// A. Kopmann 4.6.09
//


#include <stdlib.h>

#include "reader.h"


int main(int argc, char *argv[]){
	Reader *data;
	int err;
	std::string iniGroup;
	std::string inifile;
	bool runDaemon;
	bool printHelp;
	int debug;
	char *namePtr;
	std::string filename;
	std::string applicationName;
	//int posModulename;
	bool isLinkedApp;	
	struct timeval t0,t1;
	struct timezone tz;
	struct tm *t;
	
	
	gettimeofday(&t0, &tz);
	t = localtime( &t0.tv_sec); //Alternative: t = gmtime( &t0.tv_sec);
	printf("KITcube Data Reader (build %s %s)  %30s\n", __DATE__, __TIME__, asctime(t));
	
	//
	// Default program switches
	//
	inifile = "kitcube.ini";
	runDaemon = false; // Run interactive as default
	printHelp = false;
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
		filename = namePtr + 1;
	else
		filename = argv[0];
	applicationName = filename;
	if (debug > 3) printf("Running application %s\n", applicationName.c_str()); 

/*
	if (strcmp(filename.c_str(),"kitcube-reader") != 0) {
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
	
	
	while ((err = getopt(argc, argv, "dhi:m:v:")) > -1){
		// Use a colon to specify these option that require an argument
		
		//if (err > 0) printf("Option: %c %d\n", err, optind);
		//if ((err > 0) && (optarg > 0)) printf("Option Argument: %s\n",optarg);
		
		switch (err) {
		case 'd': // run as daemon
			runDaemon = true;
			debug = 0;
			break;
		case 'h': // help
			printHelp = true;
			break;
		case 'i': // inifile
			if (optarg > 0) inifile = optarg;
			break;
		case 'm': // run in console
			if (optarg > 0) iniGroup = optarg;
			break;
		case 'v': // set debug level (verbosity)
			if (optarg > 0) debug = atoi(optarg);
			//printf("Set verbosity to %d\n", debug);
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
		iniGroup = argv[1 + nOpts];
	}
	
	
	// Display help
	if (printHelp){
		printf("\t-d\t\tRun as daemon without input from keyboard\n");
		printf("\t-i <inifile>\tSelect the inifile\n");
		printf("\t-m <iniGroup>\tSelect the iniGroup name as used in the inifile\n");
		printf("\t-v <level>\tSet level of verbosity\n");	
#ifdef HAVE_PYTHON_H
        printf("\tPython interface active\n");
#else           
        printf("\tPython interface disabled\n");
#endif          
		exit(0);
	}

	if (debug > 1) {
		//printf("Debug level = %d (0 .. 5 -- low ... high verbosity)\n", debug);
		if (runDaemon) printf("Run in daemon mode (no stdin/stdout)\n");
		
		if (iniGroup.length() >  0)
			printf("Starting readout for iniGroup %s (debug = %d)\n", iniGroup.c_str(), debug);	
	}

	
	try {
		// Create the reader process
		data = new Reader();
		data->setDebugLevel(debug);
		//if (isLinkedApp) 
		data->setAppName(applicationName.c_str());
		data->readInifile(inifile.c_str(), iniGroup.c_str());
		data->runAsDaemon(runDaemon);
		
		// Start reading data file
		data->runReadout();
		
		// Close file, terminate
		delete data;
	} catch (std::invalid_argument &err){
		printf("Error: %s\n", err.what());
		fflush(stdout);
	}
	


	if (debug) {
		gettimeofday(&t1, &tz);
		
		t = localtime( &t0.tv_sec); //Alternative: t = gmtime( &t0.tv_sec);
		printf("Terminating %-38s%30s", applicationName.c_str(), asctime(t));
		t = localtime( &t1.tv_sec); //Alternative: t = gmtime( &t0.tv_sec);
		printf("%80s\n\n", asctime(t));
	}
	exit(0);
}
