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

/************************************************************************************************ 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S                *** 
 ************************************************************************************************ 
 *																								* 
 *                 Project Name: Setup                                            				* 
 *																								* 
 *                      Archive: TTFont.cpp                                             		* 
 *																								* 
 *                       Author: Maria del Mar McCready Legg									*
 *																								* 
 *                      Modtime: 8/24/99 3:44pm													*
 *																								* 
 *                     Revision: 01																*
 *																								*
 *----------------------------------------------------------------------------------------------* 
 * Functions:																					* 
 *   TTFontClass::TTFontClass -- Constructor for a font class object.							* 
 *   TTFontClass::Char_Pixel_Width -- Fetch the pixel width of the character specified.			* 
 *	 TTFontClass::Find_Text_VLength -- Finds length of text in pixels							*
 *   TTFontClass::Get_Height -- Fetch the normalized height of the nominal font character.		* 
 *   TTFontClass::Get_Width -- Get normalized width of the nominal font character.				* 
 *   TTFontClass::Print -- Print text to the surface specified.									* 
 *   TTFontClass::Raw_Height -- Fetch the height of the font.									* 
 *   TTFontClass::Raw_Width -- Fetch the raw width of a character.								* 
 *   TTFontClass::Set_XSpacing -- Set the X spacing override value.								* 
 *   TTFontClass::Set_YSpacing -- Set the vertical (Y) spacing override value.					* 
 *   TTFontClass::String_Pixel_Width -- Determines the width of the string in pixels.			* 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 

#define  STRICT
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include "ARGS.H"
#include "autorun.h"
#include "RECT.h"
#include "Wnd_File.h"
#include "TTFont.h"
#include "JSUPPORT.H"		// [OYO]
#include "Locale_API.h"


#define FONTINFOMAXHEIGHT		4
#define FONTINFOMAXWIDTH		5
#define FUDGEDIV				16

//-------------------------------------------------------------------------
// Text Fonts.
//-------------------------------------------------------------------------
TTFontClass *TTButtonFontPtr		= NULL;
TTFontClass *TTButtonFontPtrSmall	= NULL;
TTFontClass *TTTextFontPtr			= NULL;
TTFontClass	*TTTextFontPtr640		= NULL;
TTFontClass	*TTTextFontPtr800		= NULL;
TTFontClass	*TTLicenseFontPtr		= NULL;
									
FontManagerClass * FontManager		= NULL;

//unsigned long TEXT_COLOR			  		= RGB( 247, 171,  11 );
//unsigned long SHADOW_COLOR		  		= RGB(  40,   8,   8 );
unsigned long TEXT_COLOR			  		= RGB( 255, 204,  51 );
unsigned long SHADOW_COLOR		  			= RGB(  40,   8,   8 );

unsigned long TEXT_NORMAL_COLOR	  	   		= RGB( 255, 204,  51 );	 
unsigned long TEXT_NORMAL_SHADOW_COLOR 		= RGB(  40,   8,   8 );	 
unsigned long TEXT_FOCUSED_COLOR		 	= RGB( 255, 204,  51 );
unsigned long TEXT_FOCUSED_SHADOW_COLOR  	= RGB(  40,   8,   8 );	 
unsigned long TEXT_PRESSED_COLOR		 	= RGB( 194,  79,  32 );
unsigned long TEXT_PRESSED_SHADOW_COLOR  	= RGB(  70,  55,  49 );	 


unsigned long BLACK_COLOR					= RGB(   0,	  0,   0 );      
unsigned long WHITE_COLOR					= RGB( 255, 255, 255 );
unsigned long RED_COLOR						= RGB( 255,   0,   0 );    
unsigned long ORANGE_COLOR					= RGB( 199,  91,   0 );
unsigned long YELLOW_COLOR					= RGB( 255, 247,   0 );
unsigned long GREEN_COLOR					= RGB(   0, 255,   0 );    
unsigned long BLUE_COLOR					= RGB(   0,   0, 255 );
unsigned long INDIGO_COLOR		  			= RGB(  90,   2, 253 );
unsigned long VIOLET_COLOR		  			= RGB( 128,   0, 255 );


/************************************************************************************
 * TTFontClass::TTFontClass -- Constructor for a font class object.					* 
 *																					* 
 *	This constructs a font object as it is based upon the true type windows fonts.	* 
 *																					* 
 * INPUT:   char *filename -- True Type Windows font file to open.					*
 *			char *facename -- It's face name.										*
 *          int   height   -- The height requested.									*
 *																					* 
 * OUTPUT:		none																* 
 *																					* 
 * WARNINGS:	none																* 
 *                                                                                  * 
 * HISTORY:                                                                         * 
 *   08/24/1999 MML : Created.                                                      * 
 *==================================================================================*/
