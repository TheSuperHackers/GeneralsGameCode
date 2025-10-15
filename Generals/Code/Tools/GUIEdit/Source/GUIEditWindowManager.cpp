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

// FILE: GUIEditWindowManager.cpp /////////////////////////////////////////////////////////////////
// Created:    Colin Day, July 2001
// Desc:       Window manager for the GUI edit tool, we want this up
//						 fast and to look like what we use in the game so we're going
//						 to use the WW3D window manager, and just override the
//						 drawing functions to draw lines and images to the
//						 display.  We will also be adding our own functionality
//						 here for editing and interacting with the GUI windows.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include "Common/Debug.h"
#include "GameClient/GadgetSlider.h"

#include "GUIEditWindowManager.h"
#include "EditWindow.h"
#include "GUIEdit.h"
#include "HierarchyView.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
GUIEditWindowManager *TheGUIEditWindowManager = NULL;  ///< editor use only

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Is the given widnow in the clipboard at the top level.  NOTE that
	* children are NOT included in this search */
//-------------------------------------------------------------------------------------------------
Bool GUIEditWindowManager::isWindowInClipboard( GameWindow *window,
																								GameWindow **list )
{
	GameWindow *other;

	// sanity
	if( list == NULL || window == NULL )
		return FALSE;

	// just run through the window list in the clipboard to check
	for( other = *list; other; other = other->winGetNext() )
	{

		if( window == other )
			return TRUE;  // found

	}

	return FALSE;  // not found

}

//-------------------------------------------------------------------------------------------------
/** Add the window to clipboard list */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::linkToClipboard( GameWindow *window,
																						GameWindow **list )
{

	// sanity
	if( window == NULL || list == NULL )
		return;

	// debug sanity checking, can't add if already in it
	if( isWindowInClipboard( window, list ) )
		return;

	window->winSetPrev( NULL );
	window->winSetNext( *list );
	if( *list )
		(*list)->winSetPrev( window );
	*list = window;

}

//-------------------------------------------------------------------------------------------------
/** Unlink window from the clipboard list */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::unlinkFromClipboard( GameWindow *window,
																								GameWindow **list )
{
	GameWindow *next, *prev;

	// sanity
	if( window == NULL || list == NULL )
		return;

	// debug sanity checking, can't remove if not in
	if( isWindowInClipboard( window, list ) == FALSE )
		return;

	prev = window->winGetPrev();
	next = window->winGetNext();

	if( next )
		next->winSetPrev( prev );
	if( prev )
		prev->winSetNext( next );
	else
		*list = next;

}

//-------------------------------------------------------------------------------------------------
/** Remove selection entries for child windows that also have ANY of
	* their parents in the selection list. */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::removeSupervisedChildSelections( void )
{
	WindowSelectionEntry *select, *next;
	GameWindow *window, *parent;

	// iterate selections
	for( select = TheEditor->getSelectList(); select; select = next )
	{
		Bool parentSelected;

		// get next entry
		next = select->next;

		// get window data
		window = select->window;
		parent = window->winGetParent();

		// check to see if any parent is selected
		parentSelected = FALSE;
		while( parent )
		{

			if( TheEditor->isWindowSelected( parent ) )
				parentSelected = TRUE;
			parent = parent->winGetParent();

		}

		//
		// if there is a parent selected then we can remove this
		// selection
		//
		if( parentSelected == TRUE )
			TheEditor->unSelectWindow( window );

	}

}

//-------------------------------------------------------------------------------------------------
/** Traverse the selected window list, any child windows we encounter
	* are removed from their parent and placed in the window list at
	* the top level.  Window position adjustments are made to compensate
	* for the transition from "parent space" to "screen space" */
//-------------------------------------------------------------------------------------------------
/*
void GUIEditWindowManager::orphanSelectedChildren( void )
{
	WindowSelectionEntry *select;
	GameWindow *window, *parent;

	for( select = TheEditor->getSelectList(); select; select = select->next )
	{

		// get window information
		window = select->window;
		parent = window->winGetParent();

		// we have a parent we're a candidate
		if( parent )
		{
			ICoord2D pos;

			// get the screen position of the window BEFORE we remove the parent
			window->winGetScreenPosition( &pos.x, &pos.y );

			// remove the child from the parent and add to top level of window system
			window->winSetParent( NULL );

			//
			// adjust the position, which previously was relative to the parent
			// but now is relative to the screen
			//
			window->winSetPosition( pos.x, pos.y );

		}

	}

}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GUIEditWindowManager::GUIEditWindowManager( void )
{

	m_clipboard = NULL;
	m_clipboardDup = NULL;

	m_copySpacing = 8;
	m_numCopiesPasted = 0;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GUIEditWindowManager::~GUIEditWindowManager( void )
{

	// the duplicate list is only used in the actual act of pasting
	assert( m_clipboardDup == NULL );

	// free all data on the clipboard
	resetClipboard();

}

//-------------------------------------------------------------------------------------------------
/** initialize the system */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::init( void )
{

	// extending functionality
	W3DGameWindowManager::init();

}

//-------------------------------------------------------------------------------------------------
/** Destroy a game window */
//-------------------------------------------------------------------------------------------------
Int GUIEditWindowManager::winDestroy( GameWindow *window )
{

	//
	// delete the edit data from window if present, the editor is the
	// ONLY place where this edit data is allocated anyway, it is
	// NOT present in the game
	//
	GameWindowEditData *editData = window->winGetEditData();
	if( editData )
	{

		// delete it
		delete editData;

		// set the edit data to NULL in the window
		window->winSetEditData( NULL );

	}

	// call our base class functionality
	return W3DGameWindowManager::winDestroy( window );

}

