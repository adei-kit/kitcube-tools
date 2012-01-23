#!/bin/bash
# Check for alarms in the KITcube setup
# A. Kopmann, 11/2011
#
ALARMTST="../src/kitcube-reader/kitcube-monitor -i ../etc/kitcube.ini newalarm";
ALARMLOG=$HOME/kcr-alarm.log;
ALARMLIST="IMK-TRO.kitcube-alarm <TRO.kitcube-alarm@imk.fzk.de>";
ALARMSUBJ="KITcube Monitor -- Alarm status has changed";

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
	cat alarm-message.txt >> $ALARMLOG;
	echo "" >> $ALARMLOG;

	# 
	# TODO: Send mail to the KITcube alarm list
	#
        echo "TODO: Send alarm to \"$ALARMLIST\"";
else
	echo "Everything is OK";
fi

