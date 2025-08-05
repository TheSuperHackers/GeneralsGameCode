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

// FILE: GenericProperties.cpp ////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    GUIEdit
//
// File name:  GenericProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Properties dialog for generic windows
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "Common/FunctionLexicon.h"
#include "GUIEdit.h"
#include "Properties.h"
#include "resource.h"
#include "EditWindow.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// genericPropertiesCallback ==================================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK genericPropertiesCallback( HWND hWndDialog,
																									 UINT message,
																									 WPARAM wParam,
																									 LPARAM lParam )
{
	Int returnCode;

	//
	// handle any common messages between all property dialogs cause they
	// are designed to have controls doing the same functionality
	// and names
	//
	if( HandleCommonDialogMessages( hWndDialog, message,
																	wParam, lParam, &returnCode ) == TRUE )
		return returnCode;

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_DRAWITEM:
		{
      UINT controlID = (UINT)wParam;  // control identifier
      LPDRAWITEMSTRUCT drawItem = (LPDRAWITEMSTRUCT)lParam; // item drawing
			RGBColorInt *color = GetControlColor( controlID );

			// we only care about color button controls
			if( color )
			{
				HBRUSH hBrushNew, hBrushOld;
				RECT rect;
				HWND hWndControl = GetDlgItem( hWndDialog, controlID );

				// if this control is disabled just let windows handle drawing
				if( IsWindowEnabled( hWndControl ) == FALSE )
					return FALSE;

				// Get the area we have to draw in
				GetClientRect( hWndControl, &rect );

        // create a new brush and select it into DC
        hBrushNew = CreateSolidBrush (RGB ((BYTE)color->red,
                                           (BYTE)color->green,
                                           (BYTE)color->blue));
        hBrushOld = (HBRUSH)SelectObject( drawItem->hDC, hBrushNew );

        // draw the rectangle
        Rectangle( drawItem->hDC, rect.left, rect.top, rect.right, rect.bottom );

        // put the old brush back and delete the new one
        SelectObject( drawItem->hDC, hBrushOld );
        DeleteObject( hBrushNew );

        // validate this new area
        ValidateRect( hWndControl, NULL );

				// we have taken care of it
				return TRUE;

			}  // end if

			return FALSE;

		}  // end draw item

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {
//			Int notifyCode = HIWORD( wParam );  // notification code
			Int controlID = LOWORD( wParam );  // control ID
			HWND hWndControl = (HWND)lParam;  // control window handle

      switch( controlID )
      {

				// --------------------------------------------------------------------
				case BUTTON_ENABLED_COLOR:
				case BUTTON_ENABLED_BORDER_COLOR:
				case BUTTON_DISABLED_COLOR:
				case BUTTON_DISABLED_BORDER_COLOR:
				case BUTTON_HILITE_COLOR:
				case BUTTON_HILITE_BORDER_COLOR:
				{
					RGBColorInt *currColor = GetControlColor( controlID );

					// bring up color selector for this color control at the mouse
					if( currColor )
					{
						RGBColorInt *newColor;
						POINT mouse;

						GetCursorPos( &mouse );
						newColor = SelectColor( currColor->red, currColor->green,
																		currColor->blue, currColor->alpha,
																		mouse.x, mouse.y );

						if( newColor )
						{
							Color newGameColor = GameMakeColor( newColor->red,
																									newColor->green,
																									newColor->blue,
																									newColor->alpha );
							SetControlColor( controlID, newGameColor );
							InvalidateRect( hWndControl, NULL, TRUE );

						}  // end if

					}  // end if

					break;

				}  // end color buttons

				// --------------------------------------------------------------------
        case IDOK:
				{
					GameWindow *window = TheEditor->getPropertyTarget();

					// sanity
					if( window )
					{
						const Image *image;
						RGBColorInt *rgbColor;
						Color color;

						// save the common properties
						if( SaveCommonDialogProperties( hWndDialog, window ) == FALSE )
							break;

						// save callbacks
						SaveCallbacks( window, hWndDialog );

						// save the enabled colors/images
						image = ComboBoxSelectionToImage( GetDlgItem( hWndDialog, COMBO_ENABLED_IMAGE ) );
						window->winSetEnabledImage( 0, image );
						rgbColor = GetControlColor( BUTTON_ENABLED_COLOR );
						color = GameMakeColor( rgbColor->red, rgbColor->green, rgbColor->blue, rgbColor->alpha );
						window->winSetEnabledColor( 0, color );
						rgbColor = GetControlColor( BUTTON_ENABLED_BORDER_COLOR );
						color = GameMakeColor( rgbColor->red, rgbColor->green, rgbColor->blue, rgbColor->alpha );
						window->winSetEnabledBorderColor( 0, color );

						// save the disabled colors/images
						image = ComboBoxSelectionToImage( GetDlgItem( hWndDialog, COMBO_DISABLED_IMAGE ) );
						window->winSetDisabledImage( 0, image );
						rgbColor = GetControlColor( BUTTON_DISABLED_COLOR );
						color = GameMakeColor( rgbColor->red, rgbColor->green, rgbColor->blue, rgbColor->alpha );
						window->winSetDisabledColor( 0, color );
						rgbColor = GetControlColor( BUTTON_DISABLED_BORDER_COLOR );
						color = GameMakeColor( rgbColor->red, rgbColor->green, rgbColor->blue, rgbColor->alpha );
						window->winSetDisabledBorderColor( 0, color );

						// save the hilite colors/images
						image = ComboBoxSelectionToImage( GetDlgItem( hWndDialog, COMBO_HILITE_IMAGE ) );
						window->winSetHiliteImage( 0, image );
						rgbColor = GetControlColor( BUTTON_HILITE_COLOR );
						color = GameMakeColor( rgbColor->red, rgbColor->green, rgbColor->blue, rgbColor->alpha );
						window->winSetHiliteColor( 0, color );
						rgbColor = GetControlColor( BUTTON_HILITE_BORDER_COLOR );
						color = GameMakeColor( rgbColor->red, rgbColor->green, rgbColor->blue, rgbColor->alpha );
						window->winSetHiliteBorderColor( 0, color );

					}  // end if

          DestroyWindow( hWndDialog );
          break;

				}  // end OK

				// --------------------------------------------------------------------
        case IDCANCEL:
				{

          DestroyWindow( hWndDialog );
          break;

				}  // end cancel

      }  // end switch( LOWORD( wParam ) )

      return 0;

    } // end of WM_COMMAND

		// ------------------------------------------------------------------------
    case WM_CLOSE:
		{

      DestroyWindow( hWndDialog );
      return 0;

		}  // end close

		// ------------------------------------------------------------------------
		default:
			return 0;

  }  // end of switch

}  // end genericPropertiesCallback

