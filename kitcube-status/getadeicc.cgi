#! /usr/bin/env python
# Get the latest image from the cloud camera
#

import sys, os, urllib, time
import cgi, cgitb

from adeiconfig import *

# Get position of last image from configuration file
latestfile = cc2_lastimage


def getplot():

    fp = open(latestfile, "r")
    img = fp.read()
    print img


def main():
    form = cgi.FieldStorage()

    content_type = 'image/png; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)

    getplot()

    return 0;
	

if __name__ == '__main__':
    sys.exit(main())

