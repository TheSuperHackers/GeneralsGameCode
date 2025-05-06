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

// FILE: ListboxProperties.cpp ////////////////////////////////////////////////
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
// File name:  ListboxProperties.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Listbox properties dialog
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Properties.h"
#include "LayoutScheme.h"
#include "resource.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GameWindowManager.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// addScrollbar ===============================================================
/** Add a scrollbar to a listbox window that doesn't have one */
//=============================================================================
static void addScrollbar( GameWindow *listbox )
{

	// create scrollbar stuffs
	GadgetListboxCreateScrollbar( listbox );

	//
	// get the colors for the listbox and reset them to recolor the
	// newly created scrollbar parts
	//
	Color enabled = GadgetListBoxGetEnabledColor( listbox );
	Color enabledBorder = GadgetListBoxGetEnabledBorderColor( listbox );
	Color enabledSelectedItem = GadgetListBoxGetEnabledSelectedItemColor( listbox );
	Color enabledSelectedItemBorder = GadgetListBoxGetEnabledSelectedItemBorderColor( listbox );

	Color disabled = GadgetListBoxGetDisabledColor( listbox );
	Color disabledBorder = GadgetListBoxGetDisabledBorderColor( listbox );
	Color disabledSelectedItem = GadgetListBoxGetDisabledSelectedItemColor( listbox );
	Color disabledSelectedItemBorder = GadgetListBoxGetDisabledSelectedItemBorderColor( listbox );

	Color hilite = GadgetListBoxGetHiliteColor( listbox );
	Color hiliteBorder = GadgetListBoxGetHiliteBorderColor( listbox );
	Color hiliteSelectedItem = GadgetListBoxGetHiliteSelectedItemColor( listbox );
	Color hiliteSelectedItemBorder = GadgetListBoxGetHiliteSelectedItemBorderColor( listbox );

	GadgetListBoxSetColors( listbox,
													enabled,
													enabledBorder,
													enabledSelectedItem,
													enabledSelectedItemBorder,
													disabled,
													disabledBorder,
													disabledSelectedItem,
													disabledSelectedItemBorder,
													hilite,
													hiliteBorder,
													hiliteSelectedItem,
													hiliteSelectedItemBorder );

	//
	// now that colors are assigned based on the colors of the listbox
	// itself, assign the default listbox scroll images
	//
	ImageAndColorInfo *info;
	GameWindow *upButton = GadgetListBoxGetUpButton( listbox );
	if( upButton )
	{

		info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_ENABLED );
		GadgetButtonSetEnabledImage( upButton, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_ENABLED_PUSHED );
		GadgetButtonSetEnabledSelectedImage( upButton, info->image );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_DISABLED );
		GadgetButtonSetDisabledImage( upButton, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_DISABLED_PUSHED );
		GadgetButtonSetDisabledSelectedImage( upButton, info->image );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_HILITE );
		GadgetButtonSetHiliteImage( upButton, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_HILITE_PUSHED );
		GadgetButtonSetHiliteSelectedImage( upButton, info->image );

	}  // end if

	GameWindow *downButton = GadgetListBoxGetDownButton( listbox );
	if( downButton )
	{

		info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED );
		GadgetButtonSetEnabledImage( downButton, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED_PUSHED );
		GadgetButtonSetEnabledSelectedImage( downButton, info->image );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED );
		GadgetButtonSetDisabledImage( downButton, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED_PUSHED );
		GadgetButtonSetDisabledSelectedImage( downButton, info->image );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_HILITE );
		GadgetButtonSetHiliteImage( downButton, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_HILITE_PUSHED );
		GadgetButtonSetHiliteSelectedImage( downButton, info->image );

	}  // end if

	GameWindow *slider = GadgetListBoxGetSlider( listbox );
	if( slider )
	{

		// ----------------------------------------------------------------------
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_TOP );
		GadgetSliderSetEnabledImageTop( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_BOTTOM );
		GadgetSliderSetEnabledImageBottom( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_CENTER );
		GadgetSliderSetEnabledImageCenter( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_SMALL_CENTER );
		GadgetSliderSetEnabledImageSmallCenter( slider, info->image );

		// ----------------------------------------------------------------------
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_TOP );
		GadgetSliderSetDisabledImageTop( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_BOTTOM );
		GadgetSliderSetDisabledImageBottom( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_CENTER );
		GadgetSliderSetDisabledImageCenter( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_SMALL_CENTER );
		GadgetSliderSetDisabledImageSmallCenter( slider, info->image );

		// ----------------------------------------------------------------------
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_TOP );
		GadgetSliderSetHiliteImageTop( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_BOTTOM );
		GadgetSliderSetHiliteImageBottom( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_CENTER );
		GadgetSliderSetHiliteImageCenter( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_SMALL_CENTER );
		GadgetSliderSetHiliteImageSmallCenter( slider, info->image );

		//-----------------------------------------------------------------------

		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED );
		GadgetSliderSetEnabledThumbImage( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED_PUSHED );
		GadgetSliderSetEnabledSelectedThumbImage( slider, info->image );
		GadgetSliderSetEnabledSelectedThumbColor( slider, info->color );
		GadgetSliderSetEnabledSelectedThumbBorderColor( slider, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED );
		GadgetSliderSetDisabledThumbImage( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED_PUSHED );
		GadgetSliderSetDisabledSelectedThumbImage( slider, info->image );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_HILITE );
		GadgetSliderSetHiliteThumbImage( slider, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_HILITE_PUSHED );
		GadgetSliderSetHiliteSelectedThumbImage( slider, info->image );

	}  // end if, slider

}  // end addScrollbar

