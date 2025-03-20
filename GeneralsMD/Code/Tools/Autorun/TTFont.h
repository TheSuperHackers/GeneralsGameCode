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

/************************************************************************************************ 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S				  *** 
 ************************************************************************************************ 
 *																								* 
 *                 Project Name: Setup														  	* 
 *																								* 
 *                      Archive: ttfont.h														* 
 *																								* 
 *                       Author: Joe_b															*
 *																								* 
 *                      Modtime: 6/23/97 3:14p													*
 *																								* 
 *                      Updated: 08/01/2000 [MML]												*
 *																								*
 *                     Revision: 22																*
 *																								*
 *----------------------------------------------------------------------------------------------* 
 * Functions:																					* 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#pragma once

#ifndef TTFONT_H
#define TTFONT_H

#include	<stddef.h>
#include	"POINT.h"
#include	"RECT.h"


/******************************************************************************
**	These are the control flags for Fancy_Text_Print function.
*/
typedef enum TextPrintType {

	TPF_LASTPOINT		= 0x0000,	  		// Use previous font point value.
	TPF_TT_10POINT		= 0x0001,	  		// True Type Font - 10 point
	TPF_TT_12POINT		= 0x0002,	  		// True Type Font - 12 point
	TPF_TT_14POINT		= 0x000A,	  		// True Type Font - 14 point
	TPF_TT_16POINT		= 0x000B,	  		// True Type Font - 16 point
	TPF_TT_18POINT		= 0x000C,	  		// True Type Font - 18 point
	TPF_TT_20POINT		= 0x000D,	  		// True Type Font - 20 point 
	TPF_TT_22POINT		= 0x000E,			// True Type Font - 22 point
	TPF_TT_24POINT		= 0x000F,			// True Type Font - 24 point

	TPF_BUTTON_FONT		= 0x0010,
	TPF_TEXT_FONT		= 0x0020,

} TextPrintType;

typedef enum TextShadowType {

	TPF_NOSHADOW		= 0x0000,
	TPF_DROPSHADOW		= 0x0001,	  						// Use a simple drop shadow.
	TPF_LIGHTSHADOW		= 0x0002,
	TPF_FULLSHADOW		= 0x0004,	  						// Use a full outline shadow.
	TPF_DOUBLESHADOW	= 0x0008,	  						// Use a simple drop shadow.
	TPF_SHADOW			= 0x0010,	  						// Print twice, using backcolor.

} TextShadowType;

typedef enum TextFormatType {

	TPF_TOP					= DT_TOP,						// Use with DT_SINGLELINE.	Top-justifies text.
	TPF_VCENTER				= DT_VCENTER,					// Use with DT_SINGLELINE.	Centers text vertically.
	TPF_BOTTOM				= DT_BOTTOM,					// Use with DT_SINGLELINE.	Justifies test to the bottom of the rectangle. 
	TPF_LEFT				= DT_LEFT,						// Aligns text to the left.
	TPF_CENTER				= DT_CENTER,					// Centers text horizontally in the rectangle.
	TPF_RIGHT				= DT_RIGHT,						// Right justify text.
	TPF_WORDBREAK			= DT_WORDBREAK,					// Lines are automatically broken between words.
	TPF_SINGLE_LINE			= DT_SINGLELINE,				// All text on one line only.
	TPF_NO_PREFIX			= DT_NOPREFIX,					// Turns off processing of prefix characters. 
	TPF_PATH_ELLIPSIS		= DT_PATH_ELLIPSIS,				// For displayed text, replaces characters in the middle of the string with ellipses so that the result fits in the specified rectangle. 

} TextFormatType;

/******************************************************************************
** Standard button text print flags.
*/
//#define TPF_BUTTON				(TextFormatType)( DT_VCENTER | DT_CENTER | DT_SINGLELINE )
//#define TPF_CENTER_FORMAT			(TextFormatType)( DT_VCENTER | DT_CENTER | DT_WORDBREAK )
//#define TPF_CHECKBOX				(TextFormatType)( DT_LEFT | DT_VCENTER | DT_WORDBREAK )
//#define TPF_EDIT					(TextFormatType)( DT_LEFT | DT_VCENTER )
//#define TPF_DEFAULT				(TextFormatType)( DT_LEFT | DT_WORDBREAK )

#define TPF_BUTTON					(TextFormatType)( DT_CENTER | DT_VCENTER	| DT_SINGLELINE )
#define TPF_EDITBOX					(TextFormatType)( DT_LEFT   | DT_VCENTER	| DT_SINGLELINE )
#define TPF_RADIO	  				(TextFormatType)( DT_LEFT	| DT_WORDBREAK	)
#define TPF_CHECKBOX				(TextFormatType)( DT_LEFT	| DT_WORDBREAK	)
#define TPF_OUTER_SCROLL			(TextFormatType)( DT_LEFT	| DT_WORDBREAK	)
#define TPF_INNER_SCROLL			(TextFormatType)( DT_LEFT	| DT_SINGLELINE	)
								
