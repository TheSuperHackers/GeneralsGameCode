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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/widestring.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/06/02 4:59p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WIDESTRING_H
#define __WIDESTRING_H

#include <string.h>
#include <stdarg.h>
#include "always.h"
#include "wwdebug.h"
#include "win.h"
#include "wwstring.h"
#include "trim.h"
#include <wchar.h>

//////////////////////////////////////////////////////////////////////
//
//	WideStringClass
//
//	This is a UNICODE (double-byte) version of StringClass.  All
//	operations are performed on wide character strings.
//
//////////////////////////////////////////////////////////////////////
class WideStringClass
{
public:

	////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////
	WideStringClass (int initial_len = 0,				bool hint_temporary = false);
	WideStringClass (const WideStringClass &string,	bool hint_temporary = false);
	WideStringClass (const WCHAR *string,				bool hint_temporary = false);
	WideStringClass (WCHAR ch,								bool hint_temporary = false);
	WideStringClass (const char *string,				bool hint_temporary = false);
	~WideStringClass (void);

	////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////
	bool operator== (const WCHAR *rvalue) const;
	bool operator!= (const WCHAR *rvalue) const;

	inline const WideStringClass &operator= (const WideStringClass &string);
	inline const WideStringClass &operator= (const WCHAR *string);
	inline const WideStringClass &operator= (WCHAR ch);
	inline const WideStringClass &operator= (const char *string);

	const WideStringClass &operator+= (const WideStringClass &string);
	const WideStringClass &operator+= (const WCHAR *string);
	const WideStringClass &operator+= (WCHAR ch);

	friend WideStringClass operator+ (const WideStringClass &string1, const WideStringClass &string2);
	friend WideStringClass operator+ (const WCHAR *string1, const WideStringClass &string2);
	friend WideStringClass operator+ (const WideStringClass &string1, const WCHAR *string2);

	bool operator < (const WCHAR *string) const;
	bool operator <= (const WCHAR *string) const;
	bool operator > (const WCHAR *string) const;
	bool operator >= (const WCHAR *string) const;

	WCHAR operator[] (int index) const;
	WCHAR& operator[] (int index);
	operator const WCHAR * (void) const;

	////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////
	int			Compare (const WCHAR *string) const;
	int			Compare_No_Case (const WCHAR *string) const;

	inline int	Get_Length (void) const;
	bool			Is_Empty (void) const;

	void			Erase (int start_index, int char_count);
	int _cdecl  Format (const WCHAR *format, ...);
	int _cdecl  Format_Args (const WCHAR *format, va_list arg_list );
	bool			Convert_From (const char *text);
	bool			Convert_To (StringClass &string);
	bool			Convert_To (StringClass &string) const;

	// Trim leading and trailing whitespace (chars <= 32)
	void Trim(void);

	// Check if the string is composed of ANSI range characters. (0-255)
	bool Is_ANSI(void);

	WCHAR *		Get_Buffer (int new_length);
	WCHAR *		Peek_Buffer (void);

	////////////////////////////////////////////////////////////
	//	Static methods
	////////////////////////////////////////////////////////////
	static void	Release_Resources (void);

private:

	////////////////////////////////////////////////////////////
	//	Private structures
	////////////////////////////////////////////////////////////
	typedef struct _HEADER
	{
		int	allocated_length;
		int	length;
	} HEADER;

	////////////////////////////////////////////////////////////
	//	Private constants
	////////////////////////////////////////////////////////////
	enum
	{
		MAX_TEMP_STRING	= 4,
		MAX_TEMP_LEN		= 256,
		MAX_TEMP_BYTES		= (MAX_TEMP_LEN * sizeof (WCHAR)) + sizeof (HEADER),
	};

	////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////
	void			Get_String (int length, bool is_temp);
	WCHAR *		Allocate_Buffer (int length);
	void			Resize (int size);
	void			Uninitialised_Grow (int length);
	void			Free_String (void);

	inline void	Store_Length (int length);
	inline void	Store_Allocated_Length (int allocated_length);
	inline HEADER * Get_Header (void) const;
	int			Get_Allocated_Length (void) const;

	void			Set_Buffer_And_Allocated_Length (WCHAR *buffer, int length);

	////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////
	WCHAR *		m_Buffer;

	////////////////////////////////////////////////////////////
	//	Static member data
	////////////////////////////////////////////////////////////
	static char		m_TempString1[MAX_TEMP_BYTES];
	static char		m_TempString2[MAX_TEMP_BYTES];
	static char		m_TempString3[MAX_TEMP_BYTES];
	static char		m_TempString4[MAX_TEMP_BYTES];
	static WCHAR *	m_FreeTempPtr[MAX_TEMP_STRING];
	static WCHAR *	m_ResTempPtr[MAX_TEMP_STRING];

