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


if [ $# -lt 1 ] ; then

echo 
	echo "Usage: $0 <DIR> <FORMAT>"
	echo "  DIR    Base directory, the files are expected in DIR/flat"
    echo "         The hardlink should be in DIR/<year>/<month>/<day>"
    echo ""  
	exit 1
fi

echo "$0: Organizing flat folder by date"
echo "Args: <DIR>=$DIR"
echo ""

ERR=0
MASK="$DIR/flat/*"
for FILE in $MASK
do 
	# Skip path
	#FILENAME=`basename $FILE`
	FILENAME=${FILE##*/}
    #echo "<FILENAME>=$FILENAME"

    FLATDIR="$DIR/flat"
	
    RESSTAT=`stat -c %h $FLATDIR/$FILENAME`
    if [ "$RESSTAT" = "1" ] ; then
        echo "$FILENAME hardlink missing"
        ERR=1
    fi
		
done

RESFIND1=`find 20*  -type f | wc -l`
RESFIND2=`find $FLATDIR  -type f | wc -l`
if [ "$RESFIND1" != "$RESFIND2" ] ; then
    echo "Warning: Number of files in flat and 20** dirs differs"
fi

if [ "$ERR" = "0" ] ; then
    echo "Links for all files found"
fi


echo ""


