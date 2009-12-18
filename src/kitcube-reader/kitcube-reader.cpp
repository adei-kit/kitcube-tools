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
	int debug;
	
	
	printf("KITCube Data Reader\n");

	//
	// Default program switches
	//
	inifile = "kitcube.ini";
	runDaemon = false; // Run interactive as default
	debug = 2;
	
	//
	// Parse command line
	//	
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
				printf("\t-m <iniGroup>\tSelect the iniGroup name as used in the inifile\n");
				printf("\t-v <level>\tSet level of verbosity\n");	
				exit(0);
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
	
	
	if (iniGroup.length() >  0)
		printf("Starting readout for iniGroup %s (debug = %d)\n", iniGroup.c_str(), debug);
	
	
	try {
		// Read inifile
		data = new Reader();
		data->setDebugLevel(debug);
		data->readInifile(inifile.c_str(), iniGroup.c_str());
		data->runAsDaemon(runDaemon);
		
		// Start reading data file
		data->runReadout(stdout);
		
		
		// Close file, terminate
		delete data;
	} catch (std::invalid_argument &err){
		printf("Error: %s\n", err.what());
		fflush(stdout);
	}
	
	exit(0);
}
