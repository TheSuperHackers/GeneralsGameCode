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
File Name   : main.cpp
Author      : Neal Kettler
Start Date  : June 1, 1997
Last Update : June 17, 1997
\****************************************************************************/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "wstypes.h"

class Wstring
{
 public:
           Wstring();
           Wstring(const Wstring &other);
           Wstring(const char *string);
          ~Wstring();

   void    clear(void);

   bit8    cat(const char *string);
   bit8    cat(uint32 size,const char *string);
   bit8    cat(const Wstring &string);

   void    cellCopy(OUT char *dest, uint32 len);
   char    remove(sint32 pos, sint32 count);
   bit8    removeChar(char c);
   void    removeSpaces(void);
   const char *get(void) const;
   char    get(uint32 index) const;
   uint32  length(void) const;
   bit8    insert(char c, uint32 pos);
   bit8    insert(const char *instring, uint32 pos);
   bit8    beautifyNumber();
   bit8    replace(const char *replaceThis,const char *withThis);
   char    set(const char *str);
   char    set(uint32 size,const char *str);
   bit8    set(char c, uint32 index);
   char    setFormatted(const char *str, ...);		// Added by Joe Howes
   void    setSize(sint32 bytes);  // create an empty string
   void    toLower(void);
   void    toUpper(void);
   bit8    truncate(uint32 len);
   bit8    truncate(char c);  // trunc after char c
   sint32  getToken(int offset,const char *delim,Wstring &out) const;
   sint32  getLine(int offset, Wstring &out);
   void    strgrow(int length);

   bit8    operator==(const char *other) const;
   bit8    operator==(const Wstring &other) const;
   bit8    operator!=(const char *other) const;
   bit8    operator!=(const Wstring &other) const;

   Wstring  &operator=(const char *other);
   Wstring  &operator=(const Wstring &other);
   Wstring  &operator+=(const char *other);
   Wstring  &operator+=(const Wstring &other);
   Wstring   operator+(const char *other);
   Wstring   operator+(const Wstring &other);

   bool operator<(const Wstring &other) const;

 private:
   char    *str;      // Pointer to allocated string.
   int      strsize;  // allocated data length
};
