#!/bin/bash
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
#Version="0.2"
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
# 2012-02-01 ak
# 
# - Added interface to KITcube alarm notification
# - Added support for flat directory structures
#   
Version="0.4"
# 2012-07-09 ak
#
# - Added version to import data from a remote disk. This might be necessary if the 
#   data is too large for netweor transfer or network is even not available. 
# - The script is acomplished by a script that collects the calls of all 
#   remotely managed devices 
#



# DEBUG=yes|no
DEBUG=yes

# Default configurations
SYNCUSEALARM="yes"


source ~/.bashrc
#echo "Env: KITCUBEDIR=$KITCUBEDIR"
if [ "x$KITCUBEDIR" = "x" ] ; then
    KITCUBEDIR=/home/cube
fi


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
# notify alarm system
ALARMPROG="$KITCUBEDIR/bin/report_alarm.sh"
# split flat folders by date
SPLITPROG="$KITCUBEDIR/bin/split_flatdir.sh"


# Load host specific configuration 
CONF="/home/cube/kitcube_conf.sh"
if [ -f $CONF ] ; then
        source $CONF
fi



if [ -x $BASENAMEPROG ] ; then
	if [ $DEBUG = yes ] ; then
		echo -e "This is $($BASENAMEPROG "$0") version $Version\n"
	fi
else
	echo -e "\nError: cannot find or execute $BASENAMEPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi



