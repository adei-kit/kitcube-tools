/***************************************************************************

    keyboard.h  -  description



    begin                : Mon Jul 31 2000

    copyright            : (C) 2000 by Andreas Kopmann

    email                : kopmann@hpe.fzk.de

    status               :

    test                 :

    history              :

 ***************************************************************************/



#ifndef _INC_KEYBOARD_39253AB201E3_INCLUDED
#define _INC_KEYBOARD_39253AB201E3_INCLUDED



#ifdef __GNUC__
# include <cstdio>
# include <unistd.h>
# include <termios.h>
#else
# include <conio.h>
#endif // __GNUC__





/** Emulates the kbhit() function known from
  *
  * the windows world
  *
  * @ingroup akutil_group
  */

//##ModelId=3991220101D7

class keyboard {

public:

	//##ModelId=3991220101E4

  keyboard();

	//##ModelId=3991220101E3

  ~keyboard();



	//##ModelId=3991220101E2

  inline bool hit();

  /** Returns true if the right key */
  inline bool hit(char key);

	//##ModelId=3991220101E1

  inline char getchar();

  /** Read a password from the terminal */
  inline char *getpasswd();


  /** Character string that contains the last password entered */
  char passwd[20];
  
private:


#ifdef __GNUC__


	//##ModelId=399125540343

  struct termios orig;



	//##ModelId=399125540339

  struct termios newt;

	//##ModelId=3991220101D8

  int peek;

#else

  char key;

#endif



};





inline keyboard::keyboard()

{



#ifdef __GNUC__

  peek = -1;



  tcgetattr(0, &orig);

  newt = orig;

  newt.c_lflag &= ~ICANON;

  newt.c_lflag &= ~ECHO;

  newt.c_lflag &= ~ISIG;

  newt.c_cc[VMIN] = 1;

  newt.c_cc[VTIME] = 0;

  tcsetattr(0, TCSANOW, &newt);

#endif



}



inline keyboard::~keyboard()

{

#ifdef __GNUC__

  tcsetattr(0,TCSANOW, &orig);

#endif

}



inline bool keyboard::hit()

{



#ifdef __GNUC__


  char ch;
  int nread;



  if(peek != -1) return 1;

  newt.c_cc[VMIN]=0;

  tcsetattr(0, TCSANOW, &newt);

  nread = read(0,&ch,1);

  newt.c_cc[VMIN]=1;

  tcsetattr(0, TCSANOW, &newt);



  if(nread == 1) {

   peek = ch;

   return 1;

  }



#else
  
  //_kbhit(); // ??? Where is the difference
  if ( _kbhit() ) {
	  key = _getch(); 
	  return 1;
  }


#endif



  return 0;


}



inline bool keyboard::hit(char key){
  if (hit())
    return( getchar() == key);
  else
    return(false);
}



inline char keyboard::getchar()

{

#ifdef __GNUC__

  char ch;



  if(peek != -1) {

    ch = peek;

    peek = -1;

    return ch;

  }



  read(0,&ch,1);

  return ch;

#else
  return(key);

#endif



}


inline char * keyboard::getpasswd(){
  char ch;
  int i;
  
  // Clear password
  fflush(stdout);  
  ch = 0;
  i = 0;
  while ((ch != '\n') && (ch != '\r') && (i<20)){
    if (hit()){
      ch = getchar();
      switch (ch){
        case '\n':
        case '\r':
          break;
        default:
          passwd[i] = ch;
          printf("*"); fflush(stdout);
          i++;
          break;
      }    
    }    
  }
    
  passwd[i] = 0;
  printf("\n");

  return(passwd);
  
}  




#endif

