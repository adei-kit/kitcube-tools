/***************************************************************************
                          createdevice.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/



#include "daqdevice.h"

#include "mast.h"
#include "ceilometer.h"
#include "lara.h"
#include "simrandom.h"



extern "C" void *createDevice(const char *devType) {	
	
	//
	// List of defined DAQ module types
	//	
	if (strcasestr(devType, "Mast") > 0) return( new (Mast));
	if (strcasestr(devType, "Ceilometer") > 0) return( new (Ceilometer));
	if (strcasestr(devType, "SimRandom") > 0) return( new (SimRandom));

	if (strcasestr(devType, "Lara") > 0) return( new (Lara));
	
	// Device type is unknown
	return(0);
}