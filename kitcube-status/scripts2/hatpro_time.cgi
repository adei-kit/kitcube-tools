#!/usr/bin/env python
import os
import sys
import cgi, cgitb
import datetime
import numpy as np
import simplejson as json
from sqlreader import SQLReader
import config

TEST=0

if TEST:
    sensorID = [['L2A.ELEVATION.ANGLE', 'L2A.ATM.WAT.VAP.CNT']]
    sensorName = ['L2A.ATM.WAT.VAP.CNT']
    d0 = datetime.datetime(2013, 5, 30)
    stamp = (d0 - datetime.datetime(1970,1,1)).days*24*3600
else:
    #reqparams = cgi.FieldStorage()
    #today = datetime.datetime(2013, 5, 28)
    today = datetime.date.today()
    stamp = (datetime.datetime(today.year, today.month, today.day) 
            - datetime.datetime(1970,1,1)).days*24*3600
    sensorID = [
            ['L2A.ATM.WAT.VAP.CNT', 'L2A.ELEVATION.ANGLE'],
            ['L2A.ATM.LIQ.WAT.CNT', 'L2A.ELEVATION.ANGLE'],
            ['L1B.BRIGHT.TEMP.IR.000', 'L1B.BRIGHT.TEMP.IR.001', 'L1B.ELEVATION.ANGLE'],
            ['L1B.BRIGHT.TEMP.{0:03}'.format(i) for i in range(13)] + ['L1B.ELEVATION.ANGLE'],
            ] 
    sensorName =  ['HATPRO.L2A.ATM.WAT.VAP.CNT',
                   'HATPRO.L2A.ATM.LIQ.WAT.CNT',
                   'HATPRO.L1B.BRIGHT.TEMP.IR',
                   'HATPRO.L1B.BRIGHT.TEMP']

def main():
    sql = SQLReader(config.host, config.db, config.user, config.passwd)

    for ID , name in zip(sensorID, sensorName):
        t0 = stamp
        t1 = stamp + 86400
        lastts = sql.lastSensorStamp(ID[0])
        filename = "../cache/%s.%s.json" % (name,stamp)
     
        with open(filename, 'w') as f:
            ######## read data
            data = sql.fetchSensor(t0, t1, *ID) 
            ######## process data
            filtered_data = \
                [ [d[0]/1000000] + list(d[1:-1]) for d in data if d[-1]==90 ]
            filtered_data = np.array(filtered_data).T
            ######## output
            output = {  'time': list(filtered_data[0]),
                        'data': [list(d) for d in filtered_data[1:]]  }
            json.dump(output, f)
            #print np.array(output['data'])



if __name__ == "__main__":
    #cgitb.enable()
    content_type = 'text/plain; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)
    main()
    sys.exit()

