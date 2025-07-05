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

// FILE: Win32Mouse.cpp ///////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, July 2001
// Desc:       Interface for the mouse using only the Win32 messages
///////////////////////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Common/Debug.h"
#include "Common/GlobalData.h"
#include "Common/LocalFileSystem.h"
#include "GameClient/GameClient.h"
#include "Win32Device/GameClient/Win32Mouse.h"
#include "WinMain.h"


// EXTERN /////////////////////////////////////////////////////////////////////////////////////////
extern Win32Mouse *TheWin32Mouse;

HCURSOR cursorResources[Mouse::NUM_MOUSE_CURSORS][MAX_2D_CURSOR_DIRECTIONS];
///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Get a mouse event from the buffer if available, we need to translate
	* from the windows message meanings to our own internal mouse 
	* structure */
//-------------------------------------------------------------------------------------------------
UnsignedByte Win32Mouse::getMouseEvent( MouseIO *result, Bool flush )
{

	// if there is nothing here there is no event data to do
	if( m_eventBuffer[ m_nextGetIndex ].msg == 0 )
		return MOUSE_NONE;

	// translate the win32 mouse message to our own system
	translateEvent( m_nextGetIndex, result );

	// remove this event from the buffer by setting msg to zero
	m_eventBuffer[ m_nextGetIndex ].msg = 0;

	//
	// our next get index will now be advanced to the next index, wrapping at
	// the mad
	//
	m_nextGetIndex++;
	if( m_nextGetIndex >= Mouse::NUM_MOUSE_EVENTS )
		m_nextGetIndex = 0;

	// got event OK and all done with this one
	return MOUSE_OK;

}  // end getMouseEvent

