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

// FILE: GUIEditWindowManager.h ///////////////////////////////////////////////////////////////////
// Created:    Colin Day, July 2001
// Desc:       Window manager for the GUI edit tool, we want this up
//						 fast and to look like what we use in the game so we're going
//						 to use the WW3D window manager, and just override the
//						 drawing functions to draw lines and images to the
//						 display.  We will also be adding our own functionality
//						 here for editing and interacting with the GUI windows.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GUIEDITWINDOWMANAGER_H_
#define __GUIEDITWINDOWMANAGER_H_

#include <stdlib.h>
#include "W3DDevice/GameClient/W3DGameWindowManager.h"

//-------------------------------------------------------------------------------------------------
/** GUI edit interface for window manager */
//-------------------------------------------------------------------------------------------------
class GUIEditWindowManager : public W3DGameWindowManager
{

public:

	GUIEditWindowManager( void );
	virtual ~GUIEditWindowManager( void );

	virtual void init( void );  ///< initialize system

	virtual Int winDestroy( GameWindow *window );  ///< destroy this window
	/// create a new window by setting up parameters and callbacks
	virtual GameWindow *winCreate( GameWindow *parent, UnsignedInt status,
																 Int x, Int y, Int width, Int height,
																 GameWinSystemFunc system,
																 WinInstanceData *instData = NULL );

	// **************************************************************************
	// GUIEdit specific methods *************************************************
	// **************************************************************************

	/** unlink the window to move and place it ahead of the target window
	in the master chain or the child chain */
	void moveAheadOf( GameWindow *windowToMove, GameWindow *aheadOf );
	/// make target a child of the parent
	void makeChildOf( GameWindow *target, GameWindow *parent );

	void validateClipboardNames( GameWindow *root );  ///< ensure unique names
	void incrementName( GameWindow *window );  ///< make a new unique name
	void resetClipboard( void );  ///< reset the clipboard to empty
	Bool isClipboardEmpty( void );  ///< is the clipboard empty
	void duplicateSelected( GameWindow *root );  ///< dupe the selected windows into the clipboard
	void copySelectedToClipboard( void );  ///< copy selected windows to clipboard
	void cutSelectedToClipboard( void );  ///< cut selected windows to clipboard
	void pasteClipboard( void );  ///< paste the contents of the clipboard

	GameWindow *getClipboardList( void );  ///< get the clipboard list
	GameWindow *getClipboardDupeList( void );  ///< get clipboard dupe list

protected:

	/** validate window is part of the clipboard at the top level */
	Bool isWindowInClipboard( GameWindow *window, GameWindow **list );
	void linkToClipboard( GameWindow *window, GameWindow **list );  ///< add window to clipboard
	void unlinkFromClipboard( GameWindow *window, GameWindow **list );  ///< remove window from clipboard

	/** remove selected children from the select list that have a parent
	also in the select list */
	void removeSupervisedChildSelections( void );
	/** selected windows that are children will cut loose their parents
	and become adults (their parent will be NULL, otherwise the screen) */
//	void orphanSelectedChildren( void );

  /// dupe a window and its children
	GameWindow *duplicateWindow( GameWindow *source, GameWindow *parent );
	void createClipboardDuplicate( void );  ///< duplicate the clipboard on the dup list

	GameWindow *m_clipboard;  ///< list of windows in the clipboard
	GameWindow *m_clipboardDup;  ///< list duplicate of the clipboard used for pasting

	Int m_copySpacing;  ///< keeps multiple pastes from being on top of each other
	Int m_numCopiesPasted;  ///< keeps multiple pastes from being on top of each other

};

// INLINE /////////////////////////////////////////////////////////////////////////////////////////
inline GameWindow *GUIEditWindowManager::getClipboardList( void ) { return m_clipboard; }
inline GameWindow *GUIEditWindowManager::getClipboardDupeList( void ) { return m_clipboardDup; }

// EXTERN /////////////////////////////////////////////////////////////////////////////////////////
extern GUIEditWindowManager *TheGUIEditWindowManager;  ///< editor use only

#endif // __GUIEDITWINDOWMANAGER_H_

