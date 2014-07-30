/***************************************************************************
                          daqdevice.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef DAQDEVICE_H
#define DAQDEVICE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_PYTHON_H
#include <Python.h>
#endif // 

#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <sys/time.h>
#include <map>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif // of USE_MYSQL

#include <akutil/akinifile.h>


struct axisDef {
	std::string name;
	std::string desc;
	std::string unit;
	float rangeMin;
	float rangeMax;
	bool isNew;
	int id;
};


struct sensorType {
	std::string name;		// KITCube sensor name
	std::string comment;		// Description of the sensor (wo aggregation information)
	std::string longComment;	// Description of the sensor (w aggregation information)
	std::string unit;		// Unit from header -- not used
	std::string type;		// Estimated axis type -- not used
	int aggregation;		// type of aggregation
	int axis;			// Number of axis used for the sensor (from axis definition in kitcube.ini)
    std::string axis2_name;
    int axis2;          // Number of the second axis (2d data)
    int axis2_idx;           // index on second dimension /* CM */
    float axis2_val;    // value of second dimension /* CM */
	float height;			// Postion of the sensor (height in meters)
	std::string data_format;	// <scalar>, <profile>, <2D>, <3D>
    int size;           // Number of element for the sensor (skalar = 1, ...)
};


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  */

class DAQDevice {
public:
	/**  */
	DAQDevice();
	
	/**  */
	virtual ~DAQDevice();

	/** Get the number of sensors */
	int getNSensors();
	
	
	/** The function is called before reading the configuration from the inifile.
	  * Use this fucntion in the module specific implementation to override the standard defaults */
	virtual void setConfigDefaults();

	
	/** Read common parameters from inifile 
	  * (e.g. database access, directory struction, ...) */
	void readInifileCommon(const char *inifile);
	
	/** Read common and module dependant parameters from inifile */
	virtual void readInifile(const char *inifile, const char *group = 0);

	/** Read axis definition from inifile and update the axsi definition in the database */
	void readAxis(const char *inifile);
	
	/** Read the univeral KITCube sensor names from the configuration file *.sensors.
	  * If the sensor configuration file is not found in the configuration directory 
	  * a template file for all sensors is created and the execution stops until the 
	  * sensor names are filled in. 
	  *
	  * Format of the *.sensors file:
	  * Every sensor starts in a new line. The fields are separated by <TAB>s
	  * The entry of each sensor contains  
	  * - the number of the sensor in the module (1 .. nSensors)
	  * - Description of the sensor. If different aggregation types are stored also this
	  *   information is needed
	  * - KITCube sensor name (fill in)
	  * - Name of the axis to be used with the sensors. The axis needs to be defined in the
	  *   general configuration file (default: kitcube.ini, fillin).
	  *
	  */
	void getSensorNames(const char *sensor_list_file_name);
	
	
	/** Return the samping time of the device */
	void getSamplingTime(struct timeval *time);
	
    /** Get the last sampling time **/
    void getLastTimestamp(struct timeval *time);
    
    /** Get the last sampling time **/
    void getNextTimestamp(struct timeval *time);
    
    /** Set last timestamp */
    void setLastTimestamp(struct timeval *time);
    
	void createDirectories(const char *path);
	
	/** Set filename for reading data */
	virtual void openFile(const char *filenname);
	
	/** Open file for writing data */
	virtual void openFile();
	
	/** Close the actual output file and open a new one fro writing */
	virtual void openNewFile();
	
	virtual void closeFile();
	
	/** Get the path to the archive directory */
	const char *getArchiveDir();
	
	virtual const char *getDataDir();
	
	/** Implements the data filename convention of the DAQ module. */
	virtual const char *getDataFilename();

	/** Open the database and create all required tables 
	  * The required tables are axislist, sensorlist, statuslist and
	  * the data table for the current DAQ device.
	  */
	virtual void openDatabase();
	
    /** Crete status table, if necessary */
    void openStatusTab();
    
	/** Connect to the database "<project>_active". The database is
	  * created, if not existing. 
	  * The function is included from openDatabase(). */
	void connectDatabase();  
	
	/** Create data table, if it doesn't exist.
	  * The function is called by openDatabase(). */
	virtual int create_data_table();
	
	virtual void closeDatabase();
	
	/** Get time until next sample and it's id */
	virtual int readHeader(const char *header);

	/** Get time until next sample and it's id */
	virtual void readHeader();

	virtual void writeHeader();
    
