#! /usr/bin/env python
#

import sys
import CC2



if len(sys.argv) < 2:
   print "Usage: ..." 
   exit(0)


image = sys.argv[1]
CC2.mean(image)