// InitCallbackCombos =========================================================
/** load the callbacks combo boxes with the functions that the user cal
	* select to attach to this window */
//=============================================================================
void InitCallbackCombos( HWND dialog, GameWindow *window )
{
	HWND combo;
	FunctionLexicon::TableEntry *entry;
	GameWindowEditData *editData = NULL;
	AsciiString name;

	// get edit data from window
	if( window )
		editData = window->winGetEditData();

	// load the system combo ----------------------------------------------------
	combo = GetDlgItem( dialog, COMBO_SYSTEM );
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_GAME_WIN_SYSTEM );
	while( entry && entry ->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}  // end while
	SendMessage( combo, CB_INSERTSTRING, 0, (LPARAM)GUIEDIT_NONE_STRING );

	// select the current function of the window in the combo if present
	name.clear();
	if( editData )
		name = editData->systemCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendMessage( combo, CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// load the input combo -----------------------------------------------------
	combo = GetDlgItem( dialog, COMBO_INPUT );
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_GAME_WIN_INPUT );
	while( entry && entry ->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}  // end while
	SendMessage( combo, CB_INSERTSTRING, 0, (LPARAM)GUIEDIT_NONE_STRING );

	// select the current function of the window in the combo if present
	name.clear();
	if( editData )
		name = editData->inputCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendMessage( combo, CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// load the tooltip combo ---------------------------------------------------
	combo = GetDlgItem( dialog, COMBO_TOOLTIP );
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_GAME_WIN_TOOLTIP );
	while( entry && entry ->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}  // end while
	SendMessage( combo, CB_INSERTSTRING, 0, (LPARAM)GUIEDIT_NONE_STRING );

	// select the current function of the window in the combo if present
	name.clear();
	if( editData )
		name = editData->tooltipCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendMessage( combo, CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// load the draw combo ------------------------------------------------------
	combo = GetDlgItem( dialog, COMBO_DRAW );
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_GAME_WIN_DRAW );
	while( entry && entry ->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}  // end while
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_GAME_WIN_DEVICEDRAW );
	while( entry && entry ->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}  // end while
	SendMessage( combo, CB_INSERTSTRING, 0, (LPARAM)GUIEDIT_NONE_STRING );

	// select the current function of the window in the combo if present
	name.clear();
	if( editData )
		name = editData->drawCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendMessage( combo, CB_SELECTSTRING, -1, (LPARAM)name.str() );

	//=================================================================================================
	// callbacks for the layout itself
	//=================================================================================================

	// init combo
	combo = GetDlgItem( dialog, COMBO_INIT );
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_WIN_LAYOUT_INIT );
	while( entry && entry->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_WIN_LAYOUT_DEVICEINIT );
	while( entry && entry->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}
	SendMessage( combo, CB_INSERTSTRING, 0, (LPARAM)GUIEDIT_NONE_STRING );
	name = TheEditor->getLayoutInit();
	SendMessage( combo, CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// update combo
	combo = GetDlgItem( dialog, COMBO_UPDATE );
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_WIN_LAYOUT_UPDATE );
	while( entry && entry->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}
	SendMessage( combo, CB_INSERTSTRING, 0, (LPARAM)GUIEDIT_NONE_STRING );
	name = TheEditor->getLayoutUpdate();
	SendMessage( combo, CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// shutdown combo
	combo = GetDlgItem( dialog, COMBO_SHUTDOWN );
	entry = TheFunctionLexicon->getTable( FunctionLexicon::TABLE_WIN_LAYOUT_SHUTDOWN );
	while( entry && entry->key != NAMEKEY_INVALID )
	{
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)entry->name );
		entry++;
	}
	SendMessage( combo, CB_INSERTSTRING, 0, (LPARAM)GUIEDIT_NONE_STRING );
	name = TheEditor->getLayoutShutdown();
	SendMessage( combo, CB_SELECTSTRING, -1, (LPARAM)name.str() );

}  // end InitCallbackCombos

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitUserWinPropertiesDialog ================================================
/** Initialize the generic properties dialog for windows */
//=============================================================================
HWND InitUserWinPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)GENERIC_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)genericPropertiesCallback );
	if( dialog == NULL )
		return NULL;

	// do the common initialization
	CommonDialogInitialize( window, dialog );

	//
	// initialize the dialog with values from the window
	//

	// fill out the image combo boxes
	LoadImageListComboBox( GetDlgItem( dialog, COMBO_ENABLED_IMAGE ) );
	LoadImageListComboBox( GetDlgItem( dialog, COMBO_DISABLED_IMAGE ) );
	LoadImageListComboBox( GetDlgItem( dialog, COMBO_HILITE_IMAGE ) );

	// select any images in the combo boxes
	const Image *image;

	image = window->winGetEnabledImage( 0 );
	if( image )
		SendDlgItemMessage( dialog, COMBO_ENABLED_IMAGE, CB_SELECTSTRING,
												-1, (LPARAM)image->getName().str() );
	image = window->winGetDisabledImage( 0 );
	if( image )
		SendDlgItemMessage( dialog, COMBO_DISABLED_IMAGE, CB_SELECTSTRING,
												-1, (LPARAM)image->getName().str() );
	image = window->winGetHiliteImage( 0 );
	if( image )
		SendDlgItemMessage( dialog, COMBO_HILITE_IMAGE, CB_SELECTSTRING,
												-1, (LPARAM)image->getName().str() );

	// initialize the color buttons
	SetControlColor( BUTTON_ENABLED_COLOR, window->winGetEnabledColor( 0 ) );
	SetControlColor( BUTTON_ENABLED_BORDER_COLOR, window->winGetEnabledBorderColor( 0 ) );
	SetControlColor( BUTTON_DISABLED_COLOR, window->winGetDisabledColor( 0 ) );
	SetControlColor( BUTTON_DISABLED_BORDER_COLOR, window->winGetDisabledBorderColor( 0 ) );
	SetControlColor( BUTTON_HILITE_COLOR, window->winGetHiliteColor( 0 ) );
	SetControlColor( BUTTON_HILITE_BORDER_COLOR, window->winGetHiliteBorderColor( 0 ) );

	// load the combo boxes with the callbacks the user can use
	InitCallbackCombos( dialog, window );

	return dialog;

}  // end InitUserWinPropertiesDialog



