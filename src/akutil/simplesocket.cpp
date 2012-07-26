/***************************************************************************
    simplesocket.cpp  -  description

    begin                : Tue Oct 24 2000
    copyright            : (C) 2000 by A Kopmann
    email                : kopmann@hpe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifdef __GNUC__
# include <fcntl.h>
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>
# include <errno.h>
# include <string.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
#endif // __GNUC__

#ifdef __WIN32__
# include <winsock2.h>
#endif // __WIN32__

#include <fdhwlib.h>

#include "simplesocket.h"


SimpleSocket::SimpleSocket(const char *hostname, unsigned short port){
  setTimeout(SIMPLESOCKET_TIMEOUT);
  connectServer(hostname, port);
}

SimpleSocket::SimpleSocket(const char *hostname, unsigned short port, unsigned long timeout){
  setTimeout(timeout);
  connectServer(hostname, port);
}

SimpleSocket::SimpleSocket(const char *hostname, unsigned short port, struct timeval timeout){
  setTimeout(timeout);
  connectServer(hostname, port);
}


void SimpleSocket::connectServer(const char *hostname, unsigned short port){  
  struct sockaddr_in servername;
  struct hostent *hostinfo;

 
#ifdef __WIN32__
  WSADATA wsaData;

  if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) {
    fprintf(stderr,"WSAStartup failed with error %d\n",WSAGetLastError());
    WSACleanup();

    throw std::invalid_argument("WSAStartup");
  }
#endif


  // Get hostname?!
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL){
    //fprintf (stderr, "SimpleSocket: Unknown host %s.\n", hostname);
    throw std::invalid_argument(strerror(errno));
  }          

  //printf("Hostinfo received\n");

  // Create the socket.
  sock = socket (AF_INET, SOCK_STREAM, 0);   // PF_INET ???
  if (sock < 0){
    //perror ("socket (client)");
    throw std::invalid_argument(strerror(errno));
  }

  //printf("Socket created\n");

  
  // Make socket non blocking - before connecting to server
  //fprintf(stderr, "SimpleSocket: Use nonblocking socket to connect %s:%d\n",
  //                hostname, port);
  nonblock = true;
  if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1){
    nonblock = false;
    fprintf(stderr, "SimpleSocket: Failed to make socket nonblocking\n");
  }  
      
  // compile the parameters!!!
  servername.sin_family = AF_INET;
  servername.sin_port = htons (port);
  servername.sin_addr = *(struct in_addr *) hostinfo->h_addr;
  memset(&(servername.sin_zero), '\0', 8);
  //bzero(&servername, 8);        /* zero the rest of the struct */

    
  // Connect to the server.
  if (connect (sock,(struct sockaddr *) &servername,sizeof (struct sockaddr)) < 0) {
    //perror ("connect (client)");

    //printf("SimpleSocket: Result of connect is errno=%d EINPROGRESS=%d\n", errno, EINPROGRESS);
    
    if (errno == EINPROGRESS){
      //fprintf(stderr, "Connection in progress");
    } else {    
      throw std::invalid_argument(strerror(errno));
    }  
  }

  //printf("Connected\n");

  // Check the connection status
  if (nonblock){
    int err;

    err = waitForWriting();    
    //printf("SimpleSocket: Number of sockets in set %d\n", err);
    
    if (err < 0){
      fprintf(stderr, "SimpleSocket: Select error waiting for connection\n");
    }

    if (err == 0){ // Timeout
      // Close connection
      // netstat will report SYN_SEND?!
      close(sock);    
      throw std::invalid_argument("Connection timeout (host not responding)");
    }          

    // Test the connection
    // If the host is existing, there will be always a connection
    // also the port might not be served!?
    // --> Is there also another way to detect this?
    socklen_t peerlen;
    struct sockaddr peername;
    
    peerlen = sizeof (struct sockaddr);
    err = getpeername(sock, &peername, &peerlen);
    //printf("getpeername result = %d \n", err);

    if (err < 0){
      // Close connection
      // netstat will report SYN_SEND?!
      close(sock);
      throw std::invalid_argument("Connection failed (port not served)");
    }    
    
/*    
    // Set back to blocking
    if (fcntl(sock, F_SETFL, 0) == -1){
      nonblock = false;
      fprintf(stderr, "SimpleSocket: Nonblock is still active\n");
    }  
*/
        
  }  

    
}


