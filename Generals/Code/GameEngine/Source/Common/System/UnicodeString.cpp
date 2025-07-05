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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: UnicodeString.cpp 
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
// File name:  UnicodeString.cpp
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

/*static*/ UnicodeString UnicodeString::TheEmptyString;

// -----------------------------------------------------
#ifdef RTS_DEBUG
void UnicodeString::validate() const
{
	if (!m_data) return;
	DEBUG_ASSERTCRASH(m_data->m_refCount > 0, ("m_refCount is zero"));
	DEBUG_ASSERTCRASH(m_data->m_numCharsAllocated > 0, ("m_numCharsAllocated is zero"));
	DEBUG_ASSERTCRASH(wcslen(m_data->peek())+1 <= m_data->m_numCharsAllocated,("str is too long for storage"));
}
#endif

// -----------------------------------------------------
UnicodeString::UnicodeString(const UnicodeString& stringSrc) : m_data(stringSrc.m_data)
{
	ScopedCriticalSection scopedCriticalSection(TheUnicodeStringCriticalSection);
	if (m_data)
		++m_data->m_refCount;
	validate();
}

// -----------------------------------------------------
void UnicodeString::ensureUniqueBufferOfSize(int numCharsNeeded, Bool preserveData, const WideChar* strToCopy, const WideChar* strToCat)
{
	validate();

	if (m_data &&
			m_data->m_refCount == 1 &&
			m_data->m_numCharsAllocated >= numCharsNeeded)
	{
		// no buffer manhandling is needed (it's already large enough, and unique to us)
		if (strToCopy)
			// TheSuperHackers @fix Mauller 04/04/2025 Replace wcscpy with safer memmove as memory regions can overlap when part of string is copied to itself
			memmove(m_data->peek(), strToCopy, (wcslen(strToCopy) + 1) * sizeof(WideChar));
		if (strToCat)
			wcscat(m_data->peek(), strToCat);
		return;
	}

	DEBUG_ASSERTCRASH(TheDynamicMemoryAllocator != NULL, ("Cannot use dynamic memory allocator before its initialization. Check static initialization order.\n"));
	DEBUG_ASSERTCRASH(numCharsNeeded <= MAX_LEN, ("UnicodeString::ensureUniqueBufferOfSize exceeds max string length %d with requested length %d", MAX_LEN, numCharsNeeded));
	int minBytes = sizeof(UnicodeStringData) + numCharsNeeded*sizeof(WideChar);
	int actualBytes = TheDynamicMemoryAllocator->getActualAllocationSize(minBytes);
	UnicodeStringData* newData = (UnicodeStringData*)TheDynamicMemoryAllocator->allocateBytesDoNotZero(actualBytes, "STR_UnicodeString::ensureUniqueBufferOfSize");
	newData->m_refCount = 1;
	newData->m_numCharsAllocated = (actualBytes - sizeof(UnicodeStringData))/sizeof(WideChar);
#if defined(RTS_DEBUG)
	newData->m_debugptr = newData->peek();	// just makes it easier to read in the debugger
#endif

	if (m_data && preserveData)
		wcscpy(newData->peek(), m_data->peek());
	else
		newData->peek()[0] = 0;

	// do these BEFORE releasing the old buffer, so that self-copies
	// or self-cats will work correctly.
	if (strToCopy)
		wcscpy(newData->peek(), strToCopy);
	if (strToCat)
		wcscat(newData->peek(), strToCat);

	releaseBuffer();
	m_data = newData;

	validate();
}


// -----------------------------------------------------
void UnicodeString::releaseBuffer()
{
	ScopedCriticalSection scopedCriticalSection(TheUnicodeStringCriticalSection);

	validate();
	if (m_data)
	{
		if (--m_data->m_refCount == 0)
		{
			TheDynamicMemoryAllocator->freeBytes(m_data);
		}
		m_data = 0;
	}
}

// -----------------------------------------------------
UnicodeString::UnicodeString(const WideChar* s) : m_data(0)
{
	int len = wcslen(s);
	if (len)
	{
		ensureUniqueBufferOfSize(len + 1, false, s, NULL);
	}
	validate();
}

