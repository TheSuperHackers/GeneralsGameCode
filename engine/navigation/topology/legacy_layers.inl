// Legacy game implementation; compiled within engine.navigation.pathfinder.
//-------------------- PathfindLayer ----------------------------------------
PathfindLayer::PathfindLayer() : m_blockOfMapCells(nullptr), m_layerCells(nullptr), m_bridge(nullptr),
m_destroyed(FALSE),
m_height(0),
m_width(0),
m_xOrigin(0),
m_yOrigin(0),
m_zone(0)
{
	m_startCell.x = -1;
	m_startCell.y = -1;
	m_endCell.x = -1;
	m_endCell.y = -1;
}

PathfindLayer::~PathfindLayer()
{
	reset();
}

/**
 * Returns true if the layer is available for use.
 */
void PathfindLayer::reset()
{
	m_bridge = nullptr;
	if (m_layerCells) {
		Int i, j;
		for (i=0; i<m_width; i++) {
			for (j=0; j<m_height; j++) {
				PathfindCell *cell = &m_layerCells[i][j];
				cell->reset();
			}
		}
		delete [] m_layerCells;
		m_layerCells = nullptr;
	}

	delete [] m_blockOfMapCells;
	m_blockOfMapCells = nullptr;

	m_width = 0;
	m_height = 0;
	m_xOrigin = 0;
	m_yOrigin = 0;
	m_startCell.x = -1;
	m_startCell.y = -1;
	m_endCell.x = -1;
	m_endCell.y = -1;
	m_layer = LAYER_GROUND;
}

/**
 * Returns true if the layer is available for use.
 */
Bool PathfindLayer::isUnused()
{
	// Special case - wall layer is built from not a bridge.  jba.
	if (m_layer == LAYER_WALL && m_width>0) return false;

	if (m_bridge==nullptr) return true;
	return false;
}



/**
 * Draws debug cell info.
 */
#if defined(RTS_DEBUG)
void PathfindLayer::doDebugIcons() {
	if (isUnused()) return;
	extern void addIcon(const Coord3D *pos, Real width, Int numFramesDuration, RGBColor color);
	// render AI debug information
	{
		Coord3D topLeftCorner;
		RGBColor color;
		color.red = color.green = color.blue = 0;
		Coord3D center;
		center.x = (m_xOrigin+m_width/2)*PATHFIND_CELL_SIZE_F;
		center.y = (m_yOrigin+m_height/2)*PATHFIND_CELL_SIZE_F;
		center.z = 0;
		Real bridgeHeight = TheTerrainLogic->getLayerHeight(center.x , center.y, m_layer);
		if (m_layer == LAYER_WALL) {
			bridgeHeight = TheAI->pathfinder()->getWallHeight();
		}
		static Int flash = 0;
		flash--;
		if (flash<1) flash = 20;
		if (flash < 10) return;
		Bool showCells = TheGlobalData->m_debugAI==AI_DEBUG_CELLS;
		// show the pathfind grid
		for( int j=0; j<m_height; j++ )
		{
			topLeftCorner.y = (Real)(j+m_yOrigin) * PATHFIND_CELL_SIZE_F;

			for( int i=0; i<m_width; i++ )
			{
				topLeftCorner.x = (Real)(i+m_xOrigin) * PATHFIND_CELL_SIZE_F;

				color.red = color.green = color.blue = 0;
				Bool empty = false;
				Real size = 0.4f;
				const PathfindCell *cell = &m_layerCells[i][j];
				if (cell)
				{
					if (cell->getConnectLayer()==LAYER_GROUND) {
							color.green = 1;
							color.blue = 1;
							empty = false;
					}	else if (cell->getType() == PathfindCell::CELL_IMPASSABLE) {
							color.red = color.green = color.blue = 1;
							size = 0.2f;
							empty = false;
					}	else if (cell->getType() == PathfindCell::CELL_BRIDGE_IMPASSABLE) {
							color.blue = color.red = 1;
							empty = false;
					}	else if (cell->getType() == PathfindCell::CELL_CLIFF) {
							color.red = 1;
							empty = false;
					}	else {
							size = 0.2f;
					}
				}
				if (showCells) {
					empty = true;
					color.red = color.green = color.blue = 0;
					if (empty && cell) {
						if (cell->getFlags()!=PathfindCell::NO_UNITS) {
							empty = false;
							if (cell->getFlags() == PathfindCell::UNIT_GOAL) {
								color.red = 1;
							}	else if (cell->getFlags() == PathfindCell::UNIT_PRESENT_FIXED) {
								color.green = color.blue = color.red = 1;
							}	else if (cell->getFlags() == PathfindCell::UNIT_PRESENT_MOVING) {
								color.green = 1;
							}	else {
								color.green = color.red = 1;
							}
						}
					}
				}
				if (!empty) {
					Coord3D loc;
					loc.x = topLeftCorner.x + PATHFIND_CELL_SIZE_F/2.0f;
					loc.y = topLeftCorner.y + PATHFIND_CELL_SIZE_F/2.0f;
					loc.z = bridgeHeight;
					addIcon(&loc, PATHFIND_CELL_SIZE_F*size, 99, color);
				}
			}
		}

	}
}
#endif

