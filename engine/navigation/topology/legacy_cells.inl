// Legacy game implementation; compiled within engine.navigation.pathfinder.
navigation::SearchWorkspace<PathfindCell> PathfindCell::s_search;

UnsignedInt PathfindCell::searchEpoch()
{
    return s_search.epoch();
}

void PathfindCellList::reset()
{
    if (m_open) PathfindCell::releaseOpenList(*this);
    else PathfindCell::releaseClosedList(*this);
}
PathfindCell* PathfindCellList::getHead() const
{
    auto& search = PathfindCell::s_search;
    const auto id = m_open ? search.open.top() : search.closedHead();
    return id == navigation::NoSearchIndex ? nullptr : search.owner[id];
}
PathfindCell* PathfindCellList::next(PathfindCell* cell) const
{
    auto& search = PathfindCell::s_search;
    const auto id = m_open ? search.open.next(cell->m_searchIndex) : search.nextClosed(cell->m_searchIndex);
    return id == navigation::NoSearchIndex ? nullptr : search.owner[id];
}
Bool PathfindCellList::empty() const { return getHead() == nullptr; }

UnsignedShort PathfindCell::getXIndex() const { return s_search.x[m_searchIndex]; }
UnsignedShort PathfindCell::getYIndex() const { return s_search.y[m_searchIndex]; }
Bool PathfindCell::getOpen() const { return hasInfo() && s_search.isOpen(m_searchIndex); }
Bool PathfindCell::getClosed() const { return hasInfo() && s_search.isClosed(m_searchIndex); }
UnsignedInt PathfindCell::getCostSoFar() const { return s_search.g[m_searchIndex]; }
UnsignedInt PathfindCell::getTotalCost() const { return s_search.f[m_searchIndex]; }
void PathfindCell::setCostSoFar(UnsignedInt cost)
{
    if (hasInfo()) {
        s_search.touch(m_searchIndex);
        s_search.g[m_searchIndex] = cost;
    }
}
void PathfindCell::setTotalCost(UnsignedInt cost)
{
    if (hasInfo()) {
        s_search.touch(m_searchIndex);
        s_search.f[m_searchIndex] = cost;
    }
}
PathfindCell* PathfindCell::getParentCell() const
{
    const auto id = hasInfo() ? s_search.parentOf(m_searchIndex) : navigation::NoSearchIndex;
    return id == navigation::NoSearchIndex ? nullptr : s_search.owner[id];
}
ObjectID PathfindCell::getGoalUnit() const { return hasInfo() ? static_cast<ObjectID>(s_search.goalUnit[m_searchIndex]) : INVALID_ID; }
ObjectID PathfindCell::getGoalAircraft() const { return hasInfo() ? static_cast<ObjectID>(s_search.goalAircraft[m_searchIndex]) : INVALID_ID; }
ObjectID PathfindCell::getPosUnit() const { return hasInfo() ? static_cast<ObjectID>(s_search.positionUnit[m_searchIndex]) : INVALID_ID; }

/**
 * Constructor
 */
PathfindCell::PathfindCell() :m_searchIndex(NO_SEARCH_INDEX)
{
	reset();
}

/**
 * Destructor
 */
PathfindCell::~PathfindCell()
{
	if (hasInfo()) s_search.release(m_searchIndex);
	m_searchIndex = NO_SEARCH_INDEX;
}

/**
 * Reset the cell to default values
 */
void PathfindCell::reset()
{
	m_type = PathfindCell::CELL_CLEAR;
	m_flags = PathfindCell::NO_UNITS;
	m_zone = 0;
	m_aircraftGoal = false;
	m_pinched = false;
	if (hasInfo()) {
		s_search.release(m_searchIndex);
		m_searchIndex = NO_SEARCH_INDEX;
	}
	m_obstacleID = INVALID_ID;
	m_blockedByAlly = false;
	m_obstacleIsFence = false;
	m_obstacleIsTransparent = false;

	m_connectsToLayer = LAYER_INVALID;
	m_layer = LAYER_GROUND;

}

/**
 * Reset the pathfinding values in the cell.
 */
Bool PathfindCell::startPathfind(PathfindCell* goalCell)
{
    DEBUG_ASSERTCRASH(hasInfo(), ("Has to have search state."));
    s_search.beginSearch();
    s_search.touch(m_searchIndex);
    if (goalCell) {
        s_search.touch(goalCell->m_searchIndex);
        s_search.f[m_searchIndex] = costToGoal(goalCell);
    }
    return true;
}

/**
 * Set the blocked by ally flag on the pathfind cell info.
 */
