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

#include "Common/GameType.h"
#include "Common/Snapshot.h"
//#include "GameLogic/Locomotor.h"	// no, do not include this, unless you like long recompiles
#include "GameLogic/LocomotorSet.h"

#include "Pathfinder/Path.h"
#include "Pathfinder/PathfindCell.h"
#include "Pathfinder/PathfindCellInfo.h"
#include "Pathfinder/PathfindCellList.h"
#include "Pathfinder/PathfindLayer.h"
#include "Pathfinder/PathfindZoneManager.h"
#include "Pathfinder/PathNode.h"
#include "Pathfinder/ZoneBlock.h"

class Bridge;
class Object;
class Weapon;
class PathfindZoneManager;
class PathfindCell;

// How close is close enough when moving.

#define PATHFIND_CLOSE_ENOUGH 1.0f
#define PATH_MAX_PRIORITY 0x7FFFFFFF

#define INFANTRY_MOVES_THROUGH_INFANTRY

#if !RETAIL_COMPATIBLE_PATHFINDING
#undef RETAIL_COMPATIBLE_PATHFINDING_ALLOCATION
#endif

  typedef UnsignedShort zoneStorageType;


//----------------------------------------------------------------------------------------------------------

// See GameType.h for
// enum {LAYER_INVALID = 0, LAYER_GROUND = 1, LAYER_TOP=2 };

// Fits in 4 bits for now
enum {MAX_WALL_PIECES = 128};

// how close a unit has to be in z to interact with the layer.
#define LAYER_Z_CLOSE_ENOUGH_F 10.0f

#define PATHFIND_CELL_SIZE		10
#define PATHFIND_CELL_SIZE_F	10.0f

enum { PATHFIND_QUEUE_LEN=512};

struct TCheckMovementInfo;

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
		This succeeds when the destination is unreachable (like inside a building).
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
	virtual Path *findPath( Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to) override;	///< Find a short, valid path between given locations
	/** Find a short, valid path to a location NEAR the to location.
		This succeeds when the destination is unreachable (like inside a building).
		If the destination is unreachable, it will adjust the to point.  */
	virtual Path *findClosestPath( Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		Coord3D *to, Bool blocked, Real pathCostMultiplier, Bool moveAllies ) override;

	/** Find a short, valid path to a location that obj can attack victim from.  */
	virtual Path *findAttackPath( const Object *obj, const LocomotorSet& locomotorSet, const Coord3D *from,
		const Object *victim, const Coord3D* victimPos, const Weapon *weapon ) override;

	/** Find a short, valid path to a location that is away from the repulsors.  */
	virtual Path *findSafePath( const Object *obj, const LocomotorSet& locomotorSet,
		const Coord3D *from, const Coord3D* repulsorPos1, const Coord3D* repulsorPos2, Real repulsorRadius ) override;

	/** Patch to the exiting path from the current position, either because we became blocked,
  or because we had to move off the path to avoid other units. */
	virtual Path *patchPath( const Object *obj, const LocomotorSet& locomotorSet,
		Path *originalPath, Bool blocked ) override;

