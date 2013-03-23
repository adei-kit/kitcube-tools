#! /usr/bin/env python
# Created by Sanshiro Enomoto on 2 February 2011 #


import sys, os, urllib, csv, time, calendar

from adeiconfig import *

# TODO: Instead of doing one query per ADEI loggroup one
#   common query using the virtual server might be an alternative?!
# TODO: The sensors in a group need to follow the order in ADEI definition
#   Order the values?!
# TODO: Timestamp of the data is still confusing !!!
# 

# Alias names for the loggroups
g_ci = 'Data_120_SysCI_txt'
g_mast2 = 'Data_002_T02_DAL'
g_eb1_dar = 'Data_011_EB1_DAR'
g_eb1_dax = 'Data_011_EB1_DAX'

# List of loggroups (using the alias names above)
number_of_loggroups = 3
loggroup_list = number_of_loggroups * [None]
loggroup_list[0] = g_ci
loggroup_list[1] = g_eb1_dar # 10min mean values
loggroup_list[2] = g_eb1_dax # 10min calculated values

# Defintion of groups of sensors
# Format of the alias list is 
#           0,            1,             2,           3,       4,        5 
# Description, scale factor, format string, alarm limit, db_group, db_mask 
#
number_of_items = 8
alias_list = number_of_items * [None]
group_list = list()


# Computer infrastructure #
group_list.append(('Computer Infrastructure', [0,1,2]))
alias_list[0] = ("Temperature server room", 1, "%.2f C", 25, g_ci, 0);
alias_list[1] = ("Free disk space", 1, "%.0f GB", 0, g_ci, 1);
alias_list[2] = ("Free disk space", 1, "%.2f %%", 0, g_ci, 2);

group_list.append(('Energy balance 1', [3,4,5,6,7]))
alias_list[3] = ("Air pressure 0.1m", 1, "%.1f hPa", 0, g_eb1_dar, 12);
alias_list[4] = ("Temp 3m", 1, "%.2f C", 0, g_eb1_dar, 36);
alias_list[5] = ("Rel humidity 3m", 1, "%.1f %%", 0, g_eb1_dar, 52);
alias_list[6] = ("Wind speed 4m", 1, "%.2f m/s", 0, g_eb1_dax, 0);
alias_list[7] = ("Wind direction 4m", 1, "%.0f &deg;", 0, g_eb1_dax, 1);

# Global lists for the ADEI query results
timestamp_list = number_of_loggroups * [0]
name_list = number_of_items * ["???"]
data_list = number_of_items * [("-", 0)]


def getdata(loggroup_id):
    loggroup = loggroup_list[loggroup_id]
    query = '%(server_address)s/services/getdata.php?'\
		    'db_server=%(db_server)s&db_name=%(db_name)s&db_group=%(db_group)s'\
    '&db_mask=%(db_mask)s'\
    '&window=-1'
    #'&window=-1'
    #'&window=1337100301'
    #'&format=csv&window=%(start)d-%(stop)d'

    # Pick the required masks
    db_mask = ''
    res_list = list()
    for target_id in range(number_of_items):
        if alias_list[target_id][4] == loggroup:
            res_list.append(target_id) # save the postion for the result
            if not db_mask:
                db_mask = str(alias_list[target_id][5])
            else:
                db_mask = db_mask + ',' + str(alias_list[target_id][5])


    param = dict()
    param['server_address'] = adei_url
    param['db_server'] = adei_server
    param['db_name'] = adei_database
    param['db_group'] = loggroup
    param['db_mask'] = db_mask
    #param['start'] = int(timestamp) - 30
    #param['start'] = int(timestamp) - 1000
    #param['stop'] = int(timestamp)
    url = query % param

    #print url
    #print 'Number of sensors in loggroup ' + loggroup + ' are ' + str(len(res_list))
    #for id in res_list:
    #    print '  ' + str(id)

    try:
        f = urllib.urlopen(url)
    except:
        sys.stdout.write('{"error": "%s"}\n' % 'unable to connect ADEI server');
        return

    try:
        csvreader = csv.reader(f, skipinitialspace=True)
    except:
        sys.stdout.write('{"error": "%s"}\n' % 'unable to get ADEI data');
        return

    # Copy all data to a results field 
    timestamp = 0;   
    for rowindex, rowdata in enumerate(csvreader):
        for columnindex, data in enumerate(rowdata):
            if rowindex == 0:
                if columnindex > 0:
                    name_list[res_list[columnindex-1]] = data
                continue

            if columnindex == 0:
                datestring = data.partition('.')[0] + ' UTC'
                timestr = time.strptime(datestring, "%d-%b-%y %H:%M:%S %Z")
                timestamp = calendar.timegm(timestr)
            else:
		# data_list[res_list[columnindex-1]] = data
                data_list[res_list[columnindex-1]] = (data,timestamp)

    #for target_id in range(number_of_items):
    #    print 'Sensor ' + str(target_id) + ' ' + name_list[target_id] + ' ' + str(data_list[target_id]) 

    timestamp_list[loggroup_id] = timestamp
    #print "Timestamp(" + str(loggroup_id) + ") = " + str(timestamp)

 

