#!/bin/sh
################################################################################
#
# File: resize_cc2.sh
#
# Description: resize cloud camera pictures
# Author: Norbert Flatinger, IPE
# Created at: Fri Aug 19 10:36:37 CEST 2011
#
# Copyright (c) 2011 Norbert Flatinger, IPE  All rights reserved.
#
################################################################################
#
Version="0.1"
#
# 2011-08-19  Norbert Flatinger, IPE
#
# - this script searches for pictures from the KITcube cloud camera 2 in the
#   local directory SRC
# - then it uses "convert" to resize them to 25% in the subdirectory "small"
# - a marker file within the source directory is used to hold the name of
#   the most recently converted image file
# - a semaphore file within the source directory is used to prevent the script
#   to be executed more than one time (by cron)
#   in case of error, execution aborts and does not delete the semaphore; the
#   next call of the script detects the file and quits immediately; in case of
#   successful end of this script the semaphore gets deleted
#
################################################################################
#

# DEBUG=yes|no
DEBUG=yes


################################################################################
# define used commands
################################################################################
#define basename command
BASENAMEPROG="/usr/bin/basename"
# define mkdir command
MKDIRPROG="/bin/mkdir"
# define date command
DATEPROG="/bin/date"
# define find command
FINDPROG="/usr/bin/find"
# define sort command
SORTPROG="/usr/bin/sort"
# define convert command
CONVERTPROG="/usr/local/bin/convert"
#define dirname command
DIRNAMEPROG="/usr/bin/dirname"
# define rm command
RMPROG="/bin/rm"


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
if [ $# != 2 ] ; then
	echo -e "Usage:"
	echo -e "	$($BASENAMEPROG "$0") SRC FILE"
	echo -e " "
	echo -e "	SRC	- source"
	echo -e "	FILE	- filename, may contain wildcards"
	echo -e " "
	exit 1
fi


################################################################################
# check if source directory exists
################################################################################
# source directory
SRC="$1"
if [ ! -d "$SRC" ] ; then
	if [ $DEBUG = yes ] ; then
		echo -e "Source directory $SRC does not exist."
		echo -e "Aborting execution ...\n"
		exit 1
	fi
fi

# TODO: check of other arguments

# files to sync
FILES_TO_SYNC="$2"


################################################################################
# check for an already running instance or previous session which ended with
# errors - in that case the semaphore file exists.
################################################################################
if [ -x $DATEPROG ] ; then
	# define semaphore
	#SEMFILE="$0".semaphore
	SEMFILE="resize.semaphore"

	# check if semaphore file exists
	if [ -e "$SRC/$SEMFILE" ] ; then
		echo -e "\nSemaphore file exists. Either due to errors of previous instance"
		echo -e "or because there is another session running in parallel."
		echo -e "Aborting execution...\n "
		exit 1
	else
		TIMESTAMP=`$DATEPROG -u +%Y-%m-%d-%H-%M-%S`
		echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$SRC/$SEMFILE"
	fi
else
	echo -e "\nError: cannot find or execute $DATEPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# resize images to 25%
################################################################################
# check if destination directory for small images exists and create it if not
if [ -x $MKDIRPROG ] ; then
	if [ ! -d "$SRC/small" ] ; then
		if [ $DEBUG = yes ] ; then
			echo -e "Destination directory $SRC/small for small images does not exist."
			echo -e "Creating it ..."
		fi
		$MKDIRPROG -p "$SRC/small"
		RETURN_VALUE=$?
		if [ $RETURN_VALUE != 0 ] ; then
			echo -e "\nError: could not create destination directory for small images $SRC/small."
			echo -e "Aborting execution ...\n"
			exit 1
		fi
	fi
else
	echo -e "\nError: cannot find or execute $MKDIRPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi

# define file holding name of most recently resized image
MARKER_FILE="$SRC/convert.marker"

# read name of most recently resized image file
if [ -e "$MARKER_FILE" ] ; then
	source "$MARKER_FILE"
	if [ $DEBUG = yes ] ; then
		echo -e "Last converted file: $LAST_CONVERTED_FILE"
	fi
else
	echo -e "\nWarning: no marker file found, assuming first call of script."
	echo -e "Setting LAST_CONVERTED_FILE to $SRC/\n"
	LAST_CONVERTED_FILE="$SRC/"
fi

# resize new images
if [ -x $FINDPROG -a -x $SORTPROG -a -x $CONVERTPROG -a -x $DIRNAMEPROG ] ; then
	if [ $DEBUG = yes ] ; then
		echo -e "Searching file list for new and unconverted files ..."
	fi
	for i in `$FINDPROG "$SRC" -maxdepth 3 -name "$FILES_TO_SYNC" | $SORTPROG` ; do
		if [ "$i" \> "$LAST_CONVERTED_FILE" ] ; then
			if [ $DEBUG = yes ] ; then
				echo -e "Converting $i ..."
			fi
echo "Convert $i"
#$CONVERTPROG "$i" -resize 25% "$($DIRNAMEPROG "$i")/small/$($BASENAMEPROG "$i")"
			RETURN_VALUE=$?
			if [ $RETURN_VALUE != 0 ] ; then
				echo -e "Error: converting $i"
			fi
			
			# write name of last converted image to marker file
			echo -e "LAST_CONVERTED_FILE=$i" > "$MARKER_FILE"
		fi
	done
else
	echo -e "\nError: cannot find or execute $FINDPROG or $SORTPROG or $CONVERTPROG or $DIRNAMEPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# clean up
################################################################################
# delete semaphore file
if [ -x $RMPROG ] ; then
	$RMPROG "$SRC/$SEMFILE"
else
	echo -e "\nError: cannot find or execute $RMPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi

# finish script
if [ $DEBUG = yes ] ; then
	echo "$($BASENAMEPROG "$0"): successfully finished."
fi
