/***************************************************************************
                          dataserver.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/



#include "dataserver.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <fstream>

#ifdef __WIN32__ // windows
#include <io.h>
#endif

#include <akutil/simpleserver.h>
#include <akutil/simplesocket.h>
#include <akutil/procDuration.h>
#include <akutil/akinifile.h>

#ifdef __WIN32__
#include <windows.h>
#include <akutil/timeval.h>
#endif

using std::map;
using std::pair;
using std::string;


#include <../kitcube-devices/simrandom.h>
#include <../kitcube-devices/mast.h>
#include <../kitcube-devices/createdevice.h>



DataServer::DataServer(): SimpleServer(DATASERVER_PORT){
}


DataServer::~DataServer(){
}


void DataServer::setAppName(const char *name){
	if (name > 0) this->appName = name;
}


int DataServer::getAppId(){
	return (this->appId);
}


void DataServer::readInifile(const char *filename, const char *group){
	akInifile *ini;
	Inifile::result error;
	std::string value;
	float tValue;
	std::string tUnit;
	int i;
	
	//printf("ReadInifile(%s)\n", filename);
	
	this->inifile = filename;
	this->tSampleFromInifile = 10000; // ms

	
	//ini = new akInifile(inifile.c_str(), stdout);
	ini = new akInifile(inifile.c_str());
	if (ini->Status()==Inifile::kSUCCESS){
	
		error = ini->SpecifyGroup(this->appName.c_str());
		if (error == Inifile::kFAIL){
			// TODO: Alternativly terminate the application here
			// it does not make sense to run an application that 
			// has no valid configuration ...
			
			this->appName = "DataServer"; // Default
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
    
        
		if ((group == 0) || (group[0] == 0)){
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
                // Default values
                this->iniGroup[i] = "Simulation";
                this->moduleType[i] = "SimRandom";
                
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
            error = ini->SpecifyGroup(iniGroup[i].c_str());            
            if (error == Inifile::kSUCCESS){
                this->moduleType[i] = ini->GetFirstString("moduleType", moduleType[i].c_str(), &error);
				this->moduleNumber[i] = ini->GetFirstValue("moduleNumber", 0, &error);
            } else {
                printf("Error: Section [%s] not found in inifile.\n", iniGroup[i].c_str());
                printf("\n");
                throw std::invalid_argument("Module not defined.");
            }

		}
		//ini->SpecifyGroup("Readout");
		//varianceClosedShutter = ini->GetFirstValue("varlevel", varianceClosedShutter, &error);
		//varianceOverflow = ini->GetNextValue(varianceOverflow, &error);
		//bgrunFile = ini->GetFirstString("runfile", bgrunFile.c_str(), &error);
		
	}
	delete ini;
}


void DataServer::runAsDaemon(bool flag){
	this->runDaemon = flag;
}


void DataServer::runReadout(FILE *fout){
	int i;
	//int iSample;
	struct timeval tWait;
	struct timeval tStart;
	//char host[255];
	//char dataName[255];
	int servicePort;

	if (shutdown) return; // Do not start the server!!!

	struct timeval t;
	struct timezone tz;

	gettimeofday(&t, &tz);
	
	//  Start run
	//tWait.tv_sec = 0;
	//tWait.tv_usec = 250000;
	//tWait.tv_sec = 1;
	//tWait.tv_usec = 500000;
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

    // For every module one free port is existing
    // For every reader application a separate port is used
    // It's buid from the base reader port + application ID
    servicePort = DATASERVER_PORT + this->appId;
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
        
		printf("%-20s : %d (for remote monitoring)\n", "Server port", servicePort);
		printf("\n");
	}
    
	
	this->dev = new  DAQDevice * [nModules];
	for (i = 0; i < nModules; i++) {
        dev[i] = (DAQDevice *) createDevice(moduleType[i].c_str());
        dev[i]->setDebugLevel(debug);
	
        dev[i]->readInifile(inifile.c_str(), iniGroup[i].c_str());
        // Get the sensor names from the configuration file 
        dev[i]->getSensorNames(dev[i]->sensorListfile.c_str());
        
        // Select the smallest sampling time ?!
        //dev[i]->getSamplingTime(&tWait);
	
        dev[i]->openFile();
        dev[i]->readHeader(); // Read the reference time from the data
    }

	if (debug > 2)
		init();
	else
		init(0); // No server messages
	
	// Set reference time and sampling time of the server loop
	// TODO: Read the start time from configuration
	// TODO: Read sampling phase from configuration
	tStart.tv_sec = 1195487978;
	tStart.tv_usec = 100000;      // If there are two independant loops with thee same
	                              // sample rate there should be a phase shift between sampling
	setTRef(&tStart);
	enableTimeout(&tWait);

	fflush(stdout);

	// Start the server waiting for the records
	if (runDaemon)
		init_server();
	else
		init_server(fileno(stdin));

	fprintf(fout, "\n");

	//tRef.setEnd();

	for (i = 0; i < nModules; i++) {
        dev[i]->closeFile();
		delete dev[i];
	}        
	delete [] dev;
}


int DataServer::handle_timeout(){
	int i;
	//procDuration t;
	//int iSample;
	//struct timeval tWait;
	struct timeval t, tNext;
	struct timezone tz;


	// TODO: Check timing
	// The goal is to read data in periodic intervals 
	// The reference time is given by the run start time
	//t.setStart();
	gettimeofday(&t, &tz);


	// Analyse the timeing quality of the readout process
	//analyseTiming(&t);


	// TODO: Read data / Simulate data
	if (debug > 2) printf("======= Loop / tSample: %10lds %06ldus =======\n", t.tv_sec, (long) t.tv_usec);
    for (i = 0; i < nModules; i++) {
        
        // Check if the sampling time is met?!
        dev[i]->getNextTimestamp(&tNext);
        if (timercmp(&t, &tNext, >)) {
            dev[i]->setLastTimestamp(&t);
            dev[i]->writeData();
        }
    }
    
	fflush(stdout);
	return(0);
}




int DataServer::read_from_keyboard(){
    int i;
	int err;
	char buf[256];
	//char dataName[255];
	
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
		if (this->debug > 2) this->debug = 0;
        for (i = 0; i < nModules; i++) {
            dev[i]->setDebugLevel(debug);
        }
		printf("Switched debug level to %d\n", this->debug);
		break;
		
	case 'h': // Help pages
		printf("Data server -  commands:\n");
		printf("   d    Enable/disable debug output\n");
		printf("   q    Quit\n");
		printf("   s    Display status\n");
		printf("   z    Test database connection\n");
		printf("   n    Create a new data file\n");
		printf(" SPACE  Add sleep command - load simulation\n");
		break;
		
	case 'n': // force new file
	case 'N':
        for (i = 0; i < nModules; i++) {
            dev[i]->openNewFile();
        }
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



void DataServer::executeCmd(int client, short cmd, unsigned int *arg, short n){
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

void DataServer::analyseTiming(struct timeval *t){

	unsigned long long tPlan;
	unsigned long long tNow;
	unsigned long long tDiff;
 

	tPlan = ((unsigned long long) tRef.tv_sec + lastIndex * tSample.tv_sec) * 1000000
		+ (unsigned long long) tRef.tv_usec + lastIndex * tSample.tv_usec;
	tNow = (unsigned long long) t->tv_sec * 1000000 + (unsigned long long) t->tv_usec;
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
		unsigned long long buf; 

		buf = tRef.tv_sec;
		if (fout) fprintf(fout, "%6d  | %ld.%06ld \n", index, t->tv_sec, (long) t->tv_usec);
	}
}


void DataServer::displayStatus(FILE *fout){

	if (fout == 0) return;

	fprintf(fout, "\n");
	fprintf(fout,   "Server start     : %lds %ldus\n", tRef.tv_sec, (long) tRef.tv_usec);
	if (useTimeout){
		fprintf(fout, "Sampling time    : %lds %ldus\n", tSample.tv_sec, (long) tSample.tv_usec);
		fprintf(fout, "Samples          : %d of %d -- %d missing\n", nSamples, lastIndex, lastIndex - nSamples);
		fprintf(fout, "Skipped          : %d last samples in a sequel\n", nSamplesSkipped );
	
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




// ========= QD Simukation ====================

void DataServer::simQDData(struct timeval t){
	char filename[256];
	struct tm *timestamp;
	int j;
	//bool running;
	FILE *fd;
	double analog[24];
	unsigned short data[24];
	double scaled;
	unsigned long nBuffer;
	//int fd2;
	
	// TODO: Measure length of operation?
	//        Precision of timing
	
	
	//qdPath = "/Users/kopmann/tmp/qd/";// Parameter fileserver (~/tmp/qd)
	//qdPath = "c:\\tmp\\";
	//qdNSensors= 24; // number of variables (default: 24)
	nBuffer= 10000;// length of file buffer (default 10000)
	// Sampling rate (default 100ms)
	// Test signal (sinus)
	// Frequency
	// Amplitude
	// Noise
	
	
	// Generate data
	
	// Generate file name
	timestamp = localtime((time_t *) &t.tv_sec);

/*
	// Filename according to K Petry
	sprintf(filename, "%sADC%05d_QDFFF_H%02dDM%02dS%02d_D%02d-M%02d-Y%04d.RAW",
		qdPath.c_str(), index%nBuffer,
		timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec,
		timestamp->tm_mday, timestamp->tm_mon+1, timestamp->tm_year+1900);
*/

	// Filenames accordig to M Heiduk / A Augenstein
	sprintf(filename, "%s%04d_%02d_%02d_%02d_%02d_%02d_%07d.k%2di16",
		qdPath.c_str(),
		timestamp->tm_year+1900, timestamp->tm_mon+1, timestamp->tm_mday,
		timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec,
		t.tv_usec * 10, qdNSensors);
 
	// Write file
	fprintf(stdout, "%s\n", filename); fflush(stdout);
 
	// Calculate simulted values 
	qd(analog, qdNSensors);

	// A-D Conversion
	for (j=0;j<qdNSensors;j++){
		scaled = ((analog[j] + 2.5) * 4096 / 5) - 1;
		if (scaled < 0) scaled = 0;
		if (scaled > 4095) scaled = 4095;
		data[j] = (unsigned short) scaled;
	}

#if defined __i386__ 
	// #error endian swap needed
	for (int i=0;i<qdNSensors;i++)  endian_swap(data[i]);
#endif

/*
	// Write test data to file
	fd2 = open(filename, O_RDWR | O_CREAT | O_BINARY);
	if (fd2>0) {
		write(fd2, data, qdNSensors * sizeof(short));
		close(fd2);
	} else {
		printf("Error writing %s (fd=%d, errno=%d)\n", filename, fd2, errno);
	}
*/
	// Open binary file for writing the data
	fd = fopen(filename, "wb");
	if (fd > 0) {
		size_t n; 
		n = fwrite(data,2,qdNSensors,fd);
		//printf("n = %d nQd = %d\n", n, qdNSensors);
		fclose(fd);
	} else {
		printf("Error writing %s (fd=%p, errno=%d)\n", filename, fd, errno);
	}

	// TODO Clear other file with the same file number

}


/** Simulate QD signal matrix.
  * Use PC time to calculate a sinus wave plus some noise.
  */
void DataServer::qd(double *analog, int n){
	for(int i=0; i<n;i++) analog[i] =  0.02 * (index%100 + i); 
	
	// Test values
	analog[0] = 0.0;
	analog[1] = -2.5;
	analog[2] = 2.5;
}
