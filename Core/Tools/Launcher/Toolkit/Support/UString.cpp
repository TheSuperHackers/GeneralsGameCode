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

#include "Visualc.h"
#include "UString.h"
#include "StringConvert.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// Convert character to lowercase
template<typename T> T CharToLower(const T ch)
	{
	if ((ch >= (T)'A') && (ch <= (T)'Z'))
		{
		return (ch + 32);
		}

	return ch;
	}

// Convert character to uppercase
template<typename T> T CharToUpper(const T ch)
	{
	if ((ch >= (T)'a') && (ch <= (T)'z'))
		{
		return (ch - 32);
		}

	return ch;
	}


// Check if character is one of the specified characters
template<typename T>bool IsCharacter(WChar ch, const T* oneOf)
	{
	assert(oneOf != NULL);

	int length = 0;

	while (oneOf[length] != 0)
		{
		length++;
		}

	for (int index = 0; index < length; index++)
		{
		if (ch == (WChar)oneOf[index])
			{
			return true;
			}
		}

	return false;
	}


// Strip all left side characters that are trim chars
template<typename T> bool StripLeft(WChar* string, const T* trimChars)
	{
	// Strip leading trim characters from the string.
	WChar* start = string;

	while ((*start != 0) && IsCharacter<T>(*start, trimChars))
		{
		start++;
		}

	if (start != string)
		{
		wcscpy(string, start);
		}

	return true;
	}


// Strip all right side characters that are trim chars
template<typename T> bool StripRight(WChar* string, const T* trimChars)
	{
	int length = wcslen(string) - 1;
	int index = length;

	while (index >= 0)
		{
		if (!IsCharacter<T>(string[index], trimChars))
			{
			break;
			}

		string[index] = 0;
		index--;
		}

	return (index != length);
	}


/******************************************************************************
*
* NAME
*     UString::UString - Default constructor
*
* DESCRIPTION
*     Create a new empty UString.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

UString::UString()
	: mData(NULL),
	  mCapacity(0)
	{
	}


/******************************************************************************
*
* NAME
*     UString::UString - Capacity contructor
*
* DESCRIPTION
*     Create a new empty UString with the specified capacity.
*
* INPUTS
*     Capacity - Number of characters to allocated to string.
*
* RESULT
*     NONE
*
******************************************************************************/

UString::UString(UInt capacity)
	: mData(NULL),
	  mCapacity(0)
	{
	AllocString(capacity);
	}


/******************************************************************************
*
* NAME
*    UString::UString - ANSI string literal constructor
*
* DESCRIPTION
*     Create a new UString from an ANSI string literal
*
* INPUTS
*     String - Pointer to a NULL terminated ANSI string
*
* RESULT
*     NONE
*
******************************************************************************/

UString::UString(const Char* s)
	: mData(NULL),
	  mCapacity(0)
	{
	Copy(s);
	}


/******************************************************************************
*
* NAME
*    UString::UString - UNICODE string literal constructor
*
* DESCRIPTION
*     Create a new UString from a UNICODE string literal
*
* INPUTS
*     String - Pointer to a NULL terminated UNICODE string
*
* RESULT
*     NONE
*
******************************************************************************/

UString::UString(const WChar* ws)
	: mData(NULL),
	  mCapacity(0)
	{
	Copy(ws);
	}


/******************************************************************************
*
* NAME
*     UString::UString - Copy constructor
*
* DESCRIPTION
*     Create a new UString from another UString
*
* INPUTS
*     String - Reference to UString to copy
*
* RESULT
*     NONE
*
******************************************************************************/

UString::UString(const UString& s)
	: mData(NULL),
	  mCapacity(0)
	{
	Copy(s);
	}


/******************************************************************************
*
* NAME
*     UString::~UString - Destructor
*
* DESCRIPTION
*     Destroy the object
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

UString::~UString()
	{
	if (mData != NULL)
		{
		delete mData;
		}
	}


/******************************************************************************
*
* NAME
*     UString::Length
*
* DESCRIPTION
*     Retrieve the length of the string in characters.
*
* INPUTS
*     NONE
*
* RESULT
*     Length - Length of string
*
******************************************************************************/

UInt UString::Length(void) const
	{
	if (mData == NULL)
		{
		return 0;
		}

	return wcslen(mData);
	}


