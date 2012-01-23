#!/bin/bash
# Send status of the KITcube DAQ system
# A. Kopmann, 11/2011
#
STATCMD="$HOME/kitcube-tools/src/kitcube-reader/kitcube-monitor -i $HOME/kitcube.ini";
STATMSG=$HOME/status-message;
STATLIST="IMK-TRO.kitcube-alarm <TRO.kitcube-alarm@imk.fzk.de>";
STATSUBJ="KITcube Monitor -- Alarm status has changed";


# Use kitcube-monitoring to print the DAQ status 
# Alarms and list of all modules  will be listed in two sections
#
$STATCMD alarm &> $STATMSG
echo "" &>> $STATMSG
$STATCMD status &>> $STATMSG

# 
# TODO: Send mail to the KITcube alarm list
#
echo "TODO: Send status message to \"$STATLIST\"";


