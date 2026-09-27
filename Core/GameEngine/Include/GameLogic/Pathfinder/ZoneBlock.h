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

#pragma once

#include "GameLogic/LocomotorSet.h"

class PathfindCell;
class PathfindLayer;

typedef UnsignedShort zoneStorageType;

/**
 * This class is a helper class for zone manager.  It maintains information regarding the
 * LocomotorSurfaceTypeMask equivalencies within a ZONE_BLOCK_SIZE x ZONE_BLOCK_SIZE area of
 * cells.  This is used in hierarchical pathfinding to find the best coarse path at the
 * block level.
 */
class ZoneBlock
{
public:
	ZoneBlock();
	~ZoneBlock();  // not virtual, please don't override without making virtual.  jba.

	void blockCalculateZones(	PathfindCell **map, PathfindLayer layers[], const IRegion2D &bounds);	///< Does zone calculations.
	zoneStorageType getEffectiveZone(LocomotorSurfaceTypeMask acceptableSurfaces, Bool crusher, zoneStorageType zone) const;

	void clearMarkedPassable() {m_markedPassable = false;}
	Bool isPassable() {return m_markedPassable;}
	void setPassable(Bool pass) {m_markedPassable = pass;}

	Bool getInteractsWithBridge() const {return m_interactsWithBridge;}
	void setInteractsWithBridge(Bool interacts) {m_interactsWithBridge = interacts;}

protected:
	void allocateZones();
	void freeZones();

protected:
	ICoord2D		m_cellOrigin;

	zoneStorageType m_firstZone; // First zone in this block.
	UnsignedShort m_numZones;	 // Number of zones in this block.  If == 1, there is only one zone, and
														 // no zone equivalency arrays will be allocated.

	UnsignedShort m_zonesAllocated;
	zoneStorageType *m_groundCliffZones;
	zoneStorageType *m_groundWaterZones;
	zoneStorageType *m_groundRubbleZones;
	zoneStorageType *m_crusherZones;
	Bool					m_interactsWithBridge;
	Bool					m_markedPassable;
};
typedef ZoneBlock *ZoneBlockP;