Bool PathfindCell::isBlockedByAlly() const
{
	return m_blockedByAlly;
}

void PathfindCell::setBlockedByAlly(Bool blocked)
{
	m_blockedByAlly = (blocked != 0);
}

/**
 * Set the parent pointer.
 */
void PathfindCell::setParentCell( PathfindCell* parent  )
{
	DEBUG_ASSERTCRASH(hasInfo(), ("Has to have info."));
	s_search.touch(m_searchIndex);
	s_search.parent[m_searchIndex] = parent->m_searchIndex;
	Int dx = s_search.x[m_searchIndex] - s_search.x[parent->m_searchIndex];
	Int dy = s_search.y[m_searchIndex] - s_search.y[parent->m_searchIndex];
	if (dx<-1 || dx>1 || dy<-1 || dy>1) {
		DEBUG_CRASH(("Invalid parent index."));
	}
}

/**
 * Set the parent pointer.
 */
void PathfindCell::setParentCellHierarchical( PathfindCell* parent  )
{
	DEBUG_ASSERTCRASH(hasInfo(), ("Has to have info."));
	s_search.touch(m_searchIndex);
	s_search.parent[m_searchIndex] = parent->m_searchIndex;
}

/**
 * Reset the parent cell.
 */
void PathfindCell::clearParentCell(  )
{
	DEBUG_ASSERTCRASH(hasInfo(), ("Has to have info."));
	s_search.touch(m_searchIndex);
	s_search.parent[m_searchIndex] = navigation::NoSearchIndex;
}


/**
 * Allocates an info record for a cell.
 */
Bool PathfindCell::allocateInfo(const ICoord2D& pos)
{
    if (!hasInfo()) m_searchIndex = s_search.acquire(this, pos.x, pos.y);
    else s_search.touch(m_searchIndex);
    return true;
}

void PathfindCell::releaseInfo()
{
    if (!hasInfo()) return;
    if (getOpen() || getClosed()) return;
    s_search.parent[m_searchIndex] = navigation::NoSearchIndex;
    if (m_flags != NO_UNITS || m_aircraftGoal) return;
    s_search.release(m_searchIndex);
    m_searchIndex = NO_SEARCH_INDEX;
}

/**
 * Sets the goal unit into the info record for a cell.
 */
void PathfindCell::setGoalUnit(ObjectID unitID, const ICoord2D &pos )
{
	if (unitID==INVALID_ID) {
		// removing goal.
		if (hasInfo()) {
			s_search.goalUnit[m_searchIndex] = INVALID_ID;
			if (s_search.positionUnit[m_searchIndex] == INVALID_ID) {
				// No units here.
				DEBUG_ASSERTCRASH(m_flags==UNIT_GOAL, ("Bad flags."));
				m_flags = NO_UNITS;
				releaseInfo();
			} else{
				m_flags = UNIT_PRESENT_MOVING;
			}
		}	else {
			DEBUG_ASSERTCRASH(m_flags == NO_UNITS, ("Bad flags."));
		}
	} else {
		// adding goal.
		if (!hasInfo()) {
			DEBUG_ASSERTCRASH(m_flags == NO_UNITS, ("Bad flags."));
			allocateInfo(pos);
		}
		if (!hasInfo()) {
			DEBUG_CRASH(("Ran out of pathfind cells - fatal error!!!!! jba."));
			return;
		}
		s_search.goalUnit[m_searchIndex] = unitID;
		if (unitID==s_search.positionUnit[m_searchIndex]) {
			m_flags = UNIT_PRESENT_FIXED;
		} else if (s_search.positionUnit[m_searchIndex]==INVALID_ID) {
			m_flags = UNIT_GOAL;
		}	else {
			m_flags = UNIT_GOAL_OTHER_MOVING;
		}
	}
}


/**
 * Sets the goal aircraft into the info record for a cell.
 */
void PathfindCell::setGoalAircraft(ObjectID unitID, const ICoord2D &pos )
{
	if (unitID==INVALID_ID) {
		// removing goal.
		if (hasInfo()) {
			s_search.goalAircraft[m_searchIndex] = INVALID_ID;
			m_aircraftGoal = false;
			releaseInfo();
		}	else {
			DEBUG_ASSERTCRASH(m_aircraftGoal==false, ("Bad flags."));
		}
	} else {
		// adding goal.
		if (!hasInfo()) {
			DEBUG_ASSERTCRASH(m_aircraftGoal==false, ("Bad flags."));
			allocateInfo(pos);
		}
		if (!hasInfo()) {
			DEBUG_CRASH(("Ran out of pathfind cells - fatal error!!!!! jba."));
			return;
		}
		s_search.goalAircraft[m_searchIndex] = unitID;
		m_aircraftGoal = true;
	}
}