/******************************************************************************
*
* NAME
*     UString::Copy - ANSI string literal
*
* DESCRIPTION
*     Copy ANSI string literal.
*
* INPUTS
*     String - Pointer to ANSI string literal
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::Copy(const Char* s)
	{
	assert(s != NULL);
	UInt length = strlen(s);

	if (length == 0)
		{
		return;
		}

	if (Capacity() < length)
		{
		AllocString(length);
		}

	// Copy and convert ansi string to unicode
	assert(Capacity() >= length);
	WChar* wsPtr = mData;
	const Char* sPtr = s;

	while (length-- > 0)
		{
		*wsPtr++ = (WChar)*sPtr++;
		}

	*wsPtr = 0;
	}


/******************************************************************************
*
* NAME
*     UString::Copy - Unicode string literal
*
* DESCRIPTION
*     Copy a Unicode string literal
*
* INPUTS
*     String - Pointer to UNICODE string literal
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::Copy(const WChar* ws)
	{
	assert(ws != NULL);
	UInt length = wcslen(ws);

	if (length == 0)
		{
		return;
		}

	if (Capacity() < length)
		{
		AllocString(length);
		}

	assert(Capacity() >= length);
	wcscpy(mData, ws);
	}


/******************************************************************************
*
* NAME
*     UString::Copy
*
* DESCRIPTION
*     Copy string
*
* INPUTS
*     String - Reference to UString
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::Copy(const UString& s)
	{
	Copy(s.Get());
	}


/******************************************************************************
*
* NAME
*     UString::Concat - ANSI string literal
*
* DESCRIPTION
*     Concatenate an ANSI string literal to this string
*
* INPUTS
*     String - Pointer to ANSI string literal
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::Concat(const Char* s)
	{
	// Parameter check
	assert(s != NULL);

	UInt length = Length();
	UInt additional = strlen(s);
	UInt totalLength = (length + additional);

	// Resize the string if the combined size is to small
	if (Capacity() < totalLength)
		{
		Resize(totalLength);
		}

	// Concatenate and convert ansi string to unicode
	WChar* wsPtr = &mData[length];
	const Char* sPtr = s;

	while (additional-- > 0)
		{
		*wsPtr++ = (WChar)*sPtr++;
		}

	*wsPtr = 0;
	}


/******************************************************************************
*
* NAME
*     UString::Concat - Unicode string literal
*
* DESCRIPTION
*     Concatenate a Unicode string literal to this string
*
* INPUTS
*     String - Pointer to UNICODE string literal
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::Concat(const WChar* ws)
	{
	assert(ws != NULL);
	UInt length = (Length() + wcslen(ws));

	if (Capacity() < length)
		{
		Resize(length);
		}

	wcscat(mData, ws);
	}


/******************************************************************************
*
* NAME
*     UString::Concat
*
* DESCRIPTION
*     Concatenate string
*
* INPUTS
*     String - Reference to UString
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::Concat(const UString& s)
	{
	Concat(s.Get());
	}


/******************************************************************************
*
* NAME
*     UString::Compare - ANSI string literal
*
* DESCRIPTION
*     Compare strings, returning a value representing their relationship.
*
* INPUTS
*     String - String to compare against
*
* RESULT
*     Relationship - < 0 String less than this.
*                    = 0 Strings identical
*                    > 0 String greater than this.
*
******************************************************************************/

Int UString::Compare(const Char* s) const
	{
	// If comparing string is NULL and this string is NULL then strings are equal,
	// otherwise comparing string is less than this string.
	if (s == NULL)
		{
		if (Get() == NULL)
			{
			return 0;
			}

		return -1;
		}

	// If this string is NULL then comparing string is greater
	if (Get() == NULL)
		{
		return 1;
		}

	// Compare each character
	const WChar* ws = Get();
	Int index = 0;

	for (;;)
		{
		// Difference between characters
		Int diff = ((WChar)s[index] - ws[index]);

		// If the difference is not zero then the characters differ
		if (diff != 0)
			{
			return diff;
			}

		// If the end of either string has been reached then terminate loop
		if ((ws[index] == L'\0') || (s[index] == '\0'))
			{
			break;
			}

		// Advance to next character
		index++;
		}

	return 0;
	}


/******************************************************************************
*
* NAME
*     UString::Compare - Unicode string literal
*
* DESCRIPTION
*     Compare strings, returning a value representing their relationship.
*
* INPUTS
*     String - String to compare against
*
* RESULT
*     Relationship - < 0 String less than this.
*                    = 0 Strings identical
*                    > 0 String greater than this.
*
******************************************************************************/

