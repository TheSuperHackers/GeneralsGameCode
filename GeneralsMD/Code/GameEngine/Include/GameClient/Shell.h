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

// FILE: Shell.h //////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Description: Shell menu representations
//
// Using The Shell:
// ----------------
//
//	The Shell makes use of the window layouts to represent screens.  You can push and
//	pop screens on the stack so that you don't have to keep track of how you got
//  to which screen.
//
//  Here is what happens when you push a layout on the shell stack:
//	1) The layout name is stored as a "pending push"
//	2) The top of the stack has the Shutdown() method run
//	3) The Shutdown() method (or other mechanisms like Update()) will call
//		 Shell::shutdownComplete() when the shutdown process for that layout is "complete"
//  4) Shell::shutdownComplete() sees the "pending push" in the shell
//  5) "Pending push" layout is then loaded from disk and the pending push state is cleared
//  6) The new layout is put on the top of the stack
//  7) The new layout Init() method is called
//
//  Here is what happens when you pop the top of the stack
//	1) The stack sets a "pending pop" as in progress
//	2) The top layout of the stack has the Shutdown() method called
//	3) The Shutdown() method (or other mechanisms like Update()) will call
//		 Shell::shutdownComplete() when the shutdown process for that layout is "complete"
//  4) Shell::shutdownComplete() sees the "pending pop" in the shell
//  5) The "pending pop" layout is then destroyed and removed from the stack
//  6) The new top of the stack has the Init() method run
//
//  Window Layouts and the Shell:
//  -----------------------------
//
//  Window Layouts in the shell need the following functions to work property, these
//	can be assigned from the GUIEdit window layout tool:
//
//		- Init() [OPTIONAL]
//			This is called as a result of a push or pop operation (see above for more info)
//			The window layout is loaded from disk and then this Init()
//			method is run.  All shell layout Init() methods should show
//			the layout windows.  At this point you could move windows
//			to starting positions, set a state that the Update() method looks at to
//			"animate" the windows to the desired positions.
//
//		- Update() [OPTIONAL]
//			This is called once at a rate of "shellUpdateDelay" for EVERY screen on the shell
//			stack.  It does not matter if the screen is on the top, or is hidden, or
//			anything, this is always called.  Each update is run starting with the screen
//			at the top of the stack and progressing to the bottom of the stack.
//			States could be set in the Init() or Shutdown() methods of the layout
//			that the Update() looks at and reacts to appropriately if desired.
//
//		- Shutdown() [REQUIRED]
//			This is called when a layout is popped off the stack, or when a new layout
//			is pushed on top of this one (see above for more detail on what happens
//			during the push/pop process).  You can switch into a "shutdown" state and
//			animate the layout appropriately in the Update() method for the layout.
//			When shutdown is actually complete you should hide the all windows in
//			the layout and then you are REQUIRED to notify the shell by calling
//			the Shell::shutdownComplete() method.
//
//			Shutdown() is also required to be able to handle the parameter "immediatePop".
//			If this parameter is TRUE it means that when control returns from the
//			shutdown function that the layout will immediately be popped off the
//			stack.  We need to be able to handle this when in code we want to
//			traverse back down the stack rapidly (like when we lose connection to
//			an online service, we might pop all the way back to the login screen)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class WindowLayout;
class AnimateWindowManager;
class GameWindow;
class ShellMenuSchemeManager;

enum AnimTypes CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
/** This is the interface to the shell system to load, display, and
	* manage screen menu shell system layouts */
//-------------------------------------------------------------------------------------------------
class Shell : public SubsystemInterface
{

public:

	Shell( void );
	~Shell( void );

	// Inhertited from subsystem ====================================================================
	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );
	//===============================================================================================

	void recreateWindowLayouts( void );

	void showShellMap(Bool useShellMap );										///< access function to turn on and off the shell map

	void hide( Bool hide );																	///< show/hide all shell layouts

	// pseudo-stack operations for manipulating layouts
	void push( AsciiString filename, Bool shutdownImmediate = FALSE );	///< load new screen on top, optionally doing an immediate shutdown
	void pop( void );																				///< pop top layout
	void popImmediate( void );															///< pop now, don't wait for shutdown
	void showShell( Bool runInit = TRUE );									///< init the top of stack
	void hideShell( void );																	///< shutdown the top of stack
	WindowLayout *top( void );															///< return top layout

	void shutdownComplete( WindowLayout *layout, Bool impendingPush = FALSE );	///< layout has completed shutdown

	WindowLayout *findScreenByFilename( AsciiString filename );		///< find screen
	Bool isShellActive( void ) { return m_isShellActive; }  ///<	Returns true if the shell is active

	void registerWithAnimateManager( GameWindow *win, AnimTypes animType, Bool needsToFinish, UnsignedInt delayMS = 0);
	Bool isAnimFinished( void );
	void reverseAnimatewindow( void );
	Bool isAnimReversed( void );

	void loadScheme( AsciiString name );
	ShellMenuSchemeManager *getShellMenuSchemeManager( void ) { return m_schemeManager;	}

	Int getScreenCount( void ) const { return m_screenCount; } ///< Return the current number of screens
	WindowLayout *getScreenLayout( Int index ) const;

	WindowLayout *getSaveLoadMenuLayout( void );		///< create if necessary and return layout for save load menu
	WindowLayout *getPopupReplayLayout( void );			///< create if necessary and return layout for replay save menu
	WindowLayout *getOptionsLayout( Bool create );	///< return layout for options menu, create if necessary and we are allowed to.
	void destroyOptionsLayout( void );							///< destroy the shell's options layout.

protected:

	void construct( void );
	void deconstruct( void );

	void linkScreen( WindowLayout *screen );								///< link screen to list
	void unlinkScreen( WindowLayout *screen );							///< remove screen from list

	void doPush( AsciiString layoutFile );									///< workhorse for push action
	void doPop( Bool impendingPush );												///< workhorse for pop action

	enum { MAX_SHELL_STACK = 16 };													///< max simultaneous shell screens
	WindowLayout *m_screenStack[ MAX_SHELL_STACK ];					///< the screen layout stack
	Int m_screenCount;																			///< # of screens in screen stack

	WindowLayout *m_background;															///< The Background layout if the 3d shell isn't running
	Bool m_clearBackground;																	///< Flag if we're going to clear the background or not

	Bool m_pendingPush;																			///< TRUE when a push is pending
	Bool m_pendingPop;																			///< TRUE when a pop is pending
	AsciiString m_pendingPushName;													///< layout name to be pushed
	Bool m_isShellActive;																		///< TRUE when the shell is active
	Bool m_shellMapOn;																			///< TRUE when the shell map is on
	AnimateWindowManager *m_animateWindowManager;						///< The animate Window Manager
	ShellMenuSchemeManager *m_schemeManager;								///< The Shell Scheme Manager

	//
	// we keep a pointer to this layout so that we can simply just hide/unhide this
	// window layout.  Why you ask?  Well, as the result of pressing a button to start
	// a save game load a super large set of operations will happen as the game
	// loads.  One of those operations is the destruction of the menu, which although
	// it just destroys the windows and puts them on a destroyed list, that destroyed
	// list is also processed before we are out of our own window procedure.
	// This is a prime example why it's easier to just deal with windows by hiding and
	// un-hiding them rather than actually creating and destroying them.
	//
	WindowLayout *m_saveLoadMenuLayout;											///< save/load menu layout
	WindowLayout *m_popupReplayLayout;											///< replay save menu layout
	WindowLayout *m_optionsLayout;													///< options menu layout

};

// INLINING ///////////////////////////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern Shell *TheShell;  ///< the shell external interface
