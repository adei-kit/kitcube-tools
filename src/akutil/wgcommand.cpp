/***************************************************************************
    WGCommand.cpp  -  description

    begin                : Mon Sep 04 2006
    copyright            : (C) 2006 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/

#include <stdio.h>

#include <cstring>
#include <cerrno>
#include <cmath>
#include <stdexcept>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "wgcommand.h"

using namespace std;


WGCommand::WGCommand(){
  this->debug = 0;
  this->fd = 0;
  this->gain = 0.1; // -20dB
}


WGCommand::~WGCommand(){
}


void WGCommand::openDevice(const char *device){

  // Do not open if already opened before
  if (fd > 0){
    //throw std::invalid_argument("Device already opended");
    return;
  }  

  // Keep the device name
  this->device = device;


  fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
  //printf("Open: fd=%d\n",fd);

  if (fd <=0) {
    throw std::invalid_argument(strerror(errno));
  }
    
  // save current serial port settings
  tcgetattr(fd,&(this->bufferedTio));

  
  if (this->debug > 1) {
    printf("c_cflag = %08lx\n", this->bufferedTio.c_cflag);

    printf("B9600   = %08x\n", B9600);
    printf("B57600  = %08x\n", B57600);
    printf("CS8     = %08x\n", CS8);
    printf("CLOCAL  = %08x\n", CLOCAL);
    printf("PARODD  = %08x\n", PARODD);
    printf("CRTSCTS = %08x\n", CRTSCTS);
    printf("CREAD   = %08x\n", CREAD);
    printf("\n");

    printf("c_iflag = %08lx\n", this->bufferedTio.c_iflag);
    printf("IGNPAR  = %08x\n", IGNPAR);
    printf("IGNBRK  = %08x\n", IGNBRK);
    printf("\n");

    printf("c_oflag = %08lx\n", this->bufferedTio.c_oflag);
    printf("c_lflag = %08lx\n", this->bufferedTio.c_lflag);
  }
  
  // clear struct for new port settings
  bzero(&(this->wgTio), sizeof(this->wgTio));


  //    BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
  //    CRTSCTS : output hardware flow control (only used if the cable has
  //              all necessary lines. See sect. 7 of Serial-HOWTO)
  //    CS8     : 8n1 (8bit,no parity,1 stopbit)
  //    CLOCAL  : local connection, no modem contol
  //    CREAD   : enable receiving characters
  wgTio.c_cflag = B57600 | CS8 | CLOCAL | CREAD | PARODD ;


  //    IGNPAR  : ignore bytes with parity errors
  //    ICRNL   : map CR to NL (otherwise a CR input on the other computer
  //              will not terminate input)
  //    otherwise make device raw (no other input processing)
  wgTio.c_iflag = IGNBRK;


  // Raw output.
  wgTio.c_oflag = 0;


  //    ICANON  : enable canonical input
  //    disable all echo functionality, and don't send signals to calling program
  wgTio.c_lflag = 0;


  // now clean the modem line and activate the settings for the port
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&(this->wgTio));


}


void WGCommand::closeDevice(){

  if (fd == 0){
    throw std::invalid_argument("Device not open");    
  }  

  // restore the old port settings
  tcsetattr(fd,TCSANOW,&(this->bufferedTio));
  close(fd);

  fd = 0;
  
}  


void WGCommand::sendCmd(unsigned char *cmd, unsigned char *ack){
  int res;
  int i;
  
  if (fd == 0){
    throw std::invalid_argument("Device not open");
  }


  // Calculate checksum
  cmd[0] = 'X';
  cmd[6] = cmd[1] ^ cmd[2] ^ cmd[3] ^ cmd[4] ^ cmd[5];

  if (debug > 0){
    printf("Command:     ");
    for (i=0;i<7;i++)
      printf(" %02x", (unsigned char) cmd[i]);
    printf("   ");  
    for (i=0;i<7;i++)
      if (isprint(cmd[i]))
        printf("%c", (unsigned char) cmd[i]);
      else
        printf(".");  
    printf("\n");
  }


  res = write (fd, cmd, 7);
  //printf("res = %d\n", res);
  if (res < 0){
    throw std::invalid_argument(strerror(errno));
  }

  
  // Wait for the answer
  // TODO: Implement timeout loop?!
  //       or select waiting for the interface
  i=0;
  res = 0;
  while((res == 0) && (i<3)){  
    res = read (fd, ack, 7);
    //printf("res = %d msg = %s\n", res, strerror(errno));
    if (res < 0){
      throw std::invalid_argument(strerror(errno));
    }

    if (res == 0) usleep(100000);

    i = i +1;
  }

  if (res == 0) {
    throw std::invalid_argument("Timeout");
  }  
      
  // TODO: Analyse checksum..
  
  
  if (debug > 0){
    printf("Acknowledge: ");
    for (i=0;i<res;i++)
      printf(" %02x", (unsigned char) ack[i]);
    printf("   ");
    for (i=0;i<res;i++)
      if (isprint(ack[i]))
        printf("%c", (unsigned char) ack[i]);
      else
        printf(".");
    printf("\n");
  }
  
}


int WGCommand::getParameter(char cmdId){
  unsigned char cmd[7];
  unsigned char ack[7];
  int par;

  cmd[1] = cmdId;

  sendCmd(cmd, ack);

  if ((ack[2] == 0) &&
       (ack[3]== 0) &&
       (ack[4]== 0) &&
       (ack[5] == (unsigned char) 0xff ))
    throw std::invalid_argument("Command failed");

  // Decode amplitude
  par = ((int) 256) * ack[2] + ack[3];

  return(par);  
}  


void WGCommand::setParameter(char cmdId, int par1, int par2){
  unsigned char cmd[7];
  unsigned char ack[7];

  cmd[1] = cmdId;

  cmd[2] = par1/256;
  cmd[3] = par1%256;
  cmd[4] = par2;
  cmd[5] = 0x80; // Switch display

  sendCmd(cmd, ack);

  if ((ack[2] == 0) &&
       (ack[3]== 0) &&
       (ack[4]== 0) &&
       (ack[5] == (unsigned char) 0xff ))
    throw std::invalid_argument("Command failed");  
}  



void WGCommand::setDebug(int debug){
  this->debug = debug;
}
  

void WGCommand::getType(int *model, int *nChannel, int *morph){
  unsigned char cmd[7];
  unsigned char ack[7];

  cmd[1] = '&';
  cmd[5] = 0;
  
  sendCmd(cmd, ack);

  if (model > 0){
    *model = ((int) 256) * ack[2] + ack[3];
  }  
  if (nChannel > 0)
    *nChannel = ack[4];
  if (morph) {
    if (ack[5] == 'm')
      *morph = 1;
    else
      *morph = 0;  
  }
}  


int WGCommand::getVersion(char *version){
  unsigned char cmd[7];
  unsigned char ack[7];
  int iVersion;
  
  cmd[1] = 'e';

  sendCmd(cmd, ack);

  iVersion = 10 * (ack[3] - '0') + (ack[2] - '0');
  
  if (version > 0)
    strncpy(version, (char *) (ack+2), 4);
  version[4] = 0;  

  return(iVersion);    
}  


void  WGCommand::setDefaults(){
  unsigned char cmd[7];
  unsigned char ack[7];

  cmd[1] = 'S';
  cmd[2] = 'D';
  cmd[3] = 'e';
  cmd[4] = 'f';
  cmd[5] = '0';

  sendCmd(cmd, ack);
  
}


void WGCommand::setMode(int mode){
  unsigned char cmd[7];
  unsigned char ack[7];

  cmd[1] = 'R';
  if (mode == 0)
    cmd[2] = '0';
  else  
    cmd[2] = '1';
  
  sendCmd(cmd, ack);
  
}  


int WGCommand::getMode(){
  unsigned char cmd[7];
  unsigned char ack[7];

  cmd[1] = 'r';

  sendCmd(cmd, ack);

  return(ack[2]-'0');    
}  


void WGCommand::setAmplitude(double ampl){
  int iAmpl; // mV

  iAmpl = (int) round(ampl / gain);
  setParameter('A', iAmpl);       
    
}  


double WGCommand::getAmplitude(){
  int ampl;

  ampl = getParameter('a'); 
  return((double) round(ampl * gain));
}  

  
void WGCommand::setFrequency(double frequency){
  int mantissa; // 1000...9999  == 1...10Hz
  int exponent; // -99 ...99

  exponent = (int) log10(frequency);
  mantissa = (int) round(frequency / exp10(exponent-3));

  setParameter('F', mantissa, exponent);  

}  


double WGCommand::getFrequency(){
  unsigned char cmd[7];
  unsigned char ack[7];
  char *signedAck;
  double freq;
  int mantissa; // 1000...9999  == 1...10Hz
  int exponent; // -99 ...99

  // Calculate mantisse and exponent
  // freqency = (arg1 / 1000) * 10** arg2
  //

  cmd[1] = 'f';

  sendCmd(cmd, ack);
 
  if ((ack[2] == 0) &&
       (ack[3]== 0) &&
       (ack[4]== 0) &&
       (ack[5] == (unsigned char) 0xff ))
    throw std::invalid_argument("Command 'f' failed");

  // Decode frequency
  mantissa = ((int) 256) * ack[2] + ack[3];
  signedAck = (char *) ack;
  exponent = signedAck[4];

  if (debug > 0){
    printf("freq = (%d / 1000) * exp10 ( %d )\n",
                  mantissa, exponent);
  }
  
  freq = ((double) mantissa) / 1000 * exp10(exponent); 

  //printf("mantissa %d   exp %d \n", mantissa, exponent);

  return(freq);
      
}  


void WGCommand::setOffset(double offset){
  int iOffset; // mV

  iOffset = (int) round(offset / gain);
  setParameter('O', iOffset);

}


double WGCommand::getOffset(){
  int offset;

  offset = getParameter('o');
  return((double) round(offset * gain));
}


void WGCommand::setOffsetMode(int /* mode */){
}  


