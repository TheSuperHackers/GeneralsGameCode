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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: TextEntryProperties.cpp //////////////////////////////////////////////
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
// File name:  TextEntryProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Text entry properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "resource.h"
#include "GameClient/GadgetTextEntry.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// textEntryPropertiesCallback ================================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK textEntryPropertiesCallback( HWND hWndDialog,
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
    case WM_COMMAND:
    {
//			Int notifyCode = HIWORD( wParam );  // notification code
			Int controlID = LOWORD( wParam );  // control ID
//			HWND hWndControl = (HWND)lParam;  // control window handle

      switch( controlID )
      {

				// --------------------------------------------------------------------
        case IDOK:
				{
					GameWindow *window = TheEditor->getPropertyTarget();

					// sanity
					if( window )
					{
						ImageAndColorInfo *info;

						// save the common properties
						if( SaveCommonDialogProperties( hWndDialog, window ) == FALSE )
							break;

						// save the image and color data
						// ----------------------------------------------------------------
						info = GetStateInfo( TEXT_ENTRY_ENABLED_LEFT );
						GadgetTextEntrySetEnabledImageLeft( window, info->image );
						GadgetTextEntrySetEnabledColor( window, info->color );
						GadgetTextEntrySetEnabledBorderColor( window, info->borderColor );
						info = GetStateInfo( TEXT_ENTRY_ENABLED_RIGHT );
						GadgetTextEntrySetEnabledImageRight( window, info->image );
						info = GetStateInfo( TEXT_ENTRY_ENABLED_CENTER );
						GadgetTextEntrySetEnabledImageCenter( window, info->image );
						info = GetStateInfo( TEXT_ENTRY_ENABLED_SMALL_CENTER );
						GadgetTextEntrySetEnabledImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( TEXT_ENTRY_DISABLED_LEFT );
						GadgetTextEntrySetDisabledImageLeft( window, info->image );
						GadgetTextEntrySetDisabledColor( window, info->color );
						GadgetTextEntrySetDisabledBorderColor( window, info->borderColor );
						info = GetStateInfo( TEXT_ENTRY_DISABLED_RIGHT );
						GadgetTextEntrySetDisabledImageRight( window, info->image );
						info = GetStateInfo( TEXT_ENTRY_DISABLED_CENTER );
						GadgetTextEntrySetDisabledImageCenter( window, info->image );
						info = GetStateInfo( TEXT_ENTRY_DISABLED_SMALL_CENTER );
						GadgetTextEntrySetDisabledImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( TEXT_ENTRY_HILITE_LEFT );
						GadgetTextEntrySetHiliteImageLeft( window, info->image );
						GadgetTextEntrySetHiliteColor( window, info->color );
						GadgetTextEntrySetHiliteBorderColor( window, info->borderColor );
						info = GetStateInfo( TEXT_ENTRY_HILITE_RIGHT );
						GadgetTextEntrySetHiliteImageRight( window, info->image );
						info = GetStateInfo( TEXT_ENTRY_HILITE_CENTER );
						GadgetTextEntrySetHiliteImageCenter( window, info->image );
						info = GetStateInfo( TEXT_ENTRY_HILITE_SMALL_CENTER );
						GadgetTextEntrySetHiliteImageSmallCenter( window, info->image );

						// text entry props
						EntryData *entryData = (EntryData *)window->winGetUserData();

						entryData->maxTextLen = GetDlgItemInt( hWndDialog, EDIT_MAX_CHARS, NULL, TRUE );
						entryData->secretText = IsDlgButtonChecked( hWndDialog, CHECK_SECRET_TEXT );
						entryData->aSCIIOnly = IsDlgButtonChecked( hWndDialog, CHECK_ASCII_TEXT );
						if( IsDlgButtonChecked( hWndDialog, RADIO_LETTERS_AND_NUMBERS ) )
						{

							entryData->alphaNumericalOnly = TRUE;
							entryData->numericalOnly = FALSE;

						}  // end if
						else if( IsDlgButtonChecked( hWndDialog, RADIO_NUMBERS ) )
						{

							entryData->alphaNumericalOnly = FALSE;
							entryData->numericalOnly = TRUE;

						}  // end else if
						else
						{

							entryData->alphaNumericalOnly = FALSE;
							entryData->numericalOnly = FALSE;

						}  // end else

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

}  // end textEntryPropertiesCallback


///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitTextEntryPropertiesDialog ==============================================
/** Bring up the text entry properties dialog */
//=============================================================================
HWND InitTextEntryPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)TEXT_ENTRY_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)textEntryPropertiesCallback );
	if( dialog == NULL )
		return NULL;

	// do the common initialization
	CommonDialogInitialize( window, dialog );

	//
	// store in the image and color table the values for this putton
	//
	const Image *image;
	Color color, borderColor;

	// --------------------------------------------------------------------------
	image = GadgetTextEntryGetEnabledImageLeft( window );
	color = GadgetTextEntryGetEnabledColor( window );
	borderColor = GadgetTextEntryGetEnabledBorderColor( window );
	StoreImageAndColor( TEXT_ENTRY_ENABLED_LEFT, image, color, borderColor );
	image = GadgetTextEntryGetEnabledImageRight( window );
	StoreImageAndColor( TEXT_ENTRY_ENABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetTextEntryGetEnabledImageCenter( window );
	StoreImageAndColor( TEXT_ENTRY_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetTextEntryGetEnabledImageSmallCenter( window );
	StoreImageAndColor( TEXT_ENTRY_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetTextEntryGetDisabledImageLeft( window );
	color = GadgetTextEntryGetDisabledColor( window );
	borderColor = GadgetTextEntryGetDisabledBorderColor( window );
	StoreImageAndColor( TEXT_ENTRY_DISABLED_LEFT, image, color, borderColor );
	image = GadgetTextEntryGetDisabledImageRight( window );
	StoreImageAndColor( TEXT_ENTRY_DISABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetTextEntryGetDisabledImageCenter( window );
	StoreImageAndColor( TEXT_ENTRY_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetTextEntryGetDisabledImageSmallCenter( window );
	StoreImageAndColor( TEXT_ENTRY_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetTextEntryGetHiliteImageLeft( window );
	color = GadgetTextEntryGetHiliteColor( window );
	borderColor = GadgetTextEntryGetHiliteBorderColor( window );
	StoreImageAndColor( TEXT_ENTRY_HILITE_LEFT, image, color, borderColor );
	image = GadgetTextEntryGetHiliteImageRight( window );
	StoreImageAndColor( TEXT_ENTRY_HILITE_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetTextEntryGetHiliteImageCenter( window );
	StoreImageAndColor( TEXT_ENTRY_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetTextEntryGetHiliteImageSmallCenter( window );
	StoreImageAndColor( TEXT_ENTRY_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// fill out text entry properties
	EntryData *entryData = (EntryData *)window->winGetUserData();

	SetDlgItemInt( dialog, EDIT_MAX_CHARS, entryData->maxTextLen, TRUE );
	if( entryData->secretText )
		CheckDlgButton( dialog, CHECK_SECRET_TEXT, BST_CHECKED );
	if( entryData->aSCIIOnly )
		CheckDlgButton( dialog, CHECK_ASCII_TEXT, BST_CHECKED );
	if( entryData->numericalOnly )
		CheckDlgButton( dialog, RADIO_NUMBERS, BST_CHECKED );
	else if( entryData->alphaNumericalOnly )
		CheckDlgButton( dialog, RADIO_LETTERS_AND_NUMBERS, BST_CHECKED );
	else
		CheckDlgButton( dialog, RADIO_ANY_TEXT, BST_CHECKED );

	// select the button enabled state for display
	SwitchToState( TEXT_ENTRY_ENABLED_LEFT, dialog );

	//
	// initialize the dialog with values from the window
	//

	return dialog;

}  // end InitTextEntryPropertiesDialog



