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
#include <stdarg.h>

#include "wstring.h"

#define PADSIZE 32  // include a little padding on alloc for future growth

Wstring::Wstring() : str(NULL), strsize(0)
{ }

Wstring::Wstring(const char *string):str(NULL), strsize(0)
{ set(string); }

Wstring::Wstring(const Wstring &other):str(NULL), strsize(0)
{
  if (other.str!=NULL)
  {
    str=new char[strlen(other.str)+PADSIZE+1];
    strsize=strlen(other.str)+PADSIZE+1;
    strcpy(str,other.str);
  }
}

Wstring::~Wstring()
{
  clear();
}

bool Wstring::operator<(const Wstring &other) const
{
	if (str == NULL && other.str == NULL)
		return false;

	if (str == NULL)
		return true;

	return ( strcmp(str, other.str) < 0 );
}

bit8 Wstring::operator==(const char *other) const
{
  if ((str==NULL)&&(other==NULL))
    return(TRUE);
  if(strcmp(str, other) != 0)
    return(FALSE);
  else
   return(TRUE);
}

bit8 Wstring::operator==(const Wstring &other) const
{
 if((str == NULL) && (other.str == NULL))
   return(TRUE);

 if((str == NULL) || (other.str == NULL))
   return(FALSE);

 if(strcmp(str, other.str) != 0)
   return(FALSE);
 else
   return(TRUE);
}


bit8 Wstring::operator!=(const char *other) const
{
 if(strcmp(str, other) != 0)
   return(TRUE);
 else
   return(FALSE);
}


bit8 Wstring::operator!=(const Wstring &other) const
{
 if((str == NULL) && (other.str == NULL))
   return(FALSE);

 if((str == NULL) || (other.str == NULL))
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
  uint32   len;

  if (s==NULL)   // it's OK to cat nothing
    return(TRUE);

  // Determine the length of the resultant string.
  len = strlen(s) + 1;
  if(str)
    len += strlen(str);

  // Space check
  strgrow(len);

  strcat(str, s);

  return(TRUE);
}


bit8 Wstring::cat(uint32 size, const char *s)
{
  uint32   len;

  // Determine the length of the resultant string.
  len = size + 1;
  if(str)
    len += strlen(str);

  // Allocate memory for the new string.
  strgrow(len);

  strncat(str, s, size);
  str[len-1]=0;   // make sure null term'd

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
  //char    *s;
  sint32   len;

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

  memmove(str+pos,str+pos+count,len-pos-count+1);

  return(TRUE);
}

// Remove all instances of a char from the string
bit8 Wstring::removeChar(char c)
{
  int     len=0;
  char   *cptr=NULL;
  bit8    removed=FALSE;

  if (str==NULL)
    return(FALSE);

  len=strlen(str);
  while ((cptr=strchr(str,c)) !=NULL)
  {
    memmove(cptr,cptr+1,len-1-((int)(cptr-str)));
    len--;
    str[len]=0;
    removed=TRUE;
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
 if(str)
   delete[](str);
 strsize=0;
 str=NULL;
}

// This is usually used for raw storage instead of string ops...
void Wstring::setSize(sint32 size)
{
  clear();
  if (size<0)
    return;

  str=new char[size];
  strsize=size;
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
  if(str == NULL)
    return(0);
  return((uint32)strlen(str));
}


// Insert at given position and shift old stuff to right
bit8 Wstring::insert(const char *instring, uint32 pos)
{
  if (str==NULL)
    return(set(instring));
  if (pos>strlen(str))
    pos=strlen(str);

  strgrow(strlen(str)+strlen(instring)+1);
  memmove(str+pos+strlen(instring),str+pos,strlen(str)-pos+1);
  memmove(str+pos,instring,strlen(instring));

  return(TRUE);
}

// This function inserts the character specified by `k' into the string at the
// position indexed by `pos'.  If `pos' is >= the length of the string, it is
// appended to the string.  If an error occurs, FALSE is returned.  Otherwise,
// TRUE is returned.
bit8 Wstring::insert(char k, uint32 pos)
{
  char temp[2];
  temp[0]=k;
  temp[1]=0;
  return(insert(temp,pos));
}


// Joe Howes (05/19/2000):  This function inserts commas to nicely format a
// large number (i.e.  1234567890 -> 1,234,567,890).  It doesn't really care
// if the string is really a number or not.
bit8 Wstring::beautifyNumber()
{
	int len = length();
	int accum = 3 - (len % 3);
	int numcommas = 0;


        if( accum == 3 )  accum = -1;
        for(int i = 0; i < len; i++)
        {
		if( accum == 3 )
		{
			insert(',', i + numcommas);
			numcommas++;
		}
		accum = ( accum == 3 || accum == -1 ) ? 1 : accum + 1;
        }

	return(TRUE);
}


// This function replaces any occurences of the string pointed to by
// `replaceThis' with the string pointed to by `withThis'.  If an error
// occurs, FALSE is returned.  Otherwise, TRUE is returned.
bit8 Wstring::replace(const char *replaceThis,const char *withThis)
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

      src=NULL;
    }
  }
  return(set(dest.get()));
}