//-------------------------------------------------------------------------------------------------
/** Translate a win32 mouse event to our own event info */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::translateEvent( UnsignedInt eventIndex, MouseIO *result )
{
	UINT msg = m_eventBuffer[ eventIndex ].msg;
	WPARAM wParam = m_eventBuffer[ eventIndex ].wParam;
	LPARAM lParam = m_eventBuffer[ eventIndex ].lParam;
	UnsignedInt frame;

	//
	// get the current input frame from the client, if we don't have
	// a client (like in the GUI editor) we just use frame 1 so it
	// registers with the system
	//
	if( TheGameClient )
		frame = TheGameClient->getFrame();
	else
		frame = 1;

	// set these to defaults
	result->leftState = result->middleState = result->rightState = MBS_Up;
	result->leftFrame = result->middleFrame = result->rightFrame = 0;
	result->pos.x = result->pos.y = result->wheelPos = 0;

	// Time is the same for all events
	result->time = m_eventBuffer[ eventIndex ].time;
	
	switch( msg )
	{

		// ------------------------------------------------------------------------
		case WM_LBUTTONDOWN:
		{
			
			result->leftState = MBS_Down;
			result->leftFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end left button down

		// ------------------------------------------------------------------------
		case WM_LBUTTONUP:
		{

			result->leftState = MBS_Up;
			result->leftFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end left button up

		// ------------------------------------------------------------------------
		case WM_LBUTTONDBLCLK:
		{

			result->leftState = MBS_DoubleClick;
			result->leftFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end left button double click

		// ------------------------------------------------------------------------
		case WM_MBUTTONDOWN:
		{

			result->middleState = MBS_Down;
			result->middleFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end middle button down

		// ------------------------------------------------------------------------
		case WM_MBUTTONUP:
		{

			result->middleState = MBS_Up;
			result->middleFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end middle button up

		// ------------------------------------------------------------------------
		case WM_MBUTTONDBLCLK:
		{

			result->middleState = MBS_DoubleClick;
			result->middleFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end middle button double click

		// ------------------------------------------------------------------------
		case WM_RBUTTONDOWN:
		{

			result->rightState = MBS_Down;
			result->rightFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end right button down

		// ------------------------------------------------------------------------
		case WM_RBUTTONUP:
		{

			result->rightState = MBS_Up;
			result->rightFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end right button up

		// ------------------------------------------------------------------------
		case WM_RBUTTONDBLCLK:
		{

			result->rightState = MBS_DoubleClick;
			result->rightFrame = frame;
			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end right button double click

		// ------------------------------------------------------------------------
		case WM_MOUSEMOVE:
		{

			result->pos.x = LOWORD( lParam );
			result->pos.y = HIWORD( lParam );
			break;

		}  // end mouse move

		// ------------------------------------------------------------------------
		case 0x020A:	// WM_MOUSEWHEEL
		{
			POINT p;

			// translate the screen mouse position to be relative to the application window
			p.x = LOWORD( lParam );
			p.y = HIWORD( lParam );
			ScreenToClient( ApplicationHWnd, &p );
	
			// note the short cast here to keep signed information in tact
			result->wheelPos = (Short)HIWORD( wParam );
			result->pos.x = p.x;
			result->pos.y = p.y;
			break;

		}  // end mouse wheel

		// ------------------------------------------------------------------------
		default:
		{

			DEBUG_CRASH(( "translateEvent: Unknown Win32 mouse event [%d,%d,%d]",
							 msg, wParam, lParam ));
			return;

		}  // end default

	}  // end switch on message at event index in buffer

}  // end translateEvent

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Win32Mouse::Win32Mouse( void )
{

	// zero our event list
	memset( &m_eventBuffer, 0, sizeof( m_eventBuffer ) );

	m_nextFreeIndex = 0;
	m_nextGetIndex = 0;
	m_currentWin32Cursor = NONE;
	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
		for (Int j=0; j<MAX_2D_CURSOR_DIRECTIONS; j++)
			cursorResources[i][j]=NULL;
	m_directionFrame=0; //points up.
	m_lostFocus = FALSE;
}  // end Win32Mouse

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Win32Mouse::~Win32Mouse( void )
{

	// remove our global reference that was for the WndProc() only
	TheWin32Mouse = NULL;

}  // end ~Win32Mouse

//-------------------------------------------------------------------------------------------------
/** Initialize our device */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::init( void )
{

	// extending functionality
	Mouse::init();

	//
	// when we receive messages from a Windows message procedure, the mouse
	// moves report the current cursor position and not deltas, our mouse
	// needs to process those positions as absolute and not relative
	//
	m_inputMovesAbsolute = TRUE;

}  // end int

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::reset( void )
{

	// extend
	Mouse::reset();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update, called once per frame */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::update( void )
{

	// extend 
	Mouse::update();

}  // end update

//-------------------------------------------------------------------------------------------------
/** Add a window message event along with its WPARAM and LPARAM parameters
	* to our input storage buffer */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::addWin32Event( UINT msg, WPARAM wParam, LPARAM lParam, DWORD time )
{

	//
	// we can only add this event if our next free index does not already
	// have an event in it, if it does ... our buffer is full and this input
	// event will be lost
	//
	if( m_eventBuffer[ m_nextFreeIndex ].msg != 0 )
		return;

	// add to this index
	m_eventBuffer[ m_nextFreeIndex ].msg = msg;
	m_eventBuffer[ m_nextFreeIndex ].wParam = wParam;
	m_eventBuffer[ m_nextFreeIndex ].lParam = lParam;
	m_eventBuffer[ m_nextFreeIndex ].time = time;

	// wrap index at max
	m_nextFreeIndex++;
	if( m_nextFreeIndex >= Mouse::NUM_MOUSE_EVENTS )
		m_nextFreeIndex = 0;

}  // end addWin32Event

extern HINSTANCE ApplicationHInstance;

void Win32Mouse::setVisibility(Bool visible)
{
	//Extend
	Mouse::setVisibility(visible);
	//Maybe need to set cursor to force hiding of some cursors.
	Win32Mouse::setCursor(getMouseCursor());
}

/**Preload all the cursors we may need during the game.  This must be done before the D3D device
is created to avoid cursor corruption on buggy ATI Radeon cards. */
void Win32Mouse::initCursorResources(void)
{
	for (Int cursor=FIRST_CURSOR; cursor<NUM_MOUSE_CURSORS; cursor++)
	{
		for (Int direction=0; direction<m_cursorInfo[cursor].numDirections; direction++)
		{	if (!cursorResources[cursor][direction] && !m_cursorInfo[cursor].textureName.isEmpty())
			{	//this cursor has never been loaded before.
				char resourcePath[256];
				//Check if this is a directional cursor
				if (m_cursorInfo[cursor].numDirections > 1)
					sprintf(resourcePath,"data\\cursors\\%s%d.ANI",m_cursorInfo[cursor].textureName.str(),direction);
				else
					sprintf(resourcePath,"data\\cursors\\%s.ANI",m_cursorInfo[cursor].textureName.str());
				
				// check for a MOD cursor.
				Bool loaded = FALSE;
				if (TheGlobalData->m_modDir.isNotEmpty())
				{
					AsciiString fname;
					if (m_cursorInfo[cursor].numDirections > 1)
						fname.format("%sdata\\cursors\\%s%d.ANI", TheGlobalData->m_modDir.str(), m_cursorInfo[cursor].textureName.str(), direction);
					else
						fname.format("%sdata\\cursors\\%s.ANI", TheGlobalData->m_modDir.str(), m_cursorInfo[cursor].textureName.str());

					if (TheLocalFileSystem->doesFileExist(fname.str()))
					{
						cursorResources[cursor][direction]=LoadCursorFromFile(fname.str());
						loaded = TRUE;
					}
				}

				if (!loaded)
					cursorResources[cursor][direction]=LoadCursorFromFile(resourcePath);
				DEBUG_ASSERTCRASH(cursorResources[cursor][direction], ("MissingCursor %s",resourcePath));
			}
		}
//		SetCursor(cursorResources[cursor][m_directionFrame]);
	}
}

//-------------------------------------------------------------------------------------------------
/** Super basic simplistic cursor */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::setCursor( MouseCursor cursor )
{
	// extend
	Mouse::setCursor( cursor );

	if (m_lostFocus)
		return;	//stop messing with mouse cursor if we don't have focus.

	if (cursor == NONE || !m_visible)
		SetCursor( NULL );
	else
	{
		SetCursor(cursorResources[cursor][m_directionFrame]);
	}  // end switch

	// save current cursor
	m_currentWin32Cursor=m_currentCursor = cursor;
	
}  // end setCursor

//-------------------------------------------------------------------------------------------------
/** Capture the mouse to our application */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::capture( void )
{

//	SetCapture( ApplicationHWnd );

}  // end capture

//-------------------------------------------------------------------------------------------------
/** Release the mouse capture for our app window */
//-------------------------------------------------------------------------------------------------
void Win32Mouse::releaseCapture( void )
{

//	ReleaseCapture();

}  // end releaseCapture








