/***************************************************************************
    baseserver.cpp  -  description

    begin                : Fri Sep 24 2004
    copyright            : (C) 2004 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#include <sys/time.h>

#include <fdhwlib.h>

#ifdef __GNUC__
# include <akutil/keyboard.h>
#endif // __GNUC__

#ifdef __WIN32__ // windows
# include <io.h>
#endif // __WIN32__

#define MAX_LEN 255
#define LINE_ARG_MAX 5

#include "baseserver.h"

using namespace std;

BaseServer::BaseServer(unsigned short port){

  this->port = port;
  this->sock = 0; // ???
  
  shutdown=false;

  fout = stdout;
  
  useTimeout = false;

  // Get reference time
  // The defalt time is the server start time
  gettimeofday(&tRef, &tZone);
  
  // Default sampling time = 1sec
  tSample.tv_sec = 1;
  tSample.tv_usec = 0;
  
  nSamples = 0; // Clear sample counter
  debug = 0;  
  connLogging = true;
}


void BaseServer::setDebugLevel(int debug){
  this->debug = debug;
}


void BaseServer::setPort(unsigned short port){
  this->port = port;	
}


void BaseServer::init(FILE *fdebug){

  char sockname[255];
  struct sockaddr_in name;

  fout = fdebug;
  
  // There is no port specified - can create server socket
  //printf("BaseServer:port = %d\n", port);
  if (this->port <= 1){
     //printf("Warning: Do not create server socket. Port number is missing\n");
     return;
  }
#ifdef __WIN32__
  WSADATA wsaData;

  if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) {
    fprintf(stderr,"WSAStartup failed with error %d\n",WSAGetLastError());
    WSACleanup();

    throw invalid_argument("WSAStartup");
  }
#endif


  // Create the socket.
  //printf("init: Create socket on port %hd\n",this->port);
  sock = socket (PF_INET, SOCK_STREAM, 0);  // ??? AF_INET / PF_INET
  if (sock == -1) {
#ifdef __GNUC__
    perror ("socket");
#else
  fprintf(stderr,"socket() failed with error %d\n",WSAGetLastError());
  WSACleanup();
#endif
    throw invalid_argument("socket");

  } else {
    //printf("init: Socket %d created\n",sock);



    //
    //  turn off bind address checking, and allow
    // port numbers to be reused - otherwise
    // the TIME_WAIT phenomenon will prevent
    // binding to these address.port combinations
    // for (2 * MSL) seconds.
    //

    int on = 1;

    //printf("Set socket parameter: Allow re-use of port numbers in WAIT_TIME state\n");
    if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR,
                    (const char *) &on, sizeof(on)   ) < 0){

#ifdef __GNUC__
      perror("setsockopt(...,SO_REUSEADDR,...)");
#else
      fprintf(stderr,"setsockopt(...,SO_REUSEADDR,...) failed with error %d\n",WSAGetLastError());
      WSACleanup();
#endif
    }


    // Give the socket a name.
    name.sin_family = AF_INET;
    name.sin_port = htons (port);
    //name.sin_addr.s_addr = htonl (INADDR_ANY);
    name.sin_addr.s_addr = INADDR_ANY;
    //bzero(&(name.sin_zero), 8);        /* zero the rest of the struct */


    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
#ifdef __GNUC__
      perror ("bind");
#else
      fprintf(stderr,"bind() failed with error %d\n",WSAGetLastError());
      WSACleanup();
#endif
      throw invalid_argument("bind");
    } else {
      // now it is possible to call init_server()
    }


    sprintf(sockname,"%s:%hd", inet_ntoa (name.sin_addr), ntohs (name.sin_port));
    if (fout) fprintf(fout, "%3s %-24s %-24s %s\n","Fd", "Local address", "Foreign address", "State");
    if (fout) fprintf(fout, "%3d %-24s %-24s %s\n", sock, sockname, "", "Server socket created");
  }

}


BaseServer::~BaseServer(){

  if (port > 1){
    if (sock >0){
#ifdef __GNUC__
      close(sock);

#else // Windows
      closesocket(sock);
      WSACleanup();
#endif
    }
  }
  
}



void BaseServer::kill_server(){
}


void BaseServer::displayVersion(){
}

void BaseServer::displayActiveSockets(){
  displayActiveSockets(this->fout);
}


