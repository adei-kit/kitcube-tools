#!/bin/sh
#


date -j -u -f "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg" "kitcube_cc2_2010-10-15-23-54-11.jpg"

date -u -f "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg" "kitcube_cc2_2010-10-15-23-54-11.jpg"


TIMESTAMP=`date -j -u -f "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg" "kitcube_cc2_2010-10-15-23-54-11.jpg" "+%s"`
echo $TIMESTAMP

DIR=`date -j -u -f "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg" "kitcube_cc2_2010-10-15-23-54-11.jpg" "+%Y/%m/%d"` 
echo $DIR


PROXYPROG="/home/cube/bin/kcmon"
KEY="Sync.111"
FILENAME="Test"
ATIME="12345678"

echo "$PROXYPROG set $KEY secData=$ATIME"
$PROXYPROG set $KEY secData=$ATIME,status=\'$FILENAME\'
RETURN_VALUE=$?

echo "Return: $RETURN_VALUE"
if [ $RETURN_VALUE != 0 ] ; then
	echo "Need to create key=$KEY"
fi


