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

// FILE: LayoutScheme.cpp /////////////////////////////////////////////////////
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
// File name:  LayoutScheme.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Layout scheme loading and editing
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/GadgetProgressBar.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetTabControl.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetStaticText.h"
#include "GUIEdit.h"
#include "EditWindow.h"
#include "GUIEditWindowManager.h"
#include "resource.h"
#include "Properties.h"
#include "LayoutScheme.h"

// DEFINES ////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE TYPES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define SCHEME_VERSION 1

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static LayoutScheme *theScheme = NULL;

///////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
LayoutScheme *TheDefaultScheme = NULL;

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// loadSchemeDataToDialog =====================================================
/** Load the current data in theScheme to the dialog layout */
//=============================================================================
static void loadSchemeDataToDialog( HWND hWndDialog )
{

	// load the name of the current scheme
	SetDlgItemText( hWndDialog, STATIC_CURRENT_SCHEME, 
									theScheme->getSchemeFilename() );

	// load the state combo box with every option available
	SendDlgItemMessage( hWndDialog, COMBO_STATE, CB_RESETCONTENT, 0, 0 );
	LoadStateCombo( GWS_PUSH_BUTTON |
									GWS_RADIO_BUTTON |
									GWS_CHECK_BOX |
									GWS_PROGRESS_BAR |
									GWS_SCROLL_LISTBOX |
									GWS_HORZ_SLIDER |
									GWS_VERT_SLIDER |
									GWS_STATIC_TEXT |
									GWS_ENTRY_FIELD | 
									GWS_USER_WINDOW |
									GWS_COMBO_BOX,
									GetDlgItem( hWndDialog, COMBO_STATE ) );

	// load the image combo box with the images from the gui collection
	SendDlgItemMessage( hWndDialog, COMBO_IMAGE, CB_RESETCONTENT, 0, 0 );
	LoadImageListComboBox( GetDlgItem( hWndDialog, COMBO_IMAGE ) );

	//
	// load our own internal table of images and colors to the 
	// property dialog tables
	//
	Int i;
	ImageAndColorInfo *info;
	for( i = FIRST_VALID_IDENTIFIER; i < NUM_STATE_IDENTIFIERS; i++ )
	{

		info = theScheme->getImageAndColor( (StateIdentifier)i );
		assert( info );
		StoreImageAndColor( info->stateID,
												info->image,
												info->color,
												info->borderColor );

	}  // end for i

	// switch to the generic state
	SwitchToState( GENERIC_ENABLED, hWndDialog );

	// load the text color combo
	SendDlgItemMessage( hWndDialog, COMBO_TEXT_STATE, CB_RESETCONTENT, 0, 0 );
	LoadTextStateCombo( GetDlgItem( hWndDialog, COMBO_TEXT_STATE ),
											theScheme->getEnabledTextColor(),
											theScheme->getEnabledTextBorderColor(),
											theScheme->getDisabledTextColor(),
											theScheme->getDisabledTextBorderColor(),
											theScheme->getHiliteTextColor(),
											theScheme->getHiliteTextBorderColor() );

	// load the font combo if present
	SendDlgItemMessage( hWndDialog, COMBO_FONT, CB_RESETCONTENT, 0, 0 );
	LoadFontCombo( GetDlgItem( hWndDialog, COMBO_FONT ), theScheme->getFont() );

}  // end loadSchemeDataToDialog

// saveData ===================================================================
//=============================================================================
static void saveData( HWND hWndDialog )
{

	//
	// copy the image and color data from the property tables to our
	// own
	//
	Int i;
	ImageAndColorInfo *info;
	for( i = FIRST_VALID_IDENTIFIER; i < NUM_STATE_IDENTIFIERS; i++ )
	{

	info = GetStateInfo( (StateIdentifier)i );
	assert( info );
	theScheme->storeImageAndColor( info->stateID,
																 info->image,
																 info->color,
																 info->borderColor );

	}  // end for i

	// save default text colors
	theScheme->setEnabledTextColor( GetPropsEnabledTextColor() );
	theScheme->setEnabledTextBorderColor( GetPropsEnabledTextBorderColor() );
	theScheme->setDisabledTextColor( GetPropsDisabledTextColor() );
	theScheme->setDisabledTextBorderColor( GetPropsDisabledTextBorderColor() );
	theScheme->setHiliteTextColor( GetPropsHiliteTextColor() );
	theScheme->setHiliteTextBorderColor( GetPropsHiliteTextBorderColor() );

	// save the font
	theScheme->setFont( GetSelectedFontFromCombo( GetDlgItem( hWndDialog, COMBO_FONT ) ) );

}  // end saveData

// saveAsDialog ===============================================================
/** Bring up the standard windows browser save as dialog and return
	* filename selected */
//=============================================================================
char *saveAsDialog( void )
{
	static char filename[ _MAX_PATH ];
  OPENFILENAME ofn;
  Bool returnCode;
  char filter[] = "Layout Scheme Files (*.ls)\0*.ls\0"  \
                  "All Files (*.*)\0*.*\0\0" ;

  ofn.lStructSize       = sizeof( OPENFILENAME );
  ofn.hwndOwner         = TheEditor->getWindowHandle();
  ofn.hInstance         = NULL;
  ofn.lpstrFilter       = filter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 0;
  ofn.lpstrFile         = filename;
  ofn.nMaxFile          = _MAX_PATH;
  ofn.lpstrFileTitle    = NULL;
  ofn.nMaxFileTitle     = 0;
  ofn.lpstrInitialDir   = NULL;
  ofn.lpstrTitle        = NULL;
  ofn.Flags             = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = "ls";
  ofn.lCustData         = 0L ;
  ofn.lpfnHook          = NULL ;
  ofn.lpTemplateName    = NULL ;

  returnCode = GetSaveFileName( &ofn );

  if( returnCode )
		return filename;
	else
		return NULL;

}  // end saveAsDialog

// openDialog =================================================================
/** Bring up the standard windows browser open dialog and return
	* filename selected */
//=============================================================================
char *openDialog( void )
{
	static char filename[ _MAX_PATH ];
  OPENFILENAME ofn;
  Bool returnCode;
  char filter[] = "Layout Scheme Files (*.ls)\0*.ls\0"  \
                  "All Files (*.*)\0*.*\0\0" ;

  ofn.lStructSize       = sizeof( OPENFILENAME );
  ofn.hwndOwner         = TheEditor->getWindowHandle();
  ofn.hInstance         = NULL;
  ofn.lpstrFilter       = filter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 0;
  ofn.lpstrFile         = filename;
  ofn.nMaxFile          = _MAX_PATH;
  ofn.lpstrFileTitle    = NULL;
  ofn.nMaxFileTitle     = 0;
  ofn.lpstrInitialDir   = NULL;
  ofn.lpstrTitle        = NULL;
  ofn.Flags             = OFN_NOREADONLYRETURN | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = "ls";
  ofn.lCustData         = 0L ;
  ofn.lpfnHook          = NULL ;
  ofn.lpTemplateName    = NULL ;

  returnCode = GetOpenFileName( &ofn );

  if( returnCode )
		return filename;
	else
		return NULL;

}  // end openDialog

// layoutSchemeCallback =======================================================
/** Dialog callback for layout scheme dialog */
//=============================================================================
static LRESULT CALLBACK layoutSchemeCallback( HWND hWndDialog,
																							UINT message,
																							WPARAM wParam,
																							LPARAM lParam )
{
	Int returnCode;

	//
	// this is largely based on the property dialog code so lets just
	// let the default handle code work here too
	//
	if( HandleCommonDialogMessages( hWndDialog, message, 
																	wParam, lParam, &returnCode ) == TRUE )
		return returnCode;

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_INITDIALOG:
		{

			// load the dialog engine and items with the current data of the scheme
			loadSchemeDataToDialog( hWndDialog );
			break;

		}  // end init dialog

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {
//			Int notifyCode = HIWORD( wParam );  // notification code
			Int controlID = LOWORD( wParam );  // control ID
//			HWND hWndControl = (HWND)lParam;  // control window handle
		
			switch( controlID )
			{

				// --------------------------------------------------------------------
				case BUTTON_APPLY_SCHEME:
				{
					Int result;

					//
					// this will take the scheme settings currently loaded as the
					// "working scheme" to ALL windows and gadgets in the current
					// layout, because this is such a large sweeping change we will
					// ask them if they are sure
					//
					result = MessageBox( NULL, "This will apply these scheme color and image settings to ALL windows and gadgets currently loaded in the edit window.  Are you sure you want to proceed?",
															 "Are You Sure?", MB_YESNO | MB_ICONWARNING );
					if( result == IDNO )
						break;

					//
					// change all the windows to have the settings loaded in the
					// current working scheme in the state identifier entry tables,
					// note that we are applying the scheme in the property
					// entry tables and NOT the scheme in "theScheme", this is to
					// allow people to load a scheme file, or edit the parameters
					// and apply those changes to controls without changing the
					// current layout scheme.
					//
					theScheme->applyPropertyTablesToWindow( TheWindowManager->winGetWindowList() );

					//
					// also apply these changes to the window lists in the clipboard
					// just to be complete
					//
					theScheme->applyPropertyTablesToWindow( TheGUIEditWindowManager->getClipboardList() );
					theScheme->applyPropertyTablesToWindow( TheGUIEditWindowManager->getClipboardDupeList() );

					break;

				}  // end apply scheme

				// --------------------------------------------------------------------
				case BUTTON_SAVE:
				{
					char *filename = saveAsDialog();

					if( theScheme && filename )
					{

						saveData( hWndDialog );
						theScheme->saveScheme( filename );
						SetDlgItemText( hWndDialog, STATIC_CURRENT_SCHEME, 
														theScheme->getSchemeFilename() );

					}  // end if

					break;

				}  // end save
				
				// --------------------------------------------------------------------
				case BUTTON_LOAD:
				{
					char *filename = openDialog();

					if( theScheme && filename )
					{

						// load the new data
						if( theScheme->loadScheme( filename ) )
						{
										
							// load the dialog engine and items with the current data of the scheme
							loadSchemeDataToDialog( hWndDialog );

						}  // end if
						else
						{

							MessageBox( TheEditor->getWindowHandle(),
													"Unable to open scheme file.", "Error", MB_OK );

						}  // end else

					}  // end if

					break;

				}  // end load
							
				// --------------------------------------------------------------------
				case IDOK:

					saveData( hWndDialog );
					theScheme->saveScheme( theScheme->getSchemeFilename() );
					EndDialog( hWndDialog, TRUE );
					break;

				// --------------------------------------------------------------------
				case IDCANCEL:

					// do nothing, just get outta here!
					EndDialog( hWndDialog, FALSE );
					break;

			}  // end switch
				
			break;

		}  // end command

	}  // end switch, message

	return 0;

}  // end layoutSchemeCallback