//-------------------------------------------------------------------------------------------------
/** Create a new window by setting up its parameters and callbacks. */
//-------------------------------------------------------------------------------------------------
GameWindow *GUIEditWindowManager::winCreate( GameWindow *parent,
																						 UnsignedInt status,
	 																				   Int x, Int y,
																						 Int width, Int height,
																						 GameWinSystemFunc system,
																						 WinInstanceData *instData )
{
	GameWindow *window;

	// call base class create
	window = W3DGameWindowManager::winCreate( parent, status, x, y,
																						width, height, system,
																						instData );

	// allocate edit data for the window
	if( window )
	{
		GameWindowEditData *editData = new GameWindowEditData;

		// set edit data to nothing
		memset( editData, 0, sizeof( GameWindowEditData ) );

		// attach edit data to window
		window->winSetEditData( editData );

	}

	return window;

}

//-------------------------------------------------------------------------------------------------
/** Destroy all windows in the clipboard */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::resetClipboard( void )
{
	GameWindow *window, *next;

	// iterate through all windows
	for( window = m_clipboard; window; window = next )
	{

		// get next window
		next = window->winGetNext();

		//
		// in order for the destroy to really work correctly, the system
		// assumes that the window is actually part of the system, so take it
		// away from the clipboard and add it back to the regular system
		//
		unlinkFromClipboard( window, &m_clipboard );
		linkWindow( window );

		//
		// delete the window, note that these aren't part of the active editor,
		// the live only in the clipboard land so we don't need to notify
		// the edit window or the editor that this is going away ... they can't
		// possible be selected or anything
		//
		winDestroy( window );

	}

	// immediately process the destroyed windows just to be clean here
	processDestroyList();

	// nothing in the buffer now
	m_clipboard = NULL;
	m_numCopiesPasted = 0;

}

//-------------------------------------------------------------------------------------------------
/** Is the clipboard empty */
//-------------------------------------------------------------------------------------------------
Bool GUIEditWindowManager::isClipboardEmpty( void )
{

	if( m_clipboard )
		return TRUE;

	return FALSE;

}

//-------------------------------------------------------------------------------------------------
/** Remove the selected windows from the current layout and put them into
	* the clipboard.  If the parent of a window is not being cut with the
	* child the child will have no parent and will be adjusted to absolute
	* screen coordinates with no parent */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::cutSelectedToClipboard( void )
{

	//
	// reset the clipboard just to be safe, this is done in the copy but
	// it's nice cause we depend on it here for deleting
	//
	resetClipboard();

	// first copy the selected windows to the clipboard
	copySelectedToClipboard();

	//
	// now delete the selected windows from the scene if we actually
	// copied them to the clipboard
	//
	if( m_clipboard )
		TheEditor->deleteSelected();

}

//-------------------------------------------------------------------------------------------------
/** Given the current window in a list pointed to by 'root', if that
	* window is selected, duplicate it and place it on the clipboard.  If
	* it's not selected check it's children to see if any of them are
	* selected and should be duplicated.  Then just move on down the list
	* and check the other sibling windows as well */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::duplicateSelected( GameWindow *root )
{

	// end of recursion
	if( root == NULL )
		return;

	// if widow is selected duplicate and continue on
	if( TheEditor->isWindowSelected( root ) )
	{
		GameWindow *duplicate;

		// perform the duplication of window and all children
		duplicate = duplicateWindow( root, NULL );
		if( duplicate )
		{

			//
			// if our source window had a parent, that means that the child
			// was selected, but the parent wasn't.  Therefore the new
			// duplicate has no parent, but needs to have its position
			// adjusted to be relative to the screen instead of the now
			// missing parent
			//
			GameWindow *parent = root->winGetParent();
			if( parent )
			{
				ICoord2D parentPos;
				ICoord2D pos;

				parent->winGetScreenPosition( &parentPos.x, &parentPos.y );
				duplicate->winGetScreenPosition( &pos.x, &pos.y );
				pos.x += parentPos.x;
				pos.y += parentPos.y;
				duplicate->winSetPosition( pos.x, pos.y );

			}

			// add window to the clipboard
			linkToClipboard( duplicate, &m_clipboard );

		}

	}
	else
	{

		//
		// the window itself is not selected, but some of it's children
		// may be so check them
		//
		duplicateSelected( root->winGetChild() );

	}

	// move on to the next window
	duplicateSelected( root->winGetNext() );

}

//-------------------------------------------------------------------------------------------------
/** Copy the selected windows to the clipboard */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::copySelectedToClipboard( void )
{

	// reset the contents of the clipboard
	resetClipboard();

	//
	// to simplify things, child windows that ALSO have their
	// parent window selected will become unselected, anything that
	// happens to the parent will automatically include the child
	//
	removeSupervisedChildSelections();

	//
	// go through the window list and duplicate anything selected, note
	// that the clipboard will contain the list of windows in reverse
	// order from the original order in the layout
	//
	duplicateSelected( m_windowList );

}

