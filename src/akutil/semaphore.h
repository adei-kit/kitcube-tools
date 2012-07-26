/***************************************************************************
    semaphore.h  -  description

    begin                : Thu Jun 29 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/




#ifndef _INC_SEMAPHORE_1234_INCLUDED
#define _INC_SEMAPHORE_1234_INCLUDED

//#define SEM_DEBUG // turn on debug information

#ifdef __GNUC__
#define USE_SYSV_SYNC
#endif

// Do not realy work for Windows
// Use the appropriate MFC commands directly!!!
//#define USE_MFC_SYNC

#include <cstdio>
#include <stdexcept>

#ifdef USE_SYSV_SYNC
// Semaphore für Linux
// Sys V style
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifdef __linux__
//#include <linux/sem.h>
// from linux/sem.h  don't know how to handle it. You can't invoke
// both sys/sem.h and linux/sem.h !?

/* arg for semctl system calls. */
union semun {
	int val;			/* value for SETVAL */
	struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
	unsigned short *array;		/* array for GETALL & SETALL */
	struct seminfo *__buf;		/* buffer for IPC_INFO */
	void *__pad;
};
#endif

#define SEM_NUMBER_MAX 16  // Number of Semas in the array
#define SEM_RESOURCE_MAX 1 //Initial value of all semaphores
#endif

#ifdef USE_MFC_SYNC
#include <afxwin.h>
#include <afxmt.h>
#endif

/** The class provides semaphores.
  * With Linux the SysV style semaphores are used.
  *
  * The function of the semaphores is crucial to all applications
  * relying on Pbus-access. To maintain the state of the semaphores
  * use the program "semtool" or the system commands "ipcs" and "ipcrm".
  *
  * List the state of the semphores.
@verbatim
semtool t

@endverbatim
  *
  * After an abnormal program end it can happen, that semapohres stay
  * locked. Any access to the electronic will be impossible in this state.
  * Use "semtool" to clear the state of the semaphores.
@verbatim
@endverbatim
  *
  *
  * @todo Would be nice to have named semaphores
  * with a automatic numbering?!
  * @todo There is another definition of semaphores
  * in semaphore.h. Is this useful?
  * @todo GUI programm for Windows NT also need
  * protection, when multiple threads are used!
  * Try to adapt a version with the MFC Semaphores.
  *
  * @todo Add optional argument to the constructor to
  *   define a message stream.
  *
  * @ingroup akutil_group
  */
class semaphore
{
public:
   //semaphore();
   //semaphore(int mid);
   /** Creates a Semaphore array.
     * @param mid Number of the (first) Semaphore in the array
     * @param dir Name of the base directory
     */
   semaphore(int mid=0, const char *dir="/");

   //void request();
   //void release();
   void request(int nr=0);
   void release(int nr=0);

   /** Returns the number of semaphores in the
     * array */
   int number();

   /** Returns the key of the semphore array
     * This is the number that can be found in the display of the
     * system command "ipcs".
     */
#ifdef USE_SYSV_SYNC
   key_t getKey();
#endif

private:

   int memid;

#ifdef USE_SYSV_SYNC
   key_t key;

   int sid;
   int max;

   void init(int mid, const char *dir);
#endif

#ifdef USE_MFC_SYNC
   CMutex *mutex0;
   CMutex *mutex1;

   CSingleLock *lock0;
   CSingleLock *lock1;
#endif

};

#ifdef USE_SYSV_SYNC
/** The class semaControl is intended to
  * maintain the state of the semaphore arrays.
  * Every instance contains the key information
  * of the array. (Calculated form a directory
  * name and the project character 's' for semaphore,
  * see SysV key!). The instance not necessarily
  * contains a valid semaphore identifier (sid) -
  * before any other command use open() or create().
  *
  * To supervise the semaphores form system level
  * use the OS commands ipcs, ipcrm.
  *
  * @todo How can I get a list of all known semaphores?
  */
class semaControl
{
public:
   semaControl();

   /** Creates a Semaphore array.
     * @param dir Name of the base directory
     */
   semaControl(const char *dir);

   /** Returns the number of semaphores in the
     * array */
   int number();

   /** Returns the value of a semaphore.
     * Have always in mind, that in a running system it is
     * useless to read the value of a semaphore.
     * The information is only valid for the
     * time the value is read. You can
     * base any decicions on tis value!
     * Use request() instead. */
   int value(int nr);

   /** Mark the semaphore for delete.
     * Do not use this command on a semaphore
     * array another process may depend on.
     */
   void markForDelete();


   /** Open semaphore pointer to an existing semaphore */
   void open();

   /** Create a new array of semaphores */
   void create(int members);

   /** Lock a semphore.
     *
     * WARNING: The method will modify a semaphore
     * independantly of the access rights
     */
   void lock(int nr);

   /** Unlock a semphore.
     *
     * WARNING: The method will modify a semaphore
     * independantly of the access rights
     */
   void unlock(int nr);

   /** Returns the key of the semphore array
     * This is the number that can be found in the display of the
     * system command "ipcs".
     */
#ifdef USE_SYSV_SYNC
   key_t getKey();
#endif

private:
   int sid;
   int memid;
   int max;

   key_t key;

   void init(const char *dir);
};
#endif

#endif


