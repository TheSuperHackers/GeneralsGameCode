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

// FILE: Win32DIMouse.cpp /////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, June 2001
// Desc:       Win32 direct input implementation for the mouse
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <windows.h>
#include <assert.h>

#include "Common/Debug.h"
#include "GameClient/Display.h"
#include "Win32Device/GameClient/Win32DIMouse.h"
#include "WinMain.h"

// DEFINES ////////////////////////////////////////////////////////////////////////////////////////
enum { MOUSE_BUFFER_SIZE = 256, };

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Create our direct input object, mouse device, and initialize it to the
	* data formats we want */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::openMouse( void )
{
	HRESULT hr;

	// create our direct input device for mouse access
	hr = DirectInput8Create( ApplicationHInstance,
													 DIRECTINPUT_VERSION,
													 IID_IDirectInput8,
													 (void **)&m_pDirectInput,
													 NULL );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openMouse: Unabled to create direct input interface" ));
		assert( 0 );
		closeMouse();
		return;

	}  // end if

	// create a device for the system mouse
	hr = m_pDirectInput->CreateDevice( GUID_SysMouse,
																		 &m_pMouseDevice,
																		 NULL );
	if( FAILED( hr ) )
	{

			DEBUG_LOG(( "ERROR - openMouse: Unable to create mouse device" ));
			assert( 0 );
			closeMouse();
			return;

	}  // end if

	// set the data format for the mouse
	hr = m_pMouseDevice->SetDataFormat( &c_dfDIMouse );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openMouse: Unabled to set mouse data format" ));
		assert( 0 );
		closeMouse();
		return;

	}  // end if

	// set the mouse cooperative level
	hr = m_pMouseDevice->SetCooperativeLevel( ApplicationHWnd,
																						DISCL_NONEXCLUSIVE |
																						DISCL_FOREGROUND );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openMouse: Unabled to set coop level" ));
		assert( 0 );
		closeMouse();
		return;

	}  // end if

	// set the mouse buffer size
	DIPROPDWORD prop;
	prop.diph.dwSize = sizeof( DIPROPDWORD );
	prop.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	prop.diph.dwObj = 0;
	prop.diph.dwHow = DIPH_DEVICE;
	prop.dwData = MOUSE_BUFFER_SIZE;
	hr = m_pMouseDevice->SetProperty( DIPROP_BUFFERSIZE, &prop.diph );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openMouse: Unabled to set buffer property" ));
		assert( 0 );
		closeMouse();
		return;

	}  // end if

	// acquire the mouse
	hr = m_pMouseDevice->Acquire();
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openMouse: Unabled to acquire mouse" ));
		assert( 0 );
		closeMouse();
		return;

	}  // end if

	// get some information about the mouse
	DIDEVCAPS diDevCaps;

	diDevCaps.dwSize = sizeof( DIDEVCAPS );
	hr = m_pMouseDevice->GetCapabilities( &diDevCaps );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "WARNING - openMouse: Cann't get capabilities of mouse for button setup" ));

	}  // end if
	else
	{

		// keep some data about the mouse we care about
		m_numButtons = (UnsignedByte)diDevCaps.dwButtons;
		m_numAxes = (UnsignedByte)diDevCaps.dwAxes;
		m_forceFeedback = BitIsSet( diDevCaps.dwFlags, DIDC_FORCEFEEDBACK );

		DEBUG_LOG(( "OK - Mouse info: Buttons = '%d', Force Feedback = '%s', Axes = '%d'",
						 m_numButtons, m_forceFeedback ? "Yes" : "No", m_numAxes ));

	}  // end else

	DEBUG_LOG(( "OK - Mouse initialized successfully" ));

}  // end openMouse

