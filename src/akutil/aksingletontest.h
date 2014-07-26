/***************************************************************************
    aksingletontest.h  -  description

    begin                : Thu Feb 21 2002
    copyright            : (C) 2002 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef AKSINGLETONTEST_H
#define AKSINGLETONTEST_H

#include <akutil/aksingleton.h>
#include <akutil/aksingletoncleaner.h>


/** A test singleton!
  *
  * @ingroup akutil_group
  */

class akSingletonTest : public akSingleton  {

protected:
	akSingletonTest();

public:
	~akSingletonTest(){
     printf("akSingletonTest: Delete\n");
     cleaner.setObject( 0 );
  }

  static akSingletonTest *getReference();


  void dump(){
     printf("akSingletonTest: \n");
  }


private:
  static akSingletonTest instance;

  static akSingletonCleaner cleaner;


};

#endif
