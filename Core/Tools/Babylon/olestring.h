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
// OLEString
//

#ifndef __OLESTRING_H
#define __OLESTRING_H

const unsigned int OLESTRING_DEFAULT_SIZE = 256;

class OLEString
{

	OLECHAR								*ole;
	char									*sb;
	unsigned int					len;
	int										locked;

	public:

	OLEString ( void ) ;
	~OLEString ();
	void		Set ( OLECHAR *new_ole );
	void		Set ( const char *new_sb );
	OLECHAR*Get ( void ) { return ole; };
	int			Len ( void ) { return len; };
	char*		GetSB ( void ) { return sb; };
	void		StripSpaces ( void );
	void		FormatMetaString ( void );
	void		Lock ( void )	{ locked = TRUE; };
	void		Unlock ( void )	{ locked = FALSE; };
};

template <typename text> void StripSpaces ( text *string );
template <typename text> void ConvertMetaChars ( text *string );
template <typename text> void StripSpacesFromMetaString ( text *string );
template <typename text> int SameFormat ( text *string1, text *string2 );
template <typename text> void EncodeFormat ( text *string );
template <typename text> void DecodeFormat ( text *string );
template <typename text> int IsFormatTypeChar(  text string1 );



#endif // __OLESTRING_H
