/***************************************************************************
                          createdevice.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2007
    copyright            : (C) 2007 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


//#include "daqdevice.h"

#include "mast.h"
#include "ceilometer.h"
#include "lara.h"
#include "simrandom.h"
#include "norbert.h"
#include "jwd.h"
#include "parsivel.h"
#include "mrr.h"
#include "regenwippe.h"
#include "wolkenkamera.h"
#include "scintillometer.h"
#include "3m.h"
#include "windtracer.h"
#include "gps.h"
#include "sodar.h"
#include "radiosonde.h"
#include "windcube.h"
#include "hatpro.h"
#include "orcaprocess.h"


extern "C" void *createDevice(const char *devType) {
	//
	// List of defined DAQ module types
	//
	if (strcasestr(devType, "Mast") != NULL)
		return( new (Mast));
	if (strcasestr(devType, "Ceilometer") != NULL)
		return( new (Ceilometer));
	if (strcasestr(devType, "SimRandom") != NULL)
		return( new (SimRandom));
	if (strcasestr(devType, "Norbert") != NULL)
		return( new (Norbert));
	if (strcasestr(devType, "JWD") != NULL)
		return( new (jwd));
	if (strcasestr(devType, "Parsivel") != NULL)
		return( new (parsivel));
	if (strcasestr(devType, "MRR") != NULL)
		return( new (mrr));
	if (strcasestr(devType, "Regenwippe") != NULL)
		return( new (regenwippe));
	if (strcasestr(devType, "Wolkenkamera") != NULL)
		return( new (wolkenkamera));
	if (strcasestr(devType, "Scintillometer") != NULL)
		return( new (sci));
	if (strcasestr(devType, "3M") != NULL)
		return( new (dreim));
	if (strcasestr(devType, "windtracer") != NULL)
		return( new (windtracer));
	if (strcasestr(devType, "gps") != NULL)
		return( new (gps));
	if (strcasestr(devType, "sodar") != NULL)
		return( new (sodar));
	if (strcasestr(devType, "radiosonde") != NULL)
		return( new (radiosonde));
	if (strcasestr(devType, "windcube") != NULL)
		return( new (windcube));
	if (strcasestr(devType, "hatpro") != NULL)
		return( new (hatpro));
	
	if (strcasestr(devType, "OrcaProcess") != NULL)
		return( new (OrcaProcess));
	if (strcasestr(devType, "Lara") != NULL)
		return( new (Lara));
	
	// Device type is unknown
	return(0);
}
