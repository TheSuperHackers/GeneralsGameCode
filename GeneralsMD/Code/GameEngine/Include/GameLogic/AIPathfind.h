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

// AIPathfind.h
// AI pathfinding system
// Author: Michael S. Booth, October 2001

#pragma once

#ifndef _PATHFIND_H_
#define _PATHFIND_H_

#include "Common/GameType.h"
#include "Common/GameMemory.h"
#include "Common/Snapshot.h"
//#include "GameLogic/Locomotor.h"	// no, do not include this, unless you like long recompiles
#include "GameLogic/LocomotorSet.h"
#include "GameLogic/GameLogic.h"

class Bridge;
class Object;
class Weapon;
class PathfindZoneManager;

// How close is close enough when moving.

#define PATHFIND_CLOSE_ENOUGH 1.0f
#define PATH_MAX_PRIORITY 0x7FFFFFFF

#define INFANTRY_MOVES_THROUGH_INFANTRY


  typedef UnsignedShort zoneStorageType;


//----------------------------------------------------------------------------------------------------------

/**
 * PathNodes are used to create a final Path to return from the
 * pathfinder.  Note that these are not used during the A* search.
 */
class PathNode : public MemoryPoolObject
{
public:
	PathNode();

	Coord3D *getPosition( void ) { return &m_pos; }			///< return position of this node
	const Coord3D *getPosition( void ) const { return &m_pos; }			///< return position of this node
	void setPosition( const Coord3D *pos ) { m_pos = *pos; }	///< set the position of this path node

	const Coord3D *computeDirectionVector( void );			///< compute direction to next node

	PathNode *getNext( void ) { return m_next; }				///< return next node in the path
	PathNode *getPrevious( void ) { return m_prev; }		///< return previous node in the path
	const PathNode *getNext( void ) const { return m_next; }				///< return next node in the path
	const PathNode *getPrevious( void ) const { return m_prev; }		///< return previous node in the path

	PathfindLayerEnum getLayer( void ) const { return m_layer; }				///< return layer of this node.
	void setLayer( PathfindLayerEnum layer ) { m_layer = layer; }	///< set the layer of this path node

	void setNextOptimized( PathNode *node );

	PathNode *getNextOptimized(Coord2D* dir = NULL, Real* dist = NULL)  	///< return next node in optimized path
	{
		if (dir)
			*dir = m_nextOptiDirNorm2D;
		if (dist)
			*dist = m_nextOptiDist2D;
		return m_nextOpti;
	}

	const PathNode *getNextOptimized(Coord2D* dir = NULL, Real* dist = NULL) const  	///< return next node in optimized path
	{
		if (dir)
			*dir = m_nextOptiDirNorm2D;
		if (dist)
			*dist = m_nextOptiDist2D;
		return m_nextOpti;
	}

	void setCanOptimize(Bool canOpt) { m_canOptimize = canOpt;}
	Bool getCanOptimize( void ) const { return m_canOptimize;}

	/// given a list, prepend this node, return new list
	PathNode *prependToList( PathNode *list );

	/// given a list, append this node, return new list.  slow implementation.
	/// @todo optimize this
	PathNode *appendToList( PathNode *list );

	/// given a node, append to this node
	void append( PathNode *list );

public:
	mutable Int					m_id; // Used in Path::xfer() to save & recreate the path list.

private:
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( PathNode, "PathNodePool"  );		///< @todo Set real numbers for mem alloc

	PathNode*						m_nextOpti;													///< next node in the optimized path
	PathNode*						m_next;															///< next node in the path
	PathNode*						m_prev;															///< previous node in the path
	Coord3D							m_pos;															///< position of node in space
	PathfindLayerEnum		m_layer;														///< Layer for this section.
	Bool								m_canOptimize;											///< True if this cell can be optimized out.

	Real								m_nextOptiDist2D;										///< if nextOpti is nonnull, the dist to it.
	Coord2D							m_nextOptiDirNorm2D;								///< if nextOpti is nonnull, normalized dir vec towards it.

};

// this doesn't actually seem to be a particularly useful win,
// performance-wise, so I didn't enable it. (srj)
#define NO_CPOP_STARTS_FROM_PREV_SEG

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

	PathNode *getFirstNode( void ) { return m_path; }
	PathNode *getLastNode( void ) { return m_pathTail; }

	void updateLastNode( const Coord3D *pos );

	void prependNode( const Coord3D *pos, PathfindLayerEnum layer );				///< Create a new node at the head of the path
	void appendNode( const Coord3D *pos, PathfindLayerEnum layer );				///< Create a new node at the end of the path
	void setBlockedByAlly(Bool blocked) {m_blockedByAlly = blocked;}
	Bool getBlockedByAlly(void) {return m_blockedByAlly;}
	void optimize( const Object *obj, LocomotorSurfaceTypeMask acceptableSurfaces, Bool blocked );			///< Optimize the path to discard redundant nodes

	void optimizeGroundPath( Bool crusher, Int diameter );			///< Optimize the ground path to discard redundant nodes

	/// Given a location, return nearest location on path, and along-path dist to end as function result
	void computePointOnPath( const Object *obj, const LocomotorSet& locomotorSet, const Coord3D& pos, ClosestPointOnPathInfo& out);

	/// Given a location, return nearest location on path, and along-path dist to end as function result
	void peekCachedPointOnPath( Coord3D& pos ) const {pos = m_cpopOut.posOnPath;}

	/// Given a flight path, compute the distance to goal (0 if we are past it) & return the goal pos.
	Real computeFlightDistToGoal( const Coord3D *pos, Coord3D& goalPos );

	/// Given a location, return closest location on path, and along-path dist to end as function result
	void markOptimized(void) {m_isOptimized = true;}

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

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

