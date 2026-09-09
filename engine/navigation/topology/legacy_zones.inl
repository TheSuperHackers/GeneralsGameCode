// Legacy game implementation; compiled within engine.navigation.pathfinder.
inline Bool typesMatch(const PathfindCell &targetCell, const PathfindCell &sourceCell) {
	PathfindCell::CellType targetType = targetCell.getType();
	PathfindCell::CellType srcType = sourceCell.getType();
	if (targetType == srcType) return true;

	return false;
}

inline Bool waterGround(const PathfindCell &targetCell, const PathfindCell &sourceCell) {
	PathfindCell::CellType targetType = targetCell.getType();
	PathfindCell::CellType srcType = sourceCell.getType();
	if ( (targetType==PathfindCell::CELL_CLEAR &&
		(srcType&PathfindCell::CELL_WATER ))) {
			return true;
	}
	if ( (srcType==PathfindCell::CELL_CLEAR &&
		(targetType&PathfindCell::CELL_WATER ))) {
			return true;
	}

	return false;
}

inline Bool groundRubble(const PathfindCell &targetCell, const PathfindCell &sourceCell) {
	PathfindCell::CellType targetType = targetCell.getType();
	PathfindCell::CellType srcType = sourceCell.getType();
	if ( (targetType==PathfindCell::CELL_CLEAR &&
		(srcType==PathfindCell::CELL_RUBBLE ))) {
			return true;
	}
	if ( (srcType==PathfindCell::CELL_CLEAR &&
		(targetType==PathfindCell::CELL_RUBBLE ))) {
			return true;
	}

	return false;
}

inline Bool terrain(const PathfindCell &targetCell, const PathfindCell &sourceCell) {
	Int targetType = targetCell.getType();
	Int srcType = sourceCell.getType();
	if (targetType == PathfindCell::CELL_OBSTACLE) targetType = PathfindCell::CELL_CLEAR;
	if (srcType == PathfindCell::CELL_OBSTACLE) srcType = PathfindCell::CELL_CLEAR;
	if (targetType==srcType) {
		return true;
	}
	return false;
}

inline Bool crusherGround(const PathfindCell &targetCell, const PathfindCell &sourceCell) {
	Int targetType = targetCell.getType();
	Int srcType = sourceCell.getType();
	if (targetType==PathfindCell::CELL_OBSTACLE) {
		if (targetCell.isObstacleFence()) {
			if (srcType == PathfindCell::CELL_CLEAR) {
				return true;
			}
		}
	}
	if (srcType==PathfindCell::CELL_OBSTACLE) {
		if (sourceCell.isObstacleFence()) {
			if (targetType == PathfindCell::CELL_CLEAR) {
				return true;
			}
		}
	}
	return false;
}

inline Bool groundCliff(const PathfindCell &targetCell, const PathfindCell &sourceCell) {
	PathfindCell::CellType targetType = targetCell.getType();
	PathfindCell::CellType srcType = sourceCell.getType();

	if ( (targetType==PathfindCell::CELL_CLIFF ) &&
			 (srcType==PathfindCell::CELL_CLEAR) ) {
			return true;
	}
	if ( (targetType==PathfindCell::CELL_CLEAR ) &&
			 (srcType==PathfindCell::CELL_CLIFF) ) {
			return true;
	}
	return false;
}

static void __fastcall resolveBlockZones(Int srcZone, Int targetZone, zoneStorageType *zoneEquivalency, Int sizeOfZE)
{
	Int i;
	// We have two zones being combined now. Keep the lower zone.
	DEBUG_ASSERTCRASH(srcZone!=0 && targetZone!=0,  ("Bad resolve zones	."));
	if (targetZone<srcZone) {
		for (i=0; i<sizeOfZE; i++) {
			if (zoneEquivalency[i] == srcZone) {
				zoneEquivalency[i] = targetZone;
			}
		}
	} else {
		for (i=0; i<sizeOfZE; i++) {
			if (zoneEquivalency[i] == targetZone) {
				zoneEquivalency[i] = srcZone;
			}
		}
	}
}

static void __fastcall resolveZones(Int srcZone, Int targetZone, zoneStorageType *zoneEquivalency, Int sizeOfZE)
{
	Int i;
	// We have two zones being combined now. Keep the lower zone.
	DEBUG_ASSERTCRASH(srcZone!=0 && targetZone!=0,  ("Bad resolve zones	."));
	DEBUG_ASSERTCRASH(srcZone<sizeOfZE && targetZone<sizeOfZE,  ("Bad resolve zones	."));
	srcZone = zoneEquivalency[srcZone];
	targetZone = zoneEquivalency[targetZone];
	DEBUG_ASSERTCRASH(srcZone<sizeOfZE && targetZone<sizeOfZE,  ("Bad resolve zones	."));
	zoneStorageType finalZone;
	if (targetZone<srcZone) {
		finalZone = zoneEquivalency[targetZone];
	} else {
		finalZone = zoneEquivalency[srcZone];
	}
	DEBUG_ASSERTCRASH(finalZone<sizeOfZE ,  ("Bad resolve zones	."));
	for (i=0; i<sizeOfZE; i++) {
		zoneStorageType ze = zoneEquivalency[i];
		if (ze == targetZone || ze == srcZone) {
			zoneEquivalency[i] = finalZone;
		}
	}
}

static void flattenZones(zoneStorageType *zoneArray, zoneStorageType *zoneHierarchical, Int sizeOfZones)
{
	Int i;
	for (i=0; i<sizeOfZones; i++) {
		Int zone1 = zoneArray[i];
		Int zone2 = zoneHierarchical[zone1];
		zone1 = zoneArray[zone2];
		zone2 = zoneHierarchical[zone1];
		zoneArray[i] = zone2;
	}
#if 1

	for (i=0; i<sizeOfZones; i++) {
		Int zone1 = zoneArray[i];
		Int zone2 = zoneHierarchical[i];
		if (zone1!=zone2) {
			resolveZones(zone1, zone2, zoneArray, sizeOfZones);
		}
	}
#endif
}

