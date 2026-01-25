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
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/CriticalSection.h"


// -----------------------------------------------------

/*static*/ const AsciiString AsciiString::TheEmptyString;

//-----------------------------------------------------------------------------
inline char* skipSeps(char* p, const char* seps)
{
	while (*p && strchr(seps, *p) != nullptr)
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline char* skipNonSeps(char* p, const char* seps)
{
	while (*p && strchr(seps, *p) == nullptr)
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

// -----------------------------------------------------
AsciiString::AsciiString(const AsciiString& stringSrc) : m_data(stringSrc.m_data)
{
	ScopedCriticalSection scopedCriticalSection(TheAsciiStringCriticalSection);
	if (m_data)
		++m_data->m_refCount;
	validate();
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
		DEBUG_LOG(("cannot ignore the leak (no data)"));
	}
#endif
}

// -----------------------------------------------------
void AsciiString::ensureUniqueBufferOfSize(int numCharsNeeded, Bool preserveData, const char* strToCopy, const char* strToCat)
{
	validate();

	const int usableNumChars = numCharsNeeded - 1;

	if (m_data &&
			m_data->m_refCount == 1 &&
			m_data->m_numCharsAllocated >= numCharsNeeded)
	{
		// no buffer manhandling is needed (it's already large enough, and unique to us)
		if (strToCopy)
		{
			// TheSuperHackers @fix Mauller 04/04/2025 Replace strcpy with safer memmove as memory regions can overlap when part of string is copied to itself
			DEBUG_ASSERTCRASH(usableNumChars <= strlen(strToCopy), ("strToCopy is too small"));
			memmove(m_data->peek(), strToCopy, usableNumChars);
			m_data->peek()[usableNumChars] = 0;
		}
		if (strToCat)
			strcat(m_data->peek(), strToCat);
		return;
	}

	DEBUG_ASSERTCRASH(TheDynamicMemoryAllocator != nullptr, ("Cannot use dynamic memory allocator before its initialization. Check static initialization order."));
	DEBUG_ASSERTCRASH(numCharsNeeded <= MAX_LEN, ("AsciiString::ensureUniqueBufferOfSize exceeds max string length %d with requested length %d", MAX_LEN, numCharsNeeded));
	int minBytes = sizeof(AsciiStringData) + numCharsNeeded*sizeof(char);
	int actualBytes = TheDynamicMemoryAllocator->getActualAllocationSize(minBytes);
	AsciiStringData* newData = (AsciiStringData*)TheDynamicMemoryAllocator->allocateBytesDoNotZero(actualBytes, "STR_AsciiString::ensureUniqueBufferOfSize");
	newData->m_refCount = 1;
	newData->m_numCharsAllocated = (actualBytes - sizeof(AsciiStringData))/sizeof(char);
#if defined(RTS_DEBUG)
	newData->m_debugptr = newData->peek();	// just makes it easier to read in the debugger
#endif

	if (m_data && preserveData)
		strcpy(newData->peek(), m_data->peek());
	else
		newData->peek()[0] = 0;

	// do these BEFORE releasing the old buffer, so that self-copies
	// or self-cats will work correctly.
	if (strToCopy)
	{
		DEBUG_ASSERTCRASH(usableNumChars <= strlen(strToCopy), ("strToCopy is too small"));
		strncpy(newData->peek(), strToCopy, usableNumChars);
		newData->peek()[usableNumChars] = 0;
	}
	if (strToCat)
		strcat(newData->peek(), strToCat);

	releaseBuffer();
	m_data = newData;

	validate();
}


// -----------------------------------------------------
void AsciiString::releaseBuffer()
{
	ScopedCriticalSection scopedCriticalSection(TheAsciiStringCriticalSection);

	validate();
	if (m_data)
	{
		if (--m_data->m_refCount == 0)
		{
			TheDynamicMemoryAllocator->freeBytes(m_data);
		}
		m_data = nullptr;
	}
	validate();
}

// -----------------------------------------------------
AsciiString::AsciiString(const char* s) : m_data(nullptr)
{
	//DEBUG_ASSERTCRASH(isMemoryManagerOfficiallyInited(), ("Initializing AsciiStrings prior to main (ie, as static vars) can cause memory leak reporting problems. Are you sure you want to do this?"));
	int len = s ? (int)strlen(s) : 0;
	if (len > 0)
	{
		ensureUniqueBufferOfSize(len + 1, false, s, nullptr);
	}
	validate();
}

// -----------------------------------------------------
AsciiString::AsciiString(const char* s, int len) : m_data(nullptr)
{
	if (len > 0)
	{
		ensureUniqueBufferOfSize(len + 1, false, s, nullptr);
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::set(const AsciiString& stringSrc)
{
	ScopedCriticalSection scopedCriticalSection(TheAsciiStringCriticalSection);

	validate();
	if (&stringSrc != this)
	{
		releaseBuffer();
		m_data = stringSrc.m_data;
		if (m_data)
			++m_data->m_refCount;
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::set(const char* s)
{
	int len = s ? strlen(s) : 0;
	set(s, len);
}

// -----------------------------------------------------
void AsciiString::set(const char* s, int len)
{
	validate();
	if (!m_data || s != peek())
	{
		if (len > 0)
		{
			ensureUniqueBufferOfSize(len + 1, false, s, nullptr);
		}
		else
		{
			releaseBuffer();
		}
	}
	validate();
}

// -----------------------------------------------------
char*  AsciiString::getBufferForRead(Int len)
{
	validate();
	DEBUG_ASSERTCRASH(len>0, ("No need to allocate 0 len strings."));
	ensureUniqueBufferOfSize(len + 1, false, nullptr, nullptr);
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
void AsciiString::concat(const char* s)
{
	validate();
	int addlen = strlen(s);
	if (addlen == 0)
		return;	// my, that was easy

	if (m_data)
	{
		ensureUniqueBufferOfSize(getLength() + addlen + 1, true, nullptr, s);
	}
	else
	{
		set(s);
	}
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

		trimEnd();
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::trimEnd()
{
	validate();

	if (m_data)
	{
		// Clip trailing white space from the string.
		const int len = strlen(peek());
		int index = len;
		while (index > 0 && isspace(getCharAt(index - 1)))
		{
			--index;
		}

		if (index < len)
		{
			truncateTo(index);
		}
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::trimEnd(const char c)
{
	validate();

	if (m_data)
	{
		// Clip trailing consecutive occurrences of c from the string.
		const int len = strlen(peek());
		int index = len;
		while (index > 0 && getCharAt(index - 1) == c)
		{
			--index;
		}

		if (index < len)
		{
			truncateTo(index);
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
	truncateBy(1);
}

// -----------------------------------------------------
void AsciiString::truncateBy(const Int charCount)
{
	validate();
	if (m_data && charCount > 0)
	{
		const size_t len = strlen(peek());
		if (len > 0)
		{
			ensureUniqueBufferOfSize(len + 1, true, nullptr, nullptr);
			size_t count = charCount;
			if (charCount > len)
			{
				count = len;
			}
			peek()[len - count] = 0;
		}
	}
	validate();
}

// -----------------------------------------------------
void AsciiString::truncateTo(const Int maxLength)
{
	validate();
	if (m_data)
	{
		const size_t len = strlen(peek());
		if (len > maxLength)
		{
			ensureUniqueBufferOfSize(len + 1, true, nullptr, nullptr);
			peek()[maxLength] = 0;
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
	const int result = vsnprintf(buf, sizeof(buf)/sizeof(char), format, args);
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
	return m_data && ::startsWith(peek(), p);
}

// -----------------------------------------------------
Bool AsciiString::startsWithNoCase(const char* p) const
{
	return m_data && ::startsWithNoCase(peek(), p);
}

// -----------------------------------------------------
Bool AsciiString::endsWith(const char* p) const
{
	return m_data && ::endsWith(peek(), p);
}

// -----------------------------------------------------
Bool AsciiString::endsWithNoCase(const char* p) const
{
	return m_data && ::endsWithNoCase(peek(), p);
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

	if (seps == nullptr)
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