//----------------------------------------------------------------------------------------------------------

// See GameType.h for
// enum {LAYER_INVALID = 0, LAYER_GROUND = 1, LAYER_TOP=2 };

// Fits in 4 bits for now
enum {MAX_WALL_PIECES = 128};

class PathfindCellInfo
{
	friend class PathfindCell;
public:
#if RETAIL_COMPATIBLE_PATHFINDING
	static void forceCleanPathFindCellInfos(void);
#endif
	static void allocateCellInfos(void);
	static void releaseCellInfos(void);

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
	void reset( );

	PathfindCell(void);
	~PathfindCell(void);

	Bool setTypeAsObstacle( Object *obstacle, Bool isFence, const ICoord2D &pos );				///< flag this cell as an obstacle, from the given one
	Bool removeObstacle( Object *obstacle );				///< unflag this cell as an obstacle, from the given one
	void setType( CellType type );	///< set the cell type
	CellType getType( void ) const { return (CellType)m_type; }				///< get the cell type
	CellFlags getFlags( void ) const { return (CellFlags)m_flags; }				///< get the cell type
	Bool isAircraftGoal( void) const {return m_aircraftGoal != 0;}

	Bool isObstaclePresent( ObjectID objID ) const;					///< return true if the given object ID is registered as an obstacle in this cell

	Bool isObstacleTransparent( ) const{return m_info?m_info->m_obstacleIsTransparent:false; }					///< return true if the obstacle in the cell is KINDOF_CAN_SEE_THROUGHT_STRUCTURE

	Bool isObstacleFence( void ) const {return m_info?m_info->m_obstacleIsFence:false; }///< return true if the given obstacle in the cell is a fence.

	/// Return estimated cost from given cell to reach goal cell
	UnsignedInt costToGoal( PathfindCell *goal );

	UnsignedInt costToHierGoal( PathfindCell *goal );

	UnsignedInt costSoFar( PathfindCell *parent );

	/// put self on "open" list in ascending cost order, return new list
	PathfindCell *putOnSortedOpenList( PathfindCell *list );

	/// remove self from "open" list
	PathfindCell *removeFromOpenList( PathfindCell *list );

	/// put self on "closed" list, return new list
	PathfindCell *putOnClosedList( PathfindCell *list );

	/// remove self from "closed" list
	PathfindCell *removeFromClosedList( PathfindCell *list );

	/// remove all cells from closed list.
	static Int releaseClosedList( PathfindCell *list );

	/// remove all cells from closed list.
	static Int releaseOpenList( PathfindCell *list );

	inline PathfindCell *getNextOpen(void) {return m_info->m_nextOpen?m_info->m_nextOpen->m_cell:NULL;}

	inline UnsignedShort getXIndex(void) const {return m_info->m_pos.x;}
	inline UnsignedShort getYIndex(void) const {return m_info->m_pos.y;}

	inline Bool isBlockedByAlly(void) const {return m_info->m_blockedByAlly;}
	inline void setBlockedByAlly(Bool blocked)  {m_info->m_blockedByAlly = (blocked!=0);}

	inline Bool getOpen(void) const {return m_info->m_open;}
	inline Bool getClosed(void) const {return m_info->m_closed;}
	inline UnsignedInt getCostSoFar(void) const {return m_info->m_costSoFar;}
	inline UnsignedInt getTotalCost(void) const {return m_info->m_totalCost;}

	inline void setCostSoFar(UnsignedInt cost) { if( m_info ) m_info->m_costSoFar = cost;}
	inline void setTotalCost(UnsignedInt cost) { if( m_info ) m_info->m_totalCost = cost;}

	void setParentCell(PathfindCell* parent);
	void clearParentCell(void);
	void setParentCellHierarchical(PathfindCell* parent);
	inline PathfindCell* getParentCell(void) const {return m_info ? m_info->m_pathParent ? m_info->m_pathParent->m_cell : NULL : NULL;}

	Bool startPathfind( PathfindCell *goalCell );
	Bool getPinched(void) const {return m_pinched;}
	void setPinched(Bool pinch) {m_pinched = pinch;	}

	Bool allocateInfo(const ICoord2D &pos);
	void releaseInfo(void);
	Bool hasInfo(void) const {return m_info!=NULL;}
	zoneStorageType getZone(void) const {return m_zone;}
	void setZone(zoneStorageType zone) {m_zone = zone;}
	void setGoalUnit(ObjectID unit, const ICoord2D &pos );
	void setGoalAircraft(ObjectID unit, const ICoord2D &pos );
	void setPosUnit(ObjectID unit, const ICoord2D &pos );
	inline ObjectID getGoalUnit(void) const {ObjectID id = m_info?m_info->m_goalUnitID:INVALID_ID; return id;}
	inline ObjectID getGoalAircraft(void) const {ObjectID id = m_info?m_info->m_goalAircraftID:INVALID_ID; return id;}
	inline ObjectID getPosUnit(void) const {ObjectID id = m_info?m_info->m_posUnitID:INVALID_ID; return id;}

