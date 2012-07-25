#!/bin/bash
# Check for alarms in the KITcube setup
# A. Kopmann, 11/2011
#

if [ "$KITCUBEDIR" = "" ] ; then
	KITCUBEDIR=/home/cube
fi

# Default configuration
ALARMRELAY=cube@katrin.kit.edu
ALARMTST="$KITCUBEDIR/kitcube-tools/src/kitcube-reader/kitcube-monitor -i $KITCUBEDIR/kitcube.ini newalarm";
ALARMLOG=$KITCUBEDIR/alarm-$HOSTNAME-chk_alarm.log;
#ALARMLIST="IMK-TRO.kitcube-alarm <TRO.kitcube-alarm@imk.fzk.de>";
#ALARMLIST="tro.kitcube-alarm@imk.fzk.de";
ALARMLIST="andreas.kopmann@kit.edu";
ALARMSUBJ="KITcube Monitor -- Alarm status has changed";


# Load host specific configuration
CONF="/home/cube/kitcube_conf.sh"
if [ -f $CONF ] ; then
        source $CONF
fi


#
# Create alarm file w header 
#
if [ ! -f $ALARMLOG ]; then
	echo "Create alarm log" 
	echo "KITcube reader alarm log" > $ALARMLOG;
	echo "" >> $ALARMLOG;
fi

# Use kitcube-monitoring to check for changes in the alarm state 
# New and cleared will be listed in two sections
#
$ALARMTST &> alarm-message.txt
if [ $? -eq 1 ]; then
	echo "Alarm ocurred - Copy message to log file $ALARMLOG";
	cat alarm-message.txt 			>> $ALARMLOG;
	echo "" 				>> $ALARMLOG;

	# 
	# Send mail to the KITcube alarm list
	#
        echo "TODO: Send alarm to \"$ALARMLIST\"";
	echo "Subject: $ALARMSUBJ" 		> alarm-mail.txt;
	echo "From: cube@kitcube.dyndns.info"  	>> alarm-mail.txt;
	echo "To: $ALARMLIST"			>> alarm-mail.txt;
	cat alarm-message.txt 			>> alarm-mail.txt;
	echo "" 				>> alarm-mail.txt;

	ssh $ALARMRELAY /usr/sbin/sendmail -t $ALARMLIST < alarm-mail.txt;
else
	echo "Everything is OK";
fi


