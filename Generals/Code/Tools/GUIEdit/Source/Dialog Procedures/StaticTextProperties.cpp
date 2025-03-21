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

// FILE: StaticTextProperties.cpp /////////////////////////////////////////////
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
// File name:  StaticTextSliderProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Static text properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "resource.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/Gadget.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static Bool currCentered = FALSE;

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// staticTextPropertiesCallback ===============================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK staticTextPropertiesCallback( HWND hWndDialog,
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
				case BUTTON_CENTERED:
				{

					currCentered = 1 - currCentered;
					if( currCentered == TRUE )
						SetDlgItemText( hWndDialog, BUTTON_CENTERED, "Yes" );
					else
						SetDlgItemText( hWndDialog, BUTTON_CENTERED, "No" );
					break;

				}  // end centered

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
						info = GetStateInfo( STATIC_TEXT_ENABLED );
						GadgetStaticTextSetEnabledImage( window, info->image );
						GadgetStaticTextSetEnabledColor( window, info->color );
						GadgetStaticTextSetEnabledBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( STATIC_TEXT_DISABLED );
						GadgetStaticTextSetDisabledImage( window, info->image );
						GadgetStaticTextSetDisabledColor( window, info->color );
						GadgetStaticTextSetDisabledBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( STATIC_TEXT_HILITE );
						GadgetStaticTextSetHiliteImage( window, info->image );
						GadgetStaticTextSetHiliteColor( window, info->color );
						GadgetStaticTextSetHiliteBorderColor( window, info->borderColor );

						// text data
						TextData *textData = (TextData *)window->winGetUserData();
						textData->centered = currCentered;
						
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

}  // end staticTextPropertiesCallback


///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitStaticTextPropertiesDialog =============================================
/** Bring up the static text properties dialog */
//=============================================================================
HWND InitStaticTextPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)STATIC_TEXT_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)staticTextPropertiesCallback );
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
	image = GadgetStaticTextGetEnabledImage( window );
	color = GadgetStaticTextGetEnabledColor( window );
	borderColor = GadgetStaticTextGetEnabledBorderColor( window );
	StoreImageAndColor( STATIC_TEXT_ENABLED, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetStaticTextGetDisabledImage( window );
	color = GadgetStaticTextGetDisabledColor( window );
	borderColor = GadgetStaticTextGetDisabledBorderColor( window );
	StoreImageAndColor( STATIC_TEXT_DISABLED, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetStaticTextGetHiliteImage( window );
	color = GadgetStaticTextGetHiliteColor( window );
	borderColor = GadgetStaticTextGetHiliteBorderColor( window );
	StoreImageAndColor( STATIC_TEXT_HILITE, image, color, borderColor );

	// text data
	TextData *textData = (TextData *)window->winGetUserData();

	if( textData->centered )
		SetDlgItemText( dialog, BUTTON_CENTERED, "Yes" );
	else
		SetDlgItemText( dialog, BUTTON_CENTERED, "No" );
	currCentered = textData->centered;

	// select the button enabled state for display
	SwitchToState( STATIC_TEXT_ENABLED, dialog );
	
	return dialog;

}  // end InitStaticTextPropertiesDialog



