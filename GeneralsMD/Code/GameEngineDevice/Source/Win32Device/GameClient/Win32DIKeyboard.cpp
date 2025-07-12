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

// FILE: Win32DIKeyboard.cpp //////////////////////////////////////////////////////////////////////
// Created:   Colin Day, June 2001
// Desc:      Device implementation of the keyboard interface on Win32
//						using Microsoft Direct Input
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <assert.h>

#include "Common/Debug.h"
#include "Common/Language.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/Keyboard.h"
#include "Win32Device/GameClient/Win32DIKeyboard.h"
#include "WinMain.h"

// DEFINES ////////////////////////////////////////////////////////////////////////////////////////
enum { KEYBOARD_BUFFER_SIZE = 256 };

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
struct ErrorLookup
{
	HRESULT error;
	const char *string;
};
static ErrorLookup errorLookup[] = 
{
{ (HRESULT)DIERR_ACQUIRED, "DIERR_ACQUIRED" },
{ (HRESULT)DIERR_ALREADYINITIALIZED, "DIERR_ALREADYINITIALIZED" },
{ (HRESULT)DIERR_BADDRIVERVER, "DIERR_BADDRIVERVER" },
{ (HRESULT)DIERR_BETADIRECTINPUTVERSION, "DIERR_BETADIRECTINPUTVERSION" },
{ (HRESULT)DIERR_DEVICEFULL, "DIERR_DEVICEFULL" },
{ (HRESULT)DIERR_DEVICENOTREG, "DIERR_DEVICENOTREG" },
{ (HRESULT)DIERR_EFFECTPLAYING, "DIERR_EFFECTPLAYING" },
{ (HRESULT)DIERR_GENERIC, "DIERR_GENERIC" },
{ (HRESULT)DIERR_HANDLEEXISTS, "DIERR_HANDLEEXISTS" },
{ (HRESULT)DIERR_HASEFFECTS, "DIERR_HASEFFECTS" },
{ (HRESULT)DIERR_INCOMPLETEEFFECT, "DIERR_INCOMPLETEEFFECT" },
{ (HRESULT)DIERR_INPUTLOST, "DIERR_INPUTLOST" },
{ (HRESULT)DIERR_INVALIDPARAM, "DIERR_INVALIDPARAM" },
{ (HRESULT)DIERR_MAPFILEFAIL, "DIERR_MAPFILEFAIL" },
{ (HRESULT)DIERR_MOREDATA, "DIERR_MOREDATA" },
{ (HRESULT)DIERR_NOAGGREGATION, "DIERR_NOAGGREGATION" },
{ (HRESULT)DIERR_NOINTERFACE, "DIERR_NOINTERFACE" },
{ (HRESULT)DIERR_NOTACQUIRED, "DIERR_NOTACQUIRED" },
{ (HRESULT)DIERR_NOTBUFFERED, "DIERR_NOTBUFFERED" },
{ (HRESULT)DIERR_NOTDOWNLOADED, "DIERR_NOTDOWNLOADED" },
{ (HRESULT)DIERR_NOTEXCLUSIVEACQUIRED, "DIERR_NOTEXCLUSIVEACQUIRED" },
{ (HRESULT)DIERR_NOTFOUND, "DIERR_NOTFOUND" },
{ (HRESULT)DIERR_NOTINITIALIZED, "DIERR_NOTINITIALIZED" },
{ (HRESULT)DIERR_OBJECTNOTFOUND, "DIERR_OBJECTNOTFOUND" },
{ (HRESULT)DIERR_OLDDIRECTINPUTVERSION, "DIERR_OLDDIRECTINPUTVERSION" }, 
{ (HRESULT)DIERR_OTHERAPPHASPRIO, "DIERR_OTHERAPPHASPRIO" },
{ (HRESULT)DIERR_OUTOFMEMORY, "DIERR_OUTOFMEMORY" },
{ (HRESULT)DIERR_READONLY, "DIERR_READONLY" },
{ (HRESULT)DIERR_REPORTFULL, "DIERR_REPORTFULL" },
{ (HRESULT)DIERR_UNPLUGGED, "DIERR_UNPLUGGED" },
{ (HRESULT)DIERR_UNSUPPORTED, "DIERR_UNSUPPORTED" },
{ 0, NULL }
};


