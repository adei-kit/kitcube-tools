/***************************************************************************
    sharedMemory.h  -  description

    begin                : Thu Jun 29 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/



// Shared Memory für Linux
// Sys V style

#ifndef _INC_SHAREDMEMORY_1234_INCLUDED
#define _INC_SHAREDMEMORY_1234_INCLUDED

#include <cstdio>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


/**
  *
  * @ingroup akutil_group
  */
class sharedMemory
{
public:

   sharedMemory(int size,const char *dir="~");

   char *getReference();
   void remove();

   /** Returns the size of the allocated shared memory
     * segment. The size can be different from the
     * specified size in the constructor when a
     * segment with the requested key was already existing
     */
   int getSize();

private:
   int shmid;
   char *shmptr;

};

#endif
