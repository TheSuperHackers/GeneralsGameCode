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

// FILE: ComboBoxProperties.cpp ////////////////////////////////////////////////
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
// File name:  ComboBoxProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       ComboBox properties dialog
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "LayoutScheme.h"
#include "resource.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GameWindowManager.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



// comboBoxPropertiesCallback ==================================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK comboBoxPropertiesCallback( HWND hWndDialog,
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

					//
					// using the current colors in the base of the comboBox, assign a
					// reasonable color scheme to all the sub control components
					//
					info = GetStateInfo( COMBOBOX_ENABLED );
					StoreColor( COMBOBOX_LISTBOX_ENABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_TOP, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED, info->color, info->borderColor );

					info = GetStateInfo( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT );
					StoreColor( COMBOBOX_EDIT_BOX_ENABLED_LEFT, info->color, info->borderColor );
					StoreColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, info->color, info->borderColor );

					info = GetStateInfo( COMBOBOX_DISABLED );
					StoreColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DISABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_TOP, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED, info->color, info->borderColor );

					info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_LEFT );
					StoreColor( COMBOBOX_EDIT_BOX_DISABLED_LEFT, info->color, info->borderColor );
					StoreColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_LEFT, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, info->color, info->borderColor );

					info = GetStateInfo( COMBOBOX_HILITE );
					StoreColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_HILITE, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_HILITE_TOP, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE, info->color, info->borderColor );

					info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_LEFT );
					StoreColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_EDIT_BOX_HILITE_LEFT, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_LEFT, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED, info->color, info->borderColor );
					StoreColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED, info->color, info->borderColor );

					break;

				}

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
						info = GetStateInfo( COMBOBOX_ENABLED );
						GadgetComboBoxSetEnabledImage( window, info->image );
						GadgetComboBoxSetEnabledColor( window, info->color );
						GadgetComboBoxSetEnabledBorderColor( window, info->borderColor );

						info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_LEFT );
						GadgetComboBoxSetEnabledSelectedItemImageLeft( window, info->image );
						GadgetComboBoxSetEnabledSelectedItemColor( window, info->color );
						GadgetComboBoxSetEnabledSelectedItemBorderColor( window, info->borderColor );
						info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_RIGHT );
						GadgetComboBoxSetEnabledSelectedItemImageRight( window, info->image );
						info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_CENTER );
						GadgetComboBoxSetEnabledSelectedItemImageCenter( window, info->image );
						info = GetStateInfo( COMBOBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
						GadgetComboBoxSetEnabledSelectedItemImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( COMBOBOX_DISABLED );
						GadgetComboBoxSetDisabledImage( window, info->image );
						GadgetComboBoxSetDisabledColor( window, info->color );
						GadgetComboBoxSetDisabledBorderColor( window, info->borderColor );

						info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_LEFT );
						GadgetComboBoxSetDisabledSelectedItemImageLeft( window, info->image );
						GadgetComboBoxSetDisabledSelectedItemColor( window, info->color );
						GadgetComboBoxSetDisabledSelectedItemBorderColor( window, info->borderColor );
						info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_RIGHT );
						GadgetComboBoxSetDisabledSelectedItemImageRight( window, info->image );
						info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_CENTER );
						GadgetComboBoxSetDisabledSelectedItemImageCenter( window, info->image );
						info = GetStateInfo( COMBOBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
						GadgetComboBoxSetDisabledSelectedItemImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( COMBOBOX_HILITE );
						GadgetComboBoxSetHiliteImage( window, info->image );
						GadgetComboBoxSetHiliteColor( window, info->color );
						GadgetComboBoxSetHiliteBorderColor( window, info->borderColor );

						info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_LEFT );
						GadgetComboBoxSetHiliteSelectedItemImageLeft( window, info->image );
						GadgetComboBoxSetHiliteSelectedItemColor( window, info->color );
						GadgetComboBoxSetHiliteSelectedItemBorderColor( window, info->borderColor );
						info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_RIGHT );
						GadgetComboBoxSetHiliteSelectedItemImageRight( window, info->image );
						info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_CENTER );
						GadgetComboBoxSetHiliteSelectedItemImageCenter( window, info->image );
						info = GetStateInfo( COMBOBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
						GadgetComboBoxSetHiliteSelectedItemImageSmallCenter( window, info->image );

						GameWindow *dropDownButton = GadgetComboBoxGetDropDownButton( window );
						if (dropDownButton)
						{
							// save the image and color data
							// ----------------------------------------------------------------
							info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_ENABLED );
							GadgetButtonSetEnabledImage( dropDownButton, info->image );
							GadgetButtonSetEnabledColor( dropDownButton, info->color );
							GadgetButtonSetEnabledBorderColor( dropDownButton, info->borderColor );

							info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED );
							GadgetButtonSetEnabledSelectedImage( dropDownButton, info->image );
							GadgetButtonSetEnabledSelectedColor( dropDownButton, info->color );
							GadgetButtonSetEnabledSelectedBorderColor( dropDownButton, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_DISABLED );
							GadgetButtonSetDisabledImage( dropDownButton, info->image );
							GadgetButtonSetDisabledColor( dropDownButton, info->color );
							GadgetButtonSetDisabledBorderColor( dropDownButton, info->borderColor );

							info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED );
							GadgetButtonSetDisabledSelectedImage( dropDownButton, info->image );
							GadgetButtonSetDisabledSelectedColor( dropDownButton, info->color );
							GadgetButtonSetDisabledSelectedBorderColor( dropDownButton, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_HILITE );
							GadgetButtonSetHiliteImage( dropDownButton, info->image );
							GadgetButtonSetHiliteColor( dropDownButton, info->color );
							GadgetButtonSetHiliteBorderColor( dropDownButton, info->borderColor );

							info = GetStateInfo( COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED );
							GadgetButtonSetHiliteSelectedImage( dropDownButton, info->image );
							GadgetButtonSetHiliteSelectedColor( dropDownButton, info->color );
							GadgetButtonSetHiliteSelectedBorderColor( dropDownButton, info->borderColor );
						}

						GameWindow *editBox = GadgetComboBoxGetEditBox( window );
						if (editBox)
						{
							// save the image and color data
							// ----------------------------------------------------------------
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

							// ----------------------------------------------------------------
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

							// ----------------------------------------------------------------
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

						}

						GameWindow *listBox = GadgetComboBoxGetListBox( window );
						if (listBox)
						{
							// save the image and color data
							// ----------------------------------------------------------------
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

							// ----------------------------------------------------------------
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

							// ----------------------------------------------------------------
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

							// up button
							GameWindow *upButton = GadgetListBoxGetUpButton( listBox );
							if( upButton )
							{

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED );
								GadgetButtonSetEnabledImage( upButton, info->image );
								GadgetButtonSetEnabledColor( upButton, info->color );
								GadgetButtonSetEnabledBorderColor( upButton, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED );
								GadgetButtonSetEnabledSelectedImage( upButton, info->image );
								GadgetButtonSetEnabledSelectedColor( upButton, info->color );
								GadgetButtonSetEnabledSelectedBorderColor( upButton, info->borderColor );

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED );
								GadgetButtonSetDisabledImage( upButton, info->image );
								GadgetButtonSetDisabledColor( upButton, info->color );
								GadgetButtonSetDisabledBorderColor( upButton, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED );
								GadgetButtonSetDisabledSelectedImage( upButton, info->image );
								GadgetButtonSetDisabledSelectedColor( upButton, info->color );
								GadgetButtonSetDisabledSelectedBorderColor( upButton, info->borderColor );

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_HILITE );
								GadgetButtonSetHiliteImage( upButton, info->image );
								GadgetButtonSetHiliteColor( upButton, info->color );
								GadgetButtonSetHiliteBorderColor( upButton, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED );
								GadgetButtonSetHiliteSelectedImage( upButton, info->image );
								GadgetButtonSetHiliteSelectedColor( upButton, info->color );
								GadgetButtonSetHiliteSelectedBorderColor( upButton, info->borderColor );

							}

							// down button
							GameWindow *downButton = GadgetListBoxGetDownButton( listBox );
							if( downButton )
							{

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED );
								GadgetButtonSetEnabledImage( downButton, info->image );
								GadgetButtonSetEnabledColor( downButton, info->color );
								GadgetButtonSetEnabledBorderColor( downButton, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED );
								GadgetButtonSetEnabledSelectedImage( downButton, info->image );
								GadgetButtonSetEnabledSelectedColor( downButton, info->color );
								GadgetButtonSetEnabledSelectedBorderColor( downButton, info->borderColor );

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED );
								GadgetButtonSetDisabledImage( downButton, info->image );
								GadgetButtonSetDisabledColor( downButton, info->color );
								GadgetButtonSetDisabledBorderColor( downButton, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED );
								GadgetButtonSetDisabledSelectedImage( downButton, info->image );
								GadgetButtonSetDisabledSelectedColor( downButton, info->color );
								GadgetButtonSetDisabledSelectedBorderColor( downButton, info->borderColor );

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE );
								GadgetButtonSetHiliteImage( downButton, info->image );
								GadgetButtonSetHiliteColor( downButton, info->color );
								GadgetButtonSetHiliteBorderColor( downButton, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED );
								GadgetButtonSetHiliteSelectedImage( downButton, info->image );
								GadgetButtonSetHiliteSelectedColor( downButton, info->color );
								GadgetButtonSetHiliteSelectedBorderColor( downButton, info->borderColor );

							}

							// slider
							GameWindow *slider = GadgetListBoxGetSlider( listBox );
							if( slider )
							{

								// ----------------------------------------------------------------
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

								// ----------------------------------------------------------------

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

								// ----------------------------------------------------------------

								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_TOP );
								GadgetSliderSetHiliteImageTop( slider, info->image );
								GadgetSliderSetHiliteColor( slider, info->color );
								GadgetSliderSetHiliteBorderColor( slider, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_BOTTOM );
								GadgetSliderSetHiliteImageBottom( slider, info->image );

								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_CENTER );
								GadgetSliderSetHiliteImageCenter( slider, info->image );

								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_HILITE_SMALL_CENTER );
								GadgetSliderSetHiliteImageSmallCenter( slider, info->image );

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED );
								GadgetSliderSetEnabledThumbImage( slider, info->image );
								GadgetSliderSetEnabledThumbColor( slider, info->color );
								GadgetSliderSetEnabledThumbBorderColor( slider, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED );
								GadgetSliderSetEnabledSelectedThumbImage( slider, info->image );
								GadgetSliderSetEnabledSelectedThumbColor( slider, info->color );
								GadgetSliderSetEnabledSelectedThumbBorderColor( slider, info->borderColor );

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED );
								GadgetSliderSetDisabledThumbImage( slider, info->image );
								GadgetSliderSetDisabledThumbColor( slider, info->color );
								GadgetSliderSetDisabledThumbBorderColor( slider, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED );
								GadgetSliderSetDisabledSelectedThumbImage( slider, info->image );
								GadgetSliderSetDisabledSelectedThumbColor( slider, info->color );
								GadgetSliderSetDisabledSelectedThumbBorderColor( slider, info->borderColor );

								// ----------------------------------------------------------------
								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE );
								GadgetSliderSetHiliteThumbImage( slider, info->image );
								GadgetSliderSetHiliteThumbColor( slider, info->color );
								GadgetSliderSetHiliteThumbBorderColor( slider, info->borderColor );

								info = GetStateInfo( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED );
								GadgetSliderSetHiliteSelectedThumbImage( slider, info->image );
								GadgetSliderSetHiliteSelectedThumbColor( slider, info->color );
								GadgetSliderSetHiliteSelectedThumbBorderColor( slider, info->borderColor );

							}
						}
						// save specific list data
						ComboBoxData *comboData = (ComboBoxData *)window->winGetUserData();

						GadgetComboBoxSetIsEditable(window, IsDlgButtonChecked( hWndDialog, CHECK_IS_EDITABLE ));
						GadgetComboBoxSetAsciiOnly(window, IsDlgButtonChecked( hWndDialog, CHECK_ASCII_TEXT ));
						GadgetComboBoxSetLettersAndNumbersOnly(window, IsDlgButtonChecked( hWndDialog, CHECK_LETTERS_AND_NUMBERS ));

						// change in the size of the comboBox
						Int newMaxChars = GetDlgItemInt( hWndDialog, EDIT_MAX_CHARS, nullptr, FALSE );
						if( newMaxChars != comboData->maxChars)
							GadgetComboBoxSetMaxChars( window, newMaxChars );

						Int newMaxDisplay = GetDlgItemInt( hWndDialog, EDIT_MAX_ITEMS_DISPLAYED, nullptr, FALSE );
						if( newMaxDisplay != comboData->maxDisplay )
							GadgetComboBoxSetMaxDisplay( window, newMaxDisplay );


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