void BaseServer::displayActiveSockets(FILE *fout){
  int j, err;
  char sockname[255], peername[255];
  struct sockaddr_in clientname;
//#ifdef __linux__
#ifdef __GNUC__
  socklen_t size;
#else
  int size;
#endif


  size = sizeof (clientname);
  if (fout) fprintf(fout, "%3s %-24s %-24s %s\n","Fd", "Local address", "Foreign address", "State");

  for (j = 0; j < FD_SETSIZE; ++j){

    if (FD_ISSET (j, &active_fd_set)){

       // Clear strings
       sockname[0] = 0;
       peername[0] = 0;

       if (j == fileno(stdin)) { // Standard input
         if (fout) fprintf(fout, "%3d %-24s %-24s %s\n",j, "stdin", "", "Waiting for keyboard input");

       } else if (j == sock) { // Server socket
         err = getsockname (j,(struct sockaddr *) &clientname, &size);
         if (err  ==  0){
           sprintf(sockname,"%s:%d",inet_ntoa (clientname.sin_addr),
                                    ntohs (clientname.sin_port));
         }
         if (fout) fprintf(fout, "%3d %-24s %-24s %s\n",j, sockname, "", "Waiting for connections");


       } else { // Any other (client) connection
         err = getsockname (j,(struct sockaddr *) &clientname, &size);
         if (err  ==  0){
           sprintf(sockname,"%s:%d",inet_ntoa (clientname.sin_addr),
                                    ntohs (clientname.sin_port));
         }
         err = getpeername (j,(struct sockaddr *) &clientname, &size);
         if (err  ==  0){
           sprintf(peername,"%s:%d",inet_ntoa (clientname.sin_addr),
                                    ntohs (clientname.sin_port));
         }

         if (fout) fprintf(fout, "%3d %-24s %-24s\n",j, sockname, peername);
       }

    }
  }

  return;

}


void BaseServer::setTRef(struct timeval *time){

   if (time > 0){
     this->tRef.tv_sec = time->tv_sec;
	 this->tRef.tv_usec = time->tv_usec;
   }
}
   

void BaseServer::enableTimeout(struct timeval *timeout){

   this->useTimeout = true;

   if (timeout > 0){
     this->tSample.tv_sec = timeout->tv_sec;
     this->tSample.tv_usec = timeout->tv_usec;
   }

}

void BaseServer::disableTimeout(){
   this->useTimeout = false;
}


void BaseServer::getNextTimeout(int *iSample, struct timeval *timeout){
   struct timeval tNow;
   double tDiff;
   double tSampleDouble;
   double tWait;
   
   
   gettimeofday(&tNow, &tZone);
   tDiff = ((double) (tNow.tv_sec - tRef.tv_sec) ) + ((double) (tNow.tv_usec - tRef.tv_usec)) / 1000000;
   tSampleDouble =  ((double) tSample.tv_sec) + ((double) tSample.tv_usec) / 1000000;
   
    if (tDiff >0) {
		*iSample = (int) (tDiff / tSampleDouble) + 1;
	} else {
		*iSample = (int) (tDiff / tSampleDouble);
	}

	tWait = (*iSample) * tSampleDouble - tDiff;
   
   // Check if the last index has been handled
   if ((*iSample) - lastIndex > 1){
      
	  printf("Warning: Scheduled sampling time has been missed!\n");
	  if (tWait > 0.5 * tSampleDouble){
	    // Just do the sample now - without waiting
	    (*iSample) = (*iSample) -1;
		// Go through select
	    tWait = 0;
		
		// TODO: Better call handle_timeout here directly 
		// All other events have a higher priority in select() !!!
     
		// Give some message or change the next timeout
	    //lastIndex = index; // mark this sample as handled
        //handle_timeout();		

		// TODO: Recursive call of getNextTimeout
        //getNextTimeout(iSample, timeout);		
		
	  } else {
	    // Loose thhis sample, because of traffic?!
		// move the lastHandledCounter
		printf("Error: Sample(s) %d..%d skipped\n", lastIndex+1, (*iSample)-1);
		lastIndex = (*iSample) -1; 
	  }
	  	  
   }
   
   
   if (tWait > 0) {
      timeout->tv_sec = (int) tWait;
      timeout->tv_usec =  ( (int) (tWait * 1000000) % 1000000);
   } else {
	  timeout->tv_sec = 0;
	  timeout->tv_usec = 0;
   }
   
   //printf("t=%f, tSample=%f, id=%d, tWait=%f\n", tDiff, tSampleDouble, *iSample, tWait);
   //printf("tSample = %d sec + %d usec , timeout = %d sec + %d usec \n",
   //       tSample.tv_sec, tSample.tv_usec, timeout->tv_sec, timeout->tv_usec);    

}