inline void applyZone(PathfindCell &targetCell, const PathfindCell &sourceCell, zoneStorageType *zoneEquivalency, Int sizeOfZE)
{
	DEBUG_ASSERTCRASH(sourceCell.getZone()!=0, ("Unset source zone."));
	Int srcZone = zoneEquivalency[sourceCell.getZone()];
	Int targetZone = zoneEquivalency[targetCell.getZone()];

	if (targetZone == 0) {
		targetCell.setZone(srcZone);
		return;
	}
	if (targetZone == srcZone) {
		return; // already match.
	}
	resolveZones(srcZone, targetZone, zoneEquivalency, sizeOfZE);

}

inline void applyBlockZone(PathfindCell &targetCell, const PathfindCell &sourceCell,
													 zoneStorageType *zoneEquivalency, Int firstZone, Int sizeOfZE)
{
	DEBUG_ASSERTCRASH(sourceCell.getZone()>=firstZone && sourceCell.getZone()<firstZone+sizeOfZE, ("Memory overrun - FATAL ERROR."));
	Int srcZone = zoneEquivalency[sourceCell.getZone()-firstZone];
	DEBUG_ASSERTCRASH(targetCell.getZone()>=firstZone && sourceCell.getZone()<firstZone+sizeOfZE, ("Memory overrun - FATAL ERROR."));
	Int targetZone = zoneEquivalency[targetCell.getZone()-firstZone];
	if (targetZone == srcZone) {
		return; // already match.
	}
	resolveBlockZones(srcZone, targetZone, zoneEquivalency, sizeOfZE);

}

//------------------------  ZoneBlock  -------------------------------
ZoneBlock::ZoneBlock() : m_firstZone(0),
m_numZones(0),
m_groundCliffZones(nullptr),
m_groundWaterZones(nullptr),
m_groundRubbleZones(nullptr),
m_crusherZones(nullptr),
m_zonesAllocated(0),
m_interactsWithBridge(FALSE)
{
	m_cellOrigin.x = 0;
	m_cellOrigin.y = 0;
	m_firstZone = 0;
	m_markedPassable = TRUE;
}

ZoneBlock::~ZoneBlock()
{
	freeZones();
}

void ZoneBlock::freeZones()
{
	delete [] m_groundCliffZones;
	m_groundCliffZones = nullptr;

	delete [] m_groundWaterZones;
	m_groundWaterZones = nullptr;

	delete [] m_groundRubbleZones;
	m_groundRubbleZones = nullptr;

	delete [] m_crusherZones;
	m_crusherZones = nullptr;
}

/* Allocate zone equivalency arrays large enough to hold required entries.  If the arrays are already
large enough, reuse.  Then calculate terrain equivalencies. */
void ZoneBlock::blockCalculateZones(PathfindCell **map, PathfindLayer layers[], const IRegion2D &bounds)
{
	Int i, j;
	m_cellOrigin = bounds.lo;
	UnsignedInt minZone = map[bounds.lo.x][bounds.lo.y].getZone();
	UnsignedInt maxZone = minZone;

	for( j=bounds.lo.y; j<=bounds.hi.y; j++ )	{
		for( i=bounds.lo.x; i<=bounds.hi.x; i++ )	{
			PathfindCell *cell = &map[i][j];
			zoneStorageType zone = cell->getZone();
			if (minZone>zone) minZone=zone;
			if (maxZone<zone) maxZone=zone;
		}
	}
	m_firstZone = minZone;
	m_numZones = 1 + maxZone - minZone;

	allocateZones();

	if (m_numZones==1) return; // all zones are equivalent.

	// Determine water/ground equivalent zones, and ground/cliff equivalent zones.
	for (i=0; i<m_zonesAllocated; i++) {
		m_groundCliffZones[i] = i+m_firstZone;
		m_groundWaterZones[i] = i+m_firstZone;
		m_groundRubbleZones[i] = i+m_firstZone;
		m_crusherZones[i] = i+m_firstZone;
	}

	for( j=bounds.lo.y; j<=bounds.hi.y; j++ )	{
		for( i=bounds.lo.x; i<=bounds.hi.x; i++ )	{
			if (i>bounds.lo.x && map[i][j].getZone()!=map[i-1][j].getZone()) {

				if (waterGround(map[i][j], map[i-1][j])) {
					applyBlockZone(map[i][j], map[i-1][j], m_groundWaterZones, m_firstZone, m_numZones);
				}
				if (groundRubble(map[i][j], map[i-1][j])) {
					applyBlockZone(map[i][j], map[i-1][j], m_groundRubbleZones, m_firstZone, m_numZones);
				}
				if (groundCliff(map[i][j], map[i-1][j])) {
					applyBlockZone(map[i][j], map[i-1][j], m_groundCliffZones, m_firstZone, m_numZones);
				}
				if (crusherGround(map[i][j], map[i-1][j])) {
					applyBlockZone(map[i][j], map[i-1][j], m_crusherZones, m_firstZone, m_numZones);
				}
			}
			if (j>bounds.lo.y && map[i][j].getZone()!=map[i][j-1].getZone()) {
				if (waterGround(map[i][j],map[i][j-1])) {
					applyBlockZone(map[i][j], map[i][j-1], m_groundWaterZones, m_firstZone, m_numZones);
				}
				if (groundRubble(map[i][j], map[i][j-1])) {
					applyBlockZone(map[i][j], map[i][j-1], m_groundRubbleZones, m_firstZone, m_numZones);
				}
				if (groundCliff(map[i][j],map[i][j-1])) {
					applyBlockZone(map[i][j], map[i][j-1], m_groundCliffZones, m_firstZone, m_numZones);
				}
				if (crusherGround(map[i][j], map[i][j-1])) {
					applyBlockZone(map[i][j], map[i][j-1], m_crusherZones, m_firstZone, m_numZones);
				}
			}
			DEBUG_ASSERTCRASH(map[i][j].getZone() != 0, ("Cleared the zone."));
		}
	}

}