	inline ObjectID getObstacleID(void) const {ObjectID id = m_info?m_info->m_obstacleID:INVALID_ID; return id;}

	void setLayer( PathfindLayerEnum layer ) { m_layer = layer; }	///< set the cell layer
	PathfindLayerEnum getLayer( void ) const { return (PathfindLayerEnum)m_layer; }				///< get the cell layer

	void setConnectLayer( PathfindLayerEnum layer ) { m_connectsToLayer = layer; }	///< set the cell layer	connect id
	PathfindLayerEnum getConnectLayer( void ) const { return (PathfindLayerEnum)m_connectsToLayer; }				///< get the cell layer connect id

private:
	PathfindCellInfo *m_info;
	zoneStorageType m_zone:14;			///< Zone. Each zone is a set of adjacent terrain type.  If from & to in the same zone, you can successfully pathfind.  If not,
														// you still may be able to if you can cross multiple terrain types.
	UnsignedShort m_aircraftGoal:1; //< This is an aircraft goal cell.
	UnsignedShort m_pinched:1; //< This cell is surrounded by obstacle cells.
	UnsignedByte m_type:4;			///< what type of cell terrain this is.
	UnsignedByte m_flags:4;			///< what type of units are in or moving through this cell.
	UnsignedByte m_connectsToLayer:4;	///< This cell can pathfind onto this layer, if > LAYER_TOP.
  UnsignedByte m_layer:4;					 ///< Layer of this cell.
};

typedef PathfindCell *PathfindCellP;


// how close a unit has to be in z to interact with the layer.
#define LAYER_Z_CLOSE_ENOUGH_F 10.0f
/**
 * This class represents a bridge in the map. This is effectively
 * a sub-rectangle of the big pathfind map.
 */
class PathfindLayer
{
public:
	PathfindLayer();
	~PathfindLayer();
public:
	void reset(void);
	Bool init(Bridge *theBridge, PathfindLayerEnum layer);
	void allocateCells(const IRegion2D *extent);
	void allocateCellsForWallLayer(const IRegion2D *extent, ObjectID *wallPieces, Int numPieces);
	void classifyCells();
	void classifyWallCells(ObjectID *wallPieces, Int numPieces);
	Bool setDestroyed(Bool destroyed);
	Bool isUnused(void); // True if it doesn't contain a bridge.
	Bool isDestroyed(void) {return m_destroyed;} // True if it has been destroyed.
	PathfindCell *getCell(Int x, Int y);
	Int getZone(void) {return m_zone;}
	void setZone(Int zone) {m_zone = zone;}
	void applyZone(void); // Propagates m_zone to all cells.
	void getStartCellIndex(ICoord2D *start) {*start = m_startCell;}
	void getEndCellIndex(ICoord2D *end) {*end = m_endCell;}

	ObjectID getBridgeID(void);
	Bool connectsZones(PathfindZoneManager *zm, const LocomotorSet& locomotorSet,Int zone1, Int zone2);
	Bool isPointOnWall(ObjectID *wallPieces, Int numPieces, const Coord3D *pt);

#if defined(RTS_DEBUG)
	void doDebugIcons(void) ;
#endif
protected:
	void classifyLayerMapCell( Int i, Int j , PathfindCell *cell, Bridge *theBridge);
	void classifyWallMapCell( Int i, Int j, PathfindCell *cell , ObjectID *wallPieces, Int numPieces);

private:
	PathfindCell *m_blockOfMapCells;		///< Pathfinding map - contains iconic representation of the map
	PathfindCell **m_layerCells;		///< Pathfinding map indexes - contains matrix indexing into the map.
	Int m_width;		// Number of cells in x
	Int m_height;		// Number of cells in y
	Int m_xOrigin;	// Index of first cell in x
	Int m_yOrigin;	// Index of first cell in y
	ICoord2D m_startCell; // pathfind cell indexes for center cell on the from side.
	ICoord2D m_endCell; // pathfind cell indexes for center cell on the to side.

	PathfindLayerEnum m_layer;
	Int m_zone;			// Whole bridge is in same zone.
	Bridge *m_bridge; // Corresponding bridge in TerrainLogic.
	Bool m_destroyed;


};


#define PATHFIND_CELL_SIZE		10
#define PATHFIND_CELL_SIZE_F	10.0f

enum { PATHFIND_QUEUE_LEN=512};

struct TCheckMovementInfo;

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

	void clearMarkedPassable(void) {m_markedPassable = false;}
	Bool isPassable(void) {return m_markedPassable;}
	void setPassable(Bool pass) {m_markedPassable = pass;}

	Bool getInteractsWithBridge(void) const {return m_interactsWithBridge;}
	void setInteractsWithBridge(Bool interacts) {m_interactsWithBridge = interacts;}

