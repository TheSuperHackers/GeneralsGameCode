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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/font3d.h                               $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/05/01 2:19p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FONT3D_H
#define FONT3D_H

#include "always.h"
#include "refcount.h"
#include "vector4.h"
#include "widestring.h"
#include "rect.h"

class TextureClass;
class SurfaceClass;

/******************************************************************
**
** Font3DDataClass
**
** This class provides an interface to a font texture.  Once
** created and loaded with a font, the object can return texture
** u v coordinate for any character in the font, as well as the
** character width for proportional fonts.  Fonts are loaded as
** 16-bit Targa files, then converted to proportional fonts by
** finding the minimum bounding box for each chacter.  The font
** texture is then minimized to a 256x256 or 128x128 texture
** material by re-stacking chars by thier minimum bounding box.
**
** During use, this class is really no more than a data table accessor
** Only during creation is any real code run.
**
** Since the space char nevers needs to be drawn, do not use
** the conventional method of acessing and drawing chars (which will
** still work). Instead, call Get_Space_Width to determine the user-
** settable width, and skip the drawing.
**
*******************************************************************/
class Font3DDataClass : public RefCountClass {

public:

	/*
	** Constructor,  Constructor which loads a targa file,
	** and Destructor
	*/
	Font3DDataClass( const char *filename );
	~Font3DDataClass();

	// the name of the font data (used for name matching and the like.)
	char		*Name;

	/*
	** access character width and height in pixels (clamp char to 0.255)
	*/

	unsigned char	Char_Width( WCHAR ch = (WCHAR)'H' )			{ return CharWidthTable[ch&0xFF]; }// & 0xFF]; } // No need to "& 0xff" with chars!!!
	unsigned char	Char_Height( WCHAR /*ch = 'H'*/ )			{ return CharHeight; }

	// u and v are in normalized texture space
	inline float	Char_U_Offset( WCHAR ch = (WCHAR)'H')		{ return UOffsetTable[ch&0xFF]; }// & 0xFF]; }
	inline float	Char_V_Offset( WCHAR ch = (WCHAR)'H')		{ return VOffsetTable[ch&0xFF]; }// & 0xFF]; }
	inline float	Char_U_Width( WCHAR ch = (WCHAR)'H' )		{ return UWidthTable[ch&0xFF]; }// & 0xFF]; }
	inline float	Char_V_Height( WCHAR /*ch = 'H'*/)			{ return VHeight; }

	// get all four UV values as one vector4
	Vector4 Char_UV_Corners( WCHAR ch = (WCHAR)'H' )
	{
//		ch &= 0xFF;
		return Vector4( UOffsetTable[ch], VOffsetTable[ch],
						UOffsetTable[ch] + UWidthTable[ch],
						VOffsetTable[ch] + VHeight );
	}

	/*
	** access texture material
	*/
	TextureClass *	Peek_Texture( void )								{ return Texture; }

private:
	/*
	** The material (texture) which holds the font
	*/
	TextureClass *		Texture;

	/*
	** Normalized texture page offsets for each character
	*/
	float				UOffsetTable[ 256 ];
	float				VOffsetTable[ 256 ];
	float				UWidthTable[ 256 ];
	float				VHeight;

	unsigned char	CharWidthTable[ 256 ];
	unsigned char	CharHeight;

	/*
	** load a targa font image (.TGA)
	*/
	bool	Load_Font_Image( const char *filename );

	/*
	** routines to convert a mono-spaced font to a proportional
	** font and minimize texture image size as a result
	*/
	SurfaceClass *Make_Proportional( SurfaceClass *font_image );
	SurfaceClass *Minimize_Font_Image( SurfaceClass *font_image );

};

/******************************************************************
**
** Font3DInstanceClass
**
*******************************************************************/
class Font3DInstanceClass : public RefCountClass {

public:
	/*
	** Constructor which creates/gets a Font3DDataClass object,
	** and Destructor
	*/
	Font3DInstanceClass( const char *filename );
	~Font3DInstanceClass();

	/*
	** access texture material
	*/
	TextureClass *Peek_Texture( void ) { return FontData->Peek_Texture(); }

	/*
	** The non-scaled monospace char width in pixels ( set to 0 for proportional spaced font )
	*/
	void	Set_Mono_Spaced( void );
	void	Set_Proportional( void )	{ MonoSpacing = 0;  Build_Cached_Tables(); }


	/*
	** Set the font scale (default to 1)
	** This amount will be automatically applied to all Char_Screen_Width calls
	*/
	void	Set_Scale( float scale )				{ Scale = scale; Build_Cached_Tables(); }
	//	float Get_Scale() const							{ return Scale; }

   /*
	** The scaled character pixel width, height, and spacing data (clamp char to 0.255)
	*/
	float	Char_Width( WCHAR ch ) const		{ return ScaledWidthTable[ch&0xFF]; }
	float	Char_Spacing( WCHAR ch ) const	{ return ScaledSpacingTable[ch&0xFF]; }
	float	Char_Height( void ) const			{ return ScaledHeight; }


	/*
	** The scaled pixel width of a string; useful before printing to avoid screen overflows.
	*/
	float String_Width( const WCHAR *test_str );
	float String_Width( const char *test_str );

	/*
	** Char UVs
	*/
	// u and v are in normalized texture space
	//	inline float	Char_U_Offset( WCHAR ch = (WCHAR)'H') { return FontData->Char_U_Offset( ch & 0xFF ); }
	//	inline float	Char_V_Offset( WCHAR ch = (WCHAR)'H') { return FontData->Char_V_Offset( ch & 0xFF ); }
	//	inline float	Char_U_Width( WCHAR ch = (WCHAR)'H' ) { return FontData->Char_U_Width( ch & 0xFF );  }
	//	inline float	Char_V_Height( WCHAR ch = (WCHAR)'H') { return FontData->Char_V_Height( ch & 0xFF ); }
	//	Vector4 Char_UV_Corners( WCHAR ch = (WCHAR)'H' )	{ return FontData->Char_UV_Corners( ch & 0xFF ); }
	RectClass		Char_UV( WCHAR ch )	{ return RectClass(	FontData->Char_U_Offset(ch),
																				FontData->Char_V_Offset(ch),
																				FontData->Char_U_Offset(ch) + FontData->Char_U_Width(ch),
																				FontData->Char_V_Offset(ch) + FontData->Char_V_Height(ch) ); }
private:

	Font3DDataClass *	FontData;			// The font data
	float					Scale;				// The current scale factor
	float					SpaceSpacing; 		// non-scaled width of space in pixels ( defaults to 1/2 'H' width )
	float					InterCharSpacing;	// non-scaled width between chars in pixels
	float					MonoSpacing;		// non-scaled monospace char width in pixels (0 for proportional)

	float					ScaledWidthTable[256];		// scaled cache of the chars pixel widths
	float					ScaledSpacingTable[256];	// scaled cache of the chars pixel spacing
	float					ScaledHeight;					// scaled cache of the chars pixel height

	void					Build_Cached_Tables();
};


#endif
