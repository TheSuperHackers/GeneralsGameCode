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

#include "PathfindCellInfo.h"

class Object;
class PathfindCellList;

typedef UnsignedShort zoneStorageType;

#if !RETAIL_COMPATIBLE_PATHFINDING
#undef RETAIL_COMPATIBLE_PATHFINDING_ALLOCATION
#endif

/**
 * This represents one cell in the pathfinding grid.
 * These cells categorize the world into idealized cellular states,
 * and are also used for efficient A* pathfinding.
 * @todo Optimize memory usage of pathfind grid.
 */
class PathfindCell
{
public:

	enum CellType
	{
		CELL_CLEAR		= 0x00,									///< clear, unobstructed ground
		CELL_WATER		= 0x01,									///< water area
		CELL_CLIFF		= 0x02,									///< steep altitude change
		CELL_RUBBLE		= 0x03,									///< Cell is occupied by rubble.
		CELL_OBSTACLE	= 0x04,									///< Occupied by a structure
		CELL_BRIDGE_IMPASSABLE = 0x05,				///< Piece of a bridge that is impassable.
		CELL_IMPASSABLE = 0x06								///< Just plain impassable except for aircraft.
	};

	enum CellFlags
	{
		NO_UNITS		= 0x00,						///< No units in this cell.
		UNIT_GOAL		= 0x01,						///< A unit is heading to this cell.
		UNIT_PRESENT_MOVING	= 0x02,		///< A unit is moving through this cell.
		UNIT_PRESENT_FIXED	= 0x03,		///< A unit is stationary in this cell.
		UNIT_GOAL_OTHER_MOVING	= 0x05		///< A unit is moving through this cell, and another unit has this as it's goal.
	};

	/// reset the cell
	void reset();

	PathfindCell();
	~PathfindCell();

	Bool setTypeAsObstacle( Object *obstacle, Bool isFence, const ICoord2D &pos );				///< flag this cell as an obstacle, from the given one
	Bool removeObstacle( Object *obstacle );				///< unflag this cell as an obstacle, from the given one
	void setType( CellType type );	///< set the cell type
	CellType getType() const { return (CellType)m_type; }				///< get the cell type
	CellFlags getFlags() const { return (CellFlags)m_flags; }				///< get the cell type
	Bool isAircraftGoal() const {return m_aircraftGoal != 0;}

	Bool isObstaclePresent( ObjectID objID ) const;					///< return true if the given object ID is registered as an obstacle in this cell
#if RETAIL_COMPATIBLE_PATHFINDING_ALLOCATION
	// TheSuperHackers @info isObstructionInvalid() and clearObstruction() only used during retail compatible pathfinding failover cleanup
	Bool isObstructionInvalid() const { return m_obstacleID != INVALID_ID && m_info == nullptr && (m_type == CELL_OBSTACLE || m_type == CELL_IMPASSABLE); }
	void clearObstruction() { m_type = CELL_CLEAR; m_obstacleID = INVALID_ID; m_obstacleIsFence = false; m_obstacleIsTransparent = false; }
#endif

	inline Bool isObstacleTransparent() const;
	inline Bool isObstacleFence() const;

	/// Return estimated cost from given cell to reach goal cell
	UnsignedInt costToGoal( PathfindCell *goal );

	UnsignedInt costToHierGoal( PathfindCell *goal );

	UnsignedInt costSoFar( PathfindCell *parent );

#if RETAIL_COMPATIBLE_PATHFINDING
	// Forward insertion sort that is 100% retail compatible
	void forwardInsertionSortRetailCompatible(PathfindCellList& list);
#endif

	// Forward insertion sort, in ascending cost order
	void forwardInsertionSort(PathfindCellList& list);

	// Reverse insertion sort, in ascending cost order
	void reverseInsertionSort(PathfindCellList& list);

	/// put self on "open" list in ascending cost order
	void putOnSortedOpenList( PathfindCellList &list );

	/// remove self from "open" list
	void removeFromOpenList( PathfindCellList &list );

	/// put self on "closed" list, return new list
	void putOnClosedList( PathfindCellList &list );

	/// remove self from "closed" list
	void removeFromClosedList( PathfindCellList &list );

	/// remove all cells from closed list.
	static Int releaseClosedList( PathfindCellList &list );

	/// remove all cells from closed list.
	static Int releaseOpenList( PathfindCellList &list );

