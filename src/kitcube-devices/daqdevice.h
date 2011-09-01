/***************************************************************************
                          daqdevice.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef DAQDEVICE_H
#define DAQDEVICE_H

#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <cstring>
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
	float height;			// Postion of the sensor (height in meters)
	std::string data_format;	// <scalar>, <profile>, <2D>, <3D>
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

	/** Read parameter from inifile */
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

	virtual void openDatabase();
	
	/***********************************************************************
	 * create data table, if it doesn't exist
	 **********************************************************************/
	virtual int create_data_table();
	
	virtual void closeDatabase();
	
	/** Get time until next sample and it's id */
	virtual int readHeader(const char *header);

	/** Get time until next sample and it's id */
	virtual void readHeader();

	virtual void writeHeader();

	virtual int parseData(char* line, struct timeval* l_tData, double *sensorValue);
	
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
	  * file name specification of the device. In case of violation an exception
	  * std::invalid_argument has to be thrown.
	  *
	  */
	virtual long getFileNumber(char* filename);
	
	/** Get list of new files */
	void getNewFiles();
	
	/** Return the module number */
	unsigned int getModuleNumber();

	/** Return the number of the sensor group */
	virtual unsigned int getSensorGroup();
	
	/** Get a unix time stamp (in UTC) frm a date and time string of the form 
	  *  dd.mm.yyy  and hh:mm:ss  */
	unsigned long getTimestamp(const char* date, const char* time);
	
	/** Set debug level */
	void setDebugLevel(int level);
	
	/** Reached EOF during reading data in readData() */
	bool reachedEOF();
	
	/** Get the number of processed data in the last cycle */
	unsigned int getProcessedData();
	
	std::string sensorListfile;
	
protected:
	/** Length of the header block in the data file */
	int lenHeader;
	
	/** Length of one data set in the data file */
	int lenDataSet;
	
	/** Value used if no data is available */
	int noData;
	
	/** Sampling time of the daq module in milliseconds */
	int tSample; 
	
	/** Number of axis definitions */
	int nAxis;
	
	/** List of defined axis */
	struct axisDef *axis;

	/** Number of sensors in the module */
	int nSensors;
	
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

	/** Name of the sensor group. The sensor group name is used in most cases as file extention for the
	  * data file. */
	std::string sensorGroup;
	
	unsigned int sensorGroupNumber;
	
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
	
	/** The template file is used to generate an appropriate header and
	 * the data is used to have some realistic simulation when writing data */
	std::string datafileTemplate;
	
	/** Argument for the rsync call to copy data from the modules to the archive */
	std::string rsyncArgs;
	
	/** Flag to idenitfy the first run */
	int initDone;
	
#ifdef USE_MYSQL	
	MYSQL *db;
#endif
	
	std::string dbHost;
	
	std::string dbName;
	
	std::string dbUser;
	
	std::string dbPassword;
	
	std::string sensorTableName;
	
	std::string axisTableName;
	
	std::string moduleTableName;
	
	std::string dataTableName;
	
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
	int locationId;
	
	/** Name of the location */
	std::string location;
	
	/** Flag to switch between unique and relative telescope id's */
	int uniqueId;
	
	std::map<long, std::string> dateien;
	
	int get_file_list(std::string directory);
};

#endif
