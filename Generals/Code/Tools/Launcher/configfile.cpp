/*
**	Command & Conquer Generals(tm)
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
File Name   : configfile.cpp
Author      : Neal Kettler
Start Date  : June 9, 1997
Last Update : June 17, 1997  


This class will read in a config file and store the key value pairs for
later access.  This is a fairly simple class, the config file is assumed
to be of the form:

#comment
key = value

The value can then be retrieved as a string or an integer.  The key on
the left is used for retrieval and it must be specified in uppercase
for the 'get' functions. E.g. getString("KEY",valWstring);
\***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "configfile.h"

static uint32 Wstring_Hash(Wstring &string);
static char  *Eat_Spaces(char *string);

ConfigFile::ConfigFile() : dictionary(Wstring_Hash)
{ }

ConfigFile::~ConfigFile()
{ }

// Read and parse the config file.  The key value pairs will be stored
//   for later access by the getString/getInt functions.
bit8 ConfigFile::readFile(FILE *in)
{
  char    string[256];
  Wstring  key;
  Wstring  value;
  char   *cptr;

  memset(string,0,256);
  while (fgets(string,256,in))
  {
    cptr=Eat_Spaces(string);
    if ((*cptr==0)||(*cptr=='#'))  // '#' signals a comment
      continue;
    if (strchr(cptr,'=')==NULL)   // All config entries must have a '='
      continue;
    key=cptr;
    key.truncate('=');
    key.removeSpaces();  // No spaces allowed in the key
    key.toUpper();       // make key all caps 
    cptr=Eat_Spaces(strchr(cptr,'=')+1); // Jump to after the '='
    value=cptr;
    value.truncate('\r');
    value.truncate('\n');
    dictionary.add(key,value);
  } 
  return(TRUE);
}

// Get a config entry as a string
bit8 ConfigFile::getString(Wstring &key,Wstring &value)
{
  return(dictionary.getValue(key,value));
} 

// Get a config entry as a string
bit8 ConfigFile::getString(const char *key,Wstring &value)
{
  Wstring sKey;
  sKey.set(key);
  return(getString(sKey,value));
}   

// Get a config entry as an integer
bit8 ConfigFile::getInt(Wstring &key,sint32 &value)
{
  Wstring svalue;
  bit8 retval=dictionary.getValue(key,svalue);
  if (retval==FALSE)
    return(FALSE);
  value=atol(svalue.get());
  return(TRUE);
}

// Get a config entry as an integer
bit8 ConfigFile::getInt(const char *key,sint32 &value)
{
  Wstring sKey;
  sKey.set(key);
  return(getInt(sKey,value));
}    



// Get a config entry as an integer
bit8 ConfigFile::getInt(Wstring &key,sint16 &value)
{
  Wstring svalue;
  bit8 retval=dictionary.getValue(key,svalue);
  if (retval==FALSE)
    return(FALSE);
  value=atoi(svalue.get());
  return(TRUE);
}
 
// Get a config entry as an integer
bit8 ConfigFile::getInt(const char *key,sint16 &value)
{
  Wstring sKey;
  sKey.set(key);
  return(getInt(sKey,value));
}



/************* Static functions below **************/

// Given a Wstring, return a 32 bit integer that has a good numeric
//   distributation for the purposes of indexing into a hash table.
static uint32 Wstring_Hash(Wstring &string)
{
  uint32 retval=0;
  retval=string.length();
  for (uint32 i=0; i<string.length(); i++)
  {
    retval+=*(string.get()+i);
    retval+=i;
    retval=(retval<<8)^(retval>>24);  // ROL 8
  }
  return(retval);
}         

static char *Eat_Spaces(char *string)
{
  char *retval=string;
  while (isspace(*retval))
    retval++;
  return(retval);
}
