#!/usr/bin/env python
import MySQLdb as sql
import numpy as np

TEST = 1

class SQLReader:
    def __init__(self, host, db, user, passwd):
        self.db = sql.connect(host, user, passwd, db)
        self.cursor = self.db.cursor()
        self.TABLES = []
        self.SENSORS = {}

        self.listTables()   
        self.listSensors()

    def listTables(self):
        self.TABLES = []
        self.cursor.execute("SHOW TABLES")
        tables = self.cursor.fetchall()
        for t in tables:
            self.cursor.execute("DESC %s" % t[0])
            sensorlist = []
            for s in self.cursor.fetchall():
                sensorlist.append(s[0])
            self.TABLES.append({
                'name': t[0],
                'sensors': sensorlist
            })
        if TEST:
            print self.TABLES
 
    def listSensors(self):
        """
            get all sensor info
        """
        self.cursor.execute("SELECT * FROM Axislist")
        axis = self.cursor.fetchall()
        self.cursor.execute("SELECT * FROM Sensorlist")
        sensors = self.cursor.fetchall()

        if TEST:
            for s in sensors: print s
        self.SENSORS = {}
        for sensor in sensors:
            # axis unit
            if sensor[4] == 0:
                unit = ''
            else:
                self.cursor.execute( "SELECT * FROM Axislist WHERE ID = %s" % int(sensor[4]) )
                unit = self.cursor.fetchone()[3]
            # second axis unit
            if sensor[8]: 
                self.cursor.execute( "SELECT * FROM Axislist WHERE ID = %s" % int(sensor[8]) )
                unit2 = self.cursor.fetchone()[3]
            else:
                unit2 = ''
            # find table that contains sensor
            sensorname = sensor[1]
            for t in self.TABLES:
                if any(sensorname == s for s in t['sensors']):
                    tablename = t['name']
            # sensor info
            self.SENSORS[sensorname] = {
                'axis_unit': unit,
                'axis2_unit': unit2,
                'axis2_val': sensor[8],
                'table': tablename or ''
            }

    def sensors(self):
        if not self.SENSORS:
            self.listSensors()
        return self.SENSORS

    def lastSensorStamp(self, sensorname):
        sensor = self.SENSORS[sensorname]
        self.cursor.execute("SELECT usec FROM %s ORDER BY id DESC limit 1" % sensor['table'])
        return self.cursor.fetchone()[0]

    def fetchSensor(self, t0, t1, *sensorname):
        """
        t0,t1: utc time since epoch in seconds
        senorname: assume all sensor are from same table
        """
        t0 = t0*1000000
        t1 = t1*1000000
        tables = [ self.SENSORS[s]['table'] for s in sensorname]
        sensors = ','.join([ '`%s`' % s for s in sensorname ])
        querystr =  "SELECT usec,{0} FROM {1} WHERE usec >= {2} and usec <= {3}".format(\
                    sensors, tables[0], t0, t1)
        print querystr
        self.cursor.execute(querystr)
        res = self.cursor.fetchall()

        #dtype = [('usec', 'i4')] + [(s, 'f8') for s in sensorname]
        #res = np.array(res, dtype = dtype)
        #print res
        return list(res)
        

def main():
    host = 'imk-db1'
    db = 'HDCP2'
    user = 'cube'
    passwd = 'cube'

    sql_hdcp2 = SQLReader(host, db, user, passwd)

    sensors = sql_hdcp2.sensors()
    for k,v in sensors.iteritems():
        print k, v

    stamp = sql_hdcp2.lastSensorStamp('L1B.AZIMUTH.ANGLE')
    print stamp

    data = sql_hdcp2.fetchSensor(1363795203, 1363795268, 'L1B.AZIMUTH.ANGLE')
    print data

    data = sql_hdcp2.fetchSensor(1363795203, 1363795268, 'L1B.AZIMUTH.ANGLE', 'L1B.ELEVATION.ANGLE')
    print data


if __name__ == '__main__':
    main()
