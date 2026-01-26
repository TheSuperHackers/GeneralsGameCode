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

// FILE: Gadget.h /////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Gadget.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Game GUI user interface gadget controls
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindow.h"
#include "GameClient/Image.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

enum
{

	GADGET_SIZE			= 16,
	HORIZONTAL_SLIDER_THUMB_WIDTH = 13,
	HORIZONTAL_SLIDER_THUMB_HEIGHT = 16,
	ENTRY_TEXT_LEN	= 256,
	STATIC_TEXT_LEN = 256,

};

// for listboxes
enum
{

	TEXT_X_OFFSET			= 5,
	TEXT_Y_OFFSET			= 2,
	TEXT_WIDTH_OFFSET	= 7,

	TOTAL_OUTLINE_HEIGHT	= 2 // Sum of heights of tom and bottom outline

};

enum
{
	LISTBOX_TEXT	= 1,
	LISTBOX_IMAGE =	2
};

// Gadget window styles, keep in same order as WindowStyleNames[]
enum
{

	GWS_PUSH_BUTTON				= 0x00000001,
	GWS_RADIO_BUTTON			= 0x00000002,
	GWS_CHECK_BOX					= 0x00000004,
	GWS_VERT_SLIDER				= 0x00000008,
	GWS_HORZ_SLIDER				= 0x00000010,
	GWS_SCROLL_LISTBOX		= 0x00000020,
	GWS_ENTRY_FIELD				= 0x00000040,
	GWS_STATIC_TEXT				= 0x00000080,
	GWS_PROGRESS_BAR			= 0x00000100,
	GWS_USER_WINDOW				= 0x00000200,
	GWS_MOUSE_TRACK				= 0x00000400,
	GWS_ANIMATED					= 0x00000800,
	GWS_TAB_STOP					= 0x00001000,
	GWS_TAB_CONTROL				= 0x00002000,
	GWS_TAB_PANE					= 0x00004000,
	GWS_COMBO_BOX					= 0x00008000,


	GWS_ALL_SLIDER = GWS_VERT_SLIDER | GWS_HORZ_SLIDER,  // for convenience

	GWS_GADGET_WINDOW			= GWS_PUSH_BUTTON |
													GWS_RADIO_BUTTON |
													GWS_TAB_CONTROL |
													GWS_CHECK_BOX |
													GWS_VERT_SLIDER |
													GWS_HORZ_SLIDER |
													GWS_SCROLL_LISTBOX |
													GWS_ENTRY_FIELD |
													GWS_STATIC_TEXT |
													GWS_COMBO_BOX		|
													GWS_PROGRESS_BAR,
};

// Gadget parameters
enum
{

	GP_DONT_UPDATE				= 0x00000001,

};

// Gadget game messages (sent to their owners)
enum GadgetGameMessage CPP_11(: Int)
{

	// Generic messages supported by all gadgets
	GGM_LEFT_DRAG = 16384,
	GGM_SET_LABEL,
	GGM_GET_LABEL,
	GGM_FOCUS_CHANGE,
	GGM_RESIZED,
	GGM_CLOSE, // This is the message that's passed to a window if it's registered as a"right Click Menu"

	// Button messages
	GBM_MOUSE_ENTERING,
	GBM_MOUSE_LEAVING,
	GBM_SELECTED,
	GBM_SELECTED_RIGHT,	// Right click selection
	GBM_SET_SELECTION,

	// Slider messages
	GSM_SLIDER_TRACK,
	GSM_SET_SLIDER,
	GSM_SET_MIN_MAX,
	GSM_SLIDER_DONE,