//-------------------------------------------------------------------------------------------------
/** Release any resources for our direct input mouse */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::closeMouse( void )
{

	// release the mouse device
	if( m_pMouseDevice )
	{

		m_pMouseDevice->Unacquire();
		m_pMouseDevice->Release();
		m_pMouseDevice = NULL;
		DEBUG_LOG(( "OK - Mouse device closed" ));

	}  // end if

	// release our direct input interface for the mouse
	if( m_pDirectInput )
	{

		m_pDirectInput->Release();
		m_pDirectInput = NULL;
		DEBUG_LOG(( "OK - Mouse direct input interface closed" ));

	}  // end if

	DEBUG_LOG(( "OK - Mouse shutdown complete" ));

}  // end closeMouse

//-------------------------------------------------------------------------------------------------
/** Get a single mouse event from the device */
//-------------------------------------------------------------------------------------------------
UnsignedByte DirectInputMouse::getMouseEvent( MouseIO *result, Bool flush )
{
	HRESULT hr;
	DIDEVICEOBJECTDATA mdat;
	UnsignedByte mouseResult = MOUSE_NONE;
	DWORD num;

	/* set these to defaults */
	result->leftState = result->middleState = result->rightState = FALSE;
	result->leftFrame = result->middleFrame = result->rightFrame = 0;
	result->pos.x = result->pos.y = result->wheelPos = 0;

	if( m_pMouseDevice )
	{

		// get 1 event, if available
		num = 1;
		m_pMouseDevice->Poll();
		hr = m_pMouseDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ),
																				&mdat,
																				&num,
																				0 );
		switch( hr )
		{

			// ----------------------------------------------------------------------
			case DI_OK:
			{
				// nothing returned
				if( num != 0 )
				{
					mapDirectInputMouse( result, &mdat );
					mouseResult = MOUSE_OK;
				}
				break;
			}

			// ----------------------------------------------------------------------
			case DIERR_NOTACQUIRED:
			case DIERR_INPUTLOST:
			{

				// if we lost focus, attempt to re-acquire
				hr = m_pMouseDevice->Acquire();
				switch( hr )
				{

					// ------------------------------------------------------------------
					// If successful... tell system to loop back
					case DI_OK:
					case S_FALSE:
						mouseResult = MOUSE_LOST;

					// ------------------------------------------------------------------
					//If an error occurs return MOUSE_NONE
					case DIERR_INVALIDPARAM:
					case DIERR_NOTINITIALIZED:
					case DIERR_OTHERAPPHASPRIO:
					default:
						break;

				}  // end switch

				break;
			}

			// ----------------------------------------------------------------------
			default:
//				DBGPRINTF(("GetMouseEvent: GetDeviceData Error: %X.\r\n", hr ));
				break;

		}  // end switch

	}  // end if

	return mouseResult;

}  // end getMouseEvent

//-------------------------------------------------------------------------------------------------
/** Map the direct input codes to our own mouse format */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::mapDirectInputMouse( MouseIO *mouse,
																						DIDEVICEOBJECTDATA *mdat )
{

	switch( mdat->dwOfs )
	{
		case DIMOFS_BUTTON0:
			mouse->leftState = (( mdat->dwData & 0x0080 ) ? TRUE : FALSE);
			mouse->leftFrame = mdat->dwSequence;
			break;

		case DIMOFS_BUTTON1:
			mouse->rightState = (( mdat->dwData & 0x0080 ) ? TRUE : FALSE);
			mouse->rightFrame = mdat->dwSequence;
			break;

		case DIMOFS_BUTTON2:
			mouse->middleState = (( mdat->dwData & 0x0080 ) ? TRUE : FALSE);
			mouse->middleFrame = mdat->dwSequence;
			break;

		case DIMOFS_BUTTON3:
			break;

		case DIMOFS_X:
			mouse->pos.x = mdat->dwData;
			break;

		case DIMOFS_Y:
			mouse->pos.y = mdat->dwData;
			break;

		case DIMOFS_Z:
			mouse->wheelPos = mdat->dwData;
			break;
	}

}  // end mapDirectInputMouse

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DirectInputMouse::DirectInputMouse( void )
{

	m_pDirectInput = NULL;
	m_pMouseDevice = NULL;

}  // end DirectInputMouse

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DirectInputMouse::~DirectInputMouse( void )
{

	// relase all mouse resources
	closeMouse();
//	ShowCursor( TRUE );

}  // end ~DirectInputMouse