void BaseServer::init_server(int fd1, int fd2){
  int err;
  SOCKET i, j;
  int status;
  fd_set read_fd_set;
  //char sockname[255];
  char peername[255];
  struct sockaddr_in clientname;

  
//#ifdef __linux__
#ifdef __GNUC__
  socklen_t size;
#else
  int size;
#endif

#ifdef __GNUC__
  keyboard *kb;  // Change keyboard parameters

  kb = 0;
#endif

  // Clear sets used in the server
  FD_ZERO (&active_fd_set);
#ifdef USE_DYN_BYTEORDER
  FD_ZERO (&change_byteorder_fd_set);
#endif

    
  // Create the socket and set it up to accept connections.
  if (port > 0){
    if (listen (sock, 10) < 0){
#ifdef __GNUC__
      perror ("listen");
#else
      fprintf(stderr,"listen() failed with error %d\n",WSAGetLastError());
      WSACleanup();
#endif
      throw invalid_argument("listen");
    }



    // Initialize the set of active sockets.
    // In the beginning there is only the lisening server!
    if (sock >= FD_SETSIZE ) {
      fprintf (stderr,"Error: FD_SETSIZE= %d is smaller than socket =%d\n",
                     FD_SETSIZE,sock);
      return;
    }

    // Add server socket to active set
    FD_SET (sock, &active_fd_set);
  }
  

  // Add stdin to the list of active fd's
  if (fd1 > -1){
    //printf("Add fd=%d to active set\n", fd1);
    FD_SET (fd1, &active_fd_set);
#ifdef __GNUC__
    if (fd1 == fileno(stdin))
      kb = new keyboard;
#endif
  }

  if (fd2 > -1){
    //printf("Add fd=%d to active set\n", fd2);
    FD_SET (fd2, &active_fd_set);
#ifdef __GNUC__
    if (fd2 == fileno(stdin))
      kb = new keyboard;
#endif
  }

  // Test fd_set ?!
  //printf("Set size is %d, listen socket =%d\n",FD_SETSIZE,sock);
  //for (i=0;i<FD_SETSIZE;i++){
  //  printf ("%d",FD_ISSET(i, &active_fd_set));
  //}
  //printf("\n");

  // Set initial timer
  ttimer.tv_sec = tSample.tv_sec;
  ttimer.tv_usec = tSample.tv_usec;

  // Initialize the last handled index
  lastIndex = 0;


  while (!shutdown){  // main accept() loop

    // Block until input arrives on one or more active sockets.
#ifdef debug
    printf("init_server: Select waiting for packets?! (shutdown=%d)\n",shutdown);
#endif
    read_fd_set = active_fd_set;

    //for (i=0;i<FD_SETSIZE;i++){
    //  if (FD_ISSET (i, &read_fd_set))
    //    printf("Read mask %d, Active %d\n",i, FD_ISSET(i,&active_fd_set) );
    //}

    if (useTimeout){
	  // Calculate next sample time
	  getNextTimeout(&index, &ttimer);
	
      status = select (FD_SETSIZE, &read_fd_set, NULL, NULL, &ttimer);
    } else {
      status = select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);
    }
    //printf("select: status = %d\n",status);
    //printf("Id=%6d -- Timer: %d sec + %d usec\n", index, ttimer.tv_sec, ttimer.tv_usec);   

    if ( status < 0){
#ifdef __GNUC__
      perror ("select");
#else
      if (fout) fprintf(fout,"select failed %d\n",WSAGetLastError());
#endif 

      // Do not continue in case of an select error 
	  // Might be something severe?!
	  shutdown = true;

    } else if (status == 0){ // timeout

      // Give some message or change the next timeout
	  lastIndex = index; // mark this sample as handled
	  nSamples++;        // increment the timeout counter
      handle_timeout();

    } else { // no select error


      // Service all the sockets with input pending.
      for (i = 0; i < FD_SETSIZE; ++i){

        if (FD_ISSET (i, &read_fd_set)){

          //printf("init_server: Active socket #%d (this is socket #%d, FD_SETSIZE=%d)\n",
          //        i,sock,FD_SETSIZE);

          // Check for charcter input ?!
          if (i == fileno(stdin)){

            err = read_from_keyboard();

            //printf("Shutdown = %d\n", this->shutdown);

          } else if (i == sock){

            // Connection request on original socket.
            // --> All request for a new connecting will reach this socket
            size = sizeof (clientname);
            if ((status = accept (sock,(struct sockaddr *) &clientname, &size)) < 0){
                perror ("accept");

            }else{
              sprintf(peername, "%s:%hu", inet_ntoa (clientname.sin_addr),
                                          ntohs (clientname.sin_port));
              //printf("init_server: New connection\n");
              if (fout && connLogging)
	        fprintf(fout, "%3d %-24s %-24s %s\n", status, "", peername, "New connection");
              //fprintf (fout,"init_server: connect from host %s, port %hd, new socket %d.\n",
              //           inet_ntoa (clientname.sin_addr),
              //           ntohs (clientname.sin_port),
              //           status);

              // Add the new socket to the active set scanned by select()!
              FD_SET (status, &active_fd_set);
            } // end - if accept


          } else {// i != sock --> any other connected socket


            // Data received from an already connected socket.
            //printf("init_server: There is some data...\n");
            //displayActiveSockets(); // no clients found here ?!

            err = read_from_client (i);


            if (err < 0) {
              // the connection was lost - close the socket and
              // remove it from the active set!
              if (fout && connLogging)
	        fprintf(fout, "%3d %-24s %-24s %s\n",i, "", "", 
		        "Connection closed by foreign host");
              //fprintf(fout,"init_server: Connection closed by foreign host\n");
#ifdef __GNUC__
              err = close(i);
              if (err == -1)  // Allways check errors on close !!!
                 perror("close");
#else
              closesocket(i);
#endif
              FD_CLR (i, &active_fd_set);

#ifdef USE_DYN_BYTEORDER
              // Clear also all other lists
              FD_CLR (i, &change_byteorder_fd_set);
#endif
            } // end - if read error

          } // end - if sock == ...

        } // end of if active

      } // end of for

    } // end - if no select error

    //printf("Continue\n");
    //scanf("%c",&tag);

    //fflush(fout);

  } // end of while


  //printf("init_server: Server is shuting down (fout = %d, FD_SETSIZE = %d)\n",
  //       fout, FD_SETSIZE);

  // Close all connections?!
  for (j = 0; j < FD_SETSIZE; ++j){
    if (FD_ISSET (j, &active_fd_set)){
      if (j == fileno(stdin)){
         // Reset keyboard
         if (fout) fprintf(fout, "%3d %-24s %-24s %s\n",j, "Stdin", "", "Reset keyboard parameters");
#ifdef __GNUC__
         delete kb;
#endif

      } if ((j == fd1) || (j== fd2)) {
         // Do not close the connection open by external
         // programs
         
      } else { // connected socket
         if (fout) fprintf(fout, "%3d %-24s %-24s %s\n",j, "", "", "Closing socket");
         //fprintf(fout, "Shutdown: Close connection to socket %d\n", j);
#ifdef __GNUC__
         close(j);
#else
         closesocket(j);
#endif
      }
    }
  }

  return;
}


