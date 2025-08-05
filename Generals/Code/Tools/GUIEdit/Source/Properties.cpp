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

// FILE: Properties.cpp ///////////////////////////////////////////////////////
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
// File name:  Properties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Initializing property dialogs.  This file also contains
//						 helper functions for loading, populating, and saving
//						 properties that are common to all the property dialogs
//						 ranging from the generic window to any of the
//						 gadget controls.
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
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/HeaderTemplate.h"
#include "GUIEdit.h"
#include "Properties.h"
#include "EditWindow.h"
#include "resource.h"
#include "HierarchyView.h"
#include "GameClient/GameText.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static TextDrawData textDrawData[ 3 ];
static Int currTextIndex;
static Int enabledTextIndex,
			 disabledTextIndex,
			 hiliteTextIndex;

ColorControl colorControlTable[] =
{
	{ BUTTON_ENABLED_COLOR, { 255, 255, 255, 255 } },
	{ BUTTON_ENABLED_BORDER_COLOR, { 255, 255, 255, 255 } },
	{ BUTTON_DISABLED_COLOR, { 255, 255, 255, 255 } },
	{ BUTTON_DISABLED_BORDER_COLOR, { 255, 255, 255, 255 } },
	{ BUTTON_HILITE_COLOR, { 255, 255, 255, 255 } },
	{ BUTTON_HILITE_BORDER_COLOR, { 255, 255, 255, 255 } },

	{ BUTTON_COLOR, 0, },
	{ BUTTON_BORDER_COLOR, 0 },

	{ 0, { 0, 0, 0, 0 } }  // keep this last!
};