    /** Read the data from the current line of the data file. 
      * @return -1 no data found, skip storage 
      *          0 sucess, store data, 
      *          1 read another line 
      */
	virtual int parseData(char* line, struct timeval* l_tData, double *sensorValue);
	
	/** Update or create the entry for this module in the status table */
	void registerStatusTab(const char *status = 0, const char *comment = 0);
	
	virtual void storeSensorData();
	
	virtual void readData(std::string full_filename);

	/** Update the date of the current data set and replace it by the current date. 
	  * The function is used by writeData to generate simulated data with current time stamp
	  * using the sample data file */
	virtual void updateDataSet(unsigned char* buf);
	
	virtual void writeData();
	
	/** Get the ranking number from a filename. The returned number can be used
	  * to order files by their date. Smaller numbers are processed before larger ones.
	  * The function needs also to check if the given filename is valid according to the
	  * file name specification of the device. In case of violation zero is returned.
	  */
	virtual long getFileNumber(char* filename);
	
	/** Get list of new files */
	void getNewFiles(const char *dir = 0);
	
	/** Return the module number */
	unsigned int getModuleNumber(); 
	
	/** Return the number of the sensor group */
	virtual unsigned int getSensorGroup();
	
	/** Get a unix time stamp (in UTC) frm a date and time string of the form 
	  *  dd.mm.yyy  and hh:mm:ss  */
	unsigned long getTimestamp(const char* date, const char* time);
	
	/** Set debug level */
	void setDebugLevel(int level);
	
	/** Set the application ID */
	virtual void setAppId(int id);
	
	/** Get the application ID */
	int getAppId();
	
	/** Reached EOF during reading data in readData() */
	bool reachedEOF();
	
	/** Get the number of processed data in the last cycle */
	unsigned int getProcessedData();
	
	std::string sensorListfile;
		
	/** Save the file pointers to the marker file 
	  * The items stored are the index of the processed file (see getFileNumber), 
	  * the file pointer in data file and the timestmp of the latest valid data set */
	void saveFilePosition(long lastIndex, unsigned long currPos, struct timeval &timestamp);
	
	/** Restore the file pointers from the marker file */
	void loadFilePosition(long &lastIndex, unsigned long &lastPos, struct timeval &timestamp);
	
	/** Reset the file pointers in the marker file */
	void resetFilePosition();
	
	
#ifdef USE_MYSQL	
	/** Get all modules from the status list */
	MYSQL_RES * getStatusList(const char *param, const char *cond = 0);

	/** Create an entry in the status list */
	void createEntry(const char *key);

	/** Modify an parameter in the status list. The parameter assign
	  * contains a list a assignments for known parameters. */
	int setValue(const char *key, const char *assign);

    /** Get a value from the database */
    int getValue(const char *key, const char *parameter, std::string *value);
    
	/** Modify an parameter in the status list. The parameter assign
	  * contains a list a assignments for known parameters. 
	  * The function modifies all entries with the specified 
	  * module number */
	int setValue(int module, const char *assign);
#endif		
    
    
	
protected:
	/** Reset file pointer in marker file */
	bool reset;
	
	/** Use the microsecond ticks when storing the data (default: yes). */
	bool useTicks;
	
	/** Length of the header block in the data file */
	int lenHeader;
	
    /** Length of the header block in number of lines 
      * The number of lines can be given alternatively to the size
      * of the header in Bytes (lenHeader) */
    int lenHeaderLines;
    
	/** Length of one data set in the data file */
	int lenDataSet;
	
	/** Value used if no data is available */
	int noData;
	
	/** Sampling time of the daq module in milliseconds */
	int tSample; 
    
    /** Timestamp of the last sample writen by the data server */
    struct timeval tLastData;
    
	/** Maximal allowed delay time before an alarm will be generated (sec) */
	int tAlarm; 
	
	/** Number of axis definitions */
	int nAxis;
	
	/** List of defined axis */
	struct axisDef *axis;

	/** Number of sensors in the module */
	int nSensors;
	
	/** Number of sensor columns in the data table. 
	  * It is possible to have more sensors defined in the 
	  * end of the sensor list that are just alias names for existing
	  * entries */
	int nSensorCols;
	
	/** number of values in profile data */
	int profile_length;
	
	/** List of the official sensor properties */
	struct sensorType *sensor;

	struct timeval tData;
	
	float *sensorValue;
	
	/** Number of processed bytes in the last cycle */
	unsigned int processedData;
	
	/** File pointer for the data file */
	FILE *fdata;

	/** Index of the data file 
	  * This variable will be stored persitently (.kitcube-data.marker) */
	unsigned long fileIndex;
	