//
// Return the zone at this location.
//
zoneStorageType ZoneBlock::getEffectiveZone( LocomotorSurfaceTypeMask acceptableSurfaces,
																					 Bool crusher, zoneStorageType zone) const
{
#if !(RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING)
	if (zone==PathfindZoneManager::UNINITIALIZED_ZONE) {
		return zone;
	}
#endif

	if (acceptableSurfaces&LOCOMOTORSURFACE_AIR) return 1; // air is all zone 1.

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_WATER) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_CLIFF)) {
		// Locomotors can go on ground, water & cliff, so all is zone 1.
		return 1;
	}
	if (m_numZones<2) {
		return m_firstZone; // if we only got 1 zone, it's all the same zone.
	}
	DEBUG_ASSERTCRASH(zone >=m_firstZone && zone < m_firstZone+m_numZones, ("Invalid range."));
	if (zone<m_firstZone || zone >= m_firstZone+m_numZones) {
		return m_firstZone;
	}
	zone -= m_firstZone;
	if (crusher) {
		zone = m_crusherZones[zone];
		DEBUG_ASSERTCRASH(zone >=m_firstZone && zone < m_firstZone+m_numZones, ("Invalid range."));
		zone -= m_firstZone;
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_CLIFF)) {
		// Locomotors can go on ground & cliff, so use the ground cliff combiner.
		zone = m_groundCliffZones[zone];
		DEBUG_ASSERTCRASH(zone >=m_firstZone && zone < m_firstZone+m_numZones, ("Invalid range."));
		return zone;
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_WATER)) {
		// Locomotors can go on ground & water, so use the ground water combiner.
		zone = m_groundWaterZones[zone];
		DEBUG_ASSERTCRASH(zone >=m_firstZone && zone < m_firstZone+m_numZones, ("Invalid range."));
		return zone;
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_RUBBLE)) {
		// Locomotors can go on ground & rubble, so use the ground rubble combiner.
		zone = m_groundRubbleZones[zone];
		return zone;
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_CLIFF) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_WATER)) {
		// Locomotors can go on ground & cliff, so use the ground cliff combiner.
		DEBUG_CRASH(("Cliff water only locomotor sets not supported yet."));
	}

	return zone+m_firstZone;
}


/* Allocate zone equivalency arrays large enough to hold m_maxZone entries.  If the arrays are already
large enough, just return. */
void ZoneBlock::allocateZones()
{
	if (m_zonesAllocated>m_numZones && m_groundCliffZones!=nullptr) {
		return;
	}
	freeZones();

	if (m_numZones==1) {
		return; // we don't need any zone equivalency tables.
	}

	if (m_zonesAllocated == 0) {
		m_zonesAllocated = 4;
	}
	while (m_zonesAllocated <= m_numZones) {
		m_zonesAllocated *= 2;
	}
	// pool[]ify
	m_groundCliffZones = MSGNEW("PathfindZoneInfo") zoneStorageType [m_zonesAllocated];
	m_groundWaterZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
	m_groundRubbleZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
	m_crusherZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
}


//------------------------  PathfindZoneManager  -------------------------------
PathfindZoneManager::PathfindZoneManager() : m_maxZone(0),
m_nextFrameToCalculateZones(0),
m_groundCliffZones(nullptr),
m_groundWaterZones(nullptr),
m_groundRubbleZones(nullptr),
m_terrainZones(nullptr),
m_crusherZones(nullptr),
m_hierarchicalZones(nullptr),
m_blockOfZoneBlocks(nullptr),
m_zoneBlocks(nullptr),
m_zonesAllocated(0)
{
	m_zoneBlockExtent.x = 0;
	m_zoneBlockExtent.y = 0;
}

PathfindZoneManager::~PathfindZoneManager()
{
	freeZones();
	freeBlocks();
}

void PathfindZoneManager::freeZones()
{
	delete [] m_groundCliffZones;
	m_groundCliffZones = nullptr;

	delete [] m_groundWaterZones;
	m_groundWaterZones = nullptr;

	delete [] m_groundRubbleZones;
	m_groundRubbleZones = nullptr;

	delete [] m_terrainZones;
	m_terrainZones = nullptr;

	delete [] m_crusherZones;
	m_crusherZones = nullptr;

	delete [] m_hierarchicalZones;
	m_hierarchicalZones = nullptr;

	m_zonesAllocated = 0;
}

void PathfindZoneManager::freeBlocks()
{
	delete [] m_blockOfZoneBlocks;
	m_blockOfZoneBlocks = nullptr;

	delete [] m_zoneBlocks;
	m_zoneBlocks = nullptr;

	m_zoneBlockExtent.x = 0;
	m_zoneBlockExtent.y = 0;
}

/* Allocate zone equivalency arrays large enough to hold m_maxZone entries.  If the arrays are already
large enough, just return. */
void PathfindZoneManager::allocateZones()
{
	if (m_zonesAllocated>m_maxZone && m_groundCliffZones!=nullptr) {
		return;
	}
	freeZones();

	if (m_zonesAllocated == 0) {
		m_zonesAllocated = INITIAL_ZONES;
	}
	while (m_zonesAllocated <= m_maxZone) {
		m_zonesAllocated *= 2;
	}
	DEBUG_LOG(("Allocating zone tables of size %d", m_zonesAllocated));
	// pool[]ify
	m_groundCliffZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
	m_groundWaterZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
	m_groundRubbleZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
	m_terrainZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
	m_crusherZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
	m_hierarchicalZones = MSGNEW("PathfindZoneInfo") zoneStorageType[m_zonesAllocated];
}

