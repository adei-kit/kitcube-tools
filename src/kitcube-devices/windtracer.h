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
#define PRODUCT_SPECTRAL_ESTIMATE_DATA_BLOCK_ID		0x04A7
#define PRODUCT_BACKSCATTER_DATA_BLOCK_ID		0x04A8

#define PRODUCT_FILTERED_VELOCITY_DATA_BLOCK_ID		0x04B0
#define PRODUCT_FILTERED_SNR_DATA_BLOCK_ID		0x04B1
#define PRODUCT_FILTERED_BACKSCATTER_DATA_BLOCK_ID	0x04B2
#define PRODUCT_FILTERED_SPECTRAL_WIDTH_DATA_BLOCK_ID	0x04B3

#define CONFIG_DATA_BLOCK_ID				0x04F0


/*   */
struct BlockDescriptor {
	u_int16_t nId;
	u_int16_t nVersion;
	u_int32_t nBlockLength;
};


/* RecordHeader contains record time stamp and the length of the entire record */
struct RecordHeader {
	struct BlockDescriptor block_desc;
	u_int32_t nRecordLength;
	u_int16_t nYear;
	u_int8_t nMonth;
	u_int8_t nDayOfMonth;
	u_int16_t nHour;
	u_int8_t nMinute;
	u_int8_t nSecond;
	u_int32_t nNanosecond;
};


/* Configuration text block is always the first record of any datafile and contains
   information about the configuration of the system */
struct ConfigurationRecord {
	struct BlockDescriptor block_desc;
	char *chConfiguration;
};


/* Scan Info Block provides information about the scanner, it appears before
   each Raw Data Record and before a Product Data Record */
struct ScanInfo {
	struct BlockDescriptor block_desc;
	float fScanAzimuth_deg;
	float fScanElevation_deg;
	float fAzimuthRate_dps;
	float fElevationRate_dps;
	float fTargetAzimuth_deg;
	float fTargetElevation_deg;
	int32_t nScanEnabled;
	int32_t nCurrentIndex;
	int32_t nAcqScanState;
	int32_t nDriverScanState;
	int32_t nAcqDwellState;
	int32_t nScanPatternType;
	u_int32_t nValidPos;
	int32_t nSSDoneState;
	u_int32_t nErrorFlags;
};


/* ProductPulseInfo appears before Product Data Blocks and contains information
   about the pulse used for that products */
struct ProductPulseInfo {
	struct BlockDescriptor block_desc;
	float fAzimuthMin_deg;
	float fAzimuthMean_deg;
	float fAzimuthMax_deg;
	float fElevationMin_deg;
	float fElevationMean_deg;
	float fElevationMax_deg;
	float fMonitorCount;
	float fMonitorFrequency_hz;
	float fMonitorTime;
	float fMonitorPeak;
	float fOverLevel;
	float fUnderLevel;
};


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
	int readHeader(const char *header);
	
	void writeHeader();
	
	void readData(std::string full_filename);
	
	void parseData(u_char *buffer, struct RecordHeader *record_header,
		       struct ScanInfo **scan_info, struct ProductPulseInfo **pulse_info,
		       float **sensor_values, u_int32_t *sensor_values_length);
	
	int create_data_table();
	
private:
	unsigned char *headerRaw;
	
	std::string experimentName;
	
	unsigned int tSample;
	
	struct timeval tRef;
	
	struct ConfigurationRecord config_record;
	
	u_int32_t config_text_block_length;
	
	int range_gates;
	
	double *range_gate_center, *range_gate_start, *range_gate_end;
	
	int num_sensors, num_aux_sensors;
	
};
#endif
