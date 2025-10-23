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

// FILE: GameStateMap.h ///////////////////////////////////////////////////////////////////////////
// Author: Colin Day, October 2002
// Desc:   Chunk in the save game file that will hold a pristine version of the map file
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INLCUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Snapshot.h"
#include "Common/SubsystemInterface.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Xfer;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class GameStateMap : public SubsystemInterface,
										 public Snapshot
{

public:

	GameStateMap( void );
	virtual ~GameStateMap( void );

	// subsystem interface methods
	virtual void init( void ) { }
	virtual void reset( void ) { }
	virtual void update( void ) { }

	// snapshot methods
	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void ) { }

	void clearScratchPadMaps( void );		///< clear any scratch pad maps from the save directory

protected:


};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern GameStateMap *TheGameStateMap;
