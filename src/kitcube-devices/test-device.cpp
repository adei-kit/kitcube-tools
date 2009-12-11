// test-device.cpp
//
// Test DAQ device object implementation for KITCube
// A. Kopmann 4.6.09
//

#include <stdlib.h>

#include "createdevice.h"
#include "daqdevice.h"


int main(int argc, char *argv[]){
	DAQDevice *dev;
	
	printf("KITCube Test DAQ Device Object Implementation\n");
	
	// Parse command line
	
	// Read inifile
	//dev = new Mast();
	dev = (DAQDevice *) createDevice("Norbert");
	dev->setDebugLevel(3);
	dev->readInifile("kitcube.ini");

	//dev->openFile("20090625.chm"); // for reading
	//dev->readHeader(); // Might be included in the open function?!

	//sleep(2);
	
	dev->openFile();
	dev->writeData();
	dev->writeData();
	dev->writeData();

/*
	sleep(2);
	dev->writeData();
*/

    //dev->openFile("20090625.chm");
	//dev->readHeader("20090625.chm");
	//dev->readData("", "20090625.chm");
	
	
	//dev->closeFile();
	
	// Close file, terminate
	delete dev;
	
	
	// Test rsync
	//system("rsync -avz /Users/kopmann/tmp/feshell ./tmp");

}