int BaseServer::readMsg(int client, char *msg, int n){
  int nbytes;

  try{
    //nbytes = read (client, msg, n);
    nbytes = recv (client, msg, n, 0);
  } catch(...) {
  nbytes = -1;
  }

  if (nbytes > 0) {
    msg[nbytes]=0; // generate a valid string
    if (fout) fprintf (fout,"readMsg: Read %d Bytes, strlen=%ld\n", nbytes, strlen(msg));
  }
  return( nbytes );
}


int BaseServer::writeMsg(int client, char *msg, int n){
  int nbytes;
  int len;
  
  len = n;
  if (n == 0){
    len = strlen (msg) + 1;
  }
  
  try{
  //nbytes = write (client, msg, strlen (msg) + 1);
#ifdef __linux__
  nbytes = send (client, msg, len, MSG_NOSIGNAL);
#else
  nbytes = send (client, msg, len, 0);
#endif

  } catch(...) {
  nbytes = -1;
  }
  if (nbytes < 0){
    perror ("send");
    //exit (EXIT_FAILURE);
    return(1);

  } else {
    if (fout) fprintf(fout,"writeMsg: %d bytes send\n",nbytes);
    return(0);
  }

  return(0);
}


int BaseServer::readData(int client, unsigned long *data, int n){
	int err;
    int i;
    unsigned int *buf;
	
	if (sizeof(unsigned long) > sizeof(unsigned int)){
		
		buf = new unsigned int [n];
		err = readData(client, buf, n);
		for (i=0;i<n;i++) data[i] = buf[i];
		
		delete buf;
	} else {
		err = readData(client, (unsigned int *) data, n);
	}
	
	return(err);
	
}



