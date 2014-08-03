#! /usr/bin/env python
# Get the latest image from the cloud camera
#

import sys, os, urllib, time
import cgi, cgitb
#import cStringIO # *much* faster than StringIO
import urllib
#import Image

from adeiconfig import *

# Get position of last image from configuration file
latestfile = xr_lastimage


def getplot():

    file = urllib.urlopen(latestfile)
    img = file.read()
    print img

#    im = cStringIO.StringIO(file.read())
#    img = Image.open(im)
#    im.print()

def main():
    form = cgi.FieldStorage()

    content_type = 'image/png; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)

    getplot()

    return 0;
	

if __name__ == '__main__':
    sys.exit(main())

