#
#
#

import sys
import time
import datetime

print "Today is day", time.localtime()[7], "of the current year" 

res = time.strptime("16/6/1981", "%d/%m/%Y")
print res

res = time.strptime("kitcube_cc2_2010-10-15-23-54-11.jpg", "kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg")
print res
print res[0:6]


print datetime.datetime(*res[0:6]).strftime("%Y/%m/%d")

if (len(sys.argv) <= 1):
	print sys.argv[0], ": No arguments given"