ImageAndColorInfo imageAndColorTable[] =
{
	{ GWS_PUSH_BUTTON, BUTTON_ENABLED, "[Button] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_PUSH_BUTTON, BUTTON_ENABLED_PUSHED, "[Button] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_PUSH_BUTTON, BUTTON_DISABLED, "[Button] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_PUSH_BUTTON, BUTTON_DISABLED_PUSHED, "[Button] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_PUSH_BUTTON, BUTTON_HILITE, "[Button] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_PUSH_BUTTON, BUTTON_HILITE_PUSHED, "[Button] Hilite (Pushed)", NULL, 0, 0 },

	{ GWS_RADIO_BUTTON, RADIO_ENABLED, "[Radio] Enabled Surface", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_ENABLED_UNCHECKED_BOX, "[Radio] Enabled Nubbin (Un-checked)", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_ENABLED_CHECKED_BOX, "[Radio] Enabled Nubbin (Checked)", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_DISABLED, "[Radio] Disabled Surface", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_DISABLED_UNCHECKED_BOX, "[Radio] Disabled Nubbin (Un-checked)", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_DISABLED_CHECKED_BOX, "[Radio] Disabled Nubbin (Checked)", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_HILITE, "[Radio] Hilite Surface", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_HILITE_UNCHECKED_BOX, "[Radio] Hilite Nubbin (Un-checked)", NULL, 0, 0 },
	{ GWS_RADIO_BUTTON, RADIO_HILITE_CHECKED_BOX, "[Radio] Hilite Nubbin (Checked)", NULL, 0, 0 },

	{ GWS_CHECK_BOX, CHECK_BOX_ENABLED, "[Check Box] Enabled Surface", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_ENABLED_UNCHECKED_BOX, "[Check Box] Enabled Box (Un-checked)", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_ENABLED_CHECKED_BOX, "[Check Box] Enabled Box (Checked)", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_DISABLED, "[Check Box] Disabled Surface", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_DISABLED_UNCHECKED_BOX, "[Check Box] Disabled Box (Un-checked)", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_DISABLED_CHECKED_BOX, "[Check Box] Disabled Box (Checked)", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_HILITE, "[Check Box] Hilite Surface", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_HILITE_UNCHECKED_BOX, "[Check Box] Hilite Box (Un-checked)", NULL, 0, 0 },
	{ GWS_CHECK_BOX, CHECK_BOX_HILITE_CHECKED_BOX, "[Check Box] Hilite Box (Checked)", NULL, 0, 0 },

	{ GWS_HORZ_SLIDER, HSLIDER_ENABLED_LEFT, "[HSlider] Enabled Left End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_ENABLED_RIGHT, "[HSlider] Enabled Right End", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_ENABLED_CENTER, "[HSlider] Enabled Repeating Center", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_ENABLED_SMALL_CENTER, "[HSlider] Enabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_DISABLED_LEFT, "[HSlider] Disabled Left End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_DISABLED_RIGHT, "[HSlider] Disabled Right End", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_DISABLED_CENTER, "[HSlider] Disabled Repeating Center", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_DISABLED_SMALL_CENTER, "[HSlider] Disabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_HILITE_LEFT, "[HSlider] Hilite Left End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_HILITE_RIGHT, "[HSlider] Hilite Right End", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_HILITE_CENTER, "[HSlider] Hilite Repeating Center", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_HILITE_SMALL_CENTER, "[HSlider] Hilite Repeating Small Cener", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_THUMB_ENABLED, "[Thumb [HSlider]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_THUMB_ENABLED_PUSHED, "[Thumb [HSlider]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_THUMB_DISABLED, "[Thumb [HSlider]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_THUMB_DISABLED_PUSHED, "[Thumb [HSlider]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_THUMB_HILITE, "[Thumb [HSlider]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_HORZ_SLIDER, HSLIDER_THUMB_HILITE_PUSHED, "[Thumb [HSlider]] Hilite (Pushed)", NULL, 0, 0 },

	{ GWS_VERT_SLIDER, VSLIDER_ENABLED_TOP, "[VSlider] Enabled Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_ENABLED_BOTTOM, "[VSlider] Enabled Bottom End", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_ENABLED_CENTER, "[VSlider] Enabled Repeating Center", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_ENABLED_SMALL_CENTER, "[VSlider] Enabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_DISABLED_TOP, "[VSlider] Disabled Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_DISABLED_BOTTOM, "[VSlider] Disabled Bottom End", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_DISABLED_CENTER, "[VSlider] Disabled Repeating Center", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_DISABLED_SMALL_CENTER, "[VSlider] Disabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_HILITE_TOP, "[VSlider] Hilite Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_HILITE_BOTTOM, "[VSlider] Hilite Bottom End", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_HILITE_CENTER, "[VSlider] Hilite Repeating Center", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_HILITE_SMALL_CENTER, "[VSlider] Hilite Repeating Small Cener", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_THUMB_ENABLED, "[Thumb [VSlider]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_THUMB_ENABLED_PUSHED, "[Thumb [VSlider]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_THUMB_DISABLED, "[Thumb [VSlider]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_THUMB_DISABLED_PUSHED, "[Thumb [VSlider]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_THUMB_HILITE, "[Thumb [VSlider]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_VERT_SLIDER, VSLIDER_THUMB_HILITE_PUSHED, "[Thumb [VSlider]] Hilite (Pushed)", NULL, 0, 0 },

	{ GWS_SCROLL_LISTBOX, LISTBOX_ENABLED,															"[Listbox] Enabled Surface", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_ENABLED_SELECTED_ITEM_LEFT,						"[Listbox] Enabled Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_ENABLED_SELECTED_ITEM_RIGHT,					"[Listbox] Enabled Selected Item Right End", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_ENABLED_SELECTED_ITEM_CENTER,					"[Listbox] Enabled Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER,		"[Listbox] Enabled Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DISABLED,															"[Listbox] Disabled Surface", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DISABLED_SELECTED_ITEM_LEFT,					"[Listbox] Disabled Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DISABLED_SELECTED_ITEM_RIGHT,					"[Listbox] Disabled Selected Item Right End", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DISABLED_SELECTED_ITEM_CENTER,				"[Listbox] Disabled Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER,	"[Listbox] Disabled Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_HILITE,																"[Listbox] Hilite Surface", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_HILITE_SELECTED_ITEM_LEFT,						"[Listbox] Hilite Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_HILITE_SELECTED_ITEM_RIGHT,						"[Listbox] Hilite Selected Item Right End", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_HILITE_SELECTED_ITEM_CENTER,					"[Listbox] Hilite Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER,		"[Listbox] Hilite Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_UP_BUTTON_ENABLED,										"[Up Button [Listbox]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_UP_BUTTON_ENABLED_PUSHED,							"[Up Button [Listbox]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_UP_BUTTON_DISABLED,										"[Up Button [Listbox]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_UP_BUTTON_DISABLED_PUSHED,						"[Up Button [Listbox]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_UP_BUTTON_HILITE,											"[Up Button [Listbox]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_UP_BUTTON_HILITE_PUSHED,							"[Up Button [Listbox]] Hilite (Pushed)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DOWN_BUTTON_ENABLED,									"[Down Button [Listbox]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DOWN_BUTTON_ENABLED_PUSHED,						"[Down Button [Listbox]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DOWN_BUTTON_DISABLED,									"[Down Button [Listbox]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DOWN_BUTTON_DISABLED_PUSHED,					"[Down Button [Listbox]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DOWN_BUTTON_HILITE,										"[Down Button [Listbox]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_DOWN_BUTTON_HILITE_PUSHED,						"[Down Button [Listbox]] Hilite (Pushed)", NULL, 0, 0 },

	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_ENABLED_TOP, "[Slider [Listbox]] Enabled Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_ENABLED_BOTTOM, "[Slider [Listbox]] Enabled Bottom End", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_ENABLED_CENTER, "[Slider [Listbox]] Enabled Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_ENABLED_SMALL_CENTER, "[Slider [Listbox]] Enabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_DISABLED_TOP, "[Slider [Listbox]] Disabled Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_DISABLED_BOTTOM, "[Slider [Listbox]] Disabled Bottom End", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_DISABLED_CENTER, "[Slider [Listbox]] Disabled Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_DISABLED_SMALL_CENTER, "[Slider [Listbox]] Disabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_HILITE_TOP, "[Slider [Listbox]] Hilite Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_HILITE_BOTTOM, "[Slider [Listbox]] Hilite Bottom End", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_HILITE_CENTER, "[Slider [Listbox]] Hilite Repeating Center", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_HILITE_SMALL_CENTER, "[Slider [Listbox]] Hilite Repeating Small Cener", NULL, 0, 0 },

	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_THUMB_ENABLED, "[Slider Thumb [Listbox]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, "[Slider Thumb [Listbox]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_THUMB_DISABLED, "[Slider Thumb [Listbox]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, "[Slider Thumb [Listbox]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_THUMB_HILITE, "[Slider Thumb [Listbox]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_SCROLL_LISTBOX, LISTBOX_SLIDER_THUMB_HILITE_PUSHED, "[Slider Thumb [Listbox]] Hilite (Pushed)", NULL, 0, 0 },

	{ GWS_COMBO_BOX, COMBOBOX_ENABLED,															"[ComboBox] Enabled Surface", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_ENABLED_SELECTED_ITEM_LEFT,						"[ComboBox] Enabled Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_ENABLED_SELECTED_ITEM_RIGHT,					"[ComboBox] Enabled Selected Item Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_ENABLED_SELECTED_ITEM_CENTER,					"[ComboBox] Enabled Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER,		"[ComboBox] Enabled Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DISABLED,															"[ComboBox] Disabled Surface", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DISABLED_SELECTED_ITEM_LEFT,					"[ComboBox] Disabled Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DISABLED_SELECTED_ITEM_RIGHT,					"[ComboBox] Disabled Selected Item Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DISABLED_SELECTED_ITEM_CENTER,				"[ComboBox] Disabled Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER,	"[ComboBox] Disabled Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_HILITE,																"[ComboBox] Hilite Surface", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_HILITE_SELECTED_ITEM_LEFT,						"[ComboBox] Hilite Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_HILITE_SELECTED_ITEM_RIGHT,						"[ComboBox] Hilite Selected Item Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_HILITE_SELECTED_ITEM_CENTER,					"[ComboBox] Hilite Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_HILITE_SELECTED_ITEM_SMALL_CENTER,		"[ComboBox] Hilite Selected Item Small Repeating Center", NULL, 0, 0 },

	{ GWS_COMBO_BOX, COMBOBOX_DROP_DOWN_BUTTON_ENABLED, "[Button [ComboBox]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED, "[Button [ComboBox]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DROP_DOWN_BUTTON_DISABLED, "[Button [ComboBox]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED, "[Button [ComboBox]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DROP_DOWN_BUTTON_HILITE, "[Button [ComboBox]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED, "[Button [ComboBox]] Hilite (Pushed)", NULL, 0, 0 },

	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_ENABLED_LEFT, "[Text Entry [ComboBox]] Enabled Left End (Or colors for no image)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_ENABLED_RIGHT, "[Text Entry [ComboBox]] Enabled Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_ENABLED_CENTER, "[Text Entry [ComboBox]] Enabled Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_ENABLED_SMALL_CENTER, "[Text Entry [ComboBox]] Enabled Small Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_DISABLED_LEFT, "[Text Entry [ComboBox]] Disabled Left End (Or colors for no image)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_DISABLED_RIGHT, "[Text Entry [ComboBox]] Disabled Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_DISABLED_CENTER, "[Text Entry [ComboBox]] Disabled Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_DISABLED_SMALL_CENTER, "[Text Entry [ComboBox]] Disabled Small Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_HILITE_LEFT, "[Text Entry [ComboBox]] Hilite Left End (Or colors for no image)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_HILITE_RIGHT, "[Text Entry [ComboBox]] Hilite Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_HILITE_CENTER, "[Text Entry [ComboBox]] Hilite Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_EDIT_BOX_HILITE_SMALL_CENTER, "[Text Entry [ComboBox]] Hilite Small Repeating Center", NULL, 0, 0 },

	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_ENABLED,															"[Listbox [ComboBox]] Enabled Surface", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT,						"[Listbox [ComboBox]] Enabled Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_RIGHT,					"[Listbox [ComboBox]] Enabled Selected Item Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_CENTER,					"[Listbox [ComboBox]] Enabled Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER,		"[Listbox [ComboBox]] Enabled Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DISABLED,															"[Listbox [ComboBox]] Disabled Surface", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_LEFT,					"[Listbox [ComboBox]] Disabled Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_RIGHT,					"[Listbox [ComboBox]] Disabled Selected Item Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_CENTER,				"[Listbox [ComboBox]] Disabled Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER,	"[Listbox [ComboBox]] Disabled Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_HILITE,																"[Listbox [ComboBox]] Hilite Surface", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_LEFT,						"[Listbox [ComboBox]] Hilite Selected Item Left End (or colors)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_RIGHT,						"[Listbox [ComboBox]] Hilite Selected Item Right End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_CENTER,					"[Listbox [ComboBox]] Hilite Selected Item Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER,		"[Listbox [ComboBox]] Hilite Selected Item Small Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_UP_BUTTON_ENABLED,										"[Up Button [Listbox]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED,							"[Up Button [Listbox [ComboBox]]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_UP_BUTTON_DISABLED,										"[Up Button [Listbox [ComboBox]]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED,						"[Up Button [Listbox [ComboBox]]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_UP_BUTTON_HILITE,											"[Up Button [Listbox [ComboBox]]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED,							"[Up Button [Listbox [ComboBox]]] Hilite (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED,									"[Down Button [Listbox [ComboBox]]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED,						"[Down Button [Listbox [ComboBox]]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED,									"[Down Button [Listbox [ComboBox]]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED,					"[Down Button [Listbox [ComboBox]]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE,										"[Down Button [Listbox [ComboBox]]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED,						"[Down Button [Listbox [ComboBox]]] Hilite (Pushed)", NULL, 0, 0 },

	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_ENABLED_TOP, "[Slider [Listbox [ComboBox]]] Enabled Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_ENABLED_BOTTOM, "[Slider [Listbox [ComboBox]]] Enabled Bottom End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_ENABLED_CENTER, "[Slider [Listbox [ComboBox]]] Enabled Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_ENABLED_SMALL_CENTER, "[Slider [Listbox [ComboBox]]] Enabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_DISABLED_TOP, "[Slider [Listbox [ComboBox]]] Disabled Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_DISABLED_BOTTOM, "[Slider [Listbox [ComboBox]]] Disabled Bottom End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_DISABLED_CENTER, "[Slider [Listbox [ComboBox]]] Disabled Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_DISABLED_SMALL_CENTER, "[Slider [Listbox [ComboBox]]] Disabled Repeating Small Cener", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_HILITE_TOP, "[Slider [Listbox [ComboBox]]] Hilite Top End (or bar colors for no image)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_HILITE_BOTTOM, "[Slider [Listbox [ComboBox]]] Hilite Bottom End", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_HILITE_CENTER, "[Slider [Listbox [ComboBox]]] Hilite Repeating Center", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_HILITE_SMALL_CENTER, "[Slider [Listbox [ComboBox]]] Hilite Repeating Small Cener", NULL, 0, 0 },

	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED, "[Slider Thumb [Listbox [ComboBox]]] Enabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, "[Slider Thumb [Listbox [ComboBox]]] Enabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED, "[Slider Thumb [Listbox [ComboBox]]] Disabled (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, "[Slider Thumb [Listbox [ComboBox]]] Disabled (Pushed)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE, "[Slider Thumb [Listbox [ComboBox]]] Hilite (Normal)", NULL, 0, 0 },
	{ GWS_COMBO_BOX, COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED, "[Slider Thumb [Listbox [ComboBox]]] Hilite (Pushed)", NULL, 0, 0 },


	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_LEFT,							"[Bar] Enabled Left End (or color for no images)", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_RIGHT,							"[Bar] Enabled Right End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_CENTER,						"[Bar] Enabled Repeating Center End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_SMALL_CENTER,			"[Bar] Enabled Small Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_BAR_LEFT,					"[Bar] Enabled Fill Bar Left End (or color for no images)", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_BAR_RIGHT,					"[Bar] Enabled Fill Bar Right End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_BAR_CENTER,				"[Bar] Enabled Fill Bar Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_ENABLED_BAR_SMALL_CENTER,	"[Bar] Enabled Fill Bar Small Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_LEFT,							"[Bar] Disabled Left End (or color for no images)", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_RIGHT,						"[Bar] Disabled Right End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_CENTER,						"[Bar] Disabled Repeating Center End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_SMALL_CENTER,			"[Bar] Disabled Small Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_BAR_LEFT,					"[Bar] Disabled Fill Bar Left End (or color for no images)", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_BAR_RIGHT,				"[Bar] Disabled Fill Bar Right End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_BAR_CENTER,				"[Bar] Disabled Fill Bar Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_DISABLED_BAR_SMALL_CENTER,	"[Bar] Disabled Fill Bar Small Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_LEFT,								"[Bar] Hilite Left End (or color for no images)", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_RIGHT,							"[Bar] Hilite Right End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_CENTER,							"[Bar] Hilite Repeating Center End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_SMALL_CENTER,				"[Bar] Hilite Small Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_BAR_LEFT,						"[Bar] Hilite Fill Bar Left End (or color for no images)", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_BAR_RIGHT,					"[Bar] Hilite Fill Bar Right End", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_BAR_CENTER,					"[Bar] Hilite Fill Bar Repeating Center", NULL, 0, 0 },
	{ GWS_PROGRESS_BAR, PROGRESS_BAR_HILITE_BAR_SMALL_CENTER,		"[Bar] Hilite Fill Bar Small Repeating Center", NULL, 0, 0 },


	{ GWS_STATIC_TEXT, STATIC_TEXT_ENABLED, "[Static Text] Enabled", NULL, 0, 0 },
	{ GWS_STATIC_TEXT, STATIC_TEXT_DISABLED, "[Static Text] Disabled", NULL, 0, 0 },
	{ GWS_STATIC_TEXT, STATIC_TEXT_HILITE, "[Static Text] Hilite", NULL, 0, 0 },

	{ GWS_ENTRY_FIELD, TEXT_ENTRY_ENABLED_LEFT, "[Text Entry] Enabled Left End (Or colors for no image)", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_ENABLED_RIGHT, "[Text Entry] Enabled Right End", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_ENABLED_CENTER, "[Text Entry] Enabled Repeating Center", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_ENABLED_SMALL_CENTER, "[Text Entry] Enabled Small Repeating Center", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_DISABLED_LEFT, "[Text Entry] Disabled Left End (Or colors for no image)", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_DISABLED_RIGHT, "[Text Entry] Disabled Right End", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_DISABLED_CENTER, "[Text Entry] Disabled Repeating Center", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_DISABLED_SMALL_CENTER, "[Text Entry] Disabled Small Repeating Center", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_HILITE_LEFT, "[Text Entry] Hilite Left End (Or colors for no image)", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_HILITE_RIGHT, "[Text Entry] Hilite Right End", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_HILITE_CENTER, "[Text Entry] Hilite Repeating Center", NULL, 0, 0 },
	{ GWS_ENTRY_FIELD, TEXT_ENTRY_HILITE_SMALL_CENTER, "[Text Entry] Hilite Small Repeating Center", NULL, 0, 0 },

	{ GWS_TAB_CONTROL, TC_TAB_0_ENABLED,					"[Tab Control] Tab 0 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_0_DISABLED,					"[Tab Control] Tab 0 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_0_HILITE,						"[Tab Control] Tab 0 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_1_ENABLED,					"[Tab Control] Tab 1 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_1_DISABLED,					"[Tab Control] Tab 1 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_1_HILITE,						"[Tab Control] Tab 1 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_2_ENABLED,					"[Tab Control] Tab 2 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_2_DISABLED,					"[Tab Control] Tab 2 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_2_HILITE,						"[Tab Control] Tab 2 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_3_ENABLED,					"[Tab Control] Tab 3 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_3_DISABLED,					"[Tab Control] Tab 3 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_3_HILITE,						"[Tab Control] Tab 3 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_4_ENABLED,					"[Tab Control] Tab 4 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_4_DISABLED,					"[Tab Control] Tab 4 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_4_HILITE,						"[Tab Control] Tab 4 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_5_ENABLED,					"[Tab Control] Tab 5 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_5_DISABLED,					"[Tab Control] Tab 5 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_5_HILITE,						"[Tab Control] Tab 5 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_6_ENABLED,					"[Tab Control] Tab 6 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_6_DISABLED,					"[Tab Control] Tab 6 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_6_HILITE,						"[Tab Control] Tab 6 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_7_ENABLED,					"[Tab Control] Tab 7 Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_7_DISABLED,					"[Tab Control] Tab 7 Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TC_TAB_7_HILITE,						"[Tab Control] Tab 7 Hilite", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TAB_CONTROL_ENABLED,				"[Tab Control] Background Surface Enabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TAB_CONTROL_DISABLED,			"[Tab Control] Background Surface Disabled", NULL, 0, 0 },
	{ GWS_TAB_CONTROL, TAB_CONTROL_HILITE,				"[Tab Control] Background Surface Hilite", NULL, 0, 0 },

	{ GWS_USER_WINDOW, GENERIC_ENABLED, "[User]Enabled Surface", NULL, 0, 0 },
	{ GWS_USER_WINDOW, GENERIC_DISABLED, "[User]Disabled Surface", NULL, 0, 0 },
	{ GWS_USER_WINDOW, GENERIC_HILITE, "[User]Hilite Surface", NULL, 0, 0 },

	{ 0, IDENTIFIER_INVALID, NULL, NULL, 0, 0 }  // keep this last!

};

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitPropertiesDialog =======================================================
/** Bring up the correct properties dialog for the window in question */
//=============================================================================
void InitPropertiesDialog( GameWindow *window, Int x, Int y )
{
	HWND dialog;
	POINT screen;

	// sanity
	if( window == NULL )
		return;

	// translate client position to screen coords of menu
	screen.x = x;
	screen.y = y;
	ClientToScreen( TheEditWindow->getWindowHandle(), &screen );

	// bring up the right dialog
	if( BitIsSet( window->winGetStyle(), GWS_PUSH_BUTTON ) )
		dialog = InitPushButtonPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_RADIO_BUTTON ) )
		dialog = InitRadioButtonPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_TAB_CONTROL ) )
		dialog = InitTabControlPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_CHECK_BOX ) )
		dialog = InitCheckBoxPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_SCROLL_LISTBOX ) )
		dialog = InitListboxPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_PROGRESS_BAR ) )
		dialog = InitProgressBarPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_STATIC_TEXT ) )
		dialog = InitStaticTextPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_ENTRY_FIELD ) )
		dialog = InitTextEntryPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_ALL_SLIDER ) )
		dialog = InitSliderPropertiesDialog( window );
	else if( BitIsSet( window->winGetStyle(), GWS_COMBO_BOX ) )
		dialog = InitComboBoxPropertiesDialog( window );
	else
		dialog = InitUserWinPropertiesDialog( window );

	// sanity check dialog
	if( dialog == NULL )
	{

		DEBUG_LOG(( "Error creating properties dialog" ));
		MessageBox( TheEditor->getWindowHandle(), "Error creating property dialog!", "Error", MB_OK );
		assert( 0 );
		return;

	}  // end if

	// save the window we're working with
	TheEditor->setPropertyTarget( window );

	//
	// position the dialog with the upper left at the mouse position or as
	// close as possible
	//
	PositionWindowOnScreen( dialog, screen.x, screen.y );

}  // end InitPropertiesDialog

// LoadFontCombo ==============================================================
/** Load the font combo with fonts currently available */
//=============================================================================
void LoadFontCombo( HWND comboBox, GameFont *currFont )
{
	GameFont *font;
	char buffer[ 256 ];
	Int index;

	// sanity
	if( comboBox == NULL || TheFontLibrary == NULL )
		return;

	// reset the combo box

	//
	// load string representations of each font and attach the font data
	// pointer to the entry
	//
	for( font = TheFontLibrary->firstFont();
			 font;
			 font = TheFontLibrary->nextFont( font ) )
	{

		// construct name
		if( font->bold )
			sprintf( buffer, "%s - Size: %d [Bold]", font->nameString.str(), font->pointSize );
		else
			sprintf( buffer, "%s - Size: %d", font->nameString.str(), font->pointSize );

		// add to combo box
		index = SendMessage( comboBox, CB_ADDSTRING, 0, (LPARAM)buffer );

		// attach pointer to font at combo index
		SendMessage( comboBox, CB_SETITEMDATA, index, (DWORD)font );

	}  // end for font

	// add a "[None]" at the top index
	SendMessage( comboBox, CB_INSERTSTRING, 0, (LPARAM)"[None]" );

	// if no font select the top index
	if( currFont == NULL )
	{

		SendMessage( comboBox, CB_SETCURSEL, 0, 0 );

	}  // end if
	else
	{
		Int count;

		// how many entries in the combo box
		count = SendMessage( comboBox, CB_GETCOUNT, 0, 0 );

		// find the entry with the matching item data and select it
		for( Int i = 0; i < count; i++ )
		{

			// get the item data here
			font = (GameFont *)SendMessage( comboBox, CB_GETITEMDATA, i, 0 );
			if( currFont == font )
			{

				// select this item in the combo box
				SendMessage( comboBox, CB_SETCURSEL, i, 0 );
				break;  // exit for i

			}  // end if

		}  // end for i

	}  // end else

}  // end LoadFontCombo

// GetSelectedFontFromCombo ===================================================
/** Based on the combo box selection return the game font associated
	* with that selection */
//=============================================================================
GameFont *GetSelectedFontFromCombo( HWND combo )
{

	// santiy
	if( combo == NULL )
		return NULL;

	// get the selected item
	Int selected;
	selected = SendMessage( combo, CB_GETCURSEL, 0, 0 );

	// index 0 is the "none" selector
	if( selected == 0 )
		return NULL;

	// get the font from the selected item
	return (GameFont *)SendMessage( combo, CB_GETITEMDATA, selected, 0 );

}  // end GetSelectedFontFromCombo

// saveFontSelection ==========================================================
/** Save the font from the currently selected item in the font dialog */
//=============================================================================
static void saveFontSelection( HWND combo, GameWindow *window )
{
	GameFont *font;

	// sanity
	if( combo == NULL || window == NULL )
		return;

	// get the font
	font = GetSelectedFontFromCombo( combo );
	window->winSetFont( font );

}  // end saveFontSelection

// saveHeaderSelection ========================================================
/** Save the Header from the currently selected item in the font dialog */
//=============================================================================
static void saveHeaderSelection( HWND comboBox, GameWindow *window )
{
	Int selected;
	char buffer[ 512 ];

	// santiy
	if( comboBox == NULL )
		return;

	// get the selected index
	selected = SendMessage( comboBox, CB_GETCURSEL, 0, 0 );

	// do nothing if index 0 is selected (contains the string "[NONE]")
	if( selected == CB_ERR || selected == 0 )
		window->winGetInstanceData()->m_headerTemplateName.clear();

	// get the text of the selected item
	SendMessage( comboBox, CB_GETLBTEXT, selected, (LPARAM)buffer );

	// return the image loc that matches the string
	window->winGetInstanceData()->m_headerTemplateName.set(buffer);

}  // end ComboBoxSelectionToImage

// loadTooltipTextLabel ==============================================================
/** Load the edit control with the window text label */
//=============================================================================
static void loadTooltipTextLabel( HWND edit, GameWindow *window )
{

	// sanity
	if( edit == NULL || window == NULL )
		return;

	// limit the text entry field in size
	SendMessage( edit, EM_LIMITTEXT, MAX_TEXT_LABEL - 1, 0 );

	// load the text
	WinInstanceData *instData = window->winGetInstanceData();

	SendMessage( edit, WM_SETTEXT, 0, (LPARAM)instData->m_tooltipString.str() );
}  // end loadTooltipTextLabel

// loadTooltipDelayTextLabel ==============================================================
/** Load the edit control with the window text label */
//=============================================================================
static void loadTooltipDelayTextLabel( HWND dialog, HWND edit, GameWindow *window )
{

	// sanity
	if( dialog == NULL || edit == NULL || window == NULL )
		return;

	// limit the text entry field in size
	SendMessage( edit, EM_LIMITTEXT, 6, 0 );

	// load the text
//	WinInstanceData *instData = window->winGetInstanceData();

//	SetDlgItemInt( dialog, edit, instData->m_tooltipDelay, TRUE );
}  // end loadTooltipDelayTextLabel

// saveTooltipTextLabel ==============================================================
/** Save the text label entry */
//=============================================================================
static void saveTooltipTextLabel( HWND edit, GameWindow *window )
{

	// sanity
	if( edit == NULL || window == NULL )
		return;

	// get the text from the edit control into the label buffer
	char buffer[ MAX_TEXT_LABEL ];
	WinInstanceData *instData = window->winGetInstanceData();
	SendMessage( edit, WM_GETTEXT, MAX_TEXT_LABEL - 1, (LPARAM)buffer );
	instData->m_tooltipString.set(buffer);
	instData->setTooltipText(TheGameText->fetch(buffer) );

}  // end saveTooltipTextLabel

// saveTooltipTextLabel ==============================================================
/** Save the text label entry */
//=============================================================================
static void saveTooltipDelayTextLabel(HWND dialog, HWND edit, GameWindow *window )
{

	// sanity
	if( dialog == NULL || edit == NULL || window == NULL )
		return;
//  WinInstanceData *instData = window->winGetInstanceData();

//  instData->m_tooltipDelay = GetDlgItemInt( dialog, edit, NULL, TRUE );

}  // end saveTooltipDelayTextLabel


// loadTextLabel ==============================================================
/** Load the edit control with the window text label */
//=============================================================================
static void loadTextLabel( HWND edit, GameWindow *window )
{

	// sanity
	if( edit == NULL || window == NULL )
		return;

	// limit the text entry field in size
	SendMessage( edit, EM_LIMITTEXT, MAX_TEXT_LABEL - 1, 0 );

	// load the text
	WinInstanceData *instData = window->winGetInstanceData();
	SendMessage( edit, WM_SETTEXT, 0, (LPARAM)instData->m_textLabelString.str() );
}  // end loadTextLabel

// saveTextLabel ==============================================================
/** Save the text label entry */
//=============================================================================
static void saveTextLabel( HWND edit, GameWindow *window )
{

	// sanity
	if( edit == NULL || window == NULL )
		return;

	// get the text from the edit control into the label buffer
	char buffer[ MAX_TEXT_LABEL ];
	WinInstanceData *instData = window->winGetInstanceData();
	SendMessage( edit, WM_GETTEXT, MAX_TEXT_LABEL - 1, (LPARAM)buffer );
	instData->m_textLabelString.set( buffer );

	//
	// set the text into the window so we can see it
	// The localization String Manger is here
	//
	UnicodeString text;
	text = TheGameText->fetch( (char *)instData->m_textLabelString.str()); //TheWindowManager->winTextLabelToText( instData->m_textLabelString );

	UnsignedInt style = window->winGetStyle();
	if( BitIsSet( style, GWS_PUSH_BUTTON ) )
		GadgetButtonSetText( window, text );
	else if( BitIsSet( style, GWS_CHECK_BOX ) )
		GadgetCheckBoxSetText( window, text );
	else if( BitIsSet( style, GWS_RADIO_BUTTON ) )
		GadgetRadioSetText( window, text );
	else if( BitIsSet( style, GWS_STATIC_TEXT ) )
		GadgetStaticTextSetText( window, text );
	else if( BitIsSet( style, GWS_ENTRY_FIELD ) )
		GadgetTextEntrySetText( window, text );
	else
		window->winSetText( text );

}  // end saveTextLavel

// LoadTextStateCombo =========================================================
/** Load the text state combo */
//=============================================================================
void LoadTextStateCombo( HWND comboBox,
												 Color enabled, Color enabledBorder,
												 Color disabled, Color disabledBorder,
												 Color hilite, Color hiliteBorder )
	{

	// sanity
	if( comboBox == NULL )
		return;

	//
	// add the three text states to the combo box and put the draw data colors
	// as the user data for each combo index
	//
	enabledTextIndex = SendMessage( comboBox, CB_INSERTSTRING, -1, (LPARAM)"Enabled Text" );
	disabledTextIndex = SendMessage( comboBox, CB_INSERTSTRING, -1, (LPARAM)"Disabled Text" );
	hiliteTextIndex = SendMessage( comboBox, CB_INSERTSTRING, -1, (LPARAM)"Hilite Text" );

	textDrawData[ enabledTextIndex ].color = enabled;
	textDrawData[ enabledTextIndex ].borderColor = enabledBorder;
	textDrawData[ disabledTextIndex ].color = disabled;
	textDrawData[ disabledTextIndex ].borderColor = disabledBorder;
	textDrawData[ hiliteTextIndex ].color = hilite;
	textDrawData[ hiliteTextIndex ].borderColor = hiliteBorder;

	// select the enabled state
	currTextIndex = 0;
	SendMessage( comboBox, CB_SETCURSEL, currTextIndex, 0 );

}  // end LoadTextStateCombo

// LoadStateCombo =============================================================
/** Load the state combo box passed in based on the window type
	* provided.  This will look through the image and color table for
	* which state entries to add as strings to the combobox */
//=============================================================================
void LoadStateCombo( UnsignedInt style, HWND comboBox )
{
	Int index;

	// sanity
	if( comboBox == NULL )
		return;

	// load the combo box with matching bit fields
	ImageAndColorInfo *entry;

	for( entry = imageAndColorTable; entry->stateName; entry++ )
	{

		if( BitIsSet( entry->windowType, style ) )
		{

			// add string
			index = SendMessage( comboBox, CB_ADDSTRING, 0, (LPARAM)entry->stateName );

			// set the state identifier as the item data of this entry
			SendMessage( comboBox, CB_SETITEMDATA, index, entry->stateID );

		}  // end if

	}  // end for entry

}  // end LoadStateCombo

// CommonDialogInitialize =====================================================
/** Called from all dialog initializations */
//=============================================================================
void CommonDialogInitialize( GameWindow *window, HWND dialog )
{
	WinInstanceData *instData;

	// sanity
	if( window == NULL || dialog == NULL )
		return;

	// get instance data
	instData = window->winGetInstanceData();

	// populate common properties
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) )
		CheckDlgButton( dialog, CHECK_ENABLED, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_DRAGABLE ) )
		CheckDlgButton( dialog, CHECK_DRAGABLE, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_HIDDEN ) )
		CheckDlgButton( dialog, CHECK_HIDDEN, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_NO_INPUT ) )
		CheckDlgButton( dialog, CHECK_NO_INPUT, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_NO_FOCUS ) )
		CheckDlgButton( dialog, CHECK_NO_FOCUS, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_BORDER ) )
		CheckDlgButton( dialog, CHECK_BORDER, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_IMAGE ) )
		CheckDlgButton( dialog, CHECK_IMAGE, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_SEE_THRU ) )
		CheckDlgButton( dialog, CHECK_SEE_THRU, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_WRAP_CENTERED ) )
		CheckDlgButton( dialog, CHECK_WRAP_CENTERED, BST_CHECKED );
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) )
		CheckDlgButton( dialog, CHECK_CHECK_LIKE, BST_CHECKED );

	//
	// limit the window name box to the max name size minus some breathing
	// room for the filename
	//
	SendMessage( GetDlgItem( dialog, EDIT_NAME ), EM_SETLIMITTEXT,
							 MAX_WINDOW_NAME_LEN - 16, 0 );

	// set the text explaining the name size limit to the user
	char buffer[ 128 ];
	sprintf( buffer, "Name length + layout filename length (.wnd) must not exceed %d characters.",
					 MAX_WINDOW_NAME_LEN );
	SetDlgItemText( dialog, STATIC_NAME_MAX, buffer );

	// set name
	SetDlgItemText( dialog, EDIT_NAME, instData->m_decoratedNameString.str() );

	// load listbox with image names
	LoadImageListComboBox( GetDlgItem( dialog, COMBO_IMAGE ) );

		// load listbox with image names
	LoadHeaderTemplateListComboBox( GetDlgItem( dialog, COMBO_HEADER ), instData->m_headerTemplateName);


	// load the combo box for available properties
	LoadStateCombo( window->winGetStyle(), GetDlgItem( dialog, COMBO_STATE ) );

	// load the text state combo box
	LoadTextStateCombo( GetDlgItem( dialog, COMBO_TEXT_STATE ),
											instData->m_enabledText.color,
											instData->m_enabledText.borderColor,
											instData->m_disabledText.color,
											instData->m_disabledText.borderColor,
											instData->m_hiliteText.color,
											instData->m_hiliteText.borderColor );

	// load the font combo if present
	HWND combo = GetDlgItem( dialog, COMBO_FONT );
	if( combo )
		LoadFontCombo( combo, window->winGetFont() );

	// load text edit control if present
	HWND edit = GetDlgItem( dialog, EDIT_TEXT_LABEL );
	if( edit )
		loadTextLabel( edit, window );

	// load text edit control if present
	HWND tooltipEdit = GetDlgItem( dialog, EDIT_TOOLTIP_TEXT );
	if( tooltipEdit )
		loadTooltipTextLabel( tooltipEdit, window );

  // load text edit control if present
	HWND tooltipDelay = GetDlgItem( dialog, EDIT_TOOLTIP_DELAY );
	if( tooltipDelay )
    SetDlgItemInt( dialog, EDIT_TOOLTIP_DELAY, instData->m_tooltipDelay, TRUE );



}  // end CommonDialogInitialize

