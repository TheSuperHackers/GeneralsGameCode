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
#include "wdebug.h"

static uint32 Wstring_Hash(const Wstring &string);
static char  *Eat_Spaces(char *string);

ConfigFile::ConfigFile() : Dictionary_(Wstring_Hash)
{ }

ConfigFile::~ConfigFile()
{ }

// Read and parse the config file.  The key value pairs will be stored
//   for later access by the getString/getInt functions.
bit8 ConfigFile::readFile(FILE *in)
{
  char    string[256];
  char    sectionname[256];  // section name like '[user parameters]'
  Wstring  key;
  Wstring  value;
  char   *cptr;

  memset(string,0,256);
  memset(sectionname,0,256);
  sectionList.clear();

  while (fgets(string,256,in))
  {
    cptr=Eat_Spaces(string);
    if ((*cptr==0)||(*cptr=='#'))  // '#' signals a comment
      continue;

    if (*cptr=='[')  // new section
    {
      key=cptr;
      key.truncate(']');    // remove after & including the ]
      key.cat("]");         // put the ] back
      strcpy(sectionname,key.get());    // set the current section name
      Wstring wssectionname;

      if (strlen(sectionname)==2) // clear section with a "[]"
      {
        sectionname[0]=0;
        wssectionname.set("");
      }
      else
        wssectionname.set(sectionname+1);

      wssectionname.truncate(']');
      sectionList.addTail(wssectionname);


      continue;
    }

    if (strchr(cptr,'=')==NULL)   // All config entries must have a '='
      continue;
    key=cptr;
    key.truncate('=');
    key.removeSpaces();  // No spaces allowed in the key
    key.toUpper();       // make key all caps 

    // Add the section name to the end of the key 
    if (strlen(sectionname))
      key.cat(sectionname);  

    cptr=Eat_Spaces(strchr(cptr,'=')+1); // Jump to after the '='
    value=cptr;
    value.truncate('\r');
    value.truncate('\n');
    value.truncate('#');

    // TheSuperHackers @compile xezon 25/03/2025 Re-implement algorithm to avoid writing into string buffer directly.
    // Remove trailing spaces
    {
      const char* valueStr = value.get();
      const uint32 valueLen = value.length();
      uint32 spaceIdx = valueLen;
      for (; spaceIdx > 0 && isgraph(valueStr[spaceIdx - 1])==0; --spaceIdx);
      if (spaceIdx != valueLen)
        value.truncate(spaceIdx);
    }

    Critsec_.lock();
    Dictionary_.add(key,value);
    Critsec_.unlock();
  } 
  return(TRUE);
}


//
// Enum through the config strings.  To start, index & offset should be 0
// If retval is false you're done, ignore whatever's in key & value.
//
// Section specifies the configfile section.  Set to NULL if you don't care.
//
bit8 ConfigFile::enumerate(int &index, int &offset, Wstring &key, Wstring &value, IN char *section) const
{
	int seclen = strlen(section);
  while(1)
  {
    Critsec_.lock();
    if (Dictionary_.iterate(index,offset,key,value)==FALSE)   // out of keys?
    {
      Critsec_.unlock();
      return(FALSE);
    }
    Critsec_.unlock();

    if (section==NULL)  // no specified section, so any will do...
      break;

    if (strlen(section)+2 >= strlen(key.get()))  // key should have form: X[section]
      continue;

    // Is this key part of our section?
	const char *keystr = key.get() + strlen(key.get())-seclen-1;
    if (strncmp(keystr,section,strlen(section))==0)
      break;
  }
  key.truncate('[');  // remove the section name
  return(TRUE);
}



// Get a config entry as a string
bit8 ConfigFile::getString(IN Wstring &_key, Wstring &value, IN char *section) const
{
  Wstring key(_key);
  key.toUpper();

  if (section)  // append section name to key
  {
    key+="[";
    key+=section;
    key+="]";
  }

  Critsec_.lock();
  bit8 retval=Dictionary_.getValue(key,value);
  Critsec_.unlock();

  if (retval==FALSE)
  { 
	DBGMSG("Config entry missing: "<<key.get()); 
  }

  return(retval);
} 

// Get a config entry as a string
bit8 ConfigFile::getString(IN char *key,Wstring &value, IN char *section) const
{
  Wstring sKey;
  sKey.set(key);
  return(getString(sKey,value,section));
}   

// Get a config entry as an integer
bit8 ConfigFile::getInt(IN Wstring &_key,sint32 &value, IN char *section) const
{
  Wstring key(_key);
  key.toUpper();
 
  if (section)  // append section name to key
  {
    key+="[";
    key+=section;
    key+="]";
  }

  Wstring svalue;
  Critsec_.lock();
  bit8 retval=Dictionary_.getValue(key,svalue);
  Critsec_.unlock();

  if (retval==FALSE)
  { DBGMSG("Config entry missing: "<<key.get()); }

  if (retval==FALSE)
    return(FALSE);
  value=atol(svalue.get());
  return(TRUE);
}

// Get a config entry as an integer
bit8 ConfigFile::getInt(IN char *key,sint32 &value, IN char *section) const
{
  Wstring sKey;
  sKey.set(key);

  return(getInt(sKey,value,section));
}    