SimpleSocket::~SimpleSocket(){
  //printf("SimpleSocket::~SimpleSocket\n");
#ifdef __GNUC__
  close(sock);
#endif

#ifdef __WIN32__
  closesocket(sock);
  WSACleanup();
#endif
}

void SimpleSocket::setTimeout(unsigned long timeout){
  this->timeout.tv_sec = timeout / 1000;
  this->timeout.tv_usec = (timeout %1000) * 1000;

  if (timeout == 0)
    this->nonblock = false;
  else
    this->nonblock = true;    
}


void SimpleSocket::setTimeout(struct timeval timeout){

  this->timeout.tv_sec = timeout.tv_sec;
  this->timeout.tv_usec = timeout.tv_usec;

  if ((timeout.tv_sec == 0 && timeout.tv_usec == 0))
    this->nonblock = false;
  else
    this->nonblock = true;            
}


int SimpleSocket::readMsg(char *msg, int len){
  int nbytes;

  //nbytes = read (sock, msg, len);
  nbytes = recv (sock, msg, len-1, 0); // last character needed for string end (\0)

  if (nonblock){
    if ((nbytes <0) && (errno == EAGAIN)){
      int err;
      err = waitForReading();

      if (err < 0){ // error / connection lost?
         //printf("SimpleSocket::readData() Timeout error (err=%d)\n", err);
         printf("SimpleSocket::readMsg: Error waiting for data (err = %d, errno = %d)\n", err, errno);
         return(-1);

      } else if (err == 0){
         return(0); // Time-out

      } else { // err > 0
        // Read again, now that the data should have been arrived
        nbytes = recv (sock, msg, len-1, 0); // last character needed for string end (\0)
      }
        
    } 
  }
  
  if (nbytes < 0) {
    // Read error.
    //perror ("SimpleSocket::readMsg");
    //throw std::invalid_argument(strerror(errno));
    return(-errno);
    
  } else {
    if (nbytes == 0){
      // End-o -le.
      //printf("readMsg: Connection lost!\n");
      //throw std::invalid_argument("Connection lost"); // ???

      return(-EPIPE);

    } else {
      // Data read. //
      msg[nbytes] = 0;
      //printf ("readMsg: Got message: %s\n", msg);
    }
  }

  return(nbytes);
}


int SimpleSocket::writeMsg(char *msg, int n){
  int nbytes;
  int len;

  len = n;
  if (n == 0){
    len = strlen (msg) + 1;
  }
  // Requests  not  to send SIGPIPE on errors
#ifdef __linux__  
  nbytes = send  (sock, msg, len, MSG_NOSIGNAL);
#else
  nbytes = send  (sock, msg, len, 0);
#endif

  if (nonblock){
    if ((nbytes <0) && (errno == EAGAIN)){
      int err;
      err = waitForWriting();

      if (err < 0){ // error / connection lost?
         //printf("SimpleSocket::readData() Timeout error (err=%d)\n", err);
         printf("SimpleSocket::writeMsg: Error waiting for data (err = %d, errno = %d)\n", err, errno);
         return(-1);

      } else if (err == 0){
         return(0); // Time-out

      } else { // err > 0
      
      // Read again, now that the data should have been arrived
#ifdef __linux__
        nbytes = send  (sock, msg, len, MSG_NOSIGNAL);
#else
        nbytes = send  (sock, msg, len, 0);
#endif
      }

    }
  }

    
  //nbytes = write (sock, msg, strlen (msg) + 1);

  if (nbytes < 0){
    //perror ("SimpleSocket::writeMsg");
    //throw std::invalid_argument(strerror(errno));

    return(-errno);
  }

  //printf("writeMsg: %d bytes send\n",nbytes);

  return(0);

}