// LayoutScheme::applyPropertyTablesToWindow ==================================
/** apply the image and color info stored in the state identifer tables
used for "property editing" to all appropriate windows currently
loaded in the editor */
//=============================================================================
void LayoutScheme::applyPropertyTablesToWindow( GameWindow *root )
{

	// end recursion
	if( root == NULL )
		return;

	// apply changes to this window
	ImageAndColorInfo *info;

	if( BitIsSet( root->winGetStyle(), GWS_PUSH_BUTTON ) )
	{

		info = GetStateInfo( BUTTON_ENABLED );
		GadgetButtonSetEnabledImage( root, info->image );
		GadgetButtonSetEnabledColor( root, info->color );
		GadgetButtonSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( BUTTON_ENABLED_PUSHED );
		GadgetButtonSetEnabledSelectedImage( root, info->image );
		GadgetButtonSetEnabledSelectedColor( root, info->color );
		GadgetButtonSetEnabledSelectedBorderColor( root, info->borderColor );

		info = GetStateInfo( BUTTON_DISABLED );
		GadgetButtonSetDisabledImage( root, info->image );
		GadgetButtonSetDisabledColor( root, info->color );
		GadgetButtonSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( BUTTON_DISABLED_PUSHED );
		GadgetButtonSetDisabledSelectedImage( root, info->image );
		GadgetButtonSetDisabledSelectedColor( root, info->color );
		GadgetButtonSetDisabledSelectedBorderColor( root, info->borderColor );

		info = GetStateInfo( BUTTON_HILITE );
		GadgetButtonSetHiliteImage( root, info->image );
		GadgetButtonSetHiliteColor( root, info->color );
		GadgetButtonSetHiliteBorderColor( root, info->borderColor );
		info = GetStateInfo( BUTTON_HILITE_PUSHED );
		GadgetButtonSetHiliteSelectedImage( root, info->image );
		GadgetButtonSetHiliteSelectedColor( root, info->color );
		GadgetButtonSetHiliteSelectedBorderColor( root, info->borderColor );

	}  // end if
	else if( BitIsSet( root->winGetStyle(), GWS_RADIO_BUTTON ) )
	{

		info = GetStateInfo( RADIO_ENABLED );
		GadgetRadioSetEnabledImage( root, info->image );
		GadgetRadioSetEnabledColor( root, info->color );
		GadgetRadioSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( RADIO_ENABLED_UNCHECKED_BOX );
		GadgetRadioSetEnabledUncheckedBoxImage( root, info->image );
		GadgetRadioSetEnabledUncheckedBoxColor( root, info->color );
		GadgetRadioSetEnabledUncheckedBoxBorderColor( root, info->borderColor );
		info = GetStateInfo( RADIO_ENABLED_CHECKED_BOX );
		GadgetRadioSetEnabledCheckedBoxImage( root, info->image );
		GadgetRadioSetEnabledCheckedBoxColor( root, info->color );
		GadgetRadioSetEnabledCheckedBoxBorderColor( root, info->borderColor );

		info = GetStateInfo( RADIO_DISABLED );
		GadgetRadioSetDisabledImage( root, info->image );
		GadgetRadioSetDisabledColor( root, info->color );
		GadgetRadioSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( RADIO_DISABLED_UNCHECKED_BOX );
		GadgetRadioSetDisabledUncheckedBoxImage( root, info->image );
		GadgetRadioSetDisabledUncheckedBoxColor( root, info->color );
		GadgetRadioSetDisabledUncheckedBoxBorderColor( root, info->borderColor );
		info = GetStateInfo( RADIO_DISABLED_CHECKED_BOX );
		GadgetRadioSetDisabledCheckedBoxImage( root, info->image );
		GadgetRadioSetDisabledCheckedBoxColor( root, info->color );
		GadgetRadioSetDisabledCheckedBoxBorderColor( root, info->borderColor );

		info = GetStateInfo( RADIO_HILITE );
		GadgetRadioSetHiliteImage( root, info->image );
		GadgetRadioSetHiliteColor( root, info->color );
		GadgetRadioSetHiliteBorderColor( root, info->borderColor );
		info = GetStateInfo( RADIO_HILITE_UNCHECKED_BOX );
		GadgetRadioSetHiliteUncheckedBoxImage( root, info->image );
		GadgetRadioSetHiliteUncheckedBoxColor( root, info->color );
		GadgetRadioSetHiliteUncheckedBoxBorderColor( root, info->borderColor );
		info = GetStateInfo( RADIO_HILITE_CHECKED_BOX );
		GadgetRadioSetHiliteCheckedBoxImage( root, info->image );
		GadgetRadioSetHiliteCheckedBoxColor( root, info->color );
		GadgetRadioSetHiliteCheckedBoxBorderColor( root, info->borderColor );

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_CHECK_BOX ) )
	{

		info = GetStateInfo( CHECK_BOX_ENABLED );
		GadgetCheckBoxSetEnabledImage( root, info->image );
		GadgetCheckBoxSetEnabledColor( root, info->color );
		GadgetCheckBoxSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( CHECK_BOX_ENABLED_UNCHECKED_BOX );
		GadgetCheckBoxSetEnabledUncheckedBoxImage( root, info->image );
		GadgetCheckBoxSetEnabledUncheckedBoxColor( root, info->color );
		GadgetCheckBoxSetEnabledUncheckedBoxBorderColor( root, info->borderColor );
		info = GetStateInfo( CHECK_BOX_ENABLED_CHECKED_BOX );
		GadgetCheckBoxSetEnabledCheckedBoxImage( root, info->image );
		GadgetCheckBoxSetEnabledCheckedBoxColor( root, info->color );
		GadgetCheckBoxSetEnabledCheckedBoxBorderColor( root, info->borderColor );

		info = GetStateInfo( CHECK_BOX_DISABLED );
		GadgetCheckBoxSetDisabledImage( root, info->image );
		GadgetCheckBoxSetDisabledColor( root, info->color );
		GadgetCheckBoxSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( CHECK_BOX_DISABLED_UNCHECKED_BOX );
		GadgetCheckBoxSetDisabledUncheckedBoxImage( root, info->image );
		GadgetCheckBoxSetDisabledUncheckedBoxColor( root, info->color );
		GadgetCheckBoxSetDisabledUncheckedBoxBorderColor( root, info->borderColor );
		info = GetStateInfo( CHECK_BOX_DISABLED_CHECKED_BOX );
		GadgetCheckBoxSetDisabledCheckedBoxImage( root, info->image );
		GadgetCheckBoxSetDisabledCheckedBoxColor( root, info->color );
		GadgetCheckBoxSetDisabledCheckedBoxBorderColor( root, info->borderColor );

		info = GetStateInfo( CHECK_BOX_HILITE );
		GadgetCheckBoxSetHiliteImage( root, info->image );
		GadgetCheckBoxSetHiliteColor( root, info->color );
		GadgetCheckBoxSetHiliteBorderColor( root, info->borderColor );
		info = GetStateInfo( CHECK_BOX_HILITE_UNCHECKED_BOX );
		GadgetCheckBoxSetHiliteUncheckedBoxImage( root, info->image );
		GadgetCheckBoxSetHiliteUncheckedBoxColor( root, info->color );
		GadgetCheckBoxSetHiliteUncheckedBoxBorderColor( root, info->borderColor );
		info = GetStateInfo( CHECK_BOX_HILITE_CHECKED_BOX );
		GadgetCheckBoxSetHiliteCheckedBoxImage( root, info->image );
		GadgetCheckBoxSetHiliteCheckedBoxColor( root, info->color );
		GadgetCheckBoxSetHiliteCheckedBoxBorderColor( root, info->borderColor );

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_VERT_SLIDER ) )
	{

		info = GetStateInfo( VSLIDER_ENABLED_TOP );
		GadgetSliderSetEnabledImageTop( root, info->image );
		GadgetSliderSetEnabledColor( root, info->color );
		GadgetSliderSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( VSLIDER_ENABLED_BOTTOM );
		GadgetSliderSetEnabledImageBottom( root, info->image );
		info = GetStateInfo( VSLIDER_ENABLED_CENTER );
		GadgetSliderSetEnabledImageCenter( root, info->image );
		info = GetStateInfo( VSLIDER_ENABLED_SMALL_CENTER );
		GadgetSliderSetEnabledImageSmallCenter( root, info->image );

		info = GetStateInfo( VSLIDER_DISABLED_TOP );
		GadgetSliderSetDisabledImageTop( root, info->image );
		GadgetSliderSetDisabledColor( root, info->color );
		GadgetSliderSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( VSLIDER_DISABLED_BOTTOM );
		GadgetSliderSetDisabledImageBottom( root, info->image );
		info = GetStateInfo( VSLIDER_DISABLED_CENTER );
		GadgetSliderSetDisabledImageCenter( root, info->image );
		info = GetStateInfo( VSLIDER_DISABLED_SMALL_CENTER );
		GadgetSliderSetDisabledImageSmallCenter( root, info->image );

		info = GetStateInfo( VSLIDER_HILITE_TOP );
		GadgetSliderSetDisabledImageTop( root, info->image );
		GadgetSliderSetDisabledColor( root, info->color );
		GadgetSliderSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( VSLIDER_HILITE_BOTTOM );
		GadgetSliderSetDisabledImageBottom( root, info->image );
		info = GetStateInfo( VSLIDER_HILITE_CENTER );
		GadgetSliderSetDisabledImageCenter( root, info->image );
		info = GetStateInfo( VSLIDER_HILITE_SMALL_CENTER );
		GadgetSliderSetDisabledImageSmallCenter( root, info->image );

		info = GetStateInfo( VSLIDER_THUMB_ENABLED );
		GadgetSliderSetEnabledThumbImage( root, info->image );
		GadgetSliderSetEnabledThumbColor( root, info->color );
		GadgetSliderSetEnabledThumbBorderColor( root, info->borderColor );
		info = GetStateInfo( VSLIDER_THUMB_ENABLED_PUSHED );
		GadgetSliderSetEnabledSelectedThumbImage( root, info->image );
		GadgetSliderSetEnabledSelectedThumbColor( root, info->color );
		GadgetSliderSetEnabledSelectedThumbBorderColor( root, info->borderColor );

		info = GetStateInfo( VSLIDER_THUMB_DISABLED );
		GadgetSliderSetDisabledThumbImage( root, info->image );
		GadgetSliderSetDisabledThumbColor( root, info->color );
		GadgetSliderSetDisabledThumbBorderColor( root, info->borderColor );
		info = GetStateInfo( VSLIDER_THUMB_DISABLED_PUSHED );
		GadgetSliderSetDisabledSelectedThumbImage( root, info->image );
		GadgetSliderSetDisabledSelectedThumbColor( root, info->color );
		GadgetSliderSetDisabledSelectedThumbBorderColor( root, info->borderColor );

		info = GetStateInfo( VSLIDER_THUMB_HILITE );
		GadgetSliderSetHiliteThumbImage( root, info->image );
		GadgetSliderSetHiliteThumbColor( root, info->color );
		GadgetSliderSetHiliteThumbBorderColor( root, info->borderColor );
		info = GetStateInfo( VSLIDER_THUMB_HILITE_PUSHED );
		GadgetSliderSetHiliteSelectedThumbImage( root, info->image );
		GadgetSliderSetHiliteSelectedThumbColor( root, info->color );
		GadgetSliderSetHiliteSelectedThumbBorderColor( root, info->borderColor );

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_HORZ_SLIDER ) )
	{

		info = GetStateInfo( HSLIDER_ENABLED_LEFT );
		GadgetSliderSetEnabledImageLeft( root, info->image );
		GadgetSliderSetEnabledColor( root, info->color );
		GadgetSliderSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( HSLIDER_ENABLED_RIGHT );
		GadgetSliderSetEnabledImageRight( root, info->image );
		info = GetStateInfo( HSLIDER_ENABLED_CENTER );
		GadgetSliderSetEnabledImageCenter( root, info->image );
		info = GetStateInfo( HSLIDER_ENABLED_SMALL_CENTER );
		GadgetSliderSetEnabledImageSmallCenter( root, info->image );

		info = GetStateInfo( HSLIDER_DISABLED_LEFT );
		GadgetSliderSetDisabledImageLeft( root, info->image );
		GadgetSliderSetDisabledColor( root, info->color );
		GadgetSliderSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( HSLIDER_DISABLED_RIGHT );
		GadgetSliderSetDisabledImageRight( root, info->image );
		info = GetStateInfo( HSLIDER_DISABLED_CENTER );
		GadgetSliderSetDisabledImageCenter( root, info->image );
		info = GetStateInfo( HSLIDER_DISABLED_SMALL_CENTER );
		GadgetSliderSetDisabledImageSmallCenter( root, info->image );

		info = GetStateInfo( HSLIDER_HILITE_LEFT );
		GadgetSliderSetDisabledImageLeft( root, info->image );
		GadgetSliderSetDisabledColor( root, info->color );
		GadgetSliderSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( HSLIDER_HILITE_RIGHT );
		GadgetSliderSetDisabledImageRight( root, info->image );
		info = GetStateInfo( HSLIDER_HILITE_CENTER );
		GadgetSliderSetDisabledImageCenter( root, info->image );
		info = GetStateInfo( HSLIDER_HILITE_SMALL_CENTER );
		GadgetSliderSetDisabledImageSmallCenter( root, info->image );

		info = GetStateInfo( HSLIDER_THUMB_ENABLED );
		GadgetSliderSetEnabledThumbImage( root, info->image );
		GadgetSliderSetEnabledThumbColor( root, info->color );
		GadgetSliderSetEnabledThumbBorderColor( root, info->borderColor );
		info = GetStateInfo( HSLIDER_THUMB_ENABLED_PUSHED );
		GadgetSliderSetEnabledSelectedThumbImage( root, info->image );
		GadgetSliderSetEnabledSelectedThumbColor( root, info->color );
		GadgetSliderSetEnabledSelectedThumbBorderColor( root, info->borderColor );

		info = GetStateInfo( HSLIDER_THUMB_DISABLED );
		GadgetSliderSetDisabledThumbImage( root, info->image );
		GadgetSliderSetDisabledThumbColor( root, info->color );
		GadgetSliderSetDisabledThumbBorderColor( root, info->borderColor );
		info = GetStateInfo( HSLIDER_THUMB_DISABLED_PUSHED );
		GadgetSliderSetDisabledSelectedThumbImage( root, info->image );
		GadgetSliderSetDisabledSelectedThumbColor( root, info->color );
		GadgetSliderSetDisabledSelectedThumbBorderColor( root, info->borderColor );

		info = GetStateInfo( HSLIDER_THUMB_HILITE );
		GadgetSliderSetHiliteThumbImage( root, info->image );
		GadgetSliderSetHiliteThumbColor( root, info->color );
		GadgetSliderSetHiliteThumbBorderColor( root, info->borderColor );
		info = GetStateInfo( HSLIDER_THUMB_HILITE_PUSHED );
		GadgetSliderSetHiliteSelectedThumbImage( root, info->image );
		GadgetSliderSetHiliteSelectedThumbColor( root, info->color );
		GadgetSliderSetHiliteSelectedThumbBorderColor( root, info->borderColor );

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_SCROLL_LISTBOX ) )
	{

		info = GetStateInfo( LISTBOX_ENABLED );
		GadgetListBoxSetEnabledImage( root, info->image );
		GadgetListBoxSetEnabledColor( root, info->color );
		GadgetListBoxSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_LEFT );
		GadgetListBoxSetEnabledSelectedItemImageLeft( root, info->image );
		GadgetListBoxSetEnabledSelectedItemColor( root, info->color );
		GadgetListBoxSetEnabledSelectedItemBorderColor( root, info->borderColor );
		info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetEnabledSelectedItemImageRight( root, info->image );
		info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_CENTER );
		GadgetListBoxSetEnabledSelectedItemImageCenter( root, info->image );
		info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetEnabledSelectedItemImageSmallCenter( root, info->image );

		info = GetStateInfo( LISTBOX_DISABLED );
		GadgetListBoxSetDisabledImage( root, info->image );
		GadgetListBoxSetDisabledColor( root, info->color );
		GadgetListBoxSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_LEFT );
		GadgetListBoxSetDisabledSelectedItemImageLeft( root, info->image );
		GadgetListBoxSetDisabledSelectedItemColor( root, info->color );
		GadgetListBoxSetDisabledSelectedItemBorderColor( root, info->borderColor );
		info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetDisabledSelectedItemImageRight( root, info->image );
		info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_CENTER );
		GadgetListBoxSetDisabledSelectedItemImageCenter( root, info->image );
		info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetDisabledSelectedItemImageSmallCenter( root, info->image );

		info = GetStateInfo( LISTBOX_HILITE );
		GadgetListBoxSetHiliteImage( root, info->image );
		GadgetListBoxSetHiliteColor( root, info->color );
		GadgetListBoxSetHiliteBorderColor( root, info->borderColor );
		info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_LEFT );
		GadgetListBoxSetHiliteSelectedItemImageLeft( root, info->image );
		GadgetListBoxSetHiliteSelectedItemColor( root, info->color );
		GadgetListBoxSetHiliteSelectedItemBorderColor( root, info->borderColor );
		info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetHiliteSelectedItemImageRight( root, info->image );
		info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_CENTER );
		GadgetListBoxSetHiliteSelectedItemImageCenter( root, info->image );
		info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetHiliteSelectedItemImageSmallCenter( root, info->image );

		GameWindow *upButton = GadgetListBoxGetUpButton( root );
		if( upButton )
		{

			info = GetStateInfo( LISTBOX_UP_BUTTON_ENABLED );
			GadgetButtonSetEnabledImage( upButton, info->image );
			GadgetButtonSetEnabledColor( upButton, info->color );
			GadgetButtonSetEnabledBorderColor( upButton, info->borderColor );
			info = GetStateInfo( LISTBOX_UP_BUTTON_ENABLED_PUSHED );
			GadgetButtonSetEnabledSelectedImage( upButton, info->image );
			GadgetButtonSetEnabledSelectedColor( upButton, info->color );
			GadgetButtonSetEnabledSelectedBorderColor( upButton, info->borderColor );

			info = GetStateInfo( LISTBOX_UP_BUTTON_DISABLED );
			GadgetButtonSetDisabledImage( upButton, info->image );
			GadgetButtonSetDisabledColor( upButton, info->color );
			GadgetButtonSetDisabledBorderColor( upButton, info->borderColor );
			info = GetStateInfo( LISTBOX_UP_BUTTON_DISABLED_PUSHED );
			GadgetButtonSetDisabledSelectedImage( upButton, info->image );
			GadgetButtonSetDisabledSelectedColor( upButton, info->color );
			GadgetButtonSetDisabledSelectedBorderColor( upButton, info->borderColor );

			info = GetStateInfo( LISTBOX_UP_BUTTON_HILITE );
			GadgetButtonSetHiliteImage( upButton, info->image );
			GadgetButtonSetHiliteColor( upButton, info->color );
			GadgetButtonSetHiliteBorderColor( upButton, info->borderColor );
			info = GetStateInfo( LISTBOX_UP_BUTTON_HILITE_PUSHED );
			GadgetButtonSetHiliteSelectedImage( upButton, info->image );
			GadgetButtonSetHiliteSelectedColor( upButton, info->color );
			GadgetButtonSetHiliteSelectedBorderColor( upButton, info->borderColor );

		}  // end if

		GameWindow *downButton = GadgetListBoxGetDownButton( root );
		if( downButton )
		{

			info = GetStateInfo( LISTBOX_DOWN_BUTTON_ENABLED );
			GadgetButtonSetEnabledImage( downButton, info->image );
			GadgetButtonSetEnabledColor( downButton, info->color );
			GadgetButtonSetEnabledBorderColor( downButton, info->borderColor );
			info = GetStateInfo( LISTBOX_DOWN_BUTTON_ENABLED_PUSHED );
			GadgetButtonSetEnabledSelectedImage( downButton, info->image );
			GadgetButtonSetEnabledSelectedColor( downButton, info->color );
			GadgetButtonSetEnabledSelectedBorderColor( downButton, info->borderColor );

			info = GetStateInfo( LISTBOX_DOWN_BUTTON_DISABLED );
			GadgetButtonSetDisabledImage( downButton, info->image );
			GadgetButtonSetDisabledColor( downButton, info->color );
			GadgetButtonSetDisabledBorderColor( downButton, info->borderColor );
			info = GetStateInfo( LISTBOX_DOWN_BUTTON_DISABLED_PUSHED );
			GadgetButtonSetDisabledSelectedImage( downButton, info->image );
			GadgetButtonSetDisabledSelectedColor( downButton, info->color );
			GadgetButtonSetDisabledSelectedBorderColor( downButton, info->borderColor );

			info = GetStateInfo( LISTBOX_DOWN_BUTTON_HILITE );
			GadgetButtonSetHiliteImage( downButton, info->image );
			GadgetButtonSetHiliteColor( downButton, info->color );
			GadgetButtonSetHiliteBorderColor( downButton, info->borderColor );
			info = GetStateInfo( LISTBOX_DOWN_BUTTON_HILITE_PUSHED );
			GadgetButtonSetHiliteSelectedImage( downButton, info->image );
			GadgetButtonSetHiliteSelectedColor( downButton, info->color );
			GadgetButtonSetHiliteSelectedBorderColor( downButton, info->borderColor );

		}  // end if

		GameWindow *slider = GadgetListBoxGetSlider( root );
		if( slider )
		{

			info = GetStateInfo( LISTBOX_SLIDER_ENABLED_TOP );
			GadgetSliderSetEnabledImageTop( slider, info->image );
			GadgetSliderSetEnabledColor( slider, info->color );
			GadgetSliderSetEnabledBorderColor( slider, info->borderColor );
			info = GetStateInfo( LISTBOX_SLIDER_ENABLED_BOTTOM );
			GadgetSliderSetEnabledImageBottom( slider, info->image );
			info = GetStateInfo( LISTBOX_SLIDER_ENABLED_CENTER );
			GadgetSliderSetEnabledImageCenter( slider, info->image );
			info = GetStateInfo( LISTBOX_SLIDER_ENABLED_SMALL_CENTER );
			GadgetSliderSetEnabledImageSmallCenter( slider, info->image );

			info = GetStateInfo( LISTBOX_SLIDER_DISABLED_TOP );
			GadgetSliderSetDisabledImageTop( slider, info->image );
			GadgetSliderSetDisabledColor( slider, info->color );
			GadgetSliderSetDisabledBorderColor( slider, info->borderColor );
			info = GetStateInfo( LISTBOX_SLIDER_DISABLED_BOTTOM );
			GadgetSliderSetDisabledImageBottom( slider, info->image );
			info = GetStateInfo( LISTBOX_SLIDER_DISABLED_CENTER );
			GadgetSliderSetDisabledImageCenter( slider, info->image );
			info = GetStateInfo( LISTBOX_SLIDER_DISABLED_SMALL_CENTER );
			GadgetSliderSetDisabledImageSmallCenter( slider, info->image );

			info = GetStateInfo( LISTBOX_SLIDER_HILITE_TOP );
			GadgetSliderSetDisabledImageTop( slider, info->image );
			GadgetSliderSetDisabledColor( slider, info->color );
			GadgetSliderSetDisabledBorderColor( slider, info->borderColor );
			info = GetStateInfo( LISTBOX_SLIDER_HILITE_BOTTOM );
			GadgetSliderSetDisabledImageBottom( slider, info->image );
			info = GetStateInfo( LISTBOX_SLIDER_HILITE_CENTER );
			GadgetSliderSetDisabledImageCenter( slider, info->image );
			info = GetStateInfo( LISTBOX_SLIDER_HILITE_SMALL_CENTER );
			GadgetSliderSetDisabledImageSmallCenter( slider, info->image );

			info = GetStateInfo( LISTBOX_SLIDER_THUMB_ENABLED );
			GadgetSliderSetEnabledThumbImage( slider, info->image );
			GadgetSliderSetEnabledThumbColor( slider, info->color );
			GadgetSliderSetEnabledThumbBorderColor( slider, info->borderColor );
			info = GetStateInfo( LISTBOX_SLIDER_THUMB_ENABLED_PUSHED );
			GadgetSliderSetEnabledSelectedThumbImage( slider, info->image );
			GadgetSliderSetEnabledSelectedThumbColor( slider, info->color );
			GadgetSliderSetEnabledSelectedThumbBorderColor( slider, info->borderColor );

			info = GetStateInfo( LISTBOX_SLIDER_THUMB_DISABLED );
			GadgetSliderSetDisabledThumbImage( slider, info->image );
			GadgetSliderSetDisabledThumbColor( slider, info->color );
			GadgetSliderSetDisabledThumbBorderColor( slider, info->borderColor );
			info = GetStateInfo( LISTBOX_SLIDER_THUMB_DISABLED_PUSHED );
			GadgetSliderSetDisabledSelectedThumbImage( slider, info->image );
			GadgetSliderSetDisabledSelectedThumbColor( slider, info->color );
			GadgetSliderSetDisabledSelectedThumbBorderColor( slider, info->borderColor );

			info = GetStateInfo( LISTBOX_SLIDER_THUMB_HILITE );
			GadgetSliderSetHiliteThumbImage( slider, info->image );
			GadgetSliderSetHiliteThumbColor( slider, info->color );
			GadgetSliderSetHiliteThumbBorderColor( slider, info->borderColor );
			info = GetStateInfo( LISTBOX_SLIDER_THUMB_HILITE_PUSHED );
			GadgetSliderSetHiliteSelectedThumbImage( slider, info->image );
			GadgetSliderSetHiliteSelectedThumbColor( slider, info->color );
			GadgetSliderSetHiliteSelectedThumbBorderColor( slider, info->borderColor );

		}  // end if

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_COMBO_BOX ) )
	{
		info = GetStateInfo( COMBOBOX_ENABLED );
		GadgetListBoxSetEnabledImage( root, info->image );
		GadgetListBoxSetEnabledColor( root, info->color );
		GadgetListBoxSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_LEFT );
		GadgetListBoxSetEnabledSelectedItemImageLeft( root, info->image );
		GadgetListBoxSetEnabledSelectedItemColor( root, info->color );
		GadgetListBoxSetEnabledSelectedItemBorderColor( root, info->borderColor );
		info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetEnabledSelectedItemImageRight( root, info->image );
		info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_CENTER );
		GadgetListBoxSetEnabledSelectedItemImageCenter( root, info->image );
		info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetEnabledSelectedItemImageSmallCenter( root, info->image );

		info = GetStateInfo( COMBOBOX_DISABLED );
		GadgetListBoxSetDisabledImage( root, info->image );
		GadgetListBoxSetDisabledColor( root, info->color );
		GadgetListBoxSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_LEFT );
		GadgetListBoxSetDisabledSelectedItemImageLeft( root, info->image );
		GadgetListBoxSetDisabledSelectedItemColor( root, info->color );
		GadgetListBoxSetDisabledSelectedItemBorderColor( root, info->borderColor );
		info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetDisabledSelectedItemImageRight( root, info->image );
		info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_CENTER );
		GadgetListBoxSetDisabledSelectedItemImageCenter( root, info->image );
		info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetDisabledSelectedItemImageSmallCenter( root, info->image );

		info = GetStateInfo( COMBOBOX_HILITE );
		GadgetListBoxSetHiliteImage( root, info->image );
		GadgetListBoxSetHiliteColor( root, info->color );
		GadgetListBoxSetHiliteBorderColor( root, info->borderColor );
		info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_LEFT );
		GadgetListBoxSetHiliteSelectedItemImageLeft( root, info->image );
		GadgetListBoxSetHiliteSelectedItemColor( root, info->color );
		GadgetListBoxSetHiliteSelectedItemBorderColor( root, info->borderColor );
		info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetHiliteSelectedItemImageRight( root, info->image );
		info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_CENTER );
		GadgetListBoxSetHiliteSelectedItemImageCenter( root, info->image );
		info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetHiliteSelectedItemImageSmallCenter( root, info->image );

		GameWindow *dropDownButton = GadgetComboBoxGetDropDownButton( root );
		if( dropDownButton )
		{
			info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_ENABLED );
			GadgetButtonSetEnabledImage( dropDownButton, info->image );
			GadgetButtonSetEnabledColor( dropDownButton, info->color );
			GadgetButtonSetEnabledBorderColor( dropDownButton, info->borderColor );
			info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED );
			GadgetButtonSetEnabledSelectedImage( dropDownButton, info->image );
			GadgetButtonSetEnabledSelectedColor( dropDownButton, info->color );
			GadgetButtonSetEnabledSelectedBorderColor( dropDownButton, info->borderColor );

			info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_DISABLED );
			GadgetButtonSetDisabledImage( dropDownButton, info->image );
			GadgetButtonSetDisabledColor( dropDownButton, info->color );
			GadgetButtonSetDisabledBorderColor( dropDownButton, info->borderColor );
			info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED );
			GadgetButtonSetDisabledSelectedImage( dropDownButton, info->image );
			GadgetButtonSetDisabledSelectedColor( dropDownButton, info->color );
			GadgetButtonSetDisabledSelectedBorderColor( dropDownButton, info->borderColor );

			info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_HILITE );
			GadgetButtonSetHiliteImage( dropDownButton, info->image );
			GadgetButtonSetHiliteColor( dropDownButton, info->color );
			GadgetButtonSetHiliteBorderColor( dropDownButton, info->borderColor );
			info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED );
			GadgetButtonSetHiliteSelectedImage( dropDownButton, info->image );
			GadgetButtonSetHiliteSelectedColor( dropDownButton, info->color );
			GadgetButtonSetHiliteSelectedBorderColor( dropDownButton, info->borderColor );
		}// end if
		
		GameWindow *editBox = GadgetComboBoxGetEditBox( root );
		if ( editBox )
		{
			info = GetStateInfo( COMBOBOX_EDIT_BOX_ENABLED_LEFT );
			GadgetTextEntrySetEnabledImageLeft( editBox, info->image );
			GadgetTextEntrySetEnabledColor( editBox, info->color );
			GadgetTextEntrySetEnabledBorderColor( editBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_ENABLED_RIGHT );
			GadgetTextEntrySetEnabledImageRight( editBox, info->image );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_ENABLED_CENTER );
			GadgetTextEntrySetEnabledImageCenter( editBox, info->image );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_ENABLED_SMALL_CENTER );
			GadgetTextEntrySetEnabledImageSmallCenter( editBox, info->image );

			info = GetStateInfo( COMBOBOX_EDIT_BOX_DISABLED_LEFT );
			GadgetTextEntrySetDisabledImageLeft( editBox, info->image );
			GadgetTextEntrySetDisabledColor( editBox, info->color );
			GadgetTextEntrySetDisabledBorderColor( editBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_DISABLED_RIGHT );
			GadgetTextEntrySetDisabledImageRight( editBox, info->image );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_DISABLED_CENTER );
			GadgetTextEntrySetDisabledImageCenter( editBox, info->image );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_DISABLED_SMALL_CENTER );
			GadgetTextEntrySetDisabledImageSmallCenter( editBox, info->image );

			info = GetStateInfo( COMBOBOX_EDIT_BOX_HILITE_LEFT );
			GadgetTextEntrySetHiliteImageLeft( editBox, info->image );
			GadgetTextEntrySetHiliteColor( editBox, info->color );
			GadgetTextEntrySetHiliteBorderColor( editBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_HILITE_RIGHT );
			GadgetTextEntrySetHiliteImageRight( editBox, info->image );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_HILITE_CENTER );
			GadgetTextEntrySetHiliteImageCenter( editBox, info->image );
			info = GetStateInfo( COMBOBOX_EDIT_BOX_HILITE_SMALL_CENTER );
			GadgetTextEntrySetHiliteImageSmallCenter( editBox, info->image );
		} // end if

		GameWindow *listBox = GadgetComboBoxGetListBox( root );
		if( listBox )
		{
			info = GetStateInfo( COMBOBOX_LISTBOX_ENABLED );
			GadgetListBoxSetEnabledImage( listBox, info->image );
			GadgetListBoxSetEnabledColor( listBox, info->color );
			GadgetListBoxSetEnabledBorderColor( listBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT );
			GadgetListBoxSetEnabledSelectedItemImageLeft( listBox, info->image );
			GadgetListBoxSetEnabledSelectedItemColor( listBox, info->color );
			GadgetListBoxSetEnabledSelectedItemBorderColor( listBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_RIGHT );
			GadgetListBoxSetEnabledSelectedItemImageRight( listBox, info->image );
			info = GetStateInfo( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_CENTER );
			GadgetListBoxSetEnabledSelectedItemImageCenter( listBox, info->image );
			info = GetStateInfo( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
			GadgetListBoxSetEnabledSelectedItemImageSmallCenter( listBox, info->image );

			info = GetStateInfo( COMBOBOX_LISTBOX_DISABLED );
			GadgetListBoxSetDisabledImage( listBox, info->image );
			GadgetListBoxSetDisabledColor( listBox, info->color );
			GadgetListBoxSetDisabledBorderColor( listBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_LEFT );
			GadgetListBoxSetDisabledSelectedItemImageLeft( listBox, info->image );
			GadgetListBoxSetDisabledSelectedItemColor( listBox, info->color );
			GadgetListBoxSetDisabledSelectedItemBorderColor( listBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_RIGHT );
			GadgetListBoxSetDisabledSelectedItemImageRight( listBox, info->image );
			info = GetStateInfo( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_CENTER );
			GadgetListBoxSetDisabledSelectedItemImageCenter( listBox, info->image );
			info = GetStateInfo( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
			GadgetListBoxSetDisabledSelectedItemImageSmallCenter( listBox, info->image );

			info = GetStateInfo( COMBOBOX_LISTBOX_HILITE );
			GadgetListBoxSetHiliteImage( listBox, info->image );
			GadgetListBoxSetHiliteColor( listBox, info->color );
			GadgetListBoxSetHiliteBorderColor( listBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_LEFT );
			GadgetListBoxSetHiliteSelectedItemImageLeft( listBox, info->image );
			GadgetListBoxSetHiliteSelectedItemColor( listBox, info->color );
			GadgetListBoxSetHiliteSelectedItemBorderColor( listBox, info->borderColor );
			info = GetStateInfo( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_RIGHT );
			GadgetListBoxSetHiliteSelectedItemImageRight( listBox, info->image );
			info = GetStateInfo( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_CENTER );
			GadgetListBoxSetHiliteSelectedItemImageCenter( listBox, info->image );
			info = GetStateInfo( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
			GadgetListBoxSetHiliteSelectedItemImageSmallCenter( listBox, info->image );

			GameWindow *upButton = GadgetListBoxGetUpButton( listBox );
			if( upButton )
			{

				info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED );
				GadgetButtonSetEnabledImage( upButton, info->image );
				GadgetButtonSetEnabledColor( upButton, info->color );
				GadgetButtonSetEnabledBorderColor( upButton, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED );
				GadgetButtonSetEnabledSelectedImage( upButton, info->image );
				GadgetButtonSetEnabledSelectedColor( upButton, info->color );
				GadgetButtonSetEnabledSelectedBorderColor( upButton, info->borderColor );

				info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED );
				GadgetButtonSetDisabledImage( upButton, info->image );
				GadgetButtonSetDisabledColor( upButton, info->color );
				GadgetButtonSetDisabledBorderColor( upButton, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED );
				GadgetButtonSetDisabledSelectedImage( upButton, info->image );
				GadgetButtonSetDisabledSelectedColor( upButton, info->color );
				GadgetButtonSetDisabledSelectedBorderColor( upButton, info->borderColor );

				info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_HILITE );
				GadgetButtonSetHiliteImage( upButton, info->image );
				GadgetButtonSetHiliteColor( upButton, info->color );
				GadgetButtonSetHiliteBorderColor( upButton, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED );
				GadgetButtonSetHiliteSelectedImage( upButton, info->image );
				GadgetButtonSetHiliteSelectedColor( upButton, info->color );
				GadgetButtonSetHiliteSelectedBorderColor( upButton, info->borderColor );

			}  // end if

			GameWindow *downButton = GadgetListBoxGetDownButton( listBox );
			if( downButton )
			{

				info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED );
				GadgetButtonSetEnabledImage( downButton, info->image );
				GadgetButtonSetEnabledColor( downButton, info->color );
				GadgetButtonSetEnabledBorderColor( downButton, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED );
				GadgetButtonSetEnabledSelectedImage( downButton, info->image );
				GadgetButtonSetEnabledSelectedColor( downButton, info->color );
				GadgetButtonSetEnabledSelectedBorderColor( downButton, info->borderColor );

				info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED );
				GadgetButtonSetDisabledImage( downButton, info->image );
				GadgetButtonSetDisabledColor( downButton, info->color );
				GadgetButtonSetDisabledBorderColor( downButton, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED );
				GadgetButtonSetDisabledSelectedImage( downButton, info->image );
				GadgetButtonSetDisabledSelectedColor( downButton, info->color );
				GadgetButtonSetDisabledSelectedBorderColor( downButton, info->borderColor );

				info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE );
				GadgetButtonSetHiliteImage( downButton, info->image );
				GadgetButtonSetHiliteColor( downButton, info->color );
				GadgetButtonSetHiliteBorderColor( downButton, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED );
				GadgetButtonSetHiliteSelectedImage( downButton, info->image );
				GadgetButtonSetHiliteSelectedColor( downButton, info->color );
				GadgetButtonSetHiliteSelectedBorderColor( downButton, info->borderColor );

			}  // end if

			GameWindow *slider = GadgetListBoxGetSlider( listBox );
			if( slider )
			{

				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_ENABLED_TOP );
				GadgetSliderSetEnabledImageTop( slider, info->image );
				GadgetSliderSetEnabledColor( slider, info->color );
				GadgetSliderSetEnabledBorderColor( slider, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_ENABLED_BOTTOM );
				GadgetSliderSetEnabledImageBottom( slider, info->image );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_ENABLED_CENTER );
				GadgetSliderSetEnabledImageCenter( slider, info->image );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_ENABLED_SMALL_CENTER );
				GadgetSliderSetEnabledImageSmallCenter( slider, info->image );

				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_DISABLED_TOP );
				GadgetSliderSetDisabledImageTop( slider, info->image );
				GadgetSliderSetDisabledColor( slider, info->color );
				GadgetSliderSetDisabledBorderColor( slider, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_DISABLED_BOTTOM );
				GadgetSliderSetDisabledImageBottom( slider, info->image );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_DISABLED_CENTER );
				GadgetSliderSetDisabledImageCenter( slider, info->image );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_DISABLED_SMALL_CENTER );
				GadgetSliderSetDisabledImageSmallCenter( slider, info->image );

				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_TOP );
				GadgetSliderSetDisabledImageTop( slider, info->image );
				GadgetSliderSetDisabledColor( slider, info->color );
				GadgetSliderSetDisabledBorderColor( slider, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_BOTTOM );
				GadgetSliderSetDisabledImageBottom( slider, info->image );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_CENTER );
				GadgetSliderSetDisabledImageCenter( slider, info->image );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_SMALL_CENTER );
				GadgetSliderSetDisabledImageSmallCenter( slider, info->image );

				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED );
				GadgetSliderSetEnabledThumbImage( slider, info->image );
				GadgetSliderSetEnabledThumbColor( slider, info->color );
				GadgetSliderSetEnabledThumbBorderColor( slider, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED );
				GadgetSliderSetEnabledSelectedThumbImage( slider, info->image );
				GadgetSliderSetEnabledSelectedThumbColor( slider, info->color );
				GadgetSliderSetEnabledSelectedThumbBorderColor( slider, info->borderColor );

				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED );
				GadgetSliderSetDisabledThumbImage( slider, info->image );
				GadgetSliderSetDisabledThumbColor( slider, info->color );
				GadgetSliderSetDisabledThumbBorderColor( slider, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED );
				GadgetSliderSetDisabledSelectedThumbImage( slider, info->image );
				GadgetSliderSetDisabledSelectedThumbColor( slider, info->color );
				GadgetSliderSetDisabledSelectedThumbBorderColor( slider, info->borderColor );

				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE );
				GadgetSliderSetHiliteThumbImage( slider, info->image );
				GadgetSliderSetHiliteThumbColor( slider, info->color );
				GadgetSliderSetHiliteThumbBorderColor( slider, info->borderColor );
				info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED );
				GadgetSliderSetHiliteSelectedThumbImage( slider, info->image );
				GadgetSliderSetHiliteSelectedThumbColor( slider, info->color );
				GadgetSliderSetHiliteSelectedThumbBorderColor( slider, info->borderColor );

			}  // end if
		}// end if (combo listbox)
	}// end if
	else if( BitIsSet( root->winGetStyle(), GWS_ENTRY_FIELD ) )
	{

		info = GetStateInfo( TEXT_ENTRY_ENABLED_LEFT );
		GadgetTextEntrySetEnabledImageLeft( root, info->image );
		GadgetTextEntrySetEnabledColor( root, info->color );
		GadgetTextEntrySetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( TEXT_ENTRY_ENABLED_RIGHT );
		GadgetTextEntrySetEnabledImageRight( root, info->image );
		info = GetStateInfo( TEXT_ENTRY_ENABLED_CENTER );
		GadgetTextEntrySetEnabledImageCenter( root, info->image );
		info = GetStateInfo( TEXT_ENTRY_ENABLED_SMALL_CENTER );
		GadgetTextEntrySetEnabledImageSmallCenter( root, info->image );

		info = GetStateInfo( TEXT_ENTRY_DISABLED_LEFT );
		GadgetTextEntrySetDisabledImageLeft( root, info->image );
		GadgetTextEntrySetDisabledColor( root, info->color );
		GadgetTextEntrySetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( TEXT_ENTRY_DISABLED_RIGHT );
		GadgetTextEntrySetDisabledImageRight( root, info->image );
		info = GetStateInfo( TEXT_ENTRY_DISABLED_CENTER );
		GadgetTextEntrySetDisabledImageCenter( root, info->image );
		info = GetStateInfo( TEXT_ENTRY_DISABLED_SMALL_CENTER );
		GadgetTextEntrySetDisabledImageSmallCenter( root, info->image );

		info = GetStateInfo( TEXT_ENTRY_HILITE_LEFT );
		GadgetTextEntrySetHiliteImageLeft( root, info->image );
		GadgetTextEntrySetHiliteColor( root, info->color );
		GadgetTextEntrySetHiliteBorderColor( root, info->borderColor );
		info = GetStateInfo( TEXT_ENTRY_HILITE_RIGHT );
		GadgetTextEntrySetHiliteImageRight( root, info->image );
		info = GetStateInfo( TEXT_ENTRY_HILITE_CENTER );
		GadgetTextEntrySetHiliteImageCenter( root, info->image );
		info = GetStateInfo( TEXT_ENTRY_HILITE_SMALL_CENTER );
		GadgetTextEntrySetHiliteImageSmallCenter( root, info->image );

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_STATIC_TEXT ) )
	{

		info = GetStateInfo( STATIC_TEXT_ENABLED );
		GadgetStaticTextSetEnabledImage( root, info->image );
		GadgetStaticTextSetEnabledColor( root, info->color );
		GadgetStaticTextSetEnabledBorderColor( root, info->borderColor );

		info = GetStateInfo( STATIC_TEXT_DISABLED );
		GadgetStaticTextSetDisabledImage( root, info->image );
		GadgetStaticTextSetDisabledColor( root, info->color );
		GadgetStaticTextSetDisabledBorderColor( root, info->borderColor );

		info = GetStateInfo( STATIC_TEXT_HILITE );
		GadgetStaticTextSetHiliteImage( root, info->image );
		GadgetStaticTextSetHiliteColor( root, info->color );
		GadgetStaticTextSetHiliteBorderColor( root, info->borderColor );

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_PROGRESS_BAR ) )
	{

		info = GetStateInfo( PROGRESS_BAR_ENABLED_LEFT );
		GadgetProgressBarSetEnabledImageLeft( root, info->image );
		GadgetProgressBarSetEnabledColor( root, info->color );
		GadgetProgressBarSetEnabledBorderColor( root, info->borderColor );
		info = GetStateInfo( PROGRESS_BAR_ENABLED_RIGHT );
		GadgetProgressBarSetEnabledImageRight( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_ENABLED_CENTER );
		GadgetProgressBarSetEnabledImageCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_ENABLED_SMALL_CENTER );
		GadgetProgressBarSetEnabledImageSmallCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_LEFT );
		GadgetProgressBarSetEnabledBarImageLeft( root, info->image );
		GadgetProgressBarSetEnabledBarColor( root, info->color );
		GadgetProgressBarSetEnabledBarBorderColor( root, info->borderColor );
		info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_RIGHT );
		GadgetProgressBarSetEnabledBarImageRight( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_CENTER );
		GadgetProgressBarSetEnabledBarImageCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_ENABLED_BAR_SMALL_CENTER );
		GadgetProgressBarSetEnabledBarImageSmallCenter( root, info->image );

		info = GetStateInfo( PROGRESS_BAR_DISABLED_LEFT );
		GadgetProgressBarSetDisabledImageLeft( root, info->image );
		GadgetProgressBarSetDisabledColor( root, info->color );
		GadgetProgressBarSetDisabledBorderColor( root, info->borderColor );
		info = GetStateInfo( PROGRESS_BAR_DISABLED_RIGHT );
		GadgetProgressBarSetDisabledImageRight( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_DISABLED_CENTER );
		GadgetProgressBarSetDisabledImageCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_DISABLED_SMALL_CENTER );
		GadgetProgressBarSetDisabledImageSmallCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_LEFT );
		GadgetProgressBarSetDisabledBarImageLeft( root, info->image );
		GadgetProgressBarSetDisabledBarColor( root, info->color );
		GadgetProgressBarSetDisabledBarBorderColor( root, info->borderColor );
		info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_RIGHT );
		GadgetProgressBarSetDisabledBarImageRight( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_CENTER );
		GadgetProgressBarSetDisabledBarImageCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_DISABLED_BAR_SMALL_CENTER );
		GadgetProgressBarSetDisabledBarImageSmallCenter( root, info->image );

		info = GetStateInfo( PROGRESS_BAR_HILITE_LEFT );
		GadgetProgressBarSetHiliteImageLeft( root, info->image );
		GadgetProgressBarSetHiliteColor( root, info->color );
		GadgetProgressBarSetHiliteBorderColor( root, info->borderColor );
		info = GetStateInfo( PROGRESS_BAR_HILITE_RIGHT );
		GadgetProgressBarSetHiliteImageRight( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_HILITE_CENTER );
		GadgetProgressBarSetHiliteImageCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_HILITE_SMALL_CENTER );
		GadgetProgressBarSetHiliteImageSmallCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_LEFT );
		GadgetProgressBarSetHiliteBarImageLeft( root, info->image );
		GadgetProgressBarSetHiliteBarColor( root, info->color );
		GadgetProgressBarSetHiliteBarBorderColor( root, info->borderColor );
		info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_RIGHT );
		GadgetProgressBarSetHiliteBarImageRight( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_CENTER );
		GadgetProgressBarSetHiliteBarImageCenter( root, info->image );
		info = GetStateInfo( PROGRESS_BAR_HILITE_BAR_SMALL_CENTER );
		GadgetProgressBarSetHiliteBarImageSmallCenter( root, info->image );

	}  // end else if
	else if( BitIsSet( root->winGetStyle(), GWS_TAB_CONTROL ) )
	{
		info = GetStateInfo( TC_TAB_0_ENABLED );
		GadgetTabControlSetEnabledImageTabZero( root, info->image );
		GadgetTabControlSetEnabledColorTabZero( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabZero( root, info->borderColor );

		info = GetStateInfo( TC_TAB_1_ENABLED );
		GadgetTabControlSetEnabledImageTabOne( root, info->image );
		GadgetTabControlSetEnabledColorTabOne( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabOne( root, info->borderColor );

		info = GetStateInfo( TC_TAB_2_ENABLED );
		GadgetTabControlSetEnabledImageTabTwo( root, info->image );
		GadgetTabControlSetEnabledColorTabTwo( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabTwo( root, info->borderColor );

		info = GetStateInfo( TC_TAB_3_ENABLED );
		GadgetTabControlSetEnabledImageTabThree( root, info->image );
		GadgetTabControlSetEnabledColorTabThree( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabThree( root, info->borderColor );

		info = GetStateInfo( TC_TAB_4_ENABLED );
		GadgetTabControlSetEnabledImageTabFour( root, info->image );
		GadgetTabControlSetEnabledColorTabFour( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabFour( root, info->borderColor );

		info = GetStateInfo( TC_TAB_5_ENABLED );
		GadgetTabControlSetEnabledImageTabFive( root, info->image );
		GadgetTabControlSetEnabledColorTabFive( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabFive( root, info->borderColor );

		info = GetStateInfo( TC_TAB_6_ENABLED );
		GadgetTabControlSetEnabledImageTabSix( root, info->image );
		GadgetTabControlSetEnabledColorTabSix( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabSix( root, info->borderColor );

		info = GetStateInfo( TC_TAB_7_ENABLED );
		GadgetTabControlSetEnabledImageTabSeven( root, info->image );
		GadgetTabControlSetEnabledColorTabSeven( root, info->color );
		GadgetTabControlSetEnabledBorderColorTabSeven( root, info->borderColor );

		info = GetStateInfo( TAB_CONTROL_ENABLED );
		GadgetTabControlSetEnabledImageBackground( root, info->image );
		GadgetTabControlSetEnabledColorBackground( root, info->color );
		GadgetTabControlSetEnabledBorderColorBackground( root, info->borderColor );


	
		info = GetStateInfo( TC_TAB_0_DISABLED );
		GadgetTabControlSetDisabledImageTabZero( root, info->image );
		GadgetTabControlSetDisabledColorTabZero( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabZero( root, info->borderColor );

		info = GetStateInfo( TC_TAB_1_DISABLED );
		GadgetTabControlSetDisabledImageTabOne( root, info->image );
		GadgetTabControlSetDisabledColorTabOne( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabOne( root, info->borderColor );

		info = GetStateInfo( TC_TAB_2_DISABLED );
		GadgetTabControlSetDisabledImageTabTwo( root, info->image );
		GadgetTabControlSetDisabledColorTabTwo( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabTwo( root, info->borderColor );

		info = GetStateInfo( TC_TAB_3_DISABLED );
		GadgetTabControlSetDisabledImageTabThree( root, info->image );
		GadgetTabControlSetDisabledColorTabThree( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabThree( root, info->borderColor );

		info = GetStateInfo( TC_TAB_4_DISABLED );
		GadgetTabControlSetDisabledImageTabFour( root, info->image );
		GadgetTabControlSetDisabledColorTabFour( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabFour( root, info->borderColor );

		info = GetStateInfo( TC_TAB_5_DISABLED );
		GadgetTabControlSetDisabledImageTabFive( root, info->image );
		GadgetTabControlSetDisabledColorTabFive( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabFive( root, info->borderColor );

		info = GetStateInfo( TC_TAB_6_DISABLED );
		GadgetTabControlSetDisabledImageTabSix( root, info->image );
		GadgetTabControlSetDisabledColorTabSix( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabSix( root, info->borderColor );

		info = GetStateInfo( TC_TAB_7_DISABLED );
		GadgetTabControlSetDisabledImageTabSeven( root, info->image );
		GadgetTabControlSetDisabledColorTabSeven( root, info->color );
		GadgetTabControlSetDisabledBorderColorTabSeven( root, info->borderColor );

		info = GetStateInfo( TAB_CONTROL_DISABLED );
		GadgetTabControlSetDisabledImageBackground( root, info->image );
		GadgetTabControlSetDisabledColorBackground( root, info->color );
		GadgetTabControlSetDisabledBorderColorBackground( root, info->borderColor );


		

		info = GetStateInfo( TC_TAB_0_HILITE );
		GadgetTabControlSetHiliteImageTabZero( root, info->image );
		GadgetTabControlSetHiliteColorTabZero( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabZero( root, info->borderColor );

		info = GetStateInfo( TC_TAB_1_HILITE );
		GadgetTabControlSetHiliteImageTabOne( root, info->image );
		GadgetTabControlSetHiliteColorTabOne( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabOne( root, info->borderColor );

		info = GetStateInfo( TC_TAB_2_HILITE );
		GadgetTabControlSetHiliteImageTabTwo( root, info->image );
		GadgetTabControlSetHiliteColorTabTwo( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabTwo( root, info->borderColor );

		info = GetStateInfo( TC_TAB_3_HILITE );
		GadgetTabControlSetHiliteImageTabThree( root, info->image );
		GadgetTabControlSetHiliteColorTabThree( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabThree( root, info->borderColor );

		info = GetStateInfo( TC_TAB_4_HILITE );
		GadgetTabControlSetHiliteImageTabFour( root, info->image );
		GadgetTabControlSetHiliteColorTabFour( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabFour( root, info->borderColor );

		info = GetStateInfo( TC_TAB_5_HILITE );
		GadgetTabControlSetHiliteImageTabFive( root, info->image );
		GadgetTabControlSetHiliteColorTabFive( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabFive( root, info->borderColor );

		info = GetStateInfo( TC_TAB_6_HILITE );
		GadgetTabControlSetHiliteImageTabSix( root, info->image );
		GadgetTabControlSetHiliteColorTabSix( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabSix( root, info->borderColor );

		info = GetStateInfo( TC_TAB_7_HILITE );
		GadgetTabControlSetHiliteImageTabSeven( root, info->image );
		GadgetTabControlSetHiliteColorTabSeven( root, info->color );
		GadgetTabControlSetHiliteBorderColorTabSeven( root, info->borderColor );

		info = GetStateInfo( TAB_CONTROL_HILITE );
		GadgetTabControlSetHiliteImageBackground( root, info->image );
		GadgetTabControlSetHiliteColorBackground( root, info->color );
		GadgetTabControlSetHiliteBorderColorBackground( root, info->borderColor );

	}
	else
	{

		info = GetStateInfo( GENERIC_ENABLED );
		root->winSetEnabledImage( 0, info->image );
		root->winSetEnabledColor( 0, info->color );
		root->winSetEnabledBorderColor( 0, info->borderColor );

		info = GetStateInfo( GENERIC_DISABLED );
		root->winSetDisabledImage( 0, info->image );
		root->winSetDisabledColor( 0, info->color );
		root->winSetDisabledBorderColor( 0, info->borderColor );

		info = GetStateInfo( GENERIC_HILITE );
		root->winSetHiliteImage( 0, info->image );
		root->winSetHiliteColor( 0, info->color );
		root->winSetHiliteBorderColor( 0, info->borderColor );

	}  // end else

	//
	// apply changes to children of this window, unless this 
	// window is a gadget itself, those are "atomic" units remember ;)
	//
	if( TheEditor->windowIsGadget( root ) == FALSE )
		applyPropertyTablesToWindow( root->winGetChild() );

	// on to the next window
	applyPropertyTablesToWindow( root->winGetNext() );

	// force an update of the edit window
	TheEditWindow->draw();

}  // end applyPropertyTablesToWindow

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// LayoutScheme::LayoutScheme =================================================
/** */
//=============================================================================
LayoutScheme::LayoutScheme( void )
{

	strcpy( m_schemeFilename, "Default.ls" );
	memset( &m_imageAndColorTable, 0, sizeof( ImageAndColorInfo ) * NUM_STATE_IDENTIFIERS );
	m_enabledText.color = GAME_COLOR_UNDEFINED;
	m_enabledText.borderColor = GAME_COLOR_UNDEFINED;
	m_disabledText.color = GAME_COLOR_UNDEFINED;
	m_disabledText.borderColor = GAME_COLOR_UNDEFINED;
	m_hiliteText.color = GAME_COLOR_UNDEFINED;
	m_hiliteText.borderColor = GAME_COLOR_UNDEFINED;
	m_font = NULL;

}  // end LayoutScheme

// LayoutScheme::~LayoutScheme ================================================
/** */
//=============================================================================
LayoutScheme::~LayoutScheme( void )
{
	Int i;

	// free all the state name strings
	for( i = 0; i < NUM_STATE_IDENTIFIERS; i++ )
	{

		if( m_imageAndColorTable[ i ].stateNameBuffer )
		{
			
			delete [] m_imageAndColorTable[ i ].stateNameBuffer;
			m_imageAndColorTable[ i ].stateNameBuffer = NULL;
			m_imageAndColorTable[ i ].stateName = NULL;

		}  // end if

	}  // end for i

}  // end ~LayoutScheme

// LayoutScheme::init =========================================================
/** Init */
//=============================================================================
void LayoutScheme::init( void )
{
	Int i;
	ImageAndColorInfo *info;

	// just up the default state values
	for( i = FIRST_VALID_IDENTIFIER; i < NUM_STATE_IDENTIFIERS; i++ )
	{
	
		// get info from the static table created for property editing
		info = GetStateInfo( (StateIdentifier)i );
		assert( info );	
		m_imageAndColorTable[ i ].windowType = info->windowType;
		m_imageAndColorTable[ i ].stateID = info->stateID;
		m_imageAndColorTable[ i ].image = info->image;
		m_imageAndColorTable[ i ].color = info->color;
		m_imageAndColorTable[ i ].borderColor = info->borderColor;
		m_imageAndColorTable[ i ].stateNameBuffer = new char[strlen( info->stateName ) + 1];
		m_imageAndColorTable[ i ].stateName = m_imageAndColorTable[ i ].stateNameBuffer;
		strcpy(m_imageAndColorTable[ i ].stateNameBuffer, info->stateName );

	}  // end for i

	// assign a default set of colors
	UnsignedByte alpha = 255;
	Color red					= GameMakeColor( 255,   0,   0, alpha );
	Color darkRed			= GameMakeColor( 128,   0,   0, alpha );
	Color lightRed		= GameMakeColor( 255, 128, 128, alpha );
	Color green				= GameMakeColor(   0, 255,   0, alpha );
	Color darkGreen		= GameMakeColor(   0, 128,   0, alpha );
	Color lightGreen	= GameMakeColor( 128, 255, 128, alpha );
	Color blue				= GameMakeColor(   0,   0, 255, alpha );
	Color darkBlue		= GameMakeColor(   0,   0, 128, alpha );
	Color lightBlue		= GameMakeColor( 128, 128, 255, alpha );
//	Color purple			= GameMakeColor( 255,   0, 255, alpha );
//	Color darkPurple	= GameMakeColor( 128,   0, 128, alpha );
//	Color lightPurple	= GameMakeColor( 255, 128, 255, alpha );
	Color yellow			= GameMakeColor( 255, 255,   0, alpha );
//	Color darkYellow	= GameMakeColor( 128, 128,   0, alpha );
//	Color lightYellow	= GameMakeColor( 255, 255, 128, alpha );
//	Color cyan				= GameMakeColor(   0, 255, 255, alpha );
//	Color darkCyan		= GameMakeColor(  64, 128, 128, alpha );
//	Color lightCyan		= GameMakeColor( 128, 255, 255, alpha );
	Color gray				= GameMakeColor( 128, 128, 128, alpha );
	Color darkGray		= GameMakeColor(  64,  64,  64, alpha );
	Color lightGray		= GameMakeColor( 192, 192, 192, alpha );
	Color black				= GameMakeColor(   0,   0,   0, alpha );
	Color white				= GameMakeColor( 254, 254, 254, alpha );
	const Image *image;

	// push button
	image = TheMappedImageCollection->findImageByName( AsciiString(  "PushButtonEnabled" ) );
	storeImageAndColor( BUTTON_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "PushButtonEnabledSelected" ) );
	storeImageAndColor( BUTTON_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "PushButtonDisabled" ) );
	storeImageAndColor( BUTTON_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "PushButtonDisabledSelected" ) );
	storeImageAndColor( BUTTON_DISABLED_PUSHED, image, lightGray, gray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "PushButtonHilite" ) );
	storeImageAndColor( BUTTON_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "PushButtonHiliteSelected" ) );
	storeImageAndColor( BUTTON_HILITE_PUSHED, image, yellow, white );

	// radio button
	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonEnabled" ) );
	storeImageAndColor( RADIO_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonEnabledBoxUnselected" ) );
	storeImageAndColor( RADIO_ENABLED_UNCHECKED_BOX, image, darkRed, black );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonEnabledBoxSelected" ) );
	storeImageAndColor( RADIO_ENABLED_CHECKED_BOX, image, blue, lightBlue );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonDisabled" ) );
	storeImageAndColor( RADIO_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonDisabledBoxUnselected" ) );
	storeImageAndColor( RADIO_DISABLED_UNCHECKED_BOX, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonDisabledBoxSelected" ) );
	storeImageAndColor( RADIO_DISABLED_CHECKED_BOX, image, darkGray, white );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonHilite" ) );
	storeImageAndColor( RADIO_HILITE, image, green, lightGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonHiliteBoxUnselected" ) );
	storeImageAndColor( RADIO_HILITE_UNCHECKED_BOX, image, darkGreen, lightGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "RadioButtonHiliteBoxSelected" ) );
	storeImageAndColor( RADIO_HILITE_CHECKED_BOX, image, yellow, white );

	// check box
	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxEnabled" ) );
	storeImageAndColor( CHECK_BOX_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxEnabledBoxUnselected" ) );
	storeImageAndColor( CHECK_BOX_ENABLED_UNCHECKED_BOX, image, WIN_COLOR_UNDEFINED, lightBlue );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxEnabledBoxSelected" ) );
	storeImageAndColor( CHECK_BOX_ENABLED_CHECKED_BOX, image, blue, lightBlue );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxDisabled" ) );
	storeImageAndColor( CHECK_BOX_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxDisabledBoxUnselected" ) );
	storeImageAndColor( CHECK_BOX_DISABLED_UNCHECKED_BOX, image, WIN_COLOR_UNDEFINED, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxDisabledBoxSelected" ) );
	storeImageAndColor( CHECK_BOX_DISABLED_CHECKED_BOX, image, darkGray, white );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxHilite" ) );
	storeImageAndColor( CHECK_BOX_HILITE, image, green, lightGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxHiliteBoxUnselected" ) );
	storeImageAndColor( CHECK_BOX_HILITE_UNCHECKED_BOX, image, WIN_COLOR_UNDEFINED, lightBlue );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "CheckBoxHiliteBoxSelected" ) );
	storeImageAndColor( CHECK_BOX_HILITE_CHECKED_BOX, image, yellow, white );

	// horz slider
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderEnabledLeftEnd" ) );
	storeImageAndColor( HSLIDER_ENABLED_LEFT, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderEnabledRightEnd" ) );
	storeImageAndColor( HSLIDER_ENABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderEnabledRepeatingCenter" ) );
	storeImageAndColor( HSLIDER_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderEnabledSmallRepeatingCenter" ) );
	storeImageAndColor( HSLIDER_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderDisabledLeftEnd" ) );
	storeImageAndColor( HSLIDER_DISABLED_LEFT, image, gray, darkGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderDisabledRightEnd" ) );
	storeImageAndColor( HSLIDER_DISABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderDisabledRepeatingCenter" ) );
	storeImageAndColor( HSLIDER_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderDisabledSmallRepeatingCenter" ) );
	storeImageAndColor( HSLIDER_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderHiliteLeftEnd" ) );
	storeImageAndColor( HSLIDER_HILITE_LEFT, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderHiliteRightEnd" ) );
	storeImageAndColor( HSLIDER_HILITE_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderHiliteRepeatingCenter" ) );
	storeImageAndColor( HSLIDER_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderHiliteSmallRepeatingCenter" ) );
	storeImageAndColor( HSLIDER_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderThumbEnabled" ) );
	storeImageAndColor( HSLIDER_THUMB_ENABLED, image, lightRed, red );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderThumbEnabledSelected" ) );
	storeImageAndColor( HSLIDER_THUMB_ENABLED_PUSHED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderThumbDisabled" ) );
	storeImageAndColor( HSLIDER_THUMB_DISABLED, image, darkGray, gray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderThumbDisabledSelected" ) );
	storeImageAndColor( HSLIDER_THUMB_DISABLED_PUSHED, image, black, darkGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderThumbHilite" ) );
	storeImageAndColor( HSLIDER_THUMB_HILITE, image, green, lightGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "HSliderThumbHiliteSelected" ) );
	storeImageAndColor( HSLIDER_THUMB_HILITE_PUSHED, image, blue, lightBlue );

	// vert slider
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderEnabledTopEnd" ) );
	storeImageAndColor( VSLIDER_ENABLED_TOP, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderEnabledBottomEnd" ) );
	storeImageAndColor( VSLIDER_ENABLED_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderEnabledRepeatingCenter" ) );
	storeImageAndColor( VSLIDER_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderEnabledSmallRepeatingCenter" ) );
	storeImageAndColor( VSLIDER_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderDisabledTopEnd" ) );
	storeImageAndColor( VSLIDER_DISABLED_TOP, image, gray, darkGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderDisabledBottomEnd" ) );
	storeImageAndColor( VSLIDER_DISABLED_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderDisabledRepeatingCenter" ) );
	storeImageAndColor( VSLIDER_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderDisabledSmallRepeatingCenter" ) );
	storeImageAndColor( VSLIDER_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderHiliteTopEnd" ) );
	storeImageAndColor( VSLIDER_HILITE_TOP, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderHiliteBottomEnd" ) );
	storeImageAndColor( VSLIDER_HILITE_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderHiliteRepeatingCenter" ) );
	storeImageAndColor( VSLIDER_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderHiliteSmallRepeatingCenter" ) );
	storeImageAndColor( VSLIDER_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );


	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderThumbEnabled" ) );
	storeImageAndColor( VSLIDER_THUMB_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderThumbEnabledSelected" ) );
	storeImageAndColor( VSLIDER_THUMB_ENABLED_PUSHED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderThumbDisabled" ) );
	storeImageAndColor( VSLIDER_THUMB_DISABLED, image, darkGray, gray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderThumbDisabledSelected" ) );
	storeImageAndColor( VSLIDER_THUMB_DISABLED_PUSHED, image, black, darkGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderThumbHilite" ) );
	storeImageAndColor( VSLIDER_THUMB_HILITE, image, green, lightGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderThumbHiliteSelected" ) );
	storeImageAndColor( VSLIDER_THUMB_HILITE_PUSHED, image, blue, lightBlue );

	// listbox
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxEnabled" ) );
	storeImageAndColor( LISTBOX_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxEnabledSelectedItemLeftEnd" ) );
	storeImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_LEFT, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxEnabledSelectedItemRightEnd" ) );
	storeImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxEnabledSelectedItemRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxEnabledSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxDisabled" ) );
	storeImageAndColor( LISTBOX_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxDisabledSelectedItemLeftEnd" ) );
	storeImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_LEFT, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxDisabledSelectedItemRightEnd" ) );
	storeImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxDisabledSelectedItemRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxDisabledSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxHilite" ) );
	storeImageAndColor( LISTBOX_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxHiliteSelectedItemLeftEnd" ) );
	storeImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_LEFT, image, white, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxHiliteSelectedItemRightEnd" ) );
	storeImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxHiliteSelectedItemRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxHiliteSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeUpButtonEnabled" ) );
	storeImageAndColor( LISTBOX_UP_BUTTON_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeUpButtonEnabled" ) );
	storeImageAndColor( LISTBOX_UP_BUTTON_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeUpButtonDisabled" ) );
	storeImageAndColor( LISTBOX_UP_BUTTON_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeUpButtonDisabled" ) );
	storeImageAndColor( LISTBOX_UP_BUTTON_DISABLED_PUSHED, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeUpButtonHilite" ) );
	storeImageAndColor( LISTBOX_UP_BUTTON_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeUpButtonHiliteSelected" ) );
	storeImageAndColor( LISTBOX_UP_BUTTON_HILITE_PUSHED, image, white, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDownButtonEnabled" ) );
	storeImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDownButtonEnabled" ) );
	storeImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDownButtonDisabled" ) );
	storeImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDownButtonDisabled" ) );
	storeImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED_PUSHED, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDownButtonHilite" ) );
	storeImageAndColor( LISTBOX_DOWN_BUTTON_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDownButtonHiliteSelected" ) );
	storeImageAndColor( LISTBOX_DOWN_BUTTON_HILITE_PUSHED, image, white, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeEnabledTopEnd" ) );
	storeImageAndColor( LISTBOX_SLIDER_ENABLED_TOP, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeEnabledBottomEnd" ) );
	storeImageAndColor( LISTBOX_SLIDER_ENABLED_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeEnabledRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_SLIDER_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeEnabledSmallRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_SLIDER_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDisabledTopEnd" ) );
	storeImageAndColor( LISTBOX_SLIDER_DISABLED_TOP, image, gray, darkGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDisabledBottomEnd" ) );
	storeImageAndColor( LISTBOX_SLIDER_DISABLED_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDisabledRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_SLIDER_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeDisabledSmallRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_SLIDER_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeHiliteTopEnd" ) );
	storeImageAndColor( LISTBOX_SLIDER_HILITE_TOP, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeHiliteBottomEnd" ) );
	storeImageAndColor( LISTBOX_SLIDER_HILITE_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeHiliteRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_SLIDER_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeHiliteSmallRepeatingCenter" ) );
	storeImageAndColor( LISTBOX_SLIDER_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeThumbEnabled" ) );
	storeImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeThumbEnabled" ) );
	storeImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeThumbDisabled" ) );
	storeImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeThumbDisabled" ) );
	storeImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeThumbHilite" ) );
	storeImageAndColor( LISTBOX_SLIDER_THUMB_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "VSliderLargeThumbHiliteSelected" ) );
	storeImageAndColor( LISTBOX_SLIDER_THUMB_HILITE_PUSHED, image, white, darkGreen );

	// Combo Box 
	//---------------------------------------------------------------------------
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabled" ) );
	storeImageAndColor( COMBOBOX_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ListBoxEnabledSelectedItemLeftEnd" ) );
	storeImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_LEFT, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabledSelectedItemRightEnd" ) );
	storeImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabledSelectedItemRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabledSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabled" ) );
	storeImageAndColor( COMBOBOX_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemLeftEnd" ) );
	storeImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_LEFT, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemRightEnd" ) );
	storeImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHilite" ) );
	storeImageAndColor( COMBOBOX_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemLeftEnd" ) );
	storeImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_LEFT, image, white, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemRightEnd" ) );
	storeImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonEnabled" ) );
	storeImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonEnabled" ) );
	storeImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonDisabled" ) );
	storeImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonDisabled" ) );
	storeImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED, image, lightGray, gray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonHilite" ) );
	storeImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonHilite" ) );
	storeImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED, image, yellow, white );

	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryEnabledLeftEnd" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_LEFT, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryEnabledRightEnd" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryEnabledRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryEnabledSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryDisabledLeftEnd" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_LEFT, image, gray, black );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryDisabledRightEnd" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryDisabledRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryDisabledSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryHiliteLeftEnd" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_HILITE_LEFT, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryHiliteRightEnd" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_HILITE_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryHiliteRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "TextEntryHiliteSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_EDIT_BOX_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabledSelectedItemLeftEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabledSelectedItemRightEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabledSelectedItemRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxEnabledSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemLeftEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_LEFT, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemRightEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxDisabledSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHilite" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemLeftEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_LEFT, image, white, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemRightEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "ListBoxHiliteSelectedItemSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeUpButtonEnabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeUpButtonEnabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeUpButtonDisabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeUpButtonDisabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeUpButtonHilite" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeUpButtonHiliteSelected" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED, image, white, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonEnabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonEnabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonDisabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonDisabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonHilite" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDownButtonHiliteSelected" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED, image, white, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeEnabledTopEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_TOP, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeEnabledBottomEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeEnabledRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeEnabledSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDisabledTopEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_TOP, image, gray, darkGray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDisabledBottomEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDisabledRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeDisabledSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeHiliteTopEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_TOP, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeHiliteBottomEnd" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_BOTTOM, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeHiliteRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeHiliteSmallRepeatingCenter" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeThumbEnabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeThumbEnabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeThumbDisabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeThumbDisabled" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeThumbHilite" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString( "VSliderLargeThumbHiliteSelected" ) );
	storeImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED, image, white, darkGreen );


	// progress bar
	//---------------------------------------------------------------------------
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledLeftEnd" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_LEFT, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledRightEnd" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledSmallRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledBarLeftEnd" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_BAR_LEFT, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledBarRightEnd" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_BAR_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledBarRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_BAR_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarEnabledBarSmallRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_ENABLED_BAR_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	//---------------------------------------------------------------------------
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledLeftEnd" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_LEFT, image, darkGray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledRightEnd" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledSmallRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledBarLeftEnd" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_BAR_LEFT, image, lightGray, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledBarRightEnd" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_BAR_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledBarRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_BAR_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarDisabledBarSmallRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_DISABLED_BAR_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	//---------------------------------------------------------------------------
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteLeftEnd" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_LEFT, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteRightEnd" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteSmallRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteBarLeftEnd" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_BAR_LEFT, image, yellow, white );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteBarRightEnd" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_BAR_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteBarRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_BAR_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "ProgressBarHiliteBarSmallRepeatingCenter" ) );
	storeImageAndColor( PROGRESS_BAR_HILITE_BAR_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// static text
	//---------------------------------------------------------------------------
	image = TheMappedImageCollection->findImageByName( AsciiString(  "StaticTextEnabled" ) );
	storeImageAndColor( STATIC_TEXT_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "StaticTextDisabled" ) );
	storeImageAndColor( STATIC_TEXT_DISABLED, image, darkGray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "StaticTextHilite" ) );
	storeImageAndColor( STATIC_TEXT_HILITE, image, darkGreen, lightGreen );

	// text entry
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryEnabledLeftEnd" ) );
	storeImageAndColor( TEXT_ENTRY_ENABLED_LEFT, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryEnabledRightEnd" ) );
	storeImageAndColor( TEXT_ENTRY_ENABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryEnabledRepeatingCenter" ) );
	storeImageAndColor( TEXT_ENTRY_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryEnabledSmallRepeatingCenter" ) );
	storeImageAndColor( TEXT_ENTRY_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryDisabledLeftEnd" ) );
	storeImageAndColor( TEXT_ENTRY_DISABLED_LEFT, image, gray, black );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryDisabledRightEnd" ) );
	storeImageAndColor( TEXT_ENTRY_DISABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryDisabledRepeatingCenter" ) );
	storeImageAndColor( TEXT_ENTRY_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryDisabledSmallRepeatingCenter" ) );
	storeImageAndColor( TEXT_ENTRY_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryHiliteLeftEnd" ) );
	storeImageAndColor( TEXT_ENTRY_HILITE_LEFT, image, green, darkGreen );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryHiliteRightEnd" ) );
	storeImageAndColor( TEXT_ENTRY_HILITE_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryHiliteRepeatingCenter" ) );
	storeImageAndColor( TEXT_ENTRY_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TextEntryHiliteSmallRepeatingCenter" ) );
	storeImageAndColor( TEXT_ENTRY_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// Tab Control
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlZeroEnabled" ) );
	storeImageAndColor( TC_TAB_0_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlZeroDisabled" ) );
	storeImageAndColor( TC_TAB_0_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlZeroHilite" ) );
	storeImageAndColor( TC_TAB_0_HILITE, image, green, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlOneEnabled" ) );
	storeImageAndColor( TC_TAB_1_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlOneDisabled" ) );
	storeImageAndColor( TC_TAB_1_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlOneHilite" ) );
	storeImageAndColor( TC_TAB_1_HILITE, image, green, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlTwoEnabled" ) );
	storeImageAndColor( TC_TAB_2_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlTwoDisabled" ) );
	storeImageAndColor( TC_TAB_2_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlTwoHilite" ) );
	storeImageAndColor( TC_TAB_2_HILITE, image, green, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlThreeEnabled" ) );
	storeImageAndColor( TC_TAB_3_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlThreeDisabled" ) );
	storeImageAndColor( TC_TAB_3_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlThreeHilite" ) );
	storeImageAndColor( TC_TAB_3_HILITE, image, green, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlFourEnabled" ) );
	storeImageAndColor( TC_TAB_4_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlFourDisabled" ) );
	storeImageAndColor( TC_TAB_4_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlFourHilite" ) );
	storeImageAndColor( TC_TAB_4_HILITE, image, green, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlFiveEnabled" ) );
	storeImageAndColor( TC_TAB_5_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlFiveDisabled" ) );
	storeImageAndColor( TC_TAB_5_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlFiveHilite" ) );
	storeImageAndColor( TC_TAB_5_HILITE, image, green, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlSixEnabled" ) );
	storeImageAndColor( TC_TAB_6_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlSixDisabled" ) );
	storeImageAndColor( TC_TAB_6_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlSixHilite" ) );
	storeImageAndColor( TC_TAB_6_HILITE, image, green, darkGreen );

	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlSevenEnabled" ) );
	storeImageAndColor( TC_TAB_7_ENABLED, image, red, lightRed );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlSevenDisabled" ) );
	storeImageAndColor( TC_TAB_7_DISABLED, image, gray, lightGray );
	image = TheMappedImageCollection->findImageByName( AsciiString(  "TabControlSevenHilite" ) );
	storeImageAndColor( TC_TAB_7_HILITE, image, green, darkGreen );

	storeImageAndColor( TAB_CONTROL_ENABLED, NULL, black, white );
	storeImageAndColor( TAB_CONTROL_DISABLED, NULL, darkGray, white );
	storeImageAndColor( TAB_CONTROL_HILITE, NULL, black, white );	

	// generic
	storeImageAndColor( GENERIC_ENABLED, NULL, darkBlue, white );
	storeImageAndColor( GENERIC_DISABLED, NULL, darkGray, white );
	storeImageAndColor( GENERIC_HILITE, NULL, lightBlue, white );	

	// default text colors
	m_enabledText.color = white;
	m_enabledText.borderColor = black;
	m_disabledText.color = lightGray;
	m_disabledText.borderColor = darkGray;
	m_hiliteText.color = lightBlue;
	m_hiliteText.borderColor = darkBlue;

	// default font
	m_font = TheWindowManager->winFindFont( AsciiString("Times New Roman"), 14, FALSE );
}  // end init

// LayoutScheme::openDialog ===================================================
/** Bring up the layout scheme dialog box */
//=============================================================================
void LayoutScheme::openDialog( void )
{

	// save the scheme instance we're going to open the dialog on
	theScheme = this;

	// create the dialog box
	DialogBox( TheEditor->getInstance(), (LPCTSTR)LAYOUT_SCHEME_DIALOG,
						 TheEditor->getWindowHandle(), (DLGPROC)layoutSchemeCallback );

	theScheme = NULL;

}  // end openDialog

// LayoutScheme::findEntry ====================================================
/** Find the entry for the state */
//=============================================================================
ImageAndColorInfo *LayoutScheme::findEntry( StateIdentifier id )
{

	// santiy
	if( id < 0 || id >= NUM_STATE_IDENTIFIERS )
	{

		DEBUG_LOG(( "Illegal state to to layout 'findEntry' '%d'\n", id ));
		assert( 0 );
		return NULL;

	}  // end if

	// search the state table
	for( Int i = 0; i < NUM_STATE_IDENTIFIERS; i++ )
	{

		if( m_imageAndColorTable[ i ].stateID == id )
			return &m_imageAndColorTable[ i ];

	}  // end for i

	return NULL;  // not found

}  // end findEntry

// LayoutScheme::getImageAndColor =============================================
/** Get the color and color info for the state */
//=============================================================================
ImageAndColorInfo *LayoutScheme::getImageAndColor( StateIdentifier id )
{

	// sanity
	if( id < 0 || id >= NUM_STATE_IDENTIFIERS )
	{

		DEBUG_LOG(( "getImageAndColor: Illegal state '%d'\n", id ));
		assert( 0 );
		return NULL;

	}  // end if

	// return the entry for the state
	return findEntry( id );

}  // end getImageAndColor

// LayoutScheme::storeImageAndColor ===========================================
/** Store the image and colors of the specific state in our own data array */
//=============================================================================
void LayoutScheme::storeImageAndColor( StateIdentifier id, const Image *image,
																			 Color color, Color borderColor )
{

	// sanity
	if( id < 0 || id >= NUM_STATE_IDENTIFIERS )
	{

		DEBUG_LOG(( "Illegal state identifier in layout scheme store image and color '%d'\n", id ));
		assert( 0 );
		return;

	}  // end if

	// store the info
	ImageAndColorInfo *entry = findEntry( id );
	if( entry )
	{

		entry->image = image;
		entry->color = color;
		entry->borderColor = borderColor;

	}  // en dif

}  // end storeImageAndColor

// LayoutScheme::saveScheme ===================================================
/** Save the scheme to the filename provided */
//=============================================================================
Bool LayoutScheme::saveScheme( char *filename )
{
	FILE *fp;
	UnsignedByte colorR, colorG, colorB, colorA,
							 bColorR, bColorG, bColorB, bColorA;

	// open the file
	fp = fopen( filename, "w" );
	if( fp == NULL )
	{

		DEBUG_LOG(( "saveScheme: Unable to open file '%s'\n", filename ));
		MessageBox( TheEditor->getWindowHandle(),
								"Unable to open scheme for for saving.  Read only?", "Save Error", MB_OK );
		return FALSE;

	}  // end if

	// save the filename we're using now
	setSchemeFilename( filename );

	// write header
	fprintf( fp, "Window Layout Scheme: Version '%d'\n", SCHEME_VERSION );

	// write default text colors
	GameGetColorComponents( m_enabledText.color, &colorR, &colorG, &colorB, &colorA );
	fprintf( fp, "Enabled Text: (%d,%d,%d,%d)\n", colorR, colorG, colorB, colorA );
	GameGetColorComponents( m_enabledText.borderColor, &colorR, &colorG, &colorB, &colorA );
	fprintf( fp, "Enabled Text Border: (%d,%d,%d,%d)\n", colorR, colorG, colorB, colorA );

	GameGetColorComponents( m_disabledText.color, &colorR, &colorG, &colorB, &colorA );
	fprintf( fp, "Disabled Text: (%d,%d,%d,%d)\n", colorR, colorG, colorB, colorA );
	GameGetColorComponents( m_disabledText.borderColor, &colorR, &colorG, &colorB, &colorA );
	fprintf( fp, "Disabled Text Border: (%d,%d,%d,%d)\n", colorR, colorG, colorB, colorA );

	GameGetColorComponents( m_hiliteText.color, &colorR, &colorG, &colorB, &colorA );
	fprintf( fp, "Hilite Text: (%d,%d,%d,%d)\n", colorR, colorG, colorB, colorA );
	GameGetColorComponents( m_hiliteText.borderColor, &colorR, &colorG, &colorB, &colorA );
	fprintf( fp, "Hilite Text Border: (%d,%d,%d,%d)\n", colorR, colorG, colorB, colorA );

	// write default font
	char fontBuffer[ 256 ];
	if( m_font )
		sprintf( fontBuffer, "\"%s\" Size: %d Bold: %d",
						 m_font->nameString.str(), m_font->pointSize, m_font->bold );
	else
		sprintf( fontBuffer, "\"None\" Size: 0 Bold: 0" );
	fprintf( fp, "Default Font: %s\n", fontBuffer );

	// write all the data for all the states
	fprintf( fp, "Number of states: %d\n", NUM_STATE_IDENTIFIERS );
	ImageAndColorInfo *info;
	for( Int i = 0; i < NUM_STATE_IDENTIFIERS; i++ )
	{

		// get the info
		info = findEntry( (StateIdentifier)i );
		assert( info );

		// get the color data in more RGB friendly output
		GameGetColorComponents( info->color, &colorR, &colorG, 
														&colorB, &colorA );
		GameGetColorComponents( info->borderColor, &bColorR, &bColorG, 
														&bColorB, &bColorA );

		// output it
		fprintf( fp, "%d: Image: %s Color: (%d,%d,%d,%d) Border: (%d,%d,%d,%d)\n",
						 i, info->image ? info->image->getName().str() : "NONE", colorR, colorG, colorB, colorA,
						 bColorR, bColorG, bColorB, bColorA );

	}  // end for i

	// close the file
	fclose( fp );

	return TRUE;

}  // end saveScheme

// LayoutScheme::loadScheme ===================================================
/** Load the layout scheme into this class instance */
//=============================================================================
Bool LayoutScheme::loadScheme( char *filename )
{
	FILE *fp;
	UnsignedByte colorR, colorG, colorB, colorA,
							 bColorR, bColorG, bColorB, bColorA;

	// open the file
	fp = fopen( filename, "r" );
	if( fp == NULL )
		return FALSE;

	// save the filename we're using now
	setSchemeFilename( filename );

	// write header
	Int version;
	if (fscanf( fp, "Window Layout Scheme: Version '%d'\n", &version ) == 1)
	{
		if( version != SCHEME_VERSION )
		{

			DEBUG_LOG(( "loadScheme: Old layout file version '%d'\n", version ));
			MessageBox( TheEditor->getWindowHandle(),
									"Old layout version, cannot open.", "Old File", MB_OK );
			return FALSE;

		}  // end if
	}

	// default text colors
	if (fscanf( fp, "Enabled Text: (%hhu,%hhu,%hhu,%hhu)\n", &colorR, &colorG, &colorB, &colorA ) == 4)
	{
		m_enabledText.color = GameMakeColor( colorR, colorG, colorB, colorA );
	}
	if (fscanf( fp, "Enabled Text Border: (%hhu,%hhu,%hhu,%hhu)\n", &colorR, &colorG, &colorB, &colorA ) == 4)
	{
		m_enabledText.borderColor = GameMakeColor( colorR, colorG, colorB, colorA );
	}
	if (fscanf( fp, "Disabled Text: (%hhu,%hhu,%hhu,%hhu)\n", &colorR, &colorG, &colorB, &colorA ) == 4)
	{
		m_disabledText.color = GameMakeColor( colorR, colorG, colorB, colorA );
	}
	if (fscanf( fp, "Disabled Text Border: (%hhu,%hhu,%hhu,%hhu)\n", &colorR, &colorG, &colorB, &colorA ) == 4)
	{
		m_disabledText.borderColor = GameMakeColor( colorR, colorG, colorB, colorA );
	}
	if (fscanf( fp, "Hilite Text: (%hhu,%hhu,%hhu,%hhu)\n", &colorR, &colorG, &colorB, &colorA ) == 4)
	{
		m_hiliteText.color = GameMakeColor( colorR, colorG, colorB, colorA );
	}
	if (fscanf( fp, "Hilite Text Border: (%hhu,%hhu,%hhu,%hhu)\n", &colorR, &colorG, &colorB, &colorA ) == 4)
	{
		m_hiliteText.borderColor = GameMakeColor( colorR, colorG, colorB, colorA );
	}
	// default font
	char fontBuffer[ 256 ];
	Int size, bold;
	char c = fgetc( fp );

	// skip past the first quote
	while( c != '\"' )
		c = fgetc( fp );
	c = fgetc( fp );  // the quote itself

	// copy the name till the next quote is read
	Int index = 0;
	while( c != '\"' )
	{

		fontBuffer[ index++ ] = c;
		c = fgetc( fp );

	}  // end while
	fontBuffer[ index ] = '\0';
	c = fgetc( fp );  // the end quite itself

	// read the size and bold data elements
	if(fscanf( fp, " Size: %i Bold: %i\n", &size, &bold ) == 2)
	{
		// set the font
		m_font = TheFontLibrary->getFont( AsciiString(fontBuffer), size, bold );
	}

	// all the data for all the states
	Int numStates, state;
	char imageBuffer[ 128 ];
	if (fscanf( fp, "Number of states: %i\n", &numStates ) == 1)
	{
		for( Int i = 0; i < numStates; i++ )
		{

			// read all the data
			if( fscanf( fp, "%d: Image: %s Color: (%hhu,%hhu,%hhu,%hhu) Border: (%hhu,%hhu,%hhu,%hhu)\n",
							&state, imageBuffer, &colorR, &colorG, &colorB, &colorA,
							&bColorR, &bColorG, &bColorB, &bColorA ) == 10)
			{
				// sanity
				assert( state == i );

				// store the info
				storeImageAndColor( (StateIdentifier)state,
														TheMappedImageCollection->findImageByName( AsciiString(  imageBuffer ) ),
														GameMakeColor( colorR, colorG, colorB, colorA ),
														GameMakeColor( bColorR, bColorG, bColorB, bColorA ) );
			}
		}  // end for i
	}

	// close the file
	fclose( fp );

	return TRUE;

}  // end loadScheme

