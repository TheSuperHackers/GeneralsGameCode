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

// FILE: AsciiString.h
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
// File name:  AsciiString.h
//
// Created:    Steven Johnson, October 2001
//
// Desc:       General-purpose string classes
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef ASCIISTRING_H
#define ASCIISTRING_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "Lib/BaseType.h"
#include "Common/Debug.h"
#include "Common/Errors.h"

class UnicodeString;

// -----------------------------------------------------
/**
	AsciiString is the fundamental single-byte string type used in the Generals
	code base, and should be preferred over all other string constructions
	(e.g., array of char, STL string<>, WWVegas StringClass, etc.)

	Of course, other string setups may be used when necessary or appropriate!

	AsciiString is modeled after the MFC CString class, with some minor
	syntactic differences to keep in line with our coding conventions.

	Basically, AsciiString allows you to treat a string as an intrinsic
	type, rather analogous to 'int' -- when passed by value, a new string
	is created, and modifying the new string doesn't modify the original.
	This is done fairly efficiently, so that no new memory allocation is done
	unless the string is actually modified.

	Naturally, AsciiString handles all memory issues, so there's no need
	to do anything to free memory... just allow the AsciiString's
	destructor to run.

	AsciiStrings are suitable for use as automatic, member, or static variables.
*/

class AsciiString
{
private:

	// Note, this is a Plain Old Data Structure... don't
	// add a ctor/dtor, 'cuz they won't ever be called.
	struct AsciiStringData
	{
#if defined(RTS_DEBUG)
		const char* m_debugptr;	// just makes it easier to read in the debugger
#endif
		unsigned short	m_refCount;						// reference count
		unsigned short	m_numCharsAllocated;  // length of data allocated
		// char m_stringdata[];

		inline char* peek() { return (char*)(this+1); }
	};

	#ifdef RTS_DEBUG
	void validate() const;
	#else
	inline void validate() const { }
	#endif

protected:
	AsciiStringData* m_data;   // pointer to ref counted string data

	char* peek() const;
	void releaseBuffer();
	void ensureUniqueBufferOfSize(int numCharsNeeded, Bool preserveData, const char* strToCpy, const char* strToCat);

public:

	enum
	{
		MAX_FORMAT_BUF_LEN = 2048,		///< max total len of string created by format/format_va
		MAX_LEN = 32767							///< max total len of any AsciiString, in chars
	};


	/**
		This is a convenient global used to indicate the empty
		string, so we don't need to construct temporaries
		for such a common thing.
	*/
	static AsciiString TheEmptyString;

	/**
		Default constructor -- construct a new, empty AsciiString.
	*/
	AsciiString();
	/**
		Copy constructor -- make this AsciiString identical to the
		other AsciiString. (This is actually quite efficient, because
		they will simply share the same string and increment the
		refcount.)
	*/
	AsciiString(const AsciiString& stringSrc);
	/**
		Constructor -- from a literal string. Constructs an AsciiString
		with the given string. Note that a copy of the string is made;
		the input ptr is not saved.
		Note that this is no longer explicit, as the conversion is almost
		always wanted, anyhow.
	*/
	AsciiString(const char* s);

	/**
		Constructs an AsciiString with the given string and length.
		The length must not be larger than the actual string length.
	*/
	AsciiString(const char* s, int len);

	/**
		Destructor. Not too exciting... clean up the works and such.
	*/
	~AsciiString();

	/**
		Return the length, in characters, of the string up to the first zero or null terminator.
	*/
	int getLength() const;

	/**
		Return the number of bytes used by the string up to the first zero or null terminator.
	*/
	int getByteCount() const;
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
	char getCharAt(int index) const;
	/**
		Return a pointer to the (null-terminated) string. Note that this is
		a const pointer: do NOT change this! It is imperative that it be
		impossible (or at least, really difficuly) for someone to change our
		private data, since it might be shared amongst other AsciiStrings.
	*/
	const char* str() const;

	/**
		Makes sure there is room for a string of len+1 characters, and
		returns a pointer to the string buffer.  This ensures that the
		string buffer is NOT shared.  This is intended for the file reader,
		that is reading new strings in from a file. jba.
	*/
	char* getBufferForRead(Int len);

	/**
		Replace the contents of self with the given string.
		(This is actually quite efficient, because
		they will simply share the same string and increment the
		refcount.)
	*/
	void set(const AsciiString& stringSrc);

	/**
		Replace the contents of self with the given string.
		Note that a copy of the string is made; the input ptr is not saved.
	*/
	void set(const char* s);

	/**
		Replace the contents of self with the given string and length.
		Note that a copy of the string is made; the input ptr is not saved.
		The length must not be larger than the actual string length.
	*/
	void set(const char* s, int len);

