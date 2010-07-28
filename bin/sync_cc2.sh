#!/bin/sh
################################################################################
#
# File: sync_cc2.sh
#
# Description: synchronise and resize cloud camera pictures
# Author: Norbert Flatinger, IPE
# Created at: Wed Jul 28 13:36:37 CEST 2010
#
# Copyright (c) 2010 Norbert Flatinger, IPE  All rights reserved.
#
################################################################################
#
Version="0.1"
#
# 2010-07-28  Norbert Flatinger, IPE
#
# - this script uses "rsync" to synchronise pictures from the KIT-Cube cloud
#   camera 2 to the local machine (SRC to DEST)
# - then it uses "convert" to resize them to 25% in the subdirectory "small"
# - a marker file within the destination directory is used to hold the name of
#   the most recently converted image file
# - a semaphore file within the destination directory is used to prevent the
#   script to be executed more than one time (by cron)
#   in case of error, execution aborts and does not delete the semaphore; the
#   next call of the script detects the file and quits immediately; in case of
#   successful end of this script the semaphore gets deleted
#
################################################################################
#

# DEBUG=yes|no
DEBUG=yes


if [ $DEBUG = yes ] ; then
	echo "This is $0 version $Version"
fi


################################################################################
# define used commands
################################################################################
# define mkdir command
MKDIRPROG="/bin/mkdir"
# define rsync command
RSYNCPROG="/usr/bin/rsync"
#RSYNCPROG="$HOME/bin/rsync.sh"
# define find command
FINDPROG="/usr/bin/find"
# define sort command
SORTPROG="/usr/bin/sort"
# define convert command
CONVERTPROG="/usr/bin/convert"
# define rm command
RMPROG="/bin/rm"


################################################################################
# Synopsis
################################################################################
# TODO: synopsis

# TODO: check of arguments

# TODO: give SRC, DEST and FILES_TO_SYNC as arguments to script

# source directory
SRC=imk-wk1:/home/cube/CC2-pics/

# destination directory
DEST=/home/cube/archive/045/CC2-pics

# files to sync
FILES_TO_SYNC="kitcube_cc2_*.jpg"


################################################################################
# Now we check for an already running instance or previous session
# which ended with errors - in that case the semaphore file exists.
################################################################################
# define semaphore
#SEMFILE="$0".semaphore
SEMFILE=sync.semaphore

# check if semaphore file exists
if [ -e $DEST/$SEMFILE ] ; then
	echo -e "Semaphore file exists. Either due to errors of previous instance"
	echo -e "or because there is another session running in parallel."
	echo -e "Aborting execution...\n "
	exit 1
else
	TIMESTAMP=`date -u +%Y-%m-%d-%H-%M-%S`
	echo "$0: last start at $TIMESTAMP" > "$DEST/$SEMFILE"
fi


################################################################################
# check if destination directory exists and create it if not
################################################################################
# (do this only now here and not right after check of arguments, so semaphore
# file is written before)
if [ ! -d $DEST ] ; then
	$MKDIRPROG -p $DEST
	RETURN_VALUE=$?
	if [ $RETURN_VALUE != 0 ] ; then
		echo -e "\nError: could not create destination directory $DEST."
		echo -e "Aborting execution ...\n"
		exit 1
	fi
fi


################################################################################
# do synchronization
################################################################################
if [ -x $RSYNCPROG ] ; then
	# sync cloud camera 2 from imk-wk1
	$RSYNCPROG -avz --append \
		--include='*/' \
		--include=$FILES_TO_SYNC \
		--exclude='*' \
		$SRC $DEST
else
	echo -e "\nError: cannot find or execute $RSYNCPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# resize images to 25%
################################################################################
# check if destination directory for small images exists and create it if not
if [ ! -d $DEST/small ] ; then
	mkdir $DEST/small
	RETURN_VALUE=$?
	if [ $RETURN_VALUE != 0 ] ; then
		echo -e "\nError: could not create destination directory for small images $DEST/small."
		echo -e "Aborting execution ...\n"
		exit 1
	fi
fi

# define file holding name of most recently resized image
MARKER_FILE=$DEST/convert.marker

# read name of most recently resized image file
if [ -e $MARKER_FILE ] ; then
	source $MARKER_FILE
	if [ $DEBUG = yes ] ; then
		echo -e "Last converted file: $LAST_CONVERTED_FILE"
	fi
else
	echo -e "\nWarning: no marker file found, assuming first call of script."
	echo -e "Setting LAST_CONVERTED_FILE to $DEST/\n"
	LAST_CONVERTED_FILE="$DEST/"
fi

# resize new images
if [ -x $FINDPROG -a -x $SORTPROG -a -x $CONVERTPROG ] ; then
	if [ $DEBUG = yes ] ; then
		echo -e "Searching file list for new and unconverted files ..."
	fi
	for i in `$FINDPROG $DEST -maxdepth 1 -name "kitcube_cc2_*.jpg" | $SORTPROG` ; do
		if [ $i \> $LAST_CONVERTED_FILE ] ; then
			if [ $DEBUG = yes ] ; then
				echo -e "Converting $i ..."
			fi
			$CONVERTPROG $i -resize 25% $(dirname $i)/small/$(basename $i)
			RETURN_VALUE=$?
			if [ $RETURN_VALUE != 0 ] ; then
				echo -e "Error: converting $i"
			fi
			
			# write name of last converted image to marker file
			echo -e "LAST_CONVERTED_FILE=$i" > $MARKER_FILE
		fi
	done
else
	echo -e "\nError: cannot find or execute $FINDPROG or $SORTPROG or $CONVERTPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# clean up
################################################################################
# delete semaphore file
if [ -x $RMPROG ] ; then
	$RMPROG $DEST/$SEMFILE
else
	echo -e "\nError: cannot find or execute $RMPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi

# finish script
if [ $DEBUG = yes ] ; then
	echo "$0: successfully finished."
fi
