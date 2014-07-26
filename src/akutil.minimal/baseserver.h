/***************************************************************************
    baseserver.h  -  description

    begin                : Fri Sep 24 2004
    copyright            : (C) 2004 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef BASESERVER_H
#define BASESERVER_H

#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <stdexcept>
#include <sys/select.h>
#include <sys/time.h>


#ifdef __GNUC__
# include <signal.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
# define SOCKET int
#endif // __GNUC__

#ifdef __WIN32__ // Windows
#ifndef FD_SETSIZE
//#define FD_SETSIZE 512
// For WinXP a much larger number of sockets is required!!
// 4096 is just a guess - the first socket is about 1960...
// 2004-01-29, ak
#define FD_SETSIZE 4096
#endif
#include <winsock.h>
//#include <winsock2.h> // ???
#endif // of windows defines
 // of windows defines


/** Implements a simple IP server.
  *
  * The server waits for connections.
  * The communication with
  * the client is defined in the
  * protected function read_from_client.
  * Overload this function for a specific application.
  *
  * The server allows to communicate with
  * clients that use different bytes order. The list
  * change_byteorder_fd_set contains all connections
  * that do not operate with the same byte order.
  * The server always tries to communicate with it's
  * own native byte order. The protocoll does not
  * exchange all data in network byte order.
  * (Compile flag USE_DYN_BYTEORER). The disabled compile
  * flag USE_BYTEORDER transvers all variables in
  * network byte order.
  *
  *
  * @todo Use a independant threads for all
  *   connections. If one connection hangs, the other
  *   won't be affected?!
  * @todo Improve the Server managment. Loging,
  *   Shutdown via IP. For some other purpose a
  *   New class similar to Pbus/PbusProxy couble may
  *   be useful for maintenance and run control.
  *   Eg. shutdown + restart of the Server, Control
  *   of the IRdispatcher!!!
  *
  *
  * Changes:
  * @li 4.11.02 ak: Add stdin to the select functions.
  *    The server is now responding to socket and keyboard
  *    input! Improved output of the server - format similar
  *    to that of netstat --ip.
  * @li 31.10.02 ak: Closing all connected sockets before
  *    shutdown. This prevents hanging connections after
  *    terminating the server.
  * @li 18.3.01 ak: Protected variable for the
  *    output stream introduced - stop this nasty
  *    messages in the terminal!!!
  *
  *
  * @ingroup akutil_group
  */

class BaseServer {
public:
  BaseServer(unsigned short port);
  //BaseServer(unsigned short port, SimpleServer *s);

  virtual ~BaseServer();

  void setDebugLevel(int debug);
  /** Enable (default) or disable the connection logging. */
  void setConnectionLogging(bool logging=true)
   { connLogging = logging; }
	
  int readMsg(int client, char *msg, int n);

  int writeMsg(int client, char *msg, int n=0);


  int readData(int client, unsigned long *data, int n);

  int writeData(int client, unsigned long *data, int n);

	
  int readData(int client, unsigned int *data, int n);
	
  int writeData(int client, unsigned int *data, int n);
	
  /** Read a packet of raw data.
    * Each packet consist of header, raw data and trailer.
    * Header and trailer have the format 0xfff | length of raw data.
    */
  int readPacket(int client, unsigned long *data, int max);

  /** Write a packet of raw data.
    * Each packet consist of header, raw data and trailer.
    * Header and trailer have the format 0xfff | length of raw data.
    */
  int writePacket(int client, unsigned long *data, int n);

  
  void monitor();

  // test features
  void display();
  virtual void show();

  /** Create the server socket */
  void init(FILE *fout = stdout);

  /** Start server.
    * Additional file descriptors can be added to the
    * select command.
	*
	* @todo Count the sampling times to avoid data losses 
	*    due to server load
    */
  void init_server(int fd1=-1, int fd2=-1);

  void kill_server();

  /** Display all active sockets that are served.
    * The display format is somewhat similar to the output of the
    * shell command "netstat --ip". IP number and port number of
    * both sides of the connection are show. */
  void displayActiveSockets();

