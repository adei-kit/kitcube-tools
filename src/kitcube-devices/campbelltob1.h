//
// C++ Interface: campbelltob1
//
// Description: 
//
//
// Author: Andreas Kopmann  <andreas.kopmann@kit.edu>, (C) 2014
//
// Copyright: See COPYING file that comes with this distribution
//
//


#ifndef CAMPBELLTOB1_H
#define CAMPBELLTOB1_H

#include "daqbinarydevice.h"

/** Implementation of a reader for the Campbell DataLogger format TOB3 / TOBx?.
  * The format has compare to previous Campbell ASCII formats more header information.
  */  
class CampbellTOB1: public DAQBinaryDevice {
public:
	/**  */
	CampbellTOB1();
	
	/**  */
	~CampbellTOB1();

	/** Read Campbell specific header information */
	int readHeader(const char *header);

	/** Get one dataset from the buffer */
	int parseData(char *line, struct timeval *l_tData, double *sensorValue);

	
private:
	/** File type TOB1 = 1; TOB2 = 2; TOB3 = 3 */
	int tobno;
	
	/** List of sensor types (same name as in Campbell docuumentation, TODO: move to sensor description array)*/
	std::string *sensorTypes;
	
	/** Length of the data in bytes (TODO: move to sensor description array) */
	int *sensorBytes;
	
	int nColsInFile;
	
	/** Number of the column with the second counter (0...n)*/
	int secCol;
	
	/** Number of the column with the subsecond counter (0...n)*/
	int subSecCol;
	
	/** Length of a sinle frame */
	int recordSize;
	
	/** recordTimeStep */
	struct timeval recordTimeStep;
	
	/** Number of records in one frame (only TOB2/3) */
	int nRecordsInFrame;
	
	/** Number of processed records (used by parseData)*/
	int nProcessedRecords;
	
	/** Timestamp of data frame */
	struct timeval frameTimestamp;
	
};

#endif


