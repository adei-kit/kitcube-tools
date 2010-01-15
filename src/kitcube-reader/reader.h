/***************************************************************************
                          reader.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef READER_H
#define READER_H


#include <cstdio>
#include <string>
#include <map>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif // of USE_MYSQL

#include <akutil/simpleserver.h>
#include <akutil/procDuration.h>

#ifdef __WIN32__
#include <windows.h>
#include <akutil/timeval.h>
#endif


#define READER_PORT 6100



class SimpleSocket;
class SimpleServer;
class procDuration;
class DAQDevice; 

/** Recorder process for background data.
  * The background data is collected at the
  * telescopes and transfered to the central
  * data collection point (this class).
  *
  * The background data is collected using a client server concept. At every
  * connected telescope a readout process is started that afterwards autonomuosly
  * sends the data to the server running at the eyePC writing the data to the file.

  * The background data is stored in text format. The data of every parameter
  * introduced by a header line containing certain parmeters necessary to
  * interprete the data. There is a master header for a complete set of background
  * data.
  *
  * The following box shows a sample call of the background recorder.
@verbatim

  TsBackgroundLoop *bgrec;
  Pbus *thePbus;

  Pbus::init(inifile.c_str());


  // Create recorder
  bgrec = new TsBackgroundLoop();
  bgrec->readInifile(inifile.c_str());

  bgrec->setLogfile(stdout);

  // Set parameter: Period length, selected parameter, file, ....
  bgrec->setParameter(name, tSample, 0, selRecord, selDisplay, runType.c_str());  // no output to file, take data every 5s

  // Run background recorder
  // The readout process is started at all connected telescopes
  bgrec->runReadout(stdout);

  delete bgrec;
@endverbatim
  *
  *
  *
  *
  * @todo Streamer methoden einfhren, es erscheint
      unsauber, die Methoden mit zu kopieren !!!
    @todo Wie kann man eine verlorene Verbindung
      erkennen? Wie kann der Server-Verbindungen
      len?
    - Bleiben immer so viele Verbindungen liegen?
      Tw. wird eine weiterer Aufruf hierdurch sogar
      blockiert?!
    - Zeitnahmedaten mit in die Daten aufnehmen!!!
      verarbeiten der display-Funktionen.
    - Definition des Befehlssatzen in einer Struktur,
      die von beiden Seiten verwendet wird!
    - Verwenden von ROOT-Trees zum Abspeichern der Daten
    - Integration in Pbusdaemon / oder
      ein separates Programm, das vom Pbusdaemon aufgerufen
      wird?!
    - Display-Programm, das sich ebenfalls an den Server
      ankoppelt.

    - Test mit echter Hardware: Werden die Daten korrekt bertragen
    - Test mit mehreren Teleskopen
    - Test mit automatischem Start ber Pbusdaemon
    - Erweiterung des TsBackground-Formates fr die neuen Variablen
    - Dokumentation der Background-Loop (Konzept / Start / Konfiguration / ...)
    - Speichern im Root-Format, Aufteilung in Branches
    - Beispielhaftes Display-Modul (ncurses) entwerfen
    - Histogramming-Modul / Stern-Monitor ankoppeln

  * @todo The recorder does not work with a connection started via
      ssh tunnel. The readout process at the mirrorpc's seem to have problems
      to connect to the proper recorder server?!
  *
  */

class Reader : public SimpleServer  {
public:
	/**  */
	Reader();

	/**  */
	~Reader();

	/** Read parameter from inifile.
	  * The module definition will be read from the argument list, afterwards
	  * from the reader section in the inifile and at last the default name (Simulation)
	  * selected. */
	void readInifile(const char *inifile, const char *module = 0);

	/** Set operation without interactive inputs */
	void runAsDaemon(bool flag = true);
		
	/** Get time until next sample and it's id */
	void getNextSample(int *iSample, double *tWait);
	
	void getNextSample(int *iSample, struct timeval *tWait);

	/** Handler for the timeout */
	int handle_timeout();

	/** Keyboard interface for interactive control
	  * (only for compatibility reasons */
	int read_from_keyboard();

	/** Execute the command */
	void executeCmd(int client, short cmd, unsigned int *arg, short n);

	/** Start the server and all the client recording the
	  * telescopes background data.
	  */
	void runReadout(FILE *fout);

	/** Analyse timing */
	void analyseTiming(struct timeval *t);
	
	/** Display status of readout process */
	void displayStatus(FILE *fout);
	
	/** Read the free disk space from the filesystem that is used for data storage. */
	void analyseDiskSpace(const char *dir);

	/** Get index / time stamp of a folder or a filename.
	  * The index field is defined by a starting tag directly before the numerical values.
	  * The length of the field can be given by a final string of the length of the field or
	  * all following digit will be taken.
	  * In case of success the parsed numerical values will cut out of the passed filename.
	  */
	// unsigned long getIndex(char *filename, char *firstTag, char *lastTag=0, int len = 0, char *next = 0);
	