//-------------------------------------------------------------------------------------------------
/** Initialize the direct input mouse device */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::init( void )
{
	POINT p;

	// extending functionality from our base class
	Mouse::init();

	// open the mouse and create the direct input interfaces we need
	openMouse();

	// move the window mouse to the location we have initialized in our system
	p.x = m_currMouse.pos.x;
	p.y = m_currMouse.pos.y;
	ClientToScreen( ApplicationHWnd, &p );
	SetCursorPos( p.x, p.y );
//	ShowCursor( FALSE );

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset direct input mouse */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::reset( void )
{

	// extend
	Mouse::reset();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update the mouse position and button data, this is called once per
	* frame in the engine.  NOTE that this routine is extendion functionality
	* that we may need that is direct input specific, not replacing */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::update( void )
{

	// extendion functionality from our base class
	Mouse::update();

	//
	// since we are currently using the windows cursor because it updates at
	// an independent rate of our application we will always just use the windows
	// mouse cursor position
	//

	/** @todo we need to really visit this system and possibly come up with
	our own multi-threaded cursor etc */
	POINT p;
	GetCursorPos( &p );
	ScreenToClient( ApplicationHWnd, &p );
	moveMouse( p.x, p.y, MOUSE_MOVE_ABSOLUTE );

}  // end update

//-------------------------------------------------------------------------------------------------
/** Set the limits which the mouse is allowed to move around in.  We
	* will limit it to the client area, and if we are windowed we will
	* allow for the mouse to move within the title bar at the top of
	* the window */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::setMouseLimits( void )
{

	//
	// extending functionality, although we may overwrite the limits set
	// from the base class
	//
	Mouse::setMouseLimits();

	//
	// when runing windowed we want to keep the mouse within the game
	// window cause it's annoying to mouse out of the window and click
	// on a background window.
	//
	if( TheDisplay && TheDisplay->getWindowed() == TRUE )
	{
		RECT windowRect;

		// get the window rect
		GetWindowRect( ApplicationHWnd, &windowRect );

		// keep the cursor clipped to these coords when running windowed
		ClipCursor( &windowRect );

	}  // end if

}  // end setMouseLimits

//-------------------------------------------------------------------------------------------------
/** set the cursor position for windows OS */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::setPosition( Int x, Int y )
{
	POINT p;

	// extending functionality
	Mouse::setPosition( x, y );

	// set the windows cursor
	p.x = x;
	p.y = y;
	ClientToScreen( ApplicationHWnd, &p );

	// set the window mouse
	SetCursorPos( p.x, p.y );

}  // end setPosition

//-------------------------------------------------------------------------------------------------
/** Super basic simplistic cursor */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::setCursor( MouseCursor cursor )
{

	// extend
	Mouse::setCursor( cursor );

	// if we're already on this cursor ignore
	if( m_currentCursor == cursor )
		return;

	switch( cursor )
	{

		case NONE:
			SetCursor( NULL );
			break;

		case NORMAL:
		case ARROW:
			SetCursor( LoadCursor( NULL, IDC_ARROW ) );
			break;

		case SCROLL:
			SetCursor( LoadCursor( NULL, IDC_SIZEALL ) );
			break;

		case CROSS:
			SetCursor( LoadCursor( NULL, IDC_CROSS ) );
			break;

	}  // end switch

	// save current cursor
	m_currentCursor = cursor;

}  // end setCursor

//-------------------------------------------------------------------------------------------------
/** Capture the mouse to our application */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::capture( void )
{

	SetCapture( ApplicationHWnd );

}  // end capture

//-------------------------------------------------------------------------------------------------
/** Release the mouse capture for our app window */
//-------------------------------------------------------------------------------------------------
void DirectInputMouse::releaseCapture( void )
{

	ReleaseCapture();

}  // end releaseCapture