	// Listbox messages
	GLM_ADD_ENTRY,
	GLM_DEL_ENTRY,
	GLM_DEL_ALL,
	GLM_SELECTED,
	GLM_DOUBLE_CLICKED,
	GLM_RIGHT_CLICKED,
//	GLM_SET_INSERTPOS, // Not used since we now use multi column listboxes
	GLM_SET_SELECTION,
	GLM_GET_SELECTION,
	GLM_TOGGLE_MULTI_SELECTION,
	GLM_GET_TEXT,
//	GLM_SET_TEXT, // Not used Removed just to make sure
	GLM_SET_UP_BUTTON,
	GLM_SET_DOWN_BUTTON,
	GLM_SET_SLIDER,
	GLM_SCROLL_BUFFER,
	GLM_UPDATE_DISPLAY,
	GLM_GET_ITEM_DATA,
	GLM_SET_ITEM_DATA,

	//ComboBox Messages
	GCM_ADD_ENTRY,
	GCM_DEL_ENTRY,
	GCM_DEL_ALL,
	GCM_SELECTED,
	GCM_GET_TEXT,
	GCM_SET_TEXT,
	GCM_EDIT_DONE,
	GCM_GET_ITEM_DATA,
	GCM_SET_ITEM_DATA,
	GCM_GET_SELECTION,
	GCM_SET_SELECTION,
	GCM_UPDATE_TEXT,

	// Entry field messages
	GEM_GET_TEXT,
	GEM_SET_TEXT,
	GEM_EDIT_DONE,
	GEM_UPDATE_TEXT, //added so the parent will maintain real life status of the edit box.

	// Slider messages
	GPM_SET_PROGRESS,

};

// border types
enum
{

	BORDER_CORNER_UL = 0,
	BORDER_CORNER_UR,
	BORDER_CORNER_LL,
	BORDER_CORNER_LR,
	BORDER_VERTICAL_LEFT,
	BORDER_VERTICAL_LEFT_SHORT,
	BORDER_VERTICAL_RIGHT,
	BORDER_VERTICAL_RIGHT_SHORT,
	BORDER_HORIZONTAL_TOP,
	BORDER_HORIZONTAL_TOP_SHORT,
	BORDER_HORIZONTAL_BOTTOM,
	BORDER_HORIZONTAL_BOTTOM_SHORT,

	NUM_BORDER_PIECES

};

// GadgetMsg ------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _GadgetMsg
{

	GameWindow *window;           // Originator of message
	Int         data;             // Data field
	Int         data2;            // Data field

} GadgetMsgStruct, *GadgetMsg;

// SliderMsg ------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _SliderMsg
{

	GameWindow *window;						// Originator of message
	Int					minVal;						// Minimum slider value
	Int					maxVal;						// Maximum slider value
	Int					position;					// Current position of the slider

} SliderMsgStruct, *SliderMsg;

// ListboxMsg -----------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _ListboxMsg
{

	GameWindow *window;						// Originator of message
	Int					position;					// Position of the entry

} ListboxMsgStruct, *ListboxMsg;

// SliderData -----------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _SliderData
{

	Int					minVal;						// Minimum slider value
	Int					maxVal;						// Maximum slider value

	// The following fields are for internal use and
	// should not be initialized by the user
	Real				numTicks;					// Number of ticks between min and max
	Int					position;					// Current position of the slider

} SliderData;

// EntryData ------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _EntryData
{

	DisplayString *text;						///< the entry text
	DisplayString *sText;						///< for displaying 'secret' text
	DisplayString *constructText;		///< for foriegn text construction
	Bool secretText;								///< If TRUE text appears as astericks
	Bool numericalOnly;							///< If TRUE only numbers are allowed as input
	Bool alphaNumericalOnly;				///< If TRUE only numbers and letters are allowed as input
	Bool aSCIIOnly;									///< If TRUE ascii allowed as input
	Short maxTextLen;								///< Max length of edit text

// Colin: The very notion of entry width makes no sense to me since
// we already have a gadget width, and the max characters for
// an entry box so I am removing this.
//	Short entryWidth;								///< Width, in pixels, of the entry box

	// The following fields are for internal use and
	// should not be initialized by the user
	Bool receivedUnichar;	// If TRUE system just processed a UniChar

	Bool drawTextFromStart; // if FALSE, make sure end of text is visible

	GameWindow *constructList;	// Listbox for construct list.
	UnsignedShort charPos;			// Position of current character
	UnsignedShort conCharPos;		// Position of current contruct character

} EntryData;

