#!/bin/sh
#
# Example illustrating use of getopts builtin. This
# shell script would implement the paste command,
# using getopts to process options, if the underlying
# functionality was embedded in hypothetical utilities
# hpaste and vpaste, which perform horizontal and
# vertical pasting respectively.
#
paste=vpaste	# default is vertical pasting
seplist="\t"	# default separator is tab

while getopts d:s o  
do
   	case "$o" in
		d)	echo "Option d <NUM>"; seplist="$OPTARG";;
		s)	echo "Option s"; paste=hpaste;;
		[?])	echo >&2 "Usage: $0 [-s] [-d seplist] file ..."
			exit 1;;
	esac
done
#shift $OPTIND-1

# Other arguments?
echo "Next argument: $OPTIND of $#"
echo "$0 $1 $2 $3  // ${!OPTIND}"


# perform actual paste command
#$paste -d "$seplist" "$@"

echo "Option d: $seplist"
echo "Option s: $paste"


