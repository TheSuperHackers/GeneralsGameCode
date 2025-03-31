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

/******************************************************************************
 **	C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S				***
 ******************************************************************************
 *																			  *
 *                  Project Name : Autorun									  *
 *																			  *
 *                     File Name : AUTORUN.CPP								  *
 *																			  *
 *                    Programmers: Maria del Mar McCready Legg				  * 
 *																			  *
 *                    Start Date : September 5, 1997						  *
 *																			  *
 *                   Last Update : October 2, 2000  [MML]					  *
 *																			  *
 *----------------------------------------------------------------------------*
 * Functions:																  *
 *		DrawButton::DrawButton												  *
 *		DrawButton::Is_Mouse_In_Region										  *
 *		DrawButton::Return_Bitmap											  *
 *		DrawButton::Return_Area												  *
 *		DrawButton::Set_Stretched_Width										  *
 *		DrawButton::Set_Stretched_Height									  *
 *----------------------------------------------------------------------------*/


#define  STRICT
#include <windows.h>
#include <windowsx.h>
#include "autorun.h"
#include "DrawButton.h"
#include "Locale_API.h"
#include "Wnd_File.h"



#include "leanAndMeanAutorun.h"

#ifndef LEAN_AND_MEAN
///////GAMEENGINE HEADERS////////////
#include "Common/UnicodeString.h" 
#include "Common/SubsystemInterface.h"
#include "GameClient/GameText.h"
#endif


//*****************************************************************************
//  DrawButton::DrawButton -- Constructor for custom "Button" type.
// 
// 	This custom button is drawn by the dialog, and uses the WM_MOUSEMOVE,
//	WM_LBUTTONUP, and WM_LBUTTONDOWN to trigger actions.
//
//  INPUT:  	int id -- button id handle
//				int x  -- x position
//				int y  -- y position
//				int w  -- width of button
//				int h  -- height of button
//				char *normal  -- name of normal button bitmap
//				char *pressed -- name of pressed button bitmap
//				char *focus   -- name of focus button bitmap
//
//	OUTPUT: 	none.
//
//  WARNINGS:	No keyboard/mouse/paint handling built in.  Do manually.
//
//  HISTORY:                                                                
//      07/15/1996  MML : Created.                                            
//=============================================================================

DrawButton::DrawButton ( int id, RECT button_rect, const char *normal, const char *focus, const char *pressed, const char * string, TTFontClass *fontptr )
{
	Id = id;

	//--------------------------------------------------------------------------
	// Set default rectangle settings.
	//--------------------------------------------------------------------------
	rect.left	 	= button_rect.left;
	rect.top	 	= button_rect.top;
	rect.right	 	= button_rect.left + button_rect.right;
	rect.bottom	 	= button_rect.top  + button_rect.bottom;

	MyRect.X	 	= button_rect.left;
	MyRect.Y	 	= button_rect.top;
	MyRect.Width	= rect.right  - rect.left;
	MyRect.Height	= rect.bottom - rect.top;

	TextRect.X	 	= button_rect.left;
	TextRect.Y	 	= button_rect.top;
	TextRect.Width	= rect.right  - rect.left;
	TextRect.Height	= rect.bottom - rect.top;
	
	StretchedWidth 	= rect.right  - rect.left;
	StretchedHeight	= rect.bottom - rect.top;

	//--------------------------------------------------------------------------
	// Set the string variables.
	//--------------------------------------------------------------------------
	memset( String, '\0', MAX_PATH );
//	if ( string != NULL ) {
//		wcscpy( String, Locale_GetString( string_num, String ));


#ifdef LEAN_AND_MEAN
#else
	UnicodeString tempString = TheGameText->fetch(string);
	wcscpy(String, tempString.str());
#endif
//	}

	//--------------------------------------------------------------------------
	// Set the font pointer.
	//--------------------------------------------------------------------------
	MyFontPtr = NULL;

	if ( fontptr != NULL ) {
		MyFontPtr = fontptr;
	}

	//--------------------------------------------------------------------------
	// Set Button Backgrounds.
	//--------------------------------------------------------------------------
	_tcscpy( NormalBitmap, normal );
	_tcscpy( PressedBitmap, pressed );
	_tcscpy( FocusBitmap, focus );

	if( NormalBitmap[0] != '\0' ) {
		UseBitmaps = true;							// determines how to draw button.
	}

	//--------------------------------------------------------------------------
	// Start in normal mode.
	//--------------------------------------------------------------------------
	ButtonState	   	= NORMAL_STATE;

	Msg( __LINE__, TEXT(__FILE__), TEXT("	rect	= [%d,%d,%d,%d]"), rect.left, rect.top, rect.right, rect.bottom );
	Msg( __LINE__, TEXT(__FILE__), TEXT("	MyRect	= [%d,%d,%d,%d]"), MyRect.X, MyRect.Y, MyRect.Width, MyRect.Height );
}

