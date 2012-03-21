//
// C++ Interface: parsivel
//
// Description: 
//
//
// Author: Norbert Flatinger <norbert.flatinger@kit.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PARSIVEL_H
#define PARSIVEL_H

#include "daqasciidevice.h"


/** Implementation for the parsivel device. 
  * 
  * The Parsivel measures the distribution of the rain drop size 
  * and speed. To save space in the database there is only the
  * aggregated data stored and the profiles are skipped.
  * 
  * TODO: 
  * - Compress profile data. (Scalars should be uncompressed in order
  *   not to spoil the database performance)
  */

class parsivel: public DAQAsciiDevice {
public:
	/**  */
	parsivel();
	
	/**  */
	~parsivel();

	/** Define a sensor group number for all the availble sensor group files */
	unsigned int getSensorGroup();	
	
	//void setConfigDefaults();
	
    /** Defintion of the sensors */ 
	int readHeader(const char *filename);
	
    /** The information of one dataset is split over five lines.
      * The data is collected line by line. */
	int parseData(char *line, struct timeval *tData, double *sensorValue);
	
    
	//void writeHeader();

	void writeData();
	
private:

};

#endif
