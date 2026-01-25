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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

// Public Data ////////////////////////////////////////////////////////////////////////////////////
NameKeyGenerator *TheNameKeyGenerator = nullptr;  ///< name key gen. singleton

//-------------------------------------------------------------------------------------------------
NameKeyGenerator::NameKeyGenerator()
{

	m_nextID = (UnsignedInt)NAMEKEY_INVALID;  // uninitialized system

	for (Int i = 0; i < SOCKET_COUNT; ++i)
		m_sockets[i] = nullptr;

}

//-------------------------------------------------------------------------------------------------
NameKeyGenerator::~NameKeyGenerator()
{

	// free all system data
	freeSockets();

}

//-------------------------------------------------------------------------------------------------
void NameKeyGenerator::init()
{
	DEBUG_ASSERTCRASH(m_nextID == (UnsignedInt)NAMEKEY_INVALID, ("NameKeyGen already inited"));

	// start keys at the beginning again
	freeSockets();
	m_nextID = 1;

}

//-------------------------------------------------------------------------------------------------
void NameKeyGenerator::reset()
{
	freeSockets();
	m_nextID = 1;

}

//-------------------------------------------------------------------------------------------------
void NameKeyGenerator::freeSockets()
{
	for (Int i = 0; i < SOCKET_COUNT; ++i)
	{
		Bucket *next;
		for (Bucket *b = m_sockets[i]; b; b = next)
		{
			next = b->m_nextInSocket;
			deleteInstance(b);
		}
		m_sockets[i] = nullptr;
	}

}

//-------------------------------------------------------------------------------------------------
inline UnsignedInt calcHashForString(const char* p)
{
	UnsignedInt result = 0;
	Byte *pp = (Byte*)p;
	while (*pp)
		result = (result << 5) + result + *pp++;
	return result;
}

//-------------------------------------------------------------------------------------------------
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
#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
// TheSuperHackers @info xezon 04/09/2025 This key reservation is required for CRC compatibility,
// because the name keys are somehow CRC relevant. It was originally used by the file exist cache
// of the file system in Zero Hour.
Bool NameKeyGenerator::addReservedKey()
{
	switch (m_nextID)
	{
	case 97: nameToLowercaseKeyImpl("Data\\English\\Language9x.ini"); return true;
	case 98: nameToLowercaseKeyImpl("Data\\Audio\\Tracks\\English\\GLA_02.mp3"); return true;
	case 99: nameToLowercaseKeyImpl("Data\\Audio\\Tracks\\GLA_02.mp3"); return true;
	}
	return false;
}
#endif

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToKey(const AsciiString& name)
{
	const NameKeyType key = nameToKeyImpl(name);

#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
	while (addReservedKey());
#endif

	return key;
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToLowercaseKey(const AsciiString& name)
{
	const NameKeyType key = nameToLowercaseKeyImpl(name);

#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
	while (addReservedKey());
#endif

	return key;
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToKey(const char* name)
{
	const NameKeyType key = nameToKeyImpl(name);

#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
	while (addReservedKey());
#endif

	return key;
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToLowercaseKey(const char *name)
{
	const NameKeyType key = nameToLowercaseKeyImpl(name);

#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
	while (addReservedKey());
#endif

	return key;
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToKeyImpl(const AsciiString& name)
{
	const UnsignedInt hash = calcHashForString(name.str()) % SOCKET_COUNT;

	// do we have it already?
	const Bucket *b;
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (name.compare(b->m_nameString) == 0)
			return b->m_key;
	}

	// nope, guess not. let's allocate it.
	return createNameKey(hash, name);
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToLowercaseKeyImpl(const AsciiString& name)
{
	const UnsignedInt hash = calcHashForLowercaseString(name.str()) % SOCKET_COUNT;

	// do we have it already?
	const Bucket *b;
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (name.compareNoCase(b->m_nameString) == 0)
			return b->m_key;
	}

	// nope, guess not. let's allocate it.
	return createNameKey(hash, name);
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToKeyImpl(const char* name)
{
	const UnsignedInt hash = calcHashForString(name) % SOCKET_COUNT;

	// do we have it already?
	const Bucket *b;
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (strcmp(name, b->m_nameString.str()) == 0)
			return b->m_key;
	}

	// nope, guess not. let's allocate it.
	return createNameKey(hash, name);
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::nameToLowercaseKeyImpl(const char* name)
{
	const UnsignedInt hash = calcHashForLowercaseString(name) % SOCKET_COUNT;

	// do we have it already?
	const Bucket *b;
	for (b = m_sockets[hash]; b; b = b->m_nextInSocket)
	{
		if (_stricmp(name, b->m_nameString.str()) == 0)
			return b->m_key;
	}

	// nope, guess not. let's allocate it.
	return createNameKey(hash, name);
}

//-------------------------------------------------------------------------------------------------
NameKeyType NameKeyGenerator::createNameKey(UnsignedInt hash, const AsciiString& name)
{
	Bucket *b = newInstance(Bucket);
	b->m_key = (NameKeyType)m_nextID++;
	b->m_nameString = name;
	b->m_nextInSocket = m_sockets[hash];
	m_sockets[hash] = b;

	NameKeyType result = b->m_key;

#if defined(RTS_DEBUG)
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
		DEBUG_CRASH(("hmm, might need to increase the number of bucket-sockets for NameKeyGenerator (numOverThresh %d = %f%%)",numOverThresh,(Real)numOverThresh/(Real)(SOCKET_COUNT/20)));
	}
#endif

	return result;
}

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
