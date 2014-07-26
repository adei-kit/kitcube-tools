/***************************************************************************
    simplesocket.h  -  description

    begin                : Tue Oct 24 2000
    copyright            : (C) 2000 by A Kopmann
    email                : kopmann@hpe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef SIMPLESOCKET_H
#define SIMPLESOCKET_H

#include <cstdio>
#include <stdexcept>
#include <stdint.h>

#include <sys/time.h>
#include <sys/types.h>   // struct timeval


#define SIMPLESOCKET_TIMEOUT 2000 // Default timeout [ms]


/*
#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
//#include <winsock2.h>
#endif
*/

/** Implements a simple socket for use
  * with any server.
  *
  * Every instance of the class will establish
  * a connection with a server. By default the connection
  * will use a timeout of SIMPLESOCKET_TIMEOUT milliseconds.
  * The timeout can be changed when using the socket connection
  * with setTimeout. A value of zero for the timeout will
  * turn off the timeout and makes every access blocking.
  *
  * Changes:
  * @li Added timeout (8-2-2006 ak)
  *
  * @ingroup akutil_group
  */

class SimpleSocket {
public: 
  /** Create connection. There will be no timeout.
    * To change the timeout after the connection is established use
    * setTimeout function.
    * @param server hostname of the server application
    * @param port port number of the server application
    */
  SimpleSocket(const char *server, unsigned short port);

  /** Create connection with timeout
    * To change the timeout after the connection is established use
    * setTimeout function.
    * @param server hostname of the server application
    * @param port port number of the server application
    * @param timeout given in miliseconds
    */
  SimpleSocket(const char *server, unsigned short port, unsigned long timeout);

  /** Create connection with timeout
    * To change the timeout after the connection is established use
    * setTimeout function.
    * @param server hostname of the server application
    * @param port port number of the server application
    * @param timeout 
    */
  SimpleSocket(const char *server, unsigned short port, struct timeval timeout);

  /**  Destructor of class SimpleSocket. */
  ~SimpleSocket();

  /** Set timeout in miliseconds */
  void setTimeout(unsigned long timeout);

  /** Set timeout */
  void setTimeout(struct timeval timeout);
  
  int readMsg(char *msg, int max);
  int writeMsg(char *msg, int n=0);

  int readData(unsigned long *data, int max);
  int writeData(unsigned long *data, int n);

  int readData(unsigned int *data, int max);
  int writeData(unsigned int *data, int n);
  
  char *getHostName();
  char *getPeerName();

  /** Read a packet of raw data.
    * Each packet consist of header, raw data and trailer.
    * Header and trailer have the format 0xfff | length of raw data.
    */
  int readPacket(unsigned long *data, int max);

  /** Write a packet of raw data.
    * Each packet consist of header, raw data and trailer.
    * Header and trailer have the format 0xfff | length of raw data.
    */
  int writePacket(unsigned long *data, int n);

  
  /** Make a call to the connected server.
    * The command implements a simple protokoll to exchange
    * command id, arguments and results. The command
    * also considers possible error messages.
    *
    * @param cmdid Command id; has to be implemented by higher
    *    level protokolls
    * @param args List of arguments
    * @param argc Number of arguments in lists args
    * @param data List of data 
    * @param ndata Number of data
    * @param ackn List of acknowledge parameter
    * @param len Number acknowledge parameter
    *
    * @returns error status: 0 no error, 1+ error while exectution
    *    of the command, -1 socket communication closed by foreign
    *    host.
    */
  int remoteCall(short cmdid,unsigned long *args, short argc=0,
                             unsigned long *data=NULL, short ndata=0,
                             unsigned long *ackn=NULL, short len=0);

  int remoteCall(short cmdid,uint32_t *args=NULL, short argc=0,
		             uint32_t *data=NULL, short ndata=0,
		             uint32_t *ackn=NULL, short len=0);
	

  /** Wait for reading socket.
    *
    * @returns -1 error, 0 timeout, 1 ok
    */
  int waitForReading();

  /** Wait for reading socket.
    *
    * @returns -1 error, 0 timeout, 1 ok
    */
  int waitForWriting();
                                                                                                                              
  /** Wait for a certain time
    *
    * @param timeout Time to wait in seconds
    */
  int wait(int timeout);

  /** Get the file desciptor */
  int getFD();

  
private:
  /** Connect to server */
  void connectServer(const char *server, unsigned short port);

  int sock;

  /** Flag to indicate nonblocking sockets. */
  bool nonblock;

  /** Timeout for connection, reading and writing. */
  struct timeval timeout;  
};

#endif