// validateName ===============================================================
/** Validate a name before saving it into a window.  All window names
	* loaded from the current layout must have unique names */
//=============================================================================
static Bool validateName( GameWindow *root, GameWindow *exception, char *name )
{

	// end recursion, note that "" is always a valid name
	if( root == NULL || name == NULL || strlen( name ) == 0 )
		return TRUE;

	// a name cannot have a colon in it cause we use it for decoration
	if( strchr( name, ':' ) != NULL )
	{
		char buffer[ 1024 ];

		sprintf( buffer, "Names cannot have any colons (:) in them ... sorry." );
		MessageBox( TheEditor->getWindowHandle(), buffer, "Illegal Character", MB_OK );
		return FALSE;

	}  // end if

	// if this root window is not the exception window compare name
	if( root != exception )
	{
		WinInstanceData *instData = root->winGetInstanceData();

		if( strcmp( instData->m_decoratedNameString.str(), name ) == 0 )
		{
			char buffer[ 1024 ];

			sprintf( buffer, "Another window already has the name '%s'.  Duplicates are not allowed, sorry.", name );
			MessageBox( TheEditor->getWindowHandle(), buffer, "Duplicate Name", MB_OK );
			return FALSE;

		}  // end if


	}  // end if

	// check our children
	GameWindow *child;
	for( child = root->winGetChild(); child; child = child->winGetNext() )
		if( validateName( child, exception, name ) == FALSE )
			return FALSE;

	// check the next window in the list
	return validateName( root->winGetNext(), exception, name );

}  // end validateName

