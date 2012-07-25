#!/bin/bash
# Send status of the KITcube DAQ system
# A. Kopmann, 11/2011
#

if [ "$KITCUBEDIR" = "" ] ; then
        KITCUBEDIR=/home/cube
fi

# Default configuration
STATRELAY=cube@katrin.kit.edu
STATCMD="$HOME/kitcube-tools/src/kitcube-reader/kitcube-monitor -i $HOME/kitcube.ini";
STATMSG=$HOME/status-message;
#STATLIST="IMK-TRO.kitcube-alarm <TRO.kitcube-alarm@imk.fzk.de>";
STATLIST="tro.kitcube-alarm@imk.fzk.de";
#STATLIST="andreas.kopmann@kit.edu";
STATSUBJ="KITcube Monitor -- Device Status Display";

# Load host specific configuration
CONF="/home/cube/kitcube_conf.sh"
if [ -f $CONF ] ; then
        source $CONF
fi


# Use kitcube-monitoring to print the DAQ status 
# Alarms and list of all modules  will be listed in two sections
#
$STATCMD alarm &> $STATMSG
echo "" &>> $STATMSG
$STATCMD status &>> $STATMSG

# 
# Send mail to the KITcube alarm list
#
echo "Send status message to \"$STATLIST\"";

echo "Subject: $STATSUBJ"              > status-mail.txt;
echo "From: cube@kitcube.dyndns.info"   >> status-mail.txt;
echo "To: $STATLIST"			>> status-mail.txt;
cat $STATMSG	                   	>> status-mail.txt;
echo ""                                 >> status-mail.txt;

ssh $STATRELAY /usr/sbin/sendmail -t $STATLIST < status-mail.txt;