int WGCommand::getOffsetMode(){
  return(0);
}  


void WGCommand::setFilterMode(int /* mode */){
}  


int WGCommand::getFilterMode(){
  return(0);
}  


void WGCommand::setDutyCycle(int dutycycle){
  // Use only the high byte
  setParameter('D', dutycycle * 256);
}  


int WGCommand::getDutyCycle(){
  int dutycycle;

  dutycycle = getParameter('d') / 256;
  return(dutycycle);
}  


void WGCommand::setSyncPosition(int /* pos */){}

int WGCommand::getSyncPosition(){
  return(0);
}

void WGCommand::setModulation(int /* mode */){}

int WGCommand::getModulation(){
  return(0);
}

void WGCommand::setTriggerMode(){}

int WGCommand::getTriggerMode(){
  return(0);
}

void WGCommand::setClock(int /* source */){}

int WGCommand::getClock(){
  return(0);
}

void WGCommand::setWaveForm(int /* mode */){}

int WGCommand::getWaveForm(){
  return(0);
}

void WGCommand::setGain(int gainId){
  // Use only high byte
  setParameter('Q', gainId * 256);

  // Store value for use with the other functions?!
  switch (gainId){
    case 0:
      this->gain = 0;
      break;
    case 1:
      this->gain = 1;
      break;
    case 2:
      this->gain = 0.1;
      break;
  }
}

int WGCommand::getGain(){
  int gainId;
  gainId = getParameter('q');

  // Store value for use with the other functions?!
  switch (gainId){
    case 0:
      this->gain = 0;
      break;
    case 1:
      this->gain = 1;
      break;
    case 2:
      this->gain = 0.1;
      break;
  }
  return(gainId);
}  



double WGCommand::exp10(double exponent){
  return exp( exponent * log(10.) );
}  



