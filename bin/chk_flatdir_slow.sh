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
	echo "  DIR    Base directory, the files are expected in DIR/flat"
    echo "         The hardlink should be in DIR/<year>/<month>/<day>"
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



MASK="$DIR/flat/*"
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
    if [ "$OSTYPE" = "linux" ] ; then 
        TARGETDIR=`python /home/cube/bin/date.py "$FORMAT" $DATESTRING "%Y/%m/%d" 2>/dev/null`
    else
        TARGETDIR=`date -j -u -f "$FORMAT" $DATESTRING "+%Y/%m/%d" 2>/dev/null` 
    fi 
    #echo "<TARGET>=$TARGETDIR"	
 

	if [ "$TARGETDIR" != "" ]; then

		TARGETDIR="$DIR/$TARGETDIR"
		FLATDIR="$DIR/flat"
	
		if [ ! -d $TARGETDIR ] ; then 
            echo "Target dir missing $TARGETDIR"
			mkdir -p $TARGETDIR
		fi
		

		RESDIFF=`diff -q "$TARGETDIR/$FILENAME" "$FLATDIR/$FILENAME"`
        if [ "$OSTYPE" = "linux" ] ; then 
            RESSTAT1=`stat -c "%X %Y %Z" "$TARGETDIR/$FILENAME"` 
            RESSTAT2=`stat -c "%X %Y %Z" "$FLATDIR/$FILENAME"`
        else
            RESSTAT1=`stat -f "%a %m %c %B" "$TARGETDIR/$FILENAME"`
            RESSTAT2=`stat -f "%a %m %c %B" "$FLATDIR/$FILENAME"`
        fi 

        if [ "$RESDIFF" != "" ] ; then
            echo "$FILENAME files differ ($RESDIFF)";
        fi 
        
        if [ "$RESSTAT1" != "$RESSTAT2" ] ; then
            echo "$FILENAME timestamps differ";
        fi 


	fi
done

echo ""