/**
 * Sets the bridge & layer number for a layer.
 */
Bool PathfindLayer::init(Bridge *theBridge, PathfindLayerEnum layer)
{
	if (m_bridge!=nullptr) return false;
	m_bridge = theBridge;
	m_layer = layer;
	m_destroyed = false;
	return true;
}

/**
 * Allocates the pathfind cells for the bridge layer.
 */
void PathfindLayer::allocateCells(const IRegion2D *extent)
{
	if (m_bridge == nullptr) return;
	Region2D bridgeBounds = *m_bridge->getBounds();
	Int maxX, maxY;
	m_xOrigin = REAL_TO_INT_FLOOR((bridgeBounds.lo.x-PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	m_yOrigin = REAL_TO_INT_FLOOR((bridgeBounds.lo.y-PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	m_width = 0;
	m_height = 0;
	maxX = REAL_TO_INT_CEIL((bridgeBounds.hi.x+PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	maxY = REAL_TO_INT_CEIL((bridgeBounds.hi.y+PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	// Pad with 1 extra;
	m_xOrigin--;
	m_yOrigin--;
	maxX++;
	maxY++;

	if (m_xOrigin < extent->lo.x) m_xOrigin = extent->lo.x;
	if (m_yOrigin < extent->lo.y) m_yOrigin = extent->lo.y;
	if (maxX > extent->hi.x) maxX = extent->hi.x;
	if (maxY > extent->hi.y) maxY = extent->hi.y;
	if (maxX <= m_xOrigin) return;
	if (maxY <= m_yOrigin) return;
	m_width = maxX - m_xOrigin;
	m_height = maxY - m_yOrigin;

	// Allocate cells.
	// pool[]ify
	m_blockOfMapCells = MSGNEW("PathfindMapCells") PathfindCell[m_width*m_height];
	m_layerCells = MSGNEW("PathfindMapCells") PathfindCellP[m_width];
	Int i;
	for (i=0; i<m_width; i++) {
		m_layerCells[i] = &m_blockOfMapCells[i*m_height];
	}
}

/**
 * Allocates the pathfind cells for the wall bridge layer.
 */
void PathfindLayer::allocateCellsForWallLayer(const IRegion2D *extent, ObjectID *wallPieces, Int numPieces)
{
	DEBUG_ASSERTCRASH(m_layer==LAYER_WALL, ("Wrong layer for wall."));
	if (m_layer != LAYER_WALL) return;
	Region2D bridgeBounds;

	Int i;
	Bool first = true;
	for (i=0; i<numPieces; i++) {
		Object *obj = TheGameLogic->findObjectByID(wallPieces[i]);
		Region2D objBounds;
		if (obj==nullptr) continue;
		obj->getGeometryInfo().get2DBounds(*obj->getPosition(), obj->getOrientation(), objBounds);
		if (first) {
			bridgeBounds = objBounds;
			first = false;
		} else {
			if (bridgeBounds.lo.x>objBounds.lo.x) bridgeBounds.lo.x = objBounds.lo.x;
			if (bridgeBounds.lo.y>objBounds.lo.y) bridgeBounds.lo.y = objBounds.lo.y;
			if (bridgeBounds.hi.x<objBounds.hi.x) bridgeBounds.hi.x = objBounds.hi.x;
			if (bridgeBounds.hi.y<objBounds.hi.y) bridgeBounds.hi.y = objBounds.hi.y;
		}
	}

	Int maxX, maxY;
	m_xOrigin = REAL_TO_INT_FLOOR((bridgeBounds.lo.x-PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	m_yOrigin = REAL_TO_INT_FLOOR((bridgeBounds.lo.y-PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	m_width = 0;
	m_height = 0;
	maxX = REAL_TO_INT_CEIL((bridgeBounds.hi.x+PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	maxY = REAL_TO_INT_CEIL((bridgeBounds.hi.y+PATHFIND_CELL_SIZE/100)/PATHFIND_CELL_SIZE);
	// Pad with 1 extra;
	m_xOrigin--;
	m_yOrigin--;
	maxX++;
	maxY++;

	if (m_xOrigin < extent->lo.x) m_xOrigin = extent->lo.x;
	if (m_yOrigin < extent->lo.y) m_yOrigin = extent->lo.y;
	if (maxX > extent->hi.x) maxX = extent->hi.x;
	if (maxY > extent->hi.y) maxY = extent->hi.y;
	if (maxX <= m_xOrigin) return;
	if (maxY <= m_yOrigin) return;
	m_width = maxX - m_xOrigin;
	m_height = maxY - m_yOrigin;

	// Allocate cells.
	m_blockOfMapCells = MSGNEW("PathfindMapCells") PathfindCell[m_width*m_height];
	m_layerCells = MSGNEW("PathfindMapCells") PathfindCellP[m_width];

	for (i=0; i<m_width; i++) {
		m_layerCells[i] = &m_blockOfMapCells[i*m_height];
	}
}

/**
 * Checks to see if a broken bridge connects 2 zones.
 */
Bool PathfindLayer::connectsZones(PathfindZoneManager *zm, const LocomotorSet& locoSet,
																	Int zone1, Int zone2)
{
	if (!m_destroyed) {
		return false;
	}
	Bool found1 = false;
	Bool found2 = false;
	Int i, j;
	for (i=0; i<m_width; i++) {
		for (j=0; j<m_height; j++) {
			PathfindCell *cell = &m_layerCells[i][j];
			if (cell->getConnectLayer()==LAYER_GROUND) {
					PathfindCell *groundCell = TheAI->pathfinder()->getCell(LAYER_GROUND, i+m_xOrigin, j+m_yOrigin);
					DEBUG_ASSERTCRASH(groundCell, ("Should have cell."));
					if (groundCell) {
						zoneStorageType zone = zm->getEffectiveZone(locoSet.getValidSurfaces(),
							true, groundCell->getZone());
						zone = zm->getEffectiveTerrainZone(zone);
						if (zone == zone1) found1 = true;
						if (zone == zone2) found2 = true;
					}
			}
		}
	}
	return found1 && found2;
}

/**
 * Classifies the pathfind cells for the bridge layer.
 */
void PathfindLayer::classifyCells()
{
	m_startCell.x = -1;
	m_startCell.y = -1;
	m_endCell.x = -1;
	m_endCell.y = -1;
	Int i, j;
	for (i=0; i<m_width; i++) {
		for (j=0; j<m_height; j++) {
			PathfindCell *cell = &m_layerCells[i][j];
			cell->setConnectLayer(LAYER_INVALID);
			cell->setLayer(m_layer);
			classifyLayerMapCell(i+m_xOrigin, j+m_yOrigin, cell, m_bridge);
		}
		BridgeInfo info;
		m_bridge->getBridgeInfo(&info);
		Coord3D bridgeDir = info.to;
		bridgeDir.x -= info.from.x;
		bridgeDir.y -= info.from.y;
		bridgeDir.z -= info.from.z;
		bridgeDir.normalize();
		bridgeDir.x *= PATHFIND_CELL_SIZE_F*0.7f;
		bridgeDir.y *= PATHFIND_CELL_SIZE_F*0.7f;

		m_startCell.x = REAL_TO_INT_FLOOR((info.from.x-bridgeDir.x) / PATHFIND_CELL_SIZE_F);
		m_startCell.y = REAL_TO_INT_FLOOR((info.from.y-bridgeDir.y) / PATHFIND_CELL_SIZE_F);
		m_endCell.x = REAL_TO_INT_FLOOR((info.to.x+bridgeDir.x) / PATHFIND_CELL_SIZE_F);
		m_endCell.y = REAL_TO_INT_FLOOR((info.to.y+bridgeDir.y) / PATHFIND_CELL_SIZE_F);
	}
	if (m_destroyed) {
		Int i, j;
		for (i=0; i<m_width; i++) {
			for (j=0; j<m_height; j++) {
				PathfindCell *cell = &m_layerCells[i][j];
				if (cell->getConnectLayer() == LAYER_GROUND) {
					PathfindCell *groundCell = TheAI->pathfinder()->getCell(LAYER_GROUND, i+m_xOrigin, j+m_yOrigin);
					DEBUG_ASSERTCRASH(groundCell, ("Should have cell."));
					if (groundCell) {
						DEBUG_ASSERTCRASH(groundCell->getConnectLayer()==m_layer, ("Should connect to this layer.jba."));
						groundCell->setConnectLayer(LAYER_INVALID); // disconnect it.
					}
				}
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
				cell->setType(PathfindCell::CELL_IMPASSABLE);
#else
				cell->setType(PathfindCell::CELL_BRIDGE_IMPASSABLE);
#endif
			}
		}
	}
}

/**
 * Classifies the pathfind cells for the wall bridge layer.
 */
void PathfindLayer::classifyWallCells(ObjectID *wallPieces, Int numPieces)
{
	DEBUG_ASSERTCRASH(m_layer==LAYER_WALL, ("Wrong layer for wall."));
	if (m_layer != LAYER_WALL) return;
	if (m_layerCells == nullptr) return;

	Int i, j;
	for (i=0; i<m_width; i++) {
		for (j=0; j<m_height; j++) {
			PathfindCell *cell = &m_layerCells[i][j];
			cell->setConnectLayer(LAYER_INVALID);
			cell->setLayer(m_layer);
			classifyWallMapCell(i+m_xOrigin, j+m_yOrigin, cell, wallPieces, numPieces);
			cell->setPinched(false);
		}
	}
	if (m_destroyed) {
		Int i, j;
		for (i=0; i<m_width; i++) {
			for (j=0; j<m_height; j++) {
				PathfindCell *cell = &m_layerCells[i][j];
				if (cell->getConnectLayer() == LAYER_GROUND) {
					PathfindCell *groundCell = TheAI->pathfinder()->getCell(LAYER_GROUND, i+m_xOrigin, j+m_yOrigin);
					DEBUG_ASSERTCRASH(groundCell, ("Should have cell."));
					if (groundCell) {
						DEBUG_ASSERTCRASH(groundCell->getConnectLayer()==m_layer, ("Should connect to this layer.jba."));
						groundCell->setConnectLayer(LAYER_INVALID); // disconnect it.
					}
				}
				cell->setType(PathfindCell::CELL_IMPASSABLE);
			}
		}
	}

	// Tighten up 1 cell.
	for (i=1; i<m_width-1; i++) {
		for (j=1; j<m_height-1; j++) {
			PathfindCell *cell = &m_layerCells[i][j];
			Int k, l;
			for (k=i-1; k<i+2; k++) {
				for (l=j-1; l<j+2; l++) {
					PathfindCell *adjacentCell = &m_layerCells[k][l];
					if (adjacentCell->getType() != PathfindCell::CELL_CLEAR) {
						cell->setPinched(true);
					}
				}
			}
		}
	}
	for (i=0; i<m_width; i++) {
		for (j=0; j<m_height; j++) {
			PathfindCell *cell = &m_layerCells[i][j];
			if (cell->getPinched() && cell->getType() == PathfindCell::CELL_CLEAR) {
				cell->setType(PathfindCell::CELL_CLIFF);
			}
			cell->setPinched(false);
		}
	}
}

/**
 * Relassifies the pathfind cells for the destroyed bridge layer.
 */
Bool PathfindLayer::setDestroyed(Bool destroyed)
{
	if (destroyed == m_destroyed) return false;

	m_destroyed = destroyed;
	classifyCells();

	return true;
}

/**
 * Copies m_zone into the zone for all the member cells.
 */
void PathfindLayer::applyZone()
{
	Int i, j;
	for (i=0; i<m_width; i++) {
		for (j=0; j<m_height; j++) {
			PathfindCell *cell = &m_layerCells[i][j];
			cell->setZone(m_zone);
		}
	}
}


/**
 * Return the bridge's object id.
 */
ObjectID PathfindLayer::getBridgeID()
{
	return m_bridge->peekBridgeInfo()->bridgeObjectID;
}

/**
 * Return the cell at the index location.
 */
PathfindCell *PathfindLayer::getCell(Int x, Int y)
{
	DEBUG_ASSERTCRASH(m_layerCells, ("no data in layer, why get cells?"));
	if (m_layerCells==nullptr) {
		return nullptr;
	}
	x -= m_xOrigin;
	y -= m_yOrigin;
	if (x<0 || x>=m_width) return nullptr;
	if (y<0 || y>=m_height) return nullptr;
	PathfindCell *cell = &m_layerCells[x][y];
	if (cell->getType() == PathfindCell::CELL_IMPASSABLE) {
		return nullptr; // Impassable cells are ignored.
	}
	return cell;
}


/**
 * Classify the given map cell as clear, or not, etc.
 */
void PathfindLayer::classifyLayerMapCell( Int i, Int j , PathfindCell *cell, Bridge *theBridge)
{
	Coord3D topLeftCorner, bottomRightCorner;

	topLeftCorner.y = (Real)j * PATHFIND_CELL_SIZE_F;
	bottomRightCorner.y = topLeftCorner.y + PATHFIND_CELL_SIZE_F;

	topLeftCorner.x = (Real)i * PATHFIND_CELL_SIZE_F;
	bottomRightCorner.x = topLeftCorner.x + PATHFIND_CELL_SIZE_F;


	Int bridgeCount = 0;
	Coord3D pt;
	if (theBridge->isPointOnBridge(&topLeftCorner) ) {
		bridgeCount++;
	}
	pt = topLeftCorner;
	pt.y = bottomRightCorner.y;
	if (theBridge->isPointOnBridge(&pt) ) {
		bridgeCount++;
	}
	if (theBridge->isPointOnBridge(&bottomRightCorner) ) {
		bridgeCount++;
	}
	pt = topLeftCorner;
	pt.x = bottomRightCorner.x;
	if (theBridge->isPointOnBridge(&pt) ) {
		bridgeCount++;
	}
	cell->reset();
	cell->setLayer(m_layer);
	cell->setType(PathfindCell::CELL_IMPASSABLE);
	if (bridgeCount == 4) {
		cell->setType(PathfindCell::CELL_CLEAR);
	} else {
		if (bridgeCount!=0) {
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
			cell->setType(PathfindCell::CELL_CLIFF); // it's off the bridge.
#else
			cell->setType(PathfindCell::CELL_BRIDGE_IMPASSABLE); // it's off the bridge.
#endif
		}

		// check against the end lines.

		Region2D cellBounds;
		cellBounds.lo.x = topLeftCorner.x;
		cellBounds.lo.y = topLeftCorner.y;
		cellBounds.hi.x = bottomRightCorner.x;
		cellBounds.hi.y = bottomRightCorner.y;

#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
		if (m_bridge->isCellOnEnd(&cellBounds)) {
			cell->setType(PathfindCell::CELL_CLEAR);
		}
		if (m_bridge->isCellOnSide(&cellBounds)) {
			cell->setType(PathfindCell::CELL_CLIFF);
		} else {
			if (m_bridge->isCellEntryPoint(&cellBounds)) {
				cell->setType(PathfindCell::CELL_CLEAR);
				cell->setConnectLayer(LAYER_GROUND);
				PathfindCell *groundCell = TheAI->pathfinder()->getCell(LAYER_GROUND, i, j );
				groundCell->setConnectLayer(cell->getLayer());
			}
		}
#else
		if (m_bridge->isCellOnSide(&cellBounds)) {
			cell->setType(PathfindCell::CELL_BRIDGE_IMPASSABLE);
		} else {
			if (m_bridge->isCellOnEnd(&cellBounds)) {
				cell->setType(PathfindCell::CELL_CLEAR);
			}
			if (m_bridge->isCellEntryPoint(&cellBounds)) {
				cell->setType(PathfindCell::CELL_CLEAR);
				cell->setConnectLayer(LAYER_GROUND);
				PathfindCell *groundCell = TheAI->pathfinder()->getCell(LAYER_GROUND, i, j );
				groundCell->setConnectLayer(cell->getLayer());
			}
		}
#endif
	}
	Coord3D center = topLeftCorner;
	center.x += PATHFIND_CELL_SIZE/2;
	center.y += PATHFIND_CELL_SIZE/2;
	if (cell->getType()!=PathfindCell::CELL_IMPASSABLE) {
		if (!(cell->getConnectLayer()==LAYER_GROUND) ) {
			// Check for bridge clearance.  If the ground isn't 1 pathfind cells below, mark impassable.
			Real groundHeight = TheTerrainLogic->getLayerHeight( center.x, center.y, LAYER_GROUND );
			Real bridgeHeight = theBridge->getBridgeHeight( &center, nullptr );
			if (groundHeight+LAYER_Z_CLOSE_ENOUGH_F > bridgeHeight) {
				PathfindCell *groundCell = TheAI->pathfinder()->getCell(LAYER_GROUND,i, j);
				if (!(groundCell->getType()==PathfindCell::CELL_OBSTACLE)) {
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
					groundCell->setType(PathfindCell::CELL_IMPASSABLE);
#else
					groundCell->setType(PathfindCell::CELL_BRIDGE_IMPASSABLE);
#endif
				}
			}
		}
	}
}


Bool PathfindLayer::isPointOnWall(ObjectID *wallPieces, Int numPieces, const Coord3D *pt)
{
	Int i;
	for (i=0; i<numPieces; i++) {
		Object *obj = TheGameLogic->findObjectByID(wallPieces[i]);
		if (obj==nullptr) continue;
		Real major = obj->getGeometryInfo().getMajorRadius();
		Real minor = (obj->getGeometryInfo().getGeomType() == GEOMETRY_SPHERE) ? obj->getGeometryInfo().getMajorRadius() : obj->getGeometryInfo().getMinorRadius();

		Real c = (Real)Cos(-obj->getOrientation());
		Real s = (Real)Sin(-obj->getOrientation());

		// convert to a delta relative to rect ctr
		Real ptx = pt->x - obj->getPosition()->x;
		Real pty = pt->y - obj->getPosition()->y;

		// inverse-rotate it to the right coord system
		Real ptx_new = (Real)fabs(ptx*c - pty*s);
		Real pty_new = (Real)fabs(ptx*s + pty*c);

		if (ptx_new <= major && pty_new <= minor)
		{
			return true;
		}
	}
	return false;
}


/**
 * Classify the given map cell as clear, or not, etc.
 */
void PathfindLayer::classifyWallMapCell( Int i, Int j , PathfindCell *cell, ObjectID *wallPieces, Int numPieces)
{
	Coord3D topLeftCorner, bottomRightCorner;

	topLeftCorner.y = (Real)j * PATHFIND_CELL_SIZE_F;
	bottomRightCorner.y = topLeftCorner.y + PATHFIND_CELL_SIZE_F;

	topLeftCorner.x = (Real)i * PATHFIND_CELL_SIZE_F;
	bottomRightCorner.x = topLeftCorner.x + PATHFIND_CELL_SIZE_F;


	Int bridgeCount = 0;
	Coord3D pt;
	if (isPointOnWall(wallPieces, numPieces, &topLeftCorner) ) {
		bridgeCount++;
	}
	pt = topLeftCorner;
	pt.y = bottomRightCorner.y;
	if (isPointOnWall(wallPieces, numPieces, &pt) ) {
		bridgeCount++;
	}
	if (isPointOnWall(wallPieces, numPieces, &bottomRightCorner) ) {
		bridgeCount++;
	}
	pt = topLeftCorner;
	pt.x = bottomRightCorner.x;
	if (isPointOnWall(wallPieces, numPieces, &pt) ) {
		bridgeCount++;
	}
	cell->reset();
	cell->setLayer(m_layer);
	cell->setType(PathfindCell::CELL_IMPASSABLE);
	if (bridgeCount == 4) {
		cell->setType(PathfindCell::CELL_CLEAR);
	} else {
		if (bridgeCount!=0) {
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
			cell->setType(PathfindCell::CELL_CLIFF); // it's off the bridge.
#else
			cell->setType(PathfindCell::CELL_BRIDGE_IMPASSABLE); // it's off the bridge.
#endif
		}

	}
}