DrawButton::DrawButton ( int id, RECT button_rect, const char *normal, const char *focus, const char *pressed, const wchar_t *string, TTFontClass *fontptr )
{
	Id = id;

	//--------------------------------------------------------------------------
	// Set default rectangle settings.
	//--------------------------------------------------------------------------
	rect.left	 	= button_rect.left;
	rect.top	 	= button_rect.top;
	rect.right	 	= button_rect.left + button_rect.right;
	rect.bottom	 	= button_rect.top  + button_rect.bottom;

	MyRect.X	 	= button_rect.left;
	MyRect.Y	 	= button_rect.top;
	MyRect.Width	= rect.right  - rect.left;
	MyRect.Height	= rect.bottom - rect.top;

	TextRect.X	 	= button_rect.left;
	TextRect.Y	 	= button_rect.top;
	TextRect.Width	= rect.right  - rect.left;
	TextRect.Height	= rect.bottom - rect.top;

	StretchedWidth 	= rect.right  - rect.left;
	StretchedHeight	= rect.bottom - rect.top;

	//--------------------------------------------------------------------------
	// Set the string variables.
	//--------------------------------------------------------------------------
	memset( String, '\0', MAX_PATH );
	if ( string != NULL ) {
		wcscpy( String, string );
	}

	//--------------------------------------------------------------------------
	// Set the font pointer.
	//--------------------------------------------------------------------------
	MyFontPtr = NULL;

	if ( fontptr != NULL ) {
		MyFontPtr = fontptr;
	}

	//--------------------------------------------------------------------------
	// Set Button Backgrounds.
	//--------------------------------------------------------------------------
	_tcscpy( NormalBitmap, normal );
	_tcscpy( PressedBitmap, pressed );
	_tcscpy( FocusBitmap, focus );
	
	if( NormalBitmap[0] != '\0' ) {
		UseBitmaps = true;							// determines how to draw button.
	}

	//--------------------------------------------------------------------------
	// Start in normal mode.
	//--------------------------------------------------------------------------
	ButtonState	= NORMAL_STATE;

	Msg( __LINE__, TEXT(__FILE__), TEXT("	rect	= [%d,%d,%d,%d]"), rect.left, rect.top, rect.right, rect.bottom );
	Msg( __LINE__, TEXT(__FILE__), TEXT("	MyRect	= [%d,%d,%d,%d]"), MyRect.X, MyRect.Y, MyRect.Width, MyRect.Height );
}

//*****************************************************************************
// DrawButton::Draw_Text -- Check if mouse values are in button area.
// 
// INPUT: 		HDC hDC -- DC to paint to.
//
//	OUTPUT:    	none.
//
// WARNINGS:	
//
// HISTORY:                                                                
//   01/18/2002  MML : Created.                                            
//=============================================================================

void DrawButton::Draw_Text ( HDC hDC )
{
	RECT	outline_rect;
	Rect	rect;

	if( hDC == NULL ) {
		return;
	}

	Return_Area( &outline_rect );
	Return_Text_Area( &rect );

	/*
	** This function was combining the pixel color with the background,
	** so it never looked correct.
	*/
//	SetTextColor( hDC, RGB( 0, 240, 0 ));
//	DrawFocusRect(	hDC, &dst_rect );
			
	if ( Get_State() == DrawButton::PRESSED_STATE ) {
		MyFontPtr->Print( 
			hDC, 
			String, 
			rect, 
			TEXT_PRESSED_COLOR, 
			TEXT_PRESSED_SHADOW_COLOR, 
			TPF_BUTTON,
			TPF_SHADOW );

	} else if ( Get_State() == DrawButton::FOCUS_STATE ) {
		MyFontPtr->Print( 
			hDC, 
			String, 
			rect, 
			TEXT_FOCUSED_COLOR, 
			TEXT_FOCUSED_SHADOW_COLOR, 
			TPF_BUTTON,
			TPF_SHADOW );

	} else {
		MyFontPtr->Print( 
			hDC, 
			String, 
			rect, 
			TEXT_NORMAL_COLOR,	
			TEXT_NORMAL_SHADOW_COLOR, 
			TPF_BUTTON,
			TPF_SHADOW );
	}
}


