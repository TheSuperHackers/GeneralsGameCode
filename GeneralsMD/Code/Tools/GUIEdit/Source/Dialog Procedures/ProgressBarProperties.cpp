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

// FILE: ProgressBarProperties.cpp ////////////////////////////////////////////
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
// File name:  ProgressBarProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Progress bar properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "resource.h"
#include "GameClient/GadgetProgressBar.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// progressBarPropertiesCallback ==============================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK progressBarPropertiesCallback( HWND hWndDialog,
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
						info = GetStateInfo( PROGRESS_BAR_ENABLED_LEFT );
						GadgetProgressBarSetEnabledImageLeft( window, info->image );
						GadgetProgressBarSetEnabledColor( window, info->color );
						GadgetProgressBarSetEnabledBorderColor( window, info->borderColor );
						info = GetStateInfo( PROGRESS_BAR_ENABLED_RIGHT );
						GadgetProgressBarSetEnabledImageRight( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_ENABLED_CENTER );
						GadgetProgressBarSetEnabledImageCenter( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_ENABLED_SMALL_CENTER );
						GadgetProgressBarSetEnabledImageSmallCenter( window, info->image );

						info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_LEFT );
						GadgetProgressBarSetEnabledBarImageLeft( window, info->image );
						GadgetProgressBarSetEnabledBarColor( window, info->color );
						GadgetProgressBarSetEnabledBarBorderColor( window, info->borderColor );
						info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_RIGHT );
						GadgetProgressBarSetEnabledBarImageRight( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_CENTER );
						GadgetProgressBarSetEnabledBarImageCenter( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_SMALL_CENTER );
						GadgetProgressBarSetEnabledBarImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( PROGRESS_BAR_DISABLED_LEFT );
						GadgetProgressBarSetDisabledImageLeft( window, info->image );
						GadgetProgressBarSetDisabledColor( window, info->color );
						GadgetProgressBarSetDisabledBorderColor( window, info->borderColor );
						info = GetStateInfo( PROGRESS_BAR_DISABLED_RIGHT );
						GadgetProgressBarSetDisabledImageRight( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_DISABLED_CENTER );
						GadgetProgressBarSetDisabledImageCenter( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_DISABLED_SMALL_CENTER );
						GadgetProgressBarSetDisabledImageSmallCenter( window, info->image );

						info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_LEFT );
						GadgetProgressBarSetDisabledBarImageLeft( window, info->image );
						GadgetProgressBarSetDisabledBarColor( window, info->color );
						GadgetProgressBarSetDisabledBarBorderColor( window, info->borderColor );
						info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_RIGHT );
						GadgetProgressBarSetDisabledBarImageRight( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_CENTER );
						GadgetProgressBarSetDisabledBarImageCenter( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_SMALL_CENTER );
						GadgetProgressBarSetDisabledBarImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( PROGRESS_BAR_HILITE_LEFT );
						GadgetProgressBarSetHiliteImageLeft( window, info->image );
						GadgetProgressBarSetHiliteColor( window, info->color );
						GadgetProgressBarSetHiliteBorderColor( window, info->borderColor );
						info = GetStateInfo( PROGRESS_BAR_HILITE_RIGHT );
						GadgetProgressBarSetHiliteImageRight( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_HILITE_CENTER );
						GadgetProgressBarSetHiliteImageCenter( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_HILITE_SMALL_CENTER );
						GadgetProgressBarSetHiliteImageSmallCenter( window, info->image );

						info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_LEFT );
						GadgetProgressBarSetHiliteBarImageLeft( window, info->image );
						GadgetProgressBarSetHiliteBarColor( window, info->color );
						GadgetProgressBarSetHiliteBarBorderColor( window, info->borderColor );
						info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_RIGHT );
						GadgetProgressBarSetHiliteBarImageRight( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_CENTER );
						GadgetProgressBarSetHiliteBarImageCenter( window, info->image );
						info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_SMALL_CENTER );
						GadgetProgressBarSetHiliteBarImageSmallCenter( window, info->image );

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

}  // end progressBarPropertiesCallback


///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitProgressBarPropertiesDialog ============================================
/** Bring up the progress bar properties dialog */
//=============================================================================
HWND InitProgressBarPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)PROGRESS_BAR_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)progressBarPropertiesCallback );
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
	image = GadgetProgressBarGetEnabledImageLeft( window );
	color = GadgetProgressBarGetEnabledColor( window );
	borderColor = GadgetProgressBarGetEnabledBorderColor( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_LEFT, image, color, borderColor );
	image = GadgetProgressBarGetEnabledImageRight( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetEnabledImageCenter( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetEnabledImageSmallCenter( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = GadgetProgressBarGetEnabledBarImageLeft( window );
	color = GadgetProgressBarGetEnabledBarColor( window );
	borderColor = GadgetProgressBarGetEnabledBarBorderColor( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_BAR_LEFT, image, color, borderColor );
	image = GadgetProgressBarGetEnabledBarImageRight( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_BAR_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetEnabledBarImageCenter( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_BAR_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetEnabledBarImageSmallCenter( window );
	StoreImageAndColor( PROGRESS_BAR_ENABLED_BAR_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetProgressBarGetDisabledImageLeft( window );
	color = GadgetProgressBarGetDisabledColor( window );
	borderColor = GadgetProgressBarGetDisabledBorderColor( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_LEFT, image, color, borderColor );
	image = GadgetProgressBarGetDisabledImageRight( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetDisabledImageCenter( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetDisabledImageSmallCenter( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = GadgetProgressBarGetDisabledBarImageLeft( window );
	color = GadgetProgressBarGetDisabledBarColor( window );
	borderColor = GadgetProgressBarGetDisabledBarBorderColor( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_BAR_LEFT, image, color, borderColor );
	image = GadgetProgressBarGetDisabledBarImageRight( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_BAR_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetDisabledBarImageCenter( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_BAR_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetDisabledBarImageSmallCenter( window );
	StoreImageAndColor( PROGRESS_BAR_DISABLED_BAR_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetProgressBarGetHiliteImageLeft( window );
	color = GadgetProgressBarGetHiliteColor( window );
	borderColor = GadgetProgressBarGetHiliteBorderColor( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_LEFT, image, color, borderColor );
	image = GadgetProgressBarGetHiliteImageRight( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetHiliteImageCenter( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetHiliteImageSmallCenter( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = GadgetProgressBarGetHiliteBarImageLeft( window );
	color = GadgetProgressBarGetHiliteBarColor( window );
	borderColor = GadgetProgressBarGetHiliteBarBorderColor( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_BAR_LEFT, image, color, borderColor );
	image = GadgetProgressBarGetHiliteBarImageRight( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_BAR_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetHiliteBarImageCenter( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_BAR_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetProgressBarGetHiliteBarImageSmallCenter( window );
	StoreImageAndColor( PROGRESS_BAR_HILITE_BAR_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// select the button enabled state for display
	SwitchToState( PROGRESS_BAR_ENABLED_LEFT, dialog );

	//
	// initialize the dialog with values from the window
	//

	return dialog;

}  // end InitProgressBarPropertiesDialog