	/**
		replace contents of self with the given string. Note the
		nomenclature is translate rather than set; this is because
		not all single-byte strings translate one-for-one into
		UnicodeStrings, so some data manipulation may be necessary,
		and the resulting strings may not be equivalent.
	*/
	void translate(const UnicodeString& stringSrc);

	/**
		Concatenate the given string onto self.
	*/
	void concat(const AsciiString& stringSrc);
	/**
		Concatenate the given string onto self.
	*/
	void concat(const char* s);
	/**
		Concatenate the given character onto self.
	*/
	void concat(const char c);

	/**
	  Remove leading and trailing whitespace from the string.
	*/
	void trim( void );

	/**
	  Remove trailing whitespace from the string.
	*/
	void trimEnd(void);

	/**
	  Remove all consecutive occurances of c from the end of the string.
	*/
	void trimEnd(const char c);

	/**
	  Make the string lowercase
	*/
	void toLower( void );

	/**
		Remove the final character in the string. If the string is empty,
		do nothing. (This is a rather dorky method, but used a lot in
		text editing, thus its presence here.)
	*/
	void removeLastChar();

	/**
		Remove the final charCount characters in the string. If the string is empty,
		do nothing.
	*/
	void truncateBy(const Int charCount);

	/**
		Truncate the string to a length of maxLength characters, not including null termination,
		by removing from the end. If the string is empty or shorter than maxLength, do nothing.
	*/
	void truncateTo(const Int maxLength);

	/**
		Analogous to sprintf() -- this formats a string according to the
		given sprintf-style format string (and the variable argument list)
		and stores the result in self.
	*/
	void format(AsciiString format, ...);
	void format(const char* format, ...);
	/**
		Identical to format(), but takes a va_list rather than
		a variable argument list. (i.e., analogous to vsprintf.)
	*/
	void format_va(const AsciiString& format, va_list args);
	void format_va(const char* format, va_list args);

	/**
		Conceptually identical to strcmp().
	*/
	int compare(const AsciiString& stringSrc) const;
	/**
		Conceptually identical to strcmp().
	*/
	int compare(const char* s) const;
	/**
		Conceptually identical to _stricmp().
	*/
	int compareNoCase(const AsciiString& stringSrc) const;
	/**
		Conceptually identical to _stricmp().
	*/
	int compareNoCase(const char* s) const;

	/**
		Conceptually identical to strchr().
	*/
	const char* find(char c) const;

	/**
		Conceptually identical to strrchr().
	*/
	const char* reverseFind(char c) const;

	/**
		return true iff self starts with the given string.
	*/
	Bool startsWith(const char* p) const;
	inline Bool startsWith(const AsciiString& stringSrc) const { return startsWith(stringSrc.str()); }

	/**
		return true iff self starts with the given string. (case insensitive)
	*/
	Bool startsWithNoCase(const char* p) const;
	inline Bool startsWithNoCase(const AsciiString& stringSrc) const { return startsWithNoCase(stringSrc.str()); }

	/**
		return true iff self ends with the given string.
	*/
	Bool endsWith(const char* p) const;
	Bool endsWith(const AsciiString& stringSrc) const { return endsWith(stringSrc.str()); }

	/**
		return true iff self ends with the given string. (case insensitive)
	*/
	Bool endsWithNoCase(const char* p) const;
	Bool endsWithNoCase(const AsciiString& stringSrc) const { return endsWithNoCase(stringSrc.str()); }

	/**
		conceptually similar to strtok():

		extract the next seps-delimited token from the front
		of 'this' and copy it into 'token', returning true if a nonempty
		token was found. (note that this modifies 'this' as well, stripping
		the token off!)
	*/
	Bool nextToken(AsciiString* token, const char* seps = NULL);

	/**
		return true iff the string is "NONE" (case-insensitive).
		Hey, hokey, but we use it a ton.
	*/
	Bool isNone() const;

	Bool isNotEmpty() const { return !isEmpty(); }
	Bool isNotNone() const { return !isNone(); }

//
// You might think it would be a good idea to overload the * operator
// to allow for an implicit conversion to an char*. This is
// (in theory) a good idea, but in practice, there's lots of code
// that assumes it should check text fields for null, which
// is meaningless for us, since we never return a null ptr.
//
//	operator const char*() const { return str(); }
//

	AsciiString& operator=(const AsciiString& stringSrc);	///< the same as set()
	AsciiString& operator=(const char* s);				///< the same as set()

	void debugIgnoreLeaks();

};

// -----------------------------------------------------
inline char* AsciiString::peek() const
{
	DEBUG_ASSERTCRASH(m_data, ("null string ptr"));
	validate();
	return m_data->peek();
}

// -----------------------------------------------------
inline AsciiString::AsciiString() : m_data(0)
{
	validate();
}

