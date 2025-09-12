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

// FILE: GhostObject.h ////////////////////////////////////////////////////////////
// Placeholder for objects that have been deleted but need to be maintained because
// a player can see them fogged.
// Author: Mark Wilczynski, August 2002

#pragma once

#ifndef _GHOSTOBJECT_H_
#define _GHOSTOBJECT_H_

#include "Lib/BaseType.h"
#include "Common/Snapshot.h"

// #define DEBUG_FOG_MEMORY	///< this define is used to force object snapshots for all players, not just local player.

class Object;
class PartitionData;
enum GeometryType CPP_11(: Int);
enum ObjectID CPP_11(: Int);

class GhostObject : public Snapshot
{
public:
	GhostObject();
	virtual ~GhostObject();
	virtual void snapShot(int playerIndex)=0;
	virtual void updateParentObject(Object *object, PartitionData *mod)=0;
	virtual void freeSnapShot(int playerIndex)=0;
	inline PartitionData *friend_getPartitionData(void) const {return m_partitionData;}
	inline GeometryType getGeometryType(void) const {return m_parentGeometryType;}
	inline Bool getGeometrySmall(void) const {return m_parentGeometryIsSmall;}
	inline Real getGeometryMajorRadius(void) const {return m_parentGeometryMajorRadius;}
	inline Real getGeometryMinorRadius(void) const {return m_parentGeometryminorRadius;}
	inline Real getParentAngle(void) const {return m_parentAngle;}
	inline const Coord3D *getParentPosition(void) const {return &m_parentPosition;}

protected:
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Object *m_parentObject;		///< object which we are ghosting
	GeometryType m_parentGeometryType;
	Bool m_parentGeometryIsSmall;
	Real m_parentGeometryMajorRadius;
	Real m_parentGeometryminorRadius;
	Real m_parentAngle;
	Coord3D m_parentPosition;
	PartitionData	*m_partitionData;	///< our PartitionData
};

class GhostObjectManager : public Snapshot
{
public:
	GhostObjectManager();
	virtual ~GhostObjectManager();

	virtual void reset(void);
	virtual GhostObject *addGhostObject(Object *object, PartitionData *pd);
	virtual void removeGhostObject(GhostObject *mod);
	virtual void setLocalPlayerIndex(int playerIndex) { m_localPlayer = playerIndex; }
	inline int getLocalPlayerIndex(void)	{ return m_localPlayer; }
	virtual void updateOrphanedObjects(int *playerIndexList, int playerIndexCount);
	virtual void releasePartitionData(void);	///<saves data needed to later rebuild partition manager data.
	virtual void restorePartitionData(void);	///<restores ghost objects into the partition manager.
	inline void lockGhostObjects(Bool enableLock) {m_lockGhostObjects=enableLock;}	///<temporary lock on creating new ghost objects. Only used by map border resizing!
	inline void saveLockGhostObjects(Bool enableLock) {m_saveLockGhostObjects=enableLock;}

protected:
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Int m_localPlayer;
	Bool m_lockGhostObjects;
	Bool m_saveLockGhostObjects;	///< used to lock the ghost object system during a save/load
};

// the singleton
extern GhostObjectManager *TheGhostObjectManager;

#endif // _GHOSTOBJECT_H_
