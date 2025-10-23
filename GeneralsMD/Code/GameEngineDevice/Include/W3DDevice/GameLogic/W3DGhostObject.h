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

// FILE: W3DGhostObject.h ////////////////////////////////////////////////////////////
// Placeholder for objects that have been deleted but need to be maintained because
// a player can see them fogged.
// Author: Mark Wilczynski, August 2002

#pragma once

#include "GameLogic/GhostObject.h"
#include "Lib/BaseType.h"
#include "Common/GameCommon.h"
#include "GameClient/DrawableInfo.h"

class Object;
class W3DGhostObjectManager;
class W3DRenderObjectSnapshot;
class PartitionData;

class W3DGhostObject: public GhostObject
{
	friend W3DGhostObjectManager;

public:
	W3DGhostObject();
	virtual ~W3DGhostObject();
	virtual void snapShot(int playerIndex);
	virtual void updateParentObject(Object *object, PartitionData *mod);
	virtual void freeSnapShot(int playerIndex);

protected:
	virtual void crc( Xfer *xfer);
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );
	void removeParentObject(void);
	void restoreParentObject(void);	///< restore the original non-ghosted object to scene.
	Bool addToScene(int playerIndex);
	Bool removeFromScene(int playerIndex);
	ObjectShroudStatus getShroudStatus(int playerIndex);	///< used to get the partition manager to update ghost objects without parent objects.
	void freeAllSnapShots(void);				///< used to free all snapshots from all players.

	W3DRenderObjectSnapshot *m_parentSnapshots[MAX_PLAYER_COUNT];
	DrawableInfo	m_drawableInfo;

	///@todo this list should really be part of the device independent base class (CBD 12-3-2002)
	W3DGhostObject *m_nextSystem;
	W3DGhostObject *m_prevSystem;
};

class W3DGhostObjectManager : public GhostObjectManager
{
public:
	W3DGhostObjectManager();
	virtual ~W3DGhostObjectManager();
	virtual void reset(void);
	virtual GhostObject *addGhostObject(Object *object, PartitionData *pd);
	virtual void removeGhostObject(GhostObject *mod);
	virtual void setLocalPlayerIndex(int playerIndex);
	virtual void updateOrphanedObjects(int *playerIndexList, int playerIndexCount);
	virtual void releasePartitionData(void);
	virtual void restorePartitionData(void);

protected:
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	///@todo this list should really be part of the device independent base class (CBD 12-3-2002)
	W3DGhostObject	*m_freeModules;
	W3DGhostObject	*m_usedModules;
};