  /** Display all active sockets, but use not the standard output stream
    * defined in the initialization.
    */
  void displayActiveSockets(FILE *fout);
  
  /** Allows to set a common reference time, if more than 
    * one sampling tasks need to be synchronized */
  void setTRef(struct timeval *time);
  
  /** Enable period sampling */
  void enableTimeout(struct timeval *timeout=0);

  void disableTimeout();
  
  
protected:
  /** Debug level, where 0 means less output and 1 or higher will produce more information */
  int debug;	
	
  /** Set the port of the server. This will only work before calling init(). */
  void setPort(unsigned short port);
	
  /** Do some action after the timeout and before the next select follows */
  virtual int handle_timeout();

  /** Calculate the time that is left until the next sampling time */
  void getNextTimeout(int *iSample, struct timeval *timeout);

  /** Display a message byte by byte in hex format */
  void displayMsg(unsigned char *msg, int len);

  /** Defines the interface form the communication
    * with the proxy process.
    *
    * The protocol is define as follows:
    * @li command (4bytes)
    * @li len (4bytes)
    * @li data (len * 4bytes)
    */
  virtual int read_from_client(int filedes);


  /** Server the keyboard in interactive mode */
  virtual int read_from_keyboard();

  virtual void displayVersion();

  /** File descriptor for the debug output and messages */
  FILE *fout;

  /** Shutdown flag. Can be set by any function and will terminate
    * the server after the next loop. */
  bool shutdown;

  /** Set of the active sockets to be scanned by select */
  fd_set active_fd_set; 

  /** Set of all sockets where the byte order has to be
    * switched */
  fd_set change_byteorder_fd_set;  

  /** Change byte order of a short variable */
  inline void endian_swap(unsigned short& x) {
    x = (x>>8) | (x<<8);
  }

  /** Change byte order of a uint32_t variable */
  inline void endian_swap(uint32_t& x) {
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
         (x<<24);
  }

  /** Change byte order of a long variable */
  inline void endian_swap(unsigned long& x) {
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
         (x<<24);
  }

  /** Change byte order for 64bit data types */
  inline void endian_swap(unsigned long long& x) {
    x = (x>>56) |
        ((x<<40) & 0x00FF000000000000ULL) |
        ((x<<24) & 0x0000FF0000000000ULL) |
        ((x<<8) & 0x000000FF00000000ULL) |
        ((x>>8) & 0x00000000FF000000ULL) |
        ((x>>24) & 0x0000000000FF0000ULL) |
        ((x>>40) & 0x000000000000FF00ULL) |
        (x<<56);
  }


  /** Prepare transmission of character strings
    * In case of endian swap all charcter strings
    * are scrambled. It is necessary to reorder
    * the strings before transmission.
    */
   inline void endian_prepare_string(unsigned long& x){
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
         (x<<24);
   }   

  /** Prepare transmission of short arrays
    * In case of endian swap all short arrays
    * are scrambled. It is necessary to reorder
    * the arrays before transmission.
    *
    * Note: The function will only work if a even
    * number of short values is transmitted.
    */
   inline void endian_prepare_short(unsigned long& x){
    x = (x>>16) |
        (x<<16);
   }
    

#if 0
   /** Swap endians for short long ints */
   inline void endian_swap(unsigned int& x){
     //printf("Swap: %d --> ", x);
     x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
     //printf("%d \n", x);
   }
#endif

   inline void endian_swap(double& x){
     endian_swap((unsigned long long &) x);
   }



//private:
  unsigned short port;

  SOCKET sock;

  /** Reference time */
  struct timeval tRef;
  
  struct timezone tZone;
  
  /** Time atom = sampling time */
  struct timeval tSample;

  /** Index of the actual sampling interval */
  int index;
  
  /** Number of timeouts (Counter) */
  int nSamples;
  
  /** Index of the last handled sample */
  int lastIndex;

  /** Timeout value */
  //struct timeval timeout;
  
  /** Contians the actal timeout time. The timer shows the remaining time until
    * the timeout condition meets
    */
  struct timeval ttimer;

  bool useTimeout;
  
  bool connLogging;
};

#endif
