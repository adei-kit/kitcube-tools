/***************************************************************************
                          simrandom.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef SIMRANDOM_H
#define SIMRANDOM_H


#include <cstdio>
#include <string>
#include <map>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif // of USE_MYSQL

#include <akutil/simpleserver.h>
#include <akutil/procDuration.h>


#include "daqdevice.h"



/** Implementation for the weather mast DAQ devices that are
  * used for turbulence, energy balance and 20m mast.
  * 
  * @todo Implement also storage of the test data in the database
  *       The feature should be configurable from the inifile
  */

class SimRandom : public DAQDevice  {
public: 
  /**  */
  SimRandom();
  /**  */
  ~SimRandom();


  /** Read parameter from inifile */
  void readInifile(const char *inifile, const char *group = 0);

	void openFile();
	
	void copyRemoteData();

	const char *getDataDir();
	
  /** Get time until next sample and it's id */
  void readHeader();
  
  void writeHeader();
	
  void readData(const char *dir, const char *data);	

  void writeData();
	
private:	


};

#endif
