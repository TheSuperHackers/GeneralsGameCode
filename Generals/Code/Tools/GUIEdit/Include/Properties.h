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

// FILE: Properties.h /////////////////////////////////////////////////////////
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
// File name:  Properties.h
//
// Created:    Colin Day, August 2001
//
// Desc:       Header for bringing up property dialogs for windows and
//						 window gadgets.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __PROPERTIES_H_
#define __PROPERTIES_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindow.h"
#include "GUIEditColor.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ColorControl ---------------------------------------------------------------
/** This makes it easier for us to associate colors with dialog
	* control ID's for purposes of custom color buttons */
//-----------------------------------------------------------------------------
struct ColorControl
{

	UnsignedInt controlID;  ///< controlID from resource file
	RGBColorInt color;					///< the color for this controlID

};

// StateIdentifiers -----------------------------------------------------------
/** These state identifiers link to image and color info entries */
//-----------------------------------------------------------------------------
typedef enum
{

	IDENTIFIER_INVALID = 0,  // keep this 0 and keep it first

	FIRST_VALID_IDENTIFIER = 1,

	BUTTON_ENABLED = FIRST_VALID_IDENTIFIER,
	BUTTON_ENABLED_PUSHED,
	BUTTON_DISABLED,
	BUTTON_DISABLED_PUSHED,
	BUTTON_HILITE,
	BUTTON_HILITE_PUSHED,

	RADIO_ENABLED,
	RADIO_ENABLED_UNCHECKED_BOX,
	RADIO_ENABLED_CHECKED_BOX,
	RADIO_DISABLED,
	RADIO_DISABLED_UNCHECKED_BOX,
	RADIO_DISABLED_CHECKED_BOX,
	RADIO_HILITE,
	RADIO_HILITE_UNCHECKED_BOX,
	RADIO_HILITE_CHECKED_BOX,

	CHECK_BOX_ENABLED,
	CHECK_BOX_ENABLED_UNCHECKED_BOX,
	CHECK_BOX_ENABLED_CHECKED_BOX,
	CHECK_BOX_DISABLED,
	CHECK_BOX_DISABLED_UNCHECKED_BOX,
	CHECK_BOX_DISABLED_CHECKED_BOX,
	CHECK_BOX_HILITE,
	CHECK_BOX_HILITE_UNCHECKED_BOX,
	CHECK_BOX_HILITE_CHECKED_BOX,

	COMBOBOX_ENABLED,
	COMBOBOX_ENABLED_SELECTED_ITEM_LEFT,
	COMBOBOX_ENABLED_SELECTED_ITEM_RIGHT,
	COMBOBOX_ENABLED_SELECTED_ITEM_CENTER,
	COMBOBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER,
	COMBOBOX_DISABLED,
	COMBOBOX_DISABLED_SELECTED_ITEM_LEFT,
	COMBOBOX_DISABLED_SELECTED_ITEM_RIGHT,
	COMBOBOX_DISABLED_SELECTED_ITEM_CENTER,
	COMBOBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER,
	COMBOBOX_HILITE,
	COMBOBOX_HILITE_SELECTED_ITEM_LEFT,
	COMBOBOX_HILITE_SELECTED_ITEM_RIGHT,
	COMBOBOX_HILITE_SELECTED_ITEM_CENTER,
	COMBOBOX_HILITE_SELECTED_ITEM_SMALL_CENTER,
	COMBOBOX_DROP_DOWN_BUTTON_ENABLED,
	COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED,
	COMBOBOX_DROP_DOWN_BUTTON_DISABLED,
	COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED,
	COMBOBOX_DROP_DOWN_BUTTON_HILITE,
	COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED,
	COMBOBOX_EDIT_BOX_ENABLED_LEFT,
	COMBOBOX_EDIT_BOX_ENABLED_RIGHT,
	COMBOBOX_EDIT_BOX_ENABLED_CENTER,
	COMBOBOX_EDIT_BOX_ENABLED_SMALL_CENTER,
	COMBOBOX_EDIT_BOX_DISABLED_LEFT,
	COMBOBOX_EDIT_BOX_DISABLED_RIGHT,
	COMBOBOX_EDIT_BOX_DISABLED_CENTER,
	COMBOBOX_EDIT_BOX_DISABLED_SMALL_CENTER,
	COMBOBOX_EDIT_BOX_HILITE_LEFT,
	COMBOBOX_EDIT_BOX_HILITE_RIGHT,
	COMBOBOX_EDIT_BOX_HILITE_CENTER,
	COMBOBOX_EDIT_BOX_HILITE_SMALL_CENTER,
	COMBOBOX_LISTBOX_ENABLED,
	COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT,
	COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_RIGHT,
	COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_CENTER,
	COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER,
	COMBOBOX_LISTBOX_DISABLED,
	COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_LEFT,
	COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_RIGHT,
	COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_CENTER,
	COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER,
	COMBOBOX_LISTBOX_HILITE,
	COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_LEFT,
	COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_RIGHT,
	COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_CENTER,
	COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER,
	COMBOBOX_LISTBOX_UP_BUTTON_ENABLED,
	COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED,
	COMBOBOX_LISTBOX_UP_BUTTON_DISABLED,
	COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED,
	COMBOBOX_LISTBOX_UP_BUTTON_HILITE,
	COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED,
	COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED,
	COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED,
	COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED,
	COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED,
	COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE,
	COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED,
	COMBOBOX_LISTBOX_SLIDER_ENABLED_TOP,
	COMBOBOX_LISTBOX_SLIDER_ENABLED_BOTTOM,
	COMBOBOX_LISTBOX_SLIDER_ENABLED_CENTER,
	COMBOBOX_LISTBOX_SLIDER_ENABLED_SMALL_CENTER,
	COMBOBOX_LISTBOX_SLIDER_DISABLED_TOP,
	COMBOBOX_LISTBOX_SLIDER_DISABLED_BOTTOM,
	COMBOBOX_LISTBOX_SLIDER_DISABLED_CENTER,
	COMBOBOX_LISTBOX_SLIDER_DISABLED_SMALL_CENTER,
	COMBOBOX_LISTBOX_SLIDER_HILITE_TOP,
	COMBOBOX_LISTBOX_SLIDER_HILITE_BOTTOM,
	COMBOBOX_LISTBOX_SLIDER_HILITE_CENTER,
	COMBOBOX_LISTBOX_SLIDER_HILITE_SMALL_CENTER,
	COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED,
	COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED,
	COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED,
	COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED,
	COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE,
	COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED,

	VSLIDER_ENABLED_TOP,
	VSLIDER_ENABLED_BOTTOM,
	VSLIDER_ENABLED_CENTER,
	VSLIDER_ENABLED_SMALL_CENTER,
	VSLIDER_DISABLED_TOP,
	VSLIDER_DISABLED_BOTTOM,
	VSLIDER_DISABLED_CENTER,
	VSLIDER_DISABLED_SMALL_CENTER,
	VSLIDER_HILITE_TOP,
	VSLIDER_HILITE_BOTTOM,
	VSLIDER_HILITE_CENTER,
	VSLIDER_HILITE_SMALL_CENTER,
	VSLIDER_THUMB_ENABLED,
	VSLIDER_THUMB_ENABLED_PUSHED,
	VSLIDER_THUMB_DISABLED,
	VSLIDER_THUMB_DISABLED_PUSHED,
	VSLIDER_THUMB_HILITE,
	VSLIDER_THUMB_HILITE_PUSHED,

	HSLIDER_ENABLED_LEFT,
	HSLIDER_ENABLED_RIGHT,
	HSLIDER_ENABLED_CENTER,
	HSLIDER_ENABLED_SMALL_CENTER,
	HSLIDER_DISABLED_LEFT,
	HSLIDER_DISABLED_RIGHT,
	HSLIDER_DISABLED_CENTER,
	HSLIDER_DISABLED_SMALL_CENTER,
	HSLIDER_HILITE_LEFT,
	HSLIDER_HILITE_RIGHT,
	HSLIDER_HILITE_CENTER,
	HSLIDER_HILITE_SMALL_CENTER,
	HSLIDER_THUMB_ENABLED,
	HSLIDER_THUMB_ENABLED_PUSHED,
	HSLIDER_THUMB_DISABLED,
	HSLIDER_THUMB_DISABLED_PUSHED,
	HSLIDER_THUMB_HILITE,
	HSLIDER_THUMB_HILITE_PUSHED,

	LISTBOX_ENABLED,
	LISTBOX_ENABLED_SELECTED_ITEM_LEFT,
	LISTBOX_ENABLED_SELECTED_ITEM_RIGHT,
	LISTBOX_ENABLED_SELECTED_ITEM_CENTER,
	LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER,
	LISTBOX_DISABLED,
	LISTBOX_DISABLED_SELECTED_ITEM_LEFT,
	LISTBOX_DISABLED_SELECTED_ITEM_RIGHT,
	LISTBOX_DISABLED_SELECTED_ITEM_CENTER,
	LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER,
	LISTBOX_HILITE,
	LISTBOX_HILITE_SELECTED_ITEM_LEFT,
	LISTBOX_HILITE_SELECTED_ITEM_RIGHT,
	LISTBOX_HILITE_SELECTED_ITEM_CENTER,
	LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER,
	LISTBOX_UP_BUTTON_ENABLED,
	LISTBOX_UP_BUTTON_ENABLED_PUSHED,
	LISTBOX_UP_BUTTON_DISABLED,
	LISTBOX_UP_BUTTON_DISABLED_PUSHED,
	LISTBOX_UP_BUTTON_HILITE,
	LISTBOX_UP_BUTTON_HILITE_PUSHED,
	LISTBOX_DOWN_BUTTON_ENABLED,
	LISTBOX_DOWN_BUTTON_ENABLED_PUSHED,
	LISTBOX_DOWN_BUTTON_DISABLED,
	LISTBOX_DOWN_BUTTON_DISABLED_PUSHED,
	LISTBOX_DOWN_BUTTON_HILITE,
	LISTBOX_DOWN_BUTTON_HILITE_PUSHED,
	LISTBOX_SLIDER_ENABLED_TOP,
	LISTBOX_SLIDER_ENABLED_BOTTOM,
	LISTBOX_SLIDER_ENABLED_CENTER,
	LISTBOX_SLIDER_ENABLED_SMALL_CENTER,
	LISTBOX_SLIDER_DISABLED_TOP,
	LISTBOX_SLIDER_DISABLED_BOTTOM,
	LISTBOX_SLIDER_DISABLED_CENTER,
	LISTBOX_SLIDER_DISABLED_SMALL_CENTER,
	LISTBOX_SLIDER_HILITE_TOP,
	LISTBOX_SLIDER_HILITE_BOTTOM,
	LISTBOX_SLIDER_HILITE_CENTER,
	LISTBOX_SLIDER_HILITE_SMALL_CENTER,
	LISTBOX_SLIDER_THUMB_ENABLED,
	LISTBOX_SLIDER_THUMB_ENABLED_PUSHED,
	LISTBOX_SLIDER_THUMB_DISABLED,
	LISTBOX_SLIDER_THUMB_DISABLED_PUSHED,
	LISTBOX_SLIDER_THUMB_HILITE,
	LISTBOX_SLIDER_THUMB_HILITE_PUSHED,

	PROGRESS_BAR_ENABLED_LEFT,
	PROGRESS_BAR_ENABLED_RIGHT,
	PROGRESS_BAR_ENABLED_CENTER,
	PROGRESS_BAR_ENABLED_SMALL_CENTER,
	PROGRESS_BAR_ENABLED_BAR_LEFT,
	PROGRESS_BAR_ENABLED_BAR_RIGHT,
	PROGRESS_BAR_ENABLED_BAR_CENTER,
	PROGRESS_BAR_ENABLED_BAR_SMALL_CENTER,
	PROGRESS_BAR_DISABLED_LEFT,
	PROGRESS_BAR_DISABLED_RIGHT,
	PROGRESS_BAR_DISABLED_CENTER,
	PROGRESS_BAR_DISABLED_SMALL_CENTER,
	PROGRESS_BAR_DISABLED_BAR_LEFT,
	PROGRESS_BAR_DISABLED_BAR_RIGHT,
	PROGRESS_BAR_DISABLED_BAR_CENTER,
	PROGRESS_BAR_DISABLED_BAR_SMALL_CENTER,
	PROGRESS_BAR_HILITE_LEFT,
	PROGRESS_BAR_HILITE_RIGHT,
	PROGRESS_BAR_HILITE_CENTER,
	PROGRESS_BAR_HILITE_SMALL_CENTER,
	PROGRESS_BAR_HILITE_BAR_LEFT,
	PROGRESS_BAR_HILITE_BAR_RIGHT,
	PROGRESS_BAR_HILITE_BAR_CENTER,
	PROGRESS_BAR_HILITE_BAR_SMALL_CENTER,

	STATIC_TEXT_ENABLED,
	STATIC_TEXT_DISABLED,
	STATIC_TEXT_HILITE,

	TEXT_ENTRY_ENABLED_LEFT,
	TEXT_ENTRY_ENABLED_RIGHT,
	TEXT_ENTRY_ENABLED_CENTER,
	TEXT_ENTRY_ENABLED_SMALL_CENTER,
	TEXT_ENTRY_DISABLED_LEFT,
	TEXT_ENTRY_DISABLED_RIGHT,
	TEXT_ENTRY_DISABLED_CENTER,
	TEXT_ENTRY_DISABLED_SMALL_CENTER,
	TEXT_ENTRY_HILITE_LEFT,
	TEXT_ENTRY_HILITE_RIGHT,
	TEXT_ENTRY_HILITE_CENTER,
	TEXT_ENTRY_HILITE_SMALL_CENTER,

	TC_TAB_0_ENABLED,
	TC_TAB_0_DISABLED,
	TC_TAB_0_HILITE,
	TC_TAB_1_ENABLED,
	TC_TAB_1_DISABLED,
	TC_TAB_1_HILITE,
	TC_TAB_2_ENABLED,
	TC_TAB_2_DISABLED,
	TC_TAB_2_HILITE,
	TC_TAB_3_ENABLED,
	TC_TAB_3_DISABLED,
	TC_TAB_3_HILITE,
	TC_TAB_4_ENABLED,
	TC_TAB_4_DISABLED,
	TC_TAB_4_HILITE,
	TC_TAB_5_ENABLED,
	TC_TAB_5_DISABLED,
	TC_TAB_5_HILITE,
	TC_TAB_6_ENABLED,
	TC_TAB_6_DISABLED,
	TC_TAB_6_HILITE,
	TC_TAB_7_ENABLED,
	TC_TAB_7_DISABLED,
	TC_TAB_7_HILITE,
	TAB_CONTROL_ENABLED,
	TAB_CONTROL_DISABLED,
	TAB_CONTROL_HILITE,

	GENERIC_ENABLED,
	GENERIC_DISABLED,
	GENERIC_HILITE,

	NUM_STATE_IDENTIFIERS

} StateIdentifier;