int SimpleSocket::readData(unsigned long *data, int max){
    int err;
    int i;
    unsigned int *buf;
	
    if (sizeof(unsigned long) > sizeof(unsigned int)){
    	    
    	    buf = new unsigned int [max];
    	    err = readData(buf, max);
    	    for (i=0;i<max;i++) data[i] = buf[i];
    	    
    	    delete buf;
    } else {
    	    err = readData( (unsigned int *) data, max);
    }

    return(err);
}


	
int SimpleSocket::readData(unsigned int *data, int max){
  int nbytes;

#ifdef __GNUC__
  //nbytes=read(sock, data, max * sizeof(unsigned long) );
  //fcntl(sock, F_GETFL, O_NONBLOCK );
  nbytes=recv(sock, (char *) data, max * sizeof(unsigned int),0 );
#endif
#ifdef __WIN32__
  nbytes=recv(sock, (char *) data, max * sizeof(unsigned int),0 );
#endif


  if (nonblock){    
    //printf("SimpleSocket::readData with nonblocking socket %d, nbytes=%d\n", errno, nbytes);
    if ((nbytes <0) && (errno == EAGAIN)){
      int err;
      err = waitForReading();

      if (err < 0){ // error / connection lost?
         //printf("SimpleSocket::readData() Timeout error (err=%d)\n", err);
         printf("SimpleSocket::readData: Error waiting for data (err = %d, errno = %d)\n", err, errno);
         return(-1);
         
      } else if (err == 0){       
         return(0); // Time-out
      
      } else { // err > 0
        // Read again, now that the data should have been arrived
        nbytes=recv(sock, (char *) data, max * sizeof(unsigned int),0 );
        //printf("Nbytes = %d\n", nbytes);

        if ((nbytes <0) && (errno == EAGAIN)){
          return(0); // Timeout
        }        

        
      }  
    }
  }

  //printf ("readData: Read %d Bytes\n", nbytes);
  
#ifdef USE_BYTEORDER
  int ndata;
  int i;

  // Convert network format to host format
  ndata = nbytes / sizeof (unsigned int);
  for (i=0;i <ndata;i++)
    data[i] = ntohl(data[i]);
#endif
  
  if (nbytes < 0) {
    // Read error.
    //perror ("SimpleSocket::readMsg");
    //throw std::invalid_argument(strerror(errno));
    return(- errno);

  } else {
    if (nbytes == 0){
      // End-o -le.
      //printf("readMsg: Connection lost! (EPIPE = %d)\n", EPIPE);
      //throw std::invalid_argument("Connection lost"); // ???

      return(- EPIPE); // Broken pipe ???
      //return(-1);
    }

  }

  return nbytes;
}


int SimpleSocket::writeData(unsigned long *data, int n){
    int err;
    int i;
    unsigned int *buf;
	
	if (sizeof(unsigned long) > sizeof(unsigned int)){
		
		buf = new unsigned int [n];
		for (i=0;i<n;i++) buf[i] = data[i];
		err = writeData(buf, n);
		
		delete buf;
	} else {
		err = writeData( (unsigned int *) data, n);
	}
	
	return(err);
	
}