#define TPF_LEFT_TEXT				(TextFormatType)( DT_LEFT	| DT_WORDBREAK )
#define TPF_CENTER_TEXT				(TextFormatType)( DT_CENTER | DT_WORDBREAK )
#define TPF_RIGHT_TEXT				(TextFormatType)( DT_RIGHT	| DT_WORDBREAK )
								
#define TPF_LEFT_TOP_ALIGNMENT	  	(TextFormatType)( DT_LEFT	| DT_TOP		| DT_SINGLELINE )
#define TPF_LEFT_BOTTOM_ALIGNMENT	(TextFormatType)( DT_LEFT	| DT_BOTTOM		| DT_SINGLELINE )
#define TPF_LEFT_JUSTIFY			(TextFormatType)( DT_LEFT	| DT_VCENTER	| DT_SINGLELINE )

#define TPF_RIGHT_TOP_ALIGNMENT	  	(TextFormatType)( DT_RIGHT	| DT_TOP		| DT_SINGLELINE )
#define TPF_RIGHT_BOTTOM_ALIGNMENT	(TextFormatType)( DT_RIGHT	| DT_BOTTOM		| DT_SINGLELINE )
#define TPF_RIGHT_JUSTIFY			(TextFormatType)( DT_RIGHT	| DT_VCENTER	| DT_SINGLELINE )

#define TPF_CENTER_TOP_ALIGNMENT	(TextFormatType)( DT_CENTER	| DT_TOP		| DT_SINGLELINE )
#define TPF_CENTER_BOTTOM_ALIGNMENT	(TextFormatType)( DT_CENTER	| DT_BOTTOM		| DT_SINGLELINE )
#define TPF_CENTER_JUSTIFY			(TextFormatType)( DT_CENTER	| DT_VCENTER	| DT_SINGLELINE )



/******************************************************************************
**	These are the control flags for Fancy_Text_Print function.
*/
typedef enum SpecialEffectType {
	TPF_NONE				=0x0000,		// No special effects needed.
	TPF_CUTOFF_AT_WIDTH		=0x0001,		// Don't print past the allowed width.
	TPF_BURST_MODE			=0x0002,		// Print text one letter at a time like a typewriter.
	TPF_SPECIAL_WRAP		=0x0003,		// Begin at a specified point but start next line at a point before the starting point.
} SpecialEffectType;


/******************************************************************************
** Global DC.  Use it or create your own!
*/
extern	HDC BackBufferDC;

/******************************************************************************
** Global Colors for use throughout program.
*/
extern	unsigned long TEXT_COLOR;
extern	unsigned long SHADOW_COLOR;		
extern	unsigned long TEXT_NORMAL_COLOR;
extern	unsigned long TEXT_FOCUSED_COLOR;
extern	unsigned long TEXT_PRESSED_COLOR;
extern	unsigned long TEXT_NORMAL_SHADOW_COLOR;
extern	unsigned long TEXT_FOCUSED_SHADOW_COLOR;
extern	unsigned long TEXT_PRESSED_SHADOW_COLOR;

extern	unsigned long WHITE_COLOR;
extern	unsigned long BLACK_COLOR;
extern	unsigned long RED_COLOR;
extern	unsigned long ORANGE_COLOR;
extern	unsigned long YELLOW_COLOR;
extern	unsigned long GREEN_COLOR;
extern	unsigned long BLUE_COLOR;
extern	unsigned long INDIGO_COLOR;
extern	unsigned long VIOLET_COLOR;


/******************************************************************************
**	This is a True Type Font class object to create and use True Type fonts.
******************************************************************************/
// Font Weight -	Specifies the weight of the font in the range 0 through 1000. 
//					For example, 400 is normal and 700 is bold. 
//					If this value is zero, a default weight is used. 
//
//	The following values are defined for convenience:
//		FW_DONTCARE		0			FW_SEMIBOLD		600
//		FW_THIN			100 		FW_DEMIBOLD		600
//		FW_EXTRALIGHT	200 		FW_BOLD			700
//		FW_ULTRALIGHT	200 		FW_EXTRABOLD	800
//		FW_LIGHT		300 		FW_ULTRABOLD	800
//		FW_NORMAL		400 		FW_HEAVY 		900
//		FW_REGULAR		400 		FW_BLACK 		900
//		FW_MEDIUM		500 
//-----------------------------------------------------------------------------

