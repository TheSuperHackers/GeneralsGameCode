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

// FILE: DisplayString.cpp ////////////////////////////////////////////////////
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
// File name:  DisplayString.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       Contstuct for holding double byte game string data and being
//						 able to draw that text to the screen.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "Common/Language.h"
#include "GameClient/DisplayString.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// DisplayString::DisplayString ===============================================
/** */
//=============================================================================
DisplayString::DisplayString( void )
{
	// m_textString = "";	// not necessary, done by default
	m_font = NULL;

	m_next = NULL;
	m_prev = NULL;

}  // end DisplayString

// DisplayString::~DisplayString ==============================================
/** */
//=============================================================================
DisplayString::~DisplayString( void )
{

	// free any data
	reset();

}  // end ~DisplayString

// DisplayString::setText =====================================================
/** Copy the text to this instance */
//=============================================================================
void DisplayString::setText( UnicodeString text )
{
	if (text == m_textString)
		return;

	m_textString = text;

	// our text has now changed
	notifyTextChanged();

}  // end setText

// DisplayString::reset =======================================================
/** Free and reset all the data for this string, effectively making this
	* instance like brand new */
//=============================================================================
void DisplayString::reset( void )
{

	m_textString.clear();

	// no font
	m_font = NULL;

}  // end reset

// DisplayString::removeLastChar ==============================================
/** Remove the last character from the string text */
//=============================================================================
void DisplayString::removeLastChar( void )
{
	m_textString.removeLastChar();

	// our text has now changed
	notifyTextChanged();

}  // end removeLastChar

// DisplayString::truncateBy ==================================================
/** Remove the last charCount characters from the string text */
//=============================================================================
void DisplayString::truncateBy(const Int charCount)
{
	m_textString.truncateBy(charCount);

	// our text has now changed
	notifyTextChanged();

}  // end truncateBy

// DisplayString::truncateTo ==================================================
/** Remove the last characters from the string text so it's at the most
	* maxLength characters long */
	//=============================================================================
void DisplayString::truncateTo(const Int maxLength)
{
	m_textString.truncateTo(maxLength);

	// our text has now changed
	notifyTextChanged();

}  // end truncateTo

// DisplayString::appendChar ==================================================
/** Append character to the end of the string */
//=============================================================================
void DisplayString::appendChar( WideChar c )
{
	m_textString.concat(c);

	// text has now changed
	notifyTextChanged();

}  // end appendchar