int SimpleSocket::writeData(unsigned int *data, int n){
  int nbytes;

#ifdef USE_BYTEORDER
  int i;

  // Convert host format to network format
  for (i=0;i <n;i++)
    data[i] = htonl(data[i]);
#endif

  
  //nbytes = write (sock, data, n * sizeof(unsigned int));
#ifdef __linux__  
  nbytes = send (sock, (char *) data, n * sizeof(unsigned int), MSG_NOSIGNAL);
#else
  nbytes = send (sock, (char *) data, n * sizeof(unsigned int), 0);
#endif


  if (nonblock){
    if ((nbytes <0) && (errno == EAGAIN)){
      int err;
      err = waitForWriting();

      if (err < 0){ // error / connection lost?
         printf("SimpleSocket::writeData: Error waiting for data (err = %d, errno = %d)\n", err, errno);
         return(-1);
      } else if (err == 0){ // timeout
         return(0);
      } else { // err > 0
        // Read again, now that the data should have been arrived
#ifdef __linux__
        nbytes = send (sock, (char *) data, n * sizeof(unsigned int), MSG_NOSIGNAL);
#else
        nbytes = send (sock, (char *) data, n * sizeof(unsigned int), 0);
#endif

        if ((nbytes <0) && (errno == EAGAIN)){
          return(0); // Timeout
        }

      }
    }
  }

  
  if (nbytes < 0){
    //perror ("SimpleSocket::writeData");
    //throw std::invalid_argument(strerror(errno));

    return(-errno); // ???
    //return(1);
  }

  //return(0);
  return(nbytes);
}


char *SimpleSocket::getHostName(){
  int err;
  struct sockaddr_in clientname;
  char *hostName;
//#ifdef __linux__
#ifdef __GNUC__
  socklen_t size;
#else
  int size;
#endif

  hostName = 0;


  size = sizeof (clientname);
  err = getsockname (sock, (struct sockaddr *) &clientname, &size);
  if (err  ==  0){
     hostName = inet_ntoa (clientname.sin_addr);
  }


  return( hostName );

}


char *SimpleSocket::getPeerName(){
  int err;
  struct sockaddr_in clientname;
  char *peerName;
//#ifdef __linux__
#ifdef __GNUC__
  socklen_t size;
#else  
  int size;
#endif

  peerName = 0;

  size = sizeof (clientname);
  err = getpeername (sock, (struct sockaddr *) &clientname, &size);
  if (err  ==  0){
     peerName = inet_ntoa (clientname.sin_addr);
  }

  return( peerName );

}


int SimpleSocket::readPacket(unsigned long *data, int max){
  int nBytes;
  int rest;
  unsigned long header;
  unsigned long trailer;

  
  // Find a valid message header
  // Skip obsolete data from the stream

  nBytes = 1;
  while ((nBytes > 0) && (header >> 16 != 0xffff)){
    nBytes = readData(&header, 1);

    // Check the endianess ??
    
  } 

  if (nBytes <= 0){
    return(nBytes);    
  }  

  // Get the message
  // If necessary assemble from more than one messages
  rest = max;
  nBytes = 1;
  while ((nBytes > 0) && (rest > 0)){
    nBytes = readData(data+(max-rest), rest);

    if (nBytes > 0) {
      rest = rest - nBytes / sizeof(unsigned long);
    }      
  }  

  if (nBytes <= 0){
    return(nBytes);
  }

  
  // Check the trailer
  nBytes = readData(&trailer, 1);
  if (trailer != header){
    return(-3); // Packet error
  }  
    
  if (nBytes <= 0){
    return(nBytes);
  }

  // Got all data      
  return(max * sizeof(unsigned long));  
}  


int SimpleSocket::writePacket(unsigned long *data, int n){
  int nBytes;
  unsigned long *packet;

  // Add header and trailer to the original messages
  packet = new unsigned long [n+2];
  
  packet[0] = 0xffff << 16 | n;
  memcpy(packet, data, n * sizeof(unsigned long));
  packet[n+1] = 0xffff << 16 | n;
 
  
  nBytes = writeData(packet, n+2);
  
  if (nBytes > 0) nBytes = nBytes - 2 * sizeof(unsigned long);

  delete [] packet;
  
  return(nBytes);
}


