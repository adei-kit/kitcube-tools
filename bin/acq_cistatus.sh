#!/bin/bash
# Saving system monitoring information
# A. Kopmann
#
# Acquire the status of the computer infrastructure (CI)
#
# Sensor list:
# 1 Temperature of the Gude power plug in the computer room
#

# Read configuration from kitcube.ini
ARCHIVEDIR=/home/cube/archive
MODNO=120
PREFIX="sys-ci-"
SNMPGET="/usr/bin/snmpget"

SENSOR1="snmpget -v2c  -c public -OvqU  gude-1 GUDEADS-EPC2X6-MIB::epc2x6Temperature.0"


# Check if snmpget is available
if [ ! -x $SNMPGET ]; then 
	echo "Error: SNMP tools not found -- install the package net-snmp"
	echo
	exit 1
fi

# Create data folder
if [ "$1" == "" ]; then
	DATADIR="$ARCHIVEDIR/$MODNO"
else
	DATADIR="$1"
fi
if [ ! -d $DATADIR ]; then
	mkdir -p $DATADIR
fi	

# Create new file + header if not existing
TFILE=`date -u "+%y%m%d"`
DATAFILE="$DATADIR/$PREFIX$TFILE.txt"

if [ ! -f $DATAFILE ]; then
	echo -e "# $DATAFILE" > $DATAFILE
	echo -e "# Monitoring the status of the computer infrastructure" >> $DATAFILE
	echo -e "#"  >> $DATAFILE
	echo -e "Timestamp\tTGude" >> $DATAFILE
	echo -e "sec\tC" >> $DATAFILE
fi

# Read data + store with timestamp
TDATA=`date -u +%s`
RAWDATA=`$SENSOR1`
DATA="$((RAWDATA/10)).$((RAWDATA%10))"

echo -e "$TDATA\t$DATA" >> $DATAFILE
#echo "Time $TDATA, Temperature $DATA C"



