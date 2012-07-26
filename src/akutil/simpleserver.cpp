/***************************************************************************
    simpleserver.cpp  -  description

    begin                : Wed Oct 25 2000
    copyright            : (C) 2000 by A Kopmann
    email                : kopmann@hpe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#include <fdhwlib.h>

#include "simpleserver.h"


//#define debug           // Give debug output

#define CMD_LEN 100
#define CMD_LAST 100


SimpleServer::SimpleServer(unsigned short port):BaseServer(port){
}


int SimpleServer::read_from_client(int client){
  int i, j, err, n_data;

  uint32_t data[CMD_LEN], *arg;
  short cmdid, len, tlen, cmd_ptr;

  //printf("read_from_client: Request via socket %d\n",client);
  // read the request
  cmd_ptr = 0;
  err = readData(client,data,CMD_LEN);
  if (err <= 0 ) {
     return(-1);
  }
  tlen = err / sizeof(uint32_t);
  //printf("read_from_client: Bytes %d/ Long words %d\n",err,tlen);
  //for (i=0;i<tlen;i++)
  // printf("  %08lx",data[i]);
  //printf("\n\n");
#ifdef debug
  // Display data
  for (i=0;i<tlen;i++){
    printf(" %08x", data[i]);
  }
  printf("\n");
  
  // Host order
  //for (i=0;i<tlen;i++){
  //  printf(" %08x", ntohl(data[i]));
  //}
  //printf("\n");
  
#endif

  // Convert all data to host order
//  for (i=0;i<tlen;i++)
//    data[i] = ntohl( data[i] );   
  
  while (cmd_ptr < tlen) {
    // Parsing the first uint32_t word (command + length)
    cmdid = (unsigned short) (data[cmd_ptr] >> 16);

#ifdef  USE_DYN_BYTEORDER
    // Check the byte order
    // In case of a different order change the format of
    // all read data upto now...
    if (cmdid > CMD_LAST){      
      FD_SET(client, &change_byteorder_fd_set);
      printf("SimpleServer: CmdID = %d > %d swap endian ", cmdid, CMD_LAST);
      
      for (i=0;i<tlen;i++){
        endian_swap(data[i]);
      }

      // Re-read cmd id
      cmdid = (unsigned short) (data[cmd_ptr] >> 16);
      printf("--> cmdid = %d\n", cmdid);
    }        
#endif
   
    len =   (unsigned short) (data[cmd_ptr] & 0xffff);
   
    
    //printf("read_from_client: Cmd pointer= %d of %d, Cmd %d/ Length %d\n",
    //            cmd_ptr, tlen, cmdid, len);

    // Are all arguments included in this block?
    if (tlen < (cmd_ptr + len) ) {
      // Read the rest of the command before execution

      // The length of the complete command (incl. arguments
      // is known.)
      arg = new uint32_t[len-1];
      j = 0;
      for (i=cmd_ptr+1;i<CMD_LEN;i++) {
         	arg[j] = data[i];
          j++;
      }

      while (j < (len-1)) {
        //printf("read_from_client: Need %d more arguments\n",len- (j +1) );
        err = readData(client, arg + j, (len - (j +1)) );
        // Check, whether the command is complete now?!
        // number of data read?
        if (err > 0) {
          n_data = err / sizeof(uint32_t);
          j = j + n_data;
          //printf("read_from_client: Read %d more arguments\n",n_data);
        } else
          if (fout) fprintf(fout,"Warning: No data read\n");
      }

      //for (i=0;i<len-1;i++)
      //  printf("  %08lx",arg[i]);
      //printf("\n\n");

      executeCmd(client, cmdid, arg, len-1);

      delete[] arg;
    }
    else {
      // Execute the command
      executeCmd(client, cmdid, data + cmd_ptr + 1, len-1);
    }

    // Move the pointer to the next command
    cmd_ptr = cmd_ptr + len;
  }

  return 0;
}


void SimpleServer::executeCmd(int client, short cmd, unsigned long *arg, short n){
    int i;
    unsigned int *buf;

#ifdef debug
    printf("SimpleServer::ExecuteCmd(long): %d \n",cmd);
#endif
	
    if (sizeof(unsigned long) > sizeof(unsigned int)){
    	    
    	    buf = new unsigned int [n];
    	    for (i=0;i<n;i++) buf[i] = arg[i];
    	    executeCmd(client, cmd, buf, n);
    	    
    	    delete buf;
    } else {
    	    executeCmd(client, cmd, (unsigned int *) arg, n);
    }

    return;
}



void SimpleServer::executeCmd(int client, short cmd, uint32_t *arg, short){
  // unsigned int *arg, short n
  
  int err;
  uint32_t res[2], *buf;
  short acklen;

  // local function call
#ifdef debug
  printf("SimpleServer::ExecuteCmd(int): %d \n",cmd);
#endif
  buf = res;  // used, if the result is not longer than 2 words
  buf[0] = 0; // no error
  acklen = 1;
  try { switch(cmd) {
    case 41: // shutdown server
       shutdown = true;
       acklen=1;
       break;
		 
	  case 99: // echo
#ifdef debug		  
		  printf("Got argument %08x\n", arg[0]);
#endif		  
		  buf[1] = arg[0];
		  acklen = 2;

	  default: // Unknown command
		  buf[0] = 1;
		  acklen = 1;
		  break;
  		  
		  
  }  } catch (...) {
#ifdef debug
      printf("read_from_client: Time-out\n");
#endif
      buf[0] = 1; // Error occured
      acklen = 1;
  }

  // return the result
  err = writeData(client,buf,acklen);
#ifdef debug
  printf("read_from_client: status %ld, acklen=%d, err=%d\n",buf[0], acklen, err);
#endif
}


