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

// FILE: SliderProperties.cpp /////////////////////////////////////////////////
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
// File name:  SliderProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Slider properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "Resource.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/Gadget.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// sliderPropertiesCallback ===================================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK sliderPropertiesCallback( HWND hWndDialog,
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
				case BUTTON_SUBCONTROL_COLOR:
				{
					ImageAndColorInfo *info;
					GameWindow *window = TheEditor->getPropertyTarget();
					Bool vert = FALSE;

					if( window )
						vert = BitIsSet( window->winGetStyle(), GWS_VERT_SLIDER );

					//
					// using the current colors in the base of the slider, assign a
					// reasonable color scheme to all the sub control components
					//
					if( vert )
					{

						info = GetStateInfo( VSLIDER_ENABLED_TOP );
						StoreColor( VSLIDER_THUMB_ENABLED, info->color, info->borderColor );
						StoreColor( VSLIDER_THUMB_ENABLED_PUSHED, info->borderColor, info->color );

						info = GetStateInfo( VSLIDER_DISABLED_TOP );
						StoreColor( VSLIDER_THUMB_DISABLED, info->color, info->borderColor );
						StoreColor( VSLIDER_THUMB_DISABLED_PUSHED, info->borderColor, info->color );

						info = GetStateInfo( VSLIDER_HILITE_TOP );
						StoreColor( VSLIDER_THUMB_HILITE, info->color, info->borderColor );
						StoreColor( VSLIDER_THUMB_HILITE_PUSHED, info->borderColor, info->color );

					}  // end if, vertical slider
					else
					{

						info = GetStateInfo( HSLIDER_ENABLED_LEFT );
						StoreColor( HSLIDER_THUMB_ENABLED, info->color, info->borderColor );
						StoreColor( HSLIDER_THUMB_ENABLED_PUSHED, info->borderColor, info->color );

						info = GetStateInfo( HSLIDER_DISABLED_LEFT );
						StoreColor( HSLIDER_THUMB_DISABLED, info->color, info->borderColor );
						StoreColor( HSLIDER_THUMB_DISABLED_PUSHED, info->borderColor, info->color );

						info = GetStateInfo( HSLIDER_HILITE_LEFT );
						StoreColor( HSLIDER_THUMB_HILITE, info->color, info->borderColor );
						StoreColor( HSLIDER_THUMB_HILITE_PUSHED, info->borderColor, info->color );

					}  // end else horizontal slider

					break;

				}  // end case subcontrol color

				// --------------------------------------------------------------------
        case IDOK:
				{
					GameWindow *window = TheEditor->getPropertyTarget();

					// sanity
					if( window )
					{
						ImageAndColorInfo *info;
						Bool vert = BitIsSet( window->winGetStyle(), GWS_VERT_SLIDER );

						// save the common properties
						if( SaveCommonDialogProperties( hWndDialog, window ) == FALSE )
							break;

						// save the image and color data
						// ----------------------------------------------------------------
						if( vert )
						{
							info = GetStateInfo( VSLIDER_ENABLED_TOP );
							GadgetSliderSetEnabledImageTop( window, info->image );
							GadgetSliderSetEnabledColor( window, info->color );
							GadgetSliderSetEnabledBorderColor( window, info->borderColor );
						
							info = GetStateInfo( VSLIDER_ENABLED_BOTTOM );
							GadgetSliderSetEnabledImageBottom( window, info->image );
						}  // end if
						else
						{
							info = GetStateInfo( HSLIDER_ENABLED_LEFT );
							GadgetSliderSetEnabledImageLeft( window, info->image );
							GadgetSliderSetEnabledColor( window, info->color );
							GadgetSliderSetEnabledBorderColor( window, info->borderColor );
							
							info = GetStateInfo( HSLIDER_ENABLED_RIGHT );
							GadgetSliderSetEnabledImageRight( window, info->image );
						}  // end else

						info = GetStateInfo( vert ? VSLIDER_ENABLED_CENTER : HSLIDER_ENABLED_CENTER );
						GadgetSliderSetEnabledImageCenter( window, info->image );

						info = GetStateInfo( vert ? VSLIDER_ENABLED_SMALL_CENTER : HSLIDER_ENABLED_SMALL_CENTER );
						GadgetSliderSetEnabledImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						if( vert )
						{
							info = GetStateInfo( VSLIDER_DISABLED_TOP );
							GadgetSliderSetDisabledImageTop( window, info->image );
							GadgetSliderSetDisabledColor( window, info->color );
							GadgetSliderSetDisabledBorderColor( window, info->borderColor );
						
							info = GetStateInfo( VSLIDER_DISABLED_BOTTOM );
							GadgetSliderSetDisabledImageBottom( window, info->image );
						}  // end if
						else
						{
							info = GetStateInfo( HSLIDER_DISABLED_LEFT );
							GadgetSliderSetDisabledImageLeft( window, info->image );
							GadgetSliderSetDisabledColor( window, info->color );
							GadgetSliderSetDisabledBorderColor( window, info->borderColor );
							
							info = GetStateInfo( HSLIDER_DISABLED_RIGHT );
							GadgetSliderSetDisabledImageRight( window, info->image );
						}  // end else

						info = GetStateInfo( vert ? VSLIDER_DISABLED_CENTER : HSLIDER_DISABLED_CENTER );
						GadgetSliderSetDisabledImageCenter( window, info->image );

						info = GetStateInfo( vert ? VSLIDER_DISABLED_SMALL_CENTER : HSLIDER_DISABLED_SMALL_CENTER );
						GadgetSliderSetDisabledImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						if( vert )
						{
							info = GetStateInfo( VSLIDER_HILITE_TOP );
							GadgetSliderSetHiliteImageTop( window, info->image );
							GadgetSliderSetHiliteColor( window, info->color );
							GadgetSliderSetHiliteBorderColor( window, info->borderColor );
						
							info = GetStateInfo( VSLIDER_HILITE_BOTTOM );
							GadgetSliderSetHiliteImageBottom( window, info->image );
						}  // end if
						else
						{
							info = GetStateInfo( HSLIDER_HILITE_LEFT );
							GadgetSliderSetHiliteImageLeft( window, info->image );
							GadgetSliderSetHiliteColor( window, info->color );
							GadgetSliderSetHiliteBorderColor( window, info->borderColor );
							
							info = GetStateInfo( HSLIDER_HILITE_RIGHT );
							GadgetSliderSetHiliteImageRight( window, info->image );
						}  // end else

						info = GetStateInfo( vert ? VSLIDER_HILITE_CENTER : HSLIDER_HILITE_CENTER );
						GadgetSliderSetHiliteImageCenter( window, info->image );

						info = GetStateInfo( vert ? VSLIDER_HILITE_SMALL_CENTER : HSLIDER_HILITE_SMALL_CENTER );
						GadgetSliderSetHiliteImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( vert ? VSLIDER_THUMB_ENABLED : HSLIDER_THUMB_ENABLED );
						GadgetSliderSetEnabledThumbImage( window, info->image );
						GadgetSliderSetEnabledThumbColor( window, info->color );
						GadgetSliderSetEnabledThumbBorderColor( window, info->borderColor );

						info = GetStateInfo( vert ? VSLIDER_THUMB_ENABLED_PUSHED : HSLIDER_THUMB_ENABLED_PUSHED );
						GadgetSliderSetEnabledSelectedThumbImage( window, info->image );
						GadgetSliderSetEnabledSelectedThumbColor( window, info->color );
						GadgetSliderSetEnabledSelectedThumbBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( vert ? VSLIDER_THUMB_DISABLED : HSLIDER_THUMB_DISABLED );
						GadgetSliderSetDisabledThumbImage( window, info->image );
						GadgetSliderSetDisabledThumbColor( window, info->color );
						GadgetSliderSetDisabledThumbBorderColor( window, info->borderColor );

						info = GetStateInfo( vert ? VSLIDER_THUMB_DISABLED_PUSHED : HSLIDER_THUMB_DISABLED_PUSHED );
						GadgetSliderSetDisabledSelectedThumbImage( window, info->image );
						GadgetSliderSetDisabledSelectedThumbColor( window, info->color );
						GadgetSliderSetDisabledSelectedThumbBorderColor( window, info->borderColor );

						// ----------------------------------------------------------------
						info = GetStateInfo( vert ? VSLIDER_THUMB_HILITE : HSLIDER_THUMB_HILITE );
						GadgetSliderSetHiliteThumbImage( window, info->image );
						GadgetSliderSetHiliteThumbColor( window, info->color );
						GadgetSliderSetHiliteThumbBorderColor( window, info->borderColor );

						info = GetStateInfo( vert ? VSLIDER_THUMB_HILITE_PUSHED : HSLIDER_THUMB_HILITE_PUSHED );
						GadgetSliderSetHiliteSelectedThumbImage( window, info->image );
						GadgetSliderSetHiliteSelectedThumbColor( window, info->color );
						GadgetSliderSetHiliteSelectedThumbBorderColor( window, info->borderColor );

						// slider data
						SliderData *sliderData = (SliderData *)window->winGetUserData();

						sliderData->minVal = GetDlgItemInt( hWndDialog, EDIT_SLIDER_MIN, NULL, FALSE );
						sliderData->maxVal = GetDlgItemInt( hWndDialog, EDIT_SLIDER_MAX, NULL, FALSE );

						// sanity
						if( sliderData->minVal > sliderData->maxVal )
						{
							Int temp = sliderData->minVal;
							sliderData->minVal = sliderData->maxVal;
							sliderData->maxVal = temp;

							MessageBox( NULL, "Slider min greated than max, the values were swapped", 
													"Warning", MB_OK | MB_ICONINFORMATION );

						}  // end if

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

}  // end sliderPropertiesCallback


///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitSliderPropertiesDialog =================================================
/** Slider properties dialog */
//=============================================================================
HWND InitSliderPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)SLIDER_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)sliderPropertiesCallback );
	if( dialog == NULL )
		return NULL;

	// do the common initialization
	CommonDialogInitialize( window, dialog );

	//
	// init values
	//
	const Image *image;
	Color color, borderColor;
	Bool vert = BitIsSet( window->winGetStyle(), GWS_VERT_SLIDER );

	// --------------------------------------------------------------------------
	if( vert )
		image = GadgetSliderGetEnabledImageTop( window );
	else
		image = GadgetSliderGetEnabledImageLeft( window );
	color = GadgetSliderGetEnabledColor( window );
	borderColor = GadgetSliderGetEnabledBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_ENABLED_TOP : HSLIDER_ENABLED_LEFT, image, color, borderColor );

	if( vert )
		image = GadgetSliderGetEnabledImageBottom( window );
	else
		image = GadgetSliderGetEnabledImageRight( window );
	StoreImageAndColor( vert ? VSLIDER_ENABLED_BOTTOM : HSLIDER_ENABLED_RIGHT, image, color, borderColor );

	image = GadgetSliderGetEnabledImageCenter( window );
	StoreImageAndColor( vert ? VSLIDER_ENABLED_CENTER : HSLIDER_ENABLED_CENTER, image, color, borderColor );

	image = GadgetSliderGetEnabledImageSmallCenter( window );
	StoreImageAndColor( vert ? VSLIDER_ENABLED_SMALL_CENTER : HSLIDER_ENABLED_SMALL_CENTER, image, color, borderColor );

	// --------------------------------------------------------------------------

	if( vert )
		image = GadgetSliderGetDisabledImageTop( window );
	else
		image = GadgetSliderGetDisabledImageLeft( window );
	color = GadgetSliderGetDisabledColor( window );
	borderColor = GadgetSliderGetDisabledBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_DISABLED_TOP : HSLIDER_DISABLED_LEFT, image, color, borderColor );

	if( vert )
		image = GadgetSliderGetDisabledImageBottom( window );
	else
		image = GadgetSliderGetDisabledImageRight( window );
	StoreImageAndColor( vert ? VSLIDER_DISABLED_BOTTOM : HSLIDER_DISABLED_RIGHT, image, color, borderColor );

	image = GadgetSliderGetDisabledImageCenter( window );
	StoreImageAndColor( vert ? VSLIDER_DISABLED_CENTER : HSLIDER_DISABLED_CENTER, image, color, borderColor );

	image = GadgetSliderGetDisabledImageSmallCenter( window );
	StoreImageAndColor( vert ? VSLIDER_DISABLED_SMALL_CENTER : HSLIDER_DISABLED_SMALL_CENTER, image, color, borderColor );

	// --------------------------------------------------------------------------

	if( vert )
		image = GadgetSliderGetHiliteImageTop( window );
	else
		image = GadgetSliderGetHiliteImageLeft( window );
	color = GadgetSliderGetHiliteColor( window );
	borderColor = GadgetSliderGetHiliteBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_HILITE_TOP : HSLIDER_HILITE_LEFT, image, color, borderColor );

	if( vert )
		image = GadgetSliderGetHiliteImageBottom( window );
	else
		image = GadgetSliderGetHiliteImageRight( window );
	StoreImageAndColor( vert ? VSLIDER_HILITE_BOTTOM : HSLIDER_HILITE_RIGHT, image, color, borderColor );

	image = GadgetSliderGetHiliteImageCenter( window );
	StoreImageAndColor( vert ? VSLIDER_HILITE_CENTER : HSLIDER_HILITE_CENTER, image, color, borderColor );

	image = GadgetSliderGetHiliteImageSmallCenter( window );
	StoreImageAndColor( vert ? VSLIDER_HILITE_SMALL_CENTER : HSLIDER_HILITE_SMALL_CENTER, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetSliderGetEnabledThumbImage( window );
	color = GadgetSliderGetEnabledThumbColor( window );
	borderColor = GadgetSliderGetEnabledThumbBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_THUMB_ENABLED : HSLIDER_THUMB_ENABLED, image, color, borderColor );

	image = GadgetSliderGetEnabledSelectedThumbImage( window );
	color = GadgetSliderGetEnabledSelectedThumbColor( window );
	borderColor = GadgetSliderGetEnabledSelectedThumbBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_THUMB_ENABLED_PUSHED : HSLIDER_THUMB_ENABLED_PUSHED, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetSliderGetDisabledThumbImage( window );
	color = GadgetSliderGetDisabledThumbColor( window );
	borderColor = GadgetSliderGetDisabledThumbBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_THUMB_DISABLED : HSLIDER_THUMB_DISABLED, image, color, borderColor );

	image = GadgetSliderGetDisabledSelectedThumbImage( window );
	color = GadgetSliderGetDisabledSelectedThumbColor( window );
	borderColor = GadgetSliderGetDisabledSelectedThumbBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_THUMB_DISABLED_PUSHED : HSLIDER_THUMB_DISABLED_PUSHED, image, color, borderColor );

	// --------------------------------------------------------------------------
	image = GadgetSliderGetHiliteThumbImage( window );
	color = GadgetSliderGetHiliteThumbColor( window );
	borderColor = GadgetSliderGetHiliteThumbBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_THUMB_HILITE : HSLIDER_THUMB_HILITE, image, color, borderColor );

	image = GadgetSliderGetHiliteSelectedThumbImage( window );
	color = GadgetSliderGetHiliteSelectedThumbColor( window );
	borderColor = GadgetSliderGetHiliteSelectedThumbBorderColor( window );
	StoreImageAndColor( vert ? VSLIDER_THUMB_HILITE_PUSHED : HSLIDER_THUMB_HILITE_PUSHED, image, color, borderColor );

	// slider data
	SliderData *sliderData = (SliderData *)window->winGetUserData();

	SetDlgItemInt( dialog, EDIT_SLIDER_MIN, sliderData->minVal, FALSE );
	SetDlgItemInt( dialog, EDIT_SLIDER_MAX, sliderData->maxVal, FALSE );

	// select the enabled state for display
	SwitchToState( vert ? VSLIDER_ENABLED_TOP : HSLIDER_ENABLED_LEFT, dialog );

	return dialog;

}  // end InitSliderPropertiesDialog