//-------------------------------------------------------------------------------------------------
/** Take the string representation of the name of the window passed in,
	* and add a number to it at the end.  If there is already a number
	* on it at the end, that number will be incremented */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::incrementName( GameWindow *window )
{
	WinInstanceData *instData = window->winGetInstanceData();

	// bleah, this is ugly and hokey, but if I rewrite
	// this I will botch it (cuz I'm currently not sure
	// how to test it)
	char name[MAX_WINDOW_NAME_LEN];
	strcpy(name, instData->m_decoratedNameString.str());

	Int len = strlen( name );
	char numberBuffer[ MAX_WINDOW_NAME_LEN ];
	Int i, j;
	char c;
	Bool hasNumberSuffix = FALSE;
	Int numberStartIndex = 0;

	// trivial case, just append a number to the end and get outta here
	if( len == 0 )
	{

		strlcat(name, "1", ARRAY_SIZE(name));
		goto cleanup;

	}

	//
	// start from the end of the string and back up to the start picking
	// off each character, checking if it's a number, and adding that number
	// to a string buffer.  After all numbers are read from the name
	// the string is turned real number, incremented, and replaces the
	// existing number string
	//
	for( i = 0; i < MAX_WINDOW_NAME_LEN; i++ )
		numberBuffer[ i ] = ' ';
	numberBuffer[ MAX_WINDOW_NAME_LEN - 1 ] = '\0';
	for( i = len - 1, j = MAX_WINDOW_NAME_LEN - 2; i >= 0; i-- )
	{

		c = name[ i ];
		if( isdigit( c ) )
		{

			hasNumberSuffix = TRUE;
			numberBuffer[ j-- ] = c;

		}
		else
		{

			numberStartIndex = i + 1;
			break;  // exit for i

		}

	}

	//
	// if we have a number suffix change it to a number, increment it, and
	// paste it over the old number
	//
	if( hasNumberSuffix == TRUE )
	{
		Int number = atoi( numberBuffer );
		Int charsAdded;

		// increment the number
		number++;

		// turn number back into string
		itoa( number, numberBuffer, 10 );

		// put number string OVER the original number string in the name
		len = strlen( numberBuffer );
		charsAdded = 0;
		for( i = 0; i < len; i++ )
		{
			if( i + numberStartIndex <= MAX_WINDOW_NAME_LEN - 2 )
			{

				name[ i + numberStartIndex ] = numberBuffer[ i ];
				charsAdded++;

			}
		}
		name[ numberStartIndex + charsAdded ] = 0;  // terminate at end of new string

	}
	else
	{

		// no number at end, easy ... just append a number '1'
		strlcat(name, "1", ARRAY_SIZE(name));

	}

cleanup:

	instData->m_decoratedNameString = name;

}

//-------------------------------------------------------------------------------------------------
/** Validate the names for all the windows and child windows in the copy
	* buffer that they are unique and don't collide with any windows in
	* the real system.  This is in preparation for a paste of the copy
	* buffer into the real working layout */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::validateClipboardNames( GameWindow *root )
{
	Bool nameOK = FALSE;
	WinInstanceData *instData;
	Int sanityLoop = 0;

	// end of recursion
	if( root == NULL )
		return;

	// get our inst data
	instData = root->winGetInstanceData();

	//
	// we will check our name against all names in the active layout AND
	// in the clipboard, if it's a dupe we will pick a new name and
	// try it over and over until it's OK
	//
	while( nameOK == FALSE )
	{

		// if we're not a dupe with anything anywhere we're ok
		if( !TheEditor->isNameDuplicate( m_windowList, root, instData->m_decoratedNameString ) &&
				!TheEditor->isNameDuplicate( m_clipboardDup, root, instData->m_decoratedNameString ) )
			nameOK = TRUE;
		else
			incrementName( root );  // have to change our name, so sorry :(

		//
		// and just because I don't want the possibility of a infinite loop
		// EVER to occur here this is just a sanity check that will probably
		// never get run
		//
		if( ++sanityLoop > 99999999 )
		{
			char buffer[ 1024 ];

			sprintf( buffer, "Unable to make unique name for '%s', please manually edit the name before saving.",
							 instData->m_decoratedNameString.str() );
			MessageBox( TheEditor->getWindowHandle(), buffer, "Error Mapping Names", MB_OK );
			nameOK = TRUE;

		}

	}

	// validate our children names
	GameWindow *child;
	for( child = root->winGetChild(); child; child = child->winGetNext() )
		validateClipboardNames( child );

	// validate the next window in the list
	validateClipboardNames( root->winGetNext() );

}