	/* Number of the line in the data file 
	 * This variable will be stored persitently (.kitcube-data.marker) */
	int nLine;	
	
	/** Application ID. Can be used to identify the calling application. */
	int appId;
    
    /** Flag to identify flat folder mode */
    bool isFlatFolder;
	
	/** Name of the project. This variable is used to generate the database */
	std::string project;
	
	/** Name of the marker file. Used to make the file index persistent. */
	std::string filenameMarker;

	std::string inifile;
	
	std::string iniGroup;
	
	/** Path for writing data */
	std::string path;
	
	/** Filename for writing data */
	std::string filename;
	
	/** Filename for writing data */
	std::string fullFilename;	
	
	std::string moduleName;
	
	std::string moduleComment;
	
	/** Module type */
	std::string moduleType;

	unsigned int moduleNumber;
    
    /** The relative folder after the module number. Default is module name.
      * The default can be changed by the function  getDataDir for every class */
    std::string dataSubDir;

	/** Name of the sensor group. The sensor group name is used in most cases as file extention for the
	  * data file. */
	std::string sensorGroup;
	
	unsigned int sensorGroupNumber;
	
	/** Line number of the sensor definition in the data file header (default 1 = first line) */
	int headerLine;
	
	/** List of data field separators (default "\t\n") */
	std::string dataSep;
	
	/** Comment character (default "#") */
	std::string commentChar;
	
    /** Format of the timestamp (in ASCII data files) */
    std::string timestampFormat;
    
    
	/** Folder where all the configurations are stored */
	std::string configDir; 
	
	/** Directory where the data can be found - if simulation and readout run on the same machine
	  * this folder need to be the same as the dataDir */
	std::string remoteDir;
	
	/** Folder to store the local copy of the raw data files on the server */
	std::string archiveDir;
	
	/** Directory for writing the simulated data */
	std::string dataDir;
	
	/** Mask used to identify the data files in getNewFiles() */
	std::string datafileMask;
    
    /** Mask to parse the date in the index filed */
    std::string datafileIndexFormat;
	
	/** The template file is used to generate an appropriate header and
	 * the data is used to have some realistic simulation when writing data */
	std::string datafileTemplate;
	
	/** Argument for the rsync call to copy data from the modules to the archive */
	std::string rsyncArgs;
	
	/** Flag to idenitfy the first run */
	int initDone;
    
    /** Type of compression used for the raw data file */  
    std::string compressionType;
    
    
    
#ifdef USE_MYSQL	
	MYSQL *db;
#endif
	
	std::string dbHost;
    
    int dbPort;
	
	std::string dbName;
	
	std::string dbUser;
	
	std::string dbPassword;
	
	std::string sensorTableName;
	
	std::string axisTableName;
	
	std::string moduleTableName;
	
	std::string dataTableName;
	
	/** Every type of data should have a unique data table prefix.
	  * E.g. Scalar values use the prefix Data, while higher dimensional 
	  * structures are stored as BLOBs with the prefix Profile. */
	std::string dataTablePrefix;
	
	std::string statusTableName;
	
	/** Unique key used for seletion of the data set in the status table */
	std::string statusTableKey;
	
	
#ifdef HAVE_PYTHON_H	
	PyObject *pModule;
	
	PyObject *pFunc;
#endif

	std::string pythonDir;
	
	std::string pythonModule;
	
	std::string pythonFunction;
	
	/** Load analysis script for the DAQ module */
	void loadPython();
	
	/** Release the analysis script */
	void releasePython();
	
	/** Analyse data from the given data file.
	  * The function is an alternative to the function parseData() 
	  * Q: How many values?
	  */
	void readDataWithPython(const char *filename, double *sensorValue);

	
	/** Debug level (0 = no debug)*/
	int debug;
	
	std::string buffer;
	
	bool fd_eof;
	
	virtual int create_data_table_name(std::string & data_table_name);
	
	int read_ascii_line(char **buffer, size_t *length, FILE *file_ptr);
	
private:
	/** Template for the record filename */
	std::string filenameTmpl;
	
	/** Basedir of the auger file */
	std::string basedir;
	
	/** Template for the basedir of the auger file */
	std::string basedirTmpl;
	
	/** Location Id */
	//int locationId;
	
	/** Name of the location */
	std::string location;
	
	/** Flag to switch between unique and relative telescope id's */
	//int uniqueId;
	
	std::map<long, std::string> dateien;
	
	int get_file_list(std::string directory);
};

#endif