Int UString::Compare(const WChar* ws) const
	{
	return wcscmp(ws, Get());
	}


/******************************************************************************
*
* NAME
*     UString::Compare
*
* DESCRIPTION
*     Compare strings, returning a value representing their relationship.
*
* INPUTS
*     String - String to compare against
*
* RESULT
*     Relationship - < 0 String less than this.
*                    = 0 Strings identical
*                    > 0 String greater than this.
*
******************************************************************************/

Int UString::Compare(const UString& s) const
	{
	return Compare(s.Get());
	}


/******************************************************************************
*
* NAME
*     UString::CompareNoCase - ANSI string literal
*
* DESCRIPTION
*     Compare strings (case insensitive), returning a value representing their
*     relationship.
*
* INPUTS
*     String - String to compare against
*
* RESULT
*     Relationship - < 0 String less than this.
*                    = 0 Strings identical
*                    > 0 String greater than this.
*
******************************************************************************/

Int UString::CompareNoCase(const Char* s) const
	{
	// If comparing string is NULL and this string is NULL then strings are
	// equal, otherwise comparing string is less than this string.
	if (s == NULL)
		{
		if (Get() == NULL)
			{
			return 0;
			}

		return -1;
		}

	// If this string is NULL then comparing string is greater.
	if (Get() == NULL)
		{
		return 1;
		}

	// Compare each character
	const WChar* ws = Get();
	Int index = 0;

	for (;;)
		{
		// Convert to lowercase for compare
		WChar sc = (WChar)CharToLower<Char>(s[index]);
		WChar wc = CharToLower<WChar>(ws[index]);

		// Difference between characters.
		Int diff = (sc - wc);

		// If the difference is not zero then the characters differ.
		if (diff != 0)
			{
			return diff;
			}

		// If the end of either string has been reached then terminate loop.
		if ((ws[index] == L'\0') || (s[index] == '\0'))
			{
			break;
			}

		// Advance to next character
		index++;
		}

	return 0;
	}


/******************************************************************************
*
* NAME
*     UString::CompareNoCase - Unicode string literal
*
* DESCRIPTION
*     Compare strings (case insensitive), returning a value representing their
*     relationship.
*
* INPUTS
*     String - String to compare against
*
* RESULT
*     Relationship - < 0 String less than this.
*                    = 0 Strings identical
*                    > 0 String greater than this.
*
******************************************************************************/

Int UString::CompareNoCase(const WChar* ws) const
	{
	return wcsicmp(ws, Get());
	}


/******************************************************************************
*
* NAME
*     UString::CompareNoCase
*
* DESCRIPTION
*     Compare strings (case insensitive), returning a value representing their
*     relationship.
*
* INPUTS
*     String - String to compare against
*
* RESULT
*     Relationship - < 0 String less than this.
*                    = 0 Strings identical
*                    > 0 String greater than this.
*
******************************************************************************/

Int UString::CompareNoCase(const UString& s) const
	{
	return CompareNoCase(s.Get());
	}


/******************************************************************************
*
* NAME
*     UString::Find - ANSI character
*
* DESCRIPTION
*     Find the first occurance of character
*
* INPUTS
*     Char - ANSI character to search for
*
* RESULT
*     Position - Position of character (-1 if not found)
*
******************************************************************************/

Int UString::Find(Char c) const
	{
	return Find((WChar)c);
	}


/******************************************************************************
*
* NAME
*     UString::Find - Unicode character
*
* DESCRIPTION
*     Find the first occurance of character
*
* INPUTS
*     Char - Unicode character to search for.
*
* RESULT
*     Position - Position of character (-1 if not found)
*
******************************************************************************/

Int UString::Find(WChar c) const
	{
	const WChar* ptr = wcschr(Get(), c);

	// Not found?
	if (ptr == NULL)
		{
		return -1;
		}

	return ((ptr - mData) / sizeof(WChar));
	}


/******************************************************************************
*
* NAME
*     UString::FindLast - ANSI character
*
* DESCRIPTION
*     Find the last occurance of a character
*
* INPUTS
*     Char - ANSI character
*
* RESULT
*     Position - Position of character (-1 if not found)
*
******************************************************************************/

Int UString::FindLast(Char c) const
	{
	return FindLast((WChar)c);
	}