protected:
	void allocateZones(void);
	void freeZones(void);

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

	void reset(void);

	Bool needToCalculateZones(void) const {return m_nextFrameToCalculateZones <= TheGameLogic->getFrame() ;} ///< Returns true if the zones need to be recalculated.
 	void markZonesDirty( Bool insert ) ; ///< Called when the zones need to be recalculated.
 	void updateZonesForModify( PathfindCell **map,  PathfindLayer layers[], const IRegion2D &structureBounds, const IRegion2D &globalBounds ) ; ///< Called to recalculate an area when a structure has been removed.
	void calculateZones(	PathfindCell **map, PathfindLayer layers[], const IRegion2D &bounds);	///< Does zone calculations.
	zoneStorageType getEffectiveZone(LocomotorSurfaceTypeMask acceptableSurfaces, Bool crusher, zoneStorageType zone) const;
	zoneStorageType getEffectiveTerrainZone(zoneStorageType zone) const;

	zoneStorageType getNextZone(void);

	void getExtent(ICoord2D &extent) const {extent = m_zoneBlockExtent;}

	/// return zone relative the the block zone that this cell resides in.
	zoneStorageType getBlockZone(LocomotorSurfaceTypeMask acceptableSurfaces, Bool crusher, Int cellX, Int cellY, PathfindCell **map) const;
	void allocateBlocks(const IRegion2D &globalBounds);

	void clearPassableFlags(void);
	Bool isPassable(Int cellX, Int cellY) const;
	Bool clipIsPassable(Int cellX, Int cellY) const;
	void setPassable(Int cellX, Int cellY, Bool passable);

	void setAllPassable(void);

	void setBridge(Int cellX, Int cellY, Bool bridge);
	Bool interactsWithBridge(Int cellX, Int cellY) const;

private:
	void allocateZones(void);
	void freeZones(void);
	void freeBlocks(void);

private:
	ZoneBlock			*m_blockOfZoneBlocks;			///< Zone blocks - Info for hierarchical pathfinding at a "blocky" level.
	ZoneBlock			**m_zoneBlocks;						///< Zone blocks as a matrix - contains matrix indexing into the map.
	ICoord2D			m_zoneBlockExtent;				///< Zone block extents. Not the same scale as the pathfind extents.

	UnsignedShort m_maxZone;								///< Max zone used.
	UnsignedInt		m_nextFrameToCalculateZones;		///< WHen should I recalculate, next?.
	UnsignedShort m_zonesAllocated;
	zoneStorageType *m_groundCliffZones;
	zoneStorageType *m_groundWaterZones;
	zoneStorageType *m_groundRubbleZones;
	zoneStorageType *m_terrainZones;
	zoneStorageType *m_crusherZones;
	zoneStorageType *m_hierarchicalZones;
};

/**
 * The pathfinding services interface provides access to the 3 expensive path find calls:
 * findPath, findClosestPath, and findAttackPath.
 * It is only available to units when their ai interface doPathfind method is called.
 * This allows the pathfinder to spread out the pathfinding over a number of frames
 * when a lot of units are trying to pathfind all at the same time.
 */
class PathfindServicesInterface {
public:
	virtual Path *findPath( Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		const Coord3D *to )=0;	///< Find a short, valid path between given locations
	/** Find a short, valid path to a location NEAR the to location.
		This succeds when the destination is unreachable (like inside a building).
		If the destination is unreachable, it will adjust the to point.  */
	virtual Path *findClosestPath( Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		Coord3D *to, Bool blocked, Real pathCostMultiplier, Bool moveAllies )=0;

	/** Find a short, valid path to a location that obj can attack victim from.  */
	virtual Path *findAttackPath( const Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		const Object *victim, const Coord3D* victimPos, const Weapon *weapon )=0;

	/** Patch to the exiting path from the current position, either because we became blocked,
  or because we had to move off the path to avoid other units. */
	virtual Path *patchPath( const Object *obj, const LocomotorSet& locomotorSet,
		Path *originalPath, Bool blocked ) = 0;

	/** Find a short, valid path to a location that is away from the repulsors.  */
	virtual Path *findSafePath( const Object *obj, const LocomotorSet& locomotorSet,
		const Coord3D *from, const Coord3D* repulsorPos1, const Coord3D* repulsorPos2, Real repulsorRadius ) = 0;

};

/**
 * The Pathfinding engine itself.
 */
class Pathfinder : PathfindServicesInterface, public Snapshot
{
// The following routines are private, but available through the doPathfind callback to aiInterface. jba.
private:
	virtual Path *findPath( Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to);	///< Find a short, valid path between given locations
	/** Find a short, valid path to a location NEAR the to location.
		This succeds when the destination is unreachable (like inside a building).
		If the destination is unreachable, it will adjust the to point.  */
	virtual Path *findClosestPath( Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		Coord3D *to, Bool blocked, Real pathCostMultiplier, Bool moveAllies );

	/** Find a short, valid path to a location that obj can attack victim from.  */
	virtual Path *findAttackPath( const Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		const Object *victim, const Coord3D* victimPos, const Weapon *weapon );

	/** Find a short, valid path to a location that is away from the repulsors.  */
	virtual Path *findSafePath( const Object *obj, const LocomotorSet& locomotorSet,
		const Coord3D *from, const Coord3D* repulsorPos1, const Coord3D* repulsorPos2, Real repulsorRadius );