// ImageAndColorInfo ----------------------------------------------------------
/** This is a place for us to store image and color information for
	* all the various states for each of the controls, it makes it easier
	* for us cause we can just put and get queries in this table and
	* work with only one set of dialog controls on all the property
	* dialog windows */
//-----------------------------------------------------------------------------
struct ImageAndColorInfo
{

	UnsignedInt windowType;  ///< this entry applies to these kind of windows
	StateIdentifier stateID;  ///< state identifier
	const char *stateName;  ///< string state name that this affects
	char* stateNameBuffer; ///< optional buffer for state name string
	const Image *image;  ///< the image for this entry
	Color color;  ///< color for this entry
	Color borderColor;   ///< border color for this entry

};

// INLINING ///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// EXTERNALS //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern void InitPropertiesDialog( GameWindow *window, Int x, Int y );

extern HWND InitPushButtonPropertiesDialog( GameWindow *window );
extern HWND InitRadioButtonPropertiesDialog( GameWindow *window );
extern HWND InitCheckBoxPropertiesDialog( GameWindow *window );
extern HWND InitListboxPropertiesDialog( GameWindow *window );
extern HWND InitComboBoxPropertiesDialog( GameWindow *window );
extern HWND InitProgressBarPropertiesDialog( GameWindow *window );
extern HWND InitTabControlPropertiesDialog( GameWindow *window );
extern HWND InitStaticTextPropertiesDialog( GameWindow *window );
extern HWND InitTextEntryPropertiesDialog( GameWindow *window );
extern HWND InitSliderPropertiesDialog( GameWindow *window );
extern HWND InitUserWinPropertiesDialog( GameWindow *window );