	static int		m_UsedTempStringCount;
	static FastCriticalSectionClass m_TempMutex;

	static WCHAR	m_NullChar;
	static WCHAR *	m_EmptyString;
};

///////////////////////////////////////////////////////////////////
//	WideStringClass
///////////////////////////////////////////////////////////////////
inline
WideStringClass::WideStringClass (int initial_len, bool hint_temporary)
	:	m_Buffer (m_EmptyString)
{
	Get_String (initial_len, hint_temporary);
	m_Buffer[0]	= m_NullChar;

	return ;
}

///////////////////////////////////////////////////////////////////
//	WideStringClass
///////////////////////////////////////////////////////////////////
inline
WideStringClass::WideStringClass (WCHAR ch, bool hint_temporary)
	:	m_Buffer (m_EmptyString)
{
	Get_String (2, hint_temporary);
	(*this) = ch;
	return ;
}

///////////////////////////////////////////////////////////////////
//	WideStringClass
///////////////////////////////////////////////////////////////////
inline
WideStringClass::WideStringClass (const WideStringClass &string, bool hint_temporary)
 	:	m_Buffer (m_EmptyString)
{
	if (hint_temporary || (string.Get_Length()>1)) {
		Get_String(string.Get_Length()+1, hint_temporary);
	}

	(*this) = string;
	return ;
}

///////////////////////////////////////////////////////////////////
//	WideStringClass
///////////////////////////////////////////////////////////////////
inline
WideStringClass::WideStringClass (const WCHAR *string, bool hint_temporary)
	:	m_Buffer (m_EmptyString)
{
	int len=string ? wcslen(string) : 0;
	if (hint_temporary || len>0) {
		Get_String (len+1, hint_temporary);
	}

	(*this) = string;
	return ;
}


///////////////////////////////////////////////////////////////////
//	WideStringClass
///////////////////////////////////////////////////////////////////
inline
WideStringClass::WideStringClass (const char *string, bool hint_temporary)
	:	m_Buffer (m_EmptyString)
{
	if (hint_temporary || (string && strlen(string)>0)) {
		Get_String (strlen(string) + 1, hint_temporary);
	}

	(*this) = string;
	return ;
}

///////////////////////////////////////////////////////////////////
//	~WideStringClass
///////////////////////////////////////////////////////////////////
inline
WideStringClass::~WideStringClass (void)
{
	Free_String ();
	return ;
}


///////////////////////////////////////////////////////////////////
//	Is_Empty
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::Is_Empty (void) const
{
	return (m_Buffer[0] == m_NullChar);
}

///////////////////////////////////////////////////////////////////
//	Compare
///////////////////////////////////////////////////////////////////
inline int
WideStringClass::Compare (const WCHAR *string) const
{
	if (string) {
		return wcscmp (m_Buffer, string);
	}

	return -1;
}

///////////////////////////////////////////////////////////////////
//	Compare_No_Case
///////////////////////////////////////////////////////////////////
inline int
WideStringClass::Compare_No_Case (const WCHAR *string) const
{
	if (string) {
		return _wcsicmp (m_Buffer, string);
	}

	return -1;
}

///////////////////////////////////////////////////////////////////
//	operator[]
///////////////////////////////////////////////////////////////////
inline WCHAR
WideStringClass::operator[] (int index) const
{
	WWASSERT (index >= 0 && index < Get_Length ());
	return m_Buffer[index];
}

inline WCHAR&
WideStringClass::operator[] (int index)
{
	WWASSERT (index >= 0 && index < Get_Length ());
	return m_Buffer[index];
}

///////////////////////////////////////////////////////////////////
//	operator const WCHAR *
///////////////////////////////////////////////////////////////////
inline
WideStringClass::operator const WCHAR * (void) const
{
	return m_Buffer;
}

///////////////////////////////////////////////////////////////////
//	operator==
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::operator== (const WCHAR *rvalue) const
{
	return (Compare (rvalue) == 0);
}

///////////////////////////////////////////////////////////////////
//	operator!=
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::operator!= (const WCHAR *rvalue) const
{
	return (Compare (rvalue) != 0);
}

///////////////////////////////////////////////////////////////////
//	operator=
///////////////////////////////////////////////////////////////////
inline const WideStringClass &
WideStringClass::operator= (const WideStringClass &string)
{
	return operator= ((const WCHAR *)string);
}

///////////////////////////////////////////////////////////////////
//	operator <
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::operator < (const WCHAR *string) const
{
	if (string) {
		return (wcscmp (m_Buffer, string) < 0);
	}

	return false;
}

///////////////////////////////////////////////////////////////////
//	operator <=
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::operator <= (const WCHAR *string) const
{
	if (string) {
		return (wcscmp (m_Buffer, string) <= 0);
	}

	return false;
}

