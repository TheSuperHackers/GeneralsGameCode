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
File Name   : configfile.h
Author      : Neal Kettler
Start Date  : June 9, 1997
Last Update : June 17, 1997
\***************************************************************************/

#pragma once

#include "dictionary.h"
#include "wstring.h"

class ConfigFile
{
 public:
             ConfigFile();
            ~ConfigFile();
 bit8        readFile(IN FILE *config);
 bit8        getString(IN Wstring &key,OUT Wstring &value);
 bit8        getString(IN const char *key,OUT Wstring &value);

 bit8        getInt(IN Wstring &key,OUT sint32 &value);
 bit8        getInt(IN const char *key,OUT sint32 &value);

 bit8        getInt(IN Wstring &key,OUT sint16 &value);
 bit8        getInt(IN const char *key,OUT sint16 &value);

 private:
  Dictionary<Wstring,Wstring> dictionary; // stores the mappings from keys
                                        //  to value strings
};
