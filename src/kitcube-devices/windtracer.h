/********************************************************************
* Description:
* Author: Norbert Flatinger, IPE
* Created at: Mon Aug 16 16:17:30 CEST 2010
*    
* Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
*
********************************************************************/


#ifndef WINDTRACER_H
#define WINDTRACER_H


#include "daqbinarydevice.h"

#include <errno.h>


#define RAW_RECORD_ID					0x0400

#define PRODUCT_VELOCITY_RECORD_ID			0x0420
#define PRODUCT_FILTERED_VELOCITY_RECORD_ID		0x0421
#define PRODUCT_NOISE_COMP_RECORD_ID			0x0422

#define PRODUCT_VAD_RECORD_ID				0x0430

#define STATUS_RECORD_ID 				0x0440

#define REPORTER_RECORD_ID				0x0442

#define CONFIG_RECORD_ID				0x0460

#define CLUTTER_MAP_RECORD_ID				0x0462
#define TELNET_RECORD_ID				0x0463
#define SUSPEND_RECORD_ID				0x0464
#define RESUME_RECORD_ID				0x0465

#define PRODUCT_VELOCITY_DATA_BLOCK_ID			0x04A1
#define PRODUCT_SNR_DATA_BLOCK_ID			0x04A2
#define PRODUCT_SPECTRAL_WIDTH_DATA_BLOCK_ID		0x04A5
#define PRODUCT_MONITOR_SPECTRAL_DATA_BLOCK_ID		0x04A6

#define PRODUCT_BACKSCATTER_DATA_BLOCK_ID		0x04A8

#define PRODUCT_FILTERED_VELOCITY_DATA_BLOCK_ID		0x04B0
#define PRODUCT_FILTERED_SNR_DATA_BLOCK_ID		0x04B1
#define PRODUCT_FILTERED_BACKSCATTER_DATA_BLOCK_ID	0x04B2
#define PRODUCT_FILTERED_SPECTRAL_WIDTH_DATA_BLOCK_ID	0x04B3

#define CONFIG_DATA_BLOCK_ID				0x04F0


class windtracer: public DAQBinaryDevice  {
public:
	/**  */
	windtracer();
	/**  */
	~windtracer();
	
	void setConfigDefaults();
	
	/** Read parameter from inifile */
	//void readInifile(const char *inifile, const char *group = 0);
	
	/** Implements the data filename convention of the DAQ module. */
	const char *getDataFilename();
	
	void replaceItem(const char **header, const char *itemTag, const char *newValue);
	
	const char * getStringItem(const char **header, const char *itemTag);
	
	int getNumericItem(const char **header, const char *itemTag);
	
	unsigned int getSensorGroup();
	
	const char *getSensorName(const char *longname, unsigned long *aggregation = 0);
	
//	const char *getSensorType(const char *unit);
	
	/** Get time until next sample and it's id */
	void readHeader(const char *header);
	
	void writeHeader();
	
	void readData(const char *dir, const char *filename);
	
	void parseData(u_char *buffer, struct RecordHeader *record_header,
		       struct ScanInfo **scan_info, struct ProductPulseInfo **pulse_info,
		       float **velocity, float **snr, float **spectral_width,
		       float **backscatter, float **spectral_data);
	
private:
	unsigned char *headerRaw;
	
	std::string experimentName;
	
	unsigned int tSample;
	
	struct timeval tRef;
	
	int range_gates;
	
	int monitor_fft_size;
	
	int fft_size;
	
};
#endif