//-------------------------------------------------------------------------------------------------
/** Paste the contents of the clipboard into the window world */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::pasteClipboard( void )
{
	GameWindow *window, *next;
	GameWindow *firstWindow = NULL;

	// check for empty clipboard
	if( m_clipboard == NULL )
	{

		MessageBox( TheEditor->getWindowHandle(),
								"Cannot perform paste, the clipboard is empty.",
								"Clipboard Empty",
								MB_OK );
		return;

	}

	// create a duplicate of everything in the clipboard
	assert( m_clipboardDup == NULL );
	createClipboardDuplicate();

	//
	// we are about to paste the contents of the clipboard dupe into the world,
	// scan through the clipboard dupe and adjust any names that will cause
	// collisions in the current layout
	//
	validateClipboardNames( m_clipboardDup );

	// keep a pointer to the first window we will add
	firstWindow = m_clipboardDup;

	// move everything from the duplicate list into the real world
	for( window = m_clipboardDup; window; window = next )
	{

		// get next window
		next = window->winGetNext();

		// remove from clipboard
		unlinkFromClipboard( window, &m_clipboardDup );

		// place in real layout
		linkWindow( window );

		//
		// since we just pasted in a new window, offset it a little bit
		// from the original location based on how many times we're pasted
		// this set of copies.  This way they won't all stack on top of
		// each other
		//
		ICoord2D pos, safePos;

		window->winGetPosition( &pos.x, &pos.y );
		pos.x += m_copySpacing * (m_numCopiesPasted + 1);
		pos.y += m_copySpacing * (m_numCopiesPasted + 1);

		// kee the location legal
		TheEditor->computeSafeLocation( window, pos.x, pos.y,
																		&safePos.x, &safePos.y );

		// set the new location
		TheEditor->moveWindowTo( window, safePos.x, safePos.y );

		// notify the hierarchy of the new window added
		TheHierarchyView->addWindow( window, HIERARCHY_ADD_AT_TOP );

	}

	// the clipboard duplicate list is only for the act of pasting
	assert( m_clipboardDup == NULL );

	// we've now completed another successful copy pasted in
	m_numCopiesPasted++;

	//
	// now that all windows were added to the layout, and we have
	// the first window that we added ... since we know they were added
	// at the head of the window list we can traverse from the first
	// window added to the head of the window list to access each new
	// window we just pasted.  As a convenience we will unselct anything
	// selected and select all the windows we added
	//
	TheEditor->clearSelections();
	window = firstWindow;
	while( window )
	{

		TheEditor->selectWindow( window );
		window = window->winGetPrev();

		// notify the editor that each of the windows was created
		TheEditor->notifyNewWindow( window );

	}

	// if we did in fact paste a window then our file contents have changed
	if( firstWindow )
		TheEditor->setUnsaved( TRUE );

}

//-------------------------------------------------------------------------------------------------
/** Convinience funtion to copy the Draw state info for a given instance */
//-------------------------------------------------------------------------------------------------
void InstDrawCopy (  WinInstanceData *instData, WinInstanceData *sourceInstData)
{
	// do the copy of the colors for each state
	memcpy( &instData->m_enabledDrawData,
					&sourceInstData->m_enabledDrawData,
					sizeof( WinDrawData ) * MAX_DRAW_DATA );
	memcpy( &instData->m_disabledDrawData,
					&sourceInstData->m_disabledDrawData,
					sizeof( WinDrawData ) * MAX_DRAW_DATA );
	memcpy( &instData->m_hiliteDrawData,
					&sourceInstData->m_hiliteDrawData,
					sizeof( WinDrawData ) * MAX_DRAW_DATA );
}