	/** Patch to the exiting path from the current position, either because we became blocked,
  or because we had to move off the path to avoid other units. */
	virtual Path *patchPath( const Object *obj, const LocomotorSet& locomotorSet,
		Path *originalPath, Bool blocked );

public:
	Pathfinder( void );
	~Pathfinder() ;

	void reset( void );														///< Reset system in preparation for new map

	// --------------- inherited from Snapshot interface --------------
	void crc( Xfer *xfer );
	void xfer( Xfer *xfer );
	void loadPostProcess( void );

	Bool clientSafeQuickDoesPathExist( const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to );  ///< Can we build any path at all between the locations	(terrain & buildings check - fast)
	Bool clientSafeQuickDoesPathExistForUI( const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to );  ///< Can we build any path at all between the locations	(terrain onlyk - fast)
	Bool slowDoesPathExist( Object *obj, const Coord3D *from,
		const Coord3D *to, ObjectID ignoreObject=INVALID_ID );  ///< Can we build any path at all between the locations	(terrain, buildings & units check - slower)

	Bool queueForPath(ObjectID id);	 ///< The object wants to request a pathfind, so put it on the list to process.
	void processPathfindQueue(void); ///< Process some or all of the queued pathfinds.
	void forceMapRecalculation( );	///< Force pathfind map recomputation. If region is given, only that area is recomputed

	/** Returns an aircraft path to the goal.  */
	Path *getAircraftPath( const Object *obj, const Coord3D *to);
	Path *findGroundPath( const Coord3D *from, const Coord3D *to, Int pathRadius,
		Bool crusher);	///< Find a short, valid path of the desired width on the ground.

	void addObjectToPathfindMap( class Object *obj );				///< Classify the given object's cells in the map
	void removeObjectFromPathfindMap( class Object *obj );	///< De-classify the given object's cells in the map

	void removeUnitFromPathfindMap( Object *obj );	///< De-classify the given mobile unit's cells in the map
	void updateGoal( Object *obj, const Coord3D *newGoalPos, PathfindLayerEnum layer);		///< Update the given mobile unit's cells in the map
	void updateAircraftGoal( Object *obj, const Coord3D *newGoalPos);		///< Update the given aircraft unit's cells in the map
	void removeGoal( Object *obj);		///< Removes the given mobile unit's goal cells in the map
	void updatePos( Object *obj, const Coord3D *newPos);		///< Update the given mobile unit's cells in the map
	void removePos( Object *obj);		///< Removes the unit's position cells from the map

	Bool moveAllies(Object *obj, Path *path);

	// NOTE - The object MUST NOT MOVE between the call to createAWall... and removeWall...
	// or BAD THINGS will happen.  jba.
	void createAWallFromMyFootprint( Object *obj ) {internal_classifyObjectFootprint(obj, true);}  // Temporarily treat this object as an obstacle.
	void removeWallFromMyFootprint( Object *obj ){internal_classifyObjectFootprint(obj, false);}   // Undo createAWallFromMyFootprint.

	Path *getMoveAwayFromPath(Object *obj, Object *otherObj, Path *pathToAvoid, Object *otherObj2, Path *pathToAvoid2);

	void changeBridgeState( PathfindLayerEnum layer, Bool repaired );

	Bool findBrokenBridge(const LocomotorSet &locomotorSet, const Coord3D *from, const Coord3D *to, ObjectID *bridgeID);

	void newMap(void);

	PathfindCell *getCell( PathfindLayerEnum layer, Int x, Int y );							///< Return the cell at grid coords (x,y)
	PathfindCell *getCell( PathfindLayerEnum layer, const Coord3D *pos );				///< Given a position, return associated grid cell
	PathfindCell *getClippedCell( PathfindLayerEnum layer, const Coord3D *pos );				///< Given a position, return associated grid cell
	void clip(Coord3D *from, Coord3D *to);
	Bool worldToCell( const Coord3D *pos, ICoord2D *cell );	///< Given a world position, return grid cell coordinate

	const ICoord2D *getExtent(void) const {return &m_extent.hi;}

	void setIgnoreObstacleID( ObjectID objID );					///< if non-zero, the pathfinder will ignore the given obstacle

	Bool validMovementPosition( Bool isCrusher, LocomotorSurfaceTypeMask acceptableSurfaces, PathfindCell *toCell, PathfindCell *fromCell = NULL );		///< Return true if given position is a valid movement location
	Bool validMovementPosition( Bool isCrusher, PathfindLayerEnum layer, const LocomotorSet& locomotorSet, Int x, Int y );					///< Return true if given position is a valid movement location
	Bool validMovementPosition( Bool isCrusher, PathfindLayerEnum layer, const LocomotorSet& locomotorSet, const Coord3D *pos );		///< Return true if given position is a valid movement location
	Bool validMovementTerrain( PathfindLayerEnum layer, const Locomotor* locomotor, const Coord3D *pos );		///< Return true if given position is a valid movement location

	Locomotor* chooseBestLocomotorForPosition(PathfindLayerEnum layer,  LocomotorSet* locomotorSet, const Coord3D* pos );

