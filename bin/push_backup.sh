#!/bin/sh
# Make an incremental backup of the system configuration
#

# Default configurations
BACKUPHOST="cube@katrin.kit.edu"
BACKUPDIR=backup/$HOSTNAME
BACKUPEXCL=/home/cube/etc/rsync/backup-exclude
SEMFILE="/home/cube/backup.semaphore"
SRC=/home/cube/


# Applications
BASENAMEPROG="/usr/bin/basename"
DATEPROG="/bin/date"
RSYNCPROG="/usr/bin/rsync"
RMPROG="/bin/rm"

# Load host specific configuration 
CONF="/home/cube/kitcube_conf.sh"
if [ -f $CONF ] ; then
	source $CONF 
fi


#Usage
NARGS=1
if [ $# -lt $NARGS -o $# -gt $NARGS ] ; then
	echo -e "Usage:	$($BASENAMEPROG "$0")"
	echo -e " "
	echo -e "Make an incremental backup of the DAQ system"
	echo -e " "
	echo -e "Backup drive:   $BACKUPHOST/$BACKUPDIR"
	echo -e " "
fi


# Make backup
# Check semaphore
if [ -e "$SEMFILE" ] ; then
	# Get PID
        RES=`cat $DEST/$SEMFILE | grep PID | tr -d '"' `
        PID=${RES#*:}
        PID=${PID%*,}
        PIDQUERY=`ps -p $PID | wc -l`
        if [ $PIDQUERY == 2 ] ; then
		echo -e "\nSemaphore file exists. Either due to errors of previous instance"
		echo -e "or because there is another session running in parallel."
		echo -e "Aborting execution...\n "
		exit 1
	fi
fi

# Create semaphore file
TS=`$DATEPROG +"%s"`
echo "{"                                > $DEST/$SEMFILE
echo "  \"Date\": \"`date`\","          >> $DEST/$SEMFILE
echo "  \"Timestamp\": \"$TS\","        >> $DEST/$SEMFILE
echo "  \"PID\": \"$$\","               >> $DEST/$SEMFILE
echo "  \"Destination\": \"$DEST\""     >> $DEST/$SEMFILE
echo "}"                                >> $DEST/$SEMFILE


# Create basedir
ssh $BACKUPHOST "if [ ! -d $BACKUPDIR ] ; then mkdir -p $BACKUPDIR; fi"


date=`date "+%Y-%m-%d__%H_%M_%S"`

rsync -azP \
  --delete \
  --delete-excluded \
  --exclude-from=$BACKUPEXCL \
  --link-dest=../current \
  $SRC $BACKUPHOST:$BACKUPDIR/incomplete_back-$date 
ssh $BACKUPHOST \
  "mv $BACKUPDIR/incomplete_back-$date $BACKUPDIR/back-$date \
  && rm -f $BACKUPDIR/current \
  && ln -s back-$date $BACKUPDIR/current"


# Clean up
rm "$SEMFILE"


