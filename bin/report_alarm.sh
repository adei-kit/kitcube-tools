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

if [ "$KITCUBEDIR" = "" ] ; then
KITCUBEDIR=/home/cube
fi


APPNAME="sync"

#PROXYPROG="/home/cube/kitcube-tools/src/kitcube-reader/kitcube-monitor -i /home/cube/kitcube.ini"
PROXYPROG="$KITCUBEDIR/bin/kitcube-monitor -i $KITCUBEDIR/kitcube.ini"


# Welcome 
echo "report-alarm.sh - Analysing rsync results"


# Parse the module name
# In KITcube the destination dir follows the convention
# /home/cube/archive/<moduleNo>/<moduleId>
# TODO: Add more robust string operation
DEST="$1"
#MOD=${DEST:19:3}
#MODID=${DEST:23}

# Not perfect, might be improved later
dir=${DEST#*/archive/} ; MODNO=${dir%%/*}
dir=${DEST#*/$MODNO/} ; MODID=${dir%%/*}

#echo "Module identification: MODNO=$MODNO, MODID=$MODID"
if [ "$MODNO" == "" -o "$MODID" == "" -o "${MODNO//[^0-9]/}" != "$MODNO" ] ; then  
    MODNO=191
    MODID=SIM

    echo "Error: Destination dir is NOT according KITcube conventions"
    echo "       Using simulation settings - module($MODNO,$MODID)"

    exit 1
fi 

# Parse append file 
FILTER="$2"
GROUP=${FILTER#*append-filter}
if [ "$GROUP" == "$FILTER" ] ; then 
    GROUP=""
fi 
GROUP=${GROUP#*-}
if [ "$GROUP" != ""  ] ; then
    GROUPKEY=".$GROUP"
fi


# Key for status table
shopt -s extglob
KEY="$APPNAME.${MODNO##+(0)}$GROUPKEY"

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
    if [ `echo $OSTYPE | grep linux` != "" ] ; then
        ATIME=$(stat --format "%Y" $NEWEST)
    else
        # OSX, but is not able to return the UTC time ???
        ATIME=$(stat -f %B $NEWEST)
    fi 
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
        $PROXYPROG create $KEY appId=0,appName=\'$APPNAME\',module=$MODNO,moduleName=\'$MODID\',sensorGroup=\'$GROUP\',alarmLimit=900,secData=$ATIME,status=\'$FILENAME\'
    fi


else
    echo "report-alarm.sh: No new data found" 
    echo
fi



