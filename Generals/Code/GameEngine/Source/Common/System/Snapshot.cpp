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

// FILE: Snapshot.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   The Snapshot object is the base class interface for data structures that will
//				 be considered during game saves, loads, and CRC checks.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/GameState.h"
#include "Common/Snapshot.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Snapshot::Snapshot( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Snapshot::~Snapshot( void )
{

	//
	// if we're loading, there are pathological cases where we could destroy snapshots while
	// there is an entry for them in the post processing list ... need to clean this up
	//
	///@ todo, this might be needed in theory in the future, but iterating the post process
	// list in the game state is expensive because it's HUGE!
	//
//	TheGameState->notifySnapshotDeleted();

}