	inline PathfindCell *getNextOpen() {return m_info->m_nextOpen?m_info->m_nextOpen->m_cell: nullptr;}
	inline PathfindCell *getPrevOpen() {return m_info->m_prevOpen?m_info->m_prevOpen->m_cell: nullptr;}

	inline UnsignedShort getXIndex() const {return m_info->m_pos.x;}
	inline UnsignedShort getYIndex() const {return m_info->m_pos.y;}

	inline Bool isBlockedByAlly() const;
	inline void setBlockedByAlly(Bool blocked);

	inline Bool getOpen() const {return m_info->m_open;}
	inline Bool getClosed() const {return m_info->m_closed;}
	inline UnsignedInt getCostSoFar() const {return m_info->m_costSoFar;}
	inline UnsignedInt getTotalCost() const {return m_info->m_totalCost;}

	inline UnsignedInt getTotalCostDifference(PathfindCell& other) const;

	inline void setCostSoFar(UnsignedInt cost) { if( m_info ) m_info->m_costSoFar = cost;}
	inline void setTotalCost(UnsignedInt cost) { if( m_info ) m_info->m_totalCost = cost;}

	void setParentCell(PathfindCell* parent);
	void clearParentCell();
	void setParentCellHierarchical(PathfindCell* parent);
	inline PathfindCell* getParentCell() const {return m_info ? m_info->m_pathParent ? m_info->m_pathParent->m_cell : nullptr : nullptr;}

	Bool startPathfind( PathfindCell *goalCell );
	Bool getPinched() const {return m_pinched;}
	void setPinched(Bool pinch) {m_pinched = pinch;	}

	Bool allocateInfo(const ICoord2D &pos);
	void releaseInfo();
	Bool hasInfo() const {return m_info!=nullptr;}
	zoneStorageType getZone() const {return m_zone;}
	void setZone(zoneStorageType zone) {m_zone = zone;}
	void setGoalUnit(ObjectID unit, const ICoord2D &pos );
	void setGoalAircraft(ObjectID unit, const ICoord2D &pos );
	void setPosUnit(ObjectID unit, const ICoord2D &pos );
	inline ObjectID getGoalUnit() const {ObjectID id = m_info?m_info->m_goalUnitID:INVALID_ID; return id;}
	inline ObjectID getGoalAircraft() const {ObjectID id = m_info?m_info->m_goalAircraftID:INVALID_ID; return id;}
	inline ObjectID getPosUnit() const {ObjectID id = m_info?m_info->m_posUnitID:INVALID_ID; return id;}

	inline ObjectID getObstacleID() const;

	void setLayer( PathfindLayerEnum layer ) { m_layer = layer; }	///< set the cell layer
	PathfindLayerEnum getLayer() const { return (PathfindLayerEnum)m_layer; }				///< get the cell layer

	void setConnectLayer( PathfindLayerEnum layer ) { m_connectsToLayer = layer; }	///< set the cell layer	connect id
	PathfindLayerEnum getConnectLayer() const { return (PathfindLayerEnum)m_connectsToLayer; }				///< get the cell layer connect id

private:
	PathfindCellInfo *m_info;
	ObjectID m_obstacleID;	                  ///< the object ID who overlaps this cell
	UnsignedInt m_blockedByAlly : 1;          ///< True if this cell is blocked by an allied unit.
	UnsignedInt m_obstacleIsFence : 1;        ///< True if occupied by a fence.
	UnsignedInt m_obstacleIsTransparent : 1;  ///< True if obstacle is transparent (undefined if obstacleid is invalid)

	zoneStorageType m_zone : 14;              ///< Zone. Each zone is a set of adjacent terrain type.  If from & to in the same zone, you can successfully pathfind.  If not,
	                                          /// you still may be able to if you can cross multiple terrain types.
	UnsignedShort m_aircraftGoal : 1;         ///< This is an aircraft goal cell.
	UnsignedShort m_pinched : 1;              ///< This cell is surrounded by obstacle cells.
	UnsignedByte m_type : 4;                  ///< what type of cell terrain this is.
	UnsignedByte m_flags : 4;                 ///< what type of units are in or moving through this cell.
	UnsignedByte m_connectsToLayer : 4;       ///< This cell can pathfind onto this layer, if > LAYER_TOP.
	UnsignedByte m_layer : 4;                 ///< Layer of this cell.
};

typedef PathfindCell *PathfindCellP;
