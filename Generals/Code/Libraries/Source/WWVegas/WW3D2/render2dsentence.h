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

 /***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/render2dsentence.h                     $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:58a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef RENDER2DSENTENCE_H
#define RENDER2DSENTENCE_H

#include "render2d.h"
#include "refcount.h"
#include "Vector.H"
#include "vector2i.h"
#include "wwstring.h"
#include "win.h"

/*
** FontCharsClass
*/
class	SurfaceClass;

//
//	Private data structures
//
class FontCharsClassCharDataStruct : public W3DMPO
{
	W3DMPO_GLUE(FontCharsClassCharDataStruct)
public:
	WCHAR				Value;
	short				Width;
	uint16 *		Buffer;
};

enum { CHAR_BUFFER_LEN		= 32768 };

class FontCharsBuffer : public W3DMPO
{
	W3DMPO_GLUE(FontCharsBuffer)
public:
	uint16			Buffer[CHAR_BUFFER_LEN];
};


class FontCharsClass : public W3DMPO, public RefCountClass 
{
	W3DMPO_GLUE(FontCharsClass)

public:
	FontCharsClass( void );
	~FontCharsClass();

	// TR: Hack for unicode font support
	FontCharsClass					*AlternateUnicodeFont;		


	void	Initialize_GDI_Font( const char *font_name, int point_size, bool is_bold );
	bool	Is_Font( const char *font_name, int point_size, bool is_bold );
	const char * Get_Name( void )			{ return Name; }	

	int	Get_Char_Height( void )			{ return CharHeight; }
	int	Get_Char_Width( WCHAR ch );
	int	Get_Char_Spacing( WCHAR ch );
	
	int Get_Extra_Overlap(void) {return PixelOverlap;}

	void	Blit_Char( WCHAR ch, uint16 *dest_ptr, int dest_stride, int x, int y );

private:

	//
	//	Private methods
	//
	void							Create_GDI_Font( const char *font_name );
	void							Free_GDI_Font( void );
	const FontCharsClassCharDataStruct *	Store_GDI_Char( WCHAR ch );
	void							Update_Current_Buffer( int char_width );
	const FontCharsClassCharDataStruct	*	Get_Char_Data( WCHAR ch );

	void							Grow_Unicode_Array( WCHAR ch );
	void							Free_Character_Arrays( void );

	//
	//	Private member data
	//
	StringClass							Name;
	DynamicVectorClass<FontCharsBuffer*>	BufferList;
	int									CurrPixelOffset;
	int									CharHeight;
	int									CharAscent;
	int									CharOverhang;	
	int									PixelOverlap;
	int									PointSize;
	StringClass							GDIFontName;
	HFONT									OldGDIFont;
	HBITMAP								OldGDIBitmap;
	HBITMAP								GDIBitmap;	
	HFONT									GDIFont;
	uint8 *								GDIBitmapBits;
	HDC									MemDC;
	FontCharsClassCharDataStruct *					ASCIICharArray[256];
	FontCharsClassCharDataStruct **					UnicodeCharArray;
	uint16								FirstUnicodeChar;
	uint16								LastUnicodeChar;
	bool									IsBold;
};

/*
** Render2DSentenceClass
*/
class Render2DSentenceClass {
public:
	//Render2DSentenceClass( FontCharsClass * font );
	Render2DSentenceClass( void );
	~Render2DSentenceClass();

	void				Render (void);
	virtual	void	Reset (void);
	void				Reset_Polys (void);

	FontCharsClass *	Peek_Font( void )						{ return Font; }
	void	Set_Font( FontCharsClass *font );

	void	Set_Location( const Vector2 & loc );
	void	Set_Base_Location( const Vector2 & loc );
	bool	Set_Wrapping_Width (float width)					{ if(WrapWidth == width)
																											return false;
																										WrapWidth = width; 
																										return true;	}
	bool	Set_Word_Wrap_Centered( bool isCentered ) { if(Centered == isCentered)
																											return false;
																										Centered = isCentered; 
																										return true;}
	void Set_Hot_Key_Parse( bool parseHotKey ){ ParseHotKey = parseHotKey; }
	void Set_Use_Hard_Word_Wrap( bool useHardWrap){ useHardWordWrap = useHardWrap;	}
	//
	// Clipping support
	//
	void	Set_Clipping_Rect( const RectClass &rect )	{ ClipRect = rect; IsClippedEnabled = true; }
	bool	Is_Clipping_Enabled( void ) const				{ return IsClippedEnabled; }
	void	Enable_Clipping( bool onoff )						{ IsClippedEnabled = onoff; }

