#!/usr/bin/env python
import os
import sys
import cgi, cgitb
import datetime
import numpy as np
import simplejson as json
from sqlreader import SQLReader
import config
import matplotlib as mpl
import matplotlib.cm as cm
mpl.use('Agg')
import matplotlib.pyplot as plt

TEST = 1

if TEST:
    today = datetime.datetime(2013, 5, 30)
else:
    today = datetime.date.today()
    today = datetime.datetime(today.year, today.month, today.day)

stamp = (today - datetime.datetime(1970,1,1)).days*24*3600

sensorID = \
        [['L2C.AIR.POT.TEM.PRF.{0:03}'.format(i) for i in range(43)],
         ['L2C.REL.HUM.PRF.{0:03}'.format(i) for i in range(43)]]
sensorName = \
        ['HATPRO.L2C.AIR.POT.TEM.PRF',
         'HATPRO.L2C.REL.HUM.PRF'] 


def main():
    sql = SQLReader(config.host, config.db, config.user, config.passwd)
    sensors = sql.sensors()

    for ID , name in zip(sensorID, sensorName):
        t0 = stamp
        t1 = stamp + 86400
        lastts = sql.lastSensorStamp(ID[0])
        filename = "../cache/%s.%s.json" % (name,stamp)
     
        with open(filename, 'w') as f:
            ######## read data
            data = np.array(sql.fetchSensor(t0, t1, *ID))
            time = data[:,0]
            data = data[:,1:]
            height = [sensors[s]['axis2_val'] for s in ID]

            ######## process data
            Paths = []
            #colormap = cm.YlGnBu
            #contours = plt.contourf(time, height, data.T, 40, cmap=colormap)
            contours = plt.contourf(time, height, data.T, 40)
            for l, p, c in zip(contours.layers, 
                               contours.collections, 
                               contours.tcolors):
                color = mpl.colors.rgb2hex(c[0][:3])
                for path in p.get_paths():
                    pp = []
                    for vertex, code in zip(path.vertices, path.codes):
                        if len(pp) != 0 and code == 1:
                            pp.append(None)
                            pp.append(vertex.tolist())
                        else:
                            pp.append(vertex.tolist())

                    Paths.append({"layer": l, "path": pp, "color": color})

            ######## output
            output = {
                    "xmin": time[0], 
                    "xmax": time[-1], 
                    "ymin": height[0], 
                    "ymax": height[-1],
                    "time": list(time),
                    "data": Paths
                   }
            json.dump(output, f)
            #print np.array(output['data'])


if __name__ == "__main__":
    #cgitb.enable()
    #content_type = 'text/plain; charset="UTF-8"'
    #sys.stdout.write('Content-type: %s\n\n' % content_type)
    main()
    #sys.exit()


