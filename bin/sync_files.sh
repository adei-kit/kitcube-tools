#!/bin/sh
#----------------------------------------------------------------------
# Description:
# Author: Norbert Flatinger
# Created at: Wed Oct 13 16:29:29 CEST 2010
#    
# Copyright (c) 2010 Norbert Flatinger  All rights reserved.
#
#----------------------------------------------------------------------
#
Version="0.1"
#
# 2010-10-19  Norbert Flatinger, IPE
#
# - this script is intended to use rsync to synchronize files from a remote
#   machine directory (SRC) to a local machine directory (DEST)
# - SRC and DEST must be given as parameters
# - a semaphore file within DEST is used to prevent the script to be executed
#   more than one time (by cron)
#   in case of error, execution aborts and does not delete the semaphore; the
#   next call of the script detects the file and quits immediately; in case of
#   successful end of this script the semaphore gets deleted
#
################################################################################
#
Version="0.2"
#
# 2011-03-16  Norbert Flatinger, IPE
#
# - added using a filter merge file with rules for a flexible and detailed
#   selection of files to get synchronized by rsync; always use this to avoid
#   synchronizing unneeded/unwanted files
# - added the mandatory parameter <append-filter-file>
# - up to now rsync synchronizes files by appending new data to existing files
#   => add a second call to rsync using ordinary rsync synchronization
# - added the optinal paramter <sync-filter-file>
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
if [ $# -lt 3 -o $# -gt 4 ] ; then
	echo -e "Usage:	$($BASENAMEPROG "$0") SRC DEST <append-filter-file> [<sync-filter-file>]"
	echo -e " "
	echo -e "	SRC			- source directory with trailing '/'"
	echo -e "	DEST			- destination directory"
	echo -e "	<append-filter-file>	- filter merge file for appending data"
	echo -e "	<sync-filter-file>	- filter merge file for synchronizing data"
	echo -e " "
	exit 1
fi


################################################################################
# check if destination directory exists and create it if not
################################################################################
if [ -x $MKDIRPROG ] ; then
	# destination directory
	DEST="$2"
	if [ ! -d "$DEST" ] ; then
		if [ $DEBUG = yes ] ; then
			echo -e "Destination directory $DEST does not exist."
			echo -e "Creating it ..."
		fi
		$MKDIRPROG -p "$DEST"
		RETURN_VALUE=$?
		if [ $RETURN_VALUE != 0 ] ; then
			echo -e "\nError: could not create destination directory $DEST."
			echo -e "Aborting execution ...\n"
			exit 1
		fi
	fi
else
	echo -e "\nError: cannot find or execute $MKDIRPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# check if <append-filter-file> exists
################################################################################
APPEND_FILTER_FILE="$3"
if [ ! -f "$APPEND_FILTER_FILE" ] ; then
	echo -e "\nError: filter merge file '$APPEND_FILTER_FILE' does not exist."
	echo -e "Aborting execution...\n"
	exit 1
fi


################################################################################
# check if <sync-filter-file> exists
################################################################################
if [ $# -eq 4 ] ; then
	SYNC_FILTER_FILE="$4"
	if [ ! -f "$SYNC_FILTER_FILE" ] ; then
		echo -e "\nError: filter merge file '$SYNC_FILTER_FILE' does not exist."
		echo -e "Aborting execution...\n"
		exit 1
	fi
fi

# TODO: check of other arguments

# source directory
SRC="$1"


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
# --append	append data onto shorter files
# -x		don't cross filesystem boundaries
# -z		compress file data during the transfer
# -h		output numbers in a human-readable format
# -f RULE	add a file-filtering RULE
################################################################################
if [ -x $RSYNCPROG ] ; then
	# sync data
	$RSYNCPROG -avh --append -f "merge $APPEND_FILTER_FILE" "$SRC" "$DEST"
	RETURN_VALUE=$?
	if [ $RETURN_VALUE != 0 ] ; then
		echo -e "\nError: rsync failed with error code $RETURN_VALUE"
		echo -e "Aborting execution ...\n"
		exit 1
	fi
	
	if [ $# -eq 4 ] ; then
		$RSYNCPROG -avh -f "merge $SYNC_FILTER_FILE" "$SRC" "$DEST"
		RETURN_VALUE=$?
		if [ $RETURN_VALUE != 0 ] ; then
			echo -e "\nError: rsync failed with error code $RETURN_VALUE"
			echo -e "Aborting execution ...\n"
			exit 1
		fi
	fi
else
	echo -e "\nError: cannot find or execute $RSYNCPROG."
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
