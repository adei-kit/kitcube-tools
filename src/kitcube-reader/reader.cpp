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
#ifdef linux
#include <sys/vfs.h>
#endif
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



Reader::Reader(): SimpleServer(READER_PORT){
}


Reader::~Reader(){
}


void Reader::readInifile(const char *filename, const char *group){
	akInifile *ini;
	Inifile::result error;
	std::string value;
	float tValue;
	std::string tUnit;

	this->inifile = filename;
	this->tSampleFromInifile = 10000; // ms
	
	this->iniGroup = "Simulation";
	this->moduleType = "SimRandom";	
	

	ini = new akInifile(inifile.c_str(), stdout);
	//ini = new akInifile(inifile.c_str());
	if (ini->Status()==Inifile::kSUCCESS){

		ini->SpecifyGroup("Reader");
 
		//Try to read the module name from inifile if there no module name given
		if ((group == 0) || (group[0] == 0)){
			this->iniGroup = ini->GetFirstString("module", iniGroup.c_str(), &error);
		} else {
			iniGroup = group;
		}
	
		// Read sampling time
		tValue= ini->GetFirstValue("samplingTime", (float) tSampleFromInifile, &error);
		tUnit = ini->GetNextString("ms", &error);
		this->tSampleFromInifile = tValue;
		if ((tUnit == "sec") || (tUnit == "s")) this->tSampleFromInifile = tValue * 1000;
		if (tUnit == "min") this->tSampleFromInifile = tValue * 60000;
	
	
		// TODO: Guess type of the module ?!
		error = ini->SpecifyGroup(iniGroup.c_str());
		if (error == Inifile::kSUCCESS){
			this->moduleType = ini->GetFirstString("moduleType", moduleType.c_str(), &error);
		}

	}
	delete ini;

}



void Reader::runAsDaemon(bool flag){
	this->runDaemon = flag;
}



void Reader::runReadout(FILE *fout){
	int i;
	//int iSample;
	struct timeval tWait;
	struct timeval tStart;
	//char host[255];

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
 

	dev = (DAQDevice *) createDevice(moduleType.c_str());	
	dev->setDebugLevel(debug);

	dev->readInifile(this->inifile.c_str(), iniGroup.c_str());	
	dev->readAxis(this->inifile.c_str());
	
	
	// For every module one fre port is existing
	printf("_____Starting service for module %d at port %d_______________________________\n",
		   dev->getModuleNumber(), READER_PORT+dev->getModuleNumber());
	printf("Debug level = %d\n", debug);
	setPort(READER_PORT+dev->getModuleNumber()*10+dev->getSensorGroupNumber());
	init();
	
	// Set reference time and sampling time of the server loop
	// TODO: Read the start time from configuration 
	// TODO: Read sampling phase from configuration
	tStart.tv_sec = 1195487978;
	tStart.tv_usec = 100000;      // If there are two independant loops with thee same
	                              // sample rate there should be a phase shift between sampling
	setTRef(&tStart);
	enableTimeout(&tWait);


	// Start the server waiting for the records
	if (runDaemon)
		init_server();
	else
		init_server(fileno(stdin));
	
	fprintf(fout, "\n");

	//tRef.setEnd();

	dev->closeDatabase();
	delete dev;
}



int Reader::handle_timeout(){
	int i;
	//procDuration t;
	//int iSample;
	//struct timeval tWait;
	struct timeval t0, t1;
	struct timezone tz;


	// TODO: Check timing
	// The goal is to read data in periodic intervals
	// The reference time is given by the run start time
	//t.setStart();
	gettimeofday(&t0, &tz);


	// Analyse the timeing quality of the readout process
	//analyseTiming(&t);


	// TODO: Read data / Simulate data
	if (debug > 1) printf("=== Reading Data %10ld %06d=== \n", t0.tv_sec, t0.tv_usec);
	// Call rsync
	// TODO: Include also in the device class as the specific filenames are needed!!

	try {
		
		dev->copyRemoteData();
                fflush(stderr);
	
		// List all new files?!
		dev->getNewFiles();
		
	} catch (std::invalid_argument &err) {
		shutdown = true;
		printf("Error: %s\n", err.what());
	}
	
	gettimeofday(&t1, &tz);
	printf("Reader cycle duration %ldus\n", (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec));	
	
	// Get free disk space
	// Read the disk space from all devices. Report every device only once?!
	analyseDiskSpace(dev->getArchiveDir());

	// TODO: Write performance data to performance table?!
	//       Optionally add the performance data also to the data tables?!
	
	fflush(stdout);
	return(0);
}




int Reader::read_from_keyboard(){
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
			if (this->debug > 2) this->debug = 0;
			dev->setDebugLevel(debug);
			printf("Switched debug level to %d\n", this->debug);	
			break;
		
	  case 'h': // Help pages
	    printf("Data server -  commands:\n");
		printf("   d    Enable/disable debug output\n");
		printf("   q    Quit\n");
		printf("   s    Display status\n");
		printf("   z    Test database connection\n");
		printf(" SPACE  Add sleep command - load simulation\n");
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

void Reader::analyseTiming(struct timeval *t){

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
    if (fout) fprintf(fout, "%6d  | %ld.%06d \n", index, t->tv_sec, t->tv_usec);
  }


}


void Reader::displayStatus(FILE *fout){

  if (fout == 0) return;


  fprintf(fout, "\n");
  fprintf(fout,   "Server start     : %lds %dus\n", tRef.tv_sec, tRef.tv_usec);
  if (useTimeout){
    fprintf(fout, "Sampling time    : %lds %dus\n", tSample.tv_sec, tSample.tv_usec);
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


void Reader::analyseDiskSpace(const char *dir){

//
// Get size of file system 
// TODO:  How to handle the 64bit data types??? 
//        Change to 64bit version to be compatible...
struct statfs fs;
  
statfs(dir, &fs);

#ifndef linux	
// Not supported under Linux?!	
printf("Disk %s mounted to %s\n", fs.f_mntfromname, fs.f_mntonname);
#endif
printf("Total blocks  %12Ld %12.3f MByte (block size %d bytes)\n", fs.f_blocks, 
	   (float) fs.f_blocks / 1024 / 1024 * fs.f_bsize, fs.f_bsize);
printf("Free blocks   %12Ld %12.3f MByte %6.2f %s\n", fs.f_bfree, 
	   (float) fs.f_bfree / 1024 / 1024 * fs.f_bsize,
	   (float) fs.f_bfree / fs.f_blocks * 100, "%");

	
}