///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** For debugging, prints the return code using direct input errors */
//-------------------------------------------------------------------------------------------------
static void printReturnCode( char *label, HRESULT hr )
{
	ErrorLookup *error = errorLookup;

	while( error->string != NULL )
	{

		if( error->error == hr )
		{
			DEBUG_LOG(( "%s: '%s' - '0x%08x'", label, error->string, hr ));
			break;
		}
		error++;

	}

}  // end printReturnCode

//-------------------------------------------------------------------------------------------------
/** create our interface to the direct input keybard */
//-------------------------------------------------------------------------------------------------
void DirectInputKeyboard::openKeyboard( void )
{
  HRESULT hr;

	// create our interface to direct input
	hr = DirectInput8Create( ApplicationHInstance,
													 DIRECTINPUT_VERSION,
													 IID_IDirectInput8, 
													 (void **)&m_pDirectInput,
													 NULL );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openKeyboard: DirectInputCreate failed" ));
		assert( 0 );
		closeKeyboard();
		return;

	}  // end if

	// obtain an interface to the system keyboard device
	hr = m_pDirectInput->CreateDevice( GUID_SysKeyboard,
																		 &m_pKeyboardDevice,
																		 NULL );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openKeyboard: Unabled to create keyboard device" ));
		assert( 0 );
		closeKeyboard();
		return;

	}  // end if

	// set the data format for the keyboard
	hr = m_pKeyboardDevice->SetDataFormat( &c_dfDIKeyboard );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openKeyboard: Unabled to set data format for keyboard" ));
		assert( 0 );
		closeKeyboard();
		return;

	}  // end if

	/// @todo Check the cooperative level of keyboard for NT, 2000, DX8 etc ...
	// set the cooperative level for the keyboard, must be non-exclusive for
	// NT support, but we should check with the latest versions of DirectX
	// on 2000 etc
	//
	hr = m_pKeyboardDevice->SetCooperativeLevel( ApplicationHWnd, 
																							 DISCL_FOREGROUND | 
																							 DISCL_NONEXCLUSIVE );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openKeyboard: Unabled to set cooperative level" ));
		assert( 0 );
		closeKeyboard();
		return;

	}  // end if

  // set the keyboard buffer size
	DIPROPDWORD prop;
	prop.diph.dwSize = sizeof( DIPROPDWORD );
	prop.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	prop.diph.dwObj = 0;
	prop.diph.dwHow = DIPH_DEVICE;
	prop.dwData = KEYBOARD_BUFFER_SIZE;
	hr = m_pKeyboardDevice->SetProperty( DIPROP_BUFFERSIZE, &prop.diph );
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openKeyboard: Unable to set keyboard buffer size property" ));
		assert( 0 );
		closeKeyboard();
		return;

	}  // end if

	// acquire the keyboard
	hr = m_pKeyboardDevice->Acquire();
	if( FAILED( hr ) )
	{

		DEBUG_LOG(( "ERROR - openKeyboard: Unable to acquire keyboard device" ));
		// Note - This can happen in windowed mode, and we can re-acquire later.  So don't 
		// close the keyboard. jba.
		// closeKeyboard();
		return;

	}  // end if

	DEBUG_LOG(( "OK - Keyboard initialized successfully." ));

}  // end openKeyboard

//-------------------------------------------------------------------------------------------------
/** close the direct input keyboard */
//-------------------------------------------------------------------------------------------------
void DirectInputKeyboard::closeKeyboard( void )
{

	if( m_pKeyboardDevice )
	{

		m_pKeyboardDevice->Unacquire();
		m_pKeyboardDevice->Release();
		m_pKeyboardDevice = NULL;
		DEBUG_LOG(( "OK - Keyboard deviced closed" ));

	}  // end if
	if( m_pDirectInput )
	{

		m_pDirectInput->Release();
		m_pDirectInput = NULL;
		DEBUG_LOG(( "OK - Keyboard direct input interface closed" ));

	}  // end if

	DEBUG_LOG(( "OK - Keyboard shutdown complete" ));

}  // end closeKeyboard