/* Allocate zone blocks for hierarchical pathfinding.   */
void PathfindZoneManager::allocateBlocks(const IRegion2D &globalBounds)
{
	freeBlocks();

	m_zoneBlockExtent.x = (globalBounds.hi.x-globalBounds.lo.x+1+ZONE_BLOCK_SIZE-1)/ZONE_BLOCK_SIZE;
	m_zoneBlockExtent.y = (globalBounds.hi.y-globalBounds.lo.y+1+ZONE_BLOCK_SIZE-1)/ZONE_BLOCK_SIZE;

	m_blockOfZoneBlocks = MSGNEW("PathfindZoneBlocks") ZoneBlock[(m_zoneBlockExtent.x)*(m_zoneBlockExtent.y)];
	m_zoneBlocks = MSGNEW("PathfindZoneBlocks") ZoneBlockP[m_zoneBlockExtent.x];
	Int i;
	for (i=0; i<m_zoneBlockExtent.x; i++) {
		m_zoneBlocks[i] = &m_blockOfZoneBlocks[i*(m_zoneBlockExtent.y)];
	}
}

void PathfindZoneManager::reset()  ///< Called when the map is reset.
{
	freeZones();
	freeBlocks();
}


void PathfindZoneManager::markZonesDirty()  ///< Called when the zones need to be recalculated.
{
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
	m_nextFrameToCalculateZones = TheGameLogic->getFrame();
#else
	if (TheGameLogic->getFrame()<2) {
		m_nextFrameToCalculateZones = 2;
		return;
	}
	m_nextFrameToCalculateZones = MIN( m_nextFrameToCalculateZones, TheGameLogic->getFrame() + ZONE_UPDATE_FREQUENCY );
#endif
}

/**
 * Calculate zones.  A zone is an area of the same terrain - clear, water or cliff.
 * The utility of zones is that if current location and destination are in the same zone,
 * you can successfully pathfind.
 * If you are a multiple terrain vehicle, like amphibious transport, the lookup is a little more
 * complicated.
 */
