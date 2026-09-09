namespace navigation {
MovementContext MovementValidator::prepare(
	const Object *obj, Int radius, Bool center)
{
	MovementContext context;
	context.object = obj;
	context.radius = radius;
	context.cellsAbove = radius + (center ? 1 : 0);
	if (obj) {
		context.objectId = obj->getID();
		if (const auto* ai = obj->getAIUpdateInterface())
			context.ignoredId = ai->getIgnoredObstacleID();
#ifdef INFANTRY_MOVES_THROUGH_INFANTRY
		context.infantry = obj->isKindOf(KINDOF_INFANTRY);
#endif
	}
	return context;
}

namespace {
struct NativeOccupants {
    const Object* mover;
    Object* find(std::uint32_t id) const { return TheGameLogic->findObjectByID(static_cast<ObjectID>(id)); }
    Bool allied(const Object* unit) const { return mover->getRelationship(unit) == ALLIES; }
    Bool infantry(const Object* unit) const { return unit->isKindOf(KINDOF_INFANTRY); }
    Bool canMoveAside(const Object* unit) const {
        const auto* ai = unit->getAIUpdateInterface();
        if (!ai) return false;
#if RTS_GENERALS && RETAIL_COMPATIBLE_PATHFINDING
        return ai->isIdle();
#else
        return true;
#endif
    }
    Bool canCrush(Object* unit) const { return mover->canCrushOrSquish(unit, TEST_CRUSH_OR_SQUISH); }
};
}

Bool MovementValidator::check(const MovementContext& context, TCheckMovementInfo& info)
{
    info.allyFixedCount = 0;
    info.enemyFixed = info.allyMoving = info.allyGoal = false;
    if (!context.object) return true;
    OccupancyQuery query{static_cast<std::uint32_t>(context.objectId),
        static_cast<std::uint32_t>(context.ignoredId), info.cell.x, info.cell.y,
        context.radius, context.cellsAbove, info.considerTransient != 0, false};
#ifdef INFANTRY_MOVES_THROUGH_INFANTRY
    query.infantryPassThrough = context.infantry != 0;
#endif
    OccupancyResult result;
    const bool allowed = checkOccupancy(query, result, [&](Int x, Int y) {
        const auto* cell = world_.getCell(info.layer, x, y);
        if (!cell) {
            OccupancyCell invalid; invalid.valid = false; return invalid;
        }
        const auto flags = cell->getFlags();
        return OccupancyCell{static_cast<std::uint32_t>(cell->getPosUnit()), true,
            flags == PathfindCell::NO_UNITS,
            flags == PathfindCell::UNIT_GOAL || flags == PathfindCell::UNIT_GOAL_OTHER_MOVING,
            flags == PathfindCell::UNIT_PRESENT_MOVING || flags == PathfindCell::UNIT_GOAL_OTHER_MOVING,
            flags == PathfindCell::UNIT_PRESENT_FIXED};
    }, NativeOccupants{context.object});
    info.allyFixedCount = result.allyFixedCount;
    info.enemyFixed = result.enemyFixed;
    info.allyMoving = result.allyMoving;
    info.allyGoal = result.allyGoal;
    return allowed;
}

/*static*/ Int MovementValidator::visit(Pathfinder *pathfinder,
	PathfindCell *, PathfindCell *to, Int toX, Int toY, void *userData)
{
	auto *validation = static_cast<GroundSegmentValidation *>(userData);
	pathfinder->recordNavigationWork(1);
	if (!to) {
		validation->legal = false;
		return 1;
	}
	if (validation->checkStaticFootprint) {
		const auto& context = validation->context;
		const Int above = context.radius == 0 ? 1 : context.cellsAbove;
		for (Int y = toY - context.radius; y < toY + above; ++y) {
			for (Int x = toX - context.radius; x < toX + above; ++x) {
				const auto* cell = pathfinder->getCell(LAYER_GROUND, x, y);
				if (!cell || cell->getType() != PathfindCell::CELL_CLEAR) {
					validation->legal = false;
					return 1;
				}
			}
		}
	}
	auto& movement = validation->movement;
	movement.cell = {toX, toY};
	if (!MovementValidator(*pathfinder).check(validation->context, movement) ||
		movement.allyFixedCount != 0 || movement.enemyFixed) {
		validation->legal = false;
		return 1;
	}
	if (to->getPinched()) {
		validation->legal = false;
		return 1;
	}
	if (!pathfinder->validMovementPosition(validation->isCrusher,
		validation->acceptableSurfaces, to)) {
		validation->legal = false;
		return 1;
	}
	if (validation->pathDiameter > 0 &&
		pathfinder->clearCellForDiameter(validation->isCrusher, toX, toY,
			to->getLayer(), validation->pathDiameter) < validation->pathDiameter) {
		validation->legal = false;
		return 1;
	}
	if (validation->isHuman) {
		ICoord2D cell{toX, toY};
		if (pathfinder->checkCellOutsideExtents(cell)) {
			validation->legal = false;
			return 1;
		}
	}
	// The legacy iterator also probes beyond its endpoint. That cell is not
	// part of this segment and may be blocked immediately after a valid turn.
	return toX == validation->end.x && toY == validation->end.y ? 2 : 0;
}

Bool MovementValidator::segment(GroundSegmentValidation& validation,
	const Coord3D& start, const Coord3D& end)
{
	validation.legal = true;
	world_.worldToCell(&end, &validation.end);
	const Int result = world_.iterateCellsAlongLine(start, end, LAYER_GROUND,
		&MovementValidator::visit, &validation);
	return result == 2 && validation.legal;
}

}