	//
	//	Shader modification support
	//
	void			Make_Additive (void);
	ShaderClass	Get_Shader (void) const						{ return Shader; }
	void			Set_Shader (ShaderClass shader);

//	void	Draw_Block( const RectClass & screen, unsigned long color = 0xFFFFFFFF );

	const RectClass & Get_Draw_Extents( void )			{ return DrawExtents; }
//	const RectClass & Get_Total_Extents( void )			{ return TotalExtents; }
//	const Vector2 & Get_Cursor( void )						{ return Cursor; }

	Vector2	Get_Text_Extents( const WCHAR * text );
	Vector2	Get_Formatted_Text_Extents( const WCHAR * text );

	//
	//	Sentence control
	//
	void	Build_Sentence (const WCHAR *text, int *hkX, int *hkY);
	void	Draw_Sentence (uint32 color = 0xFFFFFFFF);

	//
	//	Texture hint
	//
	void	Set_Texture_Size_Hint( int hint )				{ TextureSizeHint = hint; }
	int	Get_Texture_Size_Hint( void ) const				{ return TextureSizeHint; }

	void	Set_Mono_Spaced( bool onoff )						{ MonoSpaced = onoff; }

private:

	//
	//	Private structures
	//
	struct SentenceDataStruct {
		SurfaceClass *		Surface;
		RectClass			ScreenRect;
		RectClass			UVRect;

		bool operator== (const SentenceDataStruct &src)	{ return false; }
		bool operator!= (const SentenceDataStruct &src)	{ return true; }
	};

	struct PendingSurfaceStruct {
		SurfaceClass *								Surface;
		DynamicVectorClass<Render2DClass *>	Renderers;

		bool operator== (const PendingSurfaceStruct &src)	{ return false; }
		bool operator!= (const PendingSurfaceStruct &src)	{ return true; }
	};

	struct RendererDataStruct {
		Render2DClass *	Renderer;
		SurfaceClass *		Surface;

		bool operator== (const RendererDataStruct &src)	{ return false; }
		bool operator!= (const RendererDataStruct &src)	{ return true; }
	};

	//
	//	Private methods
	//
	void	Reset_Sentence_Data (void);
	void	Build_Textures (void);
	void	Record_Sentence_Chunk (void);
	void	Allocate_New_Surface (const WCHAR *text, bool justCalcExtents = false);
	void	Release_Pending_Surfaces (void);
	void	Build_Sentence_Centered (const WCHAR *text, int *hkX, int *hkY);
	Vector2	Build_Sentence_Not_Centered (const WCHAR *text, int *hkX, int *hkY,bool justCalcExtents = false );		
	//
	//	Private member data
	//
	DynamicVectorClass<SentenceDataStruct>		SentenceData;
	DynamicVectorClass<PendingSurfaceStruct>	PendingSurfaces;
	DynamicVectorClass<RendererDataStruct>		Renderers;
	FontCharsClass	*						Font;
	Vector2											BaseLocation;
	Vector2											Location;
	Vector2											Cursor;
	Vector2i										TextureOffset;
	int													TextureStartX;
	int													CurrTextureSize;
	int													TextureSizeHint;
	SurfaceClass *							CurSurface;
	bool												MonoSpaced;
	float												WrapWidth;
	bool												Centered;			// Determines whether or not to center each line
	RectClass										ClipRect;
	RectClass										DrawExtents;
	bool												IsClippedEnabled;
	bool												ParseHotKey;
	bool												useHardWordWrap;
														
	uint16 *										LockedPtr;
	int													LockedStride;
	TextureClass *							CurTexture;
	ShaderClass									Shader;
};

#endif	// RENDER2DSENTENCE_H