TTFontClass::TTFontClass(
	HDC		hdc,
	char *filename, 
	char *facename, 
	int		height, 
	int		weight, 
	BYTE	charset, 
	int		width, 
	int		escapement, 
	int		orientation, 
	BYTE	italic, 
	BYTE	underline, 
	BYTE	strikeout, 
	BYTE	outputPrecision, 
	BYTE	clipPrecision, 
	BYTE	quality, 
	BYTE	pitchAndFamily )
{
	HGDIOBJ		old_object;
	TEXTMETRIC	tm;
	char		real_facename[MAX_PATH];

	//--------------------------------------------------------------------------
	// Get or Set a Font filename.
	//--------------------------------------------------------------------------
	if (( filename == NULL ) || ( filename[0] == '\0' )) {
		strcpy( szFilename, "Arial.ttf" );
	} else {
		strcpy( szFilename, filename );
	}

	//--------------------------------------------------------------------------
	// Get or Set a Font facename.
	//--------------------------------------------------------------------------
	if (( facename == NULL ) || ( facename[0] == '\0' )) {
		strcpy( szFacename, "Arial" );
	} else {
		strcpy( szFacename, facename );
	}
	real_facename[0] = '\0';

	Msg( __LINE__, __FILE__, "TTFontClass -- filename=%s, facename=%s, height=%d.", filename, facename, height );

	//--------------------------------------------------------------------------
	// Make sure the font file is "Registered".  Then create the font.
	//--------------------------------------------------------------------------
	AddFontResource( szFilename );
	Font = CreateFont( 
				height, 
				width, 
				escapement, 
				orientation, 
				weight, 
				italic, 
				underline, 
				strikeout,
				charset, 
				outputPrecision, 
				clipPrecision, 
				quality, 
				pitchAndFamily, 
				szFacename );

	if ( hdc && ( Font != NULL )) {

		//----------------------------------------------------------------------
		// The GetTextFace function lets a program determine the face name of
		// THE font currently selected in the device context: 
		//----------------------------------------------------------------------
		old_object = SelectObject( hdc, Font );
		GetTextFace( hdc, ( sizeof( real_facename ) / sizeof( TCHAR )), real_facename );

		if( _stricmp( real_facename, szFacename ) != 0 ) {

			strcpy( szFilename, "Arial.ttf" );
			strcpy( szFacename, "Arial" );

			SelectObject( hdc, old_object );
			DeleteObject( Font );
			Font = NULL;

			Font = CreateFont(				
					height,				// height of font             
					width, 				// average character width    
					escapement, 		// angle of escapement        
					orientation, 		// base-line orientation angle
					weight, 			// font weight                
					italic, 			// italic attribute option    
					underline, 			// underline attribute option 
					strikeout,			// strikeout attribute option 
					charset, 			// character set identifier   
					outputPrecision, 	// output precision           
					clipPrecision, 		// clipping precision         
					quality, 			// output quality             
					pitchAndFamily, 	// pitch and family           
					szFacename );		// typeface name              

			real_facename[0] = '\0';
			old_object = SelectObject( hdc, Font );
			GetTextFace( hdc, ( sizeof( real_facename ) / sizeof( TCHAR )), real_facename );
		}
	}

	//--------------------------------------------------------------------------
	// Save off the height. 
	//--------------------------------------------------------------------------
	Height = height;

	if ( hdc ) {

		//-----------------------------------------------------------------------
		// Get info from the font in BackBuffer's DC.
		//-----------------------------------------------------------------------
		old_object = SelectObject( hdc, Font );
		GetTextMetrics( hdc, &tm );

		FontXSpacing	= GetTextCharacterExtra( hdc );
		Ascent			= tm.tmAscent;
		Descent			= tm.tmDescent;
		InternalLeading	= tm.tmInternalLeading;
		ExternalLeading	= tm.tmExternalLeading;
		AveCharWidth	= tm.tmAveCharWidth;
		MaxCharWidth	= tm.tmMaxCharWidth;
		Overhang		= tm.tmOverhang;
		Italic			= tm.tmItalic;
		Underlined		= tm.tmUnderlined;
		StruckOut		= tm.tmStruckOut;
		CharSet			= tm.tmCharSet;						// [OYO] It is important to support Double Byte Chars
 		SelectObject( hdc, old_object );

	} else {

		//-----------------------------------------------------------------------
		// Set default values.
		//-----------------------------------------------------------------------
		Ascent			= 0;
		Descent			= 0;
		InternalLeading	= 0;
		ExternalLeading	= 0;
		AveCharWidth	= 0;
		MaxCharWidth	= 0;
		Overhang		= 0;
		Italic			= 0;
		Underlined		= 0;
		StruckOut		= 0;    
		CharSet			= 0;								// [OYO]
		FontXSpacing	= 0;
	}
}


/*********************************************************************************************** 
 * TTFontClass::Char_Pixel_Width -- Fetch the pixel width of the character specified.          * 
 *                                                                                             * 
 *    This will return with the pixel width of the character specified.                        * 
 *                                                                                             * 
 * INPUT:   c  -- The character to determine the pixel width for.                              * 
 *                                                                                             * 
 * OUTPUT:  Returns with the pixel width of the character.                                     * 
 *                                                                                             * 
 * WARNINGS:   The return width is the screen real estate width which may differ from the      * 
 *             actual pixels of the character. This difference is controlled by the font       * 
 *             X spacing.                                                                      * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/

int TTFontClass::Char_Pixel_Width( HDC hdc, UINT c ) const
{
	HGDIOBJ old_object;
	ABC abc;

	abc.abcA = 0;
	abc.abcB = 0;
	abc.abcC = 0;

	if ( hdc ) {
		old_object = SelectObject( hdc, Font );
		GetCharABCWidths( hdc, c, c, &abc );
		SelectObject( hdc, old_object );
	}
	return( abc.abcA + abc.abcB + abc.abcC );
}

/************************************************************************************************ 
 * TTFontClass::Char_Pixel_Width -- Fetch the pixel width of the character specified.		   
 *																							    
 *    This will return with the pixel width of the character specified.						   
 *																							    
 * INPUT:  		HDC hdc		  		-- must be passed in from calling function.				   
 *				char const * string	-- the pointer to the character in the string.			   
 *				int *num_bytes		-- return number of bytes this character has: 1 or 2.	   
 *																							    
 * OUTPUT:		Returns with the pixel width of the character.                                 
 *																							    
 * WARNINGS:	The return width is the screen real estate width which may differ from the	   
 *				actual pixels of the character. This difference is controlled by the font	   
 *				X spacing.																	   
 *																							    
 * NOTE:		This is function determines if the character is a double-byte or single-byte   
 *				character, then calls the standard Char_Pixel_Width.						   
 *																							    
 * HISTORY:																					   
 *   05/26/1997 JLB : Created.																   
 *==============================================================================================*/
//
// [OYO]	Supports DBCS ( multi-byte characters ).
//
int TTFontClass::Char_Pixel_Width ( HDC hdc, char const * string, int *num_bytes ) const
{
	char const *letter = string;
	int	length = 0;
	UINT	c;

	//--------------------------------------------------------------------------
	// These values must be passed in.
	//--------------------------------------------------------------------------
	if ( string == NULL || *string == '\0' || hdc == NULL ) {
		return( 0 );
	}

	//--------------------------------------------------------------------------
	// If this value is passed in, the set the default value (1=single).
	//--------------------------------------------------------------------------
	if ( num_bytes!= NULL ) {
		*num_bytes = 1;
	}

	//--------------------------------------------------------------------------
	// Get the pixel width of the character.  If it is a double-byte character,
	// then num_bytes will come back as '2'.
	//--------------------------------------------------------------------------
	if( IsFontDBCS() && IsDBCSLeadByte( *letter )){		
		c = Get_Double_Byte_Char( letter, num_bytes );
		length += Char_Pixel_Width( hdc, c );									// pixel length of double-byte character.
	} else {
		length += Char_Pixel_Width( hdc, *letter );								// pixel length of single-byte character.
	}
	return( length );
}