// adjustGadgetDrawMethods ====================================================
/** Based on the WIN_STATUS_IMAGE, set the draw callbacks to the
	* functions that will either draw images or that will draw plain */
//=============================================================================
static void adjustGadgetDrawMethods( Bool useImages, GameWindow *window )
{

	// sanity
	if( window == NULL )
		return;

	// get style of window
	UnsignedInt style = window->winGetStyle();

	if( TheEditor->windowIsGadget( window ) )//The below only applies to gadgets.
	{
		// check image or normal
		if( useImages )
		{

			if( BitIsSet( style, GWS_PUSH_BUTTON ) )
				window->winSetDrawFunc( TheWindowManager->getPushButtonImageDrawFunc() );
			else if( BitIsSet( style, GWS_RADIO_BUTTON ) )
				window->winSetDrawFunc( TheWindowManager->getRadioButtonImageDrawFunc() );
			else if( BitIsSet( style, GWS_TAB_CONTROL ) )
				window->winSetDrawFunc( TheWindowManager->getTabControlImageDrawFunc() );
			else if( BitIsSet( style, GWS_CHECK_BOX ) )
				window->winSetDrawFunc( TheWindowManager->getCheckBoxImageDrawFunc() );
			else if( BitIsSet( style, GWS_SCROLL_LISTBOX ) )
				window->winSetDrawFunc( TheWindowManager->getListBoxImageDrawFunc() );
			else if( BitIsSet( style, GWS_COMBO_BOX ) )
				window->winSetDrawFunc( TheWindowManager->getComboBoxImageDrawFunc() );
			else if( BitIsSet( style, GWS_PROGRESS_BAR ) )
				window->winSetDrawFunc( TheWindowManager->getProgressBarImageDrawFunc() );
			else if( BitIsSet( style, GWS_HORZ_SLIDER ) )
				window->winSetDrawFunc( TheWindowManager->getHorizontalSliderImageDrawFunc() );
			else if( BitIsSet( style, GWS_VERT_SLIDER ) )
				window->winSetDrawFunc( TheWindowManager->getVerticalSliderImageDrawFunc() );
			else if( BitIsSet( style, GWS_STATIC_TEXT ) )
				window->winSetDrawFunc( TheWindowManager->getStaticTextImageDrawFunc() );
			else if( BitIsSet( style, GWS_ENTRY_FIELD ) )
				window->winSetDrawFunc( TheWindowManager->getTextEntryImageDrawFunc() );
			else
			{

				DEBUG_LOG(( "Unable to adjust draw method, undefined gadget" ));
				assert( 0 );
				return;

			}  // end else

			// set the image status bit
			window->winSetStatus( WIN_STATUS_IMAGE );

		}  // end if, image set
		else
		{

			if( BitIsSet( style, GWS_PUSH_BUTTON ) )
				window->winSetDrawFunc( TheWindowManager->getPushButtonDrawFunc() );
			else if( BitIsSet( style, GWS_RADIO_BUTTON ) )
				window->winSetDrawFunc( TheWindowManager->getRadioButtonDrawFunc() );
			else if( BitIsSet( style, GWS_TAB_CONTROL ) )
				window->winSetDrawFunc( TheWindowManager->getTabControlDrawFunc() );
			else if( BitIsSet( style, GWS_CHECK_BOX ) )
				window->winSetDrawFunc( TheWindowManager->getCheckBoxDrawFunc() );
			else if( BitIsSet( style, GWS_SCROLL_LISTBOX ) )
				window->winSetDrawFunc( TheWindowManager->getListBoxDrawFunc() );
			else if( BitIsSet( style, GWS_COMBO_BOX ) )
				window->winSetDrawFunc( TheWindowManager->getComboBoxDrawFunc() );
			else if( BitIsSet( style, GWS_PROGRESS_BAR ) )
				window->winSetDrawFunc( TheWindowManager->getProgressBarDrawFunc() );
			else if( BitIsSet( style, GWS_HORZ_SLIDER ) )
				window->winSetDrawFunc( TheWindowManager->getHorizontalSliderDrawFunc() );
			else if( BitIsSet( style, GWS_VERT_SLIDER ) )
				window->winSetDrawFunc( TheWindowManager->getVerticalSliderDrawFunc() );
			else if( BitIsSet( style, GWS_STATIC_TEXT ) )
				window->winSetDrawFunc( TheWindowManager->getStaticTextDrawFunc() );
			else if( BitIsSet( style, GWS_ENTRY_FIELD ) )
				window->winSetDrawFunc( TheWindowManager->getTextEntryDrawFunc() );
			else
			{

				DEBUG_LOG(( "Unable to adjust draw method, undefined gadget" ));
				assert( 0 );
				return;

			}  // end else

			// clear the image bit
			window->winClearStatus( WIN_STATUS_IMAGE );

		}  // end else, image not set
	}//end if window is gadget

	// adjust any child gadgets
	GameWindow *child;
	for( child = window->winGetChild(); child; child = child->winGetNext() )
		adjustGadgetDrawMethods( useImages, child );

}  // end adjustGadgetDrawMethods