// Get a config entry as an integer
bit8 ConfigFile::getInt(IN Wstring &_key,sint16 &value, IN char *section) const
{
  Wstring key(_key);
  key.toUpper();
 
  if (section)  // append section name to key
  {
    key+="[";
    key+=section;
    key+="]";
  }

  Wstring svalue;
  Critsec_.lock();
  bit8 retval=Dictionary_.getValue(key,svalue);
  Critsec_.unlock();

  if (retval==FALSE)
  { DBGMSG("Config entry missing: "<<key.get()); }

  if (retval==FALSE)
    return(FALSE);
  value=atoi(svalue.get());
  return(TRUE);
}
 
// Get a config entry as an integer
bit8 ConfigFile::getInt(IN char *key,sint16 &value, IN char *section) const
{
  Wstring sKey;
  sKey.set(key);

  return(getInt(sKey,value,section));
}



/************* MDC; Added functionality for updating and saving config files ************/

// Remove an entry
bit8 ConfigFile::removeEntry(IN Wstring &_key, IN char *section)
{
	Wstring key(_key);
	key.toUpper();
	
	if (section)  // append section name to key
	{
		key+="[";
		key+=section;
		key+="]";
	}
	
	Critsec_.lock();
	bit8 retval=Dictionary_.remove(key);
	Critsec_.unlock();
	
	if (retval==FALSE)
	{ DBGMSG("Config entry missing: "<<key.get()); }
	
	if (retval==FALSE)
		return(FALSE);
	return(TRUE);
}

// Remove an entry
bit8 ConfigFile::removeEntry(IN char *key, IN char *section)
{
	Wstring sKey;
	sKey.set(key);
	return removeEntry(sKey, section);
}

// Set a config entry as a string
bit8 ConfigFile::setString(IN Wstring &_key, IN Wstring &value, IN char *section)
{
	Wstring key(_key);
	key.toUpper();
	
	if (section)  // append section name to key
	{
		key+="[";
		key+=section;
		key+="]";
	}
	else
	{
		section = "";  // give it a default
	}
	
	Critsec_.lock();
	Dictionary_.remove(key);
	bit8 retval=Dictionary_.add(key,value);

	// Test for a new section
	Wstring test;
	int i;
	for (i=0; i<sectionList.length(); i++)
	{
		sectionList.get(test, i);
		if (!strcmp(test.get(), section))
			break;
	}
	if (i == sectionList.length())
	{
		// New section!
		//DBGMSG("New section " << section << ", " << sectionList.length() << " entries");
		test.set(section);
		sectionList.addTail(test);
	}
	Critsec_.unlock();
	
	if (retval==FALSE)
	{ 
		DBGMSG("Config could not set entry: "<<key.get()); 
	}
	
	return(retval);
} 

// Set a config entry as a string
bit8 ConfigFile::setString(IN char *key,IN Wstring &value, IN char *section)
{
	Wstring sKey;
	sKey.set(key);
	return(setString(sKey,value,section));
}   

// Set a config entry as an integer
bit8 ConfigFile::setInt(IN Wstring &_key,IN sint32 &value, IN char *section)
{
	Wstring key(_key);
	key.toUpper();
	
	if (section)  // append section name to key
	{
		key+="[";
		key+=section;
		key+="]";
	}
	else
	{
		section = "";  // give it a default
	}
	
	Wstring svalue;
	svalue.setFormatted("%d", value);
	Critsec_.lock();
	Dictionary_.remove(key);
	bit8 retval=Dictionary_.add(key,svalue);
	// Test for a new section
	Wstring test;
	//DBGMSG("Testing " << sectionList.length() << " entries for " << section);
	int i;
	for (i=0; i<sectionList.length(); i++)
	{
		sectionList.get(test, i);
		//DBGMSG("Looking at " << test.get());
		if (!strcmp(test.get(), section))
			break;
	}
	if (i == sectionList.length() && section)
	{
		// New section!
		//DBGMSG("New section " << section << ", " << sectionList.length() << " entries");
		test.set(section);
		sectionList.addTail(test);
	}
	Critsec_.unlock();
	
	if (retval==FALSE)
	{ DBGMSG("Config could not set entry: "<<key.get()); }
	
	if (retval==FALSE)
		return(FALSE);
	return(TRUE);
}

// Set a config entry as an integer
bit8 ConfigFile::setInt(IN char *key,IN sint32 &value, IN char *section)
{
	Wstring sKey;
	sKey.set(key);
	return(setInt(sKey,value,section));
}    


// Write config file to disk.  Does not preserve comments, etc.
bit8 ConfigFile::writeFile(FILE *config)
{
	if (!config)
	{
		ERRMSG("No FP on config file write!");
		return FALSE;
	}
	int index = 0;
	int offset = 0;
	Wstring key;
	Wstring value;

	Wstring section;
	//DBGMSG(sectionList.length() << " entries");
	for (int i=0; i<sectionList.length(); i++)
	{
		sectionList.get(section, i);
		//DBGMSG("Writing " << section.get());
		fprintf(config, "[%s]\n", section.get());
		index = 0;
		offset = 0;
		while (enumerate(index, offset, key, value, section.get())!=FALSE)
		{
			fprintf(config, "%s=%s\n", key.get(), value.get());
		}
		fprintf(config, "\n");
	}
	return TRUE;
}







/************* Static functions below **************/

// Given a Wstring, return a 32 bit integer that has a good numeric
//   distributation for the purposes of indexing into a hash table.
static uint32 Wstring_Hash(const Wstring &string)
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
