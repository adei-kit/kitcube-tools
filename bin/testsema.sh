#!/bin/sh
#

SEM="./sync.semaphore.2"

RES=`cat $SEM | grep PID | tr -d '"' `
echo $RES 

RES=${RES#*:}
echo /$RES/
echo Increment by one $((RES+1))

ACTIVE=`ps -p $RES | wc -l`
if [ $ACTIVE == 2 ] ; then 
	echo "Process is still running"
fi

 

