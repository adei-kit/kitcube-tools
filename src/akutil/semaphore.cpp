/***************************************************************************
    semaphore.cpp  -  description

    begin                : Thu Jun 29 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#include <cstdlib>
#include <exception>

#include "semaphore.h"

using namespace std;

/*
semaphore::semaphore() {
#ifdef USE_SYSV_SYNC
   init(0,getenv("HOME") );
#endif

#ifdef USE_MFC_SYNC
#endif
}

semaphore::semaphore(int mid) {
#ifdef USE_SYSV_SYNC
   init(mid,getenv("HOME") );
#endif

#ifdef USE_MFC_SYNC
#endif
}
*/

semaphore::semaphore(int mid, const char *dir) {
   this->memid = mid;

#ifdef USE_SYSV_SYNC
   if (dir==0) 
     init(mid,getenv("HOME"));
   else
     init(mid,dir);
#endif

#ifdef USE_MFC_SYNC
   mutex0 = new CMutex();
   mutex1 = new CMutex();

   lock0 = new CSingleLock(mutex0);
   lock1 = new CSingleLock(mutex1);
#endif
}

#ifdef USE_SYSV_SYNC
void semaphore::init(int, const char *dir){
   // int mid
  
   //key_t key;
   union semun semopts;
   int i;

   //this->memid = mid;

   // Create unique key via call to ftok()
   key = ftok(dir, 's');
   if (key == -1) {
     fprintf(stderr,"Warning: Creating SysV key failed (key = %s)!\n", dir);
   }

#ifdef SEM_DEBUG
   fprintf(stdout,"Attempting to create new semaphore set with %d members\n",
		SEM_NUMBER_MAX);
   fprintf(stdout,"Base semaphore is %d, dir=%s\n",this->memid,dir);
#endif
   if((this->sid = semget(key, SEM_NUMBER_MAX , IPC_CREAT|IPC_EXCL|0666)) == -1) {
#ifdef SEM_DEBUG
      fprintf(stderr, "Semaphore set already exists!\n");
#endif
      this->sid = semget(key, 0, 0666);
   } else {

      semopts.val = SEM_RESOURCE_MAX;
      // Initialize all members (could be done with SETALL)
      for(i=0; i< SEM_NUMBER_MAX ; i++)
         semctl(sid, i, SETVAL, semopts);

   }

}
#endif

/*
semaphore::~semaphore() {

   // Do not remove the semaphore here! Other processes may
   // want to use it!!!
   // But perhaps it is nice to release the sema ?!
   // If the object ends to exist it can not block any
   // critical path?! But if the object does not block at the
   // the object is removed, it will bother other objects using
   // the same semaphore!!! Don't do anything with the semaphore!
   //
   //this->release();

   //semctl(sid, 0, IPC_RMID, 0);
   fprintf(stdout,"Don't need Semaphore any more\n");

} 

*/

/*
void semaphore::request() {
#ifdef USE_SYSV_SYNC
   struct sembuf sem_op={ memid, -1, 0};

   //request the semaphore
#ifdef SEM_DEBUG
   fprintf(stdout,"Request sema #%d\n",sem_op.sem_num);
#endif
   semop(sid, &sem_op, 1);
#endif
}
*/


void semaphore::request(int nr) {
#ifdef USE_SYSV_SYNC
   struct sembuf sem_op={ memid+nr, -1, 0};

   //request the semaphore
#ifdef SEM_DEBUG
   fprintf(stdout,"Request sema #%d",sem_op.sem_num);
#endif
   semop(sid, &sem_op, 1);
#endif
#ifdef SEM_DEBUG
   fprintf(stdout," done\n");
#endif

#ifdef USE_MFC_SYNC
   printf(">");
   if (memid+nr  == 0){
     printf("0");
     lock0->Lock();
   }
   if (memid +nr == 1){
	   printf("1");
	   lock1->Lock();
   }
#endif

}

/*
void semaphore::release() {
#ifdef USE_SYSV_SYNC
   struct sembuf sem_op={ memid, 1, 0};

   //release the semaphore
#ifdef SEM_DEBUG
   fprintf(stdout,"Release sema #%d\n",sem_op.sem_num);
#endif
   semop(sid, &sem_op, 1);
#endif
}
*/