def printjson():
    # TODO: Add the evaluation of the alarm limits for each sensor !!!
    #   Is it possible to automatically create the tables from this defintion?!

    # Pick the latest timestamp
    # Data from group old than a certain date should might be omited?!
    timestamp = 0
    for ts in timestamp_list:
        if ts > timestamp: 
            timestamp = ts
   
    #print data_list; 


    sys.stdout.write('{\n');
    sys.stdout.write('    "timestamp": "%d",\n' % timestamp);

    sys.stdout.write('    "data": [');
    for index in range(number_of_items):
        if alias_list[index]:
            alias = alias_list[index][0]
            if alias_list[index][2]:
                try:
                    value = alias_list[index][2] % (alias_list[index][1] * float(data_list[index][0]))
                except:
                    value = data_list[index][0]
            else:
                value = data_list[index][0]
        else:
            alias = name_list[index]
            value = data_list[index][0]

        if index == 0:
            sys.stdout.write('\n        {')
        else:
            sys.stdout.write(', \n        {')
        sys.stdout.write('"ID": "%s", ' % index)
        sys.stdout.write('"Name": "%s", ' % name_list[index])
        sys.stdout.write('"Alias": "%s", ' % alias)
	sys.stdout.write('"Timestamp": "%s", ' % data_list[index][1])
	sys.stdout.write('"Value": "%s", ' % value)
        sys.stdout.write('"RawValue": "%s", ' % data_list[index][0])
        sys.stdout.write('"LogGroup": "%s", ' % alias_list[index][4])
        sys.stdout.write('"Mask": "%s"' % alias_list[index][5])
        sys.stdout.write('}');
    sys.stdout.write('\n    ],\n');

    sys.stdout.write('    "group": [');
    for group_index, group in enumerate(group_list):
	if group_index == 0:
            sys.stdout.write('\n        {\n');
        else:
            sys.stdout.write(',\n        {\n');
        sys.stdout.write('            "name": "%s",\n' % group[0]);
	# TODO: Return the timestamp of the group used?!
	#       Get the first item in a display group
	#       Get the name of the loggroup
	#       Find the timestamp of the loggroup
        sys.stdout.write('            "timestamp": "%d",\n' % timestamp_list[group_index]);
        sys.stdout.write('            "member": [')
        for member_index, member in enumerate(group[1]):
            if member_index == 0:
                sys.stdout.write('"%d"' % member)
            else:
                sys.stdout.write(', "%d"' % member)
        sys.stdout.write(']\n')
        sys.stdout.write('        }');
    sys.stdout.write('\n    ], \n');
    sys.stdout.write('    "source": "%s"\n ' % adei_url);
    sys.stdout.write('}\n');


def main():
    content_type = 'text/plain; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)
    #getdata(time.time())

    for loggroup_id in range(number_of_loggroups):
        getdata(loggroup_id)     

    printjson()

    return 0;


if __name__ == '__main__':
    sys.exit(main())
