/***************************************************************************
                          reader.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#include "reader.h"

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>

#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h> // BSD ???

Reader::Reader(): SimpleServer(READER_PORT){

	appId = 0;
	appName = "Reader";
	nModules = 0;
	
	rx = 0;
	tx = 0;
	disk_avail_gb = 0;
    
    nError = 0; // Error counter
}


Reader::~Reader(){
	
	delete [] iniGroup;
	delete [] moduleType;
	delete [] moduleNumber;

}


void Reader::setAppName(const char *name){
	if (name > 0) this->appName = name;
}


int Reader::getAppId(){
	return (this->appId);
}


void Reader::readInifile(const char *filename, const char *group){
	akInifile *ini;
	Inifile::result error;
	std::string value;
	float tValue;
	std::string tUnit;
	int i;
	
	
	this->inifile = filename;
	this->tSampleFromInifile = 10000; // ms
	
	if (debug > 2) {
		ini = new akInifile(inifile.c_str(), stdout);
	} else {
		ini = new akInifile(inifile.c_str()); // Skip output
	}
	if (ini->Status()==Inifile::kSUCCESS){

		
		error = ini->SpecifyGroup(this->appName.c_str());
		if (error == Inifile::kFAIL){
			// TODO: Alternativly terminate the application here
			// it does not make sense to run an application that 
			// has no valid configuration ...
			
			this->appName = "Reader"; // Default
			ini->SpecifyGroup(this->appName.c_str());
			
		}
		if (debug > 2) printf("[%s]\n", this->appName.c_str());

		// Get reader id
		this->appId = ini->GetFirstValue("id", 0, &error);
	
		// Read sampling time
		tValue= ini->GetFirstValue("samplingTime", (float) tSampleFromInifile, &error);
		tUnit = ini->GetNextString("ms", &error);
		this->tSampleFromInifile = tValue; 
		if ((tUnit == "sec") || (tUnit == "s")) this->tSampleFromInifile = tValue * 1000;
		if (tUnit == "min") this->tSampleFromInifile = tValue * 60000;
		
		
		//Try to read the group name from inifile if there no group name given
		if ((group == 0) || (group[0] == 0)){

			//printf("Reading modules from inifile\n");
			// Get the number of groups from the inifile
			ini->GetFirstString("module", "", &error);
			
			nModules = 0;
			error = Inifile::kSUCCESS;
			while (error == Inifile::kSUCCESS){
				ini->GetNextString("", &error);
				nModules++;
			}
			
			//printf("Found n = %d modules\n", nModules);
			
			// Allocate the arrays for all parameters
			// TODO: Check if space has been allocated before?!
			this->iniGroup = new std::string [nModules];
			this->moduleType = new std::string [nModules];
			this->moduleNumber = new int [nModules];
			this->dev = new  DAQDevice * [nModules];
			
			//printf("Reading the modules again\n");
			
			// Read modules names
			this->iniGroup[0] = ini->GetFirstString("module", "Simulation", &error);
			for (i=1;i<nModules;i++){
				this->iniGroup[i] = ini->GetNextString("", &error);
			}
			
			
		} else {
			
			// Use the module from the command line - only single module possible
			nModules = 1;
	
			this->iniGroup = new std::string [nModules];
			this->moduleType = new std::string [nModules];
			this->moduleNumber = new int [nModules];
			
			this->iniGroup[0] = group;
		}
		
		
		// Read type of the modules
		for (i=0;i<nModules;i++){
			//printf("Reading module definition [%s]\n", iniGroup[i].c_str());
			error = ini->SpecifyGroup(iniGroup[i].c_str());
			if (error == Inifile::kSUCCESS){
				this->moduleType[i] = ini->GetFirstString("moduleType",
							iniGroup[i].c_str(), &error);
				this->moduleNumber[i] = ini->GetFirstValue("moduleNumber", 0, &error);
			} else {
				printf("Error: Section [%s] not found in inifile.\n", iniGroup[i].c_str());
				printf("\n");
				throw std::invalid_argument("Module is not defined");
			}
		}
		
		// Print summary
		printf("List of modules (n=%d)  :\n", nModules);
		for (i=0;i<nModules;i++){
			printf("#%03d:  %s, type %s\n", moduleNumber[i], iniGroup[i].c_str(), moduleType[i].c_str());
		}
		printf("\n");
		
	
	}
	delete ini;

}


void Reader::runAsDaemon(bool flag){
	this->runDaemon = flag;
}


void Reader::runReadout(){
	int i;
	//int iSample;
	struct timeval tWait;
	//struct timeval tStart;
	//char host[255];
	int servicePort;
    
	if (shutdown) return; // Do not start the server!!!
	
	
	struct timeval t;
	struct timezone tz;
	
	gettimeofday(&t, &tz);
	
	//  Start run
	//tWait.tv_sec = 0;
	//tWait.tv_usec = 250000;
	tWait.tv_sec = tSampleFromInifile / 1000;
	tWait.tv_usec = (tSampleFromInifile % 1000) * 1000; 
	
	// Initialize the QD simulation
	// Set sampling time
	//tWait.tv_sec = qdTSample / 1000;
	//tWait.tv_usec = (qdTSample % 1000) * 1000;
	
	
	//tRef.setStart(); // Set reference time
	// The first measurement point should be 5sec in the future
	//tRef.setStart((unsigned long) (tRef.getStartSec() - tSample + 5.));
	//tStart.tv_sec = tRef.getStartSec();
	//tStart.tv_usec = tRef.getStartMicroSec();
	
	// Clear the variables for the timing analysis
	timingN = 0;
	timingSum = 0;
	timingSum2 = 0;
	timingMin = 0;
	timingMax = 0;
 

	// Define the logging parameters
	//  1 Tsync  Duration of one reader cycle (rsync + file read + db storage)  [s]
	//  2 Free disk space [Bytes]
	//  3 Free disk space [%]
	//  4 Network transfer received [Bytes]
	//  5 Network transfer send [Bytes]
	// 
	
	if (debug) printf("\n");
	if (debug) printf("______Starting system logging _______________________________\n"); 
	log = new SysLog();
	
	log->setNData(8);
	log->setDebugLevel(debug);
	log->setAppId(this->appId); // Every application needs it's own log table

	log->readInifile(this->inifile.c_str(), "SysLog");	
	log->readAxis(this->inifile.c_str());
	log->getSensorNames(log->sensorListfile.c_str());
	log->setConfig(0,"T scheduler");
	log->setConfig(1,"T data transfer");
	log->setConfig(2,"T data storage");
	log->setConfig(3,"Free disk space (GB)");
	log->setConfig(4,"Free disk space (%)");
	log->setConfig(5,"Datarate stored");
	log->setConfig(6,"Datarate received");
	log->setConfig(7,"Datarate send");
	// TODO/FIXME: reduce the number of calls to setConfig!

	// For every reader application a separate port is used
	// It's buid from the base reader port + application ID
	servicePort = READER_PORT + this->appId;
	setPort(servicePort);
	
	
	// Display configuration
	if (debug) {
		printf("\n");
		printf("______Starting service for  %d  module(s)_______________________________\n", nModules+1);
		printf("%-20s : %d ms\n", "Sampling time", tSampleFromInifile);
		
		for (i = 0; i < nModules; i++) {
			printf("Module %2d            : %s,  type  %s\n",
			       i + 1, iniGroup[i].c_str(), moduleType[i].c_str());
		}
		printf("Module %2d            : Performance module, type SysLog, %d items\n",
		       nModules+1, log->getNSensors());

		printf("%-20s : %d (for remote monitoring)\n", "Server port", servicePort);
		printf("\n");
	}
	
	this->dev = new  DAQDevice * [nModules];
	for (i = 0; i < nModules; i++) {
		dev[i] = (DAQDevice *) createDevice(moduleType[i].c_str());
		if (!dev[i]) {
			printf("Error: Module definition of type %s is unkown\n", moduleType[i].c_str());
			printf("\n");
			throw std::invalid_argument("Unknown module type");
		}
		
		dev[i]->setDebugLevel(debug);
		dev[i]->setAppId(appId);
		
		dev[i]->readInifile(this->inifile.c_str(), iniGroup[i].c_str());	
		dev[i]->readAxis(this->inifile.c_str());
		try {
			dev[i]->getSensorNames(dev[i]->sensorListfile.c_str());
		} catch (std::invalid_argument &err) {
			// Show the header of the first file if available
			dev[i]->getNewFiles();
			throw std::invalid_argument("Configuration missing");			
		}
	}
	
	
	if (debug > 2)
		init();
	else
		init(0); // No server messages
	
	
	// Set reference time and sampling time of the server loop
	// TODO: Read the start time from configuration
	// TODO: Read sampling phase from configuration
	
	// The reader doesn't need to be synchronous to the last call !!!
	//tStart.tv_sec = 1195487978;
	//tStart.tv_usec = 100000;	// If there are two independant loops with thee same
	//				// sample rate there should be a phase shift between sampling
	
	t.tv_usec += 100000; // Start with the first loop after 100ms
	if (t.tv_usec >= 1000000){
		t.tv_usec = t.tv_usec % 10000000;
		t.tv_sec += 1;
	}
	setTRef(&t);
	//setTRef(&tStart);
	//setTRef();
	enableTimeout(&tWait);


	// Start the server waiting for the records
	if (runDaemon)
		init_server();
	else
		init_server(fileno(stdin));
	
	if (debug) printf("\n");

	//tRef.setEnd();

	for (i = 0; i < nModules; i++) {
		dev[i]->closeDatabase();
		delete dev[i];
	}
	delete [] dev;
	
	log->closeDatabase();
	delete log;
}


int Reader::handle_timeout(){
	int i;
	//procDuration t;
	//int iSample;
	//struct timeval tWait;
	struct timeval t0, t2, t3, t4;
	struct timezone tz;
	unsigned int nData;
	unsigned int tCycle;
	unsigned int tStorage;
	long int tScheduler; 
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// TODO: Check timing
	
	//
	// TODO: Check the timing -- at ipepdvadei there are sometimes to calls for the 
	//       same sample time !!!
	//
	
	// The goal is to read data in periodic intervals 
	// The reference time is given by the run start time
	//t.setStart();
	gettimeofday(&t0, &tz);
	log->updateTimestamp(&t0);
	
	
	// Analyse the timeing quality of the readout process
	tScheduler = analyseTiming(&t0);
	log->updateData(0, (float) tScheduler / 1000.); // ms
	
	
	// TODO: Read data / Simulate data
	if (debug > 1) {
		printf("     _____________________________________________________\n");
		printf("____/__Reading Data %12ld %06ld (sample %06d)___\\_______ \n",
		       t0.tv_sec, (long) t0.tv_usec, nSamples);
	}
	
	nData = 0;
	tStorage = 0;
	try {
		
		for (i = 0; i < nModules; i++) {
			gettimeofday(&t2, &tz);
			fflush(stderr);
			
			// List all new files?!
			dev[i]->getNewFiles();
			nData += dev[i]->getProcessedData();
			
			if (debug > 1)
				printf("Processed data %d Bytes\n", dev[i]->getProcessedData());
			gettimeofday(&t3, &tz);
			
			tStorage += (t3.tv_sec - t2.tv_sec)*1000000 + (t3.tv_usec-t2.tv_usec);
		}
		
		// Complete cycle time
		gettimeofday(&t4, &tz);
		tCycle = (t4.tv_sec - t0.tv_sec)*1000000 + (t4.tv_usec-t0.tv_usec);
		if (debug) {
			// only one line of output 
			// Not every sample time?!
		}
		if (debug > 1) {
			printf("______Performance______________________________________________\n");
			printf("Processed data: %8d Bytes     Cycle duration: %8d us\n",
			       nData, tCycle);
			printf("Scheduler     : %8ld us       Data Storage  : %8d us\n",
			       tScheduler, tStorage);
		}
		//log->updateData(0, (float) tCycle / 1000000.); // Sensor 0: Cycle time
		log->updateData(1, (float) tStorage / 1000.); // ms
		
		
		// Get free disk space
		// Read the disk space from all devices. Report every device only once?!
		analyseDiskSpace(dev[0]->getArchiveDir());
		if (nSamples > 1){
			log->storeSensorData();
			log->saveFilePosition(0,0,t0);
		}
        
        
        // Clear error flag
        nError = 0;
		
	} catch (std::invalid_argument &err) {
        if (nError < 3) {
            nError +=1;
            printf("\n==> Waiting for next interval\n");
        } else {
		  shutdown = true;
		  printf("Error: %s\n", err.what());
        }
	}
	

	if (debug > 1) {
		printf("\n");
	}
	if (debug) fflush(stdout);
	return(0);
}


int Reader::read_from_keyboard(){
	int i;
	int err;
	char buf[256];
	
	// Handle keyboard events
	//handleKeyboard(floop, kb);
	
#ifdef __GNUC__
	err = read (fileno(stdin), buf, 256);
#else // windows ?
	err = _read (fileno(stdin), buf, 256);
#endif
	
	if (err > 0){
		//keyboard_cmds(floop, buf);
		buf[err] = 0; // Add string terminator
		//printf("%s", buf); fflush(stdout);
		
		// Command interpreter for stdin
		switch (buf[0]){
		case 'd': // Enable / display debug
			//if (fout == stdout) fout = 0;
			//else fout = stdout;
			
			this->debug++;
			if (this->debug > 5) this->debug = 0;
			for (i=0;i<nModules;i++) dev[i]->setDebugLevel(debug);
			printf("Switched debug level to %d\n", this->debug);
			printf("\n");
			break;
			
		case 'h': // Help pages
			printf("Data server -  commands:\n");
			printf("   d    Enable/disable debug output\n");
			printf("   q    Quit\n");
			printf("   s    Display status\n");
			printf("   z    Test database connection\n");
			printf(" SPACE  Add sleep command - load simulation\n");
			printf("\n");
			break;
			
		case 'q': // Shutdown server
		case 'Q':
			shutdown = true;
			break;
			
		case ' ': // Simulate high load?!
			SLEEP(600); //ms
			break;
			
		case 's':
		case 'S':
			displayStatus(stdout);
			//displayActiveSockets(stdout);
			break;
			
		// Create new Experiment - means completely new database
		// Create all database tables
		case 'e': // new experiment
			//createExperiment();
			break;
			
		// Start RUN
		case 'r': // Start run
			//createRun();
			break;
			
		// Stop RUN
		case 'p': // Stop run
			//stopRun();
			break;
			
		}
		
	}
	
	return(0);
}


void Reader::executeCmd(int client, short cmd, unsigned int *arg, short n){
	//int i;
	int err;
	unsigned int res[10], *buf;
	//float *pFloat;
	short acklen;
	
	
#ifdef debug
	fprintf(fout, "ExecuteCmd: %d \n",cmd);
#endif
	
	// local function call
	buf = res;  // used, if the result is not longer than 2 words
	buf[0] = 0; // no error
	acklen = 1;
	try { switch(cmd) {
		
/*
		case 0: // save data


#ifdef USE_DYN_BYTEORDER
         // Change the order in the short array
         // There is no need to change the bool values.
         // They are stored internal as int values and
         // use 4byte per value!
         if (FD_ISSET(client, &change_byteorder_fd_set)){
           int i;
           int len;
           unsigned int *ptr;

           len = sizeof(bg->current) / sizeof(unsigned int);
           ptr = (unsigned int *) (bg->current);
           for (i=0; i< len; i++)
             endian_prepare_short(ptr[i]);

           len = sizeof(bg->threshold) / sizeof(unsigned int);
           ptr = (unsigned int *) (bg->threshold);
           for (i=0; i< len; i++)
             endian_prepare_short(ptr[i]);

           len = sizeof(bg->hitrate) / sizeof(unsigned int);
           ptr = (unsigned int *) (bg->hitrate);
           for (i=0; i< len; i++)
             endian_prepare_short(ptr[i]);
         }
#endif



       //acklen=1;
       break;

    case  1: // Get readout parameter
       buf[1] = tRef.getStartSec();
       pFloat = (float *) &buf[2]; // sizeof(float) == 4 !!!
       *pFloat = tSample;
       buf[3] = selStore && selDisplay; // The clients need to read
                                        // every parameter for display + storage
       acklen = 4;

       break;

*/

    case 41: // shutdown bgloop-recorder


       // Set shutdown flag
       shutdown = true;
       acklen=1;
       break;

  }  } catch (...) {
#ifdef debug
      fprintf(fout, "read_from_client: Error occured during execution of bg-command?!\n");
#endif
      buf[0] = 1; // Error occured
      acklen = 1;
  }


  // return the result
  err = writeData(client,buf,acklen);

