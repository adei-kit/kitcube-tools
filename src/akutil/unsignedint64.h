/***************************************************************************
    unsignedint64.h  -  description

    begin                : Thu Nov 13 2003
    copyright            : (C) 2003 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef UNSIGNEDINT64_H
#define UNSIGNEDINT64_H


/** 64bit data type.
  *
  * @todo Implement operators to make the handling comparable to
  *       standard data types
  */

class UnsignedInt64 {

public:
	UnsignedInt64();
	~UnsignedInt64();

  /** Add a 64 bit value to another */
  void add(unsigned long *sum1, unsigned long *sum2);

  /** Add a 64 bit value to another */
  void add(unsigned long *sum1, unsigned long sum2);

  /** Multiply two 64 bit values */
  void mult(unsigned long *prod1, unsigned long *prod2);

  /** Multiply a 64bit value with a 16bit value */
  void mult(unsigned long *prod1, unsigned short prod2);

  /** Divide a 64bit value by a 32bit value */
  void div(unsigned long *div1, unsigned short div2);

  /** Calculate modulo operations on a 64 number  */
  unsigned short mod(unsigned long *div1, unsigned short div2);

  /** Test the 64bit operations */
  int test(int level=0);

  /** Get pointer to the data */
  unsigned long *get();

  //UnsignedInt64& operator=(UnsignedInt64& data);

private:

  /** 64bit data */
  unsigned long data[2];

};

#endif
