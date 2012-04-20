#!/bin/sh
#


date -j -u -f "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg" "kitcube_cc2_2010-10-15-23-54-11.jpg"

TIMESTAMP=`date -j -u -f "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg" "kitcube_cc2_2010-10-15-23-54-11.jpg" "+%s"`
echo $TIMESTAMP

#DIR=`date -j -u -f "kitcube_cc2_%Y-%m-%d.jpg" "kitcube_cc2_2010-10-15.jpg" "+%Y/%m/%d"` 
DIR=`date -j -u -f "rhi_%y%m%d" "rhi_120203" "+%Y/%m/%d"`
echo $DIR