/************************************************************************************************ 
 * TTFontClass::String_Pixel_Width -- Determines the width of the string in pixels.				* 
 *																								* 
 *    This routine is used to determine how many pixels wide the string will be if it were		* 
 *    printed.																					* 
 *																								* 
 * INPUT:		string   -- The string to convert into its pixel width.							* 
 *																								* 
 * OUTPUT:		Returns with the number of pixels the string would span if it were printed.		* 
 *																								* 
 * WARNINGS:	This routine does not take into account clipping.								* 
 *                                   															*
 * HISTORY:																						* 
 *   05/26/1997 JLB : Created.																	* 
 *==============================================================================================*/

int TTFontClass::String_Pixel_Width( HDC hdc, char const * string ) const
{
	if ( string == NULL ) { 
		return(0);
	}

	int		largest = 0;		// Largest recorded width of the string.
	int		width   = 0;
	HGDIOBJ	old_object;
	int		length;
	SIZE  	size;
	bool  	make_dc = FALSE;
	HDC		localDC = hdc;

	size.cx = 0;

	if ( localDC == NULL ) {
		return( size.cx );
	}

	if ( localDC ) {
		length = strlen( string );
		old_object = SelectObject( localDC, Font );
		GetTextExtentPoint32( localDC, string, length, &size );
		SelectObject( localDC, old_object );
	}
	return( size.cx );
}

/****************************************************************************
*
* NAME
*     String_Pixel_Bounds(String, Bounds)
*
* DESCRIPTION
*     Calculate the bounding box for the specified string.
*
* INPUTS
*     String - String to calculate bounds for
*     Bounds - Rect to fill with bounds
*
* RESULT
*     NONE
*
****************************************************************************/
//
// [OYO]	Supports DBCS ( multi-byte characters ).
//
void TTFontClass::String_Pixel_Bounds( HDC hdc, const char* string, Rect& bounds ) const
{
	int width;
	int height;

	bounds.Width = 0;
	bounds.Height = 0;

	if ( string == NULL ) {
		return;
	}

	if ( hdc == NULL ) {
		return;
	}

	width	= 0;
	height	= Get_Height();

	while ( *string != 0 ) {

		if (( *string == '\r' ) || ( *string == '\n' )) {

			string++;
			height += Get_Height();
			bounds.Width = max( bounds.Width, width );
			width = 0;

		} else if( IsFontDBCS()){

#if(0)
			//--------------------------------------------------------------------
			// Using one of those _tc functions:
			//		Get a character, get the width of that character, then
			//		move the pointer to the next character.
			//--------------------------------------------------------------------
			UINT c = Get_Double_Byte_Char( string );
			width += Char_Pixel_Width( hdc, c );
			string = _tcsinc( string );
#else
			//--------------------------------------------------------------------
			// MBCS way: Get a byte.  If byte is a "Lead" byte, get the second half,
			// combine them into one character, then get the width of that character.
			//--------------------------------------------------------------------
			UINT c = *(BYTE*)string++;
			if( IsDBCSLeadByte( c )&& *string ) {
				c = ( c << 8 ) | *(BYTE *)string++;
			}
			width += Char_Pixel_Width( hdc, c );
#endif

		} else {
			width += Char_Pixel_Width( hdc, *string++ );
		}
	}

	bounds.Width	= max( bounds.Width, width );
	bounds.Height	= height;
}

/*********************************************************************************************** 
 * TTFontClass::Get_Double_Byte_Char -- Get the first character even if DBSC.				  
 *                                                                                            
 * INPUT:   char * -- string to look at.                                                      
 *                                                                                            
 * OUTPUT:  Returns with the normalized width of the character in the font.					   
 *                                                                                            
 * WARNINGS:	I added this function based on code provided by Ikeda-san.					  
 *                                                                                            
 * HISTORY:                                                                                   
 *   06/05/2000 MML : Created.                                                                
 *=============================================================================================*/
//
// [OYO]	Supports DBCS ( multi-byte characters ).
//
UINT TTFontClass::Get_Double_Byte_Char	( const char *string, int *num_bytes ) const
{
	if ( string == NULL || *string == '\0' ) {
		return( 0 );
	}

	const char *ptr = string;
	UINT c = *(BYTE *)ptr++;

	if ( num_bytes != NULL ) {
		*num_bytes = 1;
	}

	//--------------------------------------------------------------------------
	// The IsDBCSLeadByte function determines whether a character is a 
	// lead byte - that is, the first byte of a character in a double-byte
	// character set (DBCS). 
	//
	//	BOOL IsDBCSLeadByte(  BYTE TestChar  /* character to test */ );
	//
	//	TestChar -- Specifies the character to be tested. 
	//
	// Return Values
	//		If the character is a lead byte, the return value is nonzero.
	//		If the character is not a lead byte, the return value is zero. 
	//		To get extended error information, call GetLastError. 
	//	Remarks
	//		Lead bytes are unique to double-byte character sets. 
	//		A lead byte introduces a double-byte character. Lead bytes 
	//		occupy a specific range of byte values. The IsDBCSLeadByte 
	//		function uses the ANSI code page to check lead-byte ranges. 
	//		To specify a different code page, use the IsDBCSLeadByteEx function. 
	//
	//	Declared in winnls.h.
	//--------------------------------------------------------------------------
	if( IsDBCSLeadByte( c )&& *ptr ) {		// [OYO]	
		c = ( c << 8 ) | *(BYTE *)ptr++;
		if ( num_bytes != NULL ) {
			*num_bytes = 2;
		}
	}
	return( c );
}


