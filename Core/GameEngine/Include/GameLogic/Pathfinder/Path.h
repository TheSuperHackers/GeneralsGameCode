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

#include "Common/GameMemory.h"
#include "Common/Snapshot.h"
#include "GameLogic/LocomotorSet.h"

class Object;
class PathNode;

struct ClosestPointOnPathInfo
{
	Real								distAlongPath;
	Coord3D							posOnPath;
	PathfindLayerEnum		layer;
};

/**
 * This class encapsulates a "path" returned by the Pathfinder.
 */
class Path : public MemoryPoolObject, public Snapshot
{
public:
	Path();

	PathNode *getFirstNode() { return m_path; }
	PathNode *getLastNode() { return m_pathTail; }

	void updateLastNode( const Coord3D *pos );

	void prependNode( const Coord3D *pos, PathfindLayerEnum layer );				///< Create a new node at the head of the path
	void appendNode( const Coord3D *pos, PathfindLayerEnum layer );				///< Create a new node at the end of the path
	void setBlockedByAlly(Bool blocked) {m_blockedByAlly = blocked;}
	Bool getBlockedByAlly() {return m_blockedByAlly;}
	void optimize( const Object *obj, LocomotorSurfaceTypeMask acceptableSurfaces, Bool blocked );			///< Optimize the path to discard redundant nodes

	void optimizeGroundPath( Bool crusher, Int diameter );			///< Optimize the ground path to discard redundant nodes

	/// Given a location, return nearest location on path, and along-path dist to end as function result
	void computePointOnPath( const Object *obj, const LocomotorSet& locomotorSet, const Coord3D& pos, ClosestPointOnPathInfo& out);

	/// Given a location, return nearest location on path, and along-path dist to end as function result
	void peekCachedPointOnPath( Coord3D& pos ) const {pos = m_cpopOut.posOnPath;}

	/// Given a flight path, compute the distance to goal (0 if we are past it) & return the goal pos.
	Real computeFlightDistToGoal( const Coord3D *pos, Coord3D& goalPos );

	/// Given a location, return closest location on path, and along-path dist to end as function result
	void markOptimized() {m_isOptimized = true;}

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer ) override;
	virtual void xfer( Xfer *xfer ) override;
	virtual void loadPostProcess() override;

protected:
	enum {MAX_CPOP=20};			///< Max times we will return the cached cpop.
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( Path, "PathPool" );							///< @todo Set real numbers for mem alloc

	PathNode*		m_path;															///< The list of PathNode objects that define the path
	PathNode*		m_pathTail;
	Bool				m_isOptimized;											///< True if the path has been optimized
	Bool				m_blockedByAlly;										///< An ally needs to move off of this path.
	// caching info for computePointOnPath.
	Bool										m_cpopValid;
	Int											m_cpopCountdown;				///< We only return the same cpop MAX_CPOP times.  It is occasionally possible to get stuck.
	Coord3D									m_cpopIn;
	ClosestPointOnPathInfo	m_cpopOut;
	const PathNode*					m_cpopRecentStart;
};
