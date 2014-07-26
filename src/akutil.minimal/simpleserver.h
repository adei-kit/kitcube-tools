/***************************************************************************
    simpleserver.h  -  description

    begin                : Wed Oct 25 2000
    copyright            : (C) 2000 by A Kopmann
    email                : kopmann@hpe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef SIMPLESERVER_H
#define SIMPLESERVER_H


#include <akutil/baseserver.h>


/** Implements a simple IP server.
  *
  * The server waits for connections.
  * The communication with
  * the client is defined in the
  * protected function read_from_client.
  * Overload this function for a specific application.
  *
  * The SimpleServer automatically detects
  * the byte order of the client process and transfers
  * all data in client byte order.
  * 
  *
  * @todo Use a independant threads for all
  *   connections. If one connection hangs, the other
  *   won't be affected?!
  * @todo Improve the Server managment. Loging,
  *   Shutdown via IP. For some other purpose a
  *   New class similar to Pbus/PbusProxy couble may
  *   be useful for mainanace and run control.
  *   Eg. Shutdown + restart of the Server, Control
  *   of the IRdispatcher!!!
  *
  *
  * Changes:
  * @li 30.8.05 ak: Detect client byte order
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

class SimpleServer : public BaseServer {
public: 
  SimpleServer(unsigned short port);
  //SimpleServer(unsigned short port, SimpleServer *s);


protected:
  /** Defines the interface from the communication with the proxy process. 
    * The function is able to detect if the client uses a different byte 
    * order and automatically re-orders the data. All following communication 
    * will be swapped to have the appropriate byte order.
    *
    * The protocol is defined as follows:
    * @li command (4 bytes)
    * @li len (4 bytes)
    * @li data (len * 4bytes)
    */
  virtual int read_from_client(int filedes);

  /** Execute a command - The instruction set
    * on the client side remoteCall needs to fit to this implementation
    */
  virtual void executeCmd(int client, short cmd, unsigned int *arg, short n);
	
  virtual void executeCmd(int client, short cmd, unsigned long *arg, short n);
};

#endif
