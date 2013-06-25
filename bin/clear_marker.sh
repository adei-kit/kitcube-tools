#!/bin/sh
# Remove the marker files from the archive


while getopts f opt
do      case "$opt" in
        f)  REMOVE="yes";;
	[?])    echo >&2 "Usage: $0 [-f] <DIR>"
                exit 1;;
         esac
done 
DIR="${!OPTIND}"
NARGS=$((OPTIND+0))

if [ $# -lt $NARGS ]; then
        echo "clear_marker <archiveDir>"
        exit
fi

find $DIR -type f  -name ".kitcube-reader.marker.*"  -exec ls -l {} \;

if [  "$REMOVE" = "yes" ]; then
	echo "...removing..."
	find $DIR -type f  -name ".kitcube-reader.marker.*"  -exec rm {} \;
fi