// InitComboBoxPropertiesDialog ================================================
/** Bring up the comboBox properties window */
//=============================================================================
HWND InitComboBoxPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)COMBO_BOX_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)comboBoxPropertiesCallback );
	if( dialog == nullptr )
		return nullptr;

	// do the common initialization
	CommonDialogInitialize( window, dialog );

	//
	// store in the image and color table the values for this combo Box
	//
	const Image *image;
	Color color, borderColor;

	// --------------------------------------------------------------------------
	image = GadgetComboBoxGetEnabledImage( window );
	color = GadgetComboBoxGetEnabledColor( window );
	borderColor = GadgetComboBoxGetEnabledBorderColor( window );
	StoreImageAndColor( COMBOBOX_ENABLED, image, color, borderColor );

	image = GadgetComboBoxGetEnabledSelectedItemImageLeft( window );
	color = GadgetComboBoxGetEnabledSelectedItemColor( window );
	borderColor = GadgetComboBoxGetEnabledSelectedItemBorderColor( window );
	StoreImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_LEFT, image, color, borderColor );
	image = GadgetComboBoxGetEnabledSelectedItemImageRight( window );
	StoreImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetComboBoxGetEnabledSelectedItemImageCenter( window );
	StoreImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetComboBoxGetEnabledSelectedItemImageSmallCenter( window );
	StoreImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetComboBoxGetDisabledImage( window );
	color = GadgetComboBoxGetDisabledColor( window );
	borderColor = GadgetComboBoxGetDisabledBorderColor( window );
	StoreImageAndColor( COMBOBOX_DISABLED, image, color, borderColor );

	image = GadgetComboBoxGetDisabledSelectedItemImageLeft( window );
	color = GadgetComboBoxGetDisabledSelectedItemColor( window );
	borderColor = GadgetComboBoxGetDisabledSelectedItemBorderColor( window );
	StoreImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_LEFT, image, color, borderColor );
	image = GadgetComboBoxGetDisabledSelectedItemImageRight( window );
	StoreImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetComboBoxGetDisabledSelectedItemImageCenter( window );
	StoreImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetComboBoxGetDisabledSelectedItemImageSmallCenter( window );
	StoreImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetComboBoxGetHiliteImage( window );
	color = GadgetComboBoxGetHiliteColor( window );
	borderColor = GadgetComboBoxGetHiliteBorderColor( window );
	StoreImageAndColor( COMBOBOX_HILITE, image, color, borderColor );

	image = GadgetComboBoxGetHiliteSelectedItemImageLeft( window );
	color = GadgetComboBoxGetHiliteSelectedItemColor( window );
	borderColor = GadgetComboBoxGetHiliteSelectedItemBorderColor( window );
	StoreImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_LEFT, image, color, borderColor );
	image = GadgetComboBoxGetHiliteSelectedItemImageRight( window );
	StoreImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetComboBoxGetHiliteSelectedItemImageCenter( window );
	StoreImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetComboBoxGetHiliteSelectedItemImageSmallCenter( window );
	StoreImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

  GameWindow *listBox = GadgetComboBoxGetListBox( window );
	if (listBox)
	{
		image = GadgetListBoxGetEnabledSelectedItemImageLeft( listBox );
		color = GadgetListBoxGetEnabledSelectedItemColor( listBox );
		borderColor = GadgetListBoxGetEnabledSelectedItemBorderColor( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT, image, color, borderColor );
		image = GadgetListBoxGetEnabledSelectedItemImageRight( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
		image = GadgetListBoxGetEnabledSelectedItemImageCenter( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
		image = GadgetListBoxGetEnabledSelectedItemImageSmallCenter( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

		// --------------------------------------------------------------------------
		image = GadgetListBoxGetDisabledImage( listBox );
		color = GadgetListBoxGetDisabledColor( listBox );
		borderColor = GadgetListBoxGetDisabledBorderColor( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_DISABLED, image, color, borderColor );

		image = GadgetListBoxGetDisabledSelectedItemImageLeft( listBox );
		color = GadgetListBoxGetDisabledSelectedItemColor( listBox );
		borderColor = GadgetListBoxGetDisabledSelectedItemBorderColor( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_LEFT, image, color, borderColor );
		image = GadgetListBoxGetDisabledSelectedItemImageRight( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
		image = GadgetListBoxGetDisabledSelectedItemImageCenter( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
		image = GadgetListBoxGetDisabledSelectedItemImageSmallCenter( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

		// --------------------------------------------------------------------------
		image = GadgetListBoxGetHiliteImage( listBox );
		color = GadgetListBoxGetHiliteColor( listBox );
		borderColor = GadgetListBoxGetHiliteBorderColor( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_HILITE, image, color, borderColor );

		image = GadgetListBoxGetHiliteSelectedItemImageLeft( listBox );
		color = GadgetListBoxGetHiliteSelectedItemColor( listBox );
		borderColor = GadgetListBoxGetHiliteSelectedItemBorderColor( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_LEFT, image, color, borderColor );
		image = GadgetListBoxGetHiliteSelectedItemImageRight( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
		image = GadgetListBoxGetHiliteSelectedItemImageCenter( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
		image = GadgetListBoxGetHiliteSelectedItemImageSmallCenter( listBox );
		StoreImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );



		// --------------------------------------------------------------------------
		GameWindow *upButton = GadgetListBoxGetUpButton( listBox );
		if( upButton )
		{

			// ------------------------------------------------------------------------
			image = GadgetButtonGetEnabledImage( upButton );
			color = GadgetButtonGetEnabledColor( upButton );
			borderColor = GadgetButtonGetEnabledBorderColor( upButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED, image, color, borderColor );

			image = GadgetButtonGetEnabledSelectedImage( upButton );
			color = GadgetButtonGetEnabledSelectedColor( upButton );
			borderColor = GadgetButtonGetEnabledSelectedBorderColor( upButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED, image, color, borderColor );

			// ------------------------------------------------------------------------
			image = GadgetButtonGetDisabledImage( upButton );
			color = GadgetButtonGetDisabledColor( upButton );
			borderColor = GadgetButtonGetDisabledBorderColor( upButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED, image, color, borderColor );

			image = GadgetButtonGetDisabledSelectedImage( upButton );
			color = GadgetButtonGetDisabledSelectedColor( upButton );
			borderColor = GadgetButtonGetDisabledSelectedBorderColor( upButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED, image, color, borderColor );

			// ------------------------------------------------------------------------
			image = GadgetButtonGetHiliteImage( upButton );
			color = GadgetButtonGetHiliteColor( upButton );
			borderColor = GadgetButtonGetHiliteBorderColor( upButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE, image, color, borderColor );

			image = GadgetButtonGetHiliteSelectedImage( upButton );
			color = GadgetButtonGetHiliteSelectedColor( upButton );
			borderColor = GadgetButtonGetHiliteSelectedBorderColor( upButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED, image, color, borderColor );

		}

		// --------------------------------------------------------------------------
		GameWindow *downButton = GadgetListBoxGetDownButton( listBox );
		if( downButton )
		{

			// ------------------------------------------------------------------------
			image = GadgetButtonGetEnabledImage( downButton );
			color = GadgetButtonGetEnabledColor( downButton );
			borderColor = GadgetButtonGetEnabledBorderColor( downButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED, image, color, borderColor );

			image = GadgetButtonGetEnabledSelectedImage( downButton );
			color = GadgetButtonGetEnabledSelectedColor( downButton );
			borderColor = GadgetButtonGetEnabledSelectedBorderColor( downButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED, image, color, borderColor );

			// ------------------------------------------------------------------------
			image = GadgetButtonGetDisabledImage( downButton );
			color = GadgetButtonGetDisabledColor( downButton );
			borderColor = GadgetButtonGetDisabledBorderColor( downButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED, image, color, borderColor );

			image = GadgetButtonGetDisabledSelectedImage( downButton );
			color = GadgetButtonGetDisabledSelectedColor( downButton );
			borderColor = GadgetButtonGetDisabledSelectedBorderColor( downButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED, image, color, borderColor );

			// ------------------------------------------------------------------------
			image = GadgetButtonGetHiliteImage( downButton );
			color = GadgetButtonGetHiliteColor( downButton );
			borderColor = GadgetButtonGetHiliteBorderColor( downButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE, image, color, borderColor );

			image = GadgetButtonGetHiliteSelectedImage( downButton );
			color = GadgetButtonGetHiliteSelectedColor( downButton );
			borderColor = GadgetButtonGetHiliteSelectedBorderColor( downButton );
			StoreImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED, image, color, borderColor );

		}

		GameWindow *slider = GadgetListBoxGetSlider( listBox );
		if( slider )
		{

			// --------------------------------------------------------------------------
			image = GadgetSliderGetEnabledImageTop( slider );
			color = GadgetSliderGetEnabledColor( slider );
			borderColor = GadgetSliderGetEnabledBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_TOP, image, color, borderColor );

			image = GadgetSliderGetEnabledImageBottom( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_BOTTOM, image, color, borderColor );

			image = GadgetSliderGetEnabledImageCenter( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_CENTER, image, color, borderColor );

			image = GadgetSliderGetEnabledImageSmallCenter( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_SMALL_CENTER, image, color, borderColor );

			// --------------------------------------------------------------------------
			image = GadgetSliderGetDisabledImageTop( slider );
			color = GadgetSliderGetDisabledColor( slider );
			borderColor = GadgetSliderGetDisabledBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_TOP, image, color, borderColor );

			image = GadgetSliderGetDisabledImageBottom( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_BOTTOM, image, color, borderColor );

			image = GadgetSliderGetDisabledImageCenter( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_CENTER, image, color, borderColor );

			image = GadgetSliderGetDisabledImageSmallCenter( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_SMALL_CENTER, image, color, borderColor );

			// --------------------------------------------------------------------------
			image = GadgetSliderGetHiliteImageTop( slider );
			color = GadgetSliderGetHiliteColor( slider );
			borderColor = GadgetSliderGetHiliteBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_TOP, image, color, borderColor );

			image = GadgetSliderGetHiliteImageBottom( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_BOTTOM, image, color, borderColor );

			image = GadgetSliderGetHiliteImageCenter( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_CENTER, image, color, borderColor );

			image = GadgetSliderGetHiliteImageSmallCenter( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_SMALL_CENTER, image, color, borderColor );

			// --------------------------------------------------------------------------
			image = GadgetSliderGetEnabledThumbImage( slider );
			color = GadgetSliderGetEnabledThumbColor( slider );
			borderColor = GadgetSliderGetEnabledThumbBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED, image, color, borderColor );

			image = GadgetSliderGetEnabledSelectedThumbImage( slider );
			color = GadgetSliderGetEnabledSelectedThumbColor( slider );
			borderColor = GadgetSliderGetEnabledSelectedThumbBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, image, color, borderColor );

			// --------------------------------------------------------------------------
			image = GadgetSliderGetDisabledThumbImage( slider );
			color = GadgetSliderGetDisabledThumbColor( slider );
			borderColor = GadgetSliderGetDisabledThumbBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED, image, color, borderColor );

			image = GadgetSliderGetDisabledSelectedThumbImage( slider );
			color = GadgetSliderGetDisabledSelectedThumbColor( slider );
			borderColor = GadgetSliderGetDisabledSelectedThumbBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, image, color, borderColor );

			// --------------------------------------------------------------------------
			image = GadgetSliderGetHiliteThumbImage( slider );
			color = GadgetSliderGetHiliteThumbColor( slider );
			borderColor = GadgetSliderGetHiliteThumbBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE, image, color, borderColor );

			image = GadgetSliderGetHiliteSelectedThumbImage( slider );
			color = GadgetSliderGetHiliteSelectedThumbColor( slider );
			borderColor = GadgetSliderGetHiliteSelectedThumbBorderColor( slider );
			StoreImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED, image, color, borderColor );

		}

		GameWindow *dropDownButton = GadgetComboBoxGetDropDownButton( window );
		if ( dropDownButton )
		{
			image = GadgetButtonGetEnabledImage( dropDownButton );
			color = GadgetButtonGetEnabledColor( dropDownButton );
			borderColor = GadgetButtonGetEnabledBorderColor( dropDownButton );
			StoreImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED, image, color, borderColor );

			image = GadgetButtonGetEnabledSelectedImage(dropDownButton );
			color = GadgetButtonGetEnabledSelectedColor( dropDownButton );
			borderColor = GadgetButtonGetEnabledSelectedBorderColor( dropDownButton );
			StoreImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED, image, color, borderColor );

			image = GadgetButtonGetDisabledImage( dropDownButton );
			color = GadgetButtonGetDisabledColor( dropDownButton );
			borderColor = GadgetButtonGetDisabledBorderColor( dropDownButton );
			StoreImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED, image, color, borderColor );

			image = GadgetButtonGetDisabledSelectedImage( dropDownButton );
			color = GadgetButtonGetDisabledSelectedColor( dropDownButton );
			borderColor = GadgetButtonGetDisabledSelectedBorderColor( dropDownButton );
			StoreImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED, image, color, borderColor );

			image = GadgetButtonGetHiliteImage( dropDownButton );
			color = GadgetButtonGetHiliteColor( dropDownButton );
			borderColor = GadgetButtonGetHiliteBorderColor( dropDownButton );
			StoreImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE, image, color, borderColor );

			image = GadgetButtonGetHiliteSelectedImage( dropDownButton );
			color = GadgetButtonGetHiliteSelectedColor( dropDownButton );
			borderColor = GadgetButtonGetHiliteSelectedBorderColor( dropDownButton );
			StoreImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED, image, color, borderColor );
		}
		GameWindow * editBox = GadgetComboBoxGetEditBox( window );
		if ( editBox )
		{
			// --------------------------------------------------------------------------
			image = GadgetTextEntryGetEnabledImageLeft( editBox );
			color = GadgetTextEntryGetEnabledColor( editBox );
			borderColor = GadgetTextEntryGetEnabledBorderColor( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_LEFT, image, color, borderColor );
			image = GadgetTextEntryGetEnabledImageRight( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
			image = GadgetTextEntryGetEnabledImageCenter( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
			image = GadgetTextEntryGetEnabledImageSmallCenter( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

			// --------------------------------------------------------------------------
			image = GadgetTextEntryGetDisabledImageLeft( editBox );
			color = GadgetTextEntryGetDisabledColor( editBox );
			borderColor = GadgetTextEntryGetDisabledBorderColor( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_LEFT, image, color, borderColor );
			image = GadgetTextEntryGetDisabledImageRight( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
			image = GadgetTextEntryGetDisabledImageCenter( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
			image = GadgetTextEntryGetDisabledImageSmallCenter( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

			// --------------------------------------------------------------------------
			image = GadgetTextEntryGetHiliteImageLeft( editBox );
			color = GadgetTextEntryGetHiliteColor( editBox );
			borderColor = GadgetTextEntryGetHiliteBorderColor( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_HILITE_LEFT, image, color, borderColor );
			image = GadgetTextEntryGetHiliteImageRight( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_HILITE_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
			image = GadgetTextEntryGetHiliteImageCenter( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_HILITE_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
			image = GadgetTextEntryGetHiliteImageSmallCenter( editBox );
			StoreImageAndColor( COMBOBOX_EDIT_BOX_HILITE_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

		}
	}

	// init comboBox specific property section
  ComboBoxData *comboData = (ComboBoxData *)window->winGetUserData();

	SetDlgItemInt(dialog, EDIT_MAX_CHARS, comboData->maxChars, true);
	SetDlgItemInt(dialog, EDIT_MAX_ITEMS_DISPLAYED, comboData->maxDisplay,true );
	CheckDlgButton( dialog, CHECK_IS_EDITABLE, comboData->isEditable);
	CheckDlgButton( dialog, CHECK_ASCII_TEXT ,comboData->asciiOnly);
	CheckDlgButton( dialog, CHECK_LETTERS_AND_NUMBERS, comboData->lettersAndNumbersOnly);

	// select the button enabled state for display
	SwitchToState( COMBOBOX_ENABLED, dialog );

	//
	// initialize the dialog with values from the window
	//

	return dialog;

}