	/** Get list of new files */
	//void getNewFiles();
	
	/** Read new data from the file */
	//void readData(const char *dir, const char *filename);
	
private:
	/** Flag to start the server as daemon - without interactive input */
	bool runDaemon;

	/** Sampling time (ms) */
	int tSampleFromInifile;

	/** Module name s given in sensor list */
	std::string moduleName;

	/** module type describes the implementation, the class */
	std::string moduleType;

	/** a group in the *.ini file */
	std::string iniGroup;

	/** */
	DAQDevice *dev;

	/** File pointer for the data file */
	//FILE *fdata;

	/** Number of sensor groups */
	int nGroups;

	/** Number of samples (of timeout loop) */
	unsigned long samplesN;

	/** Number of analysed samples */
	long long timingN;
	
	/** Sum of all deviations from the planned sample time */
	long long timingSum;
	
	/** Sum of squares from the the planned sample time */
	long long timingSum2;
	
	/** Minimal deviation */
	long long timingMin;
	
	/** Maximal deviation */
	long long timingMax;

	/** Number of last samples skipped, because there was no vaild data / timestamp */
	int nSamplesSkipped;

	/** cfp server socket */
	SimpleSocket *cfpSock;

	std::string inifile;

	/** Record filename */
	std::string filename;

	/** Template for the record filename */
	std::string filenameTmpl;

	/** Basedir of the auger file */
	std::string basedir;
	
	/** Template for the basedir of the auger file */
	std::string basedirTmpl;
	
	/** Location Id */
	int locationId;

	/** Name of the location */
	std::string location;

	/** Flag to switch between unique and relative telescope id's */
	int uniqueId;
	
	/** File descriptor for back*/
	FILE *bgFile;

	/** Flag for writing to database */
	bool record;

	/** Reference time of the run */
	//procDuration tRef;


	/** Selected parameter for recording */
	int selStore;

	/** Selected parameter for display */
	int selDisplay;

	/** Selected telescopes */
	int selTelescopes;

	/** Limit of allows deviation from the mean variance
	* The limit is given in percent from the mean value.
	* This value is intended to be used with the noisy pixel
	* analysis. */
	int limitVar;

	/** Number of samples used for calculation of the satistics */
	unsigned long statSamples;

	/** Offset used for the calculation of the pedestal */
	unsigned long statOffset;

	/** Bits mask to mark the faulty pixel that should not
	* be included in the calculation of the mean values */
	unsigned long disabledPixel[20];

	/** Matrix of the pixel status
	*/
	unsigned long pixelStatus[20][24];

	/** Number of the corresponding FDDAS run */
	unsigned long fddasRunId;

	/** Filename template for the run mode */
	std::string bgrunFile;
	
	/** Filename template for the calibration mode */
	std::string bgcalFile;

	/** Sampling time for run mode */
	int bgrunTSample;

	/** Sampling time for calibration mode */
	int bgcalTSample;
	

	/** FEalarm log file name */
	FILE *flog;

	/** Name of the logfile */
	std::string logfile;

	/** Mode of operation (-1 unkown, 0 cal/test mode = record every thing, 1
	* run mode = record only open telescopes
	*/
	int runMode;

	/** Run type (run,  cal, test) */
	std::string runType;

	/** Run mode of telescope (0 no, 1 yes) */
	int telActive[24];

	/** Time stamp of last received data set */
	unsigned long lastDataSec[24];

	/** Flag to indicate colums with hitrate overflow */
	int hitrateOverflow[24];

	/** Last difference between PC and HW second counter */
	unsigned long lastSecondDiff[24];

	/** Level of the mean variance (do not include obvious wrong pixel!?) */
	int varianceLevel[24];

	/** Level for closed shutters */
	float varianceClosedShutter;

	/** Level for too much light */
	float varianceOverflow;

	/** Array with the last variances */
	int lastVariance[24];

	/** Array with the actual variances */
	int actualVariance[24];

	/** Last time stamp */
	unsigned long lastTimeStamp;

	/** Actual time stamp */
	unsigned long actualTimeStamp;
	
	/** Mean threshold pedestal difference */
	int thresholdLevel[24];

	/** Flag for all missing telesopes */
	int missingTel[24];

	/** Error level of the telescope (0 Ok, 1 warning, 2 error ) */
	int errorLevel[24];

	/** Error in one of the telescopes */
	int errorGeneral;

	/** List of faulty pixel (0 Ok, > 1 errors) */
	//int faultyPixelMask[24][20][22];

	/** Last dead time */
	unsigned long long lastDeadtime[24];

	/** Deadtime active */
	int deadtimeActive[24];

	/** Duration of database storage */
	int tDatabase;
	
	/** Path to the QD data */
	std::string qdPath;

	/** Name of configuration file */
	std::string qdConfig;

	/** Sampling time (ms) */
	int qdTSample;

	/** Number of values (max 24) */
	int qdNSensors;
};

#endif