///////////////////////////////////////////////////////////////////
//	operator >
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::operator > (const WCHAR *string) const
{
	if (string) {
		return (wcscmp (m_Buffer, string) > 0);
	}

	return true;
}

///////////////////////////////////////////////////////////////////
//	operator >=
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::operator >= (const WCHAR *string) const
{
	if (string) {
		return (wcscmp (m_Buffer, string) >= 0);
	}

	return true;
}


///////////////////////////////////////////////////////////////////
//	Erase
///////////////////////////////////////////////////////////////////
inline void
WideStringClass::Erase (int start_index, int char_count)
{
	int len = Get_Length ();

	if (start_index < len) {

		if (start_index + char_count > len) {
			char_count = len - start_index;
		}

		::memmove (	&m_Buffer[start_index],
						&m_Buffer[start_index + char_count],
						(len - (start_index + char_count) + 1) * sizeof (WCHAR));

		Store_Length( wcslen(m_Buffer) );
	}

	return ;
}

///////////////////////////////////////////////////////////////////
// Trim leading and trailing whitespace (chars <= 32)
///////////////////////////////////////////////////////////////////
inline void WideStringClass::Trim(void)
{
	wcstrim(m_Buffer);
	int len = wcslen(m_Buffer);
	Store_Length(len);
}


///////////////////////////////////////////////////////////////////
//	operator=
///////////////////////////////////////////////////////////////////
inline const WideStringClass &
WideStringClass::operator= (const WCHAR *string)
{
	if (string) {
		int len = wcslen (string);
		Uninitialised_Grow (len + 1);
		Store_Length (len);

		::memcpy (m_Buffer, string, (len + 1) * sizeof (WCHAR));
	}

	return (*this);
}

///////////////////////////////////////////////////////////////////
//	operator=
///////////////////////////////////////////////////////////////////
inline const WideStringClass &
WideStringClass::operator= (const char *string)
{
	Convert_From(string);
	return (*this);
}

///////////////////////////////////////////////////////////////////
//	operator=
///////////////////////////////////////////////////////////////////
inline const WideStringClass &
WideStringClass::operator= (WCHAR ch)
{
	Uninitialised_Grow (2);

	m_Buffer[0] = ch;
	m_Buffer[1] = m_NullChar;
	Store_Length (1);

	return (*this);
}

///////////////////////////////////////////////////////////////////
//	operator+=
///////////////////////////////////////////////////////////////////
inline const WideStringClass &
WideStringClass::operator+= (const WCHAR *string)
{
	if (string) {
		int cur_len = Get_Length ();
		int src_len = wcslen (string);
		int new_len = cur_len + src_len;

		//
		//	Make sure our buffer is large enough to hold the new string
		//
		Resize (new_len + 1);
		Store_Length (new_len);

		//
		//	Copy the new string onto our the end of our existing buffer
		//
		::memcpy (&m_Buffer[cur_len], string, (src_len + 1) * sizeof (WCHAR));
	}

	return (*this);
}

///////////////////////////////////////////////////////////////////
//	operator+=
///////////////////////////////////////////////////////////////////
inline const WideStringClass &
WideStringClass::operator+= (WCHAR ch)
{
	int cur_len = Get_Length ();
	Resize (cur_len + 2);

	m_Buffer[cur_len]			= ch;
	m_Buffer[cur_len + 1]	= m_NullChar;

	if (ch != m_NullChar) {
		Store_Length (cur_len + 1);
	}

	return (*this);
}

///////////////////////////////////////////////////////////////////
//	Get_Buffer
///////////////////////////////////////////////////////////////////
inline WCHAR *
WideStringClass::Get_Buffer (int new_length)
{
	Uninitialised_Grow (new_length);

	return m_Buffer;
}

///////////////////////////////////////////////////////////////////
//	Peek_Buffer
///////////////////////////////////////////////////////////////////
inline WCHAR *
WideStringClass::Peek_Buffer (void)
{
	return m_Buffer;
}

///////////////////////////////////////////////////////////////////
//	operator+=
///////////////////////////////////////////////////////////////////
inline const WideStringClass &
WideStringClass::operator+= (const WideStringClass &string)
{
	int src_len = string.Get_Length();
	if (src_len > 0) {
		int cur_len = Get_Length ();
		int new_len = cur_len + src_len;

		//
		//	Make sure our buffer is large enough to hold the new string
		//
		Resize (new_len + 1);
		Store_Length (new_len);

		//
		//	Copy the new string onto our the end of our existing buffer
		//
		::memcpy (&m_Buffer[cur_len], (const WCHAR *)string, (src_len + 1) * sizeof (WCHAR));
	}

	return (*this);
}