	Bool isViewBlockedByObstacle(const Object* obj, const Object* objOther);	///< Return true if the straight line between the given points contains any obstacle, and thus blocks vision

	Bool isAttackViewBlockedByObstacle(const Object* obj, const Coord3D& attackerPos,  const Object* victim, const Coord3D& victimPos);	///< Return true if the straight line between the given points contains any obstacle, and thus blocks vision

	Bool isLinePassable( const Object *obj, LocomotorSurfaceTypeMask acceptableSurfaces,
		PathfindLayerEnum layer, const Coord3D& startWorld, const Coord3D& endWorld,
		Bool blocked, Bool allowPinched );	///< Return true if the straight line between the given points is passable

	void moveAlliesAwayFromDestination( Object *obj,const Coord3D& destination);

	Bool isGroundPathPassable( Bool isCrusher, const Coord3D& startWorld, PathfindLayerEnum startLayer,
		const Coord3D& endWorld, Int pathDiameter);	///< Return true if the straight line between the given points is passable

	// for debugging
	const Coord3D *getDebugPathPosition( void );
	void setDebugPathPosition( const Coord3D *pos );
	Path *getDebugPath( void );
	void setDebugPath( Path *debugpath );

#if RETAIL_COMPATIBLE_PATHFINDING
	void forceCleanCells(void);
#endif
	void cleanOpenAndClosedLists(void);

	// Adjusts the destination to a spot near dest that is not occupied by other units.
	Bool adjustDestination(Object *obj, const LocomotorSet& locomotorSet,
		Coord3D *dest, const Coord3D *groupDest=NULL);

	// Adjusts the destination to a spot near dest for landing that is not occupied by other units.
	Bool adjustToLandingDestination(Object *obj, Coord3D *dest);

	// Adjusts the destination to a spot that can attack target that is not occupied by other units.
	Bool adjustTargetDestination(const Object *obj, const Object *target, const Coord3D *targetPos,
		const Weapon *weapon, Coord3D *dest);

	// Adjusts destination to a spot near dest that is possible to path to.
	Bool adjustToPossibleDestination(Object *obj, const LocomotorSet& locomotorSet, Coord3D *dest);

	void snapPosition(Object *obj, Coord3D *pos); // Snaps the current position to it's grid location.
	void snapClosestGoalPosition(Object *obj, Coord3D *pos); // Snaps the current position to a good goal position.
	Bool goalPosition(Object *obj, Coord3D *pos); // Returns the goal position on the grid.

	PathfindLayerEnum addBridge(Bridge *theBridge); // Adds a bridge layer, and returns the layer id.

	void addWallPiece(Object *wallPiece); // Adds a wall piece.
	void removeWallPiece(Object *wallPiece);  // Removes a wall piece.
	Real getWallHeight(void) {return m_wallHeight;}
	Bool isPointOnWall(const Coord3D *pos);

	void updateLayer(Object *obj, PathfindLayerEnum layer); ///< Updates object's layer.

	static void classifyMapCell( Int x, Int y, PathfindCell *cell);					///< Classify the given map cell
	Int clearCellForDiameter( Bool crusher, Int cellX, Int cellY, PathfindLayerEnum layer, Int pathDiameter );		///< Return true if given position is a valid movement location

protected:
	virtual Path *internalFindPath( Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to);	///< Find a short, valid path between given locations
	Path *findHierarchicalPath( Bool isHuman, const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to, Bool crusher);
	Path *findClosestHierarchicalPath( Bool isHuman, const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to, Bool crusher);
	Path *internal_findHierarchicalPath( Bool isHuman, const LocomotorSurfaceTypeMask locomotorSurface, const Coord3D *from, const Coord3D *to, Bool crusher, Bool closestOK);
	void processHierarchicalCell( const ICoord2D &scanCell, const ICoord2D &deltaPathfindCell,
																PathfindCell *parentCell,
																PathfindCell *goalCell, zoneStorageType parentZone,
																zoneStorageType *examinedZones, Int &numExZones,
																Bool crusher, Int &cellCount);
	Bool checkForAdjust(Object *, const LocomotorSet& locomotorSet, Bool isHuman, Int cellX, Int cellY,
		PathfindLayerEnum layer, Int iRadius, Bool center,Coord3D *dest, const Coord3D *groupDest) ;
	Bool checkForLanding(Int cellX, Int cellY,
		PathfindLayerEnum layer, Int iRadius, Bool center,Coord3D *dest) ;
	Bool checkForTarget(const Object *obj, 	Int cellX, Int cellY, const Weapon *weapon,
																const Object *victim, const Coord3D *victimPos,
																Int iRadius, Bool center,Coord3D *dest) ;
	Bool checkForPossible(Bool isCrusher, Int fromZone,  Bool center, const LocomotorSet& locomotorSet,
		Int cellX, Int cellY, PathfindLayerEnum layer, Coord3D *dest, Bool startingInObstacle) ;
	void getRadiusAndCenter(const Object *obj, Int &iRadius, Bool &center);
	void adjustCoordToCell(Int cellX, Int cellY, Bool centerInCell, Coord3D &pos, PathfindLayerEnum layer);
	Bool checkDestination(const Object *obj, Int cellX, Int cellY, PathfindLayerEnum layer, Int iRadius, Bool centerInCell);
	Bool checkForMovement(const Object *obj, TCheckMovementInfo &info);
	Bool segmentIntersectsTallBuilding(const PathNode *curNode, PathNode *nextNode,
		ObjectID ignoreBuilding, Coord3D *insertPos1, Coord3D *insertPos2, Coord3D *insertPos3);	///< Return true if the straight line between the given points intersects a tall building.
	Bool circleClipsTallBuilding(const Coord3D *from, const Coord3D *to, Real radius, ObjectID ignoreBuilding, Coord3D *adjustTo);	///< Return true if the circle at the end of the line between the given points intersects a tall building.

