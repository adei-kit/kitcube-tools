#!/bin/sh
#----------------------------------------------------------------------
# Description:
# Author: Andreas Kopmann
# Created at: Wed Jan 31 23:28:29 CEST 2011
#    
# Copyright (c) 2010 Andreas Kopmann  All rights reserved.
#
#----------------------------------------------------------------------
#

APPNAME="Sync"

PROXYPROG="../kitcube-tools/src/kitcube-reader/kitcube-monitor -i ../kitcube-tools/etc/kitcube.ini"


# Welcome 
echo "report-alarm.sh - Analysing rsync results"


# Parse the module name
# In KITcube the destination dir follows the convention
# /home/cube/archive/<moduleNo>/<moduleId>
# TODO: Add more robust string operation
DEST="$1"
MOD=${DEST:19:3}
MODID=${DEST:23}

# Not perfect, might be improved later
MODULES=${DEST#*/home/cube/archive/}
MOD=${MODULES%/*}
MODID=${MODULES#*/}

if [ ${#DEST} -eq ${#MODULES} ]; then
    MOD=191
    MODID=SIM

    echo "Error: Destination dir is NOT according KITcube conventions"
    echo "       Using simulation settings - module($MOD,$MODID)"

fi 


# Key for status table
KEY="$APPNAME.$MOD"




# Find the latest processed file
NEWEST="$DEST/compareTo"
touch -t 197001010000 $DEST
NEWEST=`cat $DEST/rsync-res.txt | ( while read f; do
if [ "$f" != "" -a "$f" != "./" ] ; then
FILE="$1/$f"
if [ -f "$FILE" -a "$FILE" -nt "$NEWEST" ] ; then	
NEWEST=$FILE
fi
#echo "$FILE, $NEWEST" 
fi	
done
echo "$NEWEST" ) `

#echo "Newset file $NEWEST"
if [ "$NEWEST" != "$DEST/compareTo" ] ; then
    # return the date
    # Linux
    #ATIME=$(stat --format "%X" $NEWEST)
    # OSX, but is not able to return the UTC time ???
    ATIME=$(stat -f %B $NEWEST)
    #echo $ATIME
  
    FILENAME=`basename $NEWEST`
    echo Latest file is $FILENAME

    # Save current time, latest data file timestamp, name of the file
    # use kitcube-monitor   
    #echo "$PROXYPROG set $KEY secData=$ATIME"
    $PROXYPROG set $KEY secData=$ATIME,status=\'$FILENAME\'
    RETURN_VALUE=$?

    if [ $RETURN_VALUE != 0 ] ; then
        echo "report-alarm.sh: Create new entry"
        $PROXYPROG create $KEY 
        $PROXYPROG set $KEY appId=0,appName=\'$APPNAME\',module=$MOD,moduleName=\'$MODID\',sensorGroup=\'\',alarmLimit=0,secData=$ATIME,status=\'$FILENAME\'
    fi


else
    echo "report-alarm.sh: No new data found" 
    echo
fi



