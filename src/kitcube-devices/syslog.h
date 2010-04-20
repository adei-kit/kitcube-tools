/***************************************************************************
                          syslog.h  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef SYSLOG_H
#define SYSLOG_H

#include "daqdevice.h"


/** Implementation for the performance module.
  *
  * Alternatively also a file could be defined. If the main tasks
  * writes all performance data to a file. the performance module 
  * would operate like all the other modules.
  *
  */

class SysLog: public DAQDevice  {
public: 
  /**  */
  SysLog();
  /**  */
  ~SysLog();


  /** Set default configuration */	
  void setConfigDefaults();
	
  /** Read parameter from inifile */
  //void readInifile(const char *inifile, const char *group = 0);


  /** Define the number arguments.
    * This function will also alocate memory */
  void setNData(int n);	 
	
  /** Initialize the data set. A channel is defined by the name and the data type. 
    * Available data types are 0 int and 1 float */
  void setConfig(int ch, const char *name);
	
  /** Update data set */	
  void updateTimestamp(struct timeval *t);

  void updateData(int ch, float data);
		
private:		

	
};

#endif
