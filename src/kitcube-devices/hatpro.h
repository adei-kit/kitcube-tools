/********************************************************************
* Description:
* Author: Norbert Flatinger
* Created at: Mon Jul  4 16:00:22 CEST 2011
*    
* Copyright (c) 2011 Norbert Flatinger  All rights reserved.
*
********************************************************************/


#ifndef HATPRO_H
#define HATPRO_H

#include "daqbinarydevice.h"

#include <netcdf.h>
#include <netcdfcpp.h>


class hatpro: public DAQBinaryDevice {
public:
	/**  */
	hatpro();
	/**  */
	~hatpro();
	
    void openDatabase();
	int create_data_table();
    unsigned int getSensorGroup();
    int readHeader(const char *filename);
    void readData(std::string filename);

protected:
    int nVars;
    int nSensorFlat;
};

#endif
