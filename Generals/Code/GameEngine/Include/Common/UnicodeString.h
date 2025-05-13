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

// FILE: UnicodeString.h 
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
// File name:  UnicodeString.h
//
// Created:    Steven Johnson, October 2001
//
// Desc:       General-purpose string classes
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef UNICODESTRING_H
#define UNICODESTRING_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "Lib/BaseType.h"
#include "Common/Debug.h"
#include "Common/Errors.h"

class AsciiString;

// -----------------------------------------------------
/**
	UnicodeString is the fundamental double-byte string type used in the Generals
	code base, and should be preferred over all other string constructions
	(e.g., array of WideChar, STL string<>, WWVegas StringClass, etc.)

	Of course, other string setups may be used when necessary or appropriate!

	UnicodeString is modeled after the MFC CString class, with some minor
	syntactic differences to keep in line with our coding conventions.

	Basically, UnicodeString allows you to treat a string as an intrinsic
	type, rather analogous to 'int' -- when passed by value, a new string
	is created, and modifying the new string doesn't modify the original.
	This is done fairly efficiently, so that no new memory allocation is done
	unless the string is actually modified. 

	Naturally, UnicodeString handles all memory issues, so there's no need
	to do anything to free memory... just allow the UnicodeString's
	destructor to run.

	UnicodeStrings are suitable for use as automatic, member, or static variables.
*/

class UnicodeString
{
private:
	
	// Note, this is a Plain Old Data Structure... don't
	// add a ctor/dtor, 'cuz they won't ever be called.
	struct UnicodeStringData
	{
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
		const WideChar* m_debugptr;	// just makes it easier to read in the debugger
#endif
		unsigned short	m_refCount;						// reference count
		unsigned short	m_numCharsAllocated;  // length of data allocated
		// WideChar m_stringdata[];

		inline WideChar* peek() { return (WideChar*)(this+1); }
	};

	#ifdef RTS_DEBUG
	void validate() const;
	#else
	inline void validate() const { }
	#endif

protected:
	UnicodeStringData* m_data;   // pointer to ref counted string data

	WideChar* peek() const;
	void releaseBuffer();
	void ensureUniqueBufferOfSize(int numCharsNeeded, Bool preserveData, const WideChar* strToCpy, const WideChar* strToCat);

public:

	enum 
	{ 
		MAX_FORMAT_BUF_LEN = 2048,		///< max total len of string created by format/format_va
		MAX_LEN = 32767							///< max total len of any UnicodeString, in chars
	};


	/**
		This is a convenient global used to indicate the empty
		string, so we don't need to construct temporaries
		for such a common thing.
	*/
	static UnicodeString TheEmptyString;

	/**
		Default constructor -- construct a new, empty UnicodeString.
	*/
	UnicodeString();
	/**
		Copy constructor -- make this UnicodeString identical to the
		other UnicodeString. (This is actually quite efficient, because
		they will simply share the same string and increment the
		refcount.)
	*/
	UnicodeString(const UnicodeString& stringSrc);
	/**
		Constructor -- from a literal string. Constructs an UnicodeString
		with the given string. Note that a copy of the string is made;
		the input ptr is not saved. Note also that this is declared
		'explicit' to avoid implicit conversions from const-WideChar-*
		(e.g., as input arguments).
	*/
	explicit UnicodeString(const WideChar* s);

	/**
		Destructor. Not too exciting... clean up the works and such.
	*/
	~UnicodeString();

	/**
		Return the length, in characters (not bytes!), of the string.
	*/
	int getLength() const;
	/**
		Return true iff the length of the string is zero. Equivalent
		to (getLength() == 0) but slightly more efficient.
	*/
	Bool isEmpty() const;
	/**
		Make the string empty. Equivalent to (str = "") but slightly more efficient.
	*/
	void clear();

	/**
		Return the character and the given (zero-based) index into the string.
		No range checking is done (except in debug mode).
	*/
	WideChar getCharAt(int index) const;
	/**
		Return a pointer to the (null-terminated) string. Note that this is 
		a const pointer: do NOT change this! It is imperative that it be 
		impossible (or at least, really difficuly) for someone to change our
		private data, since it might be shared amongst other UnicodeStrings.
	*/
	const WideChar* str() const;

	/**
		Makes sure there is room for a string of len+1 characters, and
		returns a pointer to the string buffer.  This ensures that the
		string buffer is NOT shared.  This is intended for the file reader, 
		that is reading new strings in from a file. jba.
	*/
	WideChar* getBufferForRead(Int len);

	/**
		Replace the contents of self with the given string.
		(This is actually quite efficient, because
		they will simply share the same string and increment the
		refcount.)
	*/
	void set(const UnicodeString& stringSrc);
	/**
		Replace the contents of self with the given string.
		Note that a copy of the string is made; the input ptr is not saved.
	*/
	void set(const WideChar* s);

	/**
		replace contents of self with the given string. Note the
		nomenclature is translate rather than set; this is because
		not all single-byte strings translate one-for-one into
		UnicodeStrings, so some data manipulation may be necessary,
		and the resulting strings may not be equivalent.
	*/
	void translate(const AsciiString& stringSrc);

	/**
		Concatenate the given string onto self.
	*/
	void concat(const UnicodeString& stringSrc);
	/**
		Concatenate the given string onto self.
	*/
	void concat(const WideChar* s);
	/**
		Concatenate the given character onto self.
	*/
	void concat(const WideChar c);

	/**
	  Remove leading and trailing whitespace from the string.
	*/
	void trim( void );

	/**
		Remove the final character in the string. If the string is empty,
		do nothing. (This is a rather dorky method, but used a lot in 
		text editing, thus its presence here.)
	*/
	void removeLastChar();

