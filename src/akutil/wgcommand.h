/***************************************************************************
    WGCommand.h  -  description

    begin                : Mon Sep 04 2006
    copyright            : (C) 2006 by Andreas Kopmann
    email                : kopmann@ipe.fzk.de
 ***************************************************************************/


#ifndef WGCOMMAND_H
#define WGCOMMAND_H


#include <termios.h>
#include <string>
#include <cstdio>


/** Implementation of the WG Command interface defined by
  * Mair & Rohner OEG (M&R Systems, www.mrsys.at) for their
  * waveform generators.
  *
  * Tested with waveform generator WG-1220.
  *
  * @todo Implement timeout waiting for the device
  * @todo Implement the missing commands
  *
  * @ingroup akutil_group
  */
class WGCommand {
public: 
  WGCommand();

  ~WGCommand();

  /** Open device and set correct parameter
    * to the serial device */
  void openDevice(const char *device = "/dev/ttyS0");

  /** Close device. Restore the parameters
    * of the device */
  void closeDevice();

  /** Send one command to the
    * device */
  void sendCmd(unsigned char *cmd, unsigned char *ack);

  /** Generic function to get a two byte parameter */
  int getParameter(char cmdId);

  /** Generic function to set a two byte parameter */
  void setParameter(char cmdId, int par1=0, int par2=0);
  
  /** Set debug level (0 = no debug) */
  void setDebug(int debug);

  /** Get device type */
  void getType(int *model, int *nChannel = 0, int *morph = 0);

  /** Get firmware version
    * The version V \<major\>.\<minor\> is coded as
    * \<major\> * 10 + \<minor\>*/
  int getVersion(char *version);

  
  /** Set to factory defaults */
  void setDefaults();

  /** Select mode of operation (0 = remode/ 1 = local) */
  void setMode(int mode);

  /** Request mode (0 = remote/ 1= local) */
  int getMode();

  /** Set amplitude in mV */
  void setAmplitude(double amplitude);

  /** Get amplitude in mV */
  double getAmplitude();

  /** Set frequency in Hz */
  void setFrequency(double frequency);

  /** Get freqency in Hz */
  double getFrequency();

  /** Set offset in mVpp */
  void setOffset(double offset);

  /** Get offset in mVpp */
  double getOffset();

  /** Set offset mode (signal positive, signal negtive,
      signal negative and positive)*/
  void setOffsetMode(int mode);

  /** Get offset mode */
  int getOffsetMode();

  /** Set filter mode (filter enabled, filter disabled) */
  void setFilterMode(int mode);
  
  /** Get filter mode */
  int getFilterMode();

  /** Set duty cycle in percent */
  void setDutyCycle(int dutycycle);

  /** Get duty cycle */
  int getDutyCycle();

  /** Set sync position */
  void setSyncPosition(int pos);

  /** Get sync Position */
  int getSyncPosition();

  /** Set modulation type (off, FM, AM) */
  void setModulation(int mode);

  /** Get modulation type */
  int getModulation();

  /** Set trigger mode (continuous, single shot, external trigger) */
  void setTriggerMode();
  
  /** Get trigger mode */
  int getTriggerMode();
  
  /** Set clock source */
  void setClock(int source);
  
  /** Get clock source */
  int getClock();

  /** Set wavform (sinus, square, triangular, DC, noise) */
  void setWaveForm(int mode);

  /** Get waveform */
  int getWaveForm();      

  /** Set gain (off, 0dB, -20dB) */
  void setGain(int gain);

  /** Get gain */
  int getGain();  

  
  // TODO: Add programming modes
  //


  /** Calculate 10 power exp */
  double exp10(double exp);
  
                                                                                                                                       
private:

  /** Debug level */
  int debug;

  /** Device name */
  std::string device;
  
  /** File descriptor for open device */
  int fd;

  /** Settings of serial device for WG devices */
  struct termios wgTio;

  /** Buffered terminal settings */
  struct termios bufferedTio;

  /** Buffer with the last acknowlede */
  char lastAck[7];

  /** Gain factor (0dB -> 1, -20dB -> 0.1)*/
  double gain;
  
};

#endif