/******************************************************************************
*
* NAME
*     UString::FindLast - Unicode character
*
* DESCRIPTION
*     Find the last occurance of a character
*
* INPUTS
*     Char - Unicode character
*
* RESULT
*     Position - Position of character (-1 if not found)
*
******************************************************************************/

Int UString::FindLast(WChar c) const
	{
	assert(mData != NULL);
	WChar* ptr = wcsrchr(mData, (WChar)c);

	// Not found?
	if (ptr == NULL)
		{
		return -1;
		}

	return ((ptr - mData) / sizeof(WChar));
	}


/******************************************************************************
*
* NAME
*     UString::SubString - ANSI string literal
*
* DESCRIPTION
*     Find a substring
*
* INPUTS
*     SubString - Substring to search for.
*
* RESULT
*
*
******************************************************************************/

UString UString::SubString(const Char* s)
	{
	assert(false);
	assert(s != NULL);
	return UString("");
	}


UString UString::SubString(const WChar* ws)
	{
	assert(false);
	assert(ws != NULL);
	return UString("");
	}


UString UString::SubString(const UString& s)
	{
	assert(false);
	return SubString(s.mData);
	}


/******************************************************************************
*
* NAME
*     UString::Left
*
* DESCRIPTION
*     Extract left part of the string.
*
* INPUTS
*     Count - Number of characters to extract
*
* RESULT
*     Left - Extracted left part of the string
*
******************************************************************************/

UString UString::Left(UInt count)
	{
	assert(false);

	// If the count is zero then return an empty string.
	if ((Length() == 0) || (count == 0))
		{
		return UString("");
		}

	return UString("");
	}


/******************************************************************************
*
* NAME
*     UString::Middle
*
* DESCRIPTION
*     Extract middle part of the string.
*
* INPUTS
*     First - Position of first character
*     Count - Number of characters to extract
*
* RESULT
*     Middle - Extracted middle part of the string
*
******************************************************************************/

UString UString::Middle(UInt first, UInt count)
	{
	assert(false);

	// If the first character position is greater than the length of the string
	// or the count is zero then return an empty string.
	if ((Length() < first) || (count == 0))
		{
		return UString("");
		}

	return UString("");
	}


/******************************************************************************
*
* NAME
*     UString::Right
*
* DESCRIPTION
*     Extract right part of the string.
*
* INPUTS
*     Count - Number of characters to extract
*
* RESULT
*     Right - Extracted right part of the string
*
******************************************************************************/

UString UString::Right(UInt count)
	{
	UInt length = Length();

	// If the count is zero then return an empty string.
	if ((length == 0) || (count == 0))
		{
		return UString("");
		}

	const WChar* ptr = Get();
	UInt pos = (length - count);

	return UString(ptr[pos]);
	}


/******************************************************************************
*
* NAME
*     UString::ToUpper
*
* DESCRIPTION
*     Convert string to uppercase
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::ToUpper(void)
	{
	if (mData != NULL)
		{
		wcsupr(mData);
		}
	}


/******************************************************************************
*
* NAME
*     UString::ToLower
*
* DESCRIPTION
*     Convert string to lowercase
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::ToLower(void)
	{
	if (mData != NULL)
		{
		wcslwr(mData);
		}
	}


/******************************************************************************
*
* NAME
*     UString::Reverse
*
* DESCRIPTION
*     Reverse characters of string
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::Reverse(void)
	{
	if (mData != NULL)
		{
		wcsrev(mData);
		}
	}


/******************************************************************************
*
* NAME
*     UString::Trim
*
* DESCRIPTION
*     Remove leading and trailing characters from string.
*
* INPUTS
*     TrimChars - Characters to trim
*
* RESULT
*     Removed - True if any characters removed
*
******************************************************************************/

bool UString::Trim(const Char* trimChars)
	{
	bool leftRemoved = TrimLeft(trimChars);
	bool rightRemoved = TrimRight(trimChars);
	return (leftRemoved || rightRemoved);
	}


bool UString::Trim(const WChar* trimChars)
	{
	bool leftRemoved = TrimLeft(trimChars);
	bool rightRemoved = TrimRight(trimChars);
	return (leftRemoved || rightRemoved);
	}


bool UString::Trim(const UString& trimChars)
	{
	bool leftRemoved = TrimLeft(trimChars);
	bool rightRemoved = TrimRight(trimChars);
	return (leftRemoved || rightRemoved);
	}