//-------------------------------------------------------------------------------------------------
/** Duplciate a window and all its children */
//-------------------------------------------------------------------------------------------------
GameWindow *GUIEditWindowManager::duplicateWindow( GameWindow *source,
																									 GameWindow *parent )
{
	GameWindow *duplicate = NULL;
	UnsignedInt style, status;
	WinInstanceData *instData;
	WinInstanceData instDataCopy;
	ICoord2D pos, size;

	// sanity
	if( source == NULL )
		return NULL;

	// get the window instance data and make a copy of it for creating new stuff
	instData = source->winGetInstanceData();
	instDataCopy = *instData;
//	memcpy( &instDataCopy, instData, sizeof( instDataCopy ) );

	//
	// do NOT copy the display string instances, these MUST be allocated when
	// needed in real windows
	//
	instDataCopy.m_text = NULL;
	instDataCopy.m_tooltip = NULL;

	// get a few properties we're going to need
	style = source->winGetStyle();
	status = source->winGetStatus();
	source->winGetPosition( &pos.x, &pos.y );
	source->winGetSize( &size.x, &size.y );

	// create a new window or gadget of the appropriate type
	if( BitIsSet( style, GWS_PUSH_BUTTON ) )
	{

		duplicate =
			TheWindowManager->gogoGadgetPushButton( parent,
																							status,
																							pos.x,
																							pos.y,
																							size.x,
																							size.y,
																							&instDataCopy,
																							source->winGetFont(),
																							FALSE );

	}
	else if( BitIsSet( style, GWS_RADIO_BUTTON ) )
	{
		RadioButtonData *radioData = (RadioButtonData *)source->winGetUserData();
		RadioButtonData radioDataCopy;

		radioDataCopy = *radioData;
//		memcpy( &radioDataCopy, radioData, sizeof( RadioButtonData ) );

		duplicate =
			TheWindowManager->gogoGadgetRadioButton( parent,
																							 status,
																							 pos.x,
																							 pos.y,
																							 size.x,
																							 size.y,
																							 &instDataCopy,
																							 &radioDataCopy,
																							 source->winGetFont(),
																							 FALSE );

	}
	else if( BitIsSet( style, GWS_CHECK_BOX ) )
	{

		duplicate =
			TheWindowManager->gogoGadgetCheckbox( parent,
																						status,
																						pos.x,
																						pos.y,
																						size.x,
																						size.y,
																						&instDataCopy,
																						source->winGetFont(),
																						FALSE );

	}
	else if( BitIsSet( style, GWS_HORZ_SLIDER | GWS_VERT_SLIDER ) )
	{
		SliderData *sliderData = (SliderData *)source->winGetUserData();
		SliderData sliderDataCopy;

		sliderDataCopy = *sliderData;
//		memcpy( &sliderDataCopy, sliderData, sizeof( SliderData ) );

		duplicate = TheWindowManager->gogoGadgetSlider( parent,
																										status,
																										pos.x,
																										pos.y,
																										size.x,
																										size.y,
																										&instDataCopy,
																										&sliderDataCopy,
																										source->winGetFont(),
																										FALSE );

		//
		// copy the color and image schemes of the sub controls that make
		// up a slider
		//
		if( duplicate )
		{
			WinInstanceData *instData;
			WinInstanceData *sourceInstData;

			// do the slider thumb
			GameWindow *thumb = GadgetSliderGetThumb( duplicate );
			GameWindow *sourceThumb = GadgetSliderGetThumb( source );
			if( thumb && sourceThumb )
			{

				instData = thumb->winGetInstanceData();
				sourceInstData = sourceThumb->winGetInstanceData();

				// do the copy of the colors for each state
				memcpy( &instData->m_enabledDrawData,
								&sourceInstData->m_enabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_disabledDrawData,
								&sourceInstData->m_disabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_hiliteDrawData,
								&sourceInstData->m_hiliteDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );

			}

		}

	}
	else if( BitIsSet( style, GWS_COMBO_BOX ) )
	{
		ComboBoxData *comboData = (ComboBoxData *)source->winGetUserData();
		ComboBoxData comboDataCopy;

		memset( &comboDataCopy, 0, sizeof( ComboBoxData ) );

		comboDataCopy.entryData = new EntryData;
		memset ( comboDataCopy.entryData, 0, sizeof(EntryData));
		comboDataCopy.listboxData = new ListboxData;
		memset ( comboDataCopy.listboxData, 0, sizeof(ListboxData));



		comboDataCopy.entryCount = comboData->entryCount;
		comboDataCopy.isEditable = comboData->isEditable;
		comboDataCopy.maxChars = comboData->maxChars;
		comboDataCopy.maxDisplay = comboData->maxDisplay;
		comboDataCopy.lettersAndNumbersOnly = comboData->lettersAndNumbersOnly;
		comboDataCopy.asciiOnly = comboData->asciiOnly;

		comboDataCopy.listboxData->autoPurge = comboData->listboxData->autoPurge;
		comboDataCopy.listboxData->autoScroll = comboData->listboxData->autoScroll;
		comboDataCopy.listboxData->scrollIfAtEnd = comboData->listboxData->scrollIfAtEnd;
		comboDataCopy.listboxData->forceSelect = comboData->listboxData->forceSelect;
		comboDataCopy.listboxData->listLength = comboData->listboxData->listLength;
		comboDataCopy.listboxData->multiSelect = comboData->listboxData->multiSelect;
		comboDataCopy.listboxData->scrollBar = comboData->listboxData->scrollBar;
		comboDataCopy.listboxData->columns = comboData->listboxData->columns;
		comboDataCopy.listboxData->columnWidth = comboData->listboxData->columnWidth;
		comboDataCopy.listboxData->columnWidthPercentage = comboData->listboxData->columnWidthPercentage;

		comboDataCopy.entryData->alphaNumericalOnly = comboData->entryData->alphaNumericalOnly;
		comboDataCopy.entryData->aSCIIOnly = comboData->entryData->aSCIIOnly;
		comboDataCopy.entryData->maxTextLen = comboData->entryData->maxTextLen;
		comboDataCopy.entryData->numericalOnly = comboData->entryData->numericalOnly;



		duplicate =
			TheWindowManager->gogoGadgetComboBox( parent,
																					 status,
																					 pos.x,
																					 pos.y,
																					 size.x,
																					 size.y,
																					 &instDataCopy,
																					 &comboDataCopy,
																					 source->winGetFont(),
																					 FALSE );

		//
		// copy the color and image schemes of the sub controls that make
		// up a listbox
		//
		if( duplicate )
		{
			ComboBoxData *comboData = (ComboBoxData *)duplicate->winGetUserData();
			ComboBoxData *sourceComboData = (ComboBoxData *)source->winGetUserData();
			WinInstanceData *instData;
			WinInstanceData *sourceInstData;


			// drop down button
			GameWindow *dropDownButton = comboData->dropDownButton;
			GameWindow *sourceDropDownButton = sourceComboData->dropDownButton;
			if(dropDownButton && sourceDropDownButton)
			{
				instData = dropDownButton->winGetInstanceData();
				sourceInstData = sourceDropDownButton->winGetInstanceData();

				InstDrawCopy(instData,sourceInstData);
			}

			// edit box
			GameWindow *editBox = comboData->editBox;
			GameWindow *sourceEditBox = sourceComboData->editBox;
			if(editBox && sourceEditBox)
			{
				instData = editBox->winGetInstanceData();
				sourceInstData = sourceEditBox->winGetInstanceData();

				InstDrawCopy(instData,sourceInstData);
			}

			// ListBox
			GameWindow *listBox = comboData->listBox;
			GameWindow *sourceListBox = sourceComboData->listBox;
			if(listBox && sourceListBox)
			{
				instData = listBox->winGetInstanceData();
				sourceInstData = sourceListBox->winGetInstanceData();

				InstDrawCopy(instData,sourceInstData);

				// up button
				GameWindow *upButton = comboData->listboxData->upButton;
				GameWindow *sourceUpButton = sourceComboData->listboxData->upButton;
				if( upButton && sourceUpButton )
				{

					instData = upButton->winGetInstanceData();
					sourceInstData = sourceUpButton->winGetInstanceData();

					// do the copy of the colors for each state
					memcpy( &instData->m_enabledDrawData,
									&sourceInstData->m_enabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_disabledDrawData,
									&sourceInstData->m_disabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_hiliteDrawData,
									&sourceInstData->m_hiliteDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );

				}

				// down button
				GameWindow *downButton = comboData->listboxData->downButton;
				GameWindow *sourceDownButton = sourceComboData->listboxData->downButton;
				if( downButton && sourceDownButton )
				{

					instData = downButton->winGetInstanceData();
					sourceInstData = sourceDownButton->winGetInstanceData();

					// do the copy of the colors for each state
					memcpy( &instData->m_enabledDrawData,
									&sourceInstData->m_enabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_disabledDrawData,
									&sourceInstData->m_disabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_hiliteDrawData,
									&sourceInstData->m_hiliteDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );

				}

				// slider
				GameWindow *slider = comboData->listboxData->slider;
				GameWindow *sourceSlider = sourceComboData->listboxData->slider;
				if( slider && sourceSlider )
				{

					instData = slider->winGetInstanceData();
					sourceInstData = sourceSlider->winGetInstanceData();

					// do the copy of the colors for each state
					memcpy( &instData->m_enabledDrawData,
									&sourceInstData->m_enabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_disabledDrawData,
									&sourceInstData->m_disabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_hiliteDrawData,
									&sourceInstData->m_hiliteDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );

					// do the slider thumb
					GameWindow *thumb = GadgetSliderGetThumb( slider );
					GameWindow *sourceThumb = GadgetSliderGetThumb( sourceSlider );
					if( thumb && sourceThumb )
					{

						instData = thumb->winGetInstanceData();
						sourceInstData = sourceThumb->winGetInstanceData();

					// do the copy of the colors for each state
					memcpy( &instData->m_enabledDrawData,
									&sourceInstData->m_enabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_disabledDrawData,
									&sourceInstData->m_disabledDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );
					memcpy( &instData->m_hiliteDrawData,
									&sourceInstData->m_hiliteDrawData,
									sizeof( WinDrawData ) * MAX_DRAW_DATA );

					}

				}

			}

		}

	}
	else if( BitIsSet( style, GWS_SCROLL_LISTBOX ) )
	{
		ListboxData *listData = (ListboxData *)source->winGetUserData();
		ListboxData listDataCopy;

		memset( &listDataCopy, 0, sizeof( listDataCopy ) );
		listDataCopy.autoPurge = listData->autoPurge;
		listDataCopy.autoScroll = listData->autoScroll;
		listDataCopy.scrollIfAtEnd = listData->scrollIfAtEnd;
		listDataCopy.forceSelect = listData->forceSelect;
		listDataCopy.listLength = listData->listLength;
		listDataCopy.multiSelect = listData->multiSelect;
		listDataCopy.scrollBar = listData->scrollBar;
		listDataCopy.columns = listData->columns;
		if(listData->columnWidth)
		{
			listDataCopy.columnWidth = new Int[listData->columns];
			for(Int i = 0; i < listData->columns; i ++)
			{
				listDataCopy.columnWidth[i] = listData->columnWidth[i];
			}
		}
		else
			listDataCopy.columnWidth = NULL;

		if(listData->columnWidthPercentage)
		{
			listDataCopy.columnWidthPercentage = new Int[listData->columns];
			for(Int i = 0; i < listData->columns; i ++)
			{
				listDataCopy.columnWidthPercentage[i] = listData->columnWidthPercentage[i];
			}
		}
		else
			listDataCopy.columnWidthPercentage = NULL;

		duplicate =
			TheWindowManager->gogoGadgetListBox( parent,
																					 status,
																					 pos.x,
																					 pos.y,
																					 size.x,
																					 size.y,
																					 &instDataCopy,
																					 &listDataCopy,
																					 source->winGetFont(),
																					 FALSE );

		//
		// copy the color and image schemes of the sub controls that make
		// up a listbox
		//
		if( duplicate )
		{
			ListboxData *listData = (ListboxData *)duplicate->winGetUserData();
			ListboxData *sourceListData = (ListboxData *)source->winGetUserData();
			WinInstanceData *instData;
			WinInstanceData *sourceInstData;

			// up button
			GameWindow *upButton = listData->upButton;
			GameWindow *sourceUpButton = sourceListData->upButton;
			if( upButton && sourceUpButton )
			{

				instData = upButton->winGetInstanceData();
				sourceInstData = sourceUpButton->winGetInstanceData();

				// do the copy of the colors for each state
				memcpy( &instData->m_enabledDrawData,
								&sourceInstData->m_enabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_disabledDrawData,
								&sourceInstData->m_disabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_hiliteDrawData,
								&sourceInstData->m_hiliteDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );

			}

			// down button
			GameWindow *downButton = listData->downButton;
			GameWindow *sourceDownButton = sourceListData->downButton;
			if( downButton && sourceDownButton )
			{

				instData = downButton->winGetInstanceData();
				sourceInstData = sourceDownButton->winGetInstanceData();

				// do the copy of the colors for each state
				memcpy( &instData->m_enabledDrawData,
								&sourceInstData->m_enabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_disabledDrawData,
								&sourceInstData->m_disabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_hiliteDrawData,
								&sourceInstData->m_hiliteDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );

			}

			// slider
			GameWindow *slider = listData->slider;
			GameWindow *sourceSlider = sourceListData->slider;
			if( slider && sourceSlider )
			{

				instData = slider->winGetInstanceData();
				sourceInstData = sourceSlider->winGetInstanceData();

				// do the copy of the colors for each state
				memcpy( &instData->m_enabledDrawData,
								&sourceInstData->m_enabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_disabledDrawData,
								&sourceInstData->m_disabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_hiliteDrawData,
								&sourceInstData->m_hiliteDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );

				// do the slider thumb
				GameWindow *thumb = GadgetSliderGetThumb( slider );
				GameWindow *sourceThumb = GadgetSliderGetThumb( sourceSlider );
				if( thumb && sourceThumb )
				{

					instData = thumb->winGetInstanceData();
					sourceInstData = sourceThumb->winGetInstanceData();

				// do the copy of the colors for each state
				memcpy( &instData->m_enabledDrawData,
								&sourceInstData->m_enabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_disabledDrawData,
								&sourceInstData->m_disabledDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );
				memcpy( &instData->m_hiliteDrawData,
								&sourceInstData->m_hiliteDrawData,
								sizeof( WinDrawData ) * MAX_DRAW_DATA );

				}

			}

		}

	}
	else if( BitIsSet( style, GWS_ENTRY_FIELD ) )
	{
		EntryData *entryData = (EntryData *)source->winGetUserData();
		EntryData entryDataCopy;

		memset( &entryDataCopy, 0, sizeof( entryDataCopy ) );
		entryDataCopy.alphaNumericalOnly = entryData->alphaNumericalOnly;
		entryDataCopy.aSCIIOnly = entryData->aSCIIOnly;
		entryDataCopy.maxTextLen = entryData->maxTextLen;
		entryDataCopy.numericalOnly = entryData->numericalOnly;
		entryDataCopy.secretText = entryData->secretText;

		duplicate =
			TheWindowManager->gogoGadgetTextEntry( parent,
																						 status,
																						 pos.x,
																						 pos.y,
																						 size.x,
																						 size.y,
																						 &instDataCopy,
																						 &entryDataCopy,
																						 source->winGetFont(),
																						 FALSE );

	}
	else if( BitIsSet( style, GWS_STATIC_TEXT ) )
	{
		TextData *textData = (TextData *)source->winGetUserData();
		TextData textDataCopy;

		textDataCopy.centered = textData->centered;

		duplicate =
			TheWindowManager->gogoGadgetStaticText( parent,
																							status,
																							pos.x,
																							pos.y,
																							size.x,
																							size.y,
																							&instDataCopy,
																							&textDataCopy,
																							source->winGetFont(),
																							FALSE );

	}
	else if( BitIsSet( style, GWS_PROGRESS_BAR ) )
	{

		duplicate =
			TheWindowManager->gogoGadgetProgressBar( parent,
																							 status,
																							 pos.x,
																							 pos.y,
																							 size.x,
																							 size.y,
																							 &instDataCopy,
																							 source->winGetFont(),
																							 FALSE );

	}
	else if( BitIsSet( style, GWS_USER_WINDOW ) )
	{

		// create plain ol generic window
		duplicate = TheWindowManager->winCreate( parent,
																						 status,
																						 pos.x,
																						 pos.y,
																						 size.x,
																						 size.y,
																						 NULL,
																						 &instDataCopy );

	}
	else
	{

		MessageBox( TheEditor->getWindowHandle(),
								"Cannot duplicate window, undefined style.",
								"Internal Error", MB_OK );
		assert( 0 );
		memset( &instDataCopy, 0, sizeof( instDataCopy ) );  // see comment below
		return NULL;

	}

	// sanity
	if( duplicate == NULL )
	{

		MessageBox( TheEditor->getWindowHandle(), "Unable to duplicate window",
								"Internal Error", MB_OK );
		assert( 0 );
		memset( &instDataCopy, 0, sizeof( instDataCopy ) );  // see comment below
		return NULL;

	}

	//
	// since we're using the real window system here to create things, we
	// want to immediately remove it from the real system list cause we're
	// not adding windows to the layout, only duplicating windows in
	// isolation.  Note that child windows are linked to their parents
	// but that is totally isolated in the parent so that's OK and
	// necessary.
	//
	if( parent == NULL )
		unlinkWindow( duplicate );

	// copy edit data, only for the editor
	if( duplicate )
	{
		GameWindowEditData *editData = duplicate->winGetEditData();
		GameWindowEditData *sourceEditData = source->winGetEditData();

		// set edit data to zero or copy from source
		if( sourceEditData && editData )
			*editData = *sourceEditData;
//			memcpy( editData, sourceEditData, sizeof( GameWindowEditData ) );

	}

	//
	// duplicate all the children for the window, except if this window
	// is a gadget, those windows may have children but all of that
	// is already taken care of in the creation above ... a gadget should
	// be considered a single atomic unit
	//
	if( TheEditor->windowIsGadget( duplicate ) == FALSE )
	{
		GameWindow *child;

		for( child = source->winGetChild(); child; child = child->winGetNext() )
			duplicateWindow( child, duplicate );

	}

	//
	// the inst data copy was literally a copy, now it's going to go out of
	// scope and we DONT want any data to be freed that points back to the
	// original window e got it from.  So therefore we'll just blow it away
	//
	memset( &instDataCopy, 0, sizeof( instDataCopy ) );

	// return the duped window
	return duplicate;

}