/*********************************************************************************************** 
 * TTFontClass::Get_Width -- Get normalized width of the nominal font character.               * 
 *                                                                                             * 
 *    This routine is used to fetch the width of the widest character in the font but the      * 
 *    width has been biased according to any X spacing override present.                       * 
 *                                                                                             * 
 * INPUT:   none                                                                               * 
 *                                                                                             * 
 * OUTPUT:  Returns with the normalized width of the widest character in the font.             * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int TTFontClass::Get_Width( void ) const
{
	return( MaxCharWidth - Overhang );
}

/*********************************************************************************************** 
 * TTFontClass::Get_Height -- Fetch the normalized height of the nominal font character.       * 
 *                                                                                             * 
 *    This will return the height of the font but the returned height will be adjusted by any  * 
 *    Y spacing override present.                                                              * 
 *                                                                                             * 
 * INPUT:   none                                                                               * 
 *                                                                                             * 
 * OUTPUT:  Returns with the height of the font normalized by any spacing overrides.           * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int TTFontClass::Get_Height( void ) const
{
	return( Height );
}

/*********************************************************************************************** 
 * TTFontClass::Set_XSpacing -- Set the X spacing override value.                              * 
 *                                                                                             * 
 *    Use this routine to control the horizontal spacing override for this font. If the value  * 
 *    is negative, the font becomes compressed. If the value is positive, then the font        * 
 *    becomes expanded.                                                                        * 
 *                                                                                             * 
 * INPUT:   x  -- The X spacing override to use for this font.                                 * 
 *                                                                                             * 
 * OUTPUT:  Returns with the old X spacing override value.                                     * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int TTFontClass::Set_XSpacing( HDC hdc, int x )
{
//	HGDIOBJ old_object;
//	int answer = 0;

//	if ( hdc ) {
//		old_object = SelectObject( hdc, Font );
//		answer = SetTextCharacterExtra( hdc, x );
//		SelectObject( hdc, old_object );
//	}
	FontXSpacing = x;
	return( x );
}


/*********************************************************************************************** 
 * TTFontClass::Set_YSpacing -- Set the vertical (Y) spacing override value.                   * 
 *                                                                                             * 
 *    Use this routine to control the "line spacing" of a font. If the Y spacing is negative   * 
 *    then the font becomes closer to the line above it. If value is positive, then more       * 
 *    space occurs between lines.                                                              * 
 *                                                                                             * 
 * INPUT:   y  -- The Y spacing override to use for this font.                                 * 
 *                                                                                             * 
 * OUTPUT:  Returns with the old Y spacing override value.                                     * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int TTFontClass::Set_YSpacing( int y )
{
	FontYSpacing = y;
	return(y);
}


/*********************************************************************************************** 
 * TTFontClass::Print -- Print text to the surface specified.  WCHAR Version.		           * 
 *                                                                                             * 
 *    This displays text to the surface specified and with the attributes specified.           * 
 *                                                                                             * 
 * INPUT:   string   -- The string to display to the surface.                                  * 
 *          surface  -- The surface to display the text upon.                                  * 
 *          cliprect -- The clipping rectangle that both clips the text and biases the print   * 
 *                      location to.                                                           * 
 *          point    -- The draw position for the upper left corner of the first character.    * 
 *          convert  -- The pixel convert object that is used to draw the correct colors to    * 
 *                      the destination surface.                                               * 
 *          remap    -- Auxiliary remap table for font colors.                                 * 
 *                                                                                             * 
 * OUTPUT:  Returns with the point where the next print should begin if it is to smoothly      * 
 *          continue where this print operation left off.                                      * 
 *                                                                                             * 
 * WARNINGS: There are two separate drawing routines; one for old fonts and one for new fonts. * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *	  06/20/1887 BNA : Modified to handle new fonts.														  *
 *=============================================================================================*/

#if(NDEBUG)

Point2D TTFontClass::Print( 
	HDC hdc, 
	wchar_t const * string, 
	Rect const & cliprect, 
	COLORREF forecolor,		/* = TEXT_COLOR,		*/
	COLORREF backcolor,		/* = TEXT_SHADOW_COLOR,	*/
	TextFormatType flag,  	/* = TPF_DEFAULT,		*/
	TextShadowType shadow )	/* = TPF_FULLSHADOW		*/
{
	char buffer[ _MAX_PATH ];
	int length = wcslen( string );

	memset( buffer, '\0', _MAX_PATH );
	WideCharToMultiByte( CodePage, 0, string, length, buffer, _MAX_PATH, NULL, NULL );

	return( Print( hdc, buffer, cliprect, forecolor, backcolor, flag, shadow ));
}

#else

Point2D TTFontClass::Print( 
	HDC hdc, 
	wchar_t const * string, 
	Rect const & cliprect, 
	COLORREF forecolor,		/* = TEXT_COLOR,		*/
	COLORREF backcolor,		/* = TEXT_SHADOW_COLOR,	*/
	TextFormatType flag,  	/* = TPF_DEFAULT,		*/
	TextShadowType shadow )	/* = TPF_FULLSHADOW		*/
{
	Point2D	point( cliprect.X, cliprect.Y );
	RECT	rect;
	HGDIOBJ	old_object;
	SIZE	size;
	int		length = 0;
	int		result = 0;

	//--------------------------------------------------------------------------
	// If no string, why continue?
	//--------------------------------------------------------------------------
	assert( string != NULL );
	assert( hdc != NULL );

	if (( string == NULL ) || ( string[0] == '\0' )) {
		return( point );
	}

	//--------------------------------------------------------------------------
	// Set up rectangle print regions for the "shadow" and the main color.
	//--------------------------------------------------------------------------
	length		= wcslen( string );
	rect.left	= cliprect.X;
	rect.top	= cliprect.Y;
	rect.right	= cliprect.X + cliprect.Width;
	rect.bottom	= cliprect.Y + cliprect.Height;

	Msg( __LINE__, __FILE__, "TTFontClass::Print -- string = %s.", string );
	Msg( __LINE__, __FILE__, "TTFontClass::Print -- strlen = %d.", length );
	Msg( __LINE__, __FILE__, "TTFontClass::Print -- rect = [%d, %d, %d, %d].", rect.left, rect.top, rect.right, rect.bottom );
//	Msg( __LINE__, __FILE__, "TTFontClass::Print -- rect2 = [%d, %d, %d, %d].", rect.left, rect.top, rect.right, rect.bottom );

	//--------------------------------------------------------------------------
	// Print the "Shadow" depending on style desired, then the text in the requested color.
	//--------------------------------------------------------------------------
	if ( hdc ) {

		assert( Font != NULL );

		old_object = SelectObject( hdc, Font );

		SetTextCharacterExtra( hdc, FontXSpacing );
		SetTextAlign( hdc, TA_LEFT | TA_TOP );
		SetBkMode( hdc, TRANSPARENT );

		SetTextColor( hdc, forecolor );

//		result = ExtTextOutW(
//			hdc,				// handle to DC
//			rect.left,			// x-coordinate of reference point
//			rect.top,			// y-coordinate of reference point
//			ETO_CLIPPED,		// text-output options
//			&rect,				// optional dimensions
//			string,				// string
//			length,				// number of characters in string
//			NULL				// array of spacing values
//		);

// 		result = TextOutW(
// 			hdc,				// handle to DC
// 			rect.left,			// x-coordinate of starting position
// 			rect.top,			// y-coordinate of starting position
// 			string,				// character string
// 			length				// number of characters
// 		);
 
 		result = DrawTextW( 
 			hdc, 
 			string, 
 			length, 
 			&rect, 
 			flag );
 
		GetTextExtentPointW( hdc, string, length, &size );
		SelectObject( hdc, old_object );
	}
	point.X += size.cx;
	return( point );
}