int BaseServer::readData(int client, unsigned int *data, int n){
  int nbytes;

  try{
    //nbytes=read(client, data, n * sizeof(unsigned int) );
    nbytes=recv(client, (char *) data, n * sizeof(unsigned int),0 );
  } catch(...) {
  nbytes = -1;
  }

#ifdef USE_BYTEORDER
  int ndata;
  int i;

  // Convert host format to network format
  ndata = nbytes / sizeof (unsigned int);
  for (i=0;i <ndata;i++)
    data[i] = ntohl(data[i]);
#endif

#ifdef USE_DYN_BYTEORDER
  // Change byte order if necessary
  if (FD_ISSET (client, &change_byteorder_fd_set)){
    int ndata;
    int i;

    // Convert host format to network format
    ndata = nbytes / sizeof (unsigned int);
    for (i=0;i <ndata;i++)
      endian_swap(data[i]);
  }
#endif

  //printf ("readData: Read %d Bytes\n", nbytes);
  return( nbytes );
}


int BaseServer::writeData(int client, unsigned long *data, int n){
    int err;
    int i;
    unsigned int *buf;
	
	if (sizeof(unsigned long) > sizeof(unsigned int)){
		
		buf = new unsigned int [n];
		for (i=0;i<n;i++) buf[i] = data[i];
		err = writeData(client, buf, n);
		
		delete buf;
	} else {
		err = writeData(client, (unsigned int *) data, n);
	}
	
	return(err);
}


int BaseServer::writeData(int client, unsigned int *data, int n){
  int nbytes;

  try{
    // If the blocksize n is larger than 350, the transfer rate
    // drops?? The effect occures not using a local connection but
    // via a real ip line.
    // It turned out, that it has no use splitting the following
    // command into two?!

#ifdef USE_BYTEORDER
  int i;

  // Convert host format to network format
  for (i=0;i <n;i++)
    data[i] = htonl(data[i]);
#endif


#ifdef USE_DYN_BYTEORDER
  // Change byte order if necessary
  if (FD_ISSET (client, &change_byteorder_fd_set)){
    int i;

    // Convert host format to network format
    for (i=0;i <n;i++)
      endian_swap(data[i]);
  }
#endif

    //nbytes = write (client, data, n * sizeof(unsigned int));
#ifdef __linux__
    nbytes = send (client, (char *) data, n * sizeof(unsigned int), MSG_NOSIGNAL);
#else
    nbytes = send (client, (char *) data, n * sizeof(unsigned int), 0);
#endif

  } catch (...) {
    nbytes = -1;
  }
  if (nbytes < 0){
    perror ("send");
    //exit (EXIT_FAILURE);
    return(1);

  } else {
    //printf("writeData: %d bytes send\n",nbytes);
    return(0);
  }

  return(0);
}



int BaseServer::readPacket(int client, unsigned long *data, int max){
  int nBytes;
  int rest;
  unsigned long header;
  unsigned long trailer;


  // Find a valid message header
  // Skip obsolete data from the stream

  nBytes = 1;
  while ((nBytes > 0) && (header >> 16 != 0xffff)){
    nBytes = readData(client, &header, 1);

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
    nBytes = readData(client, data+(max-rest), rest);

    if (nBytes > 0) {
      rest = rest - nBytes / sizeof(unsigned long);
    }
  }

  if (nBytes <= 0){
    return(nBytes);
  }


  // Check the trailer
  nBytes = readData(client, &trailer, 1);
  if (trailer != header){
    return(-3); // Packet error
  }

  if (nBytes <= 0){
    return(nBytes);
  }

  // Got all data
  return(max * sizeof(unsigned long));
}


