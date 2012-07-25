#!/bin/sh
#----------------------------------------------------------------------
# Description:
# Author: Andreas Kopmann
# Created at: Mon Apr 23 17:24:29 CEST 2012
#
# Copyright (c) 2012 Andreas Kopmann  All rights reserved.
#
#----------------------------------------------------------------------
#
Version="0.1"
# 
# Compress data files
# The files to be compressed are defined by
# 	- base folder
# 	- list of extensions
#	- time interval
#
# The mode of compression can be 
# 	- file by file
#	- compressed tar-ball for folders
#


# DEBUG=yes|no
DEBUG=yes

source ~/.bashrc
if [ "$KITCUBEDIR" = "" ] ; then
    KITCUBEDIR=/home/cube
fi


################################################################################
# define used commands
################################################################################
BASENAMEPROG="/usr/bin/basename"


if [ -x $BASENAMEPROG ] ; then
        if [ $DEBUG = yes ] ; then
                echo -e "This is $($BASENAMEPROG "$0") version $Version\n"
        fi
else
        echo -e "\nError: cannot find or execute $BASENAMEPROG."
        echo -e "Aborting execution...\n"
        exit 1
fi



################################################################################
# Usage
################################################################################
# Get mode of operation
# Available are
# 	- files 	Compress files 
# 	- folders 	Move the whole folder to a tar-ball
# 	- detete 	Delete obsolete data from the flat dirs