class TTFontClass 
{
	public:

		TTFontClass ( 
			HDC		hdc,
			char *	filename, 
			char *	facename, 
			int		height, 
			int		weight				= FW_NORMAL, 
			BYTE	charset				= ANSI_CHARSET, 
			int		width				= 0, 
			int		escapement			= 0, 
			int		orientation			= 0, 
			BYTE	italic				= FALSE, 
			BYTE	underline			= FALSE, 
			BYTE	strikeout			= FALSE, 
			BYTE	outputPrecision		= OUT_TT_ONLY_PRECIS, 
			BYTE	clipPrecision		= CLIP_DEFAULT_PRECIS, 
			BYTE	quality				= PROOF_QUALITY, 
			BYTE	pitchAndFamily		= FF_DONTCARE );

		virtual ~TTFontClass(void)				
			{ 
				if ( Font != NULL ) {
					DeleteObject( Font );
					Font = NULL;
				}
				RemoveFontResource( szFilename );
			};

		virtual int		Char_Pixel_Width		( HDC hdc, UINT c ) const;
		virtual int		Char_Pixel_Width		( HDC hdc, char const * string, int *num_bytes=NULL ) const;
		virtual int		String_Pixel_Width		( HDC hdc, char const * string ) const;
		virtual void	String_Pixel_Bounds		( HDC hdc, const char * string, Rect& bounds ) const;
		virtual int		Get_Width				( void ) const;
		virtual int		Get_Height				( void ) const;
		virtual int		Set_XSpacing			( HDC hdc, int x );
		virtual int		Set_YSpacing			( int y );
		virtual int		Find_Text_VLength		( HDC hdc, char *str, int width );
		virtual HFONT	Get_Font_Ptr			( void )		{ return Font; };
		virtual int		IsFontDBCS				( void ) const	{ return ((CharSet==SHIFTJIS_CHARSET)||(CharSet==HANGEUL_CHARSET)||(CharSet==CHINESEBIG5_CHARSET)); };	// [OYO]
		virtual UINT	Get_Double_Byte_Char	( const char *string, int *num_bytes=NULL ) const;

		virtual Point2D	Print( 
							HDC hdc, 
							char const * string, 
							Rect const & cliprect, 
							COLORREF forecolor		= TEXT_COLOR,
							COLORREF backcolor		= TEXT_NORMAL_SHADOW_COLOR,
							TextFormatType flag		= TPF_LEFT_TEXT, 
							TextShadowType shadow	= TPF_NOSHADOW );

		virtual Point2D	Print( 
							HDC hdc, 
							wchar_t const * string, 
							Rect const & cliprect, 
							COLORREF forecolor		= TEXT_COLOR,
							COLORREF backcolor		= TEXT_NORMAL_SHADOW_COLOR,
							TextFormatType flag		= TPF_LEFT_TEXT, 
							TextShadowType shadow	= TPF_NOSHADOW );

	private:

		HFONT Font;
		long  Height;
		long  Ascent;
		long  Descent;
		long  InternalLeading;
		long  ExternalLeading;
		long  AveCharWidth;
		long  MaxCharWidth;
		long  Overhang;
		long  Italic;
		long  Underlined;
		long  StruckOut;
		int   CharSet;						// [OYO]
		int   FontXSpacing;					// GetTextCharacterExtra;
		int	  FontYSpacing;
		char  szFacename[ MAX_PATH ];
		char  szFilename[ MAX_PATH ];
};


//-------------------------------------------------------------------------
//	Global functions.
//-------------------------------------------------------------------------
TTFontClass *	Font_From_TPF		( TextPrintType flags ); 	// Returns FontPtr based on flags passed in.
bool			Is_True_Type_Font	( TextPrintType flags );	// True Type???

//-------------------------------------------------------------------------
// This class is a wrapper around all the fonts that we want to be available.
// The constructer will make them, and the destructor will remove them for us.
//-------------------------------------------------------------------------
class FontManagerClass 
{
	public:
		FontManagerClass		( HDC hdc );
		~FontManagerClass		( void );
		TTFontClass * Get_Font	( TextPrintType flags )	{ return( Font_From_TPF( flags ));  };
};


/******************************************************************************
** FontManager Class Pointer.
*/
extern FontManagerClass *FontManager;

/******************************************************************************
** Loaded data file pointers.
*/
extern TTFontClass 			*TTButtonFontPtr;
extern TTFontClass 			*TTButtonFontPtrSmall;
extern TTFontClass 			*TTTextFontPtr;
extern TTFontClass 			*TTTextFontPtr640;
extern TTFontClass 			*TTTextFontPtr800;
extern TTFontClass 			*TTLicenseFontPtr;


#endif
