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

// FILE: CaveSystem.h /////////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood July 2002
// Desc:   System responsible for keeping track of the connectedness of all cave systems on the map
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CAVE_SYSTEM_H
#define CAVE_SYSTEM_H

class Object;
class TunnelTracker; // The player owns one such object for his Tunnels, so instead of duplicating
// so much code, this SubSystem will manage all of the Cave systems.

#include "Common/Snapshot.h"
#include "Common/SubsystemInterface.h"

/**
		System responsible for Crates as code objects - ini, new/delete etc
*/
class CaveSystem : public SubsystemInterface,
									 public Snapshot
{
public:
	CaveSystem();
	~CaveSystem();

	void init();
	void reset();
	void update();

	Bool canSwitchIndexToIndex( Int oldIndex, Int newIndex ); // If either Index has guys in it, no, you can't
	void registerNewCave( Int theIndex );			// All Caves are born with a default index, which could be new
	void unregisterCave( Int theIndex );				//
	TunnelTracker *getTunnelTrackerForCaveIndex( Int theIndex );

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void ) { }

private:
	std::vector<TunnelTracker*> m_tunnelTrackerVector;// A vector of pointers where the indexes are known by
	// others, so it can have NULLs in it to keep position.  I've been advised against a map, so don't be a jerk
	// and use spot 20 first.

};

extern CaveSystem *TheCaveSystem;
#endif
