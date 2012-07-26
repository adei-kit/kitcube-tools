/***************************************************************************
    inifile.c  -  description

    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


/*
 * File:    inifile.cc
 *
 * Author:  Th. Wiegert
 *
 * Purpose: Reading Windows(TM) like *.ini files 
 *          (derived from inifile.hxx, Rev. 1.5)
 *
 * $Id: inifile.cpp,v 1.11 2011/11/11 08:31:53 kopmann Exp $
 * $Revision: 1.11 $
 * $Date: 2011/11/11 08:31:53 $
 *
 */


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
// class Inifile
//
//
//! This module contains functions that support data handling with initiali- 
//! sation text files that are build up like MS-Windows ini-files.
//!<br>
//! Thomas Wiegert                                     27.11.95 -  10.06.98 
//!
//


#include <cstdlib>
#include <cstring>
#include <iostream>

#include <akutil/inifile.h>

using namespace std;

#define INIFILE_BAD_VALUE  127
#define LINE_BUFFER_LENGTH (INIFILE_LINE_BUFFER_SIZE - 1)


/****************************************************************************/
/*                                                                          */ 
/*!  example :                                                              */ 
/*!                                                                         */ 
/*!  Let us assume the following entry in a file named "address.ini"        */ 
/*!                                                                         */ 
/*!  [Thomas]                                                               */ 
/*!     .                                                                   */ 
/*!     .                                                                   */ 
/*!   weight= 80                                                            */ 
/*!   city= 76137 Karlsruhe                                                 */ 
/*!     .                                                                   */ 
/*!  [Helmut]                                                               */ 
/*!     .                                                                   */ 
/*!   weight= 280                                                           */ 
/*!   city= 67137 Oggersheim                                                */ 
/*!     .                                                                   */ 
/*!                                                                         */ 
/*!  If we want to get the post code and the town in the group [Thomas]     */ 
/*!  we have to call the following instructions:                            */ 
/*!                                                                         */ 
/*!  specify file:                                                          */ 
/*!   OpenIniFile ("address.ini");                                          */ 
/*!                                                                         */ 
/*!  specify group (groups are indicated by brackets []):                   */ 
/*!   SpecifyGroup ("Thomas");                                              */ 
/*!                                                                         */ 
/*!  getting first entry for "city" as an integer value                     */ 
/*!  The argument "city" only has to be passed to get the first entry.      */ 
/*!                                                                         */ 
/*!   code = GetFirstValue("city");     -> 76137                            */ 
/*!                                                                         */ 
/*!  getting next entry as a string                                         */ 
/*!                                                                         */ 
/*!   strcpy (city, GetNextString());   -> "Karlsruhe"                      */ 
/*!                                                                         */ 
/*!  closing file:                                                          */ 
/*!   CloseIniFile ();                                                      */ 
/*!                                                                         */ 
/*!  note :                                                                 */ 
/*!                                                                         */ 
/*!   An occuring '\' means skipping to next line and appending it.         */ 
/*!                                                                         */ 
/****************************************************************************/


// -------------------------------------------------------------------------

Inifile::Inifile ( char* filename )
 {
  fGrPos   = 0;
  fGroup    = NULL;
  fPtr      = NULL;
  fLineptr  = NULL;
  fStatus   = kFAIL;
  this->fFilename = NULL;

  fInifile = fopen (filename, "r");

  if ( fInifile == NULL )
   {
    char buf[1024];
    sprintf (buf, "Inifile \"%s\" could not be opened", filename ); 
    //perror (buf); // ak 22.10.01
    return;
  }

  this->fFilename = new char[strlen(filename) + 1];
  if (this->fFilename == NULL)
   {
    perror ("not enough memory for filename");
    return;
  }
  strcpy (this->fFilename, filename);
  fStatus   = kSUCCESS;
}


// -------------------------------------------------------------------------

Inifile::~Inifile ()
 {
  if (fStatus == kSUCCESS) {  // 2.8.00 ak
    fclose ( fInifile );
  }
  if ( fFilename != NULL ) delete[] fFilename;
  if ( fGroup    != NULL ) delete[] fGroup;
}

// -------------------------------------------------------------------------

char Inifile::_Digit2Value (char digit)
 {
  if ((digit >= '0') && (digit <= '9'))
    return (digit - '0');

  if ((digit >= 'A') && (digit <= 'F'))
    return (digit - 'A' + 10);

  if ((digit >= 'a') && (digit <= 'f'))
    return (digit - 'a' + 10);

  return ( INIFILE_BAD_VALUE );
}

// -------------------------------------------------------------------------

char Inifile::_Digits2Value (char** src, char* end, int base)
 {
  char digval;
  int  value = 0, maxdigits = 3;
 
  if (base == 16) maxdigits = 2;
  if ((base == 16) || (base == 8)) (*src)++;
  if ((base == 8) && (**src == 'x')) return _Digits2Value (src, end, 16);

  while ((*src < end) && maxdigits && ((digval = _Digit2Value(**src)) < base))
   {
    value *= base;
    value += digval;
    (*src)++;
    maxdigits--;
  }
  (*src)--;
 
  return ((char)(value % 256));
}

// -------------------------------------------------------------------------

char Inifile::_GetNextCharacter (char** src, char* end)
 {
  (*src)++; 
  if (*src == end) return ((char)0);
  if ((**src == '\\') || (**src == '#')) return (**src);
  if (**src == 'n') return ('\n');

  if (**src == '0') return _Digits2Value (src, end, 8);

  if ((**src > '0') && (**src <= '9'))
    return _Digits2Value (src, end, 10);

  if (**src == 'x') return _Digits2Value (src, end, 16);

  return (char)0;
}