#endif

/*********************************************************************************************** 
 * TTFontClass::Print -- Print text to the surface specified.  CHAR version.                   * 
 *                                                                                             * 
 *    This displays text to the surface specified and with the attributes specified.           * 
 *                                                                                             * 
 * INPUT:   string   -- The string to display to the surface.                                  * 
 *          surface  -- The surface to display the text upon.                                  * 
 *          cliprect -- The clipping rectangle that both clips the text and biases the print   * 
 *                      location to.                                                           * 
 *          point    -- The draw position for the upper left corner of the first character.    * 
 *          convert  -- The pixel convert object that is used to draw the correct colors to    * 
 *                      the destination surface.                                               * 
 *          remap    -- Auxiliary remap table for font colors.                                 * 
 *                                                                                             * 
 * OUTPUT:  Returns with the point where the next print should begin if it is to smoothly      * 
 *          continue where this print operation left off.                                      * 
 *                                                                                             * 
 * WARNINGS: There are two separate drawing routines; one for old fonts and one for new fonts. * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *	  06/20/1887 BNA : Modified to handle new fonts.														  *
 *=============================================================================================*/

Point2D TTFontClass::Print( 
	HDC hdc, 
	char const * string, 
	Rect const & cliprect, 
	COLORREF forecolor,		/* = TEXT_COLOR,		*/
	COLORREF backcolor,		/* = TEXT_SHADOW_COLOR,	*/
	TextFormatType flag,  	/* = TPF_DEFAULT,		*/
	TextShadowType shadow )	/* = TPF_FULLSHADOW		*/
{
	Point2D	point( cliprect.X, cliprect.Y );
	RECT	rect, rect2;
	HGDIOBJ	old_object;
	SIZE	size;
	int		length = 0;
	int		result = 0;

	//--------------------------------------------------------------------------
	// If no string, why continue?
	//--------------------------------------------------------------------------
	assert( string != NULL );
	assert( hdc != NULL );

	if (( string == NULL ) || ( string[0] == '\0' )) {
		return( point );
	}

	//--------------------------------------------------------------------------
	// Set up rectangle print regions for the "shadow" and the main color.
	//--------------------------------------------------------------------------
	length		= strlen( string );
	rect.left	= cliprect.X;
	rect.top	= cliprect.Y;
	rect.right	= cliprect.X + cliprect.Width;
	rect.bottom	= cliprect.Y + cliprect.Height;

	// Shadow
	rect2.left	= rect.left - 1;
	rect2.top 	= rect.top  - 1;
	rect2.right	= rect.right - 1;
	rect2.bottom= rect.bottom - 1;

//	Msg( __LINE__, __FILE__, "TTFontClass::Print -- strlen = %d.", length );
//	Msg( __LINE__, __FILE__, "TTFontClass::Print -- rect = [%d, %d, %d, %d].", rect.left, rect.top, rect.right, rect.bottom );
//	Msg( __LINE__, __FILE__, "TTFontClass::Print -- rect2 = [%d, %d, %d, %d].", rect.left, rect.top, rect.right, rect.bottom );

	//--------------------------------------------------------------------------
	// Print the "Shadow" depending on style desired, then the text in the requested color.
	//--------------------------------------------------------------------------
	if ( hdc ) {

		assert( Font != NULL );

		old_object = SelectObject( hdc, Font );

		SetTextCharacterExtra( hdc, FontXSpacing );
		SetTextAlign( hdc, TA_LEFT | TA_TOP );
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, backcolor );

		if ( shadow == TPF_SHADOW ) {

			// One left, One up.
			result = DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One left.
			rect2.left		= rect.left - 1;
			rect2.top 		= rect.top;
			rect2.right		= rect.right - 1;
			rect2.bottom	= rect.bottom;
			result			= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One up.
			rect2.left	= rect.left;
			rect2.top 	= rect.top - 1;
			rect2.right		= rect.right;
			rect2.bottom	= rect.bottom - 1;
			result			= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

		} else if ( shadow == TPF_DOUBLESHADOW ) {

			// One left, One up.
			result = DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One left.
			rect2.left	= rect.left - 1;
			rect2.top 	= rect.top;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One up.
			rect2.left	= rect.left;
			rect2.top 	= rect.top - 1;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Two left, Two up.
			rect2.left	= rect.left - 2;
			rect2.top 	= rect.top  - 2;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Two left.
			rect2.left	= rect.left - 2;
			rect2.top 	= rect.top;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Two up.
			rect2.left	= rect.left;
			rect2.top 	= rect.top - 2;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

		} else if ( shadow == TPF_FULLSHADOW ) {

			// One left, One up.
			result = DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One right, One up.
			rect2.left	= rect.left + 1;
			rect2.top 	= rect.top  - 1;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One left, One down.
			rect2.left	= rect.left - 1;
			rect2.top 	= rect.top  + 1;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One right, One down.
			rect2.left	= rect.left + 1;
			rect2.top 	= rect.top  + 1;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One left.
			rect2.left	= rect.left - 1;
			rect2.top 	= rect.top;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One right.
			rect2.left	= rect.left + 1;
			rect2.top 	= rect.top;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One up.
			rect2.left	= rect.left;
			rect2.top 	= rect.top - 1;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// One down.
			rect2.left	= rect.left;
			rect2.top 	= rect.top + 1;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Two right.
			rect2.left	= rect.left + 2;
			rect2.top 	= rect.top;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Two down.
			rect2.left	= rect.left;
			rect2.top 	= rect.top + 2;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Two right, One up.
			rect2.left	= rect.left + 2;
			rect2.top 	= rect.top  - 2;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Two right, One down.
			rect2.left	= rect.left + 2;
			rect2.top 	= rect.top  + 2;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );


			// Three right.
			rect2.left	= rect.left + 3;
			rect2.top 	= rect.top;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Three down.
			rect2.left	= rect.left;
			rect2.top 	= rect.top + 3;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Three right, One up.
			rect2.left	= rect.left + 3;
			rect2.top 	= rect.top  - 3;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );

			// Three right, One down.
			rect2.left	= rect.left + 3;
			rect2.top 	= rect.top  + 3;
			result		= DrawText( hdc, string, length, &rect2, flag );
			assert( result != 0 );
		}

		SetTextColor( hdc, forecolor );
		result = DrawText( hdc, string, length,	&rect, flag  );		
		assert( result != 0 );

		GetTextExtentPoint( hdc, string, length, &size );
		SelectObject( hdc, old_object );
	}
	point.X += size.cx;
	return( point );
}

