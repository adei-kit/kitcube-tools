/***************************************************************************
                          lara.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef LARA_H
#define LARA_H

#include "daqasciidevice.h"

#include <errno.h>


/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  * @todo Implement also storage of the test data in the database
  *       The feature should be configurable from the inifile
  */

class Lara: public DAQAsciiDevice  {
public: 
  /**  */
  Lara();
  /**  */
  ~Lara();

	void setConfigDefaults();
	
	const char *getDataDir();
	
	/** Implements the data filename convention of the DAQ module. */
	const char *getDataFilename();
	
	/** Re-implement the calculation of the file numbering scheme. 
	 * Lara uses date and times in the filename 
	 */
	long getFileNumber(char *filename);

	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();
	
	//void closeFile();
	
	/** Get time until next sample and it's id */
	int readHeader(const char *filename);
	
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);	
	
	void readData(std::string full_filename);
	
	void writeData();
	
	
private:	

	struct timeval tRef;
	
	/* Pointer to the last position in the template file.
	 * The pointer is not persistent. So every new call of the data server will
	 * start again at the first position of the template file. */
	int nTemplate;

	/** Number of samples in one file */
	int nSamplesInFile;

};

#endif
