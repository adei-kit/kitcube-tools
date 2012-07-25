#! /usr/bin/env python
# Created by Sanshiro Enomoto on 27 March 2011 #


import sys, os, urllib, csv, time, calendar
import cgi, cgitb

from adeiconfig import *



db_group = 'Data_120_SysCI_txt'


def getcsv(timestamp, idlist, length, interval, format):
    query = '%(server_address)s/services/getdata.php?'\
    'db_server=%(db_server)s&db_name=%(db_name)s&db_group=%(db_group)s'\
    '&db_mask=%(db_mask)s'\
    '&format=%(format)s&window=%(start)d-%(stop)d&resample=%(resample)s'

    param = dict()
    param['server_address'] = adei_url
    param['db_server'] = adei_server
    param['db_name'] = adei_database
    param['db_group'] = db_group
    param['db_mask'] = idlist
    param['format'] = format
    param['resample'] = interval
    param['start'] = int(timestamp) - int(length)
    param['stop'] = int(timestamp)
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
    interval = form.getfirst('interval')
    format = form.getfirst('format')

    if not idlist:
        idlist = ','.join([ str(i) for i in range(2, 153) ])
    if not length:
        length = '3600'
    if not interval:
        iterval = '10'
    if not format:
        format = 'csv'

    if format == 'excel':
        content_type = 'application/octet-stream; charset="UTF-8"'
    else:
        content_type = 'text/csv; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)
    getcsv(time.time(), idlist, length, interval, format)
        
    return 0;


if __name__ == '__main__':
    sys.exit(main())