// TextData -------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _TextData
{

	DisplayString *text;  ///< the text data
	Bool centered;

} TextData;

// ListEntryCell --------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _ListEntryCell
{
	Int cellType;									// Holds either LISTBOX_TEXT or LISTBOX_IMAGE
	Color						color;				// use this color
	void						*data;				// pointer to either a DisplayString or an image
	void						*userData;		// Attach user data to the cell
	Int							width;				// Used if this is an image and we don't want to use the default
	Int							height;				// used if this is an image and we don't want ot use the default
} ListEntryCell;

// ListEntryRow ---------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _ListEntryRow
{

	// The following fields are for internal use and
	// should not be initialized by the user
	Int							listHeight;		// calculated total Height at the bottom of this entry
	Int							height;				// Maintain the height of the row
	ListEntryCell		*cell;				// Holds the array of ListEntry Cells

} ListEntryRow;

// ListboxData ----------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _ListboxData
{

	Short				listLength;				// Max Number of entries in the list
	Short				columns;					// Number of Columns each line has
	Int					*columnWidthPercentage;	 // Holds the percentage value of each column in an Int array;
	Bool				autoScroll;				// If add exceeds number of lines in display
																// scroll up automatically
	Bool				autoPurge;				// If add exceeds number of entries in list
																// delete top entry automatically
	Bool				scrollBar;				// Automatically create the up/down/slider buttons
	Bool				multiSelect;			// Allow for multiple selections
	Bool				forceSelect;			// Do not allow users to unselect from a listbox
	Bool				scrollIfAtEnd;		// If we're looking at the bottom of the listbox when a new entry is added,
																// scroll up automatically

	Bool				audioFeedback;		// Audio click feedback?

	//
	// The following fields are for internal use and should not be initialized
	// by the user
	//
	Int					*columnWidth;			// Pointer to array of column widths based off of user input
	ListEntryRow	*listData;			// Pointer to an array of ListEntryRows that we create when we first create the List
	GameWindow	*upButton;					// Child window for up arrow
	GameWindow	*downButton;				// Child window for down arrow
	GameWindow	*slider;						// Child window for slider bar
	Int					totalHeight;			// total height of all entries
	Short				endPos;						// End Insertion position
	Short				insertPos;				// Insertion position

	Int					selectPos;				// Position of current selected entry (for SINGLE select)
	Int					*selections;			// Pointer to array of selections (for MULTI select)

	Short				displayHeight;		// Height in pixels of listbox display region
																// this is computed based on the existence
																// of a title or not.
	UnsignedInt doubleClickTime;	//
	Short				displayPos;				// Position of current display entry in pixels

} ListboxData;

// ComboBoxData ---------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _ComboBoxData
{
	Bool isEditable;							// Determines if the Combo box is a combo box or a dropdown box
	Int maxDisplay;								// Holds the count for the maximum displayed in the lisbox before the slider appears
	Int maxChars;									// Holds how many characters can be in the listbox/edit box.
	Bool asciiOnly;								// Used to notify the Text Entry box if it is suppose to allow only ascii characters
	Bool lettersAndNumbersOnly;   // Used to notify the Text Entry Box if it is to only allow letters and numbers
	ListboxData *listboxData;			// Needed for the listbox component of the combo box
	EntryData		*entryData;				// Needed for the text entry component of the combo box
	//
	// The following fields are for internal use and should not be initialized
	// by the user
	//
	Bool dontHide;								// A flag we'll use that'll determine if we hide the listbox or not when selected
	Int entryCount;								// Current number of entries.
	GameWindow *dropDownButton;   // Child window for drop down button
	GameWindow *editBox;          // Child window for edit box
	GameWindow *listBox;					// Child window for list box
} ComboBoxData;

// RadioButtonData ------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct _RadioButtonData
{

	Int screen;  ///< screen identifier
	Int group;  ///< group identifier

} RadioButtonData;

