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

// FILE: ProgressBar.cpp //////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: ProgressBar.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Progress bar GUI control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Language.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////

// GadgetProgressBarSystem ====================================================
/** Handle system messages for Progress Bar */
//=============================================================================
WindowMsgHandledType GadgetProgressBarSystem( GameWindow *window, UnsignedInt msg,
									            WindowMsgData mData1, WindowMsgData mData2 )
{

  switch( msg )
	{
		// ------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			window->winSetUserData( NULL );
			break;
		}

		// ------------------------------------------------------------------------
    case GPM_SET_PROGRESS:
    {
      Int newPos = (Int)mData1;

      if (newPos < 0 || newPos > 100)
        break;

      window->winSetUserData( (void *)newPos );

			break;

    }

		// ------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}

// GadgetProgressBarSetProgress ===============================================
/** send progress system message to Progress Bar */
//=============================================================================
void GadgetProgressBarSetProgress( GameWindow *g, Int progress )
{
	if(!g)
		return;

	TheWindowManager->winSendSystemMsg( g, GPM_SET_PROGRESS, progress, 0);
}