// SaveCommonDialogProperties =================================================
/** Save properties common on all dialogs for all windows */
//=============================================================================
Bool SaveCommonDialogProperties( HWND dialog, GameWindow *window )
{
	UnsignedInt bit;

	// sanity
	if( dialog == NULL || window == NULL )
		return FALSE;

	// get name in the name edit box
	char name[ MAX_WINDOW_NAME_LEN ];
	strcpy( name, "" );
	GetDlgItemText( dialog, EDIT_NAME, name, MAX_WINDOW_NAME_LEN );
	if( validateName( TheWindowManager->winGetWindowList(),
										window, name ) == FALSE )
		return FALSE;

	// assign the name to the window
	WinInstanceData *instData = window->winGetInstanceData();
	instData->m_decoratedNameString = name;

	// notify the hierarchy view of the change in window name
	TheHierarchyView->updateWindowName( window );

	// save bits
	window->winEnable( IsDlgButtonChecked( dialog, CHECK_ENABLED ) );

	bit = WIN_STATUS_DRAGABLE;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_DRAGABLE ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_HIDDEN;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_HIDDEN ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_NO_INPUT;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_NO_INPUT ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_NO_FOCUS;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_NO_FOCUS ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_BORDER;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_BORDER ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_IMAGE;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_IMAGE ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_SEE_THRU;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_SEE_THRU ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_WRAP_CENTERED;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_WRAP_CENTERED ) )
		window->winSetStatus( bit );

	bit = WIN_STATUS_CHECK_LIKE;
	window->winClearStatus( bit );
	if( IsDlgButtonChecked( dialog, CHECK_CHECK_LIKE ) )
		window->winSetStatus( bit );

	//
	// adjust the window callbacks for gadgets based on image status
	// or not
	//
	if( TheEditor->windowIsGadget( window ) )
		adjustGadgetDrawMethods( BitIsSet( window->winGetStatus(), WIN_STATUS_IMAGE ),
														 window );

	// save colors
	window->winSetEnabledTextColors( textDrawData[ enabledTextIndex ].color,
																	 textDrawData[ enabledTextIndex ].borderColor );
	window->winSetDisabledTextColors( textDrawData[ disabledTextIndex ].color,
																	  textDrawData[ disabledTextIndex ].borderColor );
	window->winSetHiliteTextColors( textDrawData[ hiliteTextIndex ].color,
																	textDrawData[ hiliteTextIndex ].borderColor );

	// save font data if present
	HWND fontCombo = GetDlgItem( dialog, COMBO_FONT );
	if( fontCombo )
		saveFontSelection( fontCombo, window );

	// save text label data if present
	HWND editText = GetDlgItem( dialog, EDIT_TEXT_LABEL );
	if( editText )
		saveTextLabel( editText, window );

	// save text label data if present
	HWND editTooltipText = GetDlgItem( dialog, EDIT_TOOLTIP_TEXT );
	if( editTooltipText )
		saveTooltipTextLabel( editTooltipText, window );

  	// save delay text label data if present
	HWND editTooltipDelayText = GetDlgItem( dialog, EDIT_TOOLTIP_DELAY );
	if( editTooltipDelayText )
  	instData->m_tooltipDelay = GetDlgItemInt( dialog, EDIT_TOOLTIP_DELAY, NULL, TRUE );

	HWND headerCombo = GetDlgItem( dialog, COMBO_HEADER );
	if( headerCombo )
		saveHeaderSelection( headerCombo, window );

	// contents of the editor have now changed
	TheEditor->setUnsaved( TRUE );

	return TRUE;

}  // end SaveCommonDialogProperties