//-------------------------------------------------------------------------------------------------
/** Create a duplicate everything on the clipboard list and put it
	* on the m_clipboardDup list for pasting */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::createClipboardDuplicate( void )
{
	GameWindow *duplicate;
	GameWindow *window;
	GameWindow *lastWindow;

	// find last window in clipboard
	lastWindow = m_clipboard;
	while( lastWindow && lastWindow->winGetNext() != NULL )
		lastWindow = lastWindow->winGetNext();

	//
	// go through each window in the clipboard, note we're starting from
	// the bottom and going to the front to preserve the same order
	// as the clipboard
	//
	for( window = lastWindow; window; window = window->winGetPrev() )
	{

		// duplicate the window and all its children
		duplicate = duplicateWindow( window, NULL );

		// add duplicate to list
		if( duplicate )
			linkToClipboard( duplicate, &m_clipboardDup );

	}

}

//-------------------------------------------------------------------------------------------------
/** Make the 'target' a child of the 'parent' */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::makeChildOf( GameWindow *target,
																				GameWindow *parent )
{

	// sanity
	if( target == NULL )
		return;

	// get target parent
	GameWindow *prevParent = target->winGetParent();

	// check for no parent
	if( parent == NULL )
	{

		// if target already has no parent nothing to do
		if( prevParent == NULL )
			return;

		//
		// we are removing a parent from the target and placing target
		// at the top of the window list.  Be sure to preserve the
		// screen position as the new coords so it's in the same place
		//
		ICoord2D screenPos;
		target->winGetScreenPosition( &screenPos.x, &screenPos.y );
		unlinkChildWindow( target );
		linkWindow( target );
		target->winSetPosition( screenPos.x, screenPos.y );
		return;

	}

	// check to see if this is already out parent, nothing to do
	if( prevParent == parent )
		return;

	// you cannot make a window a child of a gadget control
	if( TheEditor->windowIsGadget( parent ) )
	{

		MessageBox( TheEditor->getWindowHandle(),
								"You cannot give a gadget control a child.",
								"Child Not Allowed",
								MB_OK );
		return;

	}

	// get the target screen position before the move to child
	ICoord2D screenPosBeforeMove;
	target->winGetScreenPosition( &screenPosBeforeMove.x,
																&screenPosBeforeMove.y );

	// remove the target from the window or any child list
	if( prevParent )
		unlinkChildWindow( target );
	else
		unlinkWindow( target );

	// add as child of 'parent'
	addWindowToParent( target, parent );

	// adjust the position so it's relative to the new parent
	ICoord2D parentPos;
	parent->winGetScreenPosition( &parentPos.x, &parentPos.y );
	target->winSetPosition( screenPosBeforeMove.x - parentPos.x,
													screenPosBeforeMove.y - parentPos.y );

	// notify the hierarchy of the change
	TheHierarchyView->moveWindowChildOf( target, parent );

	//
	// run through the move code to keep the child in a legal position
	// inside the parent client area
	//
	ICoord2D origin, safeLoc;
	target->winGetScreenPosition( &origin.x, &origin.y );

	// kee the location legal
	TheEditor->computeSafeLocation( target, origin.x, origin.y,
																	&safeLoc.x, &safeLoc.y );

	// move the target
	TheEditor->moveWindowTo( target, safeLoc.x, safeLoc.y );

}