void PathfindZoneManager::calculateZones( PathfindCell **map, PathfindLayer layers[], const IRegion2D &globalBounds )
{
#ifdef DEBUG_QPF
#if defined(DEBUG_LOGGING)
	__int64 startTime64;
	static double timeToUpdate = 0.0f;
	static double averageTimeToUpdate = 0.0f;
	static Int updateSamples = 0;
	__int64 endTime64,freq64;
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq64);
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime64);
#endif
#endif

	m_maxZone = 1;	// we start using zone 0 as a flag.
	const Int maxZones=24000;
	zoneStorageType zoneEquivalency[maxZones];
	Int i, j;
	for (i=0; i<maxZones; i++) {
		zoneEquivalency[i] = i;
	}
	for (i=0; i<=LAYER_LAST; i++) {
		layers[i].setZone(0);
	}

	Int xCount = (globalBounds.hi.x-globalBounds.lo.x+1+ZONE_BLOCK_SIZE-1)/ZONE_BLOCK_SIZE;
	Int yCount = (globalBounds.hi.y-globalBounds.lo.y+1+ZONE_BLOCK_SIZE-1)/ZONE_BLOCK_SIZE;

	Int xBlock, yBlock;
	for (xBlock = 0; xBlock<xCount; xBlock++) {
		for (yBlock=0; yBlock<yCount; yBlock++) {
			IRegion2D bounds;
			bounds.lo.x = globalBounds.lo.x + xBlock*ZONE_BLOCK_SIZE;
			bounds.lo.y = globalBounds.lo.y + yBlock*ZONE_BLOCK_SIZE;
			bounds.hi.x = bounds.lo.x + ZONE_BLOCK_SIZE - 1; // bounds are inclusive.
			bounds.hi.y = bounds.lo.y + ZONE_BLOCK_SIZE - 1; // bounds are inclusive.
			if (bounds.hi.x > globalBounds.hi.x) {
				bounds.hi.x = globalBounds.hi.x;
			}
			if (bounds.hi.y > globalBounds.hi.y) {
				bounds.hi.y = globalBounds.hi.y;
			}
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
			if (bounds.lo.x>bounds.hi.x || bounds.lo.y>bounds.hi.y) {
				DEBUG_CRASH(("Incorrect bounds calculation. Logic error, fix me. jba."));
				continue;
			}
#endif
			m_zoneBlocks[xBlock][yBlock].setInteractsWithBridge(false);
			for( j=bounds.lo.y; j<=bounds.hi.y; j++ )	{
				for( i=bounds.lo.x; i<=bounds.hi.x; i++ )	{
					PathfindCell *cell = &map[i][j];
					cell->setZone(0);

					if (i>bounds.lo.x) {
						if (map[i][j].getType() == map[i-1][j].getType()) {
							applyZone(map[i][j], map[i-1][j], zoneEquivalency, m_maxZone);
						}
					}
					if (j>bounds.lo.y) {
						if (map[i][j].getType() == map[i][j-1].getType()) {
							applyZone(map[i][j], map[i][j-1], zoneEquivalency, m_maxZone);
						}
					}
					if (cell->getZone()==0) {
						cell->setZone(m_maxZone);
						m_maxZone++;
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
						if (m_maxZone>= maxZones) {
							DEBUG_CRASH(("Ran out of pathfind zones.  SERIOUS ERROR! jba."));
							break;
						}
#endif
					}
					if (cell->getConnectLayer() > LAYER_GROUND) {
 						m_zoneBlocks[xBlock][yBlock].setInteractsWithBridge(true);
					}

				}
			}
 		}
	}

	Int totalZones = m_maxZone;

	// Collapse the zones into a 1,2,3... sequence, removing collapsed zones.
	m_maxZone = 1;
	Int collapsedZones[maxZones];
	collapsedZones[0] = 0;
	for (i=1; i<totalZones; i++) {
		Int zone = zoneEquivalency[i];
		if (zone == i) {
			collapsedZones[i] = m_maxZone;
			++m_maxZone;
		} else {
			collapsedZones[i] = collapsedZones[zone];
		}
	}

	// Now map the zones in the map back into the collapsed zones.
	for( j=globalBounds.lo.y; j<=globalBounds.hi.y; j++ ) {
		for( i=globalBounds.lo.x; i<=globalBounds.hi.x; i++ ) {
			PathfindCell &cell = map[i][j];
			cell.setZone(collapsedZones[cell.getZone()]);
		}
	}

	for (i=0; i<=LAYER_LAST; i++) {
		PathfindLayer &r_thisLayer = layers[i];

		Int zone = collapsedZones[r_thisLayer.getZone()];
		if (zone == 0) {
			zone = m_maxZone;
			m_maxZone++;
		}

		r_thisLayer.setZone( zone );
		r_thisLayer.applyZone();

		if (!r_thisLayer.isUnused() && !r_thisLayer.isDestroyed()) {
			ICoord2D ndx;
			r_thisLayer.getStartCellIndex(&ndx);
			setBridge(ndx.x, ndx.y, true);
			r_thisLayer.getEndCellIndex(&ndx);
			setBridge(ndx.x, ndx.y, true);
		}
	}

	allocateZones();

	for (xBlock=0; xBlock<xCount; xBlock++) {
		for (yBlock=0; yBlock<yCount; yBlock++) {
			IRegion2D bounds;
			bounds.lo.x = globalBounds.lo.x + xBlock*ZONE_BLOCK_SIZE;
			bounds.lo.y = globalBounds.lo.y + yBlock*ZONE_BLOCK_SIZE;
			bounds.hi.x = bounds.lo.x + ZONE_BLOCK_SIZE - 1; // bounds are inclusive.
			bounds.hi.y = bounds.lo.y + ZONE_BLOCK_SIZE - 1; // bounds are inclusive.

			if (bounds.hi.x > globalBounds.hi.x)
				bounds.hi.x = globalBounds.hi.x;

			if (bounds.hi.y > globalBounds.hi.y)
				bounds.hi.y = globalBounds.hi.y;
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
			if (bounds.lo.x>bounds.hi.x || bounds.lo.y>bounds.hi.y) {
				DEBUG_CRASH(("Incorrect bounds calculation. Logic error, fix me. jba."));
				continue;
			}
#endif
			m_zoneBlocks[xBlock][yBlock].blockCalculateZones(map, layers, bounds);
		}
	}

	// Determine water/ground equivalent zones, and ground/cliff equivalent zones.
	for (i=0; i<m_zonesAllocated; i++) {
		m_groundCliffZones[i] = i;
		m_groundWaterZones[i] = i;
		m_groundRubbleZones[i] = i;
		m_terrainZones[i] = i;
		m_crusherZones[i] = i;
		m_hierarchicalZones[i] = i;
	}

	for( j=globalBounds.lo.y; j<=globalBounds.hi.y; j++ ) {
		for( i=globalBounds.lo.x; i<=globalBounds.hi.x; i++ ) {
			PathfindCell &r_thisCell = map[i][j];

			if ( (r_thisCell.getConnectLayer() > LAYER_GROUND) &&
				(r_thisCell.getType() == PathfindCell::CELL_CLEAR) ) {
				PathfindLayer *layer = layers + r_thisCell.getConnectLayer();
				resolveZones(r_thisCell.getZone(), layer->getZone(), m_hierarchicalZones, m_maxZone);
			}

			if ( i > globalBounds.lo.x && r_thisCell.getZone() != map[i-1][j].getZone() ) {
				const PathfindCell &r_leftCell = map[i-1][j];

#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
				if (r_thisCell.getType() == r_leftCell.getType()) {
					applyZone(r_thisCell, r_leftCell, m_hierarchicalZones, m_maxZone);
				}
				if (waterGround(r_thisCell, r_leftCell)) {
					applyZone(r_thisCell, r_leftCell, m_groundWaterZones, m_maxZone);
				}
				if (groundRubble(r_thisCell, r_leftCell)) {
					applyZone(r_thisCell, r_leftCell, m_groundRubbleZones, m_maxZone);
				}
				if (groundCliff(r_thisCell, r_leftCell)) {
					applyZone(r_thisCell, r_leftCell, m_groundCliffZones, m_maxZone);
				}
				if (terrain(r_thisCell, r_leftCell)) {
					applyZone(r_thisCell, r_leftCell, m_terrainZones, m_maxZone);
				}
				if (crusherGround(r_thisCell, r_leftCell)) {
					applyZone(r_thisCell, r_leftCell, m_crusherZones, m_maxZone);
				}
#else
				//if this is true, skip all the ones below
				if (r_thisCell.getType() == r_leftCell.getType())
					applyZone(r_thisCell, r_leftCell, m_hierarchicalZones, m_maxZone);
				else {
					Bool notTerrainOrCrusher = TRUE; // if this is false, skip the if-else-ladder below

					if (terrain(r_thisCell, r_leftCell)) {
						applyZone(r_thisCell, r_leftCell, m_terrainZones, m_maxZone);
						notTerrainOrCrusher = FALSE;
					}

					if (crusherGround(r_thisCell, r_leftCell)) {
						applyZone(r_thisCell, r_leftCell, m_crusherZones, m_maxZone);
						notTerrainOrCrusher = FALSE;
					}

					if ( notTerrainOrCrusher ) {
						if (waterGround(r_thisCell, r_leftCell))
							applyZone(r_thisCell, r_leftCell, m_groundWaterZones, m_maxZone);
						else if (groundRubble(r_thisCell, r_leftCell))
							applyZone(r_thisCell, r_leftCell, m_groundRubbleZones, m_maxZone);
						else if (groundCliff(r_thisCell, r_leftCell))
							applyZone(r_thisCell, r_leftCell, m_groundCliffZones, m_maxZone);
					}

				}
#endif

			}

			if (j>globalBounds.lo.y && r_thisCell.getZone()!=map[i][j-1].getZone()) {
				const PathfindCell &r_topCell = map[i][j-1];

#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
				if (r_thisCell.getType() == r_topCell.getType()) {
					applyZone(r_thisCell, r_topCell, m_hierarchicalZones, m_maxZone);
				}
				if (waterGround(r_thisCell, r_topCell)) {
					applyZone(r_thisCell, r_topCell, m_groundWaterZones, m_maxZone);
				}
				if (groundRubble(r_thisCell, r_topCell)) {
					applyZone(r_thisCell, r_topCell, m_groundRubbleZones, m_maxZone);
				}
				if (groundCliff(r_thisCell, r_topCell)) {
					applyZone(r_thisCell, r_topCell, m_groundCliffZones, m_maxZone);
				}
				if (terrain(r_thisCell, r_topCell)) {
					applyZone(r_thisCell, r_topCell, m_terrainZones, m_maxZone);
				}
				if (crusherGround(r_thisCell, r_topCell)) {
					applyZone(r_thisCell, r_topCell, m_crusherZones, m_maxZone);
				}
#else
				//if this is true, skip all the ones below
				if (r_thisCell.getType() == r_topCell.getType())
					applyZone(r_thisCell, r_topCell, m_hierarchicalZones, m_maxZone);
				else {
					Bool notTerrainOrCrusher = TRUE; // if this is false, skip the if-else-ladder below

					if (terrain(r_thisCell, r_topCell)) {
						applyZone(r_thisCell, r_topCell, m_terrainZones, m_maxZone);
						notTerrainOrCrusher = FALSE;
					}

					if (crusherGround(r_thisCell, r_topCell)) {
						applyZone(r_thisCell, r_topCell, m_crusherZones, m_maxZone);
						notTerrainOrCrusher = FALSE;
					}

					if (notTerrainOrCrusher) {
						if (waterGround(r_thisCell, r_topCell))
							applyZone(r_thisCell, r_topCell, m_groundWaterZones, m_maxZone);
						else if (groundRubble(r_thisCell, r_topCell))
							applyZone(r_thisCell, r_topCell, m_groundRubbleZones, m_maxZone);
						else if (groundCliff(r_thisCell, r_topCell))
							applyZone(r_thisCell, r_topCell, m_groundCliffZones, m_maxZone);
					}

				}
#endif

			}

		}
	}

	//FLATTEN HIERARCHICAL ZONES
	for (i=1; i<m_maxZone; i++) {
		Int zone = m_hierarchicalZones[i];
		m_hierarchicalZones[i] = m_hierarchicalZones[zone];
	}

	//THIS BLOCK IS 20%
	flattenZones(m_groundCliffZones, m_hierarchicalZones, m_maxZone);
	flattenZones(m_groundWaterZones, m_hierarchicalZones, m_maxZone);
	flattenZones(m_groundRubbleZones, m_hierarchicalZones, m_maxZone);
	flattenZones(m_terrainZones, m_hierarchicalZones, m_maxZone);
	flattenZones(m_crusherZones, m_hierarchicalZones, m_maxZone);

