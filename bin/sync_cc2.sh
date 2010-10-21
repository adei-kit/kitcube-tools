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
Version="0.2"
#
# 2010-10-18  Norbert Flatinger, IPE
#
# - put some more commands into variables and check if they exist
# - added script "Usage" output
# - give SRC, DEST and FILES_TO_SYNC as arguments to the script
# - check for DEST directory before writing semaphore file
# - updated rsync command line
# - quote all variables which might contain white space
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
# define rsync command
RSYNCPROG="/usr/bin/rsync"
#RSYNCPROG="$HOME/bin/rsync.sh"
# define find command
FINDPROG="/usr/bin/find"
# define sort command
SORTPROG="/usr/bin/sort"
# define convert command
CONVERTPROG="/usr/bin/convert"
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
if [ $# != 3 ] ; then
	echo -e "Usage:"
	echo -e "	$($BASENAMEPROG "$0") SRC DEST FILE"
	echo -e " "
	echo -e "	SRC	- source"
	echo -e "	DEST	- destination"
	echo -e "	FILE	- filename, may contain wildcards"
	echo -e " "
	exit 1
fi


################################################################################
# check if destination directory exists and create it if not
################################################################################
if [ -x $MKDIRPROG ] ; then
	if [ ! -d "$2" ] ; then
		if [ $DEBUG = yes ] ; then
			echo -e "Destination directory $2 does not exist."
			echo -e "Creating it ..."
		fi
		$MKDIRPROG -p "$2"
		RETURN_VALUE=$?
		if [ $RETURN_VALUE != 0 ] ; then
			echo -e "\nError: could not create destination directory $2."
			echo -e "Aborting execution ...\n"
			exit 1
		fi
	fi
else
	echo -e "\nError: cannot find or execute $MKDIRPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi

# TODO: check of other arguments?

# source directory
#SRC=imk-wk1:/home/cube/CC2-pics/
SRC="$1"

# destination directory
#DEST=/home/cube/archive/045/CC2-pics
DEST="$2"

# files to sync
#FILES_TO_SYNC="kitcube_cc2_*.jpg"
FILES_TO_SYNC="$3"


################################################################################
# check for an already running instance or previous session which ended with
# errors - in that case the semaphore file exists.
################################################################################
if [ -x $DATEPROG ] ; then
	# define semaphore
	#SEMFILE="$0".semaphore
	SEMFILE="sync.semaphore"

	# check if semaphore file exists
	if [ -e "$DEST/$SEMFILE" ] ; then
		echo -e "\nSemaphore file exists. Either due to errors of previous instance"
		echo -e "or because there is another session running in parallel."
		echo -e "Aborting execution...\n "
		exit 1
	else
		TIMESTAMP=`$DATEPROG -u +%Y-%m-%d-%H-%M-%S`
		echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$DEST/$SEMFILE"
	fi
else
	echo -e "\nError: cannot find or execute $DATEPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# do synchronization with "rsync"
# interesting options are:
# -v		increase verbosity
# -c		skip based on checksum, not mod-time & size
# -a		archive mode; equals -rlptgoD (no -H,-A,-X)
# -r		recurse into directories
# -u		skip files that are newer on the receiver
# -x		don't cross filesystem boundaries
# -z		compress file data during the transfer
# --append	append data onto shorter files
################################################################################
if [ -x $RSYNCPROG ] ; then
	# sync data
	$RSYNCPROG -avz --include='*/' --include="$FILES_TO_SYNC" --exclude='*' \
		"$SRC" "$DEST"
	RETURN_VALUE=$?
	if [ $RETURN_VALUE != 0 ] ; then
		echo -e "\nError: rsync failed with error code $RETURN_VALUE"
		echo -e "Aborting execution ...\n"
		exit 1
	fi
else
	echo -e "\nError: cannot find or execute $RSYNCPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# resize images to 25%
################################################################################
# check if destination directory for small images exists and create it if not
if [ ! -d "$DEST/small" ] ; then
	if [ $DEBUG = yes ] ; then
		echo -e "Destination directory $DEST/small for small images does not exist."
		echo -e "Creating it ..."
	fi
	$MKDIRPROG -p "$DEST/small"
	RETURN_VALUE=$?
	if [ $RETURN_VALUE != 0 ] ; then
		echo -e "\nError: could not create destination directory for small images $DEST/small."
		echo -e "Aborting execution ...\n"
		exit 1
	fi
fi

# define file holding name of most recently resized image
MARKER_FILE="$DEST/convert.marker"

# read name of most recently resized image file
if [ -e "$MARKER_FILE" ] ; then
	source "$MARKER_FILE"
	if [ $DEBUG = yes ] ; then
		echo -e "Last converted file: $LAST_CONVERTED_FILE"
	fi
else
	echo -e "\nWarning: no marker file found, assuming first call of script."
	echo -e "Setting LAST_CONVERTED_FILE to $DEST/\n"
	LAST_CONVERTED_FILE="$DEST/"
fi

# resize new images
if [ -x $FINDPROG -a -x $SORTPROG -a -x $CONVERTPROG -a -x $DIRNAMEPROG ] ; then
	if [ $DEBUG = yes ] ; then
		echo -e "Searching file list for new and unconverted files ..."
	fi
	for i in `$FINDPROG "$DEST" -maxdepth 1 -name "$FILES_TO_SYNC" | $SORTPROG` ; do
		if [ "$i" \> "$LAST_CONVERTED_FILE" ] ; then
			if [ $DEBUG = yes ] ; then
				echo -e "Converting $i ..."
			fi
			$CONVERTPROG "$i" -resize 25% "$($DIRNAMEPROG "$i")/small/$($BASENAMEPROG "$i")"
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
	$RMPROG "$DEST/$SEMFILE"
else
	echo -e "\nError: cannot find or execute $RMPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi

# finish script
if [ $DEBUG = yes ] ; then
	echo "$($BASENAMEPROG "$0"): successfully finished."
fi
