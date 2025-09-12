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

// FILE: Win32DIKeyboard.h ////////////////////////////////////////////////////
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
// File name:  Win32DIKeyboard.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Device implementation of the keyboard interface on Win32
//						 using Microsoft Direct Input
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __WIN32DIKEYBOARD_H_
#define __WIN32DIKEYBOARD_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION	0x800
#endif

#include <dinput.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Keyboard.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// class DirectInputKeyboard --------------------------------------------------
/** Class for interfacing with the keyboard using direct input as the
	* implementation */
//-----------------------------------------------------------------------------
class DirectInputKeyboard : public Keyboard
{

public:

	DirectInputKeyboard( void );
	virtual ~DirectInputKeyboard( void );

	// extend methods from the base class
	virtual void init( void );		///< initialize the keyboard, extending init functionality
	virtual void reset( void );		///< Reset the keybaord system
	virtual void update( void );  ///< update call, extending update functionality
	virtual Bool getCapsState( void );		///< get state of caps lock key, return TRUE if down

protected:

	// extended methods from the base class
	virtual void getKey( KeyboardIO *key );  ///< get a single key event

	//-----------------------------------------------------------------------------------------------

	// new methods to this derived class
	void openKeyboard( void );  ///< create direct input keyboard
	void closeKeyboard( void );  ///< release direct input keyboard

	// direct input data members
	LPDIRECTINPUT8 m_pDirectInput;  ///< pointer to direct input interface
	LPDIRECTINPUTDEVICE8 m_pKeyboardDevice;  ///< pointer to keyboard device

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __WIN32DIKEYBOARD_H_

