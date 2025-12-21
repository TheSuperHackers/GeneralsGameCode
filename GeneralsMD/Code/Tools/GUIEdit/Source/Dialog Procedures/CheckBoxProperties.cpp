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

// FILE: CheckBoxProperties.cpp ///////////////////////////////////////////////
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
// File name:  CheckBoxProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Check box properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "resource.h"
#include "GameClient/GadgetCheckBox.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// checkBoxPropertiesCallback =================================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK checkBoxPropertiesCallback( HWND hWndDialog,
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
						info = GetStateInfo( CHECK_BOX_ENABLED );
						GadgetCheckBoxSetEnabledImage( window, info->image );
						GadgetCheckBoxSetEnabledColor( window, info->color );
						GadgetCheckBoxSetEnabledBorderColor( window, info->borderColor );

						info = GetStateInfo( CHECK_BOX_ENABLED_UNCHECKED_BOX );
						GadgetCheckBoxSetEnabledUncheckedBoxImage( window, info->image );
						GadgetCheckBoxSetEnabledUncheckedBoxColor( window, info->color );
						GadgetCheckBoxSetEnabledUncheckedBoxBorderColor( window, info->borderColor );

						info = GetStateInfo( CHECK_BOX_ENABLED_CHECKED_BOX );
						GadgetCheckBoxSetEnabledCheckedBoxImage( window, info->image );
						GadgetCheckBoxSetEnabledCheckedBoxColor( window, info->color );
						GadgetCheckBoxSetEnabledCheckedBoxBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( CHECK_BOX_DISABLED );
						GadgetCheckBoxSetDisabledImage( window, info->image );
						GadgetCheckBoxSetDisabledColor( window, info->color );
						GadgetCheckBoxSetDisabledBorderColor( window, info->borderColor );

						info = GetStateInfo( CHECK_BOX_DISABLED_UNCHECKED_BOX );
						GadgetCheckBoxSetDisabledUncheckedBoxImage( window, info->image );
						GadgetCheckBoxSetDisabledUncheckedBoxColor( window, info->color );
						GadgetCheckBoxSetDisabledUncheckedBoxBorderColor( window, info->borderColor );

						info = GetStateInfo( CHECK_BOX_DISABLED_CHECKED_BOX );
						GadgetCheckBoxSetDisabledCheckedBoxImage( window, info->image );
						GadgetCheckBoxSetDisabledCheckedBoxColor( window, info->color );
						GadgetCheckBoxSetDisabledCheckedBoxBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( CHECK_BOX_HILITE );
						GadgetCheckBoxSetHiliteImage( window, info->image );
						GadgetCheckBoxSetHiliteColor( window, info->color );
						GadgetCheckBoxSetHiliteBorderColor( window, info->borderColor );

						info = GetStateInfo( CHECK_BOX_HILITE_UNCHECKED_BOX );
						GadgetCheckBoxSetHiliteUncheckedBoxImage( window, info->image );
						GadgetCheckBoxSetHiliteUncheckedBoxColor( window, info->color );
						GadgetCheckBoxSetHiliteUncheckedBoxBorderColor( window, info->borderColor );

						info = GetStateInfo( CHECK_BOX_HILITE_CHECKED_BOX );
						GadgetCheckBoxSetHiliteCheckedBoxImage( window, info->image );
						GadgetCheckBoxSetHiliteCheckedBoxColor( window, info->color );
						GadgetCheckBoxSetHiliteCheckedBoxBorderColor( window, info->borderColor );

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

// InitCheckBoxPropertiesDialog ===============================================
/** Bring up the check box properties dialog */
//=============================================================================
HWND InitCheckBoxPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)CHECK_BOX_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)checkBoxPropertiesCallback );
	if( dialog == nullptr )
		return nullptr;

	// do the common initialization
	CommonDialogInitialize( window, dialog );

	//
	// store in the image and color table the values for this putton
	//
	const Image *image;
	Color color, borderColor;

	// --------------------------------------------------------------------------
	image = GadgetCheckBoxGetEnabledImage( window );
	color = GadgetCheckBoxGetEnabledColor( window );
	borderColor = GadgetCheckBoxGetEnabledBorderColor( window );
	StoreImageAndColor( CHECK_BOX_ENABLED, image, color, borderColor );

	image = GadgetCheckBoxGetEnabledUncheckedBoxImage( window );
	color = GadgetCheckBoxGetEnabledUncheckedBoxColor( window );
	borderColor = GadgetCheckBoxGetEnabledUncheckedBoxBorderColor( window );
	StoreImageAndColor( CHECK_BOX_ENABLED_UNCHECKED_BOX, image, color, borderColor );

	image = GadgetCheckBoxGetEnabledCheckedBoxImage( window );
	color = GadgetCheckBoxGetEnabledCheckedBoxColor( window );
	borderColor = GadgetCheckBoxGetEnabledCheckedBoxBorderColor( window );
	StoreImageAndColor( CHECK_BOX_ENABLED_CHECKED_BOX, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetCheckBoxGetDisabledImage( window );
	color = GadgetCheckBoxGetDisabledColor( window );
	borderColor = GadgetCheckBoxGetDisabledBorderColor( window );
	StoreImageAndColor( CHECK_BOX_DISABLED, image, color, borderColor );

	image = GadgetCheckBoxGetDisabledUncheckedBoxImage( window );
	color = GadgetCheckBoxGetDisabledUncheckedBoxColor( window );
	borderColor = GadgetCheckBoxGetDisabledUncheckedBoxBorderColor( window );
	StoreImageAndColor( CHECK_BOX_DISABLED_UNCHECKED_BOX, image, color, borderColor );

	image = GadgetCheckBoxGetDisabledCheckedBoxImage( window );
	color = GadgetCheckBoxGetDisabledCheckedBoxColor( window );
	borderColor = GadgetCheckBoxGetDisabledCheckedBoxBorderColor( window );
	StoreImageAndColor( CHECK_BOX_DISABLED_CHECKED_BOX, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetCheckBoxGetHiliteImage( window );
	color = GadgetCheckBoxGetHiliteColor( window );
	borderColor = GadgetCheckBoxGetHiliteBorderColor( window );
	StoreImageAndColor( CHECK_BOX_HILITE, image, color, borderColor );

	image = GadgetCheckBoxGetHiliteUncheckedBoxImage( window );
	color = GadgetCheckBoxGetHiliteUncheckedBoxColor( window );
	borderColor = GadgetCheckBoxGetHiliteUncheckedBoxBorderColor( window );
	StoreImageAndColor( CHECK_BOX_HILITE_UNCHECKED_BOX, image, color, borderColor );

	image = GadgetCheckBoxGetHiliteCheckedBoxImage( window );
	color = GadgetCheckBoxGetHiliteCheckedBoxColor( window );
	borderColor = GadgetCheckBoxGetHiliteCheckedBoxBorderColor( window );
	StoreImageAndColor( CHECK_BOX_HILITE_CHECKED_BOX, image, color, borderColor );

	// select the button enabled state for display
	SwitchToState( CHECK_BOX_ENABLED, dialog );

	//
	// initialize the dialog with values from the window
	//

	return dialog;

}



