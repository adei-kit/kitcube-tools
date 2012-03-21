#
#
#

import sys
import time
import datetime

#
# Print help page
#
if (len(sys.argv) < 3):
	print "Parsing date from string with formated output"
	print "Usage: ", sys.argv[0], " <FORMAT> <STRING> <OUTPUT>"
	print "   <FORMAT>  Format of the string (strptime convention)"
	print "   <STRING>  The string that contains the time information"
	print "   <OUTPUT>  Output format of the time (again strptime convention)"
	print ""

else:
	inFormat  = sys.argv[1]
	timeString  = sys.argv[2]

	# Parse string
	try: 
		timestamp = time.strptime(timeString, inFormat)
		
		if (len(sys.argv) > 3):		
			outFormat = sys.argv[3]
			outString = datetime.datetime(*timestamp[0:6]).strftime(outFormat) 
		else:
			outString = datetime.datetime(*timestamp[0:6])

		print outString
	except:
		print >> sys.stderr,  "Format does not match"

