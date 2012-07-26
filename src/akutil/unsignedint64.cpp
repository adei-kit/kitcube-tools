/***************************************************************************
    unsignedint64.cpp  -  description

    begin                : Thu Nov 13 2003
    copyright            : (C) 2003 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#include <cstdio>

#include "unsignedint64.h"

UnsignedInt64::UnsignedInt64(){
}


UnsignedInt64::~UnsignedInt64(){
}


void UnsignedInt64::add(unsigned long *sum1, unsigned long *sum2){
 int carry;


 data[0] = sum1[0] + sum2[0];

 // Detect carry
 carry = 0;
 if ((data[0] < sum1[0]) || (data[0] < sum2[0])) carry = 1;

 data[1] = sum1[1] + sum2[1] + carry;

}


void UnsignedInt64::add(unsigned long *sum1, unsigned long sum2){
 int carry;

 data[0] = sum1[0] + sum2;

 // Detect carry
 carry = 0;
 if ((data[0] < sum1[0]) || (data[0] < sum2)) carry = 1;

 data[1] = sum1[1] + carry;

}



void UnsignedInt64::mult(unsigned long *fak1, unsigned long *fak2){
  unsigned short *pFak, *pShift, *pProd;
  unsigned long shiftedProd[2];
  UnsignedInt64 prod;

  pFak = (unsigned short *) fak1;
  pShift = (unsigned short *) shiftedProd;
  pProd = (unsigned short *) prod.get();

  // Do only multiplication od 16bit x 64bit
  mult(fak2, pFak[0]);
  //data[0] = prod[0];
  //data[1] = prod[1];


  prod.mult(fak2, pFak[1]);
  pShift[0] = 0;
  pShift[1] = pProd[0];
  pShift[2] = pProd[1];
  pShift[3] = pProd[2];
  add(data, shiftedProd);

  if (pProd[3] > 0) printf("Error: Result larger than 64bit\n");


  prod.mult(fak2, pFak[2]);
  pShift[0] = 0;
  pShift[1] = 0;
  pShift[2] = pProd[0];
  pShift[3] = pProd[1];
  add(data, shiftedProd);

  if ((pProd[2] > 0) || (pProd[3] > 0)) printf("Error: Result larger than 64bit\n");


  prod.mult(fak2, pFak[3]);
  pShift[0] = 0;
  pShift[1] = 0;
  pShift[2] = 0;
  pShift[3] = pProd[0];
  add(data, shiftedProd);

  if ((pProd[1] > 0) || (pProd[2] > 0) || (pProd[3] > 0))
    printf("Error: Result larger than 64bit\n");


}


void UnsignedInt64::mult(unsigned long *fak1, unsigned short fak2){
  unsigned short *pFak, *pRes, *pProd;
  unsigned long prod;
  unsigned long *pLongRes;



  pFak = (unsigned short *) fak1;
  pRes = (unsigned short *) this->data;
  pProd = (unsigned short *) &prod;


  this->data[0] = 0;
  this->data[1] = 0;

  // Do only 16bit x 16bit multiplication
  // The result will always fit in 32bit
  prod = (unsigned long) pFak[0]  * fak2;

  pLongRes = (unsigned long *) &(pRes[0]);
  *pLongRes = prod;

  prod = (unsigned long) pFak[1]  * fak2;

  pLongRes = (unsigned long *) &(pRes[1]);
  *pLongRes = prod;

  prod = (unsigned long) pFak[2]  * fak2;

  pLongRes = (unsigned long *) &(pRes[2]);
  *pLongRes = prod;

  prod = (unsigned long) pFak[3]  * fak2;

  pRes[3] = pRes[3] + pProd[0];

  // The result is too large for 64 bit
  if (pProd[1] > 0) printf("Error: Result larger than 64bit\n");

}


void UnsignedInt64::div(unsigned long *div1, unsigned short div2){
  unsigned short *pDiv;
  unsigned long mod23, mod13, mod03;
  unsigned long div23;
  UnsignedInt64 div13, div03;
  unsigned long n16;
  unsigned long buf[2];
  unsigned short *pBuf, *pDiv13;

  n16 = 65536;  // 2^16

  pDiv = (unsigned short *) div1;
  pBuf = (unsigned short *) buf;
  pDiv13 = (unsigned short *) div13.get();

  // Calculate the result recursively
  div23 = div1[1] / div2;
  mod23 = div1[1] %div2;

  //printf(" 23: %x %lx %lx\n", 0, div23, mod23);

  // buf = div23 * n16
  pBuf[0] = 0;
  pBuf[1] = div23 << 16; // low word
  pBuf[2] = div23 >> 16; // high word
  pBuf[3] = 0;
  div13.add(buf, (mod23 * n16 + pDiv[1]) / div2);

  mod13 = ((mod23 * n16) %div2 + pDiv[1] %div2 ) %div2;

  //printf(" 13: %lx %lx %lx\n", div13[1], div13[0], mod13);

  // buf = div13 * n16
  pBuf[0] = 0;
  pBuf[1] = pDiv13[0];
  pBuf[2] = pDiv13[1];
  pBuf[3] = pDiv13[2];
  div03.add(buf, (mod13 * n16 + pDiv[0]) / div2);

  mod03 = ((mod13 * n16) %div2 + pDiv[0] %div2 ) %div2;


  //printf(" 03: %lx %lx %lx\n", div03[1], div03[0], mod03);


  data[0] = div03.get()[0];
  data[1] = div03.get()[1];

}


unsigned short UnsignedInt64::mod(unsigned long *div1, unsigned short div2){
  unsigned short *pDiv;
  unsigned long mod23, mod13, mod03;
  unsigned long n16;

  n16 = 65536;  // 2^16

  pDiv = (unsigned short *) div1;


  // Calculate the result recursively
  mod23 = div1[1] %div2;


  mod13 = ((mod23 * n16) %div2 + pDiv[1] %div2 ) %div2;


  mod03 = ((mod13 * n16) %div2 + pDiv[0] %div2 ) %div2;


  return( (unsigned short) mod03);

}


int UnsignedInt64::test(int){
  // int level

  // Test the 64bit operations
  int i;
  unsigned short nanosec, millisec;
  //unsigned long c[2];
  //unsigned long buf[2];
  UnsignedInt64 buf;

  data[0] = 0xfffffff1; data[1] = 0;
  for (i=0; i<30; i++){

    nanosec = mod(data, 10 );

    buf.div(data, 10 );
    millisec = buf.mod(buf.get(), 10);


    printf("Counter %08lx %08lx  -  %ld:%d:%d\n",
                  data[1], data[0], buf.get()[0], millisec, nanosec);

    add(get(),1);
  }


  return(0);
}


unsigned long * UnsignedInt64::get(){
  return(data);
}


/*
UnsignedInt64& operator=(UnsignedInt64& data){
  unsigned long *pData;

  pData = data.get();
  this->data[0] = pData[0];
  this->data[1] = pData[1];

  return(this);
}
*/