#ifdef DEBUG_QPF
#if defined(DEBUG_LOGGING)
	QueryPerformanceCounter((LARGE_INTEGER *)&endTime64);
	timeToUpdate = ((double)(endTime64-startTime64) / (double)(freq64));

	if ( updateSamples < 400 ) {
		averageTimeToUpdate = ((averageTimeToUpdate * updateSamples) + timeToUpdate) / (updateSamples + 1.0f);
		updateSamples++;
		DEBUG_LOG(("computing...: %f", averageTimeToUpdate));
	}
	else if ( updateSamples == 400 ) {
		DEBUG_LOG((" =============DONE============= Average time to calculate zones: %f", averageTimeToUpdate));
		DEBUG_LOG(("                                           Percent of baseline : %f", averageTimeToUpdate/0.003335f));
		updateSamples = 777;
	}

#endif
#endif
#if defined(RTS_DEBUG)
	if (TheGlobalData->m_debugAI == AI_DEBUG_ZONES)
	{
		extern void addIcon(const Coord3D *pos, Real width, Int numFramesDuration, RGBColor color);
		RGBColor color;
		memset(&color, 0, sizeof(Color));
		addIcon(nullptr, 0, 0, color);
		for( j=0; j<globalBounds.hi.y; j++ )	{
			for( i=0; i<globalBounds.hi.x; i++ )	{
				Int zone = map[i][j].getZone();
				zone = m_hierarchicalZones[zone];

				color.blue = (zone%3) * 0.5f;
				zone = zone/3;
				color.green = (zone%3) * 0.5f;
				zone = zone/3;
				color.red = (zone%3) * 0.5;
				Coord3D pos;
				pos.x = ((Real)i + 0.5f) * PATHFIND_CELL_SIZE_F;
				pos.y = ((Real)j + 0.5f) * PATHFIND_CELL_SIZE_F;
				pos.z = TheTerrainLogic->getLayerHeight( pos.x, pos.y, map[i][j].getLayer() ) + 0.5f;
				addIcon(&pos, PATHFIND_CELL_SIZE_F*0.8f, 500, color);
			}
		}
	}
#endif
	m_nextFrameToCalculateZones = 0xffffffff;
}

/**
 * Update zones where a structure has been added or removed.
 * This can be done by just updating the equivalency arrays, without rezoning the map..
 */