int SimpleSocket::remoteCall(short cmdid, unsigned long *args, short argc,
					  unsigned long *data, short ndata,
					  unsigned long *ackn, short len){
    int err;
    int i;
    unsigned int *bufArgs, *bufData, *bufAckn;

	if (sizeof(unsigned long) > sizeof(unsigned int)){
		
		bufArgs = new unsigned int [argc];
		bufData = new unsigned int [ndata];
		bufAckn = new unsigned int [len];
		
		for (i=0;i<argc;i++) bufArgs[i] = args[i];
		for (i=0;i<ndata;i++) bufData[i] = data[i];
		err = remoteCall(cmdid, bufArgs, argc, bufData, ndata, bufAckn, len);
		for (i=0;i<len;i++) ackn[i] = bufAckn[i];
		
		delete bufArgs;
		delete bufData;
		delete bufAckn;
	} else {
	
		err = remoteCall(cmdid, (unsigned int *) args, argc, 
						 (unsigned int *) data, ndata, 
						 (unsigned int *) ackn, len);
	}
	
	return(err);
}	
	

int SimpleSocket::remoteCall(short cmdid, uint32_t *args, short argc,
                                          uint32_t *data, short ndata,
                                          uint32_t *ackn, short len){
  int i, err, nr, tlen;
  int rtn;
  unsigned int status;
  short nAck;

  rtn = 0;
    

  //printf("SimpleSocket::remoteCall(cmdid=%d, argc=%d, ndata=%d, len=%d)\n", cmdid, argc, ndata, len);	 
	
  //unsigned int parg[1+argc+ndata];
  //unsigned int parg[256], pack[256];  
  uint32_t *parg = new uint32_t [1+argc+ndata];

  nAck = argc+ndata+1;
  parg[0]= ((uint32_t) cmdid << 16) + (nAck & 0xffff);

  for (i=0;i<argc;i++)
    parg[1+i]= args[i];

  for (i=0;i<ndata;i++)
    parg[1+argc+i]=data[i];


//#ifdef USE_SEMAPHORE
//  mE.request(SEM_IP);
//#endif

//#ifdef USE_MFC_LOCAL_SYNC
  //CSingleLock lock( &s_sock,true); // starting locked
  //CSingleLock lock( s_sock,true); // starting locked
  //lock.Lock();
//#endif


  //nr = sock[telescopeId]->writeData(parg, 1+argc+ndata );

#ifdef USE_PACKET_LAYER
#error "Not tested now"
  nr = writePacket(parg, 1+argc+ndata );
#else  
  nr = writeData(parg, 1+argc+ndata );
#endif

      
  if (nr <= 0){
    //fprintf(fdebug,"remoteCall: sending cmd=%d, err=%d, parg[0]=%08x %08x\n",
    //        cmdid,nr,parg[0],parg[1]);
    printf("SimpleSocket::remoteCall(%d): err=%d errno=%d, parg[0]=%08x %08x\n",
            cmdid,nr, errno, parg[0],parg[1]);
    err = nr;
                      
  } else {  // Wait for acknoledge

    // reading the answer ?!
    // The maximum size for one packet is about 1000 Longwords?!
    // does not work , if client and server are in the same thread!!!

    // reading status

#ifdef USE_PACKET_LAYER
    recBuffer = new unsigned int [len +1];
    
    err = readPacket(recBuffer, len + 1);
    rtn = recBuffer[0]; // First byte is the status
    
    if (err == ((1+len) * sizeof(unsigned int))){
      memcpy( ackn, recBuffer+1, len * sizeof(unsigned int));
    }  

    
    delete [] recBuffer;

#else    
    //err = sock[telescopeId]->readData(&status, 1);
    err = readData(&status, 1);
#endif    
    
  }


  // Error handling
  if (err <= 0){

    printf("SimpleSocket::remoteCall(%d): Error = %d (errno = %d)\n", cmdid, err, errno);
    // Warning: Connection closed by server.
    //if (fout) fprintf(fout, "Warning: Connection closed by server.\n");
    // return(-1); // Connection closed by foreign host
   
    if (err < 0)  rtn = -1; // Connection closed by foreign host

    if (err == 0) rtn = -2; // Timeout

    
  }

#ifndef USE_PACKET_LAYER
  else {
    // Command was successful
    rtn = status;
        
    //printf("The Status is %ld\n",status);
    if (status== 0){ // pbus command was successfull!

      nr=0;
      tlen = 0;
      err = 0;
      while (tlen < len) {
        //err = sock[telescopeId]->readData(ackn + tlen, len);
        err = readData(ackn + tlen, len - tlen);
        if (err <0) {
          printf("SimpleSocket::remoteCall: Error while reading the data (len=%d)\n", len);
          break;
        }
        nr = nr + err;

        tlen = nr / (int) sizeof(unsigned int);

        //printf("remoteCall: acknoledge err=%d, tlen=%d, len=%d\n",nr,tlen,len);
      }

      // TODO: Error handling
      if (err < 0){
        
      }  
      
      
    } else {

//#ifdef USE_SEMAPHORE
//    mE.release(SEM_IP);
//#endif

//#ifdef USE_MFC_LOCAL_SYNC
  //lock.Unlock();
//#endif

      //printf("remoteCall: Pbus time-out?!\n");
      //throw PbusError("remoteCall: Pbus time-out?!");
    }
    
  }  
#endif

    
//#ifdef USE_SEMAPHORE
//  mE.release(SEM_IP);
//#endif

//#ifdef USE_MFC_LOCAL_SYNC
  //lock.Unlock();
//#endif



/*
  for(i=0;i<tlen;i++) {
    ackn[i]=pack[i];
    printf("---%d: len=%ld\n",i,ackn[i]);
  }
*/

  delete [] parg;


  return(rtn);

}