	enum {NO_ATTACK=0};
	Int examineNeighboringCells(PathfindCell *parentCell, PathfindCell *goalCell,
										const LocomotorSet& locomotorSet, Bool isHumanPlayer,
										Bool centerInCell, Int radius, const ICoord2D &startCellNdx,
										const Object *obj, Int attackDistance);

 	Bool pathDestination( Object *obj, const LocomotorSet& locomotorSet, Coord3D *dest,
		PathfindLayerEnum layer, const Coord3D *groupDest);	///< Checks cost between given locations

	Int checkPathCost(Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		const Coord3D *to);

	void tightenPath(Object *obj, const LocomotorSet& locomotorSet, Coord3D *from,
		const Coord3D *to);

	/**
		return 0 to continue iterating the line, nonzero to terminate the iteration.
		the nonzero result will be returned as the result of iterateCellsAlongLine().
		iterateCellsAlongLine will return zero if it completes.
	*/
	typedef Int (*CellAlongLineProc)(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
	Int iterateCellsAlongLine(const Coord3D& startWorld, const Coord3D& endWorld,
		PathfindLayerEnum layer, CellAlongLineProc proc, void* userData);

	Int iterateCellsAlongLine(const ICoord2D &start, const ICoord2D &end,
		PathfindLayerEnum layer, CellAlongLineProc proc, void* userData);

	static Int linePassableCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
	static Int groundPathPassableCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
	static Int lineBlockedByObstacleCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
	static Int tightenPathCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
	static Int attackBlockedByObstacleCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
 	static Int examineCellsCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
 	static Int groundCellsCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);
 	static Int moveAlliesDestinationCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);

	static Int segmentIntersectsBuildingCallback(Pathfinder* pathfinder, PathfindCell* from, PathfindCell* to, Int to_x, Int to_y, void* userData);

	void classifyMap( void );					///< Classify all cells in grid as obstacles, etc
	void classifyObjectFootprint( Object *obj, Bool insert );	/** Classify the cells under the given object
																																If 'insert' is true, object is being added
																																If 'insert' is false, object is being removed */
	void internal_classifyObjectFootprint( Object *obj, Bool insert );	/** Classify the cells under the given object
																																If 'insert' is true, object is being added
																																If 'insert' is false, object is being removed */
	void classifyFence( Object *obj, Bool insert );	/** Classify the cells under the given fence object. */
	void classifyUnitFootprint( Object *obj, Bool insert, Bool remove, Bool update );	/** Classify the cells under the given object If 'insert' is true, object is being added */
	/// Convert world coordinate to array index
	void worldToGrid( const Coord3D *pos, ICoord2D *cellIndex );

	Bool evaluateCell(PathfindCell* newCell, PathfindCell *parentCell,
									const LocomotorSet& locomotorSet,
									 Bool centerInCell, Int radius,
									 const Object *obj, Int attackDistance);

	Path *buildActualPath( const Object *obj, LocomotorSurfaceTypeMask acceptableSurfaces,
		const Coord3D *fromPos, PathfindCell *goalCell, Bool center, Bool blocked );	///< Work backwards from goal cell to construct final path
	Path *buildGroundPath( Bool isCrusher,const Coord3D *fromPos, PathfindCell *goalCell,
		Bool center, Int pathDiameter );	///< Work backwards from goal cell to construct final path
	Path *buildHierachicalPath( const Coord3D *fromPos, PathfindCell *goalCell);	///< Work backwards from goal cell to construct final path

	void  prependCells( Path *path, const Coord3D *fromPos,
																	PathfindCell *goalCell, Bool center ); ///< Add pathfind cells to a path.

	void debugShowSearch( Bool pathFound );				///< Show all cells touched in the last search
	static LocomotorSurfaceTypeMask validLocomotorSurfacesForCellType(PathfindCell::CellType t);

	void checkChangeLayers(PathfindCell *parentCell);

#if defined(RTS_DEBUG)
	void doDebugIcons(void) ;
#endif