/**
 * Sets the position unit into the info record for a cell.
 */
void PathfindCell::setPosUnit(ObjectID unitID, const ICoord2D &pos )
{
	if (unitID==INVALID_ID) {
		// removing position.
		if (hasInfo()) {
			s_search.positionUnit[m_searchIndex] = INVALID_ID;
			if (s_search.goalUnit[m_searchIndex] == INVALID_ID) {
				// No units here.
				DEBUG_ASSERTCRASH(m_flags==UNIT_PRESENT_MOVING, ("Bad flags."));
				m_flags = NO_UNITS;
				releaseInfo();
			}	else {
				m_flags = UNIT_GOAL;
			}
		}	else {
			DEBUG_ASSERTCRASH(m_flags == NO_UNITS, ("Bad flags."));
		}
	} else {
		// adding goal.
		if (!hasInfo()) {
			DEBUG_ASSERTCRASH(m_flags == NO_UNITS, ("Bad flags."));
			allocateInfo(pos);
		}
		if (!hasInfo()) {
			DEBUG_CRASH(("Ran out of pathfind cells - fatal error!!!!! jba."));
			return;
		}
		if (s_search.goalUnit[m_searchIndex]!=INVALID_ID && (s_search.goalUnit[m_searchIndex]==s_search.positionUnit[m_searchIndex])) {
			// A unit is already occupying this cell.
			return;
		}
		s_search.positionUnit[m_searchIndex] = unitID;
		if (unitID==s_search.goalUnit[m_searchIndex]) {
			m_flags = UNIT_PRESENT_FIXED;
		} else if (s_search.goalUnit[m_searchIndex]==INVALID_ID) {
			m_flags = UNIT_PRESENT_MOVING;
		}	else {
			m_flags = UNIT_GOAL_OTHER_MOVING;
		}
	}
}


/**
 * Return the relevant obstacle ID.
 */
ObjectID PathfindCell::getObstacleID() const
{
	return m_obstacleID;
}


/**
 * Flag this cell as an obstacle, from the given one.
 * Return true if cell was flagged.
 */
Bool PathfindCell::setTypeAsObstacle( Object *obstacle, Bool isFence, const ICoord2D &pos )
{
	if (m_type!=PathfindCell::CELL_CLEAR && m_type != PathfindCell::CELL_IMPASSABLE) {
		return false;
	}

	Bool isRubble = false;
	if (obstacle->getBodyModule() && obstacle->getBodyModule()->getDamageState() == BODY_RUBBLE)
	{
		isRubble = true;
	}

	if (isRubble) {
		m_type = PathfindCell::CELL_RUBBLE;
		m_obstacleID = INVALID_ID;
		m_obstacleIsFence = false;
		m_obstacleIsTransparent = false;
		return true;
	}

	m_type = PathfindCell::CELL_OBSTACLE;
	m_obstacleID = obstacle->getID();
	m_obstacleIsFence = isFence;
	m_obstacleIsTransparent = obstacle->isKindOf(KINDOF_CAN_SEE_THROUGH_STRUCTURE);
	return true;
}

/**
 * Flag this cell as given type.
 */
void PathfindCell::setType( CellType type )
{
	if (m_obstacleID != INVALID_ID) {
		DEBUG_ASSERTCRASH(type == PathfindCell::CELL_OBSTACLE, ("Wrong type."));
		m_type = PathfindCell::CELL_OBSTACLE;
		return;
	}
	m_type = type;
}

/**
 * Unflag this cell as an obstacle, from the given one.
 * Return true if this cell was previously flagged as an obstacle by this object.
 */
Bool PathfindCell::removeObstacle( Object *obstacle )
{
	if (m_type == PathfindCell::CELL_RUBBLE) {
		m_type = PathfindCell::CELL_CLEAR;
	}
	if (m_obstacleID != obstacle->getID()) return false;
	m_type = PathfindCell::CELL_CLEAR;
	m_obstacleID = INVALID_ID;
	m_obstacleIsFence = false;
	m_obstacleIsTransparent = false;
	return true;
}


