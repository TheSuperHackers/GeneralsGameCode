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

// FILE: ResourceGatheringManager.h ///////////////////////////////////////////////////////////
// The part of a Player's brain that keeps track of all Resource type Objects and makes
// gathering type decisions based on them.
// Author: Graham Smallwood, January, 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/GameType.h"
#include "Common/Snapshot.h"

class Object;

// ------------------------------------------------------------------------------------------------
class ResourceGatheringManager : public MemoryPoolObject,
																 public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ResourceGatheringManager, "ResourceGatheringManager" );

public:
	ResourceGatheringManager();

	Object *findBestSupplyWarehouse( Object *queryObject );		///< What Warehouse should this truck go to?
	Object *findBestSupplyCenter( Object *queryObject );			///< What Center should this truck return to?

	void addSupplyCenter( Object *newCenter );					///< I captured or built a Supply Center, so record it
	void removeSupplyCenter( Object *oldCenter );				///< Lost a supply center

	void addSupplyWarehouse( Object *newWarehouse );		///< Warehouse created, or this is start of game recording
	void removeSupplyWarehouse( Object *oldWarehouse );	///< Warehouse that doesn't replenish has run out of Supply

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

private:
	/// @todo Make sure the allocator for std::list<> is a good one.  Otherwise override it.
	typedef std::list<ObjectID> objectIDList;
	typedef std::list<ObjectID>::iterator objectIDListIterator;

	objectIDList m_supplyWarehouses;
	objectIDList m_supplyCenters;

};
