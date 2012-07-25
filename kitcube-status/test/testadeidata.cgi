#! /Library/Frameworks/Python.framework/Versions/2.7/bin/python

#! /usr/bin/env python
# Created by Sanshiro Enomoto on 2 February 2011 #


import sys, os, urllib, csv, time, calendar


version = sys.version
path = sys.path


#server_address = 'http://clean.phys.washington.edu'
#server_address = 'http://192.168.110.67'
#server_address = 'http://katrin.kit.edu/adei-detector'
server_address = 'http://localhost:8024/adei'

db_server = 'kitcube'
db_name = 'hatzenbuehl'
db_group = 'Data_120_SysCI_txt'


number_of_items = 3
alias_list = number_of_items * [None]
group_list = list()

# Computer infrastructure #
group_list.append(('Computer Infrastructure', [0,1,2]))
alias_list[0] = ("Temperature server room", 1, "%.2f C");
alias_list[1] = ("Free disk space", 1, "%.0f GB");
alias_list[2] = ("Free disk space", 1, "%.0f %%");


def getdata():
    query = '%(server_address)s/services/getdata.php?'\
    'db_server=%(db_server)s&db_name=%(db_name)s&db_group=%(db_group)s'\
    '&db_mask=%(db_mask)s'\
    '&window=-1'
    #'&window=-1'
    #'&window=1337100301'
    #'&format=csv&window=%(start)d-%(stop)d'

    db_mask = ''
    for target_id in range(number_of_items):
        if not db_mask:
            db_mask = str(target_id)
        else:
            db_mask = db_mask + ',' + str(target_id)

    param = dict()
    param['server_address'] = server_address
    param['db_server'] = db_server
    param['db_name'] = db_name
    param['db_group'] = db_group
    param['db_mask'] = db_mask
    #param['start'] = int(timestamp) - 30
    #param['start'] = int(timestamp) - 1000
    #param['stop'] = int(timestamp)
    url = query % param

    #print url

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


    timestamp = 0
    name_list = number_of_items * ["???"]
    data_list = number_of_items * [""]

    for rowindex, rowdata in enumerate(csvreader):
        for columnindex, data in enumerate(rowdata):
            if rowindex == 0:
                if columnindex > 0:
                    name_list[columnindex-1] = data
                continue
                
            if columnindex == 0:
                datestring = data.partition('.')[0] + ' UTC'
                timestr = time.strptime(datestring, "%d-%b-%y %H:%M:%S %Z")
                timestamp = calendar.timegm(timestr)
            else:
                data_list[columnindex-1] = data


    sys.stdout.write('{\n');
    sys.stdout.write('    "timestamp": "%d",\n' % timestamp);

    sys.stdout.write('    "data": [');
    for index in range(number_of_items):
        if alias_list[index]:
            alias = alias_list[index][0]
            if alias_list[index][2]:
                try:
                    value = alias_list[index][2] % (alias_list[index][1] * float(data_list[index]))
                except:
                    value = data_list[index]
            else:
                value = data_list[index]
        else:
            alias = name_list[index]
            value = data_list[index]

        if index == 0:
            sys.stdout.write('\n        {')
        else:
            sys.stdout.write(', \n        {')
        sys.stdout.write('"ID": "%s", ' % index)
        sys.stdout.write('"Name": "%s", ' % name_list[index])
        sys.stdout.write('"Alias": "%s", ' % alias)
        sys.stdout.write('"Value": "%s", ' % value)
        sys.stdout.write('"RawValue": "%s"' % data_list[index])
        sys.stdout.write('}');
    sys.stdout.write('\n    ],\n');

    sys.stdout.write('    "group": [');
    for group_index, group in enumerate(group_list):
        if group_index == 0:
            sys.stdout.write('\n        {\n');
        else:
            sys.stdout.write(',\n        {\n');
        sys.stdout.write('            "name": "%s",\n' % group[0]);
        sys.stdout.write('            "member": [')
        for member_index, member in enumerate(group[1]):
            if member_index == 0:
                sys.stdout.write('"%d"' % member)
            else:
                sys.stdout.write(', "%d"' % member)
        sys.stdout.write(']\n')
        sys.stdout.write('        }');
    sys.stdout.write('\n    ]\n');

    sys.stdout.write('}\n');


def main():
    content_type = 'text/plain; charset="UTF-8"'
    sys.stdout.write('Content-type: %s\n\n' % content_type)
    #getdata(time.time())
    getdata();     

    sys.stdout.write ('<h1>%s</h1>' % version )                                                                 
    sys.stdout.write ('<h1>%s</h1>' % path )

    return 0;


if __name__ == '__main__':
    sys.exit(main())
