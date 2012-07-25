#!/bin/sh
#

# Breaking flat folders in a hierarchical structure by date 
# The timestamp of the data file might be obtained from the creating date
# or from the file name of the files
#
# 
#
# TODO: Implement sorting by creation date?!
#

if [ "$KITCUBEDIR" = "" ] ; then
    KITCUBEDIR=/home/cube
fi


DIR="$1"
FORMAT="$2"   

# Predefined formats
FORMAT_CC="kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg"
FORMAT_CM="chm%Y%m%d_%H%M%S.dat"
FORMAT_RPG="%y%m%d"

if [ "$FORMAT" == "CC" ] ; then FORMAT=$FORMAT_CC; fi
if [ "$FORMAT" == "CM" ] ; then FORMAT=$FORMAT_CM; fi
if [ "$FORMAT" == "RPG" ] ; then FORMAT=$FORMAT_RPG; fi

if [ $# -lt 2 ] ; then

echo 
	echo "Usage: $0 <DIR> <FORMAT>"
	echo "  DIR    Base directory, the files are expected in DIR/incoming"
	echo "         All matching file are moved to DIR/flat"
    echo "         A hardlink is created in DIR/data/<year>/<month>/<day>"
    echo " FORMAT  Format that allows the command date to parse the filename"
	echo "         Know formats"
	echo "            Cloud camera: CC = $FORMAT_CC"
  	echo "            Ceilometer:   CM = $FORMAT_CM" 
    echo "            HATPRO:       RPG= (<prefix>_$FORMAT_RPG|$FORMAT_RPG<hour>).<ext1>[.<ext2>].ASC"
    echo ""  
	exit 1
fi

echo "$0: Organizing flat folder by date"
echo "Args: <DIR>=$DIR, <FORMAT>=$FORMAT"



MASK="$DIR/incoming/*"
for FILE in $MASK
do 
	# Skip path
	#FILENAME=`basename $FILE`
	FILENAME=${FILE##*/}
	#echo "<FILENAME>=$FILENAME"

	DATESTRING=$FILENAME

	if [ "$FORMAT" == "$FORMAT_RPG" ] ; then
        	# HATPRO requires to extract the date from the files before applying strptime
        	# Format 1:  yymmddhh.<ext1>.[<ext2>].ASC
        	# Format 2:  <prefix>_yymmdd_hhmmss.<ext1>.[<ext2>].ASC

        	# Skip prefix separated by underscore
        	# Take the first 6 characters
        	DATESTRING=${DATESTRING#*_}
        	DATESTRING=${DATESTRING:0:6}
        
		#echo "RPG filename processing result: $FILENAME --> $DATESTRING"
	fi 

	# Get the target folder name
	if [ `echo $OSTYPE | grep linux` != "" ] ; then 
		#echo "Using linux implementation of BSD data command"
        	TARGETDIR=`/usr/bin/python $KITCUBEDIR/bin/date.py "$FORMAT" $DATESTRING "%Y/%m/%d" 2>/dev/null`
	else
		TARGETDIR=`date -j -u -f "$FORMAT" $DATESTRING "+%Y/%m/%d" 2>/dev/null` 
	fi 
	#echo "<TARGET>=$TARGETDIR"	
 
	if [ "$TARGETDIR" != "" ]; then

		TARGETDIR="$DIR/data/$TARGETDIR"
		FLATDIR="$DIR/flat"
		echo "Moving file $FILE to $TARGETDIR" 	
		if [ ! -d "$TARGETDIR" ] ; then 
		echo "Create dir $TARGETDIR"
			mkdir -p $TARGETDIR
		fi
		if [ ! -d $FLATDIR ] ; then
			echo "Create dir $FLATDIR"
			mkdir -p $FLATDIR
		fi

		mv "$DIR/incoming/$FILENAME" "$FLATDIR"
		ln "$FLATDIR/$FILENAME" "$TARGETDIR/$FILENAME"

	fi
done

echo ""


