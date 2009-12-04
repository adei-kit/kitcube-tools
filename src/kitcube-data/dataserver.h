/***************************************************************************
                          dataserver.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef DATASERVER_H
#define DATASERVER_H


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


#define DATASERVER_PORT 4900


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
      l�en?
    - Bleiben immer so viele Verbindungen liegen?
      Tw. wird eine weiterer Aufruf hierdurch sogar
      blockiert?!
    - Zeitnahmedaten mit in die Daten aufnehmen!!!
      �erarbeiten der display-Funktionen.
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

class DataServer : public SimpleServer  {
	public:
		/**  */
		DataServer();
		/**  */
		~DataServer();

		/** Read parameter from inifile */
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
		
		void simulate();
		
		/** Simulate data of the quench detection unit */
		void simQDData(struct timeval t);
		
		/** Generate analog data input */
		void qd(double *analog, int n);
		
		/** Write one long data file */
		//void simHeader(FILE *fdata, struct timeval t);
		//void simDataSample(FILE *fdata, struct timeval t);
		
		/** Index of the data files */
		int simDataIndex;
		
	private:
		/** Flag to start the server as daemon - without interactive input */
		bool runDaemon;
		
		std::string moduleName;
		std::string moduleType;
	
		DAQDevice *dev;
		
		/** File pointer for the data file */
		FILE *fdata;	
		
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
		
		std::string inifile;
		
		/** Record filename */
		std::string filename;
		
		/** Template for the record filename */
		std::string filenameTmpl;
		
		/** Basedir of the auger file */
		std::string basedir;
		
		/** Template for the basedir of the auger file */
		std::string basedirTmpl;
		
		/** FEalarm log file name */
		FILE *flog;
		
		/** Name of the logfile */
		std::string logfile;
		
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