void PathfindZoneManager::updateZonesForModify(PathfindCell **map, PathfindLayer layers[], const IRegion2D &structureBounds, const IRegion2D &globalBounds )
{

#ifdef DEBUG_QPF
#if defined(DEBUG_LOGGING)
	__int64 startTime64;
	double timeToUpdate=0.0f;
	__int64 endTime64,freq64;
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq64);
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime64);
#endif
#endif
	IRegion2D bounds = structureBounds;
	bounds.hi.x++;
	bounds.hi.y++;
	if (bounds.hi.x > globalBounds.hi.x) {
		bounds.hi.x = globalBounds.hi.x;
	}
	if (bounds.hi.y > globalBounds.hi.y) {
		bounds.hi.y = globalBounds.hi.y;
	}

	Int xBlock, yBlock;
	for (xBlock = 0; xBlock<m_zoneBlockExtent.x; xBlock++) {
		for (yBlock=0; yBlock<m_zoneBlockExtent.y; yBlock++) {
			IRegion2D blockBounds;
			blockBounds.lo.x = globalBounds.lo.x + xBlock*ZONE_BLOCK_SIZE;
			blockBounds.lo.y = globalBounds.lo.y + yBlock*ZONE_BLOCK_SIZE;
			blockBounds.hi.x = blockBounds.lo.x + ZONE_BLOCK_SIZE - 1; // blockBounds are inclusive.
			blockBounds.hi.y = blockBounds.lo.y + ZONE_BLOCK_SIZE - 1; // blockBounds are inclusive.
			if (blockBounds.hi.x > bounds.hi.x) {
				blockBounds.hi.x = bounds.hi.x;
			}
			if (blockBounds.hi.y > bounds.hi.y) {
				blockBounds.hi.y = bounds.hi.y;
			}
			if (blockBounds.lo.x < bounds.lo.x) {
				blockBounds.lo.x = bounds.lo.x;
			}
			if (blockBounds.lo.y < bounds.lo.y) {
				blockBounds.lo.y = bounds.lo.y;
			}
			if (blockBounds.lo.x>blockBounds.hi.x || blockBounds.lo.y>blockBounds.hi.y) {
				continue;
			}
			m_zoneBlocks[xBlock][yBlock].setInteractsWithBridge(false);
			Int i, j;
			for( j=blockBounds.lo.y; j<=blockBounds.hi.y; j++ )	{
				for( i=blockBounds.lo.x; i<=blockBounds.hi.x; i++ )	{
					PathfindCell *cell = &map[i][j];
					if (cell->getZone()!=UNINITIALIZED_ZONE) continue;

					if (i>blockBounds.lo.x) {
						if (map[i][j].getType() == map[i-1][j].getType()) {
							cell->setZone(map[i-1][j].getZone());
							if (cell->getZone()!=UNINITIALIZED_ZONE) continue;
						}
					}
					if (j>blockBounds.lo.y) {
						if (cell->getType() == map[i][j-1].getType()) {
							cell->setZone(map[i][j-1].getZone());
							if (cell->getZone()!=UNINITIALIZED_ZONE) continue;
						}
						if (i<blockBounds.hi.x) {
							if (typesMatch(*cell, map[i+1][j-1]) &&
									typesMatch(*cell, map[i+1][j])) {
								cell->setZone(map[i+1][j-1].getZone());
								if (cell->getZone()!=UNINITIALIZED_ZONE) continue;
							}
						}
					}
				}
			}
			for( j=blockBounds.hi.y; j>=blockBounds.lo.y; j-- )	{
				for( i=blockBounds.hi.x; i>=blockBounds.lo.x; i-- )	{
					PathfindCell *cell = &map[i][j];
					if (cell->getZone()!=UNINITIALIZED_ZONE) continue;
					if (i<blockBounds.hi.x) {
						if (map[i][j].getType() == map[i+1][j].getType()) {
							cell->setZone(map[i+1][j].getZone());
							if (cell->getZone()!=UNINITIALIZED_ZONE) continue;
						}
					}
					if (j<blockBounds.hi.y) {
						if (cell->getType() == map[i][j+1].getType()) {
							cell->setZone(map[i][j+1].getZone());
							if (cell->getZone()!=UNINITIALIZED_ZONE) continue;
						}
						if (i<blockBounds.hi.x) {
							if (typesMatch(*cell, map[i+1][j+1]) &&
									typesMatch(*cell, map[i+1][j])) {
								cell->setZone(map[i+1][j+1].getZone());
								if (cell->getZone()!=UNINITIALIZED_ZONE) continue;
							}
						}
					}
				}
			}
		}
	}
#ifdef DEBUG_QPF
#if defined(DEBUG_LOGGING)
	QueryPerformanceCounter((LARGE_INTEGER *)&endTime64);
	timeToUpdate = ((double)(endTime64-startTime64) / (double)(freq64));
#endif
#endif
#if defined(RTS_DEBUG)
	if (TheGlobalData->m_debugAI==AI_DEBUG_ZONES)
	{
		extern void addIcon(const Coord3D *pos, Real width, Int numFramesDuration, RGBColor color);
		RGBColor color;
		memset(&color, 0, sizeof(Color));
		addIcon(nullptr, 0, 0, color);
		Int i, j;
		for( j=0; j<globalBounds.hi.y; j++ )	{
			for( i=0; i<globalBounds.hi.x; i++ )	{
				Int zone = map[i][j].getZone();
				//zone = m_terrainZones[zone];
				zone = m_hierarchicalZones[zone];

				color.blue = (zone%3) * 0.5f;
				zone = zone/3;
				color.green = (zone%3) * 0.5f;
				zone = zone/3;
				color.red = (zone%3) * 0.5;
				Coord3D pos;
				pos.x = ((Real)i + 0.5f) * PATHFIND_CELL_SIZE_F;
				pos.y = ((Real)j + 0.5f) * PATHFIND_CELL_SIZE_F;
				pos.z = TheTerrainLogic->getLayerHeight( pos.x, pos.y, map[i][j].getLayer() ) + 0.5f;
				addIcon(&pos, PATHFIND_CELL_SIZE_F*0.8f, 200, color);
			}
		}
	}
#endif

}

//
// Clear the passable flags.
//
void PathfindZoneManager::clearPassableFlags()
{	Int blockX;
	Int blockY;
	for (blockX = 0; blockX<m_zoneBlockExtent.x; blockX++) {
		for (blockY = 0; blockY<m_zoneBlockExtent.y; blockY++) {
			m_zoneBlocks[blockX][blockY].setPassable(false);
		}
	}
}

//
// Set the passable flags.
//
void PathfindZoneManager::setAllPassable()
{	Int blockX;
	Int blockY;
	for (blockX = 0; blockX<m_zoneBlockExtent.x; blockX++) {
		for (blockY = 0; blockY<m_zoneBlockExtent.y; blockY++) {
			m_zoneBlocks[blockX][blockY].setPassable(true);
		}
	}
}

//
// Set the passable flag for the block at this location.
//
void PathfindZoneManager::setPassable(Int cellX, Int cellY, Bool passable)
{
	Int blockX = cellX/ZONE_BLOCK_SIZE;
	Int blockY = cellY/ZONE_BLOCK_SIZE;

	if (blockX<0 || blockX>=m_zoneBlockExtent.x) {
		DEBUG_CRASH(("Invalid block."));
		return;
	}
	if (blockY<0 || blockY>=m_zoneBlockExtent.y) {
		DEBUG_CRASH(("Invalid block."));
		return;
	}
	m_zoneBlocks[blockX][blockY].setPassable(passable);
}

