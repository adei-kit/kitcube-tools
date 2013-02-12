#!/bin/sh
# Export the lastest data to a remote disk
# A. Kopmann, 7/12
#

DEBUG="yes"

if [ "$KITCUBEDIR" = "" ] ; then
        KITCUBEDIR=/home/cube
fi

WORKDIR=`pwd`

# Default configuration
EXPORTARCHIVE="/home/cube/archive"
EXPORTAGE=7 # days
#EXPORTTIME=30 # days
LOG="log"


# Applications
BASENAMEPROG="/usr/bin/basename"
DATEPROG="/bin/date"
MKDIRPROG="/bin/mkdir"
RMPROG="/bin/rm"
RSYNCPROG="/usr/bin/rsync"


# Read command line arguments
CONF="/home/cube/kitcube_conf.sh"
DELOLD="false"

while getopts a:dhi:m:t:v opt
do	case "$opt" in
	a) 	ARCHIVE="$OPTARG";;
	d)	DELOLD="true";;
	i)	CONF="$OPTARG";;
	m) 	AGE="$OPTARG";;
	t)	TIME="$OPTARG";;
	v) 	VERB="yes";;
	[?])	echo >&2 "Usage: $0 [-d] [-i inifile] <DEST>"
		exit 1;;
	esac
done

# Load host specific configuration
if [ -f $CONF ] ; then
        source $CONF
fi

if [ "$ARCHIVE" != "" ]; then EXPORTARCHIVE=$ARCHIVE; fi
if [ "$AGE" != "" ]; then EXPORTAGE=$AGE; fi
if [ "$TIME" != "" ]; then EXPORTTIME=$TIME; fi

if [ "$EXPORTTIME" == "" ]; then EXPORTTIME=$EXPORTAGE; fi


# Parameter
SRC="$EXPORTARCHIVE"
DESTURL="${!OPTIND}"
#SEMFILE="$SRC/export.semaphore"
SEMFILE="$LOG/export.semaphore"

# Expand path
cd $LOG
LOG=`pwd`
cd $WORKDIR