///////////////////////////////////////////////////////////////////
//	operator+=
///////////////////////////////////////////////////////////////////
inline WideStringClass
operator+ (const WideStringClass &string1, const WideStringClass &string2)
{
	WideStringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

///////////////////////////////////////////////////////////////////
//	operator+=
///////////////////////////////////////////////////////////////////
inline WideStringClass
operator+ (const WCHAR *string1, const WideStringClass &string2)
{
	WideStringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

///////////////////////////////////////////////////////////////////
//	operator+=
///////////////////////////////////////////////////////////////////
inline WideStringClass
operator+ (const WideStringClass &string1, const WCHAR *string2)
{
	WideStringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

///////////////////////////////////////////////////////////////////
//	Get_Allocated_Length
//
//	Return allocated size of the string buffer
///////////////////////////////////////////////////////////////////
inline int
WideStringClass::Get_Allocated_Length (void) const
{
	int allocated_length = 0;

	//
	//	Read the allocated length from the header
	//
	if (m_Buffer != m_EmptyString) {
		HEADER *header		= Get_Header ();
		allocated_length	= header->allocated_length;
	}

	return allocated_length;
}

///////////////////////////////////////////////////////////////////
//	Get_Length
//
//	Return text legth. If length is not known calculate it, otherwise
// just return the previously stored value (strlen tends to take
// quite a lot cpu time if a lot of string combining operations are
// performed.
///////////////////////////////////////////////////////////////////
inline int
WideStringClass::Get_Length (void) const
{
	int length = 0;

	if (m_Buffer != m_EmptyString) {

		//
		//	Read the length from the header
		//
		HEADER *header	= Get_Header ();
		length			= header->length;

		//
		//	Hmmm, a zero length was stored in the header,
		// we better manually get the string length.
		//
		if (length == 0) {
			length = wcslen (m_Buffer);
			((WideStringClass *)this)->Store_Length (length);
		}
	}

	return length;
}

///////////////////////////////////////////////////////////////////
//	Set_Buffer_And_Allocated_Length
//
// Set buffer pointer and init size variable. Length is set to 0
// as the contents of the new buffer are not necessarily defined.
///////////////////////////////////////////////////////////////////
inline void
WideStringClass::Set_Buffer_And_Allocated_Length (WCHAR *buffer, int length)
{
	Free_String ();
	m_Buffer = buffer;

	//
	//	Update the header (if necessary)
	//
	if (m_Buffer != m_EmptyString) {
		Store_Allocated_Length (length);
		Store_Length (0);
	} else {
		WWASSERT (length == 0);
	}

	return ;
}

///////////////////////////////////////////////////////////////////
// Allocate_Buffer
///////////////////////////////////////////////////////////////////
inline WCHAR *
WideStringClass::Allocate_Buffer (int length)
{
	//
	//	Allocate a buffer that is 'length' characters long, plus the
	// bytes required to hold the header.
	//
	char *buffer = W3DNEWARRAY char[(sizeof (WCHAR) * length) + sizeof (WideStringClass::_HEADER)];

	//
	//	Fill in the fields of the header
	//
	HEADER *header					= reinterpret_cast<HEADER *>(buffer);
	header->length					= 0;
	header->allocated_length	= length;

	//
	//	Return the buffer as if it was a WCHAR pointer
	//
	return reinterpret_cast<WCHAR *>(buffer + sizeof (WideStringClass::_HEADER));
}

///////////////////////////////////////////////////////////////////
// Get_Header
///////////////////////////////////////////////////////////////////
inline WideStringClass::HEADER *
WideStringClass::Get_Header (void) const
{
	return reinterpret_cast<HEADER *>(((char *)m_Buffer) - sizeof (WideStringClass::_HEADER));
}

///////////////////////////////////////////////////////////////////
// Store_Allocated_Length
///////////////////////////////////////////////////////////////////
inline void
WideStringClass::Store_Allocated_Length (int allocated_length)
{
	if (m_Buffer != m_EmptyString) {
		HEADER *header					= Get_Header ();
		header->allocated_length	= allocated_length;
	} else {
		WWASSERT (allocated_length == 0);
	}

	return ;
}

///////////////////////////////////////////////////////////////////
// Store_Length
//
// Set length... The caller of this (private) function better
// be sure that the len is correct.
///////////////////////////////////////////////////////////////////
inline void
WideStringClass::Store_Length (int length)
{
	if (m_Buffer != m_EmptyString) {
		HEADER *header		= Get_Header ();
		header->length		= length;
	} else {
		WWASSERT (length == 0);
	}

	return ;
}

///////////////////////////////////////////////////////////////////
// Convert_To
///////////////////////////////////////////////////////////////////
inline bool
WideStringClass::Convert_To (StringClass &string)
{
	return (string.Copy_Wide (m_Buffer));
}


inline bool
WideStringClass::Convert_To (StringClass &string) const
{
	return (string.Copy_Wide (m_Buffer));
}

#endif //__WIDESTRING_H