// LoadImageListComboBox ======================================================
/** Load a combo box with image names from the GUI image collection
	* including a [NONE] at the top that is selected */
//=============================================================================
void LoadImageListComboBox( HWND comboBox )
{
	Image *image;

	// sanity
	if( comboBox == NULL )
		return;

	// clear the content of the box
	SendMessage( comboBox, CB_RESETCONTENT, 0, 0 );

	// load the combo box with string names from the GUI image collection
	for( image = TheMappedImageCollection->firstImage();
			 image;
			 image = TheMappedImageCollection->nextImage( image ) )
	{

		SendMessage( comboBox, CB_ADDSTRING, 0, (LPARAM)image->getName().str() );

	}  // end for image

	// add a [NONE] at the top of the image lists
	SendMessage( comboBox, CB_INSERTSTRING, 0, (LPARAM)"[NONE]" );

	// select the [NONE] label
	SendMessage( comboBox, CB_SETCURSEL, 0, 0 );

}  // end LoadImageListComboBox

// LoadHeaderTemplateListComboBox =============================================
/** Load a combo box with header template names
	* including a [NONE] at the top that is selected */
//=============================================================================
void LoadHeaderTemplateListComboBox( HWND comboBox, AsciiString selected )
{
	HeaderTemplate *ht;

	// sanity
	if( comboBox == NULL )
		return;

	// clear the content of the box
	SendMessage( comboBox, CB_RESETCONTENT, 0, 0 );

	// load the combo box with string names from the Header Templates
	for( ht = TheHeaderTemplateManager->getFirstHeader();
			 ht;
			 ht = TheHeaderTemplateManager->getNextHeader(ht) )
	{

		SendMessage( comboBox, CB_ADDSTRING, 0, (LPARAM)ht->m_name.str());

	}  // end for image

	// add a [NONE] at the top of the image lists
	SendMessage( comboBox, CB_INSERTSTRING, 0, (LPARAM)"[NONE]" );

	// select the [NONE] label
	if(selected.isEmpty())
		SendMessage( comboBox, CB_SETCURSEL, 0, 0 );
	else
		SendMessage( comboBox, CB_SELECTSTRING, -1, (LPARAM)selected.str() );


}  // end LoadHeaderTemplateListComboBox