//-------------------------------------------------------------------------------------------------
/** Move the 'windowToMove' to be just in front of the 'aheadOf' window
	* in either the master window list or in the child list */
//-------------------------------------------------------------------------------------------------
void GUIEditWindowManager::moveAheadOf( GameWindow *windowToMove,
																				GameWindow *aheadOf )
{

	// sanity
	if( windowToMove == NULL || aheadOf == NULL || windowToMove == aheadOf )
		return;

	//
	// get our screen position before the move ... we want to appear in the
	// same place after we're moved
	//
	ICoord2D beforeMoveScreenPos;
	windowToMove->winGetScreenPosition( &beforeMoveScreenPos.x,
																			&beforeMoveScreenPos.y );

	// get the parent of that which we are moving
	GameWindow *prevParent = windowToMove->winGetParent();

	// take the window to move off the chain
	if( prevParent )
	{
		ICoord2D pos;

		// get he screen position of the window BEFORE we unlink it
		windowToMove->winGetScreenPosition( &pos.x, &pos.y );

		// take off the child list, this will remove our parent pointer as well
		unlinkChildWindow( windowToMove );

		//
		// now that we're unlinked, set the position to be accurate with
		// the screen position and no parent
		//
		windowToMove->winSetPosition( pos.x, pos.y );

	}
	else
	{

		// just take off the main list
		unlinkWindow( windowToMove );

	}

	// insert the window at the specified location
	insertWindowAheadOf( windowToMove, aheadOf );

	//
	// adjust our position to be in the same place in our new location
	// in the chain ... if we gained a parent we have to offset by
	// the difference between our parent screen and our screen positions.
	// if we lost a parent our position must be set to the absolute
	// screen position
	//
	GameWindow *newParent = windowToMove->winGetParent();
	ICoord2D parentPos = { 0 };  // init to top left of screen
	if( newParent )
		newParent->winGetScreenPosition( &parentPos.x, &parentPos.y );
	windowToMove->winSetPosition( beforeMoveScreenPos.x - parentPos.x,
																beforeMoveScreenPos.y - parentPos.y );

	// move the window hierarchy representation of this window
	TheHierarchyView->moveWindowAheadOf( windowToMove, aheadOf );

	//
	// run through the move code to keep the child in a legal position
	// inside the parent client area
	//
	ICoord2D origin, safeLoc;
	windowToMove->winGetScreenPosition( &origin.x, &origin.y );

	// kee the location legal
	TheEditor->computeSafeLocation( windowToMove, origin.x, origin.y,
																	&safeLoc.x, &safeLoc.y );

	// move the target
	TheEditor->moveWindowTo( windowToMove, safeLoc.x, safeLoc.y );

}