//*****************************************************************************
// DrawButton::Is_Mouse_In_Region -- Check if mouse values are in button area.
// 
// INPUT: 		int mouse_x  -- mouse x position
//				int mouse_y  -- mouse y position
//
//	OUTPUT: 	bool -- true of false.
//
// WARNINGS:	No keyboard/mouse/paint handling built in.  Do manually.
//				Note: width is shortened below to accomodate actual bitmap area.
//
// HISTORY:                                                                
//   07/15/1996  MML : Created.                                            
//=============================================================================

bool DrawButton::Is_Mouse_In_Region ( int mouse_x, int mouse_y )
{
	if ( mouse_x < 0 || mouse_y < 0 ) {
		return( false );
	}

	if (( mouse_x >= rect.left )	&& 
		( mouse_y >= rect.top )		&&
		( mouse_x <= rect.left + StretchedWidth ) && 
		( mouse_y <= rect.top  + StretchedHeight )) {
		return ( TRUE );
	}
	return ( FALSE );
}

//*****************************************************************************
// DrawButton::Return_Bitmap 
//
//	    Return name of correct bitmap based on state of button.
// 
// INPUT:  		none.
//
// OUTPUT: 		char *bitmap -- name of bitmap file.
//
// WARNINGS:	No keyboard/mouse/paint handling built in.  Do manually.
//
// HISTORY:                                                                
//   07/15/1996  MML : Created.                                            
//=============================================================================

char *DrawButton::Return_Bitmap ( void )
{
	if ( ButtonState == PRESSED_STATE ) {
		return ( PressedBitmap );
	} else if ( ButtonState == FOCUS_STATE ) {
		return ( FocusBitmap );
	} else {
		return ( NormalBitmap );
	}
}

//*****************************************************************************
// DrawButton::Return_Area -- Return x, y and width and height of button.
// 
// INPUT:  		RECT area -- holds the x,y,width,height information.
//
// OUTPUT: 		none.
//
// WARNINGS:	No keyboard/mouse/paint handling built in.  Do manually.
//
// HISTORY:                                                                
//   07/15/1996  MML : Created.                                            
//=============================================================================

void DrawButton::Return_Area ( RECT *area )
{
	area->left		= rect.left;
	area->top		= rect.top;
	area->right		= rect.left +	StretchedWidth;
	area->bottom	= rect.top  +	StretchedHeight;
}

void DrawButton::Return_Area ( Rect *area )
{
	area->X			= MyRect.X;
	area->Y			= MyRect.Y;
	area->Width		= MyRect.Width;
	area->Height	= MyRect.Height;
}

void DrawButton::Return_Text_Area ( Rect *area )
{
	area->X			= TextRect.X;
	area->Y			= TextRect.Y;
	area->Width		= TextRect.Width;
	area->Height	= TextRect.Height;
}


//*****************************************************************************
// DrawButton::Set_Stretched_Width -- Set draw width of button.
// 
// INPUT:  		int value -- destination width size.
//
//OUTPUT: 		none.
//
// WARNINGS:	none.
//
// HISTORY:    08/12/1996  MML : Created.                                            
//=============================================================================

int DrawButton::Set_Stretched_Width	 ( int value )
{
	int nWidth = StretchedWidth;

	StretchedWidth = value;
	return ( nWidth );
}

//*****************************************************************************
// DrawButton::Set_Stretched_Height -- Set draw height of button.
// 
// INPUT:  		int value -- destination height size.
//
// OUTPUT: 		none.
//
// WARNINGS:	none.
//
// HISTORY:    08/12/1996  MML : Created.                                            
//=============================================================================

int DrawButton::Set_Stretched_Height ( int value )
{
	int nHeight = StretchedHeight;

	StretchedHeight = value;
	return( nHeight );
}


