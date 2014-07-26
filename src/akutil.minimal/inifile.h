/***************************************************************************
    inifile.h  -  description

    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/

/*
 * File:    inifile.hh
 *
 * Author:  Th. Wiegert
 *
 * Purpose: Header file for inifile.cc (derived from inifile.hxx, Rev. 2.1)
 *
 */


#ifndef _INIFILE_HXX_
#define _INIFILE_HXX_

#include <cstdio>

/*
#ifndef kSUCCESS
# define kSUCCESS              0
#endif

#ifndef kFAIL
# define kFAIL                -1
#endif
*/

#define INIFILE_LINE_BUFFER_SIZE 1024



/** Functions to parse a windows like
  * ini file
  *
  * Changes:
  *    8.3.01 ak: Internal preprocessor define pragmas SUCCESS and FAIL
  *       changed to kSUCCESS and KFAIL
  *
  * @todo There are warning of not initialized string functions ?!
  *
  * @ingroup akutil_group
  */

//##ModelId=39912206001B
class Inifile {
    
 public :

     /** List of possible results */
    enum result {
       kFAIL,
       kSUCCESS
    };

 
	//##ModelId=399122060061
    Inifile    (char* _filename);
	//##ModelId=39912206005A
    ~Inifile   ();

	//##ModelId=399122060059
//    inline int    Status()
      inline result Status()
     { return fStatus; }
	//##ModelId=399122060058
    inline char*  GetFilename    ()
     { return fFilename; }
	//##ModelId=399122060053
    char*         GetFirstString (char* _entry);
	//##ModelId=399122060050
    int           GetFirstValue  (char* _entry, result * error = NULL);
	//##ModelId=39912206004F
    char*         GetNextString  ();
	//##ModelId=39912206004D
    int           GetNextValue   (result * error = NULL);
	//##ModelId=39912206004A
    void          Reset          ();
	//##ModelId=399122060048
    result        SpecifyGroup   (char *string);


 protected :
	//##ModelId=399122060039
    FILE* fInifile;
	//##ModelId=399122060033
    char  fLinebuf[INIFILE_LINE_BUFFER_SIZE];
	//##ModelId=399122060032
    char* fFilename;
	//##ModelId=399122060031
    char* fGroup;
	//##ModelId=399122060030
    char* fLineptr;
	//##ModelId=39912206002F
    char* fPtr;
	//##ModelId=399122060026
    int   fGrPos;
    /** kSUCCESS indicates a existing ini-file */
	//##ModelId=399122060025
    //int   fStatus;
    result fStatus;

	//##ModelId=399122060046
    char  _Digit2Value (char digit);
	//##ModelId=39912206003E
    char  _Digits2Value (char** src, char* end, int base);
	//##ModelId=39912206003B
    char  _GetNextCharacter (char** src, char* end);
	//##ModelId=39912206003A
    char* _GetLine ();

};

#endif // _INIFILE_HXX_