// -----------------------------------------------------
void UnicodeString::set(const UnicodeString& stringSrc)
{
	ScopedCriticalSection scopedCriticalSection(TheUnicodeStringCriticalSection);

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
void UnicodeString::set(const WideChar* s)
{
	validate();
	if (!m_data || s != peek())
	{
		int len = s ? wcslen(s) : 0;
		if (len)
		{
			ensureUniqueBufferOfSize(len + 1, false, s, NULL);
		}
		else
		{
			releaseBuffer();
		}
	}
	validate();
}

// -----------------------------------------------------
WideChar* UnicodeString::getBufferForRead(Int len)
{
	validate();
	DEBUG_ASSERTCRASH(len>0, ("No need to allocate 0 len strings."));
	ensureUniqueBufferOfSize(len + 1, false, NULL, NULL);
	validate();
	return peek();
}

// -----------------------------------------------------
void UnicodeString::translate(const AsciiString& stringSrc)
{
	validate();
	/// @todo srj put in a real translation here; this will only work for 7-bit ascii
	clear();
	Int len = stringSrc.getLength();
	for (Int i = 0; i < len; i++)
		concat((WideChar)stringSrc.getCharAt(i));
	validate();
}

// -----------------------------------------------------
void UnicodeString::concat(const WideChar* s)
{
	validate();
	int addlen = wcslen(s);
	if (addlen == 0)
		return;	// my, that was easy

	if (m_data)
	{
		ensureUniqueBufferOfSize(getLength() + addlen + 1, true, NULL, s);
	}
	else
	{
		set(s);
	}
	validate();
}

// -----------------------------------------------------
void UnicodeString::trim()
{
	validate();

	if (m_data)
	{
		const WideChar *c = peek();

		//	Strip leading white space from the string.
		while (c && iswspace(*c))
		{
			c++;
		}
		if (c != peek())
		{
			set(c);
		}

		if (m_data) // another check, because the previous set() could erase m_data
		{
			//	Clip trailing white space from the string.
			int len = wcslen(peek());
			for (int index = len-1; index >= 0; index--)
			{
				if (iswspace(getCharAt(index)))
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
void UnicodeString::removeLastChar()
{
	validate();
	if (m_data)
	{
		int len = wcslen(peek());
		if (len > 0)
		{
			ensureUniqueBufferOfSize(len+1, true, NULL, NULL);
			peek()[len - 1] = 0;
		}
	}
	validate();
}

// -----------------------------------------------------
void UnicodeString::format(UnicodeString format, ...)
{
	validate();
	va_list args;
  va_start(args, format);
	format_va(format, args);
  va_end(args);
	validate();
}

// -----------------------------------------------------
void UnicodeString::format(const WideChar* format, ...)
{
	validate();
	va_list args;
  va_start(args, format);
	format_va(format, args);
  va_end(args);
	validate();
}

// -----------------------------------------------------
void UnicodeString::format_va(const UnicodeString& format, va_list args)
{
	format_va(format.str(), args);
}

// -----------------------------------------------------
void UnicodeString::format_va(const WideChar* format, va_list args)
{
	validate();
	WideChar buf[MAX_FORMAT_BUF_LEN];
	const int result = vswprintf(buf, sizeof(buf)/sizeof(WideChar), format, args);
	if (result >= 0)
	{
		set(buf);
		validate();
	}
	else
	{
		DEBUG_CRASH(("UnicodeString::format_va failed with code:%d", result));
	}
}

//-----------------------------------------------------------------------------
Bool UnicodeString::nextToken(UnicodeString* tok, UnicodeString delimiters)
{
	if (this->isEmpty() || tok == this)
		return false;

	if (delimiters.isEmpty())
		delimiters = UnicodeString(L" \t\n\r");

	Int offset;

	offset = wcsspn(peek(), delimiters.str());
	WideChar* start = peek() + offset;

	offset = wcscspn(start, delimiters.str());
	WideChar* end = start + offset;

	if (end > start)
	{
		Int len = end - start;
		WideChar* tmp = tok->getBufferForRead(len + 1);
		memcpy(tmp, start, len*2);
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