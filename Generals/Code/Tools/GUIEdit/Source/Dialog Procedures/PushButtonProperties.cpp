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

// FILE: PushButtonProperties.cpp /////////////////////////////////////////////
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
// File name:  PushButtonProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Push button properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "resource.h"
#include "GameClient/GadgetPushButton.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// pushButtonPropertiesCallback ===============================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK pushButtonPropertiesCallback( HWND hWndDialog,
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
						info = GetStateInfo( BUTTON_ENABLED );
						GadgetButtonSetEnabledImage( window, info->image );
						GadgetButtonSetEnabledColor( window, info->color );
						GadgetButtonSetEnabledBorderColor( window, info->borderColor );

						info = GetStateInfo( BUTTON_ENABLED_PUSHED );
						GadgetButtonSetEnabledSelectedImage( window, info->image );
						GadgetButtonSetEnabledSelectedColor( window, info->color );
						GadgetButtonSetEnabledSelectedBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( BUTTON_DISABLED );
						GadgetButtonSetDisabledImage( window, info->image );
						GadgetButtonSetDisabledColor( window, info->color );
						GadgetButtonSetDisabledBorderColor( window, info->borderColor );

						info = GetStateInfo( BUTTON_DISABLED_PUSHED );
						GadgetButtonSetDisabledSelectedImage( window, info->image );
						GadgetButtonSetDisabledSelectedColor( window, info->color );
						GadgetButtonSetDisabledSelectedBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( BUTTON_HILITE );
						GadgetButtonSetHiliteImage( window, info->image );
						GadgetButtonSetHiliteColor( window, info->color );
						GadgetButtonSetHiliteBorderColor( window, info->borderColor );

						info = GetStateInfo( BUTTON_HILITE_PUSHED );
						GadgetButtonSetHiliteSelectedImage( window, info->image );
						GadgetButtonSetHiliteSelectedColor( window, info->color );
						GadgetButtonSetHiliteSelectedBorderColor( window, info->borderColor );

						UnsignedInt bit;
						bit = WIN_STATUS_RIGHT_CLICK;
						window->winClearStatus( bit );
						if( IsDlgButtonChecked( hWndDialog, CHECK_RIGHT_CLICK ) )
							window->winSetStatus( bit );

					}

          DestroyWindow( hWndDialog );
          break;

				}

				// --------------------------------------------------------------------
        case IDCANCEL:
				{

          DestroyWindow( hWndDialog );
          break;

				}

      }

      return 0;

    }

		// ------------------------------------------------------------------------
    case WM_CLOSE:
		{

      DestroyWindow( hWndDialog );
      return 0;

		}

		// ------------------------------------------------------------------------
		default:
			return 0;

  }

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitPushButtonPropertiesDialog =============================================
/** Bring up the push button properties dialog */
//=============================================================================
HWND InitPushButtonPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)PUSH_BUTTON_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)pushButtonPropertiesCallback );
	if( dialog == nullptr )
		return nullptr;

	// do the common initialization
	CommonDialogInitialize( window, dialog );

	//
	// store in the image and color table the values for this putton
	//
	const Image *image;
	Color color, borderColor;

	image = GadgetButtonGetEnabledImage( window );
	color = GadgetButtonGetEnabledColor( window );
	borderColor = GadgetButtonGetEnabledBorderColor( window );
	StoreImageAndColor( BUTTON_ENABLED, image, color, borderColor );

	image = GadgetButtonGetEnabledSelectedImage( window );
	color = GadgetButtonGetEnabledSelectedColor( window );
	borderColor = GadgetButtonGetEnabledSelectedBorderColor( window );
	StoreImageAndColor( BUTTON_ENABLED_PUSHED, image, color, borderColor );

	image = GadgetButtonGetDisabledImage( window );
	color = GadgetButtonGetDisabledColor( window );
	borderColor = GadgetButtonGetDisabledBorderColor( window );
	StoreImageAndColor( BUTTON_DISABLED, image, color, borderColor );

	image = GadgetButtonGetDisabledSelectedImage( window );
	color = GadgetButtonGetDisabledSelectedColor( window );
	borderColor = GadgetButtonGetDisabledSelectedBorderColor( window );
	StoreImageAndColor( BUTTON_DISABLED_PUSHED, image, color, borderColor );

	image = GadgetButtonGetHiliteImage( window );
	color = GadgetButtonGetHiliteColor( window );
	borderColor = GadgetButtonGetHiliteBorderColor( window );
	StoreImageAndColor( BUTTON_HILITE, image, color, borderColor );

	image = GadgetButtonGetHiliteSelectedImage( window );
	color = GadgetButtonGetHiliteSelectedColor( window );
	borderColor = GadgetButtonGetHiliteSelectedBorderColor( window );
	StoreImageAndColor( BUTTON_HILITE_PUSHED, image, color, borderColor );

	// select the button enabled state for display
	SwitchToState( BUTTON_ENABLED, dialog );

	//
	// initialize the dialog with values from the window
	//

	if( BitIsSet( window->winGetStatus(), WIN_STATUS_RIGHT_CLICK ) )
		CheckDlgButton( dialog, CHECK_RIGHT_CLICK, BST_CHECKED );

	return dialog;

}