// ComboBoxSelectionToImage ===================================================
/** Given a combo box assumed to be loaded with a list of image names,
	* if there is a selection, translate that selection into an
	* image Loc from the GUI collection
	*
	* NOTE: The image list combo boxes have a [NONE] at index 0, if that
	* is selected NULL will be returned
	*/
//=============================================================================
const Image *ComboBoxSelectionToImage( HWND comboBox )
{
	Int selected;
	char buffer[ 512 ];

	// santiy
	if( comboBox == NULL )
		return NULL;

	// get the selected index
	selected = SendMessage( comboBox, CB_GETCURSEL, 0, 0 );

	// do nothing if index 0 is selected (contains the string "[NONE]")
	if( selected == CB_ERR || selected == 0 )
		return NULL;

	// get the text of the selected item
	SendMessage( comboBox, CB_GETLBTEXT, selected, (LPARAM)buffer );

	// return the image loc that matches the string
	return TheMappedImageCollection->findImageByName( AsciiString( buffer ) );

}  // end ComboBoxSelectionToImage

// GetControlColor ============================================================
/** Search the control color table and return the color for the
	* matching ID */
//=============================================================================
RGBColorInt *GetControlColor( UnsignedInt controlID )
{
	ColorControl *entry;

	for( entry = colorControlTable; entry->controlID; entry++ )
	{

		if( entry->controlID == controlID )
			return &entry->color;

	}  // end for

	// not found
	return NULL;

}  // end GetControlColor

// SetControlColor ============================================================
/** Set the color in the table with the matching control ID */
//=============================================================================
void SetControlColor( UnsignedInt controlID, Color color )
{
	ColorControl *entry;
	UnsignedByte red, green, blue, alpha;

	// get the color components
	GameGetColorComponents( color, &red, &green, &blue, &alpha );

	for( entry = colorControlTable; entry->controlID; entry++ )
	{

		if( entry->controlID == controlID )
		{

			entry->color.alpha = alpha;
			entry->color.red = red;
			entry->color.green = green;
			entry->color.blue = blue;
			break;

		}  // end if

	}  // end for

}  // end SetControlColor

// GetStateInfo ===============================================================
/** Get a image and color state entry */
//=============================================================================
ImageAndColorInfo *GetStateInfo( StateIdentifier id )
{
	ImageAndColorInfo *entry;

	for( entry = imageAndColorTable; entry->stateName; entry++ )
	{

		if( entry->stateID == id )
			return entry;

	}  // end for entry

	return NULL;

}  // end GetStateInfo

// SwitchToState ==============================================================
/** Switch the image and color combo box to specified state, invalidate
	* the color previews and select the correct image for the new state */
//=============================================================================
void SwitchToState( StateIdentifier id, HWND dialog )
{
	HWND stateBox = GetDlgItem( dialog, COMBO_STATE );
	HWND imageBox = GetDlgItem( dialog, COMBO_IMAGE );
	HWND colorButton = GetDlgItem( dialog, BUTTON_COLOR );
	HWND borderColorButton = GetDlgItem( dialog, BUTTON_BORDER_COLOR );
	ImageAndColorInfo *info;

	// get the data for the new state
	info = GetStateInfo( id );
	if( info == NULL )
	{

		DEBUG_LOG(( "Invalid state request" ));
		assert( 0 );
		return;

	}  // end if

	// select the string in the state combo box
	SendMessage( stateBox, CB_SELECTSTRING, -1, (LPARAM)info->stateName );

	// select the image in the image combo box
	if( info->image )
		SendMessage( imageBox, CB_SELECTSTRING, -1, (LPARAM)info->image->getName().str() );
	else
		SendMessage( imageBox, CB_SETCURSEL, 0, 0 );

	//
	// invalidate the color previews, they will redraw with the new
	// state automatically
	//
	InvalidateRect( colorButton, NULL, TRUE );
	InvalidateRect( borderColorButton, NULL, TRUE );

}  // end SwitchToState

// StoreImageAndColor =========================================================
/** Store the image and colors in the table */
//=============================================================================
void StoreImageAndColor( StateIdentifier id, const Image *image,
												 Color color, Color borderColor )
{
	ImageAndColorInfo *entry;

	for( entry = imageAndColorTable; entry->stateName; entry++ )
	{

		if( entry->stateID == id )
		{

			entry->image = image;
			entry->color = color;
			entry->borderColor = borderColor;
			break;  // exit for

		}  // end if

	}  // end for

}  // end StoreImageAndColor

// StoreColor =================================================================
/** Store the colors in the table */
//=============================================================================
void StoreColor( StateIdentifier id, Color color, Color borderColor )
{
	ImageAndColorInfo *entry;

	for( entry = imageAndColorTable; entry->stateName; entry++ )
	{

		if( entry->stateID == id )
		{

			entry->color = color;
			entry->borderColor = borderColor;
			break;  // exit for

		}  // end if

	}  // end for

}  // end StoreColor

// GetCurrentStateInfo ========================================================
/** Get the info on the current state selected in the state combo */
//=============================================================================
ImageAndColorInfo *GetCurrentStateInfo( HWND dialog )
{
	HWND stateCombo = GetDlgItem( dialog, COMBO_STATE );
	Int selected;
	StateIdentifier stateID;

	// get selected state
	selected = SendMessage( stateCombo, CB_GETCURSEL, 0, 0 );
	if( selected == CB_ERR )
		return NULL;

	// get the state ID of the selected item (stored in the item data)
	stateID = (StateIdentifier)SendMessage( stateCombo, CB_GETITEMDATA, selected, 0 );

	return GetStateInfo( stateID );

}  // end GetCurrentStateInfo

// PositionWindowOnScreen =====================================================
/** Position the window on the screen, but keep the window completely
	* on-screen.  Position passed in should be in screen coordinates */
//=============================================================================
void PositionWindowOnScreen( HWND window, Int x, Int y )
{
	RECT windowRect;
	ICoord2D windowPos;
	ICoord2D windowSize;

	// get the window rectangle
	GetWindowRect( window, &windowRect );
	windowSize.x = windowRect.right - windowRect.left;
	windowSize.y = windowRect.bottom - windowRect.top;

	// get screen size not obscured by taskbar and toolbars
	RECT screenRect;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &screenRect, 0 );

	// find window position, but dont let it go outside of the screen
	if( x < screenRect.left )
		x = screenRect.left;
	if( y < screenRect.top )
		y = screenRect.top;
	windowPos.x = x;
	windowPos.y = y;
	if( windowPos.x + windowSize.x > screenRect.right )
		windowPos.x = screenRect.right - windowSize.x;
	if( windowPos.y + windowSize.y > screenRect.bottom )
		windowPos.y = screenRect.bottom - windowSize.y;

	// place the window
	MoveWindow( window, windowPos.x, windowPos.y,
							windowSize.x, windowSize.y, TRUE );

	// show the window
	ShowWindow( window, SW_SHOW );

}  // end PositionWindowOnScreen