/***************************************************************************
 * TTFontClass::FIND_TEXT_VLENGTH -- Finds length of text in pixels        *
 *                                                                         *
 * INPUT:	BYTE *string - the strength to find the vertical length of 	   *
 *          int width    - the width of the region to fit it in        	   *
 *                                                                     	   *
 * OUTPUT:  int -- the number of pixels it takes up veritcally.        	   *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/27/1994 PWG : Created.                                             *
 *=========================================================================*/
//
// [OYO]	Supports DBCS ( multi-byte characters ).
//
int TTFontClass::Find_Text_VLength( HDC hdc, char *str, int width )
{
	int	curlen	= 0;
	int	lastlen	= 0;
	int	lines		= Get_Height();
	char *letter	= str; 
	bool	make_dc	= FALSE;
	HDC	localDC	= hdc;

	if ( *str == '\0' || str == NULL ) {
		return( 0 );
	}

	//--------------------------------------------------------------------------
	// If no DC was passed in, then we need to get one.
	//--------------------------------------------------------------------------
	if ( localDC == NULL ) {
		return( 0 );
	}

	//==========================================================================
	// Process languages EXCEPT Double-byte languages.
	//==========================================================================
	if( !(IS_LANGUAGE_DBCS( LanguageID ))) {

		//-----------------------------------------------------------------------
		// Get the pixel length of the string.
		//-----------------------------------------------------------------------
		curlen = String_Pixel_Width( localDC, str );
		lines = 0;

		//-----------------------------------------------------------------------
		// If the string in longer than the width given, calculate the number of 
		// lines needed in pixels (height of string in pixels ).
		//-----------------------------------------------------------------------
		if ( curlen > width ) {

			while( curlen > 0 ) {
				if ( curlen > width ) {
					curlen -= width;
				} else {
					curlen = 0;
				}
				lines += Get_Height();
			}

		} else {
			lines = Get_Height();
		}

		//-----------------------------------------------------------------------
		// Check for any newlines.  Add one line per newline.
		//-----------------------------------------------------------------------
		letter = str; 
		while ( *letter ) {
			if ( *letter == '\n') {
				lines += Get_Height();
			}
			letter++;
		}

	} else {

		//=======================================================================
		// Process Double-Byte language text.
		//=======================================================================
		int	i, n, wspc;
		UINT	c;
		int 	fdbcs = IsFontDBCS();

		lines	= 0;

		//-----------------------------------------------------------------------
		// For each word...
		//-----------------------------------------------------------------------
		while ( n = nGetWord( letter, fdbcs )) {

			//--------------------------------------------------------------------
			// For each character in the word...
			//--------------------------------------------------------------------
			for ( c=0, wspc=0, curlen=0, i=0; i < n; i++ ) {

				if ( c ) {
					//--------------------------------------------------------------
					// Double-byte character.
					//--------------------------------------------------------------
					c = ( c<<8 )|(UINT)(BYTE)letter[i];
					curlen += Char_Pixel_Width( localDC, c ) + wspc;
					c = 0;
					wspc = 0;

				} else if( fdbcs && IsDBCSLeadByte((BYTE)letter[i])) {
					//--------------------------------------------------------------
					// First half of a Double-byte character.
					//--------------------------------------------------------------
					c = (UINT)(BYTE)letter[i];

				} else if((BYTE)letter[i] > ' ' ) {
					//--------------------------------------------------------------
					// Single-byte character.
					//--------------------------------------------------------------
					curlen += Char_Pixel_Width( localDC, (UINT)(BYTE)letter[i] ) + wspc;
					wspc = 0;

				} else if( letter[i] == ' ') {
					//--------------------------------------------------------------
					// Space character.
					//--------------------------------------------------------------
					wspc += Char_Pixel_Width( localDC, ' ' );
				}
			} // end-of-for

			//--------------------------------------------------------------------
			//
			//--------------------------------------------------------------------
			if( lastlen + curlen > width ) {
				lines += Get_Height();
				lastlen = 0;
			}

			//--------------------------------------------------------------------
			// !!! If one block length bigger than available width, 
			// next line will be overflow. but never endless loop. !!!
			//--------------------------------------------------------------------
			lastlen += curlen + wspc;

			//--------------------------------------------------------------------
			//
			//--------------------------------------------------------------------
			if( letter[i] == '\n' ) {
				lines += Get_Height();
				lastlen = 0;
			}
			letter += n;

		} // end-of-while

		//-----------------------------------------------------------------------
		// Left over, add a line.
		//-----------------------------------------------------------------------
		if( lastlen ) {
			lines += Get_Height();
		}
	}

	//--------------------------------------------------------------------------
	// Return the number of lines.
	//--------------------------------------------------------------------------
	if ( !lines ) {
		lines = Get_Height();
	}
	return( lines );
}

/*********************************************************************************************** 
 * FontManagerClass::FontManagerClass -- Constructor for FontManager class.                    * 
 *                                                                                             * 
 * INPUT:		none.                                                                          *
 *                                                                                             * 
 * OUTPUT:  	none                                                                           * 
 *                                                                                             * 
 * WARNINGS:	none                                                                           * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   03/26/1998 MML : Created.                                                                 * 
 *=============================================================================================*/
