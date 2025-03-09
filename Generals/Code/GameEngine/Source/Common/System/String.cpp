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

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    String
//
// File name: WSYS_String.cpp
//
// Created:   11/5/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include "PreRTS.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "Common/string.h"

// 'assignment within condition expression'.
#pragma warning(disable : 4706)

//----------------------------------------------------------------------------
//         Externals                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines                                                         
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data                                                      
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions                                                
//----------------------------------------------------------------------------


//============================================================================
// WSYS_String::WSYS_String
//============================================================================

WSYS_String::WSYS_String(const Char *string)
: m_data(NULL)
{
	set(string);
}

//============================================================================
// WSYS_String::~WSYS_String
//============================================================================

WSYS_String::~WSYS_String()
{
	delete [] m_data;
}

//============================================================================
// WSYS_String::operator== 
//============================================================================

Bool WSYS_String::operator== (const char *rvalue) const
{
	return strcmp( get(), rvalue) == 0;
}

//============================================================================
// WSYS_String::operator!= 
//============================================================================

Bool WSYS_String::operator!= (const char *rvalue) const
{
	return strcmp( get(), rvalue) != 0;
}

//============================================================================
// WSYS_String::operator= 
//============================================================================

const WSYS_String&	WSYS_String::operator= (const WSYS_String &string)
{
	set( string.get());
	return (*this);
}

//============================================================================
// WSYS_String::operator= 
//============================================================================

const WSYS_String&	WSYS_String::operator= (const Char *string)
{
	set( string );
	return (*this);
}

//============================================================================
// WSYS_String::operator+= 
//============================================================================

const WSYS_String&	WSYS_String::operator+= (const WSYS_String &string)
{
	Char *buffer = new Char [ length() + string.length() + 1 ];		// pool[]ify

	if ( buffer != NULL )
	{
		strcpy( buffer, m_data );
		strcat( buffer, string.get() );
		delete [] m_data;
		m_data = buffer;
	}

	return (*this);
}

//============================================================================
// WSYS_String::operator+= 
//============================================================================

const WSYS_String&	WSYS_String::operator+= (const Char *string)
{
	if ( string != NULL )
	{
		Char *buffer = new Char [ length() + strlen( string ) + 1 ];	

		if ( buffer != NULL )
		{
			strcpy( buffer, m_data );
			strcat( buffer, string );
			delete [] m_data;
			m_data = buffer;
		}
	}

	return (*this);
}

//============================================================================
// operator+ (const WSYS_String &string1, const WSYS_String &string2)
//============================================================================

WSYS_String	operator+ (const WSYS_String &string1, const WSYS_String &string2)
{
	WSYS_String temp(string1);
	temp += string2;
	return temp;

}

//============================================================================
// operator+ (const Char *string1, const WSYS_String &string2)
//============================================================================

WSYS_String	operator+ (const Char *string1, const WSYS_String &string2)
{
	WSYS_String temp(string1);
	temp += string2;
	return temp;
}

//============================================================================
// operator+ (const WSYS_String &string1, const Char *string2)
//============================================================================

WSYS_String	operator+ (const WSYS_String &string1, const Char *string2)
{
	WSYS_String temp(string1);
	temp += string2;
	return temp;
}

//============================================================================
// WSYS_String::length
//============================================================================

Int WSYS_String::length(void) const
{
	return (int)strlen( m_data );
}

//============================================================================
// WSYS_String::isEmpty
//============================================================================

Bool WSYS_String::isEmpty(void) const
{
	return m_data[0] == 0;
}

//============================================================================
// WSYS_String::format
//============================================================================

Int _cdecl WSYS_String::format(const Char *format, ...)
{
	Int result = 0;
	char *buffer = new char[100*1024];

	if ( buffer )
	{
		va_list args;
		va_start( args, format );     /* Initialize variable arguments. */
		result = vsprintf ( buffer, format, args );
		va_end( args );
		set( buffer );
		delete [] buffer;
	}
	else
	{
		set("");
	}

	return result;
}

//============================================================================
// WSYS_String::set
//============================================================================

void WSYS_String::set( const Char *string )
{
	delete [] m_data;

	if ( string == NULL )
	{
		string = "";
	}

	m_data = new Char [ strlen(string) + 1];
	strcpy ( m_data, string );
}


//============================================================================
// WSYS_String::makeUpperCase
//============================================================================

void WSYS_String::makeUpperCase( void )
{
	Char *chr = m_data;
	Char ch;

	while( (ch = *chr) )
	{
		*chr++ = (Char) toupper( ch );
	}
}

//============================================================================
// WSYS_String::makeLowerCase
//============================================================================

void WSYS_String::makeLowerCase( void )
{
	Char *chr = m_data;
	Char ch;

	while( (ch = *chr) )
	{
		*chr++ = (Char) tolower( ch );
	}
}
