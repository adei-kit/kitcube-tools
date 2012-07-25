#! /usr/bin/env python
# Created by Sanshiro Enomoto on 29 March 2011 #


import sys, os, urllib, time
import cgi, cgitb

from adeiconfig import *



db_group = 'Data_120_SysCI_txt'


def getlegend(timestamp, idlist, length):
    query = '%(server_address)s/services/legend.php?'\
    'time_format=text'\
    '&db_server=%(db_server)s&db_name=%(db_name)s&db_group=%(db_group)s'\
    '&db_mask=%(db_mask)s'\
    '&xmin=%(start)d&xmax=%(stop)d&ymin=0&ymax=900&x=%(middle)d&y=141'

    param = dict()
    param['server_address'] = adei_url
    param['db_server'] = adei_server
    param['db_name'] = adei_database
    param['db_group'] = db_group
    param['db_mask'] = idlist
    param['start'] = int(timestamp) - int(length)
    param['stop'] = int(timestamp)
    param['middle'] = int(timestamp) - int(length)/2;
    url = query % param

    try:
        f = urllib.urlopen(url)
    except:
        sys.stdout.write('{"error": "%s"}\n' % 'unable to connect ADEI server');
        return

    sys.stdout.write(f.read())


def main():
    form = cgi.FieldStorage()
    idlist = form.getfirst('idlist')
    length = form.getfirst('length')

    if not idlist:
        idlist = ','.join([ str(i) for i in range(2, 153) ])

    if not length:
        length = '3600'

    content_type = 'text/plain; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)
    getlegend(time.time(), idlist, length)
        
    return 0;


if __name__ == '__main__':
    sys.exit(main())