FontManagerClass::FontManagerClass ( HDC hdc )
{
	//--------------------------------------------------------------------------
	// Open a DC to the BackBuffer.
	//--------------------------------------------------------------------------
	if ( hdc ) {

		char	szPath[ MAX_PATH ];
		char	szFile[ MAX_PATH ];
		char	szFacename[ MAX_PATH ];
		char	drive[ _MAX_DRIVE ];
		char	dir[ _MAX_DIR ];
		bool	b640X480 = false;
	  	RECT	rect;								// Desktop Window ( used once ).

		strcpy( szFile, "Arial.ttf" );
		strcpy( szFacename, "Arial" );

		strcpy( szPath, Args->Get_argv(0));
		_splitpath( szPath, drive, dir, NULL, NULL );
		_makepath( szPath, drive, dir, "Setup\\Setup", ".ini" );

		GetPrivateProfileString( "Fonts", "Font", "Arial.tff", szFile, MAX_PATH, szPath );
		GetPrivateProfileString( "Fonts", "Fontname", "Arial", szFacename, MAX_PATH, szPath );

		//---------------------------------------------------------------------
		// Use codepage set by Locomoto class.
		//---------------------------------------------------------------------
		UINT codepage = CodePage;				// GetACP();

		GetClientRect( GetDesktopWindow(), &rect );
		if( rect.right <= 640 ) {
			b640X480 = TRUE;
		}

		Msg( __LINE__, __FILE__, "FontManagerClass -- szFile = %s, szFilename = %s.", szFile, szFacename );

		//---------------------------------------------------------------------
		// Create the True Type Fonts.
		//
		//	Value			Weight 
		//	____________________________
		//	FW_DONTCARE		0 
		//	FW_THIN			100 
		//	FW_EXTRALIGHT	200 
		//	FW_ULTRALIGHT	200 
		//	FW_LIGHT		300 
		//	FW_NORMAL		400 
		//	FW_REGULAR		400 
		//	FW_MEDIUM		500 
		//	FW_SEMIBOLD		600 
		//	FW_DEMIBOLD		600 
		//	FW_BOLD			700 
		//	FW_EXTRABOLD	800 
		//	FW_ULTRABOLD	800 
		//	FW_HEAVY		900 
		//	FW_BLACK		900 
		//---------------------------------------------------------------------

		switch( LanguageID ) { 	// [OYO] Add this line if you wish to support another languages

			//=================================================================
			// JAPANESE
			//=================================================================
			case LANG_JAP:		// [OYO] Use MS PGothic for Japanese Win9x

				if( codepage == 932 ) {

					strcpy( szFile, "MSGothic.ttc" );
					strcpy( szFacename, "MS PGothic" );

					TTButtonFontPtr		= new TTFontClass( hdc, szFile, szFacename, 20, FW_NORMAL,	SHIFTJIS_CHARSET );
					TTButtonFontPtrSmall= new TTFontClass( hdc, szFile, szFacename, 12, FW_NORMAL,	SHIFTJIS_CHARSET );

					TTTextFontPtr		= new TTFontClass( hdc, szFile, szFacename, 16, FW_MEDIUM,	SHIFTJIS_CHARSET );
					TTTextFontPtr640	= new TTFontClass( hdc, szFile, szFacename, 14, FW_MEDIUM,	SHIFTJIS_CHARSET );
					TTTextFontPtr800	= new TTFontClass( hdc, szFile, szFacename, 14, FW_MEDIUM,	SHIFTJIS_CHARSET );
					TTLicenseFontPtr	= new TTFontClass( hdc, szFile, szFacename, 12, FW_NORMAL,	SHIFTJIS_CHARSET );
				}																	
				break;

			//=================================================================
			// KOREAN
			//=================================================================
			case LANG_KOR:		// [OYO] Use GulimChe for Korean Win9x

				if ( codepage == 949 ) {

					strcpy( szFile, "Gulim.tff" );
					strcpy( szFacename, "Gulim" );

					TTButtonFontPtr		= new TTFontClass( hdc, szFile,	szFacename, 20, FW_NORMAL,	HANGEUL_CHARSET );
					TTButtonFontPtrSmall= new TTFontClass( hdc, szFile, szFacename, 20, FW_NORMAL,	HANGEUL_CHARSET );

					TTTextFontPtr		= new TTFontClass( hdc, szFile,	szFacename, 16, FW_MEDIUM,	HANGEUL_CHARSET );
					TTTextFontPtr640	= new TTFontClass( hdc, szFile, szFacename, 14, FW_MEDIUM,	HANGEUL_CHARSET );
					TTTextFontPtr800	= new TTFontClass( hdc, szFile, szFacename, 14, FW_MEDIUM,	HANGEUL_CHARSET );
					TTLicenseFontPtr	= new TTFontClass( hdc, szFile, szFacename, 12, FW_NORMAL,	HANGEUL_CHARSET );
				}																	
				break;

			//=================================================================
			// CHINESE
			//=================================================================
			case LANG_CHI:

				if ( codepage == 950 ) {

					strcpy( szFile, "mingliu.ttc" );
					strcpy( szFacename, "mingliu" );

					TTButtonFontPtr		= new TTFontClass( hdc, szFile, szFacename, 20, FW_NORMAL,	CHINESEBIG5_CHARSET );
					TTButtonFontPtrSmall= new TTFontClass( hdc, szFile, szFacename, 14, FW_NORMAL,	CHINESEBIG5_CHARSET );

					TTTextFontPtr		= new TTFontClass( hdc, szFile, szFacename, 16, FW_MEDIUM,	CHINESEBIG5_CHARSET );
					TTTextFontPtr640	= new TTFontClass( hdc, szFile, szFacename, 14, FW_MEDIUM,	CHINESEBIG5_CHARSET );
					TTTextFontPtr800	= new TTFontClass( hdc, szFile, szFacename, 14, FW_MEDIUM,	CHINESEBIG5_CHARSET );
					TTLicenseFontPtr	= new TTFontClass( hdc, szFile, szFacename, 12, FW_NORMAL,	CHINESEBIG5_CHARSET );
				}
				break;

			//=================================================================
			// ENGLISH, FRENCH, GERMAN
			//=================================================================
			case LANG_GER:
			case LANG_FRE:
			case LANG_USA:
			default:

				TTButtonFontPtr		= new TTFontClass( hdc, szFile, szFacename, 22, FW_SEMIBOLD,	ANSI_CHARSET, 0, 0, 0, FALSE );

				if( LANG_FRE == LanguageID ) {
					TTButtonFontPtrSmall= new TTFontClass( hdc, szFile, szFacename, 20, FW_SEMIBOLD, ANSI_CHARSET, 0, 0, 0, FALSE );
				} else {
					TTButtonFontPtrSmall= new TTFontClass( hdc, szFile, szFacename, 22, FW_SEMIBOLD, ANSI_CHARSET, 0, 0, 0, FALSE );
				}

				TTTextFontPtr		= new TTFontClass( hdc, szFile, szFacename, 16, FW_SEMIBOLD,	ANSI_CHARSET, 0, 0, 0, FALSE );
				TTTextFontPtr640	= new TTFontClass( hdc, szFile, szFacename, 14, FW_SEMIBOLD,	ANSI_CHARSET, 0, 0, 0, FALSE );
				TTTextFontPtr800	= new TTFontClass( hdc, szFile, szFacename, 14, FW_SEMIBOLD,	ANSI_CHARSET, 0, 0, 0, FALSE );
				TTLicenseFontPtr	= new TTFontClass( hdc, szFile, szFacename, 12, FW_MEDIUM,		ANSI_CHARSET, 0, 0, 0, FALSE );
				break;
		}

		//----------------------------------------------------------------------
		// If we fell through...
		//----------------------------------------------------------------------
		if( TTButtonFontPtr == NULL || TTTextFontPtr == NULL ) {

			strcpy( szFile, "Arial.tff" );
			strcpy( szFacename, "Arial" );

			if( TTButtonFontPtr	== NULL ) {
				TTButtonFontPtr		= new TTFontClass( hdc, szFile, szFacename, 22, FW_SEMIBOLD,	ANSI_CHARSET, 0, 0, 0, FALSE );
			}
			if( TTButtonFontPtrSmall == NULL ) {
				TTButtonFontPtrSmall= new TTFontClass( hdc, szFile, szFacename, 22, FW_SEMIBOLD,	ANSI_CHARSET, 0, 0, 0, FALSE );
			}
			if( TTTextFontPtr == NULL ) {
				TTTextFontPtr		= new TTFontClass( hdc, szFile, szFacename, 16, FW_SEMIBOLD,	ANSI_CHARSET, 0, 0, 0, FALSE );
			}
			if( TTTextFontPtr640 == NULL ) {
				TTTextFontPtr640	= new TTFontClass( hdc, szFile, szFacename, 14, FW_SEMIBOLD, 	ANSI_CHARSET, 0, 0, 0, FALSE );
			}
			if( TTTextFontPtr800 == NULL ) {
				TTTextFontPtr800	= new TTFontClass( hdc, szFile, szFacename, 14, FW_SEMIBOLD, 	ANSI_CHARSET, 0, 0, 0, FALSE );
			}
			if( TTLicenseFontPtr == NULL ) {
				TTLicenseFontPtr	= new TTFontClass( hdc, szFile, szFacename, 12, FW_MEDIUM,		ANSI_CHARSET, 0, 0, 0, FALSE );
			}
		}
	}
	assert( TTTextFontPtr			!= NULL );
	assert( TTTextFontPtr640		!= NULL );
	assert( TTTextFontPtr800		!= NULL );
	assert( TTButtonFontPtr			!= NULL );
	assert( TTButtonFontPtrSmall	!= NULL );
	assert( TTLicenseFontPtr		!= NULL );
}