#ifdef debug
  fprintf(fout, "read_from_client: status %ld, err=%d\n",buf[0],err);
#endif


}



// ========= periodically sampling Unit ============

long int Reader::analyseTiming(struct timeval *t){
	unsigned long tPlan;
	unsigned long tNow;
	long tDiff;


	tPlan = ((unsigned long) tRef.tv_sec + lastIndex * tSample.tv_sec) * 1000000
		+ (unsigned long) tRef.tv_usec + lastIndex * tSample.tv_usec;
	tNow = (unsigned long) t->tv_sec * 1000000 + (unsigned long) t->tv_usec;
	tDiff = tNow - tPlan;
	
	if (timingN == 0){
		timingMin = tDiff;
		timingMax = tDiff;
	}

	timingN++;

	timingSum += tDiff;
	timingSum2 += tDiff * tDiff;

	if (tDiff > (unsigned) timingMax)  timingMax = tDiff;
	if (tDiff < (unsigned) timingMin)  timingMin = tDiff;


	if (fout > 0){
		unsigned long buf;
		
		buf = tRef.tv_sec;
		if (debug >2) printf("%6d  | %ld.%06ld \n", index, t->tv_sec, (long) t->tv_usec);
	}
	
	return(tDiff);
}


void Reader::displayStatus(FILE *fout){
	
	if (fout == 0) return;
	
	fprintf(fout, "\n");
	fprintf(fout,   "Server start      : %lds %ldus\n", tRef.tv_sec, (long) tRef.tv_usec);
	if (useTimeout){
		fprintf(fout, "Sampling time     : %lds %ldus\n", tSample.tv_sec, (long) tSample.tv_usec);
		fprintf(fout, "Samples           : %d of %d -- %d missing\n", nSamples, lastIndex, lastIndex - nSamples);
		fprintf(fout, "Skipped           : %d last samples in a sequel\n", nSamplesSkipped );
		
		if (timingN > 0){
			double var;
			var = (double) timingSum2 / timingN - (double) timingSum / timingN * timingSum / timingN;
			if (var >= 0)
				fprintf(fout, "Sample error     : %lld +- %8.1f  (%lld .. %lld) \n", timingSum/timingN,
					sqrt(var),  timingMin, timingMax);
			else
				fprintf(fout, "Sample error     : %lld +- %8s  (%lld .. %lld) \n", timingSum/timingN,
					"err", timingMin, timingMax);
		}
		
		// TODO: Display error status of sensor groups
		
		
	} else {
		fprintf(fout, "Sampling         : Disabled\n");
	}
	fprintf(fout, "\n");
}


