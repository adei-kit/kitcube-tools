#! /usr/bin/env python
# Created by Sven Werchner

import sys, os, urllib, csv, time, calendar, math,cgi
import matplotlib
import matplotlib.pyplot as plt
from adeiconfig import *


g_rs = rs_g_rs
flight_times = []
flight_heights = []
flight_pressure = []
flight_temperatures = []
flight_moisture = []

#Determines the starting time of all Radiosonde experiments
def getAllRS():
	query = '%(server_address)s/services/getdata.php?'\
			'db_server=%(db_server)s&db_name=%(db_name)s&db_group=%(db_group)s'\
			'&db_mask=%(db_mask)s'\
			'&window=0'\
			'&resample=60' #data every minute -> no avg. just the raw values in minutely intervall
	
	param = dict()
	param['server_address'] = adei_url
	param['db_server'] = adei_server
	param['db_name'] = adei_database
	param['db_group'] = g_rs
	param['db_mask'] = "0,1,2,7"
	
	url = query % param
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

	temp_flight_times = []
	temp_flight_heights = []
	temptime = 0
	lastheight = 0.0
	temppress = 0.0
	tempheat = 0.0
	tempmois = 0.0
	temp_flight_pressure = []
	temp_flight_temperatures = []
	temp_flight_moisture = []

	for rowindex, rowdata in enumerate(csvreader):
		for columnindex, data in enumerate(rowdata):
			if rowindex==0:
				continue
			if columnindex == 1 and not data == "":
				temppress = float(data)
				continue
			if columnindex == 2 and not data == "":
				tempheat = float(data)
				continue
			if columnindex == 3 and not data == "":
				tempmois = float(data)
				continue
			if columnindex == 4 and data=="0":
				continue
			if columnindex == 0:
				datestring = data.partition('.')[0] + ' UTC'
				timestr = time.strptime(datestring,"%d-%b-%y %H:%M:%S %Z")
				temptime = calendar.timegm(timestr)
			if columnindex == 4:
				if not data =="":
					temp_flight_times.append(temptime)
					temp_flight_heights.append(float(data))
					temp_flight_pressure.append(temppress)
					temp_flight_temperatures.append(tempheat)
					temp_flight_moisture.append(tempmois)
					lastheight = float(data)
				else:
					temp_flight_times.append(temptime)
					temp_flight_heights.append(lastheight)
					temp_flight_pressure.append(temppress)
					temp_flight_temperatures.append(tempheat)
					temp_flight_moisture.append(tempmois)
	

	flights = []
	heights	= []
	temperatures = []
	pressure = []
	moisture = []
	newflight = []
	newheight = []
	newtemperatures = []
	newpressure = []
	newmoisture = []
	lastflight = 0
	iteration = 0
	

	for flight in temp_flight_times:
		if flight - lastflight > 180:
			if not lastflight == 0:
				flights.append(newflight)
				heights.append(newheight)
				temperatures.append(newtemperatures)
				pressure.append(newpressure)
				moisture.append(newmoisture)
			newflight = []
			newheight = []
			newtemperatures = []
			newpressure = []
			newmoisture = []
		newflight.append(flight)
		newheight.append(temp_flight_heights[iteration])
		newtemperatures.append(temp_flight_temperatures[iteration])
		newpressure.append(temp_flight_pressure[iteration])
		newmoisture.append(temp_flight_moisture[iteration])
		lastflight = flight
		iteration = iteration + 1
	


	flights.append(newflight)
	heights.append(newheight)
	temperatures.append(newtemperatures)
	pressure.append(newpressure)
	moisture.append(newmoisture)

	global flight_times
	flight_times = flights
	global flight_heights
	flight_heights = heights
	global flight_temperatures
	flight_temperatures = temperatures
	global flight_pressure
	flight_pressure = pressure
	global flight_moisture
	flight_moisture = moisture

	return

def printJSonSummary():
	
	print "{"
	sys.stdout.write('\t"tableData": [\n')


	iteration = 0
	for flight in flight_times:
		print '\t  {'
		sys.stdout.write('\t\t"ID":"%d",\n' % iteration)
		starttimestring = time.gmtime(flight[0])
		sys.stdout.write('\t\t"Starttime":"%s",\n' % time.strftime("%d-%b-%y %H:%M:%S",starttimestring))
		sys.stdout.write('\t\t"maxheight":"%d m",\n' % max(flight_heights[iteration]))
		durationtimestring = time.gmtime(flight[-1] - flight[0])
		sys.stdout.write('\t\t"Duration":"%s",\n' % time.strftime("%H:%M:%S",durationtimestring))
		print '\t  },\n'
		iteration = iteration + 1
	sys.stdout.write('\t],\n}')
	return 0

def plotRS(idlist,phyQuan):
	fig = plt.figure(figsize=(6.5,7.2))
	plt.grid()
	axis = fig.add_subplot(111)
	for id in idlist:
		if phyQuan == "T":
			axis.plot(flight_temperatures[id],flight_heights[id],label=str(id))
			plt.xlabel("                                 Temperature [C]",ha='left')
		if phyQuan == "P":
			axis.plot(flight_pressure[id],flight_heights[id],label=str(id))
			plt.xlabel("                                 Pressure [mbar]",ha='left')
		if phyQuan == "M":
			axis.plot(flight_moisture[id],flight_heights[id],label=str(id))
			plt.xlabel("                                     Moisture [%]",ha='left')
	axis.set_ylabel("                                                                                                     Height [m]")
	axis.legend(loc=0)
	plt.savefig(sys.stdout)


def main():
	form = cgi.FieldStorage()
	mode = form.getfirst('mode')
	idlist = []
	idliststring = form.getfirst('id')
	phyQuan = form.getfirst('phyQuan')
	getAllRS()
	
	if mode=="summary":
		content_type = 'text/plain; charset="UTF-8"'
		sys.stdout.write('Content-type: %s\n\n' % content_type)
		printJSonSummary()
		return 0;


        while not idliststring.find(",") == -1:
                idlist.append(int(idliststring.partition(",")[0]))
                idliststring = idliststring.partition(",")[2]
	if not idliststring=='-':
		idlist.append(int(idliststring))
		idlist.sort()

	if mode=="plot":
		content_type = 'image/png; charset="UTF-8"'
		sys.stdout.write('Content-type: %s\n\n' % content_type)
		if len(idlist)>0:
			plotRS(idlist,phyQuan)
		else:
			fig=plt.figure(figsize=(6.5,7.2))
			plt.grid()
			plt.xlabel("No Field Selected")
			axs = fig.add_subplot(111)
			axs.plot([],[],label="Please select")
			axs.plot([],[],label="the fields")
			axs.plot([],[],label="that are to")
			axs.plot([],[],label="be plotted")
			axs.legend(loc=0)
			plt.savefig(sys.stdout)
		return 0;

	return 0


if __name__ == '__main__':
	sys.exit(main())