// -------------------------------------------------------------------------

char* Inifile::_GetLine()
 {
  int bufsize = sizeof (fLinebuf) - 1; 

  char* ptr = fLinebuf;
  char *src, *dest, *end;

  ptr = fgets (fLinebuf, bufsize, fInifile);
  if (!ptr) return NULL;
 
  dest = src = ptr; 
  end = ptr + strlen (ptr);

  while (*src)
   {
    if (*src == '\\') 
     {
      char next;
      next = _GetNextCharacter( &src, end );

      if (next == (char)0)
       {
        bufsize -= (dest - ptr);
        *(ptr = src = dest) = 0;
        if (fgets (dest, bufsize, fInifile) == NULL) return fLinebuf;
        dest--; src--;
        end = ptr + strlen (ptr);
      }
      else *dest = next;
    }
    else
     {
      if (*src == '#') *dest = 0;  /* for comments */
      else *dest = *src;
    }
    
    dest++; src++;
  } 
  *dest = (char)0;
  
  return fLinebuf;
}

// -------------------------------------------------------------------------

char* Inifile::GetFirstString (char* _entry)
 {  
  if (fStatus == kFAIL) {  // 2.8.00 ak
   return NULL;
  }

  if ((fGroup == NULL) || (*fGroup == (char)0)) 
   {
    perror ("group has to be defined!!");
    return NULL;
  }
     
  fseek ( fInifile, fGrPos, SEEK_SET);
  fLineptr = _GetLine(); 
  
  while ( fLineptr && (*fLineptr != '['))
   {
    fPtr = strtok (fLineptr, "\n\t= ");
    if ((fPtr != NULL) && !strcmp(_entry, fPtr)){
      // TODO: Improve string parser ! ak - 7.2.07
      
      // Get next token of the same string
      fPtr = strtok(NULL, "\n\t= ");

      // Return next string or an empty string
      // NULL is only returned if the parameter can
      // not be found - ak 7.2.07
      if (fPtr > 0)
        return(fPtr);
      else
        // No string found - return an empty string          
        return (fLineptr + strlen(fLineptr));
    }  
    fLineptr = _GetLine();
  }
  
  return NULL; 
}

// -------------------------------------------------------------------------

int Inifile::GetFirstValue (char* _entry, result * error)
 {
  if (fStatus == kFAIL) {  // 2.8.00 ak
   *error = kFAIL;
   return 0;
  }

  if (error != NULL) *error = kFAIL;
  fPtr = GetFirstString (_entry);
  if (fPtr == NULL) {
    if ( error != NULL ) *error = kFAIL;
    return 0;
  }
 
  char* end;
  int ret;
  ret = strtol ( fPtr, &end, 0 ); 
  if ((error != NULL) && (end != fPtr)) *error = kSUCCESS;

  return ret;
}

// -------------------------------------------------------------------------

char* Inifile::GetNextString (void)
 {
  if (fStatus == kFAIL) {  // 2.8.00 ak
   return NULL;
  }

  if ((fGroup == NULL) || (*fGroup == (char)0)) 
   {
    perror ("group has to be defined!!");
    return NULL;
  }
  if (!fPtr) return NULL;  

  return (fPtr = strtok(NULL, "\n\t= "));
}

// -------------------------------------------------------------------------

int Inifile::GetNextValue (result* error)
 {
  char* ptr;

  if (fStatus == kFAIL) {  // 2.8.00 ak
   *error = kFAIL;
   return 0;
  }

  if (error != NULL) *error = kFAIL;
  ptr = GetNextString ();
  if (ptr == NULL) {
    if ( error != NULL ) *error = kFAIL;
    return 0;
  }
 
  char* end;
  int ret;
  ret = strtol ( ptr, &end, 0 );
  if ((error != NULL) && (end != ptr)) *error = kSUCCESS;
  
  return ret;
}

// -------------------------------------------------------------------------
             
void Inifile::Reset ()
 {
  fseek (fInifile, 0, SEEK_SET);
  fLineptr = fLinebuf;
}

// -------------------------------------------------------------------------

Inifile::result Inifile::SpecifyGroup (char *string)
 {
  fGrPos = 0;

  if (fStatus == kFAIL) {  // 2.8.00 ak
   return kFAIL;
  }

  if (fGroup != NULL)
   {
    delete[] fGroup;
    fGroup = NULL;
  }

  fGroup = new char[strlen(string) + 3];
  if (fGroup == NULL)
   {
    cerr << "no memory for [" << string << "]\n";
    return kFAIL;
  }

  Reset(); 
  sprintf ( fGroup, "[%s]", string); 
 
  do
   {
    if (!(fLineptr = _GetLine()))
     return kFAIL;

    fPtr = strtok (fLineptr, "\n"); 

    if (fPtr != NULL)
    fPtr = strstr (fPtr, fGroup);

    if (fPtr != NULL)
     {
      while (*fPtr && (*fPtr == ' ')) fPtr++;

      if (!strcmp (fPtr, fGroup))
       {
        fGrPos = ftell (fInifile);
        if (!(fLineptr = _GetLine()))
          return kFAIL;

        return kSUCCESS;
      }
    }
  }
  while (fLineptr);
  
  return kFAIL;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
