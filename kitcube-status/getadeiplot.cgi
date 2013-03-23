#! /usr/bin/env python
# Created by Sanshiro Enomoto on 21 March 2011 #


import sys, os, urllib, time
import cgi, cgitb
import Image
import ImageDraw
import ImageFont

from adeiconfig import *


def getplot(timestamp, idlist, length, width=640, height=480):
    if width == None: 
      width = 640;
    if height == None:
      height = 480;
   
    try: 
 
      if use_virtualserver == '1':
        query = '%(server_address)s/services/getimage.php?'\
        'db_server=%(db_server)s&db_name=%(db_name)s&db_group=-3&db_mask=all'\
        '&experiment=0'\
        '&module=graph&virtual=srctree&srctree=%(srctree)s'\
        '&window=%(start)d-%(stop)d'\
        '&width=%(width)s&height=%(height)s'

        param = dict()
        param['server_address'] = adei_url
        param['db_server'] = 'virtual'
        param['db_name'] = 'srctree'
        param['srctree'] = idlist
        param['start'] = int(timestamp) - int(length)
        param['stop'] = int(timestamp)
        param['width'] = width
        param['height'] = height

      else:

        # Get db_group and according ids
        sensorlist = idlist.split(',')
        db_group = ''
        db_mask = ''
        for sensor in sensorlist:
            keylist = sensor.split('__')
            if db_group == '':
                db_group = keylist[2]
            if keylist[2] == db_group:
                if db_mask != '':
                    db_mask += ','
                db_mask += keylist[3]

        #print "db_group = " + db_group
        #print "db_mask  = " + db_mask

        query = '%(server_address)s/services/getimage.php?'\
        'db_server=%(db_server)s&db_name=%(db_name)s&db_group=%(db_group)s&db_mask=%(db_mask)s'\
        '&experiment=0'\
        '&window=%(start)d-%(stop)d'\
        '&width=%(width)s&height=%(height)s'

        param = dict()
        param['server_address'] = adei_url
        param['db_server'] = adei_server
        param['db_name'] = adei_database
        param['db_group'] = db_group
        param['db_mask'] = db_mask
        param['start'] = int(timestamp) - int(length)
        param['stop'] = int(timestamp)
        param['width'] = width
        param['height'] = height

      url = query % param

      #print "URL = " + url
      #return

    
      f = urllib.urlopen(url)
      sys.stdout.write(f.read())

    except:
      im = Image.new( "RGB", (int(width),int(height)), "#ddd"); 
      #sys.stdout.write('{"error": "%s"}\n' % 'unable to connect ADEI server');
      font = ImageFont.truetype("/usr/share/fonts/truetype/verdana.ttf",12);

      draw = ImageDraw.Draw(im);
      draw.text((10, 0), idlist, (0,0,0), font=font);
      im.save(sys.stdout, "PNG");

    return




def main():
    form = cgi.FieldStorage()
    idlist = form.getfirst('idlist')
    length = form.getfirst('length')
    width = form.getfirst('width')
    height = form.getfirst('height')

    # Test parameters
    #idlist = 'kitcube__hatzenbuehl__Data_120_SysCI_txt__1,kitcube__hatzenbuehl__Data_120_SysCI_txt__2' 
    #length = '86400'
    #width = '480'
    #height = '320'

    if not idlist:
        idlist = ','.join([ str(i) for i in range(2, 153) ])

    if not length:
        length = '3600'

    content_type = 'image/png; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)
    getplot(time.time(), idlist, length, width, height)
    #getplot(time.time(), '0,1,2', 100000, 640, 480)

    return 0;


if __name__ == '__main__':
    sys.exit(main())
