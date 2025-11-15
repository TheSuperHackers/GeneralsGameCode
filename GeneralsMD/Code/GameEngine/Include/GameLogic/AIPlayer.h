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

// AIPlayer.h
// Computerized opponent
// Author: Michael S. Booth, January 2002

#pragma once

#include "Common/GameMemory.h"
#include "Common/Snapshot.h"

enum { INVALID_SKILLSET_SELECTION = -1 };

class BuildListInfo;

/**
 * When a team is selected for training, a list of these
 * "work orders" are created, one for each member of the team.
 * This pairs team members with production buildings to keep
 * track of who is building what, and allows us to track if
 * a building was destroyed while in the process of training a unit.
 */
class WorkOrder : public MemoryPoolObject,
									public Snapshot
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WorkOrder, "WorkOrder" )

public:

	WorkOrder():m_thing(NULL), m_factoryID(INVALID_ID), m_isResourceGatherer(false), m_numCompleted(0), m_numRequired(1), m_next(NULL) {};

	Bool isWaitingToBuild( void );		///< return true if nothing is yet building this unit
	void validateFactory( Player *thisPlayer );			///< verify factoryID still refers to an active object

public:

	const ThingTemplate *m_thing;			///< thing to build
	ObjectID m_factoryID;							///< ID of object that is building this, or zero if no-one is
	WorkOrder *m_next;
	Int			m_numCompleted;					  ///< Number built.
	Int			m_numRequired;					  ///< Number needed.
	Bool		m_required;								///< True if part of minimum requirement.
	Bool		m_isResourceGatherer;			///< True if resource gatherer.

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

};

inline Bool WorkOrder::isWaitingToBuild( void )
{
	if (m_factoryID!=INVALID_ID)
		return false;
	if (m_numCompleted >= m_numRequired)
		return false;
	return true;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TeamInQueue : public MemoryPoolObject,
										public Snapshot
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TeamInQueue, "TeamInQueue"  )

private:

	MAKE_DLINK(TeamInQueue, TeamBuildQueue)				///< the instances of our prototype
	MAKE_DLINK(TeamInQueue, TeamReadyQueue)				///< the instances of our prototype

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

public:

	TeamInQueue() :
		m_workOrders(NULL),
		m_team(NULL),
		m_nextTeamInQueue(NULL),
		m_sentToStartLocation(false),
		m_reinforcement(false),
		m_stopQueueing(false),
		m_reinforcementID(INVALID_ID),
		m_frameStarted(0),
		m_priorityBuild(FALSE)
	{
	}

	Bool isAllBuilt( void );				///< Returns true if the team is finished building.
	Bool isBuildTimeExpired( void );///< Returns true if the team has run out of build time.
	Bool isMinimumBuilt( void );		///< Returns true if the team has started building at least the minimum number of units.
	Bool includesADozer( void );		///< Returns true if the team includes a dozer unit.
	Bool areBuildsComplete( void );	///< Returns true if all units in factories have finished building.
	void disband( void );						///< Disbands the team (moves units into the default team).
	void stopQueueing(void) {m_stopQueueing=true;} ///< Stops building new units, just finishes current.

public:

	WorkOrder *m_workOrders;				///< list of work orders
	Bool m_priorityBuild;						///< True if the team is specifically requested.
	Team *m_team;										///< the team that units built by the m_workOrders go into
	TeamInQueue *m_nextTeamInQueue; ///< next
	Int	m_frameStarted;							///< Frame we started building.
	Bool m_sentToStartLocation;			///< Has it been sent to it's start location?
	Bool m_stopQueueing;						///< True if we are to quit queueing units (usually because we ran out of build time.)
	Bool m_reinforcement;						///< True if it is a unit to reinforce an existing team.
	ObjectID m_reinforcementID;			///< True if it is a unit to reinforce an existing team.

};


/**
 * The computer-controlled opponent.
 */
class AIPlayer : public MemoryPoolObject,
								 public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AIPlayer, "AIPlayer"  )

public:

	AIPlayer( Player *p );							///< constructor

	virtual Bool computeSuperweaponTarget(const SpecialPowerTemplate *power, Coord3D *pos, Int playerNdx, Real weaponRadius); ///< Calculates best pos for weapon given radius.

public: // AIPlayer interface, may be overridden by AISkirmishPlayer.  jba.

	virtual void update();											///< simulates the behavior of a player

	virtual void newMap();											///< New map loaded call.

	/// Invoked when a unit I am training comes into existence
	virtual void onUnitProduced( Object *factory, Object *unit );

	/// Invoked when a structure I am building comes into existence
	virtual void onStructureProduced( Object *factory, Object *structure );

	virtual void buildSpecificAITeam(TeamPrototype *teamProto, Bool priorityBuild); ///< Builds this team immediately.

	virtual void buildAIBaseDefense(Bool flank); ///< Builds base defense on front or flank of base.

	virtual void buildAIBaseDefenseStructure(const AsciiString &thingName, Bool flank); ///< Builds base defense on front or flank of base.

	virtual void buildSpecificAIBuilding(const AsciiString &thingName); ///< Builds this building as soon as possible.


	virtual void recruitSpecificAITeam(TeamPrototype *teamProto, Real recruitRadius); ///< Builds this team immediately.

	virtual Bool isSkirmishAI(void) {return false;}
	virtual Player *getAiEnemy(void) {return NULL;}	///< Solo AI attacks based on scripting.  Only skirmish auto-acquires an enemy at this point.  jba.
	virtual Bool checkBridges(Object *unit, Waypoint *way) {return false;}
	virtual void repairStructure(ObjectID structure);

	virtual void selectSkillset(Int skillset);