//
// Get the passable flag for the block at this location.
//
Bool PathfindZoneManager::isPassable(Int cellX, Int cellY) const
{
	Int blockX = cellX/ZONE_BLOCK_SIZE;
	Int blockY = cellY/ZONE_BLOCK_SIZE;

	if (blockX<0 || blockX>=m_zoneBlockExtent.x) {
		DEBUG_CRASH(("Invalid block."));
		return false;
	}
	if (blockY<0 || blockY>=m_zoneBlockExtent.y) {
		DEBUG_CRASH(("Invalid block."));
		return false;
	}
	return m_zoneBlocks[blockX][blockY].isPassable();
}

//
// Get the passable flag for the block at this location.
//
Bool PathfindZoneManager::clipIsPassable(Int cellX, Int cellY) const
{
	Int blockX = cellX/ZONE_BLOCK_SIZE;
	Int blockY = cellY/ZONE_BLOCK_SIZE;

	if (blockX<0 || blockX>=m_zoneBlockExtent.x) {
		return false;
	}
	if (blockY<0 || blockY>=m_zoneBlockExtent.y) {
		return false;
	}
	return m_zoneBlocks[blockX][blockY].isPassable();
}

//
// Set the bridge flag for the block at this location.
//
void PathfindZoneManager::setBridge(Int cellX, Int cellY, Bool bridge)
{
	Int blockX = cellX/ZONE_BLOCK_SIZE;
	Int blockY = cellY/ZONE_BLOCK_SIZE;

	if (blockX<0 || blockX>=m_zoneBlockExtent.x) {
		// DEBUG_CRASH(("Invalid block."));  Bridges can be off the playable grid, so don't crash. jba.
		return;
	}
	if (blockY<0 || blockY>=m_zoneBlockExtent.y) {
		// DEBUG_CRASH(("Invalid block."));  Bridges can be off the playable grid, so don't crash. jba.
		return;
	}
	m_zoneBlocks[blockX][blockY].setInteractsWithBridge(bridge);
}


//
// Set the bridge flag for the block at this location.
//
Bool PathfindZoneManager::interactsWithBridge(Int cellX, Int cellY) const
{
	Int blockX = cellX/ZONE_BLOCK_SIZE;
	Int blockY = cellY/ZONE_BLOCK_SIZE;

	if (blockX<0 || blockX>=m_zoneBlockExtent.x) {
		DEBUG_CRASH(("Invalid block."));
		return false;
	}
	if (blockY<0 || blockY>=m_zoneBlockExtent.y) {
		DEBUG_CRASH(("Invalid block."));
		return false;
	}
	return m_zoneBlocks[blockX][blockY].getInteractsWithBridge();
}


//
// Return the zone at this location.
//
zoneStorageType PathfindZoneManager::getBlockZone(LocomotorSurfaceTypeMask acceptableSurfaces, Bool crusher,Int cellX, Int cellY, PathfindCell **map) const
{
	PathfindCell *cell = &(map[cellX][cellY]);
	Int blockX = cellX/ZONE_BLOCK_SIZE;
	Int blockY = cellY/ZONE_BLOCK_SIZE;

	if (blockX<0 || blockX>=m_zoneBlockExtent.x) {
		DEBUG_CRASH(("Invalid block."));
		return 0;
	}
	if (blockY<0 || blockY>=m_zoneBlockExtent.y) {
		DEBUG_CRASH(("Invalid block."));
		return 0;
	}
	zoneStorageType zone =  m_zoneBlocks[blockX][blockY].getEffectiveZone(acceptableSurfaces, crusher, cell->getZone());
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
	if (zone > m_maxZone) {
#else
	if (zone >= m_maxZone) {
#endif
		DEBUG_CRASH(("Invalid zone."));
		return UNINITIALIZED_ZONE;
	}
	return zone;
}

//
// Return the zone at this location.
//
zoneStorageType PathfindZoneManager::getEffectiveTerrainZone(zoneStorageType zone) const
{
	return m_hierarchicalZones[m_terrainZones[zone]];
}

//
// Return the zone at this location.
//
zoneStorageType PathfindZoneManager::getEffectiveZone( LocomotorSurfaceTypeMask acceptableSurfaces,
																										Bool crusher, zoneStorageType zone) const
{
	//DEBUG_ASSERTCRASH(zone, ("Zone not set"));
	if (zone>m_maxZone) {
		DEBUG_CRASH(("Invalid zone"));
		return (0);
	}
	if (zone>m_maxZone) {
		DEBUG_CRASH(("Invalid zone"));
		return (0);
	}
	if (acceptableSurfaces&LOCOMOTORSURFACE_AIR) return 1; // air is all zone 1.

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_WATER) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_CLIFF)) {
		// Locomotors can go on ground, water & cliff, so all is zone 1.
		return 1;
	}

	if (crusher) {
		zone = m_crusherZones[zone];
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_CLIFF)) {
		// Locomotors can go on ground & cliff, so use the ground cliff combiner.
		zone = m_groundCliffZones[zone];
		return zone;
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_WATER)) {
		// Locomotors can go on ground & water, so use the ground water combiner.
		zone = m_groundWaterZones[zone];
		return zone;
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_GROUND) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_RUBBLE)) {
		// Locomotors can go on ground & rubble, so use the ground rubble combiner.
		zone = m_groundRubbleZones[zone];
		return zone;
	}

	if ( (acceptableSurfaces&LOCOMOTORSURFACE_CLIFF) &&
			(acceptableSurfaces&LOCOMOTORSURFACE_WATER)) {
		// Locomotors can go on ground & cliff, so use the ground cliff combiner.
		DEBUG_CRASH(("Cliff water only locomotor sets not supported yet."));
	}
	zone = m_hierarchicalZones[zone];

	return zone;
}
