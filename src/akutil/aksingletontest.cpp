/***************************************************************************
    aksingletontest.cpp  -  description

    begin                : Thu Feb 21 2002
    copyright            : (C) 2002 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#include "aksingletontest.h"


/** Attention: cleaner has to be defined before instance !!! */
akSingletonCleaner akSingletonTest::cleaner;

akSingletonTest akSingletonTest::instance;



akSingletonTest *akSingletonTest::getReference(){

  cleaner.dump();

  return ( &instance );

}


akSingletonTest::akSingletonTest(){

  // Clean up at the program end to avoid memory holes ?!
  cleaner.setObject( this );

  cleaner.dump();

}