public:
	Pathfinder();
	~Pathfinder() ;

	void reset();														///< Reset system in preparation for new map

	// --------------- inherited from Snapshot interface --------------
	virtual void crc( Xfer *xfer ) override;
	virtual void xfer( Xfer *xfer ) override;
	virtual void loadPostProcess() override;

	Bool clientSafeQuickDoesPathExist( const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to );  ///< Can we build any path at all between the locations	(terrain & buildings check - fast)
	Bool clientSafeQuickDoesPathExistForUI( const LocomotorSet& locomotorSet, const Coord3D *from, const Coord3D *to );  ///< Can we build any path at all between the locations	(terrain only - fast)
	Bool slowDoesPathExist( Object *obj, const Coord3D *from,
		const Coord3D *to, ObjectID ignoreObject=INVALID_ID );  ///< Can we build any path at all between the locations	(terrain, buildings & units check - slower)

	Bool queueForPath(ObjectID id);	 ///< The object wants to request a pathfind, so put it on the list to process.
	void processPathfindQueue(); ///< Process some or all of the queued pathfinds.
	void forceMapRecalculation();	///< Force pathfind map recomputation. If region is given, only that area is recomputed

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

	void newMap();

	PathfindCell *getCell( PathfindLayerEnum layer, Int x, Int y );							///< Return the cell at grid coords (x,y)
	PathfindCell *getCell( PathfindLayerEnum layer, const Coord3D *pos );				///< Given a position, return associated grid cell
	PathfindCell *getClippedCell( PathfindLayerEnum layer, const Coord3D *pos );				///< Given a position, return associated grid cell
	void clip(Coord3D *from, Coord3D *to);
	Bool worldToCell( const Coord3D *pos, ICoord2D *cell );	///< Given a world position, return grid cell coordinate

	const ICoord2D *getExtent() const {return &m_extent.hi;}

	void setIgnoreObstacleID( ObjectID objID );					///< if non-zero, the pathfinder will ignore the given obstacle

	Bool validMovementPosition( Bool isCrusher, LocomotorSurfaceTypeMask acceptableSurfaces, PathfindCell *toCell, PathfindCell *fromCell = nullptr );		///< Return true if given position is a valid movement location
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
	const Coord3D *getDebugPathPosition();
	void setDebugPathPosition( const Coord3D *pos );
	Path *getDebugPath();
	void setDebugPath( Path *debugpath );

#if RETAIL_COMPATIBLE_PATHFINDING
	void forceCleanCells();
#endif
	void cleanOpenAndClosedLists();

	// Adjusts the destination to a spot near dest that is not occupied by other units.
	Bool adjustDestination(Object *obj, const LocomotorSet& locomotorSet,
		Coord3D *dest, const Coord3D *groupDest=nullptr);

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
	Real getWallHeight() {return m_wallHeight;}
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

	void classifyMap();					///< Classify all cells in grid as obstacles, etc
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
	Path *buildHierarchicalPath( const Coord3D *fromPos, PathfindCell *goalCell);	///< Work backwards from goal cell to construct final path

	void  prependCells( Path *path, const Coord3D *fromPos,
																	PathfindCell *goalCell, Bool center ); ///< Add pathfind cells to a path.

	void debugShowSearch( Bool pathFound );				///< Show all cells touched in the last search
	static LocomotorSurfaceTypeMask validLocomotorSurfacesForCellType(PathfindCell::CellType t);

	void checkChangeLayers(PathfindCell *parentCell);

	bool checkCellOutsideExtents(ICoord2D& cell);

#if defined(RTS_DEBUG)
	void doDebugIcons() ;
#endif

private:
	/// This uses WAY too much memory.  Should at least be array of pointers to cells w/ many fewer cells
	PathfindCell *m_blockOfMapCells;		///< Pathfinding map - contains iconic representation of the map
	PathfindCell **m_map;		///< Pathfinding map indexes - contains matrix indexing into the map.
	IRegion2D m_extent;														///< Grid extent limits
	IRegion2D m_logicalExtent;										///< Logical grid extent limits

	PathfindCellList m_openList;									///< Cells ready to be explored
	PathfindCellList m_closedList;								///< Cells already explored

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

#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
public:
	Bool					m_classifyFenceZeroInit;
#endif
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

inline const Coord3D *Pathfinder::getDebugPathPosition()
{
	return &debugPathPos;
}

inline void Pathfinder::setDebugPathPosition( const Coord3D *pos )
{
	debugPathPos = *pos;
}

inline Path *Pathfinder::getDebugPath()
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
		PathfindCell *cell = nullptr;
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
		return nullptr;
	}
}

inline PathfindCell *Pathfinder::getCell( PathfindLayerEnum layer, const Coord3D *pos )
{
	ICoord2D cell;
	Bool overflow = worldToCell( pos, &cell );
	if (overflow) return nullptr;
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