//-------------------------------------------------------------------------------------------------
/** Get a single keyboard event from direct input */
//-------------------------------------------------------------------------------------------------
void DirectInputKeyboard::getKey( KeyboardIO *key )
{
	static int errs = 0;
	DIDEVICEOBJECTDATA kbdat;
	DWORD num = 0;
//	int done = 0;
	HRESULT hr;

	assert( key );
	key->sequence = 0;
	key->key = KEY_NONE;

	if( m_pKeyboardDevice )
	{
		// get 1 key, if available
		num = 1;
		hr = m_pKeyboardDevice->Acquire();
		if (hr == DI_OK || hr == S_FALSE)
			hr = m_pKeyboardDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ),
																						 &kbdat, &num, 0 );
		switch( hr )
		{

			// ----------------------------------------------------------------------
			case DI_OK:
				break;

			// ----------------------------------------------------------------------
			case DIERR_INPUTLOST:
			case DIERR_NOTACQUIRED:

				// if we lost focus, attempt to re-acquire
				hr = m_pKeyboardDevice->Acquire();
				switch( hr )
				{

					// ------------------------------------------------------------------
					//If an error occurs return KEY_NONE
					case DIERR_INVALIDPARAM:
					case DIERR_NOTINITIALIZED:
					case DIERR_OTHERAPPHASPRIO:
						break;

					// ------------------------------------------------------------------
					// If successful... tell system to loop back
					case DI_OK:
					case S_FALSE:
					{

						// this will tell the system to loop again
						key->key = KEY_LOST;

						break;

					}  // end, got the keyboard back OK

				}  // end switch

				return;

			// ----------------------------------------------------------------------
			default:
				return;

		}  // end switch( hr )

		// no keys returned
		if( num == 0 )
			return;

		// set the key
		key->key = (UnsignedByte)(kbdat.dwOfs & 0xFF);

		// sequence
		key->sequence = kbdat.dwSequence;

		//
		// state of key, note we are setting the key state here with an assignment
		// and not a bit set of the up/down state, this is the "start"
		// of building this "key"
		//
		key->state = (( kbdat.dwData & 0x0080 ) ? KEY_STATE_DOWN : KEY_STATE_UP);

		// set status as unused (unprocessed)
		key->status = KeyboardIO::STATUS_UNUSED;

	}  // end if, we have a DI keyboard device

}  // end getKey

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DirectInputKeyboard::DirectInputKeyboard( void )
{

	m_pDirectInput = NULL;
	m_pKeyboardDevice = NULL;


	if( GetKeyState( VK_CAPITAL ) & 0x01 )
	{
		m_modifiers |= KEY_STATE_CAPSLOCK;
	}
	else
	{
		m_modifiers &= ~KEY_STATE_CAPSLOCK;
	}

}  // end DirectInputKeyboard

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DirectInputKeyboard::~DirectInputKeyboard( void )
{

	// close keyboard and release all resource
	closeKeyboard();

}  // end ~DirectInputKeyboard

//-------------------------------------------------------------------------------------------------
/** initialize the keyboard */
//-------------------------------------------------------------------------------------------------
void DirectInputKeyboard::init( void )
{

	// extending functionality
	Keyboard::init();

	// open the direct input keyboard
	openKeyboard();

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset keyboard system */
//-------------------------------------------------------------------------------------------------
void DirectInputKeyboard::reset( void )
{

	// extend functionality
	Keyboard::reset();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** called once per frame to update the keyboard state */
//-------------------------------------------------------------------------------------------------
void DirectInputKeyboard::update( void )
{

	// extending functionality
	Keyboard::update();

/*
	// make sure the keyboard buffer is flushed
	if( m_pKeyboardDevice )
	{
		DWORD items = INFINITE;

		m_pKeyboardDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ),
																			NULL, &items, 0 );

	}  // end if
*/

}  // end update

//-------------------------------------------------------------------------------------------------
/** Return TRUE if the caps lock key is down/hilighted */
//-------------------------------------------------------------------------------------------------
Bool DirectInputKeyboard::getCapsState( void )
{

	return BitIsSet( GetKeyState( VK_CAPITAL ), 0X01);

}  // end getCapsState
