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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: NameKeyGenerator.cpp /////////////////////////////////////////////////////////////////////
// Created:   Michael Booth, May 2001
//						Colin Day, May 2001
// Desc:      Name key system to translate between names and unique key ids
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// Public Data ////////////////////////////////////////////////////////////////////////////////////
NameKeyGenerator *TheNameKeyGenerator = NULL;  ///< name key gen. singleton

//------------------------------------------------------------------------------------------------- 
NameKeyGenerator::NameKeyGenerator()
{

	m_nextID = (UnsignedInt)NAMEKEY_INVALID;  // uninitialized system

	for (Int i = 0; i < SOCKET_COUNT; ++i)
		m_sockets[i] = NULL;

}  // end NameKeyGenerator

//------------------------------------------------------------------------------------------------- 
NameKeyGenerator::~NameKeyGenerator()
{
	
	// free all system data
	freeSockets();

}  // end ~NameKeyGenerator

//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::init()
{
	DEBUG_ASSERTCRASH(m_nextID == (UnsignedInt)NAMEKEY_INVALID, ("NameKeyGen already inited"));

	// start keys at the beginning again
	freeSockets();
	m_nextID = 1;

}  // end init

//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::reset()
{
	freeSockets();
	m_nextID = 1;

}  // end reset

//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::freeSockets()
{
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		Bucket *next;
		for (Bucket *b = m_sockets[i]; b; b = next)
		{
			next = b->m_nextInSocket;
			b->deleteInstance();
		}
		m_sockets[i] = NULL;
	}

}  // end freeSockets

/* ------------------------------------------------------------------------ */
inline UnsignedInt calcHashForString(const char* p)
{
	UnsignedInt result = 0; 
	Byte *pp = (Byte*)p;
	while (*pp) 
		result = (result << 5) + result + *pp++; 
	return result;
}

/* ------------------------------------------------------------------------ */
inline UnsignedInt calcHashForLowercaseString(const char* p)
{
	UnsignedInt result = 0; 
	Byte *pp = (Byte*)p;
	while (*pp) 
		result = (result << 5) + result + tolower(*pp++); 
	return result;
}

//------------------------------------------------------------------------------------------------- 
AsciiString NameKeyGenerator::keyToName(NameKeyType key)
{
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		for (Bucket *b = m_sockets[i]; b; b = b->m_nextInSocket)
		{
			if (key == b->m_key)
				return b->m_nameString;
		}
	}
	return AsciiString::TheEmptyString;
}

//------------------------------------------------------------------------------------------------- 
NameKeyType NameKeyGenerator::nameToKey(const char* nameString)
{
	Bucket *b;

	UnsignedInt hash = calcHashForString(nameString) % SOCKET_COUNT;

	// hmm, do we have it already?
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (strcmp(nameString, b->m_nameString.str()) == 0)
			return b->m_key; 
	}

	// nope, guess not. let's allocate it.
	b = newInstance(Bucket);
	b->m_key = (NameKeyType)m_nextID++;
	b->m_nameString = nameString;
	b->m_nextInSocket = m_sockets[hash];
	m_sockets[hash] = b;

	NameKeyType result = b->m_key;

#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	// reality-check to be sure our hasher isn't going bad.
	const Int maxThresh = 3;
	Int numOverThresh = 0;
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		Int numInThisSocket = 0;
		for (b = m_sockets[i]; b; b = b->m_nextInSocket)
			++numInThisSocket;

		if (numInThisSocket > maxThresh)
			++numOverThresh;
	}
	
	// if more than a small percent of the sockets are getting deep, probably want to increase the socket count.
	if (numOverThresh > SOCKET_COUNT/20)
	{
		DEBUG_CRASH(("hmm, might need to increase the number of bucket-sockets for NameKeyGenerator (numOverThresh %d = %f%%)\n",numOverThresh,(Real)numOverThresh/(Real)(SOCKET_COUNT/20)));
	}
#endif

	return result;

}  // end nameToKey

//------------------------------------------------------------------------------------------------- 
NameKeyType NameKeyGenerator::nameToLowercaseKey(const char* nameString)
{
	Bucket *b;

	UnsignedInt hash = calcHashForLowercaseString(nameString) % SOCKET_COUNT;

	// hmm, do we have it already?
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (_stricmp(nameString, b->m_nameString.str()) == 0)
			return b->m_key; 
	}

	// nope, guess not. let's allocate it.
	b = newInstance(Bucket);
	b->m_key = (NameKeyType)m_nextID++;
	b->m_nameString = nameString;
	b->m_nextInSocket = m_sockets[hash];
	m_sockets[hash] = b;

	NameKeyType result = b->m_key;

#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	// reality-check to be sure our hasher isn't going bad.
	const Int maxThresh = 3;
	Int numOverThresh = 0;
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		Int numInThisSocket = 0;
		for (b = m_sockets[i]; b; b = b->m_nextInSocket)
			++numInThisSocket;

		if (numInThisSocket > maxThresh)
			++numOverThresh;
	}
	
	// if more than a small percent of the sockets are getting deep, probably want to increase the socket count.
	if (numOverThresh > SOCKET_COUNT/20)
	{
		DEBUG_CRASH(("hmm, might need to increase the number of bucket-sockets for NameKeyGenerator (numOverThresh %d = %f%%)\n",numOverThresh,(Real)numOverThresh/(Real)(SOCKET_COUNT/20)));
	}
#endif

	return result;

}  // end nameToLowercaseKey

//------------------------------------------------------------------------------------------------- 
// Get a string out of the INI. Store it into a NameKeyType
//------------------------------------------------------------------------------------------------- 
void NameKeyGenerator::parseStringAsNameKeyType( INI *ini, void *instance, void *store, const void* userData )
{
  *(NameKeyType *)store = TheNameKeyGenerator->nameToKey( ini->getNextToken() );
}


//------------------------------------------------------------------------------------------------- 
NameKeyType StaticNameKey::key() const
{
	if (m_key == NAMEKEY_INVALID)
	{
		DEBUG_ASSERTCRASH(TheNameKeyGenerator, ("no TheNameKeyGenerator yet"));
		if (TheNameKeyGenerator)
			m_key = TheNameKeyGenerator->nameToKey(m_name);
	}
	return m_key;
}