void Reader::analyseDiskSpace(const char *dir){
	//
	// Get size of file system 
	// TODO:  How to handle the 64bit data types??? 
	//        Change to 64bit version to be compatible...
	struct statfs fs;
	double disk_size_gb;
	double disk_avail_percent;
	double disk_space_diff;
	double t_diff;
	uint64_t diff_rx;
	uint64_t diff_tx;
	double rx_rate, tx_rate;
	
	
	if (debug > 2)
		printf("\033[34m_____%s_____\033[0m\n", __PRETTY_FUNCTION__);
	
	
	// get time since last call to this function
	t_diff = (double) time_now.tv_sec + (double) time_now.tv_usec / 1000000.;
	gettimeofday(&time_now, NULL);
	t_diff = (double) time_now.tv_sec + (double) time_now.tv_usec / 1000000. - t_diff;
	
	
	// save "old" available disk space
	disk_space_diff = disk_avail_gb;
	
	// get file system statistics
	statfs(dir, &fs);
	
	// evaluate file system stats
	disk_size_gb = (double) fs.f_blocks / 1073741824. * fs.f_bsize;	// disk size in GB
	disk_avail_gb = (double) fs.f_bavail / 1073741824. * fs.f_bsize;	// available disk space in GB
	disk_avail_percent = (double) fs.f_bavail / fs.f_blocks * 100.;	// available disk space in %
	
	// difference in available disk space in MB
	disk_space_diff = (disk_avail_gb - disk_space_diff) * 1024.;
	
	if (debug >= 2) {
		printf("Total blocks : %lld equals to %.3f GB (block size: %ld B)\n",
		       fs.f_blocks, disk_size_gb, (long) fs.f_bsize);
		printf("Avail. blocks: %lld equals to %.3f GB or %.2f %%\n",
		       fs.f_bavail, disk_avail_gb, disk_avail_percent);
		printf("Change in avail. disk space: %+.3f MB, rate: %.3f MB/s (Sampling time %.1f s)\n",
		       disk_space_diff, disk_space_diff / t_diff, t_diff);
	}
	
	log->updateData(2, disk_avail_gb);
	log->updateData(3, disk_avail_percent );
	log->updateData(4, - disk_space_diff / t_diff );	// MB/s
	
#ifndef linux // DARWIN
	//
	// Read network statistics
	//
	const char *iface = "en1"; // Name of the device
	struct ifaddrs *ifap, *ifa;
	struct if_data *ifd = NULL;
	int check = 0;
	
	if (getifaddrs(&ifap) < 0) {
		if (debug)
			printf("getifaddrs() failed.. exiting.\n");
		return;
	}
	
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if ((strcmp(ifa->ifa_name, iface) == 0) && (ifa->ifa_addr->sa_family == AF_LINK)) {
			ifd = (struct if_data *) ifa->ifa_data;
			check = 1;
			break;
		}
	}
	freeifaddrs(ifap);
	
	if (check == 0) {
		if (debug)
			printf("Requested interface \"%s\" not found.\n", iface);
		return;
	} else {
		
		diff_rx = ifd->ifi_ibytes -rx;
		diff_tx = ifd->ifi_obytes -tx;
		
		rx = ifd->ifi_ibytes;
		tx = ifd->ifi_obytes;
		
		rx_rate = (double) diff_rx / 1048576. / t_diff;	// MB/s
		tx_rate = (double) diff_tx / 1048576. / t_diff;	// MB/s
		
		/*
		strncpy(ifinfo.name, iface, 32);
		ifinfo.rx = ifd->ifi_ibytes;
		ifinfo.tx = ifd->ifi_obytes;
		ifinfo.rxp = ifd->ifi_ipackets;
		ifinfo.txp = ifd->ifi_opackets;
		ifinfo.filled = 1;
		 */
	}