# Extract mode from script file name sync_<mode>.sh
#MODE=$($BASENAMEPROG -s .sh "$0")
MODE=${0##*/}        
MODE=${MODE#*_}
MODE=${MODE%.*}





################################################################################
# Usage
################################################################################
# Get mode of operation 
# Available are 
#     standard (sync_files) Get files from remote host
#     flat (sync_flat)      Get files from remote host and split flat dirs
# TODO: push (sync_push)      Push file to a remote server
#

NARGS=3
OPTS=""
if [ $MODE = "flat" ] ; then
    NARGS=4
    OPTS="<filemask>"
fi

if [ $# -lt $NARGS -o $# -gt $NARGS ] ; then
	echo -e "Usage:	$($BASENAMEPROG "$0") <SRC> <DEST> <rsync-filter-file> $OPTS"
	echo -e " "
	echo -e "	<SRC>			- source directory with trailing '/'"
	echo -e "	<DEST>			- destination directory"
	echo -e "	<rsync-filter-file>	- filter merge file for appending data"
	echo -e " "

if [ $MODE = "append" ] ; then
    echo -e "   The script only appends data to existing files."
    echo -e " "
fi

if [ $MODE = "offline" ] ; then
    echo -e "   The script imports data tranfered by USB disks / USB stick or other removable media."
    echo -e " "
fi

if [ $MODE = "flat" ] ; then
    echo -e "     $OPTS              - pattern for detecting date from filename by strptime."
    echo -e "                               The pattern is transfered to split_flatdir.sh"

    $SPLITPROG
fi

    #echo -e "Basedir: $KITCUBEDIR"

	exit 1
fi


# source directory
SRC="$1"
# destination directory
DEST="$2"

# Detector push operation 
# Check if the DEST name contains a host name separated by a colon
HOST=${DEST%:*}
if [ $HOST != $DEST ] ; then
	PUSH="yes"
	echo -e "Push data to host $HOST"
else
	PUSH="no"
fi


# define semaphore and log file
if [ $PUSH == "yes" ] ; then 
	SEMFILE="$SRC/push.semaphore"
	SYNCLOG="$SRC/rsync-push.txt"
else
	SEMFILE="$DEST/sync.semaphore"
	SYNCLOG="$DEST/rsync-res.txt"
fi



if [ $MODE = "append" ] ; then

    echo -e "Only append data"

fi

if [ $MODE = "import" ] ; then
  
    echo -e "Import data from removable storage"

fi


if [ $MODE = "flat" ] ; then

    # Additonal option for flat folders
    FLATDIR="$DEST/flat"
    DEST="$DEST/incoming"

    echo -e "Using flat folder mode: The second directory $FLATDIR will also be compared by rsync"
    echo -e "Make sure the flat-folder-mode is also enabled in kitcube-reader"
    echo -e " "
    echo -e "$FLATOPTS"
    echo -e "$DEST"
    echo -e ""
fi





################################################################################
# check if destination directory exists and create it if not
################################################################################
if [ $PUSH != "yes" ] ; then
if [ -x $MKDIRPROG ] ; then
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
#if [ $# -eq 4 ] ; then
#	SYNC_FILTER_FILE="$4"
#	if [ ! -f "$SYNC_FILTER_FILE" ] ; then
#		echo -e "\nError: filter merge file '$SYNC_FILTER_FILE' does not exist."
#		echo -e "Aborting execution...\n"
#		exit 1
#	fi
#fi

# TODO: check of other arguments



################################################################################
# check for an already running instance or previous session which ended with
# errors - in that case the semaphore file exists.
################################################################################
if [ -x $DATEPROG ] ; then
	# check if semaphore file exists
	if [ -e "$SEMFILE" ] ; then
                # Get PID
                RES=`cat $SEMFILE | grep PID | tr -d '"' `
                PID=${RES#*:}
                PID=${PID%*,}
                PIDQUERY=`ps -p $PID | wc -l`
                if [ $PIDQUERY == 2 ] ; then
                        echo -e "\nSemaphore file $SEMFILE exists. Either due to errors of previous instance"
                        echo -e "or because there is another session running in parallel."
                        echo -e "Aborting execution...\n "
                        exit 2
                fi
	else
		TIMESTAMP=`$DATEPROG -u +%Y-%m-%d-%H-%M-%S`
		echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$SEMFILE"
		#echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$DEST/sync_files.log"
	fi
else
	echo -e "\nError: cannot find or execute $DATEPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi

# Create semaphore file
TIMESTAMP=`$DATEPROG -u +%Y-%m-%d-%H-%M-%S`
#echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$SEMFILE"
#echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$DEST/sync_files.log"

TS=`$DATEPROG +"%s"`
echo "{"                                >  $SEMFILE
echo "  \"Date\": \"`date`\","          >> $SEMFILE
echo "  \"Timestamp\": \"$TS\","        >> $SEMFILE
echo "  \"PID\": \"$$\","               >> $SEMFILE
echo "  \"Destination\": \"$DEST\""     >> $SEMFILE
echo "}"                                >> $SEMFILE




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
    if [ $MODE = "flat" ] ; then
        # Warming the compare-dest option seems to work only with absolute path
        $RSYNCPROG -avh --compare-dest="$FLATDIR" --append -f "merge $APPEND_FILTER_FILE"  "$SRC" "$DEST" > "$SYNCLOG"
    elif [ $MODE = "append" ] ; then 
        $RSYNCPROG -avh  --append -f "merge $APPEND_FILTER_FILE" "$SRC" "$DEST" > "$SYNCLOG"
    elif [ $MODE = "offline" ] ; then
        # Do NOT overwrite newer files !!!
        $RSYNCPROG -avh --update -f "merge $APPEND_FILTER_FILE" "$SRC" "$DEST" > "$SYNCLOG"
    else
        $RSYNCPROG -avh  -f "merge $APPEND_FILTER_FILE" "$SRC" "$DEST" > "$SYNCLOG"
    fi
	RETURN_VALUE=$?
	if [ $RETURN_VALUE != 0 ] ; then
		echo -e "\nError: rsync failed with error code $RETURN_VALUE"
		echo -e "Aborting execution ...\n"
#		exit 1
	fi
	
    # Report the rsync progress to the alarm system - if defined
    # TODO: Add the key already here
    if [ "$SYNCUSEALARM" == "yes" ] ; then
    	if [ -x $ALARMPROG ]; then
		if [ $PUSH == "yes" ] ; then
			$ALARMPROG "$SRC" "$APPEND_FILTER_FILE" "$SYNCLOG"
		else
        		$ALARMPROG "$DEST" "$APPEND_FILTER_FILE" "$SYNCLOG"
		fi
    	fi
    fi

    # split the files by date
    if [ $MODE = "flat" ] ; then
 	#echo "Split files in a hierachical directory structure using $SPLITPROG"
        if [ -x $SPLITPROG ] ; then
            #echo "Splitting..."
            echo ""
            $SPLITPROG "$2" "$4" > "$DEST/split_flatdir.log"
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
	$RMPROG "$SEMFILE"
else
	echo -e "\nError: cannot find or execute $RMPROG."
	echo -e "Aborting execution...\n"
	exit 1
fi

# finish script
if [ $DEBUG = yes ] ; then
	echo "$($BASENAMEPROG "$0"): successfully finished."
fi


