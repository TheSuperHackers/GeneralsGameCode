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

// FILE: WindowLayout.h ///////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Encapsulation of all windows loaded from a .wnd file for
//						 purposes of a "shell" layout screen
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "GameClient/GameWindow.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class GameWindow;
class WindowLayout;

// TYPE DEFINES ///////////////////////////////////////////////////////////////////////////////////
typedef void (*WindowLayoutInitFunc)( WindowLayout *layout, void *userData );
typedef void (*WindowLayoutUpdateFunc)( WindowLayout *layout, void *userData );
typedef void (*WindowLayoutShutdownFunc)( WindowLayout *layout, void *userData );

//-------------------------------------------------------------------------------------------------
/** The representation of a screen layout loaded from a .wnd layout
	* script file */
//-------------------------------------------------------------------------------------------------
class WindowLayout : public MemoryPoolObject
{

	// memory pool for screen layouts
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WindowLayout, "WindowLayoutPool" );

public:

	WindowLayout( void );
	// ~WindowLayout( void );												///< defined by memory pool glue

	// manipulating screen properties ---------------------------------------------------------------
	AsciiString getFilename( void ) const;					///< return source window filename
	Bool load( AsciiString filename );							///< create windows and load from .wnd file
	void hide( Bool hide );													///< hide/show all windows on this screen
	Bool isHidden( void ) const;										///< return visible state of screen
	void bringForward( void );											///< bring all windows in this screen forward

	// manipulating window lists --------------------------------------------------------------------
	void addWindow( GameWindow *window );						///< add window to screen
	void removeWindow( GameWindow *window );				///< remove window from screen
	void destroyWindows( void );										///< destroy all windows in this screen
	GameWindow *getFirstWindow( void ) const;				///< get first window in list for screen

	// accessing layout callbacks  ------------------------------------------------------------------
	void runInit( void *userData = nullptr );									///< run the init method if available
	void runUpdate( void *userData = nullptr );								///< run the update method if available
	void runShutdown( void *userData = nullptr );							///< run the shutdown method if available
	void setInit( WindowLayoutInitFunc init );							///< set the init callback
	void setUpdate( WindowLayoutUpdateFunc update );				///< set the update callback
	void setShutdown( WindowLayoutShutdownFunc shutdown);		///< set the shutdown callback

protected:

	// internal helpers -----------------------------------------------------------------------------
	GameWindow *findWindow( GameWindow *window );		///< find window in this layout

	//===============================================================================================
	// protected data ===============================================================================
	//===============================================================================================

	AsciiString m_filenameString;										///< layout filename
	GameWindow *m_windowList;												///< list of windows in this layout
	GameWindow *m_windowTail;												///< end of m_windowList
	Int m_windowCount;															///< how man windows are in the list
	Bool m_hidden;																	///< visible state of this screen

	//
	// These are callbacks you can attach to a "layout file" ... they are not
	// automatically called when using the WindowManager to load and create
	// the layout.  You can incorporate when and where init, shutdown and update should
	// be called for any system or code that is uses these window layouts
	//
	WindowLayoutInitFunc		 m_init;								///< init callback
	WindowLayoutUpdateFunc	 m_update;							///< update callback
	WindowLayoutShutdownFunc m_shutdown;						///< shutdown callback

};

// INLINING ///////////////////////////////////////////////////////////////////////////////////////
inline AsciiString WindowLayout::getFilename( void ) const { return m_filenameString; }
inline GameWindow *WindowLayout::getFirstWindow( void ) const { return m_windowList; }
inline Bool WindowLayout::isHidden( void ) const { return m_hidden; }

inline void WindowLayout::runInit( void *userData ) { if( m_init ) m_init( this, userData ); }
inline void WindowLayout::runUpdate( void *userData ) { if( m_update ) m_update( this, userData ); }
inline void WindowLayout::runShutdown( void *userData ) { if( m_shutdown ) m_shutdown( this, userData ); }

inline void WindowLayout::setInit( WindowLayoutInitFunc init ) { m_init = init; }
inline void WindowLayout::setUpdate( WindowLayoutUpdateFunc update ) { m_update = update; }
inline void WindowLayout::setShutdown( WindowLayoutShutdownFunc shutdown ) {m_shutdown = shutdown;}
