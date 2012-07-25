#!/Library/Frameworks/Python.framework/Versions/2.7/bin/python
# Created by A Kopmann on 18 May 2012 #
# Read module status from KITcube database


import sys, time
import MySQLdb

db_server = 'localhost'
db_name = 'kitcube_active'
db_user = 'cube'
db_passwd = 'cube'

# Use JSON file that has been pushed to server?!
db_server =''
#db_file = 'modulestatus.json'
db_file = 'modstat.json'



def main():
    if len(db_server) > 0:
        content_type = 'text/plain; charset="UTF-8"'
        sys.stdout.write('Content-type: %s\n\n' % content_type)
    
        #getstatus();

    else:
        content_type = 'text/plain; charset="UTF-8"'
        sys.stdout.write('Content-type: %s\n\n' % content_type)

        file = open(db_file)
        res = file.read()
        print res

    return 0;


if __name__ == '__main__':
    sys.exit(main())