// these functions help us in managing the dialog "engine"
extern Bool HandleCommonDialogMessages( HWND hWndDialog, UINT message,
																				WPARAM wParam, LPARAM lParam,
																				Int *returnCode );
extern void CommonDialogInitialize( GameWindow *window, HWND dialog );
extern void LoadImageListComboBox( HWND comboBox );
extern void LoadHeaderTemplateListComboBox( HWND comboBox, AsciiString selected );
extern const Image *ComboBoxSelectionToImage( HWND comboBox );
extern RGBColorInt *GetControlColor( UnsignedInt controlID );
extern void SetControlColor( UnsignedInt controlID, Color color );
extern void PositionWindowOnScreen( HWND window, Int x, Int y );
extern Bool SaveCommonDialogProperties( HWND dialog, GameWindow *window );
extern void LoadStateCombo( UnsignedInt style, HWND comboBox );
extern void LoadTextStateCombo( HWND comboBox,
																Color enabled, Color enabledBorder,
																Color disabled, Color disabledBorder,
																Color hilite, Color hiliteBorder );
extern void InitCallbackCombos( HWND dialog, GameWindow *window );
extern void SaveCallbacks( GameWindow *window, HWND dialog );

extern ImageAndColorInfo *GetStateInfo( StateIdentifier id );
extern ImageAndColorInfo *GetCurrentStateInfo( HWND dialog );
void SwitchToState( StateIdentifier id, HWND dialog );
extern void StoreImageAndColor( StateIdentifier id, const Image *image,
																Color color, Color borderColor );
extern void StoreColor( StateIdentifier id, Color color, Color borderColor );
extern void LoadFontCombo( HWND comboBox, GameFont *currFont );
extern GameFont *GetSelectedFontFromCombo( HWND combo );

extern Color GetPropsEnabledTextColor( void );
extern Color GetPropsEnabledTextBorderColor( void );
extern Color GetPropsDisabledTextColor( void );
extern Color GetPropsDisabledTextBorderColor( void );
extern Color GetPropsHiliteTextColor( void );
extern Color GetPropsHiliteTextBorderColor( void );

#endif // __PROPERTIES_H_