private:
	/// This uses WAY too much memory.  Should at least be array of pointers to cells w/ many fewer cells
	PathfindCell *m_blockOfMapCells;		///< Pathfinding map - contains iconic representation of the map
	PathfindCell **m_map;		///< Pathfinding map indexes - contains matrix indexing into the map.
	IRegion2D m_extent;														///< Grid extent limits
	IRegion2D m_logicalExtent;										///< Logical grid extent limits

	PathfindCell *m_openList;											///< Cells ready to be explored
	PathfindCell *m_closedList;										///< Cells already explored

	Bool m_isMapReady;														///< True if all cells of map have been classified
	Bool m_isTunneling;														///< True if path started in an obstacle

	Int m_frameToShowObstacles;										///< Time to redraw obstacles.  For debug output.

	Coord3D debugPathPos;													///< Used for visual debugging
	Path *debugPath;															///< Used for visual debugging

	ObjectID m_ignoreObstacleID;									///< Ignore the given obstacle

	PathfindZoneManager m_zoneManager;						///< Handles the pathfind zones.

	PathfindLayer m_layers[LAYER_LAST+1];

	ObjectID			m_wallPieces[MAX_WALL_PIECES];
	Int						m_numWallPieces;
	Real					m_wallHeight;

	Int						m_moveAlliesDepth;


	// Pathfind queue
	ObjectID			m_queuedPathfindRequests[PATHFIND_QUEUE_LEN];
	Int						m_queuePRHead;
	Int						m_queuePRTail;
	Int						m_cumulativeCellsAllocated;
};


inline void Pathfinder::setIgnoreObstacleID( ObjectID objID )
{
	m_ignoreObstacleID = objID;
}

inline void Pathfinder::worldToGrid( const Coord3D *pos, ICoord2D *cellIndex )
{
	cellIndex->x = REAL_TO_INT(pos->x/PATHFIND_CELL_SIZE);
	cellIndex->y = REAL_TO_INT(pos->y/PATHFIND_CELL_SIZE);
}

inline Bool Pathfinder::validMovementPosition( Bool isCrusher, PathfindLayerEnum layer, const LocomotorSet& locomotorSet, Int x, Int y )
{
	return validMovementPosition( isCrusher, locomotorSet.getValidSurfaces(), getCell( layer, x, y ) );
}

inline Bool Pathfinder::validMovementPosition( Bool isCrusher, PathfindLayerEnum layer, const LocomotorSet& locomotorSet, const Coord3D *pos )
{

	Int x = REAL_TO_INT(pos->x/PATHFIND_CELL_SIZE);
	Int y = REAL_TO_INT(pos->y/PATHFIND_CELL_SIZE);

	return validMovementPosition( isCrusher, layer, locomotorSet, x, y );
}

inline const Coord3D *Pathfinder::getDebugPathPosition( void )
{
	return &debugPathPos;
}

inline void Pathfinder::setDebugPathPosition( const Coord3D *pos )
{
	debugPathPos = *pos;
}

inline Path *Pathfinder::getDebugPath( void )
{
	return debugPath;
}

inline void Pathfinder::addObjectToPathfindMap( class Object *obj )
{
	classifyObjectFootprint( obj, true );
}

inline void Pathfinder::removeObjectFromPathfindMap( class Object *obj )
{
	classifyObjectFootprint( obj, false );
}

inline PathfindCell *Pathfinder::getCell( PathfindLayerEnum layer, Int x, Int y )
{
	if (x >= m_extent.lo.x && x <= m_extent.hi.x &&
		y >= m_extent.lo.y && y <= m_extent.hi.y)
	{
		PathfindCell *cell = NULL;
		if (layer > LAYER_GROUND && layer <= LAYER_LAST)
		{
			cell = m_layers[layer].getCell(x, y);
			if (cell)
				return cell;
		}
		return &m_map[x][y];
	}
	else
	{
		return NULL;
	}
}

inline PathfindCell *Pathfinder::getCell( PathfindLayerEnum layer, const Coord3D *pos )
{
	ICoord2D cell;
	Bool overflow = worldToCell( pos, &cell );
	if (overflow) return NULL;
	return getCell( layer, cell.x, cell.y );
}

inline PathfindCell *Pathfinder::getClippedCell( PathfindLayerEnum layer, const Coord3D *pos)
{
	ICoord2D cell;
	worldToCell( pos, &cell );
	return getCell( layer, cell.x, cell.y );
}

inline Bool Pathfinder::worldToCell( const Coord3D *pos, ICoord2D *cell )
{
	cell->x = REAL_TO_INT_FLOOR(pos->x/PATHFIND_CELL_SIZE);
	cell->y = REAL_TO_INT_FLOOR(pos->y/PATHFIND_CELL_SIZE);
	Bool overflow = false;
	if (cell->x < m_extent.lo.x) {overflow = true; cell->x = m_extent.lo.x;}
	if (cell->y < m_extent.lo.y) {overflow = true; cell->y = m_extent.lo.y;}
	if (cell->x > m_extent.hi.x) {overflow = true; cell->x = m_extent.hi.x;}
	if (cell->y > m_extent.hi.y) {overflow = true; cell->y = m_extent.hi.y;}
	return overflow;
}

/**
 * Return true if the given object ID is registered as an obstacle in this cell
 */
inline Bool PathfindCell::isObstaclePresent( ObjectID objID ) const
{
	if (objID != INVALID_ID && (getType() == PathfindCell::CELL_OBSTACLE))
	{
		DEBUG_ASSERTCRASH(m_info, ("Should have info to be obstacle."));
		return (m_info && m_info->m_obstacleID == objID);
	}

	return false;
}


#endif // _PATHFIND_H_