void semaphore::release(int nr) {
#ifdef USE_SYSV_SYNC
   struct sembuf sem_op={ memid+nr, 1, 0};

   //release the semaphore
#ifdef SEM_DEBUG
   fprintf(stdout,"Release sema #%d\n",sem_op.sem_num);
#endif
   semop(sid, &sem_op, 1);
#endif

#ifdef USE_MFC_SYNC
   if (memid+nr  == 0)
     lock0->Unlock();
   if (memid +nr == 1)
	 lock1->Unlock();
#endif
}


int  semaphore::number(){
#ifdef USE_SYSV_SYNC
   //int rc;
   struct semid_ds mysemds;
   union semun semopts;

   // Get current values for internal data structure
   semopts.buf = &mysemds;
/*   if((rc = semctl(sid, 0, IPC_STAT, semopts)) == -1) {
      perror("semctl");
      exit(1);
   } */

   // return number of semaphores in set
   return(semopts.buf->sem_nsems);
#endif

#ifdef USE_MFC_SYNC
   return (2); // There are two CMutex variables defined!
#endif

   return (0);
}

#ifdef USE_SYSV_SYNC
key_t semaphore::getKey(){
  return(this->key);
}
#endif

#ifdef USE_SYSV_SYNC
// ================= semaControl =================


semaControl::semaControl() {
  //init("~");
  init(getenv("HOME"));
}

semaControl::semaControl(const char *dir) {

   if (dir==0)
     init(getenv("HOME"));
   else
     init(dir);

   //init(dir);
}

void semaControl::init(const char *dir){

   // Create SysV key, Project 's' = Semaphores
   key = ftok(dir, 's');
   //printf("Key = %8x\n",key);
   if  (key == -1) // error!
     throw invalid_argument("ftok - wrong key?!");
}

void semaControl::open(){
   // Open existing Semaphore array
   sid = semget(key, 1 , 0666);
   if(sid == -1)
      throw invalid_argument("Semaphore not found");
}

int  semaControl::number(){
   int rc;
   struct semid_ds mysemds;
   union semun semopts;

   // Get current values for internal data structure
   semopts.buf = &mysemds;
   rc = semctl(sid, 0, IPC_STAT, semopts);
   if(rc == -1)
      throw invalid_argument("semctl");

   // return number of semaphores in set
   return(semopts.buf->sem_nsems);
}


int semaControl::value(int nr){
  int rc;

  rc = semctl(sid, nr, GETVAL, 0);
  if (rc == -1)
    throw invalid_argument("semctl");

  return( rc );
}


void semaControl::markForDelete(){
  int rc;

  rc =  semctl(sid, 0, IPC_RMID, 0);
  if (rc == -1)
    throw invalid_argument("semctl");

}


void semaControl::lock(int nr){
   struct sembuf sem_lock={ 0, -1, IPC_NOWAIT};

   if( (nr<0) || (nr>(number()-1)) )
      throw invalid_argument("Semaphore member out of range");

   // Attempt to lock the semaphore set
   if(!value(nr))
      throw invalid_argument("Semaphore resources exhausted (no lock)!");

   sem_lock.sem_num = nr;
   if((semop(sid, &sem_lock, 1)) == -1)
      throw invalid_argument("Lock failed");

}

void semaControl::unlock(int nr){
   struct sembuf sem_unlock={ nr, 1, IPC_NOWAIT};
   int semval;

   if( nr<0 || nr>(number()-1))
      throw invalid_argument ("Semaphore member %d out of range");

   // Is the semaphore set locked?
   semval = value(nr);
   if(semval == SEM_RESOURCE_MAX)
      throw invalid_argument("Semaphore not locked!\n");

   sem_unlock.sem_num = nr;
   // Attempt to lock the semaphore set
   if((semop(sid, &sem_unlock, 1)) == -1)
      throw invalid_argument("Unlock failed");

}


void semaControl::create(int members){
   int cntr;
   union semun semopts;
   int sid;

   //printf("Key=%8x\n",key);

   if((sid = semget(key, members, IPC_CREAT|IPC_EXCL|0666)) == -1) {
      throw invalid_argument("Semaphore set already exists!");
   }
   this->sid = sid; // copy to member, if successful

   semopts.val = SEM_RESOURCE_MAX;
   // Initialize all members (could be done with SETALL)
   for(cntr=0; cntr<members; cntr++)
      semctl(sid, cntr, SETVAL, semopts);

}

#ifdef USE_SYSV_SYNC
key_t semaControl::getKey(){
  return(this->key);
}
#endif


#endif

