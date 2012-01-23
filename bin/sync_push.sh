#!/bin/sh
#----------------------------------------------------------------------
# Name: sync_push.sh
# Description: Push files from the DAQ system to a remote file storage
# Author: Andreas Kopmann, Norbert Flatinger
# Created at: Wed Nov 23 16:29:29 CEST 2011
#    
# Copyright (c) 2011 Norbert Flatinger  All rights reserved.
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
Version="0.3"
# 
# 2011-11-23 Andreas Kopmann, IPE
#
# - Removed parts of the code that are not working for file upload
#   E.g. generation of directories, ...
# - Move sync-file to the local directory (here SRC-side)
# - Remove echo -e option. Put in a few more echo lines instead.
#
# TODO:
# - Automatically find the remote part (SRC or DEST)
# - Add option --partial-dir=<tmp-dir>
#	But: This dir should be unique for every call !!!
#
# Examples:
#
# cronjob for KATRIN FPD data:
#  # [detdaqorcamac2:~] katrin% crontab -l
#  # Copy run files to central file server
#  * * * * * /Users/katrin/bin/sync_push.sh /Users/katrin/KIT/ ipepdvadei2.ka.fzk.de:/mnt/katrin/FPD_comm/530/ /Users/katrin/bin/detdaq-append-filter >> /Users/katrin/sync_files.log
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

if [ -x $DATEPROG ] ; then
	TIMESTAMP=`$DATEPROG -u`
fi

if [ -x $BASENAMEPROG ] ; then
	if [ $DEBUG = yes ] ; then
		echo "This is $($BASENAMEPROG "$0") version $Version (started $TIMESTAMP)"
		echo 
	fi
else
	echo 
	echo "Error: cannot find or execute $BASENAMEPROG."
	echo "Aborting execution...\n"
	exit 1
fi

################################################################################
# Usage
################################################################################
if [ $# -lt 3 -o $# -gt 4 ] ; then
	echo "Usage:	$($BASENAMEPROG "$0") SRC DEST <append-filter-file> [<sync-filter-file>]"
	echo " "
	echo "	SRC			- source directory with trailing '/'"
	echo "	DEST			- destination directory"
	echo "	<append-filter-file>	- filter merge file for appending data"
	echo "	<sync-filter-file>	- filter merge file for synchronizing data"
	echo " "
	exit 1
fi


################################################################################
# check if destination directory exists and create it if not
################################################################################
# destination directory
DEST="$2"

#if [ -x $MKDIRPROG ] ; then
#	if [ ! -d "$DEST" ] ; then
#		if [ $DEBUG = yes ] ; then
#			echo -e "Destination directory $DEST does not exist."
#			echo -e "Creating it ..."
#		fi
#		$MKDIRPROG -p "$DEST"
#		RETURN_VALUE=$?
#		if [ $RETURN_VALUE != 0 ] ; then
#			echo -e "\nError: could not create destination directory $DEST."
#			echo -e "Aborting execution ...\n"
#			exit 1
#		fi
#	fi
#else
#	echo -e "\nError: cannot find or execute $MKDIRPROG."
#	echo -e "Aborting execution...\n"
#	exit 1
#fi


################################################################################
# check if <append-filter-file> exists
################################################################################
APPEND_FILTER_FILE="$3"
if [ ! -f "$APPEND_FILTER_FILE" ] ; then
	echo "Error: filter merge file '$APPEND_FILTER_FILE' does not exist."
	echo "Aborting execution..."
	echo
	exit 1
fi


################################################################################
# check if <sync-filter-file> exists
################################################################################
if [ $# -eq 4 ] ; then
	SYNC_FILTER_FILE="$4"
	if [ ! -f "$SYNC_FILTER_FILE" ] ; then
		echo "\nError: filter merge file '$SYNC_FILTER_FILE' does not exist."
		echo "Aborting execution..."
		echo
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
	if [ -e "$SRC/$SEMFILE" ] ; then
		echo "Semaphore file exists. Either due to errors of previous instance"
		echo "or because there is another session running in parallel."
		echo "Aborting execution..."
		echo
		exit 1
	else
		TIMESTAMP=`$DATEPROG -u +%Y-%m-%d-%H-%M-%S`
		echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$SRC/$SEMFILE"
	fi
else
	echo "Error: cannot find or execute $DATEPROG."
	echo "Aborting execution..."
	echo
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
		echo
		echo "Error: rsync failed with error code $RETURN_VALUE"
		#echo "Aborting execution ..."
		#echo
		#exit 1
	fi
	
	if [ $# -eq 4 ] ; then
		$RSYNCPROG -avh -f "merge $SYNC_FILTER_FILE" "$SRC" "$DEST"
		RETURN_VALUE=$?
		if [ $RETURN_VALUE != 0 ] ; then
			echo
			echo "Error: rsync failed with error code $RETURN_VALUE"
			echo "Aborting execution ..."
			echo
			exit 1
		fi
	fi
else
	echo "Error: cannot find or execute $RSYNCPROG."
	echo "Aborting execution..."
	echo
	exit 1
fi


################################################################################
# clean up
################################################################################
# delete semaphore file
if [ -x $RMPROG ] ; then
	$RMPROG "$SRC/$SEMFILE"
else
	echo
	echo "Error: cannot find or execute $RMPROG."
	echo "Aborting execution..."
	echo
	exit 1
fi

# finish script
if [ $DEBUG = yes ] ; then
	echo "$($BASENAMEPROG "$0") has finished."
	echo
fi