/*********************************************************************************************** 
 * FontManagerClass::~FontManagerClass -- Destructor for FontManager class.                    * 
 *                                                                                             * 
 * INPUT:		none.                                                                          *
 *                                                                                             * 
 * OUTPUT:  	none                                                                           * 
 *                                                                                             * 
 * WARNINGS:	none																		   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   03/26/1998 MML : Created.                                                                 * 
 *=============================================================================================*/
FontManagerClass::~FontManagerClass ( void )
{
	if ( TTButtonFontPtr != NULL ) {
		delete TTButtonFontPtr;
		TTButtonFontPtr = NULL;
	}
	if ( TTTextFontPtr != NULL ) {
		delete TTTextFontPtr;
		TTTextFontPtr = NULL;
	}
}


/*********************************************************************************************** 
 * Font_From_TPF -- Convert flags into a font pointer.                                         * 
 *                                                                                             * 
 *    This routine will examine the specified flags and return with a pointer to the font      * 
 *    that the flags represent.                                                                * 
 *                                                                                             * 
 * INPUT:   flags -- The flags to convert into a font pointer.                                 * 
 *                                                                                             * 
 * OUTPUT:  Returns with a font pointer that matches the flags.                                * 
 *                                                                                             * 
 * WARNINGS:   If no match could be found, a default font pointer is returned.                 * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
TTFontClass * Font_From_TPF ( TextPrintType flags )
{
	TTFontClass *fontptr= NULL;
   
	switch (flags & 0x000F) {

		case TPF_BUTTON_FONT:
			fontptr = TTButtonFontPtr;
			break;

		case TPF_TEXT_FONT:
			fontptr = TTTextFontPtr;
			break;

		default:
			fontptr = TTTextFontPtr;
			break;
	}
	return( fontptr );
}


/************************************************************************************************ 
 * Is_True_Type_Font -- Convert flags into a font pointer.										* 
 *																								* 
 *    This routine will examine the specified flags and return with a pointer to the font		* 
 *    that the flags represent.																	* 
 *																								* 
 * INPUT:   flags -- The flags to convert into a font pointer.									* 
 *																								* 
 * OUTPUT:  Returns with a font pointer that matches the flags.									* 
 *																								* 
 * WARNINGS:   If no match could be found, a default font pointer is returned.					* 
 *																								* 
 * HISTORY:																						* 
 *   05/26/1997 JLB : Created.																	* 
 *==============================================================================================*/

bool Is_True_Type_Font( TextPrintType flags )
{
	if (( flags == TPF_BUTTON_FONT ) || ( flags == TPF_TEXT_FONT )) {
		return TRUE;
	} else { 
		return FALSE;
	}
}



