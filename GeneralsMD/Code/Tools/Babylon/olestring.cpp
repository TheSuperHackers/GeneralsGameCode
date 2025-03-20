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

//
// OLEString.cpp
//


#include "StdAfx.h"
#include <assert.h>
#include "olestring.h"

template void StripSpaces<OLECHAR> ( OLECHAR *string);
template void StripSpaces<char> ( char *string );
template void StripSpacesFromMetaString<OLECHAR> ( OLECHAR *string );
template void StripSpacesFromMetaString<char> ( char *string);
template void ConvertMetaChars<OLECHAR> ( OLECHAR *string);
template void ConvertMetaChars<char> ( char *string);
template int SameFormat<char> (  char *string1,  char *string2 );
template int SameFormat<OLECHAR> ( OLECHAR  *string1,  OLECHAR *string2 );
template void EncodeFormat<char> (  char *string );
template void EncodeFormat<OLECHAR> ( OLECHAR  *string );
template void DecodeFormat<char> (  char *string1 );
template void DecodeFormat<OLECHAR> ( OLECHAR  *string );
template int IsFormatTypeChar<char> (  char string1 );
template int IsFormatTypeChar<OLECHAR> ( OLECHAR  string );



static char *format_type = "cCdiouxXeEfgGnpsS"; // printf type as in %<width>.<presicion>{h|l|i64|L}<type>

template <typename text>int IsFormatTypeChar ( text ch )
{
	char *ptr = format_type;

	while ( *ptr )
	{
		if ( 	(unsigned int)*ptr++ == (unsigned int) ch )
		{
			return TRUE;
		}
	}
	return FALSE;
}



OLEString::OLEString ( void )
{
	ole = NULL;
	sb = NULL;
	len = 0;

	Unlock ();
	Set ("");

}

OLEString::~OLEString ( )
{
	delete [] ole;
	delete [] sb;
	ole = NULL;
	sb = NULL;
	len = 0;

}

void OLEString::Set ( OLECHAR *new_ole )
{

	if ( !locked )
	{
		delete [] ole;
		delete [] sb;
		ole = NULL;
		sb = NULL;
		
		len = wcslen ( new_ole );
		{
			ole = new OLECHAR[len+1];
			wcscpy ( ole, new_ole );
			sb = new char[len+1];
			sprintf ( sb, "%S", ole );
		}
	}

}

void OLEString::Set ( char *new_sb )
{

	if ( !locked )
	{
		delete [] ole;
		delete [] sb;
		ole = NULL;
		sb = NULL;
		
		len = strlen ( new_sb );
		
		{
			ole = new OLECHAR[len+1];
			swprintf ( ole, L"%S", new_sb );
			sb = new char[len+1];
			strcpy ( sb, new_sb );
		}
	}
}

void OLEString::StripSpaces ( void )
{
	if ( locked )
	{
		return;
	}

	if ( ole )
	{
		::StripSpaces ( ole );
	}
	if ( sb )
	{
		::StripSpaces ( sb );
	}
}


void OLEString::FormatMetaString ( void )
{
	char *str, *ptr;
	char ch, last = -1;
	int skipall = TRUE;
	int slash = FALSE;

	if ( !len || locked )
	{
		return;
	}

	char *string = new char[len*2];

	str = string;
	ptr = sb;

	while ( (ch = *ptr++) )
	{
		if ( ch == ' '  )
		{
			if ( last == ' ' )
			{
				continue;
			}
		}

		skipall= FALSE;

		if ( ch == '\\' )
		{
			char esc;
			slash = !slash;

			if ( slash && ((esc = *ptr) == 'n' || esc == 't') )
			{
					// remove last space
					if ( last != ' ' )
					{
						*str++ = ' ';
					}

					*str++ = '\\';
					ptr++;
					*str++ = esc;
					last = *str++ = ' ';
					continue;
			}
		}
		else
		{
			slash = FALSE;
		}

		last = *str++ = ch;
	}

	if ( last == ' ' )
	{
		str--;
	}

	*str = 0;

	Set ( string );
	delete [] string;
	string = NULL;
}

template <typename text> void StripSpaces ( text *string )
{
	text *str, *ptr;
	text ch, last = -1;
	int skipall = TRUE;

	str = ptr = string;

	while ( (ch = *ptr++) )
	{
		if ( ch == ' '  )
		{
			if ( last == ' ' || skipall )
			{
				continue;
			}
		}

		if ( ch == '\n' || ch == '\t' )
		{
				// remove last space
				if ( last == ' ' )
				{
					str--;
				}

				skipall = TRUE;		// skip all spaces 
				last = *str++ = ch;
				continue;
		}

		last = *str++ = ch;
		skipall = FALSE;
	}

	if ( last == ' ' )
	{
		str--;
	}

	*str = 0;
}

template <typename text> void StripSpacesFromMetaString ( text *string )
{
	text *str, *ptr;
	text ch, last = -1;
	int skipall = TRUE;
	int slash = FALSE;

	str = ptr = string;

	while ( (ch = *ptr++) )
	{
		if ( ch == ' '  )
		{
			if ( last == ' ' || skipall )
			{
				continue;
			}
		}

		skipall= FALSE;

		if ( ch == '\\' )
		{
			text esc;
			slash = !slash;

			if ( slash && (esc = *ptr) == 'n' || esc == 't' )
			{
					// remove last space
					if ( last == ' ' )
					{
						str--;
					}

					skipall = TRUE;		// skip all spaces 
					*str++ = '\\';
					ptr++;
					last = *str++ = esc;
					continue;
			}
		}
		else
		{
			slash = FALSE;
		}

		last = *str++ = ch;
	}

	if ( last == ' ' )
	{
		str--;
	}

	*str = 0;
}


template <typename text> void ConvertMetaChars ( text *string )
{
	text *ptr;
	text ch;

	ptr = string;

	while ( (ch = *string++) )
	{
		if ( ch == '\\' )
		{
			if ( ch = *string )
			{
				switch  ( ch )
				{
					case 'n':
					case 'N':
						ch = '\n';
						break;
					case 't':
					case 'T':
						ch = '\t';
						break;
				}
				string++;
			}
		}

		*ptr++ = ch;
	}

	*ptr = 0;
}

template <typename text> int SameFormat ( text *string1, text *string2 )
{

	while ( *string1 && *string2 )
	{

		while ( *string1 )
		{
				if (*string1 == '%')
				{
					string1++;
					break;
				}

				if ( *string1 == '\\' )
				{
					string1++;
				}

				if ( *string1 )
				{
					string1++;
				}
		}

		while ( *string2 )
		{
				if (*string2 == '%')
				{
					string2++;
					break;
				}

				if ( *string2 == '\\' )
				{
					string2++;
				}
				if ( *string2)
				{
					string2++;
				}
		}

		if ( !*string1 && !*string2)
		{
			return TRUE;
		}

		int found_type = FALSE;

		while ( *string1 && *string2 && !found_type )
		{
			found_type = IsFormatTypeChar ( *string1 );

			if ( *string1 != *string2 )
			{
				return FALSE;
			}
		
			string1++;
			string2++;
		}
		
	}
	return TRUE;
}

template <typename text> void EncodeFormat ( text *string )
{

}

template <typename text> void DecodeFormat ( text *string )
{

}

