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

// FILE: AsciiString.cpp 
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  AsciiString.cpp
//
// Created:    Steven Johnson, October 2001
//
// Desc:       General-purpose string classes
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/CriticalSection.h"


// -----------------------------------------------------

/*static*/ AsciiString AsciiString::TheEmptyString;

//-----------------------------------------------------------------------------
inline char* skipSeps(char* p, const char* seps)
{
	while (*p && strchr(seps, *p) != NULL)
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline char* skipNonSeps(char* p, const char* seps)
{
	while (*p && strchr(seps, *p) == NULL)
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline char* skipWhitespace(char* p)
{
	while (*p && isspace(*p))
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline char* skipNonWhitespace(char* p)
{
	while (*p && !isspace(*p))
		++p;
	return p;
}

void AsciiString::freeBytes(void)
{
	TheDynamicMemoryAllocator->freeBytes(m_data);
}

// -----------------------------------------------------
#ifdef RTS_DEBUG
void AsciiString::validate() const
{
	if (!m_data) return;
	DEBUG_ASSERTCRASH(m_data->m_refCount > 0, ("m_refCount is zero"));
	DEBUG_ASSERTCRASH(m_data->m_refCount < 32000, ("m_refCount is suspiciously large"));
	DEBUG_ASSERTCRASH(m_data->m_numCharsAllocated > 0, ("m_numCharsAllocated is zero"));
//	DEBUG_ASSERTCRASH(m_data->m_numCharsAllocated < 1024, ("m_numCharsAllocated suspiciously large"));
	DEBUG_ASSERTCRASH(strlen(m_data->peek())+1 <= m_data->m_numCharsAllocated,("str is too long (%d) for storage",strlen(m_data->peek())+1));
}
#endif

// -----------------------------------------------------
void AsciiString::debugIgnoreLeaks()
{
#ifdef MEMORYPOOL_DEBUG
	if (m_data)
	{
		TheDynamicMemoryAllocator->debugIgnoreLeaksForThisBlock(m_data);
	}
	else
	{
		DEBUG_LOG(("cannot ignore the leak (no data)\n"));
	}
#endif
}

// -----------------------------------------------------
void AsciiString::ensureUniqueBufferOfSize(int numCharsNeeded, Bool preserveData, const char* strToCopy, const char* strToCat)
{
	validate();

	if (m_data &&
			m_data->m_refCount == 1 &&
			m_data->m_numCharsAllocated >= numCharsNeeded)
	{
		// no buffer manhandling is needed (it's already large enough, and unique to us)
		if (strToCopy)
			// TheSuperHackers @fix Mauller 04/04/2025 Replace strcpy with safer memmove as memory regions can overlap when part of string is copied to itself
			memmove(m_data->peek(), strToCopy, strlen(strToCopy) + 1);
		if (strToCat)
			strcat(m_data->peek(), strToCat);
		return;
	}

	int minBytes = sizeof(AsciiStringData) + numCharsNeeded*sizeof(char);
	if (minBytes > MAX_LEN)
		throw ERROR_OUT_OF_MEMORY;

	int actualBytes = TheDynamicMemoryAllocator->getActualAllocationSize(minBytes);
	AsciiStringData* newData = (AsciiStringData*)TheDynamicMemoryAllocator->allocateBytesDoNotZero(actualBytes, "STR_AsciiString::ensureUniqueBufferOfSize");
	newData->m_refCount = 1;
	newData->m_numCharsAllocated = (actualBytes - sizeof(AsciiStringData))/sizeof(char);
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	newData->m_debugptr = newData->peek();	// just makes it easier to read in the debugger
#endif

	if (m_data && preserveData)
		strcpy(newData->peek(), m_data->peek());
	else
		newData->peek()[0] = 0;

	// do these BEFORE releasing the old buffer, so that self-copies
	// or self-cats will work correctly.
	if (strToCopy)
		strcpy(newData->peek(), strToCopy);
	if (strToCat)
		strcat(newData->peek(), strToCat);

	releaseBuffer();
	m_data = newData;

	validate();
}


// -----------------------------------------------------
char*  AsciiString::getBufferForRead(Int len)
{
	validate();
	DEBUG_ASSERTCRASH(len>0, ("No need to allocate 0 len strings."));
	ensureUniqueBufferOfSize(len + 1, false, NULL, NULL);
	validate();
	return peek();
}

// -----------------------------------------------------
void AsciiString::translate(const UnicodeString& stringSrc)
{
	validate();
	/// @todo srj put in a real translation here; this will only work for 7-bit ascii
	clear();
	Int len = stringSrc.getLength();
	for (Int i = 0; i < len; i++)
		concat((char)stringSrc.getCharAt(i));
	validate();
}

// -----------------------------------------------------
void AsciiString::trim()
{
	validate();

	if (m_data)
	{
		char *c = peek();

		//	Strip leading white space from the string.
		c = skipWhitespace(c);
		if (c != peek())
		{
			set(c);
		}

		if (m_data) // another check, because the previous set() could erase m_data
		{
			//	Clip trailing white space from the string.
			int len = strlen(peek());
			for (int index = len-1; index >= 0; index--)
			{
				if (isspace(getCharAt(index)))
				{
					removeLastChar();
				}
				else
				{
					break;
				}
			}
		}
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::toLower()
{
	validate();
	if (m_data)
	{
		char buf[MAX_FORMAT_BUF_LEN];
		strcpy(buf, peek());

		char *c = buf;
		while (c && *c)
		{
			*c = tolower(*c);
			c++;
		}
		set(buf);
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::removeLastChar()
{
	validate();
	if (m_data)
	{
		int len = strlen(peek());
		if (len > 0)
		{
			ensureUniqueBufferOfSize(len+1, true, NULL, NULL);
			peek()[len - 1] = 0;
		}
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::format(AsciiString format, ...)
{
	validate();
	va_list args;
  va_start(args, format);
	format_va(format, args);
  va_end(args);
	validate();
}

// -----------------------------------------------------
void AsciiString::format(const char* format, ...)
{
	validate();
	va_list args;
  va_start(args, format);
	format_va(format, args);
  va_end(args);
	validate();
}

// -----------------------------------------------------
void AsciiString::format_va(const AsciiString& format, va_list args)
{
	format_va(format.str(), args);
}

// -----------------------------------------------------
void AsciiString::format_va(const char* format, va_list args)
{
	validate();
	char buf[MAX_FORMAT_BUF_LEN];
	const int result = _vsnprintf(buf, sizeof(buf)/sizeof(char)-1, format, args);
	if (result >= 0)
	{
		set(buf);
		validate();
	}
	else
	{
		DEBUG_CRASH(("AsciiString::format_va failed with code:%d format:\"%s\"", result, format));
	}
}

// -----------------------------------------------------
Bool AsciiString::startsWith(const char* p) const
{
	if (*p == 0)
		return true;	// everything starts with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strncmp(peek(), p, lenThat) == 0;
}

// -----------------------------------------------------
Bool AsciiString::startsWithNoCase(const char* p) const
{
	if (*p == 0)
		return true;	// everything starts with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strnicmp(peek(), p, lenThat) == 0;
}

// -----------------------------------------------------
Bool AsciiString::endsWith(const char* p) const
{
	if (*p == 0)
		return true;	// everything ends with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strncmp(peek() + lenThis - lenThat, p, lenThat) == 0;
}

// -----------------------------------------------------
Bool AsciiString::endsWithNoCase(const char* p) const
{
	if (*p == 0)
		return true;	// everything ends with the empty string

	int lenThis = getLength();
	int lenThat = strlen(p);
	if (lenThis < lenThat)
		return false;	// that must be smaller than this

	return strnicmp(peek() + lenThis - lenThat, p, lenThat) == 0;
}

//-----------------------------------------------------------------------------
Bool AsciiString::isNone() const
{
	return m_data && stricmp(peek(), "None") == 0;
}

//-----------------------------------------------------------------------------
Bool AsciiString::nextToken(AsciiString* tok, const char* seps)
{
	if (this->isEmpty() || tok == this)
		return false;

	if (seps == NULL)
		seps = " \n\r\t";

	char* start = skipSeps(peek(), seps);
	char* end = skipNonSeps(start, seps);

	if (end > start)
	{
		Int len = end - start;
		char* tmp = tok->getBufferForRead(len + 1);
		memcpy(tmp, start, len);
		tmp[len] = 0;

		this->set(end);

		return true;
	}
	else
	{
		this->clear();
		tok->clear();
		return false;
	}
}
