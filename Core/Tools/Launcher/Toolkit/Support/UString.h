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

/******************************************************************************
*
* FILE
*     $Archive:  $
*
* DESCRIPTION
*     String management class (Unicode)
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author:  $
*
* VERSION INFO
*     $Modtime:  $
*     $Revision:  $
*
******************************************************************************/

#pragma once

#include "UTypes.h"
#include "RefCounted.h"

class UString
	: public RefCounted
	{
	public:
		// Constructors
		UString();
		UString(UInt capacity);
		UString(const Char* s);
		UString(const WChar* ws);
		UString(const UString& s);
		virtual ~UString();

		//! Get the length of the string
		UInt Length(void) const;

		//! Copy string
		void Copy(const Char* s);
		void Copy(const WChar* ws);
		void Copy(const UString& s);

		//! Concatenate string
		void Concat(const Char* s);
		void Concat(const WChar* ws);
		void Concat(const UString& s);

		//! Compare strings
		Int Compare(const Char* s) const;
		Int Compare(const WChar* s) const;
		Int Compare(const UString& s) const;

		//! Compare strings not case-sensitive
		Int CompareNoCase(const Char* s) const;
		Int CompareNoCase(const WChar* ws) const;
		Int CompareNoCase(const UString& s) const;

		//! Find the first occurance of character
		Int Find(Char c) const;
		Int Find(WChar wc) const;

		//! Find the last occurance of a character
		Int FindLast(Char c) const;
		Int FindLast(WChar c) const;

		//! Find a substring
		UString SubString(const Char* s);
		UString SubString(const WChar* ws);
		UString SubString(const UString& s);

		//! Extract left part of the string
		UString Left(UInt count);

		//! Extract middle part of the string.
		UString Middle(UInt first, UInt count);

		//! Extract right part of the string.
		UString Right(UInt count);

		//! Convert string to uppercase
		void ToUpper(void);

		//! Convert string to lowercase
		void ToLower(void);

		//! Reverse characters of string
		void Reverse(void);

		//! Remove leading and trailing characters from string.
		//  Returns true if any characters removed
		bool Trim(const Char* trimChars);
		bool Trim(const WChar* trimChars);
		bool Trim(const UString& trimChars);

		//! Remove characters from left side of string
		//  Returns true if any characters removed
		bool TrimLeft(const Char* trimChars);
		bool TrimLeft(const WChar* trimChars);
		bool TrimLeft(const UString& trimChars);

		//! Remove characters from right side of string
		//  Returns true if any characters removed
		bool TrimRight(const Char* trimChars);
		bool TrimRight(const WChar* trimChars);
		bool TrimRight(const UString& trimChars);

		// Convert string to ANSI
		void ConvertToANSI(Char* buffer, UInt bufferLength) const;

		//! Get the size (in bytes) of the string.
		UInt Size(void) const;

		//! Get the maximum number of characters this string can hold.
		UInt Capacity(void) const;

		//! Resize the string
		bool Resize(UInt size);

		const WChar* Get(void) const
			{return (mData != NULL) ? mData : L"";}

		//! Assignment operator
		UString operator=(const Char* s)
			{Copy(s); return *this;};

		UString operator=(const WChar* ws)
			{Copy(ws); return *this;};

		UString operator=(const UString& s)
			{Copy(s); return *this;};

		//! Addition operator (concatenate)
		UString operator+(const Char* s)
			{UString ns(*this); ns += s; return ns;}

		UString operator+(const WChar* ws)
			{UString ns(*this); ns += ws; return ns;}

		UString operator+(const UString& s)
			{UString ns(*this); ns += s; return ns;}

		UString operator+=(const Char* s)
			{Concat(s); return *this;}

		UString operator+=(const WChar* ws)
			{Concat(ws); return *this;}

		UString operator+=(const UString& s)
			{Concat(s); return *this;}

		//! Equal operator (case sensitive compare)
		bool operator==(const Char* s)
			{return (Compare(s) == 0);}

		bool operator==(const WChar* ws)
			{return (Compare(ws) == 0);}

		bool operator==(const UString& s)
			{return (Compare(s) == 0);}

		bool operator!=(const Char* s)
			{return (Compare(s) != 0);}

		bool operator!=(const WChar* ws)
			{return (Compare(ws) != 0);}

		bool operator!=(const UString& s)
			{return (Compare(s) != 0);}

		//! Less than operator (case sensitive compare)
		bool operator<(const Char* s)
			{return (Compare(s) == -1);}

		bool operator<(const WChar* ws)
			{return (Compare(ws) == -1);}

		bool operator<(const UString& s)
			{return (Compare(s) == -1);}

		bool operator<=(const Char* s)
			{return (Compare(s) <= 0);}

		bool operator<=(const WChar* ws)
			{return (Compare(ws) <= 0);}

		bool operator<=(const UString& s)
			{return (Compare(s) <= 0);}

		//! Greater than operator (case sensitive compare)
		bool operator>(const Char* s)
			{return (Compare(s) == 1);}

		bool operator>(const WChar* ws)
			{return (Compare(ws) == 1);}

		bool operator>(const UString& s)
			{return (Compare(s) == 1);}

		bool operator>=(const Char* s)
			{return (Compare(s) >= 0);}

		bool operator>=(const WChar* ws)
			{return (Compare(ws) >= 0);}

		bool operator>=(const UString& s)
			{return (Compare(s) >= 0);}

		// Conversion operator
		operator const WChar*() const
			{return Get();}

	private:
		bool AllocString(UInt size);

		WChar* mData;
		UInt mCapacity;
	};
