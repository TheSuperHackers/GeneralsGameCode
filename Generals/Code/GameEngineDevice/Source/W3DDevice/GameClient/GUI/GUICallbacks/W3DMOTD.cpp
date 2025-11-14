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

// FILE: W3DMOTD.cpp //////////////////////////////////////////////////////////
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
// File name:  W3DMOTD.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Message of the day
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GadgetSlider.h"

///////////////////////////////////////////////////////////////////////////////
// DEFINES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static NameKeyType closeButtonID = NAMEKEY_INVALID;

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// MOTDSystem =================================================================
/** Message of the day */
//=============================================================================
WindowMsgHandledType MOTDSystem( GameWindow *window, UnsignedInt msg,
								 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{

		// ------------------------------------------------------------------------
		case GWM_CREATE:
		{

			// load id's needed
			closeButtonID = TheNameKeyGenerator->nameToKey( "MOTD.wnd:CloseMOTD" );

			break;

		}

		// ------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}

		// ------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			if( controlID == closeButtonID )
				window->winHide( !window->winIsHidden() );

			break;

		}

		default:
			return MSG_IGNORED;
	}

	return MSG_HANDLED;

}