	/**
		Analogous to sprintf() -- this formats a string according to the
		given sprintf-style format string (and the variable argument list)
		and stores the result in self.
	*/
	void format(UnicodeString format, ...);
	void format(const WideChar* format, ...);
	/**
		Identical to format(), but takes a va_list rather than
		a variable argument list. (i.e., analogous to vsprintf.)
	*/
	void format_va(const UnicodeString& format, va_list args);
	void format_va(const WideChar* format, va_list args);

	/**
		Conceptually identical to wsccmp().
	*/
	int compare(const UnicodeString& stringSrc) const;
	/**
		Conceptually identical to wsccmp().
	*/
	int compare(const WideChar* s) const;
	/**
		Conceptually identical to _wcsicmp().
	*/
	int compareNoCase(const UnicodeString& stringSrc) const;
	/**
		Conceptually identical to _wcsicmp().
	*/
	int compareNoCase(const WideChar* s) const;

	/**
		conceptually similar to strtok():

		extract the next whitespace-delimited token from the front
		of 'this' and copy it into 'token', returning true if a nonempty
		token was found. (note that this modifies 'this' as well, stripping
		the token off!)
	*/
	Bool nextToken(UnicodeString* token, UnicodeString delimiters = UnicodeString::TheEmptyString);

//
// You might think it would be a good idea to overload the * operator
// to allow for an implicit conversion to an WideChar*. This is
// in theory a good idea, but in practice, there's lots of code
// that assumes it should check text fields for null, which
// is meaningless for us, since we never return a null ptr.
//
//	operator const WideChar*() const { return str(); }
//

	UnicodeString& operator=(const UnicodeString& stringSrc);	///< the same as set()
	UnicodeString& operator=(const WideChar* s);				///< the same as set()
};


// -----------------------------------------------------
inline WideChar* UnicodeString::peek() const
{
	DEBUG_ASSERTCRASH(m_data, ("null string ptr"));
	validate();
	return m_data->peek();
}

// -----------------------------------------------------
inline UnicodeString::UnicodeString() : m_data(0)
{
	validate();
}

// -----------------------------------------------------
inline UnicodeString::~UnicodeString()
{
	validate();
	releaseBuffer();
}

// -----------------------------------------------------
inline int UnicodeString::getLength() const
{
	validate();
	return m_data ? wcslen(peek()) : 0;
}

// -----------------------------------------------------
inline Bool UnicodeString::isEmpty() const
{
	validate();
	return m_data == NULL || peek()[0] == 0;
}

// -----------------------------------------------------
inline void UnicodeString::clear()
{
	validate();
	releaseBuffer();
	validate();
}

// -----------------------------------------------------
inline WideChar UnicodeString::getCharAt(int index) const
{
	DEBUG_ASSERTCRASH(index >= 0 && index < getLength(), ("bad index in getCharAt"));
	validate();
	return m_data ? peek()[index] : 0;
}

// -----------------------------------------------------
inline const WideChar* UnicodeString::str() const
{
	validate();
	static const WideChar TheNullChr = 0;
	return m_data ? peek() : &TheNullChr;
}

// -----------------------------------------------------
inline UnicodeString& UnicodeString::operator=(const UnicodeString& stringSrc)
{
	validate();
	set(stringSrc);
	validate();
	return *this;
}

// -----------------------------------------------------
inline UnicodeString& UnicodeString::operator=(const WideChar* s)
{
	validate();
	set(s);
	validate();
	return *this;
}

// -----------------------------------------------------
inline void UnicodeString::concat(const UnicodeString& stringSrc)
{
	validate();
	concat(stringSrc.str());
	validate();
}

// -----------------------------------------------------
inline void UnicodeString::concat(const WideChar c)
{
	validate();
	/// this can probably be made more efficient, if necessary
	WideChar tmp[2] = { c, 0 };
	concat(tmp);
	validate();
}

// -----------------------------------------------------
inline int UnicodeString::compare(const UnicodeString& stringSrc) const
{
	validate();
	return wcscmp(this->str(), stringSrc.str());
}

// -----------------------------------------------------
inline int UnicodeString::compare(const WideChar* s) const
{
	validate();
	return wcscmp(this->str(), s);
}

// -----------------------------------------------------
inline int UnicodeString::compareNoCase(const UnicodeString& stringSrc) const
{
	validate();
	return _wcsicmp(this->str(), stringSrc.str());
}

// -----------------------------------------------------
inline int UnicodeString::compareNoCase(const WideChar* s) const
{
	validate();
	return _wcsicmp(this->str(), s);
}

// -----------------------------------------------------
inline Bool operator==(const UnicodeString& s1, const UnicodeString& s2)
{
	return wcscmp(s1.str(), s2.str()) == 0;
}

// -----------------------------------------------------
inline Bool operator!=(const UnicodeString& s1, const UnicodeString& s2)
{
	return wcscmp(s1.str(), s2.str()) != 0;
}

// -----------------------------------------------------
inline Bool operator<(const UnicodeString& s1, const UnicodeString& s2)
{
	return wcscmp(s1.str(), s2.str()) < 0;
}

// -----------------------------------------------------
inline Bool operator<=(const UnicodeString& s1, const UnicodeString& s2)
{
	return wcscmp(s1.str(), s2.str()) <= 0;
}

// -----------------------------------------------------
inline Bool operator>(const UnicodeString& s1, const UnicodeString& s2)
{
	return wcscmp(s1.str(), s2.str()) > 0;
}

// -----------------------------------------------------
inline Bool operator>=(const UnicodeString& s1, const UnicodeString& s2)
{
	return wcscmp(s1.str(), s2.str()) >= 0;
}

#endif // UNICODESTRING_H
