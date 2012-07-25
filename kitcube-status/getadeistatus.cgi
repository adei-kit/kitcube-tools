#! /usr/bin/env python
# Created by A Kopmann on 18 May 2012 #
# Read module status from KITcube database #


import sys, time

try:
    import MySQLdb
    usemysql = 1
except ImportError, e:
    usemysql = 0


from adeiconfig import *


def getstatus():
    # connect to the MySQL server
    try:
        conn = MySQLdb.connect (host = kcdb_server,
                                user = kcdb_user,
                                passwd = kcdb_passwd,
                                db = kcdb_name)
    except MySQLdb.Error, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit (1)

    # perform a fetch loop using fetchall()
    cursor = conn.cursor ()
    cursor.execute ("SELECT module, moduleName, secData, appId, sec, status, alarmLimit, alarm, alarmEnable, sensorGroup, skey, appName FROM Statuslist ORDER BY module")
    rows = cursor.fetchall ()


    sys.stdout.write('{\n');
    timestamp = int(time.time());
    sys.stdout.write('    "timestamp": "%d",\n' % timestamp);
    sys.stdout.write('    "modules": [');

    index = 0;
    for row in rows:
        #print "%s, %s, %s, %s" % (row[0], row[1], row[2], row[3])

        if index == 0:
            sys.stdout.write('\n        {')
        else:
            sys.stdout.write(', \n        {')
        sys.stdout.write('"ID": "%s", ' % index)
        sys.stdout.write('"module": "%s", ' % row[0])
        sys.stdout.write('"moduleName": "%s", ' % row[1] )
        sys.stdout.write('"secData": "%s", ' % row[2])

        # Last data / delay
        tdiff = timestamp - int(row[2]); 
        tm = time.gmtime(int(row[2]));
        #sys.stdout.write('"lastData": "%d", ' % tdiff)
        if tdiff < 172800: 
            sys.stdout.write('"lastData": "%02d:%02d:%02d", ' %(tm.tm_hour, tm.tm_min, tm.tm_sec) )
            sys.stdout.write('"delayedBy": "%02d:%02d:%02d", ' %(tdiff/3600, (tdiff/60)%60, tdiff%60) )
        else:   
            sys.stdout.write('"lastData": "%02d.%02d.%4d", ' %(tm.tm_mday, tm.tm_mon, tm.tm_year) ) 
            sys.stdout.write('"delayedBy": "%d days", ' %(tdiff/86400) ) 

        sys.stdout.write('"appId": "%s", ' % row[3])
        sys.stdout.write('"sec": "%s", ' % row[4])
        sys.stdout.write('"status": "%s", ' % row[5])
        # TODO: Add status string 
        # ALARM, disabled, not configured, ...
        # Grey out the status of non configured objects
        alarmLimit = int(row[6]);
        alarmEnable = int(row[8]);
        #sys.stdout.write('"alarmStatus": "%s %s", ' % (alarmLimit, alarmEnable))
        if (alarmLimit == 0):
            sys.stdout.write('"alarmStatus": "--------", ')
        elif (alarmEnable == 0):
            sys.stdout.write('"alarmStatus": "disabled", ')
        elif (tdiff > alarmLimit) and (alarmLimit > 0)  and (alarmEnable > 0):
            sys.stdout.write('"alarmStatus": "ALARM", ')
        else:
            sys.stdout.write('"alarmStatus": "", ')

        sys.stdout.write('"alarmLimit": "%s", ' % row[6])
        sys.stdout.write('"alarm": "%s", ' % row[7])
        sys.stdout.write('"alarmEnable": "%s", ' % row[8])
        sys.stdout.write('"sensorGroup": "%s", ' % row[9])
        sys.stdout.write('"skey": "%s", ' % row[10])
        sys.stdout.write('"appName": "%s"' % row[11])
        sys.stdout.write('}');
        index += 1;

    #sys.stdout.write('\n    ],\n');
    sys.stdout.write('\n    ]\n');
    sys.stdout.write('}\n');


    #print "Number of rows returned: %d" % cursor.rowcount
    cursor.close ()

    conn.commit ()
    conn.close ()



def main():
    if (len(kcdb_server) > 0) and (usemysql == 1):
        content_type = 'text/plain; charset="UTF-8"'
        sys.stdout.write('Content-type: %s\n\n' % content_type)
    
        getstatus();

    else:
        file = open(kcdb_file)
        res = file.read()
        print res

    return 0;


if __name__ == '__main__':
    sys.exit(main())
