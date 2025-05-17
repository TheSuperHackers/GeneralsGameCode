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

// FILE: WindowLayout.cpp /////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Layouts for grouping windows together
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/WindowLayout.h"
#include "GameClient/Shell.h"
#include "GameClient/GameWindowManager.h"
#ifdef RTS_INTERNAL

// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WindowLayout::WindowLayout( void )
{
	m_filenameString.set("EmptyLayout");

	m_windowList = NULL;
	m_windowTail = NULL;
	m_windowCount = 0;

	m_hidden = FALSE;

	m_init = NULL;
	m_update = NULL;
	m_shutdown = NULL;

}  // end WindowLayout

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WindowLayout::~WindowLayout( void )
{

	//
	// it is the users responsability to remove windows from the layout beforing destroying the
	// layout itself.  This allows for maximum flexibility of the window layouts and you can
	// use them in any you see fit, as long as they are clean when they go away
	//
	DEBUG_ASSERTCRASH( m_windowList == NULL, ("Window layout being destroyed still has window references") );
	DEBUG_ASSERTCRASH( m_windowTail == NULL, ("Window layout being destroyed still has window references") );	

}  // end ~WindowLayout

//-------------------------------------------------------------------------------------------------
/** Set the hidden/visible status of all the windows in this layout */
//-------------------------------------------------------------------------------------------------
void WindowLayout::hide( Bool hide )
{
	GameWindow *window;

	// hide or unhide all windows in this layout
	for( window = m_windowList; window; window = window->winGetNextInLayout() )
	{

		window->winHide( hide );

	}  // end for window

	// save the new visible state of the system
	m_hidden = hide;

}  // end hide

//-------------------------------------------------------------------------------------------------
/** Add window to this layout */
//-------------------------------------------------------------------------------------------------
void WindowLayout::addWindow( GameWindow *window )
{
	GameWindow *win = findWindow( window );

	// only add window if window is not in this layout already	
	if( win == NULL )
	{

		DEBUG_ASSERTCRASH( window->winGetNextInLayout() == NULL,
											 ("NextInLayout should be NULL before adding") );
		DEBUG_ASSERTCRASH( window->winGetPrevInLayout() == NULL,
											 ("PrevInLayout should be NULL before adding") );

		window->winSetPrevInLayout( NULL );
		window->winSetNextInLayout( m_windowList );
		if( m_windowList )
			m_windowList->winSetPrevInLayout( window );
		m_windowList = window;

		// set layout into window
		window->winSetLayout( this );

		// if no tail pointer, this is it
		if( m_windowTail == NULL )
			m_windowTail = window;

		// we gots another window now
		m_windowCount++;

	}  // end if

}  // end addWindow

//-------------------------------------------------------------------------------------------------
/** Remove window from this layout */
//-------------------------------------------------------------------------------------------------
void WindowLayout::removeWindow( GameWindow *window )
{
	GameWindow *win = findWindow( window );

	// can't remove window unless it's really part of this layout
	if( win )
	{
		GameWindow *prev, *next;

		prev = win->winGetPrevInLayout();
		next = win->winGetNextInLayout();

		if( next )
			next->winSetPrevInLayout( prev );
		if( prev )
			prev->winSetNextInLayout( next );
		else
			m_windowList = next;

		// set window as having no layout info
		win->winSetLayout( NULL );
		win->winSetNextInLayout( NULL );
		win->winSetPrevInLayout( NULL );

		// if we removed the tail, set the new tail
		if( m_windowTail == win )
			m_windowTail = prev;

		// we lost one sir!
		m_windowCount--;

	}  // end if

}  // end removeWindow

//-------------------------------------------------------------------------------------------------
/** Destroy all the windows in a layout */
//-------------------------------------------------------------------------------------------------
void WindowLayout::destroyWindows( void )
{
	if (this == NULL)
	{
		return;
	}
	GameWindow *window;

	while( (window = getFirstWindow()) != 0 )
	{

		// remove window from this layout
		removeWindow( window );

		// destroy window in window system
		TheWindowManager->winDestroy( window );

	}  // end while

}  // end destroyWindows

//-------------------------------------------------------------------------------------------------
/** Create the windows using the .wnd file script and load all windows into
	* this layout */
//-------------------------------------------------------------------------------------------------
Bool WindowLayout::load( AsciiString filename )
{

	// sanity
	if( filename.isEmpty() )
		return FALSE;

	//
	// when we create new windows they are always placed on the top
	// of the window stack.  The load layout from script will return the
	// FIRST window loaded from the script, if we traverse from the head of
	// the window list till we see that window returned from the layout
	// loader, that will be all the root windows loaded from the .wnd file
	//
	GameWindow *target;
	WindowLayoutInfo info;

	target = TheWindowManager->winCreateFromScript( filename, &info );
	if( target == NULL )
	{

		DEBUG_ASSERTCRASH( target, ("WindowLayout::load - Failed to load layout") );
		DEBUG_LOG(( "WindowLayout::load - Unable to load layout file '%s'\n", filename.str() ));
		return FALSE;

	}  // end if

	//
	// add windows loaded from .wnd file to the layout, via info.windows.
	//
	std::list<GameWindow *>::iterator it;
	for (it = info.windows.begin(); it != info.windows.end(); ++it)
	{
		// add window to this layout
		addWindow( *it );
	}

	/* MDC - can't do this, as modal windows will be at the head...
	//
	// add windows loaded from .wnd file to the layout, note we start at
	// target and go backwards as the layout->addWindow() puts at top
	// of list and will therefore reverse the order
	//
	GameWindow *window;
	for( window = target; window; window = window->winGetPrev() )
	{

		// add window to this layout
		addWindow( window );

	}  // end for window
	*/

	// copy filename
	m_filenameString = filename;

	// assign script info to the layout
	setInit( info.init );
	setUpdate( info.update );
	setShutdown( info.shutdown );

	return TRUE;  // success

}  // end load

//-------------------------------------------------------------------------------------------------
/** Bring all windows in this layout forward */
//-------------------------------------------------------------------------------------------------
void WindowLayout::bringForward( void )
{

	//
	// loop through all our windows and bring each of them to the top of
	// the window stack, note that we are getting a prev pointer because the
	// action of bringing a window to the top of the stack does detach it
	// from the window list and the layout window list ... it is then
	// re-attached at the top.  Also note, to preserve the ordering of
	// the windows we pull from the tail and add to the top
	//
	GameWindow *window, *prev;
	Int countLeft = m_windowCount;

	for( window = m_windowTail; countLeft; window = prev )
	{

		DEBUG_ASSERTCRASH( window, ("Must have window: m_windowCount is off") );
		prev = window->winGetPrevInLayout();
		window->winBringToTop();
		countLeft--;

	}  // end for window

}  // end bringForward


// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
/** Find window within this layout */
//-------------------------------------------------------------------------------------------------
GameWindow *WindowLayout::findWindow( GameWindow *window )
{
	GameWindow *win;

	for( win = m_windowList; win; win = win->winGetNextInLayout() )
		if( win == window )
			return win;

	return NULL;  // window not found

}  // end findWindow