public:
	Bool getBaseCenter(Coord3D *pos) const {*pos = m_baseCenter; return m_baseCenterSet;}
	/// Difficulty level for this player.
	GameDifficulty getAIDifficulty(void) const;
	void setAIDifficulty(GameDifficulty difficulty) {m_difficulty = difficulty;}
	void buildBySupplies(Int minimumCash, const AsciiString &thingName ); ///< Builds a building by supplies.
	void buildSpecificBuildingNearestTeam( const AsciiString &thingName, const Team *team );
	void buildUpgrade(const AsciiString &upgrade ); ///< Builds an upgrade.
	/// A team is about to be destroyed.
	void aiPreTeamDestroy( const Team *team );
	/// Is the nearest supply source safe?
 	Bool isSupplySourceSafe( Int minSupplies );
	/// Is a supply source attacked?
	Bool isSupplySourceAttacked( void );

	Bool isLocationSafe( const Coord3D *pos, const ThingTemplate *tthing);

	/// Have the team guard a supply center.
	void guardSupplyCenter( Team *team, Int minSupplies );

	void setTeamDelaySeconds(Int delay) {m_teamSeconds = delay;}

	/// Calculates the closest construction zone location based on a template.
	Bool calcClosestConstructionZoneLocation( const ThingTemplate *constructTemplate, Coord3D *location );

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	virtual void doBaseBuilding(void);
	virtual void checkReadyTeams(void);
	virtual void checkQueuedTeams(void);
	virtual void doTeamBuilding(void);
	virtual void doUpgradesAndSkills(void);
	virtual Object *findDozer(const Coord3D *pos);
	virtual void queueDozer(void);
	virtual Bool selectTeamToBuild( void );			///< determine the next team to build
	virtual Bool selectTeamToReinforce( Int minPriority );			///< determine the next team to reinforce
	virtual Bool startTraining( WorkOrder *order, Bool busyOK, AsciiString teamName);	///< find a production building that can handle the order, and start building
	virtual Bool isAGoodIdeaToBuildTeam( TeamPrototype *proto );		///< return true if team should be built
	virtual void processBaseBuilding( void );		///< do base-building behaviors
	virtual void processTeamBuilding( void );		///< do team-building behaviors
 	static Int getPlayerSuperweaponValue( Coord3D *center, Int playerNdx, Real radius, Bool includeMilitaryUnits = TRUE );
// End of aiplayer interface.

protected:

	MAKE_DLINK_HEAD(TeamInQueue, TeamBuildQueue);		///< List of teams being build
	MAKE_DLINK_HEAD(TeamInQueue, TeamReadyQueue);		///< List of teams built, waiting to reach rally point.

protected:
	Bool isPossibleToBuildTeam( TeamPrototype *proto, Bool requireIdleFactory, Bool &needMoney );		///< return true if team can be considered for building
	Object *buildStructureNow(const ThingTemplate *bldgPlan, BuildListInfo *info );		///< Build a base buiding.
	Object *buildStructureWithDozer(const ThingTemplate *bldgPlan, BuildListInfo *info );		///< Build a base buiding.
	void clearTeamsInQueue( void );			///< Delete all teams in the build queue.
	void computeCenterAndRadiusOfBase(Coord3D *center, Real *radius);
	Object *findFactory(const ThingTemplate *thing, Bool busyOK); ///< Find a factory to build a unit.  If force is true, may return a busy factory.
	void queueUnits( void );						///< Check the team build list, & queue up units at any idle factories.
	void checkForSupplyCenter( BuildListInfo *info, Object *bldg);
 	void queueSupplyTruck(void);
	void updateBridgeRepair(void);
	Bool dozerInQueue(void);
	Object *findSupplyCenter(Int minSupplies);
	static void getPlayerStructureBounds(Region2D *bounds, Int playerNdx, Bool conservative = FALSE );

protected:

	Player *m_player;									///< the Player we represent

	Bool		m_readyToBuildTeam;				///< True if the team select timer has expired.
	Bool		m_readyToBuildStructure;	///< True if the buildDelay timer has expired.
	Int			m_teamTimer;							///< Counts out the time between teams, as specified by ini.
	Int			m_structureTimer;					///< Counts out the time between structures, as specified by ini.
	Int			m_teamSeconds;						///< How many seconds to delay between teams.

	Int			m_buildDelay;							///< Delay for building in case we are resource or prereq. limited.
	Int			m_teamDelay;							///< Delay for teams in case we are resource or factory prereq. limited.

	Int			m_frameLastBuildingBuilt;	///< When we built the last building.

	GameDifficulty m_difficulty;

	Int			m_skillsetSelector;

	Coord3D m_baseCenter; // Center of the initial build list of structures.
	Bool		m_baseCenterSet; // True if baseCenter is valid.
	Real m_baseRadius; // Radius of the initial build list of structures.

	// Bridge repair info.
	enum {MAX_STRUCTURES_TO_REPAIR = 2};
	ObjectID m_structuresToRepair[MAX_STRUCTURES_TO_REPAIR];
	ObjectID m_repairDozer;
	Coord3D  m_repairDozerOrigin;
	Int			 m_structuresInQueue;
	Bool		 m_dozerQueuedForRepair;
	Bool		 m_dozerIsRepairing;			///< the repair dozer is trying to repair the bridge.
	Int			 m_bridgeTimer;

	UnsignedInt	m_supplySourceAttackCheckFrame;
	ObjectID m_attackedSupplyCenter;

	ObjectID m_curWarehouseID;
};