void PathfindCell::putOnSortedOpenList(PathfindCellList& list)
{
    DEBUG_ASSERTCRASH(list.m_open && hasInfo(), ("Invalid frontier insertion"));
    s_search.pushOpen(m_searchIndex);
}
void PathfindCell::removeFromOpenList(PathfindCellList& list)
{
    DEBUG_ASSERTCRASH(list.m_open && getOpen(), ("Invalid frontier removal"));
    s_search.eraseOpen(m_searchIndex);
}
void PathfindCell::putOnClosedList(PathfindCellList& list)
{
    DEBUG_ASSERTCRASH(!list.m_open && hasInfo(), ("Invalid closed-state insertion"));
    s_search.close(m_searchIndex);
}
void PathfindCell::removeFromClosedList(PathfindCellList& list)
{
    DEBUG_ASSERTCRASH(!list.m_open && getClosed(), ("Invalid closed-state removal"));
    s_search.eraseClosed(m_searchIndex);
}
Int PathfindCell::releaseOpenList(PathfindCellList&)
{
    Int count = 0;
    while (!s_search.open.empty()) {
        const auto id = s_search.drainOpen();
        s_search.owner[id]->releaseInfo();
        ++count;
    }
    return count;
}
Int PathfindCell::releaseClosedList(PathfindCellList&)
{
    Int count = 0;
    while (s_search.closedHead() != navigation::NoSearchIndex) {
        const auto id = s_search.closedHead();
        s_search.eraseClosed(id);
        s_search.owner[id]->releaseInfo();
        ++count;
    }
    return count;
}

/**
 * Return true if the given object ID is registered as an obstacle in this cell
 */
Bool PathfindCell::isObstaclePresent(ObjectID objID) const
{
	if (objID != INVALID_ID && (getType() == PathfindCell::CELL_OBSTACLE))
	{
		return m_obstacleID == objID;
	}

	return false;
}


/**
 * return true if the obstacle in the cell is KINDOF_CAN_SEE_THROUGHT_STRUCTURE
 */
Bool PathfindCell::isObstacleTransparent() const
{
	return m_obstacleIsTransparent;
}

/**
 * return true if the given obstacle in the cell is a fence.
 */
Bool PathfindCell::isObstacleFence() const
{
	return m_obstacleIsFence;
}


const Int COST_ORTHOGONAL = 10;
const Int COST_DIAGONAL = 14;
const Real COST_TO_DISTANCE_FACTOR = 1.0f/10.0f;
const Real COST_TO_DISTANCE_FACTOR_SQR = COST_TO_DISTANCE_FACTOR*COST_TO_DISTANCE_FACTOR;

UnsignedInt PathfindCell::costToGoal( PathfindCell *goal )
{
	DEBUG_ASSERTCRASH(hasInfo(), ("Has to have info."));
	Int dx = s_search.x[m_searchIndex] - goal->getXIndex();
	Int dy = s_search.y[m_searchIndex] - goal->getYIndex();
#define NO_REAL_DIST
#ifdef REAL_DIST
	Int cost = COST_ORTHOGONAL*sqrt(dx*dx + dy*dy);
#else
	Int cost = navigation::estimateGoalCost(dx, dy);

#endif


	return cost;
}

UnsignedInt PathfindCell::costToHierGoal( PathfindCell *goal )
{
	if (!hasInfo())
	{
		DEBUG_CRASH( ("Has to have info.") );
		return 100000; //...patch hack 1.01
	}
	Int dx = s_search.x[m_searchIndex] - goal->getXIndex();
	Int dy = s_search.y[m_searchIndex] - goal->getYIndex();
	Int cost = REAL_TO_INT_FLOOR(COST_ORTHOGONAL*sqrt(dx*dx + dy*dy) + 0.5f);
	return cost;
}

UnsignedInt PathfindCell::costSoFar( PathfindCell *parent )
{
	DEBUG_ASSERTCRASH(hasInfo(), ("Has to have info."));
	// very first node in path - no turns, no cost
	if (parent == nullptr)
		return 0;

	// add in number of turns in path so far
	ICoord2D prevDir;
	Int cost;

	prevDir.x = parent->getXIndex() - s_search.x[m_searchIndex];
	prevDir.y = parent->getYIndex() - s_search.y[m_searchIndex];

	cost = navigation::stepCost(parent->getCostSoFar(), prevDir.x, prevDir.y, getPinched());

#if 1
	// Increase cost of turns.
	Int numTurns = 0;
	PathfindCell *prevCell = parent->getParentCell();
	if (prevCell) {


		ICoord2D dir;
		dir.x = prevCell->getXIndex() - parent->getXIndex();
		dir.y = prevCell->getYIndex() - parent->getYIndex();

		numTurns = navigation::turnCost(dir.x, dir.y, prevDir.x, prevDir.y);
	}

	return cost + numTurns;
#else
	return cost;
#endif

}