# Check if the target remote of local
REMOTE=${DESTURL%:*}
DEST=${DESTURL#*:}
if [ "$REMOTE" = "$DESTURL" ]; then
    #echo "Export within the local file system."
    REMOTE=""
    REMOTECMD=""

    # Expand path
    cd $DEST
    DEST=`pwd`
    DESTURL=`pwd`
    cd $WORKDIR    
else
    REMOTECMD="ssh $REMOTE "
fi 
#echo "Remote machine $REMOTE"
#echo "Target directory $DEST "


# Welcome
echo ""
echo "Export latest data from the archive to a (remote) backup disk"

# Usage
NARGS=$((OPTIND+0)) # NARGS = (OPTIND-1) + 1 
if [ $# -lt $NARGS -o $# -gt $NARGS ] ; then
        echo -e "Usage: $($BASENAMEPROG "$0") <OPTS> <DEST>  $OPTS"
        echo -e " "
        echo -e "   <OPTS>"
        echo -e "       a <archive> - Path to the archive"
        echo -e "                     source directory: $SRC"
        echo -e "       d           - delete old files"
        echo -e "       i <inifile> - alternative configuration file"
        echo -e "       m <days>    - maximum age of the files to save"
        echo -e "       t <days>    - time interval to save"
        echo -e "       v           - display file lists to be deleted from <DEST>"
        echo -e "   <DEST>          - destination directory"
        echo -e "       "
        echo -e " "

	exit 1
fi



# Check semaphore
# TODO: Allow oly one local instance at the same time - sem in /home/cube !!!
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
            exit 1
        fi
    fi
else
    echo -e "\nError: cannot find or execute $DATEPROG."
    echo -e "Aborting execution...\n"
    exit 1
fi


# Create semaphore file
TIMESTAMP=`$DATEPROG -u +%Y-%m-%d-%H-%M-%S`
#echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$SEMFILE"
echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$LOG/export_files.log"

TS=`$DATEPROG +"%s"`
echo "{"                                > $SEMFILE
echo "  \"Date\": \"`date`\","          >> $SEMFILE
echo "  \"Timestamp\": \"$TS\","        >> $SEMFILE
echo "  \"PID\": \"$$\","               >> $SEMFILE
echo "  \"Destination\": \"$DEST\""     >> $SEMFILE
echo "}"                                >> $SEMFILE



# Check if removable disk is mounted
# TODO: Check if DEST is identical or part of the archive?!

# Create log dir
if [ ! -d "$LOG" ] ; then
    if [ $DEBUG = "yes" ] ; then
        echo -e "Destination directory $LOG does not exist."
        echo -e "Creating it ..."
    fi
    $MKDIRPROG -p "$LOG"
    RETURN_VALUE=$?
    if [ $RETURN_VALUE != 0 ] ; then
        echo -e "\nError: could not create destination directory $LOG."
        echo -e "Aborting execution ...\n"
        exit 1
    fi
fi


#
# Create destination dir
# TODO: --> ssh? / return value: <cm>; echo $?
# <cmd>; echo $? || echo "Command failed"
if [ "$REMOTE" == "" ]; then
if [ -x $MKDIRPROG ] ; then
    if [ ! -d "$DEST" ] ; then
                if [ $DEBUG = "yes" ] ; then
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

# Add complete path for destination dir
cd $DEST
DEST=`pwd`
cd $WORKDIR

else # Remote operation
    RETURN_VALUE=`$REMOTECMD $MKDIRPROG -p "$DEST"; echo $?`
    #echo "Error $RETURN_VALUE"

    if [ $RETURN_VALUE != 0 ] ; then
        echo -e "\nError: could not create destination directory $REMOTE:$DEST."
        echo -e "Aborting execution ...\n"
        exit 1
    fi
fi



# Remove old data from the remote disk
# TODO: It makes no sense to transfer the possible long list
# to the source node.
# This part might be left out and executed by a script remotely
# Because it's too slow?!
# Alterntive: Use sshfs ?!

MAXTIME=`expr $EXPORTAGE - $EXPORTTIME`
if [ $MAXTIME -lt 0 ]; then MAXTIME=0; fi
echo "Archive: $SRC --> $DESTURL"
echo  "Selected interval is $EXPORTAGE .. $MAXTIME days ago"
echo ""

if [ "$REMOTE" == "" ]; then
    NFILES=`find $DEST -type f | wc -l`
    NOLD=`find $DEST -type f -mtime +$EXPORTAGE | wc -l`
    NNEW=`find $DEST -type f -not -name "export*" -mtime -$MAXTIME   | wc -l`
else
    NFILES=`$REMOTECMD find $DEST -type f | wc -l`
    NOLD=`$REMOTECMD find $DEST -type f -mtime +$EXPORTAGE | wc -l`
    NNEW=`$REMOTECMD find $DEST -type f -not -name "export*" -mtime -$MAXTIME   | wc -l`
fi

echo "There are $NFILES files at the export disk"
echo "There are $NOLD files older than $EXPORTAGE days"
if [ $MAXTIME -gt 0 ]; then
	echo "There are $NNEW files newer then $MAXTIME days"
fi 


if [ "$REMOTE" == "" ]; then
if [ $NOLD -gt 0 ] ; then
	echo "Old files: $DESTURL/export-to-be-deleted.txt"
    cd $DEST
    find * -type f -mtime +$EXPORTAGE > export-to-be-deleted.txt
        if [ "$VERB" != "" ]; then
		find * -type f  -mtime +$EXPORTAGE -exec ls -l {} \;
	fi
	if [ "$DELOLD" = "true" ] ; then
		find * -type f -mtime +$EXPORTAGE -exec  rm {} \;
		echo "Old files deteted"
	fi
	cd $WORKDIR
fi

if [ $MAXTIME -gt 0 -a  $NNEW -gt 0 ] ; then
       	echo "New files: $DESTURL/export-to-be-deleted.txt"
	 
    cd $DEST
    find * -type f -not -name "export*" -mtime -$MAXTIME >> export-to-be-deleted.txt
	if [ "$VERB" != "" ]; then 
		find * -type f -not -name "export*" -mtime -$MAXTIME -exec ls -l {} \;
	fi
	if [ "$DELOLD" = "true" ] ; then
             find * -type f -not -name "export*" -mtime -$MAXTIME -exec  rm {} \;	
	     echo "New files deteted"
     	fi 
 	cd $WORKDIR
 fi
else # Remote operation / leave out
if [ $NOLD -gt 0 ] ; then
    echo "Old files: $DESTURL/export-to-be-deleted.txt"
    $REMOTECMD "cd $DEST; find * -type f -mtime +$EXPORTAGE > export-to-be-deleted.txt"
    if [ "$VERB" != "" ]; then
        $REMOTECMD "cd $DEST; find * -type f  -mtime +$EXPORTAGE -exec ls -l {} \;"
        # Alternative: cat export-to-be-deleted.txt ?!
    fi
    if [ "$DELOLD" = "true" ] ; then
        $REMOTECMD "cd $DEST; find * -type f -mtime +$EXPORTAGE -exec  rm {} \;"
        echo "Old files deteted"
    fi
    cd $WORKDIR
fi

if [ $MAXTIME -gt 0 -a  $NNEW -gt 0 ] ; then
    echo "New files: $DEST/export-to-be-deleted.txt"

    $REMOTECMD "cd $DEST; find * -type f -not -name "export*" -mtime -$MAXTIME >> export-to-be-deleted.txt"
    if [ "$VERB" != "" ]; then
        $REMOTECMD "cd $DEST; find * -type f -not -name "export*" -mtime -$MAXTIME -exec ls -l {} \;"
    fi
    if [ "$DELOLD" = "true" ] ; then
        $REMOTECMD "cd $DEST; find * -type f -not -name "export*" -mtime -$MAXTIME -exec  rm {} \;"
        echo "New files deteted"
    fi
    cd $WORKDIR
fi

# TODO: Copy export-to-be-deleted.txt back to the main machine ($SRC)

fi


# Synchronize data
# TODO: export-files need to be stored locally etc /home/cube/.export-backup
echo -e "\nSynchronize data"

cd $SRC
if [ $MAXTIME -gt 0 ]; then 
	find * -type f -not -mtime +$EXPORTAGE -and -not -mtime -$MAXTIME > $LOG/export-files.txt
else
    find * -type f -not -mtime +$EXPORTAGE  > $LOG/export-files.txt
fi

NSYNC=`cat $LOG/export-files.txt | wc -l`
echo "There are $NSYNC files to copy - starting rsync ..."
echo "(Check progress with with: tail -f $LOG/export-res.txt)"

rsync -av --files-from=$LOG/export-files.txt $SRC $DESTURL > $LOG/export-res.txt

#find * -type f -not -mtime +$EXPORTAGE | rsync -av --files-from=- $SRC $DEST

echo "Done."
echo ""
echo "Logfile: $LOG/export-res.txt"
echo "Detete:  $DESTURL/export-to-be-deleted.txt" 
cd $WORKDIR

# Clear 
# delete semaphore file
if [ -x $RMPROG ] ; then
        $RMPROG "$SEMFILE"
else
        echo -e "\nError: cannot find or execute $RMPROG."
        echo -e "Aborting execution...\n"
        exit 1
fi  


echo " "




