//
// C++ Interface: campbelltoa5
//
// Description: 
//
//
// Author: Andreas Kopmann  <andreas.kopmann@kit.edu>, (C) 2014
//
// Copyright: See COPYING file that comes with this distribution
//
//


#ifndef CAMPBELLTOA5_H
#define CAMPBELLTOA5_H

#include "daqasciidevice.h"

/** Implementation of a reader for the Campbell DataLogger format A05.
  * The format has compare to previous Campbell ASCII formats more header information.
  */  
class CampbellTOA5: public DAQAsciiDevice {
public:
	/**  */
	CampbellTOA5();
	
	/**  */
	~CampbellTOA5();

	/** Read Campbell specific header information */
	int readHeader(const char *header);

};

#endif