// removeScrollbar ============================================================
/** Remove all scrollbar constructs froma listbox that has it already */
//=============================================================================
static void removeScrollbar( GameWindow *listbox )
{
	ListboxData *listData = (ListboxData *)listbox->winGetUserData();

	// delete the up button
	TheWindowManager->winDestroy( listData->upButton );
	listData->upButton = NULL;

	// delete down button
	TheWindowManager->winDestroy( listData->downButton );
	listData->downButton = NULL;

	// delete the slider
	TheWindowManager->winDestroy( listData->slider );
	listData->slider = NULL;

	// remove the scrollbar flag from the listbox data
	listData->scrollBar = FALSE;

}  // end removeScrollbar

// resizeMaxItems =============================================================
/** Change the max items that a listbox can accomodate */
//=============================================================================
static void resizeMaxItems( GameWindow *listbox, UnsignedInt newMaxItems )
{
//	ListboxData *listData = (ListboxData *)listbox->winGetUserData();



}  // end resizeMaxItems

// listboxPropertiesCallback ==================================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK listboxPropertiesCallback( HWND hWndDialog,
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
					// using the current colors in the base of the listbox, assign a
					// reasonable color scheme to all the sub control components
					//
					info = GetStateInfo( LISTBOX_ENABLED );
					StoreColor( LISTBOX_UP_BUTTON_ENABLED, info->color, info->borderColor );
					StoreColor( LISTBOX_DOWN_BUTTON_ENABLED, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_ENABLED_TOP, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_THUMB_ENABLED, info->color, info->borderColor );

					info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_LEFT );
					StoreColor( LISTBOX_UP_BUTTON_ENABLED_PUSHED, info->color, info->borderColor );
					StoreColor( LISTBOX_DOWN_BUTTON_ENABLED_PUSHED, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, info->color, info->borderColor );

					info = GetStateInfo( LISTBOX_DISABLED );
					StoreColor( LISTBOX_UP_BUTTON_DISABLED, info->color, info->borderColor );
					StoreColor( LISTBOX_DOWN_BUTTON_DISABLED, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_DISABLED_TOP, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_THUMB_DISABLED, info->color, info->borderColor );

					info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_LEFT );
					StoreColor( LISTBOX_UP_BUTTON_DISABLED_PUSHED, info->color, info->borderColor );
					StoreColor( LISTBOX_DOWN_BUTTON_DISABLED_PUSHED, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, info->color, info->borderColor );

					info = GetStateInfo( LISTBOX_HILITE );
					StoreColor( LISTBOX_UP_BUTTON_HILITE, info->color, info->borderColor );
					StoreColor( LISTBOX_DOWN_BUTTON_HILITE, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_HILITE_TOP, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_THUMB_HILITE, info->color, info->borderColor );
					
					info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_LEFT );
					StoreColor( LISTBOX_UP_BUTTON_HILITE_PUSHED, info->color, info->borderColor );
					StoreColor( LISTBOX_DOWN_BUTTON_HILITE_PUSHED, info->color, info->borderColor );
					StoreColor( LISTBOX_SLIDER_THUMB_HILITE_PUSHED, info->color, info->borderColor );

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

						// save the common properties
						if( SaveCommonDialogProperties( hWndDialog, window ) == FALSE )
							break;

						// save the image and color data
						// ----------------------------------------------------------------
						info = GetStateInfo( LISTBOX_ENABLED );
						GadgetListBoxSetEnabledImage( window, info->image );
						GadgetListBoxSetEnabledColor( window, info->color );
						GadgetListBoxSetEnabledBorderColor( window, info->borderColor );

						info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_LEFT );
						GadgetListBoxSetEnabledSelectedItemImageLeft( window, info->image );
						GadgetListBoxSetEnabledSelectedItemColor( window, info->color );
						GadgetListBoxSetEnabledSelectedItemBorderColor( window, info->borderColor );
						info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_RIGHT );
						GadgetListBoxSetEnabledSelectedItemImageRight( window, info->image );
						info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_CENTER );
						GadgetListBoxSetEnabledSelectedItemImageCenter( window, info->image );
						info = GetStateInfo( LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
						GadgetListBoxSetEnabledSelectedItemImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( LISTBOX_DISABLED );
						GadgetListBoxSetDisabledImage( window, info->image );
						GadgetListBoxSetDisabledColor( window, info->color );
						GadgetListBoxSetDisabledBorderColor( window, info->borderColor );

						info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_LEFT );
						GadgetListBoxSetDisabledSelectedItemImageLeft( window, info->image );
						GadgetListBoxSetDisabledSelectedItemColor( window, info->color );
						GadgetListBoxSetDisabledSelectedItemBorderColor( window, info->borderColor );
						info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_RIGHT );
						GadgetListBoxSetDisabledSelectedItemImageRight( window, info->image );
						info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_CENTER );
						GadgetListBoxSetDisabledSelectedItemImageCenter( window, info->image );
						info = GetStateInfo( LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
						GadgetListBoxSetDisabledSelectedItemImageSmallCenter( window, info->image );

						// ----------------------------------------------------------------
						info = GetStateInfo( LISTBOX_HILITE );
						GadgetListBoxSetHiliteImage( window, info->image );
						GadgetListBoxSetHiliteColor( window, info->color );
						GadgetListBoxSetHiliteBorderColor( window, info->borderColor );

						info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_LEFT );
						GadgetListBoxSetHiliteSelectedItemImageLeft( window, info->image );
						GadgetListBoxSetHiliteSelectedItemColor( window, info->color );
						GadgetListBoxSetHiliteSelectedItemBorderColor( window, info->borderColor );
						info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_RIGHT );
						GadgetListBoxSetHiliteSelectedItemImageRight( window, info->image );
						info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_CENTER );
						GadgetListBoxSetHiliteSelectedItemImageCenter( window, info->image );
						info = GetStateInfo( LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
						GadgetListBoxSetHiliteSelectedItemImageSmallCenter( window, info->image );

						// up button
						GameWindow *upButton = GadgetListBoxGetUpButton( window );
						if( upButton )
						{

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_UP_BUTTON_ENABLED );
							GadgetButtonSetEnabledImage( upButton, info->image );
							GadgetButtonSetEnabledColor( upButton, info->color );
							GadgetButtonSetEnabledBorderColor( upButton, info->borderColor );

							info = GetStateInfo( LISTBOX_UP_BUTTON_ENABLED_PUSHED );
							GadgetButtonSetEnabledSelectedImage( upButton, info->image );
							GadgetButtonSetEnabledSelectedColor( upButton, info->color );
							GadgetButtonSetEnabledSelectedBorderColor( upButton, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_UP_BUTTON_DISABLED );
							GadgetButtonSetDisabledImage( upButton, info->image );
							GadgetButtonSetDisabledColor( upButton, info->color );
							GadgetButtonSetDisabledBorderColor( upButton, info->borderColor );

							info = GetStateInfo( LISTBOX_UP_BUTTON_DISABLED_PUSHED );
							GadgetButtonSetDisabledSelectedImage( upButton, info->image );
							GadgetButtonSetDisabledSelectedColor( upButton, info->color );
							GadgetButtonSetDisabledSelectedBorderColor( upButton, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_UP_BUTTON_HILITE );
							GadgetButtonSetHiliteImage( upButton, info->image );
							GadgetButtonSetHiliteColor( upButton, info->color );
							GadgetButtonSetHiliteBorderColor( upButton, info->borderColor );

							info = GetStateInfo( LISTBOX_UP_BUTTON_HILITE_PUSHED );
							GadgetButtonSetHiliteSelectedImage( upButton, info->image );
							GadgetButtonSetHiliteSelectedColor( upButton, info->color );
							GadgetButtonSetHiliteSelectedBorderColor( upButton, info->borderColor );

						}  // end if

						// down button
						GameWindow *downButton = GadgetListBoxGetDownButton( window );
						if( downButton )
						{

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_DOWN_BUTTON_ENABLED );
							GadgetButtonSetEnabledImage( downButton, info->image );
							GadgetButtonSetEnabledColor( downButton, info->color );
							GadgetButtonSetEnabledBorderColor( downButton, info->borderColor );

							info = GetStateInfo( LISTBOX_DOWN_BUTTON_ENABLED_PUSHED );
							GadgetButtonSetEnabledSelectedImage( downButton, info->image );
							GadgetButtonSetEnabledSelectedColor( downButton, info->color );
							GadgetButtonSetEnabledSelectedBorderColor( downButton, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_DOWN_BUTTON_DISABLED );
							GadgetButtonSetDisabledImage( downButton, info->image );
							GadgetButtonSetDisabledColor( downButton, info->color );
							GadgetButtonSetDisabledBorderColor( downButton, info->borderColor );

							info = GetStateInfo( LISTBOX_DOWN_BUTTON_DISABLED_PUSHED );
							GadgetButtonSetDisabledSelectedImage( downButton, info->image );
							GadgetButtonSetDisabledSelectedColor( downButton, info->color );
							GadgetButtonSetDisabledSelectedBorderColor( downButton, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_DOWN_BUTTON_HILITE );
							GadgetButtonSetHiliteImage( downButton, info->image );
							GadgetButtonSetHiliteColor( downButton, info->color );
							GadgetButtonSetHiliteBorderColor( downButton, info->borderColor );

							info = GetStateInfo( LISTBOX_DOWN_BUTTON_HILITE_PUSHED );
							GadgetButtonSetHiliteSelectedImage( downButton, info->image );
							GadgetButtonSetHiliteSelectedColor( downButton, info->color );
							GadgetButtonSetHiliteSelectedBorderColor( downButton, info->borderColor );

						}  // end if

						// slider
						GameWindow *slider = GadgetListBoxGetSlider( window );
						if( slider )
						{

							// ----------------------------------------------------------------
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

							// ----------------------------------------------------------------

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

							// ----------------------------------------------------------------

							info = GetStateInfo( LISTBOX_SLIDER_HILITE_TOP );
							GadgetSliderSetHiliteImageTop( slider, info->image );
							GadgetSliderSetHiliteColor( slider, info->color );
							GadgetSliderSetHiliteBorderColor( slider, info->borderColor );

							info = GetStateInfo( LISTBOX_SLIDER_HILITE_BOTTOM );
							GadgetSliderSetHiliteImageBottom( slider, info->image );

							info = GetStateInfo( LISTBOX_SLIDER_HILITE_CENTER );
							GadgetSliderSetHiliteImageCenter( slider, info->image );

							info = GetStateInfo( LISTBOX_SLIDER_HILITE_SMALL_CENTER );
							GadgetSliderSetHiliteImageSmallCenter( slider, info->image );

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_SLIDER_THUMB_ENABLED );
							GadgetSliderSetEnabledThumbImage( slider, info->image );
							GadgetSliderSetEnabledThumbColor( slider, info->color );
							GadgetSliderSetEnabledThumbBorderColor( slider, info->borderColor );

							info = GetStateInfo( LISTBOX_SLIDER_THUMB_ENABLED_PUSHED );
							GadgetSliderSetEnabledSelectedThumbImage( slider, info->image );
							GadgetSliderSetEnabledSelectedThumbColor( slider, info->color );
							GadgetSliderSetEnabledSelectedThumbBorderColor( slider, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_SLIDER_THUMB_DISABLED );
							GadgetSliderSetDisabledThumbImage( slider, info->image );
							GadgetSliderSetDisabledThumbColor( slider, info->color );
							GadgetSliderSetDisabledThumbBorderColor( slider, info->borderColor );

							info = GetStateInfo( LISTBOX_SLIDER_THUMB_DISABLED_PUSHED );
							GadgetSliderSetDisabledSelectedThumbImage( slider, info->image );
							GadgetSliderSetDisabledSelectedThumbColor( slider, info->color );
							GadgetSliderSetDisabledSelectedThumbBorderColor( slider, info->borderColor );

							// ----------------------------------------------------------------
							info = GetStateInfo( LISTBOX_SLIDER_THUMB_HILITE );
							GadgetSliderSetHiliteThumbImage( slider, info->image );
							GadgetSliderSetHiliteThumbColor( slider, info->color );
							GadgetSliderSetHiliteThumbBorderColor( slider, info->borderColor );

							info = GetStateInfo( LISTBOX_SLIDER_THUMB_HILITE_PUSHED );
							GadgetSliderSetHiliteSelectedThumbImage( slider, info->image );
							GadgetSliderSetHiliteSelectedThumbColor( slider, info->color );
							GadgetSliderSetHiliteSelectedThumbBorderColor( slider, info->borderColor );

						}  // end if

						// save specific list data
						ListboxData *listData = (ListboxData *)window->winGetUserData();

						listData->forceSelect		= IsDlgButtonChecked( hWndDialog, CHECK_FORCE_SELECT );
						listData->autoScroll		= IsDlgButtonChecked( hWndDialog, CHECK_AUTO_SCROLL );
						listData->scrollIfAtEnd	= IsDlgButtonChecked( hWndDialog, CHECK_SCROLL_IF_AT_END );
						listData->autoPurge			= IsDlgButtonChecked( hWndDialog, CHECK_AUTO_PURGE );

						// addition or subtraction of a scroll bar
						Bool wantScrollBar = IsDlgButtonChecked( hWndDialog, CHECK_HAS_SCROLLBAR );
						if( wantScrollBar == TRUE && listData->scrollBar == FALSE )
							addScrollbar( window );
						else if( wantScrollBar == FALSE && listData->scrollBar == TRUE )
							removeScrollbar( window );

						// change in the size of the listbox
						Int newMaxItems = GetDlgItemInt( hWndDialog, EDIT_MAX_ITEMS, NULL, FALSE );
						if( newMaxItems != listData->listLength )
							GadgetListBoxSetListLength( window, newMaxItems );

						// multi-select
						Bool wantMultiSelect = IsDlgButtonChecked( hWndDialog, CHECK_MULTI_SELECT );
						if( wantMultiSelect == TRUE && listData->multiSelect == FALSE )
							GadgetListBoxAddMultiSelect( window );
						else if( wantMultiSelect == FALSE && listData->multiSelect == TRUE )
							GadgetListBoxRemoveMultiSelect( window );


						// Wordwrap
						UnsignedInt bit = WIN_STATUS_ONE_LINE;
						window->winClearStatus( bit );
						if( IsDlgButtonChecked( hWndDialog, CHECK_NO_WORDWRAP ) )
							window->winSetStatus( bit );

						// Multi-column
						Int newColumns = GetDlgItemInt( hWndDialog, EDIT_NUM_COLUMNS,NULL,FALSE);
						
						if(newColumns > 1)
						{
							char *percentages = new char[60];
							char *token;
							GetDlgItemText(hWndDialog,EDIT_COLUMN_PERCENT,percentages,200);
							if(strlen(percentages) == 0)
							{
								MessageBox(NULL,"You have specified a column amount greater then 1, please enter the same about of percentages","whoops",MB_OK | MB_ICONSTOP | MB_APPLMODAL);
								break;
							}
														
							Int *newPercentages = new Int[newColumns];
							Int i = 0;
							Int total = 0;
							token = strtok( percentages, "," );
							while( token != NULL )
							{			
								newPercentages[i] = atoi(token);
								total += newPercentages[i];
								token = strtok( NULL, "," );
								i++;
								if(i > newColumns && token)
								{
									Char *whoopsMsg = new char[250];
									sprintf(whoopsMsg,"You have Specified %d columns but I have read in more then that for the percentages, please double check your data", newColumns);
									MessageBox(NULL, whoopsMsg,"Whoops",MB_OK | MB_ICONSTOP | MB_APPLMODAL);
									return 0;
								}
								else if( i < newColumns && !token )
								{
									Char *whoopsMsg = new char[250];
									sprintf(whoopsMsg,"You have Specified %d columns but I have read in only %d for the percentages, please double check your data", newColumns, i );
									MessageBox(NULL, whoopsMsg,"Whoops",MB_OK | MB_ICONSTOP | MB_APPLMODAL);
									return 0;
								}
								else if((total > 100 ) || (total < 100 && !token ))
								{
									Char *whoopsMsg = new char[250];
									sprintf(whoopsMsg,"Please Double check to make sure your percentages add up to 100.");
									MessageBox(NULL, whoopsMsg,"Whoops",MB_OK | MB_ICONSTOP | MB_APPLMODAL);
									return 0;
								}
							}						
							listData->columnWidthPercentage = newPercentages;
						}
						listData->columns = newColumns;

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

}  // end listboxPropertiesCallback