// PushButtonData -------------------------------------------------------------
//-----------------------------------------------------------------------------

#define NO_CLOCK			0
#define NORMAL_CLOCK	1
#define INVERSE_CLOCK	2

typedef struct _PushButtonData
{
	UnsignedByte drawClock;	///< We only want to draw the clock if, well, we want to
	Int  percentClock;			///< The percentage of the clock we want to draw
	Color colorClock;				///< The color to display the clock at
	Bool drawBorder;				///< We only want to draw the border if we want to
	Color colorBorder;			///< The color for the border around the button
	void *userData;					///< random additional data we can set
	const Image *overlayImage; ///< An overlay image (like a veterancy symbol)
	AsciiString altSound;		///< use an alternitive sound if one is set
} PushButtonData;

// TabControlData ------------------------------------------------------------
//-----------------------------------------------------------------------------
enum //Tab Position
{
	TP_CENTER,//Orientation
	TP_TOPLEFT,
	TP_BOTTOMRIGHT,

	TP_TOP_SIDE,//... on which side
	TP_RIGHT_SIDE,
	TP_LEFT_SIDE,
	TP_BOTTOM_SIDE
};

enum
{
	NUM_TAB_PANES = 8,//(MAX_DRAW_DATA - 1)
};

typedef struct _TabControlData
{
	//Set in editor
	Int tabOrientation;
	Int tabEdge;

	Int tabWidth;
	Int tabHeight;
	Int tabCount;

	GameWindow *subPanes[NUM_TAB_PANES];
	Bool subPaneDisabled[NUM_TAB_PANES];//tabCount will control how many even exist.  Individual ones can be disabled
	Int paneBorder;

	//Working computations
	Int activeTab;

	Int tabsLeftLimit;
	Int tabsRightLimit;
	Int tabsTopLimit;
	Int tabsBottomLimit;

} TabControlData;

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
extern WindowMsgHandledType GadgetPushButtonSystem( GameWindow *window, UnsignedInt msg,
																										WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetPushButtonInput( GameWindow *window, UnsignedInt msg,
																									 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetCheckBoxInput( GameWindow *window, UnsignedInt msg,
																								 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetCheckBoxSystem( GameWindow *window, UnsignedInt msg,
																									WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetRadioButtonInput( GameWindow *window, UnsignedInt msg,
																										WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetRadioButtonSystem( GameWindow *window, UnsignedInt msg,
																										 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetTabControlInput( GameWindow *window, UnsignedInt msg,
																										WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetTabControlSystem( GameWindow *window, UnsignedInt msg,
																										 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetListBoxInput( GameWindow *window, UnsignedInt msg,
																								WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetListBoxMultiInput( GameWindow *window, UnsignedInt msg,
																										 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetListBoxSystem( GameWindow *window, UnsignedInt msg,
																								 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetHorizontalSliderInput( GameWindow *window, UnsignedInt msg,
																												 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetHorizontalSliderSystem( GameWindow *window, UnsignedInt msg,
																													WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetVerticalSliderInput( GameWindow *window, UnsignedInt msg,
																											 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetVerticalSliderSystem( GameWindow *window, UnsignedInt msg,
																												WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetProgressBarSystem( GameWindow *window, UnsignedInt msg,
																										 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetStaticTextInput( GameWindow *window, UnsignedInt msg,
																									 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetStaticTextSystem( GameWindow *window, UnsignedInt msg,
																										WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetTextEntryInput( GameWindow *window, UnsignedInt msg,
																									WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetTextEntrySystem( GameWindow *window, UnsignedInt msg,
																									 WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetComboBoxInput( GameWindow *window, UnsignedInt msg,
																								WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GadgetComboBoxSystem( GameWindow *window, UnsignedInt msg,
																								 WindowMsgData mData1, WindowMsgData mData2 );


extern Bool InitializeEntryGadget( void );

extern Bool ShutdownEntryGadget( void );

// Entry Gadget Functions
extern void InformEntry( WideChar c );

// list box stuff
extern Int GetListboxTopEntry( ListboxData list );