# Extract mode from script file name compress_<mode>.sh
MODE=${0##*/}
MODE=${MODE#*_}
MODE=${MODE%.*}

NARGS=1
OPTS=""

if [ $# -lt $NARGS -o $# -gt $NARGS ] ; then
        echo -e "Usage: $($BASENAMEPROG "$0") <DEV> $OPTS"
        echo -e " "
        echo -e "       <DEV>                   - source directory with trailing '/'"
        echo -e " "

        exit 1
fi

# Command line 
DEV=$1


# Control parameters
TMIN=0 		# compress everything older than n days = n x 24hs 
SUFARCH=".gz"	# suffix of the archive
SUFDATA=".jpg"	# suffix of the data files
EXCL=".*"	# Never include hidden files (.*)



if [ $DEV = "test" ] ; then 
DAQHOST=localhost
DAQDIR=/Users/kopmann/src/kitcube/kitcube-tools/bin/tmp/remote
ARCHDIR=/Users/kopmann/src/kitcube/kitcube-tools/bin/tmp
FILEMASK="*.jpg"
fi

if [ $DEV = "RPG" ] ; then 
DAQHOST=imkpchatpro
DAQDIR=/cygdrive/c/RPG-HATPRO/Data/
ARCHDIR=/home/cube/archive/080/RPG/
FILEMASK="vertical_*"
FILEFORMAT="%y%m%d"
COMPRESS=yes
fi

if [ $DEV = "CC" ] ; then 
DAQHOST=imk-wk1
DAQDIR=/home/cube/CC2-pics/
ARCHDIR=/home/cube/archive/045/CC2/
FILEMASK="*.jpg"
FILEFORMAT="kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg"
COMPRESS=no
fi

if [ $DEV = "CC-test" ] ; then
	DAQHOST=imk-wk1
	DAQDIR=/home/cube/CC2-pics/
	ARCHDIR=/home/cube/archive/045/test/
	FILEMASK="*.jpg"
	FILEFORMAT="kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg"
	COMPRESS=no
fi


if [ $DEV = "PARS" ] ; then 
DAQHOST=kc-wora
DAQDIR=/home/cube/Parsivel/Daten/
ARCHDIR=/home/cube/archive/030/PARS/
FILEMASK="*.dat"
COMPRESS=yes
fi


if [ $DEV = "CM" ] ; then 
DAQHOST=kc-wora
DAQDIR=/home/cube/Ceilometer/Daten/
ARCHDIR=/home/cube/archive/020/CM/
FILEMASK="*.dat"
FILEFORMAT="chm%Y%m%d_%H%M%S.dat"
COMPRESS=dir
fi

if [ "$ARCHDIR" = "" ] ; then
	echo "Error: Device definition for DEV=$DEV is missing"
	echo ""
	exit 1;
fi


################################################################################
# Get oldest file on DAQ server
################################################################################

echo "Remote system: $DAQHOST:$DAQDIR"

# TODO: Add list of file masks separated by white character

# Check remote file system
CMD="cd $DAQDIR; find * -maxdepth 1 -type f -name \"$FILEMASK\"  | sort | head -n 1"
#echo "$CMD"
OLDFILE=`ssh $DAQHOST "$CMD" `

# Strip path
OLDFILE=$($BASENAMEPROG "$OLDFILE")
echo "Oldest file:   $OLDFILE"
echo



################################################################################
# Compress files
################################################################################

cd $ARCHDIR
if [ "$COMPRESS" = "yes" ] ; then 
    # Find the oldest file in the archive
    REF=`find 20* -type f -name $OLDFILE`
    #echo "Reference file is $REF"

    # Get list of files that can be compressed
    #echo "This is the list of file that are older than the oldest remote files"
    LIST=`find 20* -type f -not -newer $REF -not -name $OLDFILE -not -name *.gz`
    #echo $LIST


    # Compress this files 
    for FILE in $LIST
    do
        gzip -f $FILE
    done


    if [ "$LIST" = "" ] ; then
       echo "No files found that can be compressed"
    else 
       NFILES=`echo $LIST | wc -w`
       echo "Compressed $NFILES data file(s) in $DIR"
    fi 
fi


if [ "$COMPRESS" = "dir" ] ; then 
    # Find the oldest file in the archive
    REF=`find 20* -type f -name $OLDFILE`
    #echo "Reference file is $REF"

    # Get list of files that can be compressed
    #echo "This is the list of file that are older than the oldest remote files"
    LIST=`find 20* -type d -not -newer $REF`
    echo $LIST


    # Compress this files 
    for SUBDIR in $LIST
    do
        echo "tar -czf <name of the archive> $SUBDIR"
    done


    if [ "$LIST" = "" ] ; then
        echo "No directories found that can be compressed"
    else 
        NFILES=`echo $LIST | wc -w`
        echo "Compressed $NFILES data folder(s) in $DIR"
    fi 
fi



################################################################################
# Clean flat file structure
################################################################################


# flat structure
# In case of a flat structure the copy in the flat directory is also 
# not required any more !!!
# Check if a flat folder is existing
# Move files from the flat dir to trash

#echo
#echo "Check obsolete files at localhost dir = $DAQDIR"

cd $ARCHDIR
if [ -d "flat" ] ; then
  #echo "This is the list of files in flat that are older than the oldest remote files"
  LIST=`find flat -type f -not -newer flat/$OLDFILE -not -name $OLDFILE`
  #echo $LIST

  # Create trash folder 
  if [ ! -d "trash" ] ; then
    mkdir "trash"
  fi 

  for FILE in $LIST
  do
    # Test if copy in hierachical tree is available
    # Get the target folder namei
    # TODO: Move to a separate function 
    FILENAME=$($BASENAMEPROG "$FILE")
    if [ "`echo $OSTYPE | grep linux`" != "" ] ; then 
	#echo "Using linux implementation of BSD data command"
        TARGETDIR=`/usr/bin/python $KITCUBEDIR/bin/date.py "$FILEFORMAT" $FILENAME "%Y/%m/%d" 2>/dev/null`
    else
	#echo "OS: Darwin = non linux"
	TARGETDIR=`date -j -u -f "$FILEFORMAT" $FILENAME "+%Y/%m/%d" 2>/dev/null` 
    fi 
    #echo "$FILE / $FILENAME / <TARGET>=$TARGETDIR"	

    TARGETDIR="data/$TARGETDIR"
    if [ -e "$TARGETDIR/$FILENAME" ] ; then 
        #echo "Removing $FILE (orig in $TARGETDIR/$FILENAME)"
        mv $FILE trash
    else
        echo "Data file $TARGETDIR/$FILENAME is missing!!!"
    fi
  done

   if [ "$LIST" = "" ] ; then
     echo "No obsolete files found in dir flat"
   else
     NFILES=`echo $LIST | wc -w`
     echo "Moved $NFILES data file(s) from flat to trash"
   fi 
fi 


echo
exit 0





