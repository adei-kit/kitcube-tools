/***************************************************************************
    aksingleton.h  -  description

    begin                : Thu Feb 21 2002
    copyright            : (C) 2002 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef AKSINGLETON_H
#define AKSINGLETON_H


class akSingletonCleaner;

/** A singleton base class used by akSingletonCleaner
  *
  * @ingroup akutil_group
  */

class akSingleton {

public:

	akSingleton();

	~akSingleton();

};

#endif