///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitListboxPropertiesDialog ================================================
/** Bring up the listbox properties window */
//=============================================================================
HWND InitListboxPropertiesDialog( GameWindow *window )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)LISTBOX_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)listboxPropertiesCallback );
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
	image = GadgetListBoxGetEnabledImage( window );
	color = GadgetListBoxGetEnabledColor( window );
	borderColor = GadgetListBoxGetEnabledBorderColor( window );
	StoreImageAndColor( LISTBOX_ENABLED, image, color, borderColor );

	image = GadgetListBoxGetEnabledSelectedItemImageLeft( window );
	color = GadgetListBoxGetEnabledSelectedItemColor( window );
	borderColor = GadgetListBoxGetEnabledSelectedItemBorderColor( window );
	StoreImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_LEFT, image, color, borderColor );
	image = GadgetListBoxGetEnabledSelectedItemImageRight( window );
	StoreImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetListBoxGetEnabledSelectedItemImageCenter( window );
	StoreImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetListBoxGetEnabledSelectedItemImageSmallCenter( window );
	StoreImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetListBoxGetDisabledImage( window );
	color = GadgetListBoxGetDisabledColor( window );
	borderColor = GadgetListBoxGetDisabledBorderColor( window );
	StoreImageAndColor( LISTBOX_DISABLED, image, color, borderColor );

	image = GadgetListBoxGetDisabledSelectedItemImageLeft( window );
	color = GadgetListBoxGetDisabledSelectedItemColor( window );
	borderColor = GadgetListBoxGetDisabledSelectedItemBorderColor( window );
	StoreImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_LEFT, image, color, borderColor );
	image = GadgetListBoxGetDisabledSelectedItemImageRight( window );
	StoreImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetListBoxGetDisabledSelectedItemImageCenter( window );
	StoreImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetListBoxGetDisabledSelectedItemImageSmallCenter( window );
	StoreImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	image = GadgetListBoxGetHiliteImage( window );
	color = GadgetListBoxGetHiliteColor( window );
	borderColor = GadgetListBoxGetHiliteBorderColor( window );
	StoreImageAndColor( LISTBOX_HILITE, image, color, borderColor );

	image = GadgetListBoxGetHiliteSelectedItemImageLeft( window );
	color = GadgetListBoxGetHiliteSelectedItemColor( window );
	borderColor = GadgetListBoxGetHiliteSelectedItemBorderColor( window );
	StoreImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_LEFT, image, color, borderColor );
	image = GadgetListBoxGetHiliteSelectedItemImageRight( window );
	StoreImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_RIGHT, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetListBoxGetHiliteSelectedItemImageCenter( window );
	StoreImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );
	image = GadgetListBoxGetHiliteSelectedItemImageSmallCenter( window );
	StoreImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER, image, WIN_COLOR_UNDEFINED, WIN_COLOR_UNDEFINED );

	// --------------------------------------------------------------------------
	GameWindow *upButton = GadgetListBoxGetUpButton( window );
	if( upButton )
	{

		// ------------------------------------------------------------------------
		image = GadgetButtonGetEnabledImage( upButton );
		color = GadgetButtonGetEnabledColor( upButton );
		borderColor = GadgetButtonGetEnabledBorderColor( upButton );
		StoreImageAndColor( LISTBOX_UP_BUTTON_ENABLED, image, color, borderColor );

		image = GadgetButtonGetEnabledSelectedImage( upButton );
		color = GadgetButtonGetEnabledSelectedColor( upButton );
		borderColor = GadgetButtonGetEnabledSelectedBorderColor( upButton );
		StoreImageAndColor( LISTBOX_UP_BUTTON_ENABLED_PUSHED, image, color, borderColor );

		// ------------------------------------------------------------------------
		image = GadgetButtonGetDisabledImage( upButton );
		color = GadgetButtonGetDisabledColor( upButton );
		borderColor = GadgetButtonGetDisabledBorderColor( upButton );
		StoreImageAndColor( LISTBOX_UP_BUTTON_DISABLED, image, color, borderColor );

		image = GadgetButtonGetDisabledSelectedImage( upButton );
		color = GadgetButtonGetDisabledSelectedColor( upButton );
		borderColor = GadgetButtonGetDisabledSelectedBorderColor( upButton );
		StoreImageAndColor( LISTBOX_UP_BUTTON_DISABLED_PUSHED, image, color, borderColor );

		// ------------------------------------------------------------------------
		image = GadgetButtonGetHiliteImage( upButton );
		color = GadgetButtonGetHiliteColor( upButton );
		borderColor = GadgetButtonGetHiliteBorderColor( upButton );
		StoreImageAndColor( LISTBOX_UP_BUTTON_HILITE, image, color, borderColor );

		image = GadgetButtonGetHiliteSelectedImage( upButton );
		color = GadgetButtonGetHiliteSelectedColor( upButton );
		borderColor = GadgetButtonGetHiliteSelectedBorderColor( upButton );
		StoreImageAndColor( LISTBOX_UP_BUTTON_HILITE_PUSHED, image, color, borderColor );

	}  // end if

	// --------------------------------------------------------------------------
	GameWindow *downButton = GadgetListBoxGetDownButton( window );
	if( downButton )
	{

		// ------------------------------------------------------------------------
		image = GadgetButtonGetEnabledImage( downButton );
		color = GadgetButtonGetEnabledColor( downButton );
		borderColor = GadgetButtonGetEnabledBorderColor( downButton );
		StoreImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED, image, color, borderColor );

		image = GadgetButtonGetEnabledSelectedImage( downButton );
		color = GadgetButtonGetEnabledSelectedColor( downButton );
		borderColor = GadgetButtonGetEnabledSelectedBorderColor( downButton );
		StoreImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED_PUSHED, image, color, borderColor );

		// ------------------------------------------------------------------------
		image = GadgetButtonGetDisabledImage( downButton );
		color = GadgetButtonGetDisabledColor( downButton );
		borderColor = GadgetButtonGetDisabledBorderColor( downButton );
		StoreImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED, image, color, borderColor );

		image = GadgetButtonGetDisabledSelectedImage( downButton );
		color = GadgetButtonGetDisabledSelectedColor( downButton );
		borderColor = GadgetButtonGetDisabledSelectedBorderColor( downButton );
		StoreImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED_PUSHED, image, color, borderColor );

		// ------------------------------------------------------------------------
		image = GadgetButtonGetHiliteImage( downButton );
		color = GadgetButtonGetHiliteColor( downButton );
		borderColor = GadgetButtonGetHiliteBorderColor( downButton );
		StoreImageAndColor( LISTBOX_DOWN_BUTTON_HILITE, image, color, borderColor );

		image = GadgetButtonGetHiliteSelectedImage( downButton );
		color = GadgetButtonGetHiliteSelectedColor( downButton );
		borderColor = GadgetButtonGetHiliteSelectedBorderColor( downButton );
		StoreImageAndColor( LISTBOX_DOWN_BUTTON_HILITE_PUSHED, image, color, borderColor );

	}  // end if

	GameWindow *slider = GadgetListBoxGetSlider( window );
	if( slider )
	{

		// --------------------------------------------------------------------------
		image = GadgetSliderGetEnabledImageTop( slider );
		color = GadgetSliderGetEnabledColor( slider );
		borderColor = GadgetSliderGetEnabledBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_ENABLED_TOP, image, color, borderColor );

		image = GadgetSliderGetEnabledImageBottom( slider );
		StoreImageAndColor( LISTBOX_SLIDER_ENABLED_BOTTOM, image, color, borderColor );

		image = GadgetSliderGetEnabledImageCenter( slider );
		StoreImageAndColor( LISTBOX_SLIDER_ENABLED_CENTER, image, color, borderColor );

		image = GadgetSliderGetEnabledImageSmallCenter( slider );
		StoreImageAndColor( LISTBOX_SLIDER_ENABLED_SMALL_CENTER, image, color, borderColor );

		// --------------------------------------------------------------------------
		image = GadgetSliderGetDisabledImageTop( slider );
		color = GadgetSliderGetDisabledColor( slider );
		borderColor = GadgetSliderGetDisabledBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_DISABLED_TOP, image, color, borderColor );

		image = GadgetSliderGetDisabledImageBottom( slider );
		StoreImageAndColor( LISTBOX_SLIDER_DISABLED_BOTTOM, image, color, borderColor );

		image = GadgetSliderGetDisabledImageCenter( slider );
		StoreImageAndColor( LISTBOX_SLIDER_DISABLED_CENTER, image, color, borderColor );

		image = GadgetSliderGetDisabledImageSmallCenter( slider );
		StoreImageAndColor( LISTBOX_SLIDER_DISABLED_SMALL_CENTER, image, color, borderColor );

		// --------------------------------------------------------------------------
		image = GadgetSliderGetHiliteImageTop( slider );
		color = GadgetSliderGetHiliteColor( slider );
		borderColor = GadgetSliderGetHiliteBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_HILITE_TOP, image, color, borderColor );

		image = GadgetSliderGetHiliteImageBottom( slider );
		StoreImageAndColor( LISTBOX_SLIDER_HILITE_BOTTOM, image, color, borderColor );

		image = GadgetSliderGetHiliteImageCenter( slider );
		StoreImageAndColor( LISTBOX_SLIDER_HILITE_CENTER, image, color, borderColor );

		image = GadgetSliderGetHiliteImageSmallCenter( slider );
		StoreImageAndColor( LISTBOX_SLIDER_HILITE_SMALL_CENTER, image, color, borderColor );

		// --------------------------------------------------------------------------
		image = GadgetSliderGetEnabledThumbImage( slider );
		color = GadgetSliderGetEnabledThumbColor( slider );
		borderColor = GadgetSliderGetEnabledThumbBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED, image, color, borderColor );

		image = GadgetSliderGetEnabledSelectedThumbImage( slider );
		color = GadgetSliderGetEnabledSelectedThumbColor( slider );
		borderColor = GadgetSliderGetEnabledSelectedThumbBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED_PUSHED, image, color, borderColor );

		// --------------------------------------------------------------------------
		image = GadgetSliderGetDisabledThumbImage( slider );
		color = GadgetSliderGetDisabledThumbColor( slider );
		borderColor = GadgetSliderGetDisabledThumbBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED, image, color, borderColor );

		image = GadgetSliderGetDisabledSelectedThumbImage( slider );
		color = GadgetSliderGetDisabledSelectedThumbColor( slider );
		borderColor = GadgetSliderGetDisabledSelectedThumbBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED_PUSHED, image, color, borderColor );

		// --------------------------------------------------------------------------
		image = GadgetSliderGetHiliteThumbImage( slider );
		color = GadgetSliderGetHiliteThumbColor( slider );
		borderColor = GadgetSliderGetHiliteThumbBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_THUMB_HILITE, image, color, borderColor );

		image = GadgetSliderGetHiliteSelectedThumbImage( slider );
		color = GadgetSliderGetHiliteSelectedThumbColor( slider );
		borderColor = GadgetSliderGetHiliteSelectedThumbBorderColor( slider );
		StoreImageAndColor( LISTBOX_SLIDER_THUMB_HILITE_PUSHED, image, color, borderColor );

	}  // end if

	// init listbox specific property section
	ListboxData *listData = (ListboxData *)window->winGetUserData();

	CheckDlgButton( dialog, CHECK_HAS_SCROLLBAR, listData->scrollBar );
	CheckDlgButton( dialog, CHECK_MULTI_SELECT, listData->multiSelect );
	CheckDlgButton( dialog, CHECK_FORCE_SELECT, listData->forceSelect );
	CheckDlgButton( dialog, CHECK_AUTO_SCROLL, listData->autoScroll );
	CheckDlgButton( dialog, CHECK_SCROLL_IF_AT_END, listData->scrollIfAtEnd );
	CheckDlgButton( dialog, CHECK_AUTO_PURGE, listData->autoPurge );
	SetDlgItemInt( dialog, EDIT_MAX_ITEMS, listData->listLength, FALSE );
	SetDlgItemInt( dialog, EDIT_NUM_COLUMNS, listData->columns, FALSE );
	if(listData->columns > 1)
	{
		char *percentages = new char[60];
		char *tempStr = new char[60];
		sprintf(percentages,"%d",listData->columnWidthPercentage[0]);
		for(Int i = 1; i < listData->columns; i++ )
		{
			strcat(percentages,",");
			strcat(percentages,itoa(listData->columnWidthPercentage[i],tempStr,10));
		}
		SetDlgItemText(dialog,EDIT_COLUMN_PERCENT,percentages);

	}
	// WordWrap Check Box
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ONE_LINE ) )
		CheckDlgButton( dialog, CHECK_NO_WORDWRAP, BST_CHECKED );


	// select the button enabled state for display
	SwitchToState( LISTBOX_ENABLED, dialog );

	//
	// initialize the dialog with values from the window
	//

	return dialog;

}  // end InitListboxPropertiesDialog