// NOT working at the moment - needs more testing ???
int SimpleSocket::wait(int timeout){
  int i, status;
  //int err;
  //unsigned long buf;
  fd_set active_fd_set, read_fd_set;
  struct timeval tv;
  bool interupted;

  interupted = false;

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  // Collect a list with the file descriptor to wait for
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);

  read_fd_set = active_fd_set;

  status = select (FD_SETSIZE, &read_fd_set, NULL, NULL, &tv);
    //printf("select: status = %d\n",status);
  if ( status < 0){
#ifdef __GNUC__
    perror ("select");
#endif
#ifdef __WIN32__
    printf("select failed %d\n",WSAGetLastError());
#endif

  }else{ // no select error


    // Service all the sockets with input pending.
    for (i = 0; i < FD_SETSIZE; ++i){

      if (FD_ISSET (i, &read_fd_set)){
        printf("init_server: Active socket #%d (this is socket #%d, FD_SETSIZE=%d)\n",
                  i,sock,FD_SETSIZE);

        //err = readData(&buf, 0);

        //if (err <= 0){
            // Warning: Connection closed by server.
            //if (fout) fprintf(fout, "Warning: Connection closed by server.\n");
            interupted = true;
        //}


      }


    }



  }

  return(interupted);

}


int SimpleSocket::waitForReading(){
    fd_set connect_fd_set;
    int err;
    struct timeval t;

    FD_ZERO (&connect_fd_set);
    FD_SET (sock, &connect_fd_set);

    t.tv_sec = timeout.tv_sec;
    t.tv_usec = timeout.tv_usec;
    
    // Wait for the socket
    errno = 0;
    err = select (FD_SETSIZE, &connect_fd_set, NULL, NULL, &t);

    
    return(err);
}    



int SimpleSocket::waitForWriting(){
    fd_set connect_fd_set;
    int err;
    struct timeval t;

    FD_ZERO (&connect_fd_set);
    FD_SET (sock, &connect_fd_set);

    t.tv_sec = timeout.tv_sec;
    t.tv_usec = timeout.tv_usec;

    // Wait for the socket
    errno = 0;
    err = select (FD_SETSIZE, NULL, &connect_fd_set, NULL, &t);

    
    return(err);
}


int SimpleSocket::getFD(){
  return(sock);
}
