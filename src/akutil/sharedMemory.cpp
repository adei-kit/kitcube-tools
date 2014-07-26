/***************************************************************************
    sharedMemory.cpp  -  description

    begin                : Thu Jun 29 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/



#include <cstdlib>

#include "sharedMemory.h"

#define SHMMAX 4048 * 1024 // missing in header ?!

//#define debug

sharedMemory::sharedMemory(int size, const char *dir) {
   key_t key;
   int sSize;

   // There is a maximal possible size of a shared mem segment
   if (size > SHMMAX)
     sSize = SHMMAX;
   else
     sSize = size;

   // Create unique key via call to ftok()
   if (dir > 0)
     key = ftok(dir, 'S');
   else   
     key = ftok("/", 'S');

      
#ifdef debug
   fprintf(stdout,"Attempting to create new sharedMemory of size %d Bytes, key =%d, dir=%s\n",
           sSize,key,dir);
#endif
   // Open the shared memory segment - create if necessary
   if((shmid = shmget(key, sSize, IPC_CREAT|IPC_EXCL|0666)) == -1){
#ifdef debug
      perror("shmget");
      printf("Shared memory segment exists - opening as client\n");
#endif
      // Segment probably already exists - try as a client
      if((shmid =  shmget(key, sSize, 0)) == -1){
         perror("shmget");
         exit(1);
      }
   } else {
#ifdef debug
      printf("Creating new shared memory segment\n");
#endif
   }

#ifdef debug
   printf("Attach new shared memory segment to the current process\n");
#endif
   
   // Attach (map) the shared memory segment into the current process
   if((shmptr = (char *) shmat(shmid, 0, 0)) == (char *) -1) {
      perror("shmat");
      exit(1);
   }
}


void sharedMemory::remove() {
   shmctl(shmid, IPC_RMID, 0);
   printf("Shared memory segment marked for deletion\n");

   return;
}



char *sharedMemory::getReference() {
   return(shmptr);
}


int sharedMemory::getSize(){
    struct shmid_ds parm;

    shmctl(shmid,IPC_STAT,&parm);

    return(parm.shm_segsz);
}
