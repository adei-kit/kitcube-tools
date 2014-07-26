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

};

#endif