int BaseServer::writePacket(int client, unsigned long *data, int n){
  int nBytes;
  unsigned long *packet;

  // Add header and trailer to the original messages
  packet = new unsigned long [n+2];

  packet[0] = 0xffff << 16 | n;
  memcpy(packet, data, n * sizeof(unsigned long));
  packet[n+1] = 0xffff << 16 | n;


  nBytes = writeData(client, packet, n+2);

  if (nBytes > 0) nBytes = nBytes - 2 * sizeof(unsigned long);

  delete [] packet;

  return(nBytes);
}



int BaseServer::handle_timeout(){
  struct timeval t;
  struct timezone tz;
  
  if (fout) {
     gettimeofday(&t, &tz);
     fprintf(fout, "%12d %10d Timeout occured (time [s/us]= %d/%d)\n",
		   (int)t.tv_sec, (int)t.tv_usec, (int)tSample.tv_sec, (int)tSample.tv_usec);
  }					

  return(0);
}


void BaseServer::displayMsg(unsigned char *msg, int len){
  int i, j;
  
  if (fout){
    for (i=0;i<len;i+=16){
	  for (j=0;j<16; j++){
        if (i+j<len) fprintf(fout, " %02x", msg[i+j]);
	    else fprintf(fout, " %2s", "");
	  }
	  fprintf(fout, "   ");
	  for (j=0;j<16; j++){
        if (i+j<len) {
		  if (isprint(msg[i+j]))
		    fprintf(fout, "%c", msg[i+j]);
		  else
		    fprintf(fout, "%c", '.');		  	 
	    } else fprintf(fout, " %c", ' ');
	  }
	  fprintf(fout, "\n");
    }
	fprintf(fout, "\n");
  }  

}

int BaseServer::read_from_client (int filedes){
  char buffer[MAX_LEN];
  int nbytes;

  nbytes = readMsg (filedes, buffer, MAX_LEN);
  if (nbytes < 0) {
    // Read error.
    perror ("read");

  } else {
    if (nbytes == 0)
      // End-o -le.
      return -1;
    else {
      // Data read.
      if (fout) {
	    fprintf (fout, "read_from_client: got message (%d bytes)\n", nbytes);
		displayMsg( (unsigned char *) buffer, nbytes);
	  }	
      return 0;
    }
  }

  return 0;
}


int BaseServer::read_from_keyboard(){
  int err;
  char buf[256];

#ifdef __GNUC__
  err = read (fileno(stdin), buf, 256);
#else // windows ?
  err = _read (fileno(stdin), buf, 256);
#endif

  if (err > 0) {
    buf[err] = 0; // Add string terminator
    //printf("%s", buf); fflush(stdout);

    // Command interpreter for stdin
    switch (buf[0]){
      case 'q': // Shutdown server
      case 'Q':
        shutdown = true;
        break;

      case 's':
      case 'S':
        displayActiveSockets();
        break;
    }
  } else {
    // In background mode the keyboard will be active
    // but the process will not receive any characters ?!
    //fprintf(stderr,"Warning: No characters found\n");
  }

  return(0);

}


void BaseServer::monitor(){

  int i;
  char cmd, line[255],tag[LINE_ARG_MAX][20];
  int largc;
  char *largv[LINE_ARG_MAX];
  bool running=true;


  // Initialize variables for parsing cmd + arguments
  strcpy(tag[0],"");
  for (i=0; i< LINE_ARG_MAX;i++)
      largv[i]=tag[i];


  // quasi command line interpreter
  while(running) {

      if (fout) fprintf(fout, "\ns#>");
      fgets(line,255,stdin);
      largc=sscanf(line,"%s%s%s%s",tag[0],tag[1],tag[2],tag[3]);


      if((cmd=tolower(largv[0][0]))=='-')
      cmd=tolower(largv[0][1]);
      //printf("%c largc=%d\n",cmd,largc);

      switch(cmd) {

/*
        case 's': // shutdown
          // The process handling is not included in the class,
          // the monitor won't be able to perform init or shutdown!!!
          printf("Shutdown server\n");
          //kill_server();
          break;

        case 'i': // init server
          // The process handling is not included in the class,
          // the monitor won't be able to perform init or shutdown!!!
          printf("Init Server\n");
          //init_server();
          break;
*/

        case 'q': // quit
          if (fout) fprintf(fout, "Exit\n");
          running=false;
          break;

        case 'v': // version
          displayVersion();
          break;

      }
  }
}



void BaseServer::display(){
  show();
}


void BaseServer::show(){
  if (fout) fprintf(fout,"BaseServer\n");
}