/******************************************************************************
*
* NAME
*     UString::TrimLeft
*
* DESCRIPTION
*     Remove characters from left side of string
*
* INPUTS
*     TrimChars - Characters to trim
*
* RESULT
*     Removed - True if any characters removed
*
******************************************************************************/

bool UString::TrimLeft(const Char* trimChars)
	{
	if ((trimChars == NULL) || (strlen(trimChars) == 0))
		{
		return false;
		}

	return StripLeft<Char>(mData, trimChars);
	}


bool UString::TrimLeft(const WChar* trimChars)
	{
	if ((trimChars == NULL) || (wcslen(trimChars) == 0))
		{
		return false;
		}

	return StripLeft<WChar>(mData, trimChars);
	}


bool UString::TrimLeft(const UString& trimChars)
	{
	return TrimLeft(trimChars.Get());
	}


/******************************************************************************
*
* NAME
*     UString::TrimRight
*
* DESCRIPTION
*     Remove characters from right side of string
*
* INPUTS
*     TrimChars - Characters to trim
*
* RESULT
*     Removed - True if any characters removed
*
******************************************************************************/

bool UString::TrimRight(const Char* trimChars)
	{
	if ((trimChars == NULL) || (strlen(trimChars) == 0))
		{
		return false;
		}

	return StripRight<Char>(mData, trimChars);
	}


bool UString::TrimRight(const WChar* trimChars)
	{
	if ((trimChars == NULL) || (wcslen(trimChars) == 0))
		{
		return false;
		}

	return StripRight<WChar>(mData, trimChars);
	}


bool UString::TrimRight(const UString& trimChars)
	{
	return TrimRight(trimChars.mData);
	}


/******************************************************************************
*
* NAME
*     UString::ConvertToANSI
*
* DESCRIPTION
*     Convert the string to ANSI string
*
* INPUTS
*     Buffer - Buffer to convert into
*     Length - Maximum length of buffer
*
* RESULT
*     NONE
*
******************************************************************************/

void UString::ConvertToANSI(Char* buffer, UInt bufferLength) const
	{
	UStringToANSI(*this, buffer, bufferLength);
	}


/******************************************************************************
*
* NAME
*     UString::Size
*
* DESCRIPTION
*     Retrieve the size of the string in bytes
*
* INPUTS
*     NONE
*
* RESULT
*     Size - Size of string in bytes
*
******************************************************************************/

UInt UString::Size(void) const
	{
	if (mData == NULL)
		{
		return 0;
		}

	return ((Length() + 1) * sizeof(WChar));
	}


/******************************************************************************
*
* NAME
*     UString::Capacity
*
* DESCRIPTION
*     Retrieve the capacity (maximum number of characters) of the string.
*
* INPUTS
*     NONE
*
* RESULT
*     Capacity - Maximum number of characters that string can hold.
*
******************************************************************************/

UInt UString::Capacity(void) const
	{
	return mCapacity;
	}


/******************************************************************************
*
* NAME
*     UString::Resize
*
* DESCRIPTION
*     Resize the string capacity.
*
* INPUTS
*     NewSize - Size to resize string to.
*
* RESULT
*     Success - True if successful; otherwise false
*
******************************************************************************/

bool UString::Resize(UInt size)
	{
	// Allocate new storage
	assert(size > 0);
	WChar* data = new WChar[size + 1];
	assert(data != NULL);

	if (data == NULL)
		{
		return false;
		}

	// Copy existing string into new storage buffer
	if (mData != NULL)
		{
		UInt minSize = __min(Capacity(), size);
		wcsncpy(data, mData, minSize);
		data[minSize] = 0;
		delete mData;
		}

	// Set new storage
	mData = data;
	mCapacity = size;

	return true;
	}


/******************************************************************************
*
* NAME
*     UString::AllocString
*
* DESCRIPTION
*     Allocate string storage
*
* INPUTS
*     Size - Number of characters
*
* RESULT
*     Success - True if allocate successful; otherwise false
*
******************************************************************************/

bool UString::AllocString(UInt size)
	{
	WChar* data = new WChar[size + 1];
	assert(data != NULL);

	if (data == NULL)
		{
		return false;
		}

	data[0] = 0;

	if (mData != NULL)
		{
		delete mData;
		}

	mData = data;
	mCapacity = size;

	return true;
	}
