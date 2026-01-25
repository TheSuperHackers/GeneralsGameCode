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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    Generals
//
// Module:     IME
//
// File name:  GameClient/IMEManager.h
//
// Created:    11/14/01 TR
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Lib/BaseType.h"
#include "Common/SubsystemInterface.h"
#include "Common/UnicodeString.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class GameWindow;
class UnicodeString;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------


//===============================
// IMEManagerInterface
//===============================

class IMEManagerInterface : public SubsystemInterface
{

	public:

		virtual ~IMEManagerInterface() {};

		virtual void					attach( GameWindow *window ) = 0;		///< attach IME to specified window
		virtual void					detach( void ) = 0;								///< detach IME from current window
		virtual void					enable( void ) = 0;									///< Enable IME
		virtual void					disable( void ) = 0;								///< Disable IME
		virtual Bool					isEnabled( void ) = 0;							///< Is IME enabled
		virtual Bool					isAttachedTo( GameWindow *window ) = 0;	///< Is the manager currently attached to the window
		virtual GameWindow*		getWindow( void ) = 0;							///< Returns the window we are currently attached to
		virtual Bool					isComposing( void ) = 0;						///< Manager is currently composing new input string
		virtual void					getCompositionString( UnicodeString &string ) = 0; ///< Return the current composition string
		virtual Int						getCompositionCursorPosition( void ) =0;			///< Returns the composition cursor position
		virtual Int						getIndexBase( void ) = 0;						///< Get index base for candidate list


		virtual Int						getCandidateCount() = 0;						///< Returns the total number of candidates
		virtual const UnicodeString* getCandidate( Int index ) = 0;	///< Returns the candidate string
		virtual Int						getSelectedCandidateIndex() = 0;		///< Returns the indexed of the currently selected candidate
		virtual Int						getCandidatePageSize() = 0;					///< Returns the page size for the candidates list
		virtual Int						getCandidatePageStart() = 0;				///< Returns the index of the first visibel candidate



		/// Checks for and service IME messages. Returns TRUE if message serviced
		virtual Bool serviceIMEMessage(	void *windowsHandle,
												UnsignedInt message,
												Int wParam,
												Int lParam ) = 0;
		virtual Int result( void ) = 0;							///< result return value of last serviced IME message
};


extern IMEManagerInterface *TheIMEManager;
extern IMEManagerInterface *CreateIMEManagerInterface( void );


//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