// HandleCommonDialogMessages =================================================
/** Handle any messages common to all controls on all property dialogs */
//=============================================================================
Bool HandleCommonDialogMessages( HWND hWndDialog, UINT message,
																 WPARAM wParam, LPARAM lParam,
																 Int *returnCode )
{
	Bool used = FALSE;

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_DRAWITEM:
		{
      UINT controlID = (UINT)wParam;  // control identifier
      LPDRAWITEMSTRUCT drawItem = (LPDRAWITEMSTRUCT)lParam; // item drawing
			Color color = GAME_COLOR_UNDEFINED;
//			ImageAndColorInfo *info = GetCurrentStateInfo( hWndDialog );

			// we only care about color button controls
			if( controlID == BUTTON_COLOR || controlID == BUTTON_BORDER_COLOR )
			{
				ImageAndColorInfo *info = GetCurrentStateInfo( hWndDialog );

				if( info )
					if( controlID == BUTTON_COLOR )
						color = info->color;
					else
						color = info->borderColor;

			}  // end if
			else if( controlID == BUTTON_TEXT_COLOR || controlID == BUTTON_TEXT_BORDER_COLOR )
			{
				TextDrawData textDraw = textDrawData[ currTextIndex ];

				if( controlID == BUTTON_TEXT_COLOR )
					color = textDraw.color;
				else
					color = textDraw.borderColor;

			}  // end else if

			if( color != GAME_COLOR_UNDEFINED )
			{
				HBRUSH hBrushNew, hBrushOld;
				RECT rect;
				HWND hWndControl = GetDlgItem( hWndDialog, controlID );
				UnsignedByte r, g, b, a;

				// if this control is disabled just let windows handle drawing
				if( IsWindowEnabled( hWndControl ) == FALSE )
				{

					*returnCode = FALSE;
					break;

				}  // end if

				// get the color info
				GameGetColorComponents( color, &r, &g, &b, &a );

				// Get the area we have to draw in
				GetClientRect( hWndControl, &rect );

        // create a new brush and select it into DC
        hBrushNew = CreateSolidBrush( RGB ( r, g, b ) );
        hBrushOld = (HBRUSH)SelectObject( drawItem->hDC, hBrushNew );

        // draw the rectangle
        Rectangle( drawItem->hDC, rect.left, rect.top, rect.right, rect.bottom );

        // put the old brush back and delete the new one
        SelectObject( drawItem->hDC, hBrushOld );
        DeleteObject( hBrushNew );

        // validate this new area
        ValidateRect( hWndControl, NULL );

				// we have taken care of it
				*returnCode = TRUE;
				used = TRUE;
				break;

			}  // end if

			*returnCode = FALSE;
			break;

		}  // end draw item

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {
			Int notifyCode = HIWORD( wParam );  // notification code
			Int controlID = LOWORD( wParam );  // control ID
			HWND hWndControl = (HWND)lParam;  // control window handle

      switch( controlID )
      {

				// --------------------------------------------------------------------
				case COMBO_STATE:
				{

					// property switch to the new state
					if( notifyCode == CBN_SELCHANGE )
					{
						Int selected;
						StateIdentifier newState;

						// get new state selected
						selected = SendMessage( hWndControl, CB_GETCURSEL, 0, 0 );
						newState = (StateIdentifier)SendMessage( hWndControl, CB_GETITEMDATA, selected, 0 );
						SwitchToState( newState, hWndDialog );

					}  // end if
					used = TRUE;
					break;

				}  // end state

				// --------------------------------------------------------------------
				case COMBO_TEXT_STATE:
				{

					// invalidate text color preview boxes
					if( notifyCode == CBN_SELCHANGE )
					{

						// get the selected index as the current text data
						currTextIndex = SendMessage( hWndControl, CB_GETCURSEL, 0, 0 );

						// invalidate each of the preview windows for text colors
						InvalidateRect( GetDlgItem( hWndDialog, BUTTON_TEXT_COLOR ), NULL, TRUE );
						InvalidateRect( GetDlgItem( hWndDialog, BUTTON_TEXT_BORDER_COLOR ), NULL, TRUE );

					}  // end if
					used = TRUE;
					break;

				}  // end text state

				// --------------------------------------------------------------------
				case COMBO_IMAGE:
				{

					// store image selection changes
					if( notifyCode == CBN_SELCHANGE )
					{
						ImageAndColorInfo *info = GetCurrentStateInfo( hWndDialog );
						const Image *newImage = ComboBoxSelectionToImage( hWndControl );

						StoreImageAndColor( info->stateID, newImage,
																info->color, info->borderColor );

					}  // end if

					used = TRUE;
					break;

				}  // end image

				// --------------------------------------------------------------------
				case BUTTON_COLOR:
				case BUTTON_BORDER_COLOR:
				case BUTTON_TEXT_COLOR:
				case BUTTON_TEXT_BORDER_COLOR:
				{
					Color oldColor = GAME_COLOR_UNDEFINED;
					UnsignedByte r, g, b, a;
					ImageAndColorInfo *info = GetCurrentStateInfo( hWndDialog );

					// get the old color
					if( controlID == BUTTON_COLOR && info )
						oldColor = info->color;
					else if( controlID == BUTTON_BORDER_COLOR && info )
						oldColor = info->borderColor;
					else if( controlID == BUTTON_TEXT_COLOR )
						oldColor = textDrawData[ currTextIndex ].color;
					else if( controlID == BUTTON_TEXT_BORDER_COLOR )
						oldColor = textDrawData[ currTextIndex ].borderColor;
					else
						assert( 0 );

					GameGetColorComponents( oldColor, &r, &g, &b, &a );

					// get the mouse position for color dialog placement
					POINT mouse;
					GetCursorPos( &mouse );

					// open the color selector
					RGBColorInt *selectedColor = SelectColor( r, g, b, a, mouse.x, mouse.y );

					// store the new color
					if( selectedColor )
					{
						Color newColor = GameMakeColor( selectedColor->red,
																						selectedColor->green,
																						selectedColor->blue,
																						selectedColor->alpha );

						if( controlID == BUTTON_COLOR && info )
							StoreImageAndColor( info->stateID, info->image, newColor, info->borderColor );
						else if( controlID == BUTTON_BORDER_COLOR && info )
							StoreImageAndColor( info->stateID, info->image, info->color, newColor );
						else if( controlID == BUTTON_TEXT_COLOR )
							textDrawData[ currTextIndex ].color = newColor;
						else if( controlID == BUTTON_TEXT_BORDER_COLOR )
							textDrawData[ currTextIndex ].borderColor = newColor;
						else
							assert( 0 );

						// invalidate the color preview
						InvalidateRect( hWndControl, NULL, TRUE );

					}  // end if

					used = TRUE;
					break;

				}  // end color buttons

      }  // end switch( LOWORD( wParam ) )

      *returnCode = 0;
			break;

    } // end of WM_COMMAND

  }  // end of switch

	return used;

}  // end HandleCommonDialogMessages

// GetProprsEnabledTextColor ==================================================
//=============================================================================
Color GetPropsEnabledTextColor( void )
{
	return textDrawData[ enabledTextIndex ].color;
}

// GetPropsEnabledTextBorderColor =============================================
//=============================================================================
Color GetPropsEnabledTextBorderColor( void )
{
	return textDrawData[ enabledTextIndex ].borderColor;
}

// GetProprsDisabledTextColor =================================================
//=============================================================================
Color GetPropsDisabledTextColor( void )
{
	return textDrawData[ disabledTextIndex ].color;
}

// GetPropsDisabledTextBorderColor ============================================
//=============================================================================
Color GetPropsDisabledTextBorderColor( void )
{
	return textDrawData[ disabledTextIndex ].borderColor;
}

// GetProprsHiliteTextColor ===================================================
//=============================================================================
Color GetPropsHiliteTextColor( void )
{
	return textDrawData[ hiliteTextIndex ].color;
}

// GetPropsHiliteTextBorderColor ==============================================
//=============================================================================
Color GetPropsHiliteTextBorderColor( void )
{
	return textDrawData[ hiliteTextIndex ].borderColor;
}
