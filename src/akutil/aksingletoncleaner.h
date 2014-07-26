/***************************************************************************
    aksingletoncleaner.h  -  description

    begin                : Thu Feb 21 2002
    copyright            : (C) 2002 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef AKSINGLETONCLEANER_H
#define AKSINGLETONCLEANER_H

#include <cstdio>

#include <akutil/aksingleton.h>


/** Avoid memory leakage through instances of
  * singletons.
  *
  * @ingroup akutil_group
  */

class akSingletonCleaner {

public :
   /** Constructor */
   akSingletonCleaner( akSingleton * pObject = 0 ) : singleton( pObject ) { }

   /** Static objects will be deleted at the program end. So
     * every singleton class needs to contain a static oject of type
     * akSingletonCleaner and pass it own reference to this object */
   virtual ~akSingletonCleaner() {
      if( singleton != 0 ) {
          delete singleton;
      }

   }

   void setObject( akSingleton * pObject ){
      //printf("SingletonCleaner (%p): Setting singleton reference from %p to %p\n",
      //         this, singleton, pObject);
      singleton = pObject;

   }

   void dump(){
      printf("SingletonCleaner (%p): Reference to %p\n", (void *) this, (void *) singleton);
   }


   akSingleton * getObject(){
      return singleton;
   }


private :
   /** Reference of the singleton to care for */
   akSingleton *singleton;

};


#endif