bit8 Wstring::set(const char *s)
{
 //uint32 len;

 strgrow(strlen(s)+1);
 strcpy(str,s);

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
 //uint32 len;

 strgrow(size+1);
 strncpy(str,string,size);
 str[size]=0;

 return(TRUE);
}


// Added by Joe Howes.  Takes a printf formatted string and a set of args.
// The expanded string must not exceed 1k or twice the length of the format
// string, whichever is larger.  It would probably be better to traverse
// the format string and properly calculate, the length so this will
// work in all cases, but this should be good enough for 99% of Wstring usage.
char Wstring::setFormatted(const char *msg, ...)
{
	if( msg == NULL || strlen(msg) <= 0 )  return FALSE;

	char*  string;
        va_list args;
	int len = (strlen(msg) < 1024) ? 1024 : (strlen(msg)*2);

	string = new char[len];
        va_start(args, msg);
        vsprintf(string, msg, args);
        va_end(args);
	set(string);
	delete[] string;
	return(TRUE);
}


// This function converts all alphabetical characters in the string to lower
// case.
void Wstring::toLower(void)
{
  uint32 i;
  int    strlength=length();

  for(i = 0; i < (uint32)strlength; i++)
  {
    if((str[i] >= 'A') && (str[i] <= 'Z'))
      str[i] = (sint8)tolower(str[i]);
  }
}


// This function converts all alphabetical characters in the string to upper
// case.
void Wstring::toUpper(void)
{
  uint32 i;
  int    strlength=length();

  for(i = 0; i < (uint32)strlength; i++)
  {
    if((str[i] >= 'a') && (str[i] <= 'z'))
      str[i] = (sint8)toupper(str[i]);
  }
}


//  This function truncates the string so its length will match the specified
// `len'.  If an error occurs, FALSE is returned.  Otherwise, TRUE is returned.
bit8 Wstring::truncate(uint32 len)
{
  strgrow(len+1);
  str[len]=0;
  return(TRUE);
}

// Truncate the string after the character 'c' (gets rid of 'c' as well)
//   Do nothing if 'c' isn't in the string
bit8 Wstring::truncate(char c)
{
  sint32  len;

  if (str==NULL)
    return(FALSE);

  char   *cptr=strchr(str,c);
  if (cptr==NULL)
    return(FALSE);
  len=(sint32)(cptr-str);
  truncate((uint32)len);
  return(TRUE);
}

// Get a token from this string that's seperated by one or more
//  chars from the 'delim' string , start at offset & return offset
sint32 Wstring::getToken(int offset,const char *delim,Wstring &out) const
{
  int i;
  sint32 start;
  sint32 stop;

  if (offset<0)  // check for bad input
    return(-1);

  for (i=offset; i<(int)length(); i++) {
    if(strchr(delim,str[i])==NULL)
      break;
  }
  if (i>=(int)length())
    return(-1);
  start=i;

  for (; i<(int)length(); i++) {
    if(strchr(delim,str[i])!=NULL)
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
  if (start >= (sint32)length())
    return(-1);

  for (; i<(int)length(); i++) {
    if(strchr("\r\n",str[i])!=NULL)
      break;
  }
  stop=i;
  if ((str[stop]=='\r')&&(str[stop+1]=='\n'))
    stop++;

  out.set(str+start);
  out.truncate((uint32)stop-start+1);
  return(stop+1);
}

//
// Make sure there's AT LEAST length bytes in this string object
//
void Wstring::strgrow(int length)
{
  if (str==NULL)
  {
    str=new char[length+PADSIZE];
    str[0]=0;
    strsize=length+PADSIZE;
    return;
  }
  else if (strsize >= length)   // no need to alloc more data
    return;
  else    // bah, gotta grow...
  {
    char *newstr=new char[length+PADSIZE];
    strsize=length+PADSIZE;
    strcpy(newstr,str);
    delete[](str);
    str=newstr;
    return;
  }
}
