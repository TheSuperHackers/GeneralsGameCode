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

#include "GameLogic/GameLogic.h"
#include "GameLogic/LocomotorSet.h"

class PathfindCell;
class PathfindLayer;
class ZoneBlock;

typedef UnsignedShort zoneStorageType;

/**
 * This class manages the zones in the map.  A zone is an area in the map that
 * is one contiguous type of terrain (clear, cliff, water, building).  If
 * a unit is in a zone, and wants to move to another location, the destination
 * zone has to be the same, or it can't get there.
 * There are equivalency tables for meta-zones.  For example, an amphibious craft can
 * travel through water and clear cells.
 */
class PathfindZoneManager
{
public:
	enum {INITIAL_ZONES = 256};
	enum {ZONE_BLOCK_SIZE = 10};	// Zones are calculated in blocks of 20x20.  This way, the raw zone numbers can be used to
	enum {UNINITIALIZED_ZONE = 0};
																// compute hierarchically between the 20x20 blocks of cells. jba.
	PathfindZoneManager();
	~PathfindZoneManager();

	void reset();

	Bool needToCalculateZones() const {return m_nextFrameToCalculateZones <= TheGameLogic->getFrame() ;} ///< Returns true if the zones need to be recalculated.
	void markZonesDirty() ; ///< Called when the zones need to be recalculated.
	void updateZonesForModify( PathfindCell **map,  PathfindLayer layers[], const IRegion2D &structureBounds, const IRegion2D &globalBounds ) ; ///< Called to recalculate an area when a structure has been removed.
	void calculateZones(	PathfindCell **map, PathfindLayer layers[], const IRegion2D &bounds);	///< Does zone calculations.
	zoneStorageType getEffectiveZone(LocomotorSurfaceTypeMask acceptableSurfaces, Bool crusher, zoneStorageType zone) const;
	zoneStorageType getEffectiveTerrainZone(zoneStorageType zone) const;

	void getExtent(ICoord2D &extent) const {extent = m_zoneBlockExtent;}

	/// return zone relative the the block zone that this cell resides in.
	zoneStorageType getBlockZone(LocomotorSurfaceTypeMask acceptableSurfaces, Bool crusher, Int cellX, Int cellY, PathfindCell **map) const;
	void allocateBlocks(const IRegion2D &globalBounds);

	void clearPassableFlags();
	Bool isPassable(Int cellX, Int cellY) const;
	Bool clipIsPassable(Int cellX, Int cellY) const;
	void setPassable(Int cellX, Int cellY, Bool passable);

	void setAllPassable();

	void setBridge(Int cellX, Int cellY, Bool bridge);
	Bool interactsWithBridge(Int cellX, Int cellY) const;

private:
	void allocateZones();
	void freeZones();
	void freeBlocks();

private:
	ZoneBlock			*m_blockOfZoneBlocks;			///< Zone blocks - Info for hierarchical pathfinding at a "blocky" level.
	ZoneBlock			**m_zoneBlocks;						///< Zone blocks as a matrix - contains matrix indexing into the map.
	ICoord2D			m_zoneBlockExtent;				///< Zone block extents. Not the same scale as the pathfind extents.

	UnsignedShort m_maxZone;								///< Max zone used.
	UnsignedInt		m_nextFrameToCalculateZones;		///< When should I recalculate, next?.
	UnsignedShort m_zonesAllocated;
	zoneStorageType *m_groundCliffZones;
	zoneStorageType *m_groundWaterZones;
	zoneStorageType *m_groundRubbleZones;
	zoneStorageType *m_terrainZones;
	zoneStorageType *m_crusherZones;
	zoneStorageType *m_hierarchicalZones;
};
