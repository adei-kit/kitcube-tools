#! /usr/bin/env python
#

import os, sys, time

from adeiconfig import *


hostlist = ci_hostlist 


def getdata():
    sys.stdout.write('{\n');
    timestamp = int(time.time());
    sys.stdout.write('    "timestamp": "%d",\n' % timestamp);
    sys.stdout.write('    "ports": [');

    first = 1;
    hostid = 0;
    for host in hostlist:
      for index in range(12):
	 # Use snmp protocol to get configration and status of the power ports
         # Note: Snmp-get needs to be enabled for the power plugs 
         cmd = 'snmpget -v2c  -c public -OvqU ' + host + ' GUDEADS-EPC2X6-MIB::epc2x6PortName.' + str(index+1)
	 name = os.popen(cmd).readline()
	 name = name[:-1]
         name = name.strip('"')
 
         display = 1
         # Drop unused ports
         if name.find("Power Port") >= 0: display = 0

         if (display == 1):
	    # Drop the prefix A1: | B1: and so on
	    pos = name.find(":")
	    if pos >= 0: name = name[pos+2:]

            cmd = 'snmpget -v2c  -c public -OvqU ' + host + ' GUDEADS-EPC2X6-MIB::epc2x6PortState.' + str(index+1)
            value = os.popen(cmd).readline()
	    value = value[:-1]
            # Drop unused ports
	    # Drop the prefix A1: | B1: and so on

	    if first ==  1:
		first = 0; 
		sys.stdout.write('\n        {')
	    else:
		sys.stdout.write(', \n        {')
	
	    form = "ov.html"
	    if index >= 6: form = "ovb.html"
		    

            sys.stdout.write('"ID": "%s", ' % (index + hostid * 12));
	    sys.stdout.write('"host": "%s", ' % (host + '/' + form) );
	    sys.stdout.write('"port": "%d", ' % (index+1));
	    sys.stdout.write('"name": "%s", ' % name);
	    sys.stdout.write('"value": "%s" ' % value);
            sys.stdout.write('}');
      hostid += 1

    sys.stdout.write('\n    ]\n');
    sys.stdout.write('}\n');

def main():

    content_type = 'text/plain; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)

    if not 'ci_file' in globals():
        getdata();
    else:
        file = open(ci_file)
        res = file.read()
        print res

    return 0;

if __name__ == '__main__':
    sys.exit(main())


