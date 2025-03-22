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

// FILE: RadioButtonProperties.cpp ////////////////////////////////////////////
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
// File name:  RadioButtonProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Radio button properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameWindowManager.h"
#include "GUIEdit.h"
#include "Properties.h"
#include "Resource.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/Gadget.h"

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

// radioButtonPropertiesCallback ==============================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK radioButtonPropertiesCallback( HWND hWndDialog,
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
				case BUTTON_CLEAR_GROUP:
				{

					SetDlgItemInt( hWndDialog, COMBO_GROUP, 0, FALSE );
					break;

				}  // end clear group

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
						info = GetStateInfo( RADIO_ENABLED );
						GadgetRadioSetEnabledImage( window, info->image );
						GadgetRadioSetEnabledColor( window, info->color );
						GadgetRadioSetEnabledBorderColor( window, info->borderColor );

						info = GetStateInfo( RADIO_ENABLED_UNCHECKED_BOX );
						GadgetRadioSetEnabledUncheckedBoxImage( window, info->image );
						GadgetRadioSetEnabledUncheckedBoxColor( window, info->color );
						GadgetRadioSetEnabledUncheckedBoxBorderColor( window, info->borderColor );

						info = GetStateInfo( RADIO_ENABLED_CHECKED_BOX );
						GadgetRadioSetEnabledCheckedBoxImage( window, info->image );
						GadgetRadioSetEnabledCheckedBoxColor( window, info->color );
						GadgetRadioSetEnabledCheckedBoxBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( RADIO_DISABLED );
						GadgetRadioSetDisabledImage( window, info->image );
						GadgetRadioSetDisabledColor( window, info->color );
						GadgetRadioSetDisabledBorderColor( window, info->borderColor );

						info = GetStateInfo( RADIO_DISABLED_UNCHECKED_BOX );
						GadgetRadioSetDisabledUncheckedBoxImage( window, info->image );
						GadgetRadioSetDisabledUncheckedBoxColor( window, info->color );
						GadgetRadioSetDisabledUncheckedBoxBorderColor( window, info->borderColor );

						info = GetStateInfo( RADIO_DISABLED_CHECKED_BOX );
						GadgetRadioSetDisabledCheckedBoxImage( window, info->image );
						GadgetRadioSetDisabledCheckedBoxColor( window, info->color );
						GadgetRadioSetDisabledCheckedBoxBorderColor( window, info->borderColor );
						
						// ----------------------------------------------------------------
						info = GetStateInfo( RADIO_HILITE );
						GadgetRadioSetHiliteImage( window, info->image );
						GadgetRadioSetHiliteColor( window, info->color );
						GadgetRadioSetHiliteBorderColor( window, info->borderColor );

						info = GetStateInfo( RADIO_HILITE_UNCHECKED_BOX );
						GadgetRadioSetHiliteUncheckedBoxImage( window, info->image );
						GadgetRadioSetHiliteUncheckedBoxColor( window, info->color );
						GadgetRadioSetHiliteUncheckedBoxBorderColor( window, info->borderColor );

						info = GetStateInfo( RADIO_HILITE_CHECKED_BOX );
						GadgetRadioSetHiliteCheckedBoxImage( window, info->image );
						GadgetRadioSetHiliteCheckedBoxColor( window, info->color );
						GadgetRadioSetHiliteCheckedBoxBorderColor( window, info->borderColor );

						// save group
						Int group = GetDlgItemInt( hWndDialog, COMBO_GROUP, NULL, FALSE );
						Int screen = TheNameKeyGenerator->nameToKey( AsciiString(TheEditor->getSaveFilename()) );
						GadgetRadioSetGroup( window, group, screen );
																	
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

}  // end radioButtonPropertiesCallback


