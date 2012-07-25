#!/bin/sh
# Export the lastest data to a remote disk
# A. Kopmann, 7/12
#

if [ "$KITCUBEDIR" = "" ] ; then
        KITCUBEDIR=/home/cube
fi

# Default configuration
EXPORTARCHIVE="/home/cube/archive"
EXPORTAGE=7 # days

# Applications
BASENAMEPROG="/usr/bin/basename"
DATEPROG="/bin/date"
MKDIRPROG="/bin/mkdir"
RMPROG="/bin/rm"
RSYNCPROG="/usr/bin/rsync"

# Read command line arguments
CONF="/home/cube/kitcube_conf.sh"
DELOLD="false"

while getopts di: opt
do	case "$opt" in
	d)	DELOLD="true";;
	i)	CONF="$OPTARG";;
	[?])	echo >&2 "Usage: $0 [-d] [-i inifile] <DEST>"
		exit 1;;
	esac
done

# Load host specific configuration
if [ -f $CONF ] ; then
        source $CONF
fi


# Parameter
SRC="$EXPORTARCHIVE/"
DEST="${!OPTIND}"
SEMFILE="export.semaphore"
WORKDIR=`pwd`

# Complete path?!
cd $DEST
DEST=`pwd`
cd $WORKDIR


# Welcome

echo ""
echo "Export latest data from the archive to a removable disk or backup"
echo ""

# Usage
NARGS=$((OPTIND+0)) # NARGS = (OPTIND-1) + 1 
if [ $# -lt $NARGS -o $# -gt $NARGS ] ; then
        echo -e "Usage: $($BASENAMEPROG "$0") <OPTS> <DEST>  $OPTS"
        echo -e " "
 	echo -e " 	<OPTS>"
	echo -e "	 	d - delete old files"
	echo -e "		i <inifile> - alternative configuration file"
        echo -e "       <DEST>    - destination directory"
        echo -e "                 - source directory: $SRC"
        echo -e " "

	exit 1
fi


# Check if removable disk is mounted
# TODO: Check if DEST is identical or part of the archive?!




# Create destination dir
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



# Check semaphore
if [ -x $DATEPROG ] ; then
        # check if semaphore file exists
        if [ -e "$DEST/$SEMFILE" ] ; then
                # Get PID
                RES=`cat $DEST/$SEMFILE | grep PID | tr -d '"' `
                PID=${RES#*:}
                PID=${PID%*,}
                PIDQUERY=`ps -p $PID | wc -l`
                if [ $PIDQUERY == 2 ] ; then
                        echo -e "\nSemaphore file $DEST/$SEMFILE exists. Either due to errors of previous instance"
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
#echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$DEST/$SEMFILE"
echo "$($BASENAMEPROG "$0"): last start at $TIMESTAMP" > "$DEST/sync_files.log"

TS=`$DATEPROG +"%s"`
echo "{"                                > $DEST/$SEMFILE
echo "  \"Date\": \"`date`\","          >> $DEST/$SEMFILE
echo "  \"Timestamp\": \"$TS\","        >> $DEST/$SEMFILE
echo "  \"PID\": \"$$\","               >> $DEST/$SEMFILE
echo "  \"Destination\": \"$DEST\""     >> $DEST/$SEMFILE
echo "}"                                >> $DEST/$SEMFILE



# Remove old data from the remote disk

NFILES=`find $DEST -type f | wc -l`
NOLD=`find $DEST -type f -mtime +$EXPORTAGE | wc -l`
echo "There are $NFILES files at the export disk"
echo "There are $NOLD files older than $EXPORTAGE days"

if [ $NOLD -gt 0 ] ; then 
	echo "Old files: $DEST/export-to-be-deleted.txt"
	cd $DEST
	find * -type f -mtime +$EXPORTAGE > $DEST/export-to-be-deleted.txt  
	#find * -type f  -mtime +$EXPORTAGE -exec ls -l {} \;
	if [ "$DELOLD" = "true" ] ; then
		find * -type f -mtime +$EXPORTAGE -exec  rm {} \;
	fi
	cd $WORKDIR
	
	echo "Old files deteted"
fi


# Synchronize data 
cd $SRC
find * -type f -not -mtime +$EXPORTAGE > $DEST/export-files.txt
NSYNC=`cat $DEST/export-files.txt | wc -l`
echo "There are $NSYNC files to copy"
echo ""

rsync -av --files-from=$DEST/export-files.txt $SRC $DEST > $DEST/export-res.txt

#find * -type f -not -mtime +$EXPORTAGE | rsync -av --files-from=- $SRC $DEST

echo "Done."
echo "Logfile: $DEST/export-res.txt"
echo "Detete:  $DEST/export-to-be-deleted.txt"
cd $WORKDIR

# Clear 
# delete semaphore file
if [ -x $RMPROG ] ; then
        $RMPROG "$DEST/$SEMFILE"
else
        echo -e "\nError: cannot find or execute $RMPROG."
        echo -e "Aborting execution...\n"
        exit 1
fi  


echo " "




