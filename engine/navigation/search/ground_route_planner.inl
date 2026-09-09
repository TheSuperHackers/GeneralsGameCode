namespace navigation {
struct GroundRoutePlanner::State {
    ClearanceCache clearance;
    std::array<JumpGridWorkspace, 6> workspaces;
    Int originX = 0, originY = 0;
};
GroundRoutePlanner::GroundRoutePlanner(Pathfinder& world)
    : state_(std::make_unique<State>()), world_(world), movement_(world), builder_(world) {}
GroundRoutePlanner::~GroundRoutePlanner() = default;
void GroundRoutePlanner::reset() {
    state_->clearance.reset(); stats_ = {}; enabled_ = directEnabled_ = true;
}
void GroundRoutePlanner::invalidate() { state_->clearance.invalidateAll(); }
void GroundRoutePlanner::invalidate(const IRegion2D& region) {
    state_->clearance.invalidate({region.lo.x - state_->originX, region.lo.y - state_->originY,
        region.hi.x - state_->originX, region.hi.y - state_->originY});
}
Pathfinder::JpsAdapterStats GroundRoutePlanner::stats() const {
    auto result = stats_;
    result.topologyRebuilds = state_->clearance.rebuilds();
    result.baseCellsExamined = state_->clearance.baseCells();
    result.clearanceCellsExamined = state_->clearance.clearanceCells();
    return result;
}
Bool GroundRoutePlanner::prepareTopology(Int radius, Bool center) {
    if (!world_.m_map || !world_.m_isMapReady || radius < 0 || radius > 2) return false;
    const auto& extent = world_.m_extent;
    const Int width = extent.hi.x - extent.lo.x + 1, height = extent.hi.y - extent.lo.y + 1;
    if (width <= 0 || height <= 0) return false;
    if (state_->originX != extent.lo.x || state_->originY != extent.lo.y) {
        state_->clearance.invalidateAll();
        state_->originX = extent.lo.x; state_->originY = extent.lo.y;
    }
    state_->clearance.setShape(width, height);
    const bool ready = state_->clearance.prepare(radius, center, [this](Int x, Int y) {
        const auto* cell = world_.getCell(LAYER_GROUND, x + state_->originX, y + state_->originY);
        return !cell || cell->getType() != PathfindCell::CELL_CLEAR;
    });
    if (!ready) RELEASE_CRASH("Ground clearance topology synchronization failed");
    return ready;
}
Path *GroundRoutePlanner::find(const Pathfinder::JpsGroundQuery& query,
	const Coord3D *from, const Coord3D *rawTo)
{
	if (!enabled_ || !from || !rawTo) {
		++stats_.unavailableFallback;
		return nullptr;
	}
	if (query.acceptableSurfaces != LOCOMOTORSURFACE_GROUND ||
		query.radius < 0 || query.radius > 2 || world_.m_isTunneling ||
		(query.usePathDiameter &&
			(query.pathDiameter != 1 && query.pathDiameter != 2 &&
			 query.pathDiameter != 4 || query.crusher))) {
		++stats_.unavailableFallback;
		return nullptr;
	}
	if (query.object && query.object->getLayer() != LAYER_GROUND) {
		++stats_.unavailableFallback;
		return nullptr;
	}

	Coord3D adjustedTo = *rawTo;
	Coord3D clippedFrom = *from;
	world_.clip(&clippedFrom, &adjustedTo);
	if (!query.centerInCell) {
		adjustedTo.x += PATHFIND_CELL_SIZE_F / 2;
		adjustedTo.y += PATHFIND_CELL_SIZE_F / 2;
	}
	if (TheTerrainLogic->getLayerForDestination(&clippedFrom) != LAYER_GROUND ||
		TheTerrainLogic->getLayerForDestination(&adjustedTo) != LAYER_GROUND) {
		++stats_.unavailableFallback;
		return nullptr;
	}

	ICoord2D startCell;
	ICoord2D goalCell;
	world_.worldToCell(&clippedFrom, &startCell);
	world_.worldToCell(&adjustedTo, &goalCell);
	if (query.usePathDiameter &&
		query.pathDiameter != world_.clearCellForDiameter(query.crusher, goalCell.x,
			goalCell.y, LAYER_GROUND, query.pathDiameter)) {
		ICoord2D originalGoal = goalCell;
		const Int maxOffset = 8;
		Int offset = 1;
		while (offset < maxOffset) {
			goalCell = originalGoal;
			goalCell.x += offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			goalCell.y += offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			goalCell.x -= offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			goalCell.x -= offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			goalCell.y -= offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			goalCell.y -= offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			goalCell.x += offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			goalCell.x += offset;
			if (world_.clearCellForDiameter(query.crusher, goalCell.x, goalCell.y,
				LAYER_GROUND, query.pathDiameter) == query.pathDiameter) break;
			++offset;
		}
		if (offset >= maxOffset) {
			++stats_.unavailableFallback;
			return nullptr;
		}
	}
	auto *startPathCell = world_.getCell(LAYER_GROUND, startCell.x, startCell.y);
	auto *goalPathCell = world_.getCell(LAYER_GROUND, goalCell.x, goalCell.y);
	if (!startPathCell || !goalPathCell ||
		(!query.usePathDiameter && !world_.checkDestination(query.object, goalCell.x,
			goalCell.y, LAYER_GROUND, query.radius, query.centerInCell)) ||
		!world_.validMovementPosition(query.crusher, query.acceptableSurfaces, startPathCell) ||
		!world_.validMovementPosition(query.crusher, query.acceptableSurfaces, goalPathCell) ||
		(query.usePathDiameter &&
			(world_.clearCellForDiameter(query.crusher, startCell.x, startCell.y,
				LAYER_GROUND, query.pathDiameter) < query.pathDiameter))) {
		++stats_.unavailableFallback;
		return nullptr;
	}
	GroundSegmentValidation validation;
	validation.context = MovementValidator::prepare(query.object, query.radius, query.centerInCell);
	validation.movement.layer = LAYER_GROUND;
	validation.movement.radius = query.radius;
	validation.movement.centerInCell = query.centerInCell;
	validation.movement.considerTransient = false;
	validation.movement.acceptableSurfaces = query.acceptableSurfaces;
	validation.acceptableSurfaces = query.acceptableSurfaces;
	validation.isCrusher = query.crusher;
	validation.pathDiameter = query.usePathDiameter ? query.pathDiameter : 0;
	validation.isHuman = query.isHuman;
	++stats_.attempted;
	if (directEnabled_ &&
		(startCell.x != goalCell.x || startCell.y != goalCell.y)) {
		Coord3D goalPosition;
		world_.adjustCoordToCell(goalCell.x, goalCell.y, query.centerInCell, goalPosition, LAYER_GROUND);
		validation.checkStaticFootprint = true;
		if (movement_.segment(validation, *from, goalPosition)) {
			auto *direct = newInstance(Path);
			direct->appendNode(from, LAYER_GROUND);
			direct->appendNode(&goalPosition, LAYER_GROUND);
			direct->getFirstNode()->setNextOptimized(direct->getLastNode());
			direct->markOptimized();
			builder_.materialize(direct, query.centerInCell);
			++stats_.directAccepted;
			++stats_.accepted;
			return direct;
		}
		validation.checkStaticFootprint = false;
	}
	if (!prepareTopology(query.radius, query.centerInCell)) {
		++stats_.unavailableFallback;
		return nullptr;
	}

	auto& workspace = state_->workspaces[ClearanceCache::variantIndex(query.radius, query.centerInCell)];
    const auto snapshot = state_->clearance.snapshot(query.radius, query.centerInCell);
	if (!snapshot.valid()) {
		DEBUG_ASSERTCRASH(false, ("JPS topology snapshot disappeared after synchronization"));
		RELEASE_CRASH("JPS topology snapshot disappeared after synchronization");
		return nullptr;
	}
	const navigation::JumpGridCell start{startCell.x - world_.m_extent.lo.x,
		startCell.y - world_.m_extent.lo.y};
	const navigation::JumpGridCell goal{goalCell.x - world_.m_extent.lo.x,
		goalCell.y - world_.m_extent.lo.y};
	const auto search = workspace.find(snapshot, start, goal);
	const auto maxWork = std::numeric_limits<std::uint64_t>::max();
	const auto totalWork = search.scanned > maxWork - search.expanded
		? maxWork : search.scanned + search.expanded;
	const auto maxInt = static_cast<std::uint64_t>(std::numeric_limits<Int>::max());
	world_.recordNavigationWork(totalWork > maxInt ? std::numeric_limits<Int>::max()
		: static_cast<Int>(totalWork));
	if (!search.found || search.path.empty() || search.path.front() != start ||
		search.path.back() != goal || search.path.size() < 2) {
		++stats_.unavailableFallback;
		return nullptr;
	}

	Path *candidate = newInstance(Path);
	candidate->appendNode(from, LAYER_GROUND);
	Coord3D previous = *from;
	for (std::size_t i = 1; i < search.path.size(); ++i) {
		const auto point = search.path[i];
		Coord3D current;
		world_.adjustCoordToCell(point.x + world_.m_extent.lo.x,
			point.y + world_.m_extent.lo.y, query.centerInCell, current, LAYER_GROUND);
		if (!movement_.segment(validation, previous, current)) {
			deleteInstance(candidate);
			++stats_.validationRejected;
			return nullptr;
		}
		candidate->appendNode(&current, LAYER_GROUND);
		previous = current;
	}

	if (query.usePathDiameter)
		candidate->optimizeGroundPath(query.crusher, query.pathDiameter);
	else
		candidate->optimize(query.object, query.acceptableSurfaces, false);
	builder_.materialize(candidate, query.centerInCell);
	++stats_.accepted;
	return candidate;
}

}
