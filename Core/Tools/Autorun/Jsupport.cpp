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

// JSUPPORT.CPP
// DBCS Support Codes

#include <windows.h>
#include "JSUPPORT.H"

// 前置禁則文字
// Can't set these characters on top of line
static BOOL IsDBCSInvalidAtTop(unsigned int c)
{
	static BYTE * dtbl = (BYTE *)"￠°’”‰′″℃、。々〉》」』】〕ぁぃぅぇぉっゃゅょゎ゛゜ゝゞァィゥェォッャュョヮヵヶ・ーヽヾ！％），．：；？］｝";
	static BYTE * stbl = (BYTE *)"!%),.:;?]}｡｣､･ﾞﾟ";

	if(c<0x100)
	{
		if(strchr((char *)stbl,(char)c))
			return TRUE;
	}
	else
	{
		BYTE	c1,c2,*p;
		c1 = (BYTE)(c >> 8);
		c2 = (BYTE)(c & 0xff);
		p = dtbl;
		while(*p)
		{
			if((*p==c1)&&(*(p+1)==c2))
				return TRUE;
			p+=2;
		}
	}
	return FALSE;
}

// 後置禁則文字
// Can't set these characters on end of line
static BOOL IsDBCSInvalidAtEnd( unsigned int c )
{
	static BYTE * dtbl = (BYTE *)"‘“〈《「『【〔（［｛＄￡￥";
	static BYTE * stbl = (BYTE *)"｢({[";

	if(c<0x100)
	{
		if(strchr((char *)stbl,(char)c))
			return TRUE;
	}
	else
	{
		BYTE	c1,c2,*p;
		c1 = (BYTE)(c >> 8);
		c2 = (BYTE)(c & 0xff);
		p = dtbl;
		while(*p)
		{
			if((*p==c1)&&(*(p+1)==c2))
				return TRUE;
			p+=2;
		}
	}
	return FALSE;
}

int nGetWord( char *string, int fdbcs )
{
	BOOL	bCiae0, bCiat1, bDbcs0, bDbcs1;
	BYTE	*p = (BYTE *)string;
	UINT	c0, c1, c;

	//--------------------------------------------------------------------------
	// If no string was passed in, exit.
	//--------------------------------------------------------------------------
	if( !p || !( c0 = *p++ )) {
//	if(( p == NULL ) || ( *p == '\0' )) {
		return 0;
	}
//	c0 = *p;

	//--------------------------------------------------------------------------
	// If we are NOT a double-byte language, then just parse first word.
	//--------------------------------------------------------------------------
	if( !fdbcs ) {

		int n = 0;

		while( *p >' ' ) {
			n++;
			p++;
		}

		if( *p ) {
			n++;
		}
		return n;
	}

	//--------------------------------------------------------------------------
	// If we are a double-byte language...
	//--------------------------------------------------------------------------
	bDbcs0 = IsDBCSLeadByte( c0 ) && *p;

	if( bDbcs0 ) {
		c0 = ( c0 << 8 ) | *p++;
	}

	bCiae0 = IsDBCSInvalidAtEnd( c0 );

	while( c1 = *p ) {

		bDbcs1 = ( IsDBCSLeadByte( c1 ) && ( c = *( p + 1 )));
		if( bDbcs1 ) {
			c1 = ( c1<<8 ) | c;
		}

		if(( bDbcs0 || bDbcs1 ) && !( bDbcs0 && bDbcs1 )) {		// XOR
			break;
		}

		if( bDbcs1 ) {

			bCiat1 = IsDBCSInvalidAtTop( c1 );

			if( !( bCiae0 || bCiat1 ))	{
				break;
			}
			bCiae0 = IsDBCSInvalidAtEnd( c1 );
			p+=2;

		} else {

			if( c0<=' ' ) {
				break;
			}
			p++;
		}

		bDbcs0 = bDbcs1;
		c0 = c1;
	}
	return( p - (BYTE *)string );
}