///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// loadExistingGroupsCombo ====================================================
/** fill the group combo box with all the other groups in the screen */
//=============================================================================
static void loadExistingGroupsCombo( HWND combo, GameWindow *window )
{

	// sanity
	if( combo == NULL )
		return;

	// end of recursion
	if( window == NULL )
		return;

	// if this is a radio button get the group
	if( BitIsSet( window->winGetStyle(), GWS_RADIO_BUTTON ) )
	{
		RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();
		char buffer[ 64 ];

		// convert to string
		sprintf( buffer, "%d", radioData->group );

		// only add it not already there
		if( SendMessage( combo, CB_FINDSTRINGEXACT, -1, (LPARAM)buffer ) == CB_ERR )
			SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)buffer );

	}  // end if

	// search our children
	GameWindow *child;
	for( child = window->winGetChild(); child; child = child->winGetNext() )
		loadExistingGroupsCombo( combo, child );

	// search the next in line
	loadExistingGroupsCombo( combo, window->winGetNext() );
	
}  // end loadExistingGroupsCombo

// InitRadioButtonPropertiesDialog ============================================
/** Bring up the radio button properties dialog */
//=============================================================================
HWND InitRadioButtonPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)RADIO_BUTTON_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)radioButtonPropertiesCallback );
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
	image = GadgetRadioGetEnabledImage( window );
	color = GadgetRadioGetEnabledColor( window );
	borderColor = GadgetRadioGetEnabledBorderColor( window );
	StoreImageAndColor( RADIO_ENABLED, image, color, borderColor );

	image = GadgetRadioGetEnabledUncheckedBoxImage( window );
	color = GadgetRadioGetEnabledUncheckedBoxColor( window );
	borderColor = GadgetRadioGetEnabledUncheckedBoxBorderColor( window );
	StoreImageAndColor( RADIO_ENABLED_UNCHECKED_BOX, image, color, borderColor );

	image = GadgetRadioGetEnabledCheckedBoxImage( window );
	color = GadgetRadioGetEnabledCheckedBoxColor( window );
	borderColor = GadgetRadioGetEnabledCheckedBoxBorderColor( window );
	StoreImageAndColor( RADIO_ENABLED_CHECKED_BOX, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetRadioGetDisabledImage( window );
	color = GadgetRadioGetDisabledColor( window );
	borderColor = GadgetRadioGetDisabledBorderColor( window );
	StoreImageAndColor( RADIO_DISABLED, image, color, borderColor );

	image = GadgetRadioGetDisabledUncheckedBoxImage( window );
	color = GadgetRadioGetDisabledUncheckedBoxColor( window );
	borderColor = GadgetRadioGetDisabledUncheckedBoxBorderColor( window );
	StoreImageAndColor( RADIO_DISABLED_UNCHECKED_BOX, image, color, borderColor );

	image = GadgetRadioGetDisabledCheckedBoxImage( window );
	color = GadgetRadioGetDisabledCheckedBoxColor( window );
	borderColor = GadgetRadioGetDisabledCheckedBoxBorderColor( window );
	StoreImageAndColor( RADIO_DISABLED_CHECKED_BOX, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetRadioGetHiliteImage( window );
	color = GadgetRadioGetHiliteColor( window );
	borderColor = GadgetRadioGetHiliteBorderColor( window );
	StoreImageAndColor( RADIO_HILITE, image, color, borderColor );

	image = GadgetRadioGetHiliteUncheckedBoxImage( window );
	color = GadgetRadioGetHiliteUncheckedBoxColor( window );
	borderColor = GadgetRadioGetHiliteUncheckedBoxBorderColor( window );
	StoreImageAndColor( RADIO_HILITE_UNCHECKED_BOX, image, color, borderColor );

	image = GadgetRadioGetHiliteCheckedBoxImage( window );
	color = GadgetRadioGetHiliteCheckedBoxColor( window );
	borderColor = GadgetRadioGetHiliteCheckedBoxBorderColor( window );
	StoreImageAndColor( RADIO_HILITE_CHECKED_BOX, image, color, borderColor );

	// radio data
	RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();

	// fill the group combo box with all the other groups in the screen
	loadExistingGroupsCombo( GetDlgItem( dialog, COMBO_GROUP ),
													 TheWindowManager->winGetWindowList() );

	// set the group for this radio button
	SetDlgItemInt( dialog, COMBO_GROUP, radioData->group, FALSE );

	// select the button enabled state for display
	SwitchToState( RADIO_ENABLED, dialog );

	//
	// initialize the dialog with values from the window
	//

	return dialog;

}  // end InitRadioButtonPropertiesDialog



