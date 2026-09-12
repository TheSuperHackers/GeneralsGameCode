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

// FILE: W3DGameWindow.h //////////////////////////////////////////////////////
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
// File name:  W3DGameWindow.h
//
// Created:    Colin Day, June 2001
//
// Desc:       W3D implementations for the game windowing system
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindow.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// W3DGameWindow --------------------------------------------------------------
/** W3D implementation for a game window */
// ----------------------------------------------------------------------------
class W3DGameWindow : public GameWindow
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(W3DGameWindow, "W3DGameWindow")

public:

	W3DGameWindow();
	// already defined by MPO.
	//~W3DGameWindow();

	/// draw borders for this window only, NO child windows or anything else
	virtual void winDrawBorder() override;
};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
extern Bool W3DGameWinDefaultDrawData( GameWindow *window,
																					 WinInstanceData *instData, void *drawList );
extern Bool W3DGameWinBorderDrawData( GameWindow *window,
																					 WinInstanceData *instData, void *drawList );