// -----------------------------------------------------
inline AsciiString::~AsciiString()
{
	validate();
	releaseBuffer();
}

// -----------------------------------------------------
inline int AsciiString::getLength() const
{
	validate();
	return m_data ? strlen(peek()) : 0;
}

// -----------------------------------------------------
inline int AsciiString::getByteCount() const
{
	validate();
	return m_data ? getLength() : 0;
}

// -----------------------------------------------------
inline Bool AsciiString::isEmpty() const
{
	validate();
	return m_data == NULL || peek()[0] == 0;
}

// -----------------------------------------------------
inline void AsciiString::clear()
{
	validate();
	releaseBuffer();
	validate();
}

// -----------------------------------------------------
inline char AsciiString::getCharAt(int index) const
{
	DEBUG_ASSERTCRASH(index >= 0 && index < getLength(), ("bad index in getCharAt"));
	validate();
	return m_data ? peek()[index] : 0;
}

// -----------------------------------------------------
inline const char* AsciiString::str() const
{
	validate();
	static const char TheNullChr = 0;
	return m_data ? peek() : &TheNullChr;
}

// -----------------------------------------------------
inline AsciiString& AsciiString::operator=(const AsciiString& stringSrc)
{
	validate();
	set(stringSrc);
	validate();
	return *this;
}

// -----------------------------------------------------
inline AsciiString& AsciiString::operator=(const char* s)
{
	validate();
	set(s);
	validate();
	return *this;
}

// -----------------------------------------------------
inline void AsciiString::concat(const AsciiString& stringSrc)
{
	validate();
	concat(stringSrc.str());
	validate();
}

// -----------------------------------------------------
inline void AsciiString::concat(const char c)
{
	validate();
	/// this can probably be made more efficient, if necessary
	char tmp[2] = { c, 0 };
	concat(tmp);
	validate();
}

// -----------------------------------------------------
inline int AsciiString::compare(const AsciiString& stringSrc) const
{
	validate();
	return strcmp(this->str(), stringSrc.str());
}

// -----------------------------------------------------
inline int AsciiString::compare(const char* s) const
{
	validate();
	return strcmp(this->str(), s);
}

// -----------------------------------------------------
inline int AsciiString::compareNoCase(const AsciiString& stringSrc) const
{
	validate();
	return _stricmp(this->str(), stringSrc.str());
}

// -----------------------------------------------------
inline int AsciiString::compareNoCase(const char* s) const
{
	validate();
	return _stricmp(this->str(), s);
}

// -----------------------------------------------------
inline const char* AsciiString::find(char c) const
{
	return strchr(this->str(), c);
}

// -----------------------------------------------------
inline const char* AsciiString::reverseFind(char c) const
{
	return strrchr(this->str(), c);
}

// -----------------------------------------------------
inline Bool operator==(const AsciiString& s1, const AsciiString& s2)
{
	return strcmp(s1.str(), s2.str()) == 0;
}

// -----------------------------------------------------
inline Bool operator!=(const AsciiString& s1, const AsciiString& s2)
{
	return strcmp(s1.str(), s2.str()) != 0;
}

// -----------------------------------------------------
inline Bool operator<(const AsciiString& s1, const AsciiString& s2)
{
	return strcmp(s1.str(), s2.str()) < 0;
}

// -----------------------------------------------------
inline Bool operator<=(const AsciiString& s1, const AsciiString& s2)
{
	return strcmp(s1.str(), s2.str()) <= 0;
}

// -----------------------------------------------------
inline Bool operator>(const AsciiString& s1, const AsciiString& s2)
{
	return strcmp(s1.str(), s2.str()) > 0;
}

// -----------------------------------------------------
inline Bool operator>=(const AsciiString& s1, const AsciiString& s2)
{
	return strcmp(s1.str(), s2.str()) >= 0;
}

// -----------------------------------------------------
inline Bool operator==(const AsciiString& s1, const char* s2)
{
	return strcmp(s1.str(), s2) == 0;
}

// -----------------------------------------------------
inline Bool operator!=(const AsciiString& s1, const char* s2)
{
	return strcmp(s1.str(), s2) != 0;
}

// -----------------------------------------------------
inline Bool operator<(const AsciiString& s1, const char* s2)
{
	return strcmp(s1.str(), s2) < 0;
}

// -----------------------------------------------------
inline Bool operator<=(const AsciiString& s1, const char* s2)
{
	return strcmp(s1.str(), s2) <= 0;
}

// -----------------------------------------------------
inline Bool operator>(const AsciiString& s1, const char* s2)
{
	return strcmp(s1.str(), s2) > 0;
}

// -----------------------------------------------------
inline Bool operator>=(const AsciiString& s1, const char* s2)
{
	return strcmp(s1.str(), s2) >= 0;
}

#endif // ASCIISTRING_H
