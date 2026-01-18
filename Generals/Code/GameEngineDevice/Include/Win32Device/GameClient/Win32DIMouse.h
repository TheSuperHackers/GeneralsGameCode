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

// FILE: Win32DIMouse.h ///////////////////////////////////////////////////////
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
// File name:  Win32DIMouse.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Win32 direct input implementation for the mouse
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION	0x800
#endif

#include <dinput.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Mouse.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// class DirectInputMouse -----------------------------------------------------
/** Direct input implementation for the mouse device */
//-----------------------------------------------------------------------------
class DirectInputMouse : public Mouse
{

public:

	DirectInputMouse( void );
	virtual ~DirectInputMouse( void );

	// extended methods from base class
	virtual void init( void );		///< initialize the direct input mouse, extending functionality
	virtual void reset( void );		///< reset system
	virtual void update( void );  ///< update the mouse data, extending functionality
	virtual void setPosition( Int x, Int y );  ///< set position for mouse

	virtual void setMouseLimits( void );  ///< update the limit extents the mouse can move in

	virtual void setCursor( MouseCursor cursor );  ///< set mouse cursor

	virtual void capture( void );  ///< capture the mouse
	virtual void releaseCapture( void );  ///< release mouse capture

protected:

	/// device implementation to get mouse event
	virtual UnsignedByte getMouseEvent( MouseIO *result, Bool flush );

	// new internal methods for our direct input implementation
	void openMouse( void );  ///< create the direct input mouse
	void closeMouse( void );  ///< close and release mouse resources
	/// map direct input mouse data to our own format
	void mapDirectInputMouse( MouseIO *mouse, DIDEVICEOBJECTDATA *mdat );

	// internal data members for our direct input mouse
	LPDIRECTINPUT8 m_pDirectInput;  ///< pointer to direct input interface
	LPDIRECTINPUTDEVICE8 m_pMouseDevice;  ///< pointer to mouse device

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