#endif
	
#ifdef linux
	//
	// Linux
	//
	const char *iface = "eth0"; // Name of the device
	
	FILE *fp;
	char procline[512], *proclineptr, ifaceid[33];
	int check;
	
#define PROCNETDEV "/proc/net/dev"
	
	if ((fp = fopen(PROCNETDEV, "r")) == NULL) {
		if (debug)
			printf("Error: Unable to read %s.\n", PROCNETDEV);
		return;
	}
	
	strncpy(ifaceid, iface, 32);
	strcat(ifaceid, ":");
	
	check = 0;
	while (fgets(procline, 512, fp) != NULL) {
		if (strstr(procline, ifaceid) != NULL) {
			/* if (debug)
			 printf("\n%s\n", procline); */
			check = 1;
			break;
		}
	}
	fclose(fp);
	
	if (check == 0) {
		if (debug)
			printf("Requested interface \"%s\" not found.\n", iface);
		return;
	} else {
		diff_rx = rx;
		diff_tx = tx;
		
		/* get rx and tx from procline */
		proclineptr = strchr(procline, ':');
		sscanf(proclineptr + 1, "%lu %*u %*u %*u %*u %*u %*u %*u %lu", &rx, &tx);
		
		if (debug >= 4)
			printf("Received bytes: %lu, transmitted bytes: %lu\n", rx, tx);
		
		diff_rx = rx - diff_rx;
		diff_tx = tx - diff_tx;
		
		rx_rate = (double) diff_rx / 1048576. / t_diff;	// MB/s
		tx_rate = (double) diff_tx / 1048576. / t_diff;	// MB/s
	}
#endif
	
	if (debug >= 2)
		printf("Network device %s: rcvd %llu B at %.3f MB/s, sent %llu B at %.3f MB/s\n",
			iface, diff_rx, rx_rate, diff_tx, tx_rate);
	
	// Write data to the syslog structure
	log->updateData(5, rx_rate);	// MB/s
	log->updateData(6, tx_rate);	// MB/s
}
