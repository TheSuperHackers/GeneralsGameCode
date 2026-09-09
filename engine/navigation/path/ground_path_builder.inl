#if defined(RTS_DEBUG)
extern void addIcon(const Coord3D*, Real, Int, RGBColor);
#endif
namespace navigation {
namespace {
struct GroundPathMaterialization {
	PathNode *previous;
	ICoord2D end;
	Bool center;
};
}

Int GroundPathBuilder::visit(Pathfinder *pathfinder,
	PathfindCell *from, PathfindCell *, Int x, Int y, void *userData)
{
	auto& materialization = *static_cast<GroundPathMaterialization *>(userData);
	if (x == materialization.end.x && y == materialization.end.y)
		return 1; // The endpoint already exists; stop before the iterator's extra end probe.
	if (!from)
		return 0; // Preserve the unit's exact starting position.
	Coord3D position;
	pathfinder->adjustCoordToCell(x, y, materialization.center, position, LAYER_GROUND);
	auto *node = newInstance(PathNode);
	node->setPosition(&position);
	node->setLayer(LAYER_GROUND);
	node->setCanOptimize(true);
	materialization.previous->append(node);
	materialization.previous = node;
	pathfinder->recordNavigationWork(1);
	return 0;
}

void GroundPathBuilder::materialize(Path *path, Bool center)
{
	// Searches may use long edges, but moveAllies and patchPath consume raw
	// cells, not optimized segments. Keep their nearby occupancy/rejoin points.
	// Insert after smoothing without changing the optimized links: smoothing
	// needs only the search waypoints, not every intermediate avoidance cell.
	// Hierarchical paths and layer transitions do not use this conversion.
	for (auto *start = path->getFirstNode(); start && start->getNext();) {
		auto *end = start->getNext();
		if (start->getLayer() == LAYER_GROUND && end->getLayer() == LAYER_GROUND) {
			ICoord2D fromCell, toCell;
			world_.worldToCell(start->getPosition(), &fromCell);
			world_.worldToCell(end->getPosition(), &toCell);
			if ((abs(toCell.x - fromCell.x) > 1 || abs(toCell.y - fromCell.y) > 1) &&
				world_.getCell(LAYER_GROUND, fromCell.x, fromCell.y) &&
				world_.getCell(LAYER_GROUND, toCell.x, toCell.y)) {
				GroundPathMaterialization materialization{start, toCell, center};
				world_.iterateCellsAlongLine(fromCell, toCell, LAYER_GROUND,
					&GroundPathBuilder::visit, &materialization);
			}
		}
		start = end;
	}
}


Path *GroundPathBuilder::buildGround(Bool isCrusher, const Coord3D *fromPos, PathfindCell *goalCell, Bool center, Int pathDiameter )
{
	DEBUG_ASSERTCRASH( goalCell, ("Pathfinder::buildActualPath: goalCell == nullptr") );

	Path *path = newInstance(Path);

	prepend(path, fromPos, goalCell, center);

	// cleanup the path by checking line of sight
	path->optimizeGroundPath( isCrusher, pathDiameter );
	materialize(path, center);


#if defined(RTS_DEBUG)
	if (TheGlobalData->m_debugAI==AI_DEBUG_GROUND_PATHS)
	{
		
 		RGBColor color;
		color.blue = 0;
		color.red = color.green = 1;
		Coord3D pos;
		PathNode *node = path->getFirstNode();
		for( ; node; node = node->getNext() )
		{

			// create objects to show path - they decay

			pos = *node->getPosition();
			color.red = color.green = 1;
			if (node->getLayer() != LAYER_GROUND) {
				color.red = 0;
			}
			::addIcon(&pos, PATHFIND_CELL_SIZE_F*.25f, 200, color);
		}

		// show optimized path
		for( node = path->getFirstNode(); node; node = node->getNextOptimized() )
		{
			pos = *node->getPosition();
			::addIcon(&pos, PATHFIND_CELL_SIZE_F*.8f, 200, color);
		}
		world_.setDebugPath(path);
	}
#endif
	return path;
}

Path *GroundPathBuilder::buildHierarchical( const Coord3D *fromPos, PathfindCell *goalCell )
{
	DEBUG_ASSERTCRASH( goalCell, ("Pathfinder::buildHierarchicalPath: goalCell == nullptr") );

	Path *path = newInstance(Path);

	prepend(path, fromPos, goalCell, true);

#if !(RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING)
	// Expand the hierarchical path around the starting point. jba [8/24/2003]
	// This allows the unit to get around friendly units that may be near it.
	Coord3D pos = *path->getFirstNode()->getPosition();
	Coord3D minPos = pos;
	minPos.x -= PathfindZoneManager::ZONE_BLOCK_SIZE*PATHFIND_CELL_SIZE_F;
	minPos.y -= PathfindZoneManager::ZONE_BLOCK_SIZE*PATHFIND_CELL_SIZE_F;
	Coord3D maxPos = pos;
	maxPos.x += PathfindZoneManager::ZONE_BLOCK_SIZE*PATHFIND_CELL_SIZE_F;
	maxPos.y += PathfindZoneManager::ZONE_BLOCK_SIZE*PATHFIND_CELL_SIZE_F;
	ICoord2D cellNdxMin, cellNdxMax;
	world_.worldToCell(&minPos, &cellNdxMin);
	world_.worldToCell(&maxPos, &cellNdxMax);
	Int i, j;
	for (i=cellNdxMin.x; i<=cellNdxMax.x; i++) {
		for (j=cellNdxMin.y; j<=cellNdxMax.y; j++) {
			world_.m_zoneManager.setPassable(i, j, true);
		}
	}
#endif

#if defined(RTS_DEBUG)
	if (TheGlobalData->m_debugAI==AI_DEBUG_PATHS)
	{
		
 		RGBColor color;
		color.blue = 0;
		color.red = color.green = 1;
		Coord3D pos;
		Int i;
		for (i=0; i<3; i++)
		for( PathNode *node = path->getFirstNode(); node; node = node->getNext() )
		{

			// create objects to show path - they decay

			pos = *node->getPosition();
			color.red = 1;
			color.green = 0.4f;
			if (node->getLayer() != LAYER_GROUND) {
				color.red = 0;
			}
			::addIcon(&pos, PATHFIND_CELL_SIZE_F, 200, color);
		}
		world_.setDebugPath(path);
	}
#endif
	return path;
}

Path *GroundPathBuilder::buildActual( const Object *obj, LocomotorSurfaceTypeMask acceptableSurfaces, const Coord3D *fromPos,
																	PathfindCell *goalCell, Bool center, Bool blocked )
{
	DEBUG_ASSERTCRASH( goalCell, ("Pathfinder::buildActualPath: goalCell == nullptr") );

	Path *path = newInstance(Path);

	if (goalCell->getPinched() && goalCell->getParentCell() && !goalCell->getParentCell()->getPinched()) {
		goalCell = goalCell->getParentCell();
	}

	prepend(path, fromPos, goalCell, center);

	// cleanup the path by checking line of sight
	path->optimize(obj, acceptableSurfaces, blocked);
	materialize(path, center);

#if defined(RTS_DEBUG)
	if (TheGlobalData->m_debugAI==AI_DEBUG_PATHS)
	{
		
 		RGBColor color;
		color.blue = 0;
		color.red = color.green = 1;
		Coord3D pos;
		PathNode *node = path->getFirstNode();
		for( ; node; node = node->getNext() )
		{

			// create objects to show path - they decay

			pos = *node->getPosition();
			color.red = color.green = 1;
			if (node->getLayer() != LAYER_GROUND) {
				color.red = 0;
			}
			::addIcon(&pos, PATHFIND_CELL_SIZE_F*.25f, 200, color);
		}

		// show optimized path
		for( node = path->getFirstNode(); node; node = node->getNextOptimized() )
		{
			pos = *node->getPosition();
			::addIcon(&pos, PATHFIND_CELL_SIZE_F*.8f, 200, color);
		}
		world_.setDebugPath(path);
	}
#endif
	return path;
}

void GroundPathBuilder::prepend( Path *path, const Coord3D *fromPos,
																	PathfindCell *goalCell, Bool center )
{
	// traverse path cells in REVERSE order, creating path in desired order
	// skip the LAST node, as that will be in the same cell as the unit itself - so use the unit's position
	Coord3D pos;
	PathfindCell *cell, *prevCell = nullptr;
	Bool goalCellNull = (goalCell->getParentCell()==nullptr);
	for( cell = goalCell; cell->getParentCell(); cell = cell->getParentCell() )
	{
		world_.m_zoneManager.setPassable(cell->getXIndex(), cell->getYIndex(), true);
		coordinate(cell->getXIndex(), cell->getYIndex(), center, pos, cell->getLayer());
		if (prevCell && cell->getXIndex()==prevCell->getXIndex() && cell->getYIndex()==prevCell->getYIndex()) {
			// transitioning layers.
			PathfindLayerEnum layer = cell->getLayer();
			if (layer==LAYER_GROUND) {
				layer = prevCell->getLayer();
			}
			DEBUG_ASSERTCRASH(layer!=LAYER_GROUND, ("Should have at 1 non-ground layer. jba"));
			path->getFirstNode()->setLayer(layer);
			continue;
		}

		Bool canOptimize = true;
		if (cell->getType() == PathfindCell::CELL_CLIFF) {
			if (prevCell && prevCell->getType() != PathfindCell::CELL_CLIFF) {
				if (path->getFirstNode()) {
					path->getFirstNode()->setCanOptimize(false);
				}
			}
		}	else {
			if (prevCell && prevCell->getType() == PathfindCell::CELL_CLIFF) {
				canOptimize = false;
			}
		}

		path->prependNode( &pos, cell->getLayer() );
		path->getFirstNode()->setCanOptimize(canOptimize);
		if (cell->isBlockedByAlly()) {
			path->setBlockedByAlly(true);
		}
		if (prevCell) {
			prevCell->clearParentCell();
		}
		prevCell = cell;
	}


	world_.m_zoneManager.setPassable(cell->getXIndex(), cell->getYIndex(), true);
	if (goalCellNull) {
		// Very short path.
		coordinate(cell->getXIndex(), cell->getYIndex(), center, pos, cell->getLayer());
		path->prependNode( &pos, cell->getLayer() );
	}
	// put actual start position as first node on the path, so it begins right at the unit's feet
	if (fromPos->x != path->getFirstNode()->getPosition()->x || fromPos->y != path->getFirstNode()->getPosition()->y) {
		path->prependNode( fromPos, cell->getLayer() );
	}

}

void GroundPathBuilder::coordinate(Int cellX, Int cellY, Bool centerInCell, Coord3D &pos, PathfindLayerEnum layer)
{
	if (centerInCell) {
		pos.x = ((Real)cellX + 0.5f) * PATHFIND_CELL_SIZE_F;
		pos.y = ((Real)cellY + 0.5f) * PATHFIND_CELL_SIZE_F;
	} else {
		pos.x = ((Real)cellX+0.05) * PATHFIND_CELL_SIZE_F;
		pos.y = ((Real)cellY+0.05) * PATHFIND_CELL_SIZE_F;
	}
	pos.z = TheTerrainLogic->getLayerHeight( pos.x, pos.y, layer );
}
}
