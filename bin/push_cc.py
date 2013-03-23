#! /usr/bin/env python
#


import os
import sys
import time
import datetime
import Image
import ImageDraw
import ImageFont


# Parameter

# Source folder
#DAQDIR = "/Users/kopmann/data/kitcube/045/CC4/20*"
DAQDIR = "/home/cube/archive/045/CC2/data/20*"

# Target dir
LATESTFILE = "CC-latest.jpg"
REMOTEDIR = "public_html/kitcubestatus/data"
WEBSERVER = "imk-adei1"


# Filename format to get the date
FORMAT_CC="kitcube_cc2_%Y-%m-%d-%H-%M-%S.jpg"
FORMAT_CM="chm%Y%m%d_%H%M%S.dat"

inFormat = FORMAT_CC

# Center part of the images to be displayed
centercrop = 0.65


#########

print "Searching for the latest file of a certain device"

FILEMASK = "*.jpg"
TMP = "/home/cube/tmp/CC-latest.txt"

# TODO: Try to improve performance with the newer flag
#       Problem newer or equal is the required function !!!
#       Calculate the number of days since the last file and use the mtime option
#
# Use the resized image from the small folder !!!
# The last file is always stored in convert.marker !!!!
try:
   fp = open(TMP,"r")
   latestFile = fp.read()
   latestFile = " -newer " + latestFile[:-1]
   fp.close()
except:
   print "No file with last result found - searching full path"

CMD = "find " + DAQDIR +  " -mtime -5 -type f -name \"" + FILEMASK + "\" | sort -r | head -n 1 > " + TMP
print CMD
os.system(CMD)
print "Done"

fp = open(TMP,"r")
latestFile = fp.read()
latestFile = latestFile[:-1] # drop the new line character
fp.close()

print "Latest file is " + latestFile


# Get the date of this file ?!
# Parse the filename and use the device specific naming scheme
# Or take the date from the filesystem?!
#

timeString  = os.path.basename(latestFile)
#print "Latest file is " + timeString

# Parse string
try: 
    timestamp = time.strptime(timeString, inFormat)
    tdata = time.mktime(timestamp) 
    tnow = time.time()
    age = tnow - tdata

    print "Age of the lastest image (sec): " + str(age)

    if (len(sys.argv) > 3):         
        outFormat = sys.argv[3]
        outString = datetime.datetime(*timestamp[0:6]).strftime(outFormat) 
    else:
        outString = datetime.datetime(*timestamp[0:6])

    print outString
except:
    print >> sys.stderr,  "Format does not match"



# Image processing 
# Resize to 480 x 320  4:3
# Take only the middle part of the image
#
# TODO: Use the same code also for the processing of the small CC images
#

try:
	img = Image.open(latestFile)

	(x,y) = img.size
	#print "Image size is " + str(x) + 'x' + str (y) 

	height = int ( centercrop * y ) 
	width = height * 4 / 3
	xmin = (x-width)/2
	xmax = xmin + width
	ymin = (y-height)/2
	ymax = ymin + height

	center = (xmin,ymin,xmax,ymax)
	img= img.crop(center)

	img2 = img.resize((480,320))
except:
	img2 = Image.new("RGB", (480,320), "#000000")
	font = ImageFont.truetype("/usr/share/fonts/truetype/verdana.ttf",18)
	draw = ImageDraw.Draw(img2)
	draw.text((330, 5), "Image corrupt", (180,180,180), font)

# Add timestamp to the image
# TODO: Find out how to select the font?!

#font = ImageFont.load_default()
#font = ImageFont.load_path("/Library/fonts/Verdana.ttf",12)
font = ImageFont.truetype("/usr/share/fonts/truetype/verdana.ttf",18)
bigfont = ImageFont.truetype("/usr/share/fonts/truetype/verdana.ttf",50)

draw = ImageDraw.Draw(img2)
draw.text((10, 5), str(outString), (180,180,180), font)

if age > 3600: 
	print "Print OUTDATED"
  	draw.text((130,100), str("CAMERA"), (212,0,0), bigfont)
	draw.text((123,160), str("STOPPED"), (212,0,0), bigfont)

#img2.show()


# Save
# TODO: Find a nice ssh/scp Implementation for Pyhton

img2.save(LATESTFILE)

os.system('scp "%s" "cube@%s:%s"' % (LATESTFILE, WEBSERVER, REMOTEDIR) )




