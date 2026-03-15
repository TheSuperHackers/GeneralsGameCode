/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/****************************************************************************\
*        C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S         *
******************************************************************************
Project Name: Carpenter  (The RedAlert ladder creator)
File Name   : string.cpp
Author      : Neal Kettler
Start Date  : June 1, 1997
Last Update : June 17, 1997

A fairly typical string class.  This string class always copies any input
string to it's own memory (for assignment or construction).
\***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wstring.h"

Wstring::Wstring():str(nullptr)
{ }

Wstring::Wstring(const char *string):str(nullptr)
{ set(string); }

Wstring::Wstring(const Wstring &other):str(nullptr)
{
  if (other.str!=nullptr)
  {
    str=new char[strlen(other.str)+1];
    strcpy(str,other.str);
  }
}

Wstring::~Wstring()
{ clear(); }

bit8 Wstring::operator==(const char *other)
{
  if ((str==nullptr)&&(other==nullptr))
    return(TRUE);
  if(strcmp(str, other) != 0)
    return(FALSE);
  else
   return(TRUE);
}

bit8 Wstring::operator==(const Wstring &other)
{
 if((str == nullptr) && (other.str == nullptr))
   return(TRUE);

 if((str == nullptr) || (other.str == nullptr))
   return(FALSE);

 if(strcmp(str, other.str) != 0)
   return(FALSE);
 else
   return(TRUE);
}


bit8 Wstring::operator!=(const char *other)
{
 if(strcmp(str, other) != 0)
   return(TRUE);
 else
   return(FALSE);
}


bit8 Wstring::operator!=(const Wstring &other)
{
 if((str == nullptr) && (other.str == nullptr))
   return(FALSE);

 if((str == nullptr) || (other.str == nullptr))
   return(TRUE);

 if(strcmp(str, other.str) != 0)
   return(TRUE);
 else
   return(FALSE);
}


Wstring &Wstring::operator=(const char *other)
{
  set(other);
  return(*this);
}


Wstring &Wstring::operator=(const Wstring &other)
{
 if(*this == other)
   return(*this);

 set(other.get());
 return(*this);
}


bit8 Wstring::cat(const char *s)
{
  char    *oldStr;
  uint32   len;

  if (s==nullptr)   // it's OK to cat nothing
    return(TRUE);

  // Save the contents of the string.
  oldStr = str;

  // Determine the length of the resultant string.
  len = strlen(s) + 1;
  if(oldStr)
    len += strlen(oldStr);

  // Allocate memory for the new string.

  if(!(str = new char[(len * sizeof(char))]))
  {
    str = oldStr;
    return(FALSE);
  }

  // Copy the contents of the old string and concatenate the passed
  // string.
  if(oldStr)   strcpy(str, oldStr);
  else         str[0] = 0;

  strcat(str, s);

  delete[](oldStr);

  return(TRUE);
}


bit8 Wstring::cat(uint32 size, const char *s)
{
  char    *oldStr;
  uint32   len;

  // Save the contents of the string.
  oldStr = str;

  // Determine the length of the resultant string.
  len = size + 1;
  if(oldStr)
    len += strlen(oldStr);

  // Allocate memory for the new string.
  if(!(str = new char[(len * sizeof(char))]))
  {
    str = oldStr;
    return(FALSE);
  }

  // Copy the contents of the old string and concatenate the passed
  // string.
  if(oldStr)
    strcpy(str, oldStr);
  else
    str[0] = 0;

  strncat(str, s, size);

  delete[](oldStr);

  return(TRUE);
}

bit8 Wstring::cat(const Wstring &other)
{
  return cat(other.get());
}

Wstring &Wstring::operator+=(const char *string)
{
  cat(string);
  return(*this);
}

Wstring &Wstring::operator+=(const Wstring &other)
{
  cat(other.get());
  return(*this);
}

Wstring Wstring::operator+(const char *string)
{
  Wstring temp = *this;
  temp.cat(string);
  return(temp);
}

Wstring Wstring::operator+(const Wstring &s)
{
  Wstring temp = *this;
  temp.cat(s);
  return(temp);
}

//
// This function deletes 'count' characters indexed by `pos' from the Wstring.
// If `pos'+'count' is > the length of the array, the last 'count' characters
// of the string are removed.  If an error occurs, FALSE is returned.
// Otherwise, TRUE is returned.  Note: count has a default value of 1.
//
//
char Wstring::remove(sint32 pos,sint32 count)
{
  char    *s;
  sint32   len;

  if (!str)
    return(FALSE);

  len = (sint32)strlen(str);

  if(pos+count > len)
    pos = len - count;
  if (pos < 0)
  {
    count+=pos;    // If they remove before 0, ignore up till beginning
    pos=0;
  }
  if (count<=0)
    return(FALSE);

  if(!(s = new char[len-count+1]))
  {
    //ErrorMessage(SET_EM, "Insufficient memory to modify Wstring.");
    return(FALSE);
  }

  ///////DBGMSG("Wstring::remove  POS: "<<pos<<"  LEN: "<<len);

  // put nulls on both ends of substring to be removed
  str[pos] = 0;
  str[pos+count-1] = 0;

  strcpy(s, str);
  strcat(s, &(str[pos + count]));
  delete[](str);
  str = s;

  return(TRUE);
}

// Remove all instances of a char from the string
bit8 Wstring::removeChar(char c)
{
  int     len=0;
  char   *cptr=nullptr;
  bit8    removed=FALSE;

  if (str==nullptr)
    return(FALSE);

  len=strlen(str);
  while ((cptr=strchr(str,c)) !=nullptr)
  {
    memmove(cptr,cptr+1,len-1-((int)(cptr-str)));
    len--;
    str[len]=0;
    removed=TRUE;
  }
  if (removed)
  {
    char *newStr=new char[strlen(str)+1];
    strcpy(newStr,str);
    delete[](str);
    str=newStr;
  }
  return(removed);
}

void Wstring::removeSpaces(void)
{
  removeChar(' ');
  removeChar('\t');
}

void Wstring::clear(void)
{
 delete[](str);
 str=nullptr;
}

void Wstring::setSize(sint32 size)
{
  clear();
  if (size<0)
    return;
  str=new char[size];
  memset(str,0,size);
}

void Wstring::cellCopy(char *dest, uint32 len)
{
  uint32 i;

  strncpy(dest, str, len);
  for(i = (uint32)strlen(str); i < len; i++)
    dest[i] = ' ';
  dest[len] = 0;
}

const char *Wstring::get(void) const
{
  if(!str)
    return "";
  return str;
}

char Wstring::get(uint32 index) const
{
 if(index < strlen(str))
   return str[index];
 return(0);
}

uint32 Wstring::length(void) const
{
  if(str == nullptr)
    return(0);
  return((uint32)strlen(str));
}


// Insert at given position and shift old stuff to right
bit8 Wstring::insert(const char *instring, uint32 pos)
{
  if (str==nullptr)
    return(set(instring));
  if (pos>strlen(str))
    pos=strlen(str);
  char *newstr=new char[strlen(str)+strlen(instring)+1];
  memset(newstr,0,strlen(str)+strlen(instring)+1);
  strcpy(newstr,str);
  // move the old data out of the way
  int bytes=strlen(str)+1-pos;
  memmove(newstr+pos+strlen(instring),newstr+pos,bytes);
  // move the new data into place
  memmove(newstr+pos,instring,strlen(instring));
  delete[](str);
  str=newstr;
  return(TRUE);
}

// This function inserts the character specified by `k' into the string at the
// position indexed by `pos'.  If `pos' is >= the length of the string, it is
// appended to the string.  If an error occurs, FALSE is returned.  Otherwise,
// TRUE is returned.
bit8 Wstring::insert(char k, uint32 pos)
{
  char    *s;
  uint32   len;
  char     c[2];

  if(!str)
  {
    c[0] = k;
    c[1] = 0;
    return(set(c));
  }

  len = (uint32)strlen(str);

  if(pos > len)
    pos = len;

  if(!(s = (char *)new char[(len + 2)]))
  {
    //ErrorMessage(SET_EM, "Insufficient memory to modify Wstring.");
    return(FALSE);
  }

  c[0]     = str[pos];
  str[pos] = 0;
  c[1]     = 0;
  strcpy(s, str);
  str[pos] = c[0];
  c[0] = k;
  strcat(s, c);
  strcat(s, &(str[pos]));
  delete[](str);
  str = s;

  return(TRUE);
}


// This function replaces any occurences of the string pointed to by
// `replaceThis' with the string pointed to by `withThis'.  If an error
// occurs, FALSE is returned.  Otherwise, TRUE is returned.
bit8 Wstring::replace(const char *replaceThis, const char *withThis)
{
  Wstring  dest;
  const char *foundStr, *src;
  uint32   len;

  src=get();
  while(src && src[0])
  {
    foundStr = strstr(src, replaceThis);
    if(foundStr)
    {
      len = (uint32)foundStr - (uint32)src;
      if(len)
      {
        if(!dest.cat(len, src))
          return(FALSE);
      }
      if(!dest.cat(withThis))
        return(FALSE);
      src = foundStr + strlen(replaceThis);
    }
    else
    {
      if(!dest.cat(src))
        return(FALSE);

      src=nullptr;
    }
  }
  return(set(dest.get()));
}


bit8 Wstring::set(const char *s)
{
 uint32 len;

 clear();

 len = (uint32)strlen(s) + 1;

 if(!(str = new char[len]))
 {
   //ErrorMessage(SET_EM, "Insufficient memory to set Wstring.");
   return(FALSE);
 }
 strcpy(str, s);

 return(TRUE);
}


bit8 Wstring::set(char c, uint32 index)
{
 if(index >= (uint32)strlen(str))
   return FALSE;

 str[index] = c;

 return TRUE;
}


char Wstring::set(uint32 size, const char *string)
{
 uint32 len;

 clear();
 len = size + 1;

 if(!(str = new char[len]))
 {
   //ErrorMessage(SET_EM, "Insufficient memory to set Wstring.");
   return(FALSE);
 }

 // Copy the bytes in the string, and null-terminate it.
 strncpy(str, string, size);
 str[size] = 0;

 return(TRUE);
}


// This function converts all alphabetical characters in the string to lower
// case.
void Wstring::toLower(void)
{
  uint32 i;

  for(i = 0; i < length(); i++)
  {
    if((str[i] >= 'A') && (str[i] <= 'Z'))
      str[i] = tolower(str[i]);
  }
}


// This function converts all alphabetical characters in the string to upper
// case.
void Wstring::toUpper(void)
{
  uint32 i;

  for(i = 0; i < length(); i++)
  {
    if((str[i] >= 'a') && (str[i] <= 'z'))
      str[i] = toupper(str[i]);
  }
}


//  This function truncates the string so its length will match the specified
// `len'.  If an error occurs, FALSE is returned.  Otherwise, TRUE is returned.
bit8 Wstring::truncate(uint32 len)
{
  Wstring tmp;
  if(!tmp.set(len, get()) || !set(tmp.get()))
    return(FALSE);
  return(TRUE);
}

// Truncate the string after the character 'c' (gets rid of 'c' as well)
//   Do nothing if 'c' isn't in the string
bit8 Wstring::truncate(char c)
{
  sint32  len;

  if (str==nullptr)
    return(FALSE);

  char   *cptr=strchr(str,c);
  if (cptr==nullptr)
    return(FALSE);
  len=(sint32)(cptr-str);
  truncate((uint32)len);
  return(TRUE);
}

// Get a token from this string that's separated by one or more
//  chars from the 'delim' string , start at offset & return offset
sint32 Wstring::getToken(int offset,const char *delim,Wstring &out)
{
  int i;
  sint32 start;
  sint32 stop;
  for (i=offset; i<(int)length(); i++) {
    if(strchr(delim,str[i])==nullptr)
      break;
  }
  if (i>=(int)length())
    return(-1);
  start=i;

  for (; i<(int)length(); i++) {
    if(strchr(delim,str[i])!=nullptr)
      break;
  }
  stop=i-1;
  out.set(str+start);
  out.truncate((uint32)stop-start+1);
  return(stop+1);
}

// Get the first line of text after offset.  Lines are terminated by '\r\n' or '\n'
sint32 Wstring::getLine(int offset, Wstring &out)
{
  int i;
  sint32 start;
  sint32 stop;

  start=i=offset;
  if (start >= (int)length())
    return(-1);

  for (; i<(int)length(); i++) {
    if(strchr("\r\n",str[i])!=nullptr)
      break;
  }
  stop=i;
  if ((str[stop]=='\r')&&(str[stop+1]=='\n'))
    stop++;

  out.set(str+start);
  out.truncate((uint32)stop-start+1);
  return(stop+1);
}
