#!/bin/bash
# Saving system monitoring information
# A. Kopmann
#
# Acquire the status of the computer infrastructure (CI)
#
# Sensor list:
# 1 Temperature of the Gude power plug in the computer room
#

# Default configurations 
CIARCHIVEDIR=/home/cube/archive
CIMODNO=120
CIMODID=SysCI
CIPREFIX="sys-ci-"
CITESTMODE="no"
CITEMPHOST="gude-1"
CITEMPSENSOR="GUDEADS-EPC2X6-MIB::epc2x6Temperature.0"

# Applications
SNMPGET="/usr/bin/snmpget"


# Load host specific configuration 
CONF="/home/cube/kitcube_conf.sh"
if [ -f $CONF ] ; then
	source $CONF 
fi



ARCHIVEDIR=$CIARCHIVEDIR
MODNO=$CIMODNO
MODID=$CIMODID
PREFIX=$CIPREFIX

SENSOR1="snmpget -v2c  -c public -OvqU  $CITEMPHOST $CITEMPSENSOR" 
SENSOR2="stat -f --format %a /home/cube/archive"

BLOCKSIZE="stat -f --format %s /home/cube/archive"
BLOCKN="stat -f --format %b /home/cube/archive"

# Check if snmpget is available
if [ ! -x $SNMPGET ]; then 
	echo "Error: SNMP tools not found -- install the package net-snmp"
	echo
	exit 1
fi

# Create data folder
if [ "$1" == "" ]; then
	DATADIR="$ARCHIVEDIR/$MODNO/$MODID"
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
	echo -e "# Timestamp\tTServerRoom (C)\tDiskSpace (GB)\tDiskSpace (%)" >> $DATAFILE
	echo -e "# sec\tC\tGByte\t%" >> $DATAFILE
fi

# Read data + store with timestamp
TDATA=`date -u +%s`

# Server room temperature
if [ $CITESTMODE != "yes" ] ; then 
	# Gude temp sensor
	RAWDATA=`$SENSOR1`
	DATA1="$((RAWDATA/10)).$((RAWDATA%10))"
else
	# IPE 404 computer sensor
	RAWDATA=`ssh $CITEMPHOST "sensors | grep $CITEMPSENSOR" `
	DATA1=${RAWDATA#*+}; 
	DATA1=${DATA1%%°*}
fi

# Disk space
NBL=`$BLOCKN`
NSZ=`$BLOCKSIZE`
RAWDATA=`$SENSOR2`
DATA2="$((RAWDATA*NSZ/1024/1024/1024))"
#DATA3="$((RAWDATA*100/NBL))"
DATA3=`echo "scale=2; $RAWDATA * 100 / $NBL" | bc`

echo -e "$TDATA\t$DATA1\t$DATA2\t$DATA3" >> $DATAFILE
#echo "Time $TDATA, Temperature $DATA1 C, Disk space $DATA2 GB, $DATA3 %"



