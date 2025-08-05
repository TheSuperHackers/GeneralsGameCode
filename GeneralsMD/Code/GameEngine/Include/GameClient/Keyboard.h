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

// FILE: Keyboard.h ///////////////////////////////////////////////////////////
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
// File name:  Keyboard.h
//
// Created:    Mike Morrison, 1995
//						 Colin Day, June 2001
//
// Desc:       Basic keyboard
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/SubsystemInterface.h"
#include "GameClient/KeyDefs.h"
#include "Lib/BaseType.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINITIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// KeyboardIO -----------------------------------------------------------------
/** Holds a single keyboard event */
//-----------------------------------------------------------------------------
struct KeyboardIO
{

	enum StatusType
	{
		STATUS_UNUSED		= 0x00,					// Key has not been used
		STATUS_USED			= 0x01					// Key has been eaten
	};

	UnsignedByte	key;										// key data
	UnsignedByte	status;									// StatusType, above
	UnsignedShort	state;									// KEY_STATE_* in KeyDefs.h
	UnsignedInt		sequence;								// sequence info from DirectX used for order

};  // end KeyboardIO

// class Keyboard =============================================================
/** Keyboard singleton to interface with the keyboard */
//=============================================================================
class Keyboard : public SubsystemInterface
{

	enum { KEY_REPEAT_DELAY = 10 };

public:

	Keyboard( void );
	virtual ~Keyboard( void );

	// you may extend the functionanilty of these for your device
	virtual void init( void );							/**< initialize the keyboard, only extend this
																							 functionality, do not replace */
	virtual void reset( void );							///< Reset keyboard system
	virtual void update( void );						/**< gather current state of all keys, extend
																							 this functionality, do not replace */
	virtual Bool getCapsState( void ) = 0;  ///< get state of caps lock key, return TRUE if down

	virtual void createStreamMessages( void );  /**< given state of device, create
																							messages and put them on the
																							stream for the raw state. */
	// simplified versions where the caller doesn't care which key type was pressed.
	Bool isShift();
	Bool isCtrl();
	Bool isAlt();
	Int getModifierFlags() { return m_modifiers; }

	// access methods for key data
	void resetKeys( void );												///< reset the state of the keys
	KeyboardIO *getFirstKey( void );							///< get first key ready for processing
	void setKeyStatusData( UnsignedByte key,
												 KeyboardIO::StatusType data );   ///< set key status
	WideChar translateKey( WideChar keyCode );		///< translte key code to printable UNICODE char
	WideChar getPrintableKey( UnsignedByte key, Int state );
	enum { MAX_KEY_STATES = 3};
protected:

	/** get the key data for a single key, KEY_NONE should be returned when
	no key data is available to get anymore, you must implement this for your device */
	virtual void getKey( KeyboardIO *key ) = 0;

	// internal methods to update the key states
	void initKeyNames( void );  ///< initialize the key names table
	void updateKeys( void );  ///< update the state of our key data
	Bool checkKeyRepeat( void );  ///< check for repeating keys
	UnsignedByte getKeyStatusData( UnsignedByte key );  ///< get key status
	Bool getKeyStateBit( UnsignedByte key, Int bit );  ///< get key state bit
	UnsignedInt getKeySequenceData( UnsignedByte key );  ///< get key sequence
	void setKeyStateData( UnsignedByte key, UnsignedByte data );  ///< get key state

	UnsignedShort m_modifiers;
	// internal keyboard data members
	//Bool m_capsState;			// 1 if caps lock is on
	//Bool m_shiftState;		// 1 if either shift key is pressed
	//Bool m_shift2State;		// 1 if secondary shift key is pressed
	//Bool m_lShiftState;		// 1 if left state is down
	//Bool m_rShiftState;		// 1 if right shift is down
	//Bool m_lControlState; // 1 if left control is down
	//Bool m_rControlState; // 1 if right control is down
	//Bool m_lAltState;			// 1 if left alt is down
	//Bool m_rAltState;			// 1 if right alt is down
	UnsignedByte m_shift2Key;  // what key is the secondary shift key

	enum { NUM_KEYS  = 256 };
	KeyboardIO m_keys[ NUM_KEYS ];  ///< the keys
	KeyboardIO m_keyStatus[ NUM_KEYS ];  ///< the key status flags

	enum { KEY_NAMES_COUNT = 256 };
	struct
	{

		WideChar stdKey;
		WideChar shifted;
		WideChar shifted2;

	} m_keyNames[ KEY_NAMES_COUNT ];
	UnsignedInt m_inputFrame;  ///< frame input was gathered on

};  // end Keyboard

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
extern Keyboard *TheKeyboard;

#endif // __KEYBOARD_H_
