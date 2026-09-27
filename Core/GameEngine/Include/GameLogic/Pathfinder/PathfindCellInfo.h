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

class PathfindCell;

class PathfindCellInfo
{
	friend class PathfindCell;
public:
#if RETAIL_COMPATIBLE_PATHFINDING
	static void forceCleanPathFindCellInfos();
#endif
	static void allocateCellInfos();
	static void releaseCellInfos();

	static PathfindCellInfo * getACellInfo(PathfindCell *cell, const ICoord2D &pos);
	static void releaseACellInfo(PathfindCellInfo *theInfo);

protected:
	static PathfindCellInfo *s_infoArray;
	static PathfindCellInfo *s_firstFree;							///<


	PathfindCellInfo *m_nextOpen, *m_prevOpen;						///< for A* "open" list, shared by closed list

	PathfindCellInfo *m_pathParent;												///< "parent" cell from pathfinder
	PathfindCell *m_cell;															///< Cell this info belongs to currently.

	UnsignedShort m_totalCost, m_costSoFar;	///< cost estimates for A* search

	/// have to include cell's coordinates, since cells are often accessed via pointer only
	ICoord2D m_pos;

	ObjectID m_goalUnitID; ///< The objectID of the ground unit whose goal this is.
	ObjectID m_posUnitID;  ///< The objectID of the ground unit that is occupying this cell.
	ObjectID m_goalAircraftID; ///< The objectID of the aircraft whose goal this is.

	ObjectID m_obstacleID;	///< the object ID who overlaps this cell

	UnsignedInt m_isFree:1;
	UnsignedInt m_blockedByAlly:1;///< True if this cell is blocked by an allied unit.
	UnsignedInt m_obstacleIsFence:1;///< True if occupied by a fence.
	UnsignedInt m_obstacleIsTransparent:1;///< True if obstacle is transparent (undefined if obstacleid is invalid)
	/// @todo Do we need both mark values in this cell?  Can't store a single value and compare it?
	UnsignedInt m_open:1;													///< place for marking this cell as on the open list
	UnsignedInt m_closed:1;												///< place for marking this cell as on the closed list
};
