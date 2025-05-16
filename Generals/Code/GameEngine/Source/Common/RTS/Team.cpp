/*
**	Command & Conquer Generals(tm)
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

// FILE: Team.cpp /////////////////////////////////////////////////////////////////////////////////
// Team interface implementation
// Author: Michael S. Booth, March 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/GameState.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/PerfTimer.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/ThingTemplate.h"
#include "Common/WellKnownKeys.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"

#include "GameLogic/SidesList.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptActions.h"
#include "GameLogic/ScriptEngine.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

///@todo - do delayed script evaluations for team scripts. jba.

// GLOBALS ////////////////////////////////////////////////////////////////////
TeamFactory *TheTeamFactory = NULL;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
TeamRelationMap::TeamRelationMap( void )
{

}  // end TeamRelationMap

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
TeamRelationMap::~TeamRelationMap( void )
{

	// maek sure the data is clear
	m_map.clear();

}  // end ~TeamRelationMap

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TeamRelationMap::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method 
	* Version Info;
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void TeamRelationMap::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// team relation count
	TeamRelationMapType::iterator teamRelationIt;
	UnsignedShort teamRelationCount = m_map.size();
	xfer->xferUnsignedShort( &teamRelationCount );

	// team relations
	TeamID teamID;
	Relationship r;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// go through all team relations
		for( teamRelationIt = m_map.begin(); teamRelationIt != m_map.end(); ++teamRelationIt )
		{

			// write team ID
			teamID = (*teamRelationIt).first;
			xfer->xferUser( &teamID, sizeof( TeamID ) );
			
			// write relationship
			r = (*teamRelationIt).second;
			xfer->xferUser( &r, sizeof( Relationship ) );

		}  // end for

	}  // end if, save
	else
	{

		for( UnsignedShort i = 0; i < teamRelationCount; ++i )
		{

			// read team ID
			xfer->xferUser( &teamID, sizeof( TeamID ) );

			// read relationship
			xfer->xferUser( &r, sizeof( Relationship ) );

			// assign relationship
			m_map[teamID] = r;
			
		}  // end for, i

	}  // end else load

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TeamRelationMap::loadPostProcess( void )
{

}  // end loadPostProcess

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// STATIC FUNCTIONS ///////////////////////////////////////////////////////////
static Bool locoSetMatches(LocomotorSurfaceTypeMask lstm, UnsignedInt surfaceBitFlags)
{
	surfaceBitFlags = surfaceBitFlags & 0x01 | ((surfaceBitFlags & 0x02) << 2);
	return (surfaceBitFlags & lstm) != 0;
}

// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
TeamFactory::TeamFactory()
{

	m_uniqueTeamPrototypeID = TEAM_PROTOTYPE_ID_INVALID;
	m_uniqueTeamID = TEAM_ID_INVALID;

}

// ------------------------------------------------------------------------
TeamFactory::~TeamFactory()
{
	clear();
}

// ------------------------------------------------------------------------
void TeamFactory::init( void )
{
	clear();
}

// ------------------------------------------------------------------------
void TeamFactory::reset( void )
{
	m_uniqueTeamPrototypeID = TEAM_PROTOTYPE_ID_INVALID;
	m_uniqueTeamID = TEAM_ID_INVALID;
	clear();
}

// ------------------------------------------------------------------------
void TeamFactory::update( void )
{
}

// ------------------------------------------------------------------------
void TeamFactory::clear()
{
		// must remove it from the map before deleting the TeamProto, since
		// the TeamProto will try to remove itself from the list when it goes away
	TeamPrototypeMap tmp = m_prototypes;
	m_prototypes.clear();
	for (TeamPrototypeMap::iterator it = tmp.begin(); it != tmp.end(); ++it)
	{
		deleteInstance(it->second);
	}
}

// ------------------------------------------------------------------------
void TeamFactory::initFromSides(SidesList *sides)
{
	clear();

	// create the teams we need.
	for(Int i = 0; i < sides->getNumTeams(); i++)
	{
		Dict *d = sides->getTeamInfo(i)->getDict();

		AsciiString tname = d->getAsciiString(TheKey_teamName);
		AsciiString oname = d->getAsciiString(TheKey_teamOwner);
		Bool singleton = d->getBool(TheKey_teamIsSingleton);
		initTeam(tname, oname, singleton, d);
	}

}

// ------------------------------------------------------------------------
void TeamFactory::initTeam(const AsciiString& name, const AsciiString& owner, Bool isSingleton, Dict *d)
{
	DEBUG_ASSERTCRASH(findTeamPrototype(name)==NULL,("team already exists"));
	Player *pOwner = ThePlayerList->findPlayerWithNameKey(NAMEKEY(owner));
	DEBUG_ASSERTCRASH(pOwner, ("no owner found for team %s (%s)\n",name.str(),owner.str()));
	if (!pOwner)
		pOwner = ThePlayerList->getNeutralPlayer(); 
	/*TeamPrototype *tp =*/ newInstance(TeamPrototype)(this, name, pOwner, isSingleton, d, ++m_uniqueTeamPrototypeID);
	if (isSingleton) {
		// Create the singleton team.
		createInactiveTeam(name);
	}
}

//=============================================================================
void TeamFactory::addTeamPrototypeToList(TeamPrototype* team)
{
	NameKeyType nk = NAMEKEY(team->getName());
	TeamPrototypeMap::iterator it = m_prototypes.find(nk);
	if (it != m_prototypes.end())
	{
		DEBUG_ASSERTCRASH((*it).second==team, ("TeamFactory::addTeamPrototypeToList: Team %s already exists... skipping.", team->getName().str()));
		return;	// already present
	}

	m_prototypes[nk] = team;
}

//=============================================================================
void TeamFactory::removeTeamPrototypeFromList(TeamPrototype* team)
{
	NameKeyType nk = NAMEKEY(team->getName());
	TeamPrototypeMap::iterator it = m_prototypes.find(nk);
	if (it != m_prototypes.end())
		m_prototypes.erase(it);
}

// ------------------------------------------------------------------------
TeamPrototype *TeamFactory::findTeamPrototype(const AsciiString& name)
{
	NameKeyType nk = NAMEKEY(name);
	TeamPrototypeMap::iterator it = m_prototypes.find(nk);
	if (it != m_prototypes.end())
		return it->second;

	return NULL;
}

// ------------------------------------------------------------------------
TeamPrototype *TeamFactory::findTeamPrototypeByID( TeamPrototypeID id )
{
	TeamPrototypeMap::iterator it;
	TeamPrototype *prototype = NULL;

	for( it = m_prototypes.begin(); it != m_prototypes.end(); ++it )
	{
	
		prototype = it->second;
		if( prototype->getID() == id )
			return prototype;

	}  // end for
	
	// not found	
	return NULL;

}

// ------------------------------------------------------------------------
Team *TeamFactory::findTeamByID( TeamID teamID )
{

	// simple case
	if( teamID == TEAM_ID_INVALID )
		return NULL;

	// search all prototypes for the matching team ID
	TeamPrototype *tp;
	TeamPrototypeMap::iterator it;
	Team *team;
	for( it = m_prototypes.begin(); it != m_prototypes.end(); ++it )
	{
		tp = (*it).second;
		team = tp->findTeamByID( teamID );
		if( team )
			return team;
	}

	return NULL;

}

// ------------------------------------------------------------------------
/** Creates an inactive team, suitable for adding members to as they are built.
Call team->setActive() when all members are added. */
Team *TeamFactory::createInactiveTeam(const AsciiString& name)
{
	TeamPrototype *tp = findTeamPrototype(name);
	if (!tp)
		throw ERROR_BAD_ARG;
	
	Team *t = NULL;
	if (tp->getIsSingleton())
	{
		t = tp->getFirstItemIn_TeamInstanceList();
		if (t) {
			if (tp->getTemplateInfo()->m_executeActions) {
				const Script *script = TheScriptEngine->findScriptByName(tp->getTemplateInfo()->m_productionCondition);
				if (script) {
					TheScriptEngine->friend_executeAction(script->getAction());
				}
			}
			return t;
		}
	}

	t = newInstance(Team)(tp, ++m_uniqueTeamID );
	if (tp->getTemplateInfo()->m_executeActions) {
		const Script *script = TheScriptEngine->findScriptByName(tp->getTemplateInfo()->m_productionCondition);
		if (script) {
			TheScriptEngine->friend_executeAction(script->getAction());
		}
	}
	
	return t;
}

// ------------------------------------------------------------------------
Team *TeamFactory::createTeam(const AsciiString& name)
{
	Team *t = NULL;
	t = createInactiveTeam(name);
	t->setActive();
	return t;
}

// ------------------------------------------------------------------------
Team *TeamFactory::createTeamOnPrototype( TeamPrototype *prototype )
{
	if( prototype == NULL )
		throw ERROR_BAD_ARG;

	Team *t = NULL;
	if( prototype->getIsSingleton() )
	{
		t = prototype->getFirstItemIn_TeamInstanceList();
		if( t )
			return t;
	}
	t = newInstance(Team)( prototype, ++m_uniqueTeamID );
	t->setActive();
	return t;
}
// ------------------------------------------------------------------------
Team* TeamFactory::findTeam(const AsciiString& name)
{
	TeamPrototype *tp = findTeamPrototype(name);
	if (tp)
	{
		Team *t = tp->getFirstItemIn_TeamInstanceList();
		if (t == NULL && !tp->getIsSingleton()) 
		{
			t = createInactiveTeam(name);
		}
		return t;
	}
	return NULL;
}

// ------------------------------------------------------------------------
void TeamFactory::teamAboutToBeDeleted(Team* team)
{
	for (TeamPrototypeMap::iterator it = m_prototypes.begin(); it != m_prototypes.end(); ++it)
	{
		it->second->teamAboutToBeDeleted(team);
	}
	if (ThePlayerList)
		ThePlayerList->teamAboutToBeDeleted(team);
}

// ------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------
void TeamFactory::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------
void TeamFactory::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// unique team ID counter
	xfer->xferUser( &m_uniqueTeamID, sizeof( TeamID ) );

	// how many team prototypes of data do we have to write
	UnsignedShort prototypeCount = m_prototypes.size();
	xfer->xferUnsignedShort( &prototypeCount );

	//
	// prototypes cannot change in number during run time so the count should be the
	// same as that already loaded into us from a map load
	//
	if( prototypeCount != m_prototypes.size() )
	{

		DEBUG_CRASH(( "TeamFactory::xfer - Prototype count mismatch '%d should be '%d'\n",
									prototypeCount, m_prototypes.size() ));
		throw SC_INVALID_DATA;

	}  // end if

	// xfer each of the prototype information
	TeamPrototypeMap::iterator it;
	TeamPrototypeID teamPrototypeID;
	TeamPrototype *teamPrototype;
	AsciiString prototypeName;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// iterate each prototype and xfer if it needs to be in the save file
		for( it = m_prototypes.begin(); it != m_prototypes.end(); ++it )
		{

			// get prototype
			teamPrototype = it->second;

			// xfer prototype id
			teamPrototypeID = teamPrototype->getID();
			xfer->xferUser( &teamPrototypeID, sizeof( TeamPrototypeID ) );

			// xfer prototype data
			xfer->xferSnapshot( teamPrototype );
		
		}  //end for, it

	}  // end if, saving
	else
	{

		// read all the team prototype info
		for( UnsignedShort i = 0; i < prototypeCount; ++i )
		{

			// read prototype ID
			xfer->xferUser( &teamPrototypeID, sizeof( TeamPrototypeID ) );

			// find the prototype
			teamPrototype = findTeamPrototypeByID( teamPrototypeID );

			// sanity
			if( teamPrototype == NULL )
			{

				DEBUG_CRASH(( "TeamFactory::xfer - Unable to find team prototype by id\n" ));
				throw SC_INVALID_DATA;

			}  // end if

			// xfer prototype data
			xfer->xferSnapshot( teamPrototype );

		}  // end for, i

	}  // end else, loading

/*
// SAVE_LOAD_DEBUG 
if( xfer->getXferMode() == XFER_SAVE )
{

FILE *fp = fopen( "TeamCheckSave.txt", "w+t" );
if( fp == NULL )
	return;

Object *obj;
TeamPrototypeMap::iterator prototypeIt;
TeamPrototype *prototype;
Team *team;
for( prototypeIt = m_prototypes.begin(); prototypeIt != m_prototypes.end(); ++prototypeIt )
{
	prototype = prototypeIt->second;
fprintf( fp, "Prototype '%s' for player index '%d'\n", prototype->getName().str(), prototype->getControllingPlayer()->getPlayerIndex() );
	for( DLINK_ITERATOR<Team> teamIt = prototype->iterate_TeamInstanceList(); !teamIt.done(); teamIt.advance() ) 
	{
		team = teamIt.cur();
fprintf( fp, "  Team Instance '%s', id is '%d'\n", team->getName().str(), team->getID() );
		for( DLINK_ITERATOR<Object> objIt = team->iterate_TeamMemberList(); !objIt.done(); objIt.advance() )
		{
			obj = objIt.cur();
fprintf( fp, "    Member '%s', id '%d'\n", obj->getTemplate()->getName().str(), obj->getID() );
		}
	}  // end for

}  // end for
fclose( fp );

}  // end if, save
*/

}  // end xfer

// ------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------
void TeamFactory::loadPostProcess( void )
{

	// set the next unique team and prototype ID to just over the highest one in use
	m_uniqueTeamID = 0;
	m_uniqueTeamPrototypeID = 0;
	TeamPrototypeMap::iterator prototypeIt;
	TeamPrototype *prototype;
	Team *team;
	for( prototypeIt = m_prototypes.begin(); prototypeIt != m_prototypes.end(); ++prototypeIt )
	{

		// get prototype
		prototype = prototypeIt->second;

		// do protype ID check
		if( prototype->getID() >= m_uniqueTeamPrototypeID )
			m_uniqueTeamPrototypeID = prototype->getID() + 1;

		// iterate team instances on each prototype and do the team ID check
		for( DLINK_ITERATOR<Team> iter = prototype->iterate_TeamInstanceList(); !iter.done(); iter.advance() ) 
		{

			team = iter.cur();
			if( team->getID() >= m_uniqueTeamID )
				m_uniqueTeamID = team->getID() + 1;

		}  // end for

	}  // end for, it

/*
// SAVE_LOAD_DEBUG
FILE *fp = fopen( "TeamCheckLoad.txt", "w+t" );
if( fp == NULL )
	return;

Object *obj;
for( prototypeIt = m_prototypes.begin(); prototypeIt != m_prototypes.end(); ++prototypeIt )
{
	prototype = prototypeIt->second;
fprintf( fp, "Prototype '%s' for player index '%d'\n", prototype->getName().str(), prototype->getControllingPlayer()->getPlayerIndex() );
	for( DLINK_ITERATOR<Team> teamIt = prototype->iterate_TeamInstanceList(); !teamIt.done(); teamIt.advance() ) 
	{
		team = teamIt.cur();
fprintf( fp, "  Team Instance '%s', id is '%d'\n", team->getName().str(), team->getID() );
		team->reverse_TeamMemberList();
		for( DLINK_ITERATOR<Object> objIt = team->iterate_TeamMemberList(); !objIt.done(); objIt.advance() )
		{
			obj = objIt.cur();
fprintf( fp, "    Member '%s', id '%d'\n", obj->getTemplate()->getName().str(), obj->getID() );
		}
	}  // end for

}  // end for
fclose( fp );
*/

}  // end loadPostProcess

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
TeamTemplateInfo::TeamTemplateInfo(Dict *d) :
	m_numUnitsInfo(0)
{
	Bool exists;
	Int min, max;
	AsciiString templateName;
	min = d->getInt(TheKey_teamUnitMinCount1, &exists);
	max = d->getInt(TheKey_teamUnitMaxCount1, &exists);
	templateName = d->getAsciiString(TheKey_teamUnitType1, &exists);
	if (max>0 && exists) {
		m_unitsInfo[m_numUnitsInfo].minUnits = min;
		m_unitsInfo[m_numUnitsInfo].maxUnits = max;
		m_unitsInfo[m_numUnitsInfo].unitThingName = templateName;
		m_numUnitsInfo++;
	}

	min = d->getInt(TheKey_teamUnitMinCount2, &exists);
	max = d->getInt(TheKey_teamUnitMaxCount2, &exists);
	templateName = d->getAsciiString(TheKey_teamUnitType2, &exists);
	if (max>0 && exists) {
		m_unitsInfo[m_numUnitsInfo].minUnits = min;
		m_unitsInfo[m_numUnitsInfo].maxUnits = max;
		m_unitsInfo[m_numUnitsInfo].unitThingName = templateName;
		m_numUnitsInfo++;
	}

	min = d->getInt(TheKey_teamUnitMinCount3, &exists);
	max = d->getInt(TheKey_teamUnitMaxCount3, &exists);
	templateName = d->getAsciiString(TheKey_teamUnitType3, &exists);
	if (max>0 && exists) {
		m_unitsInfo[m_numUnitsInfo].minUnits = min;
		m_unitsInfo[m_numUnitsInfo].maxUnits = max;
		m_unitsInfo[m_numUnitsInfo].unitThingName = templateName;
		m_numUnitsInfo++;
	}

	min = d->getInt(TheKey_teamUnitMinCount4, &exists);
	max = d->getInt(TheKey_teamUnitMaxCount4, &exists);
	templateName = d->getAsciiString(TheKey_teamUnitType4, &exists);
	if (max>0 && exists) {
		m_unitsInfo[m_numUnitsInfo].minUnits = min;
		m_unitsInfo[m_numUnitsInfo].maxUnits = max;
		m_unitsInfo[m_numUnitsInfo].unitThingName = templateName;
		m_numUnitsInfo++;
	}

	min = d->getInt(TheKey_teamUnitMinCount5, &exists);
	max = d->getInt(TheKey_teamUnitMaxCount5, &exists);
	templateName = d->getAsciiString(TheKey_teamUnitType5, &exists);
	if (max>0 && exists) {
		m_unitsInfo[m_numUnitsInfo].minUnits = min;
		m_unitsInfo[m_numUnitsInfo].maxUnits = max;
		m_unitsInfo[m_numUnitsInfo].unitThingName = templateName;
		m_numUnitsInfo++;
	}

	min = d->getInt(TheKey_teamUnitMinCount6, &exists);
	max = d->getInt(TheKey_teamUnitMaxCount6, &exists);
	templateName = d->getAsciiString(TheKey_teamUnitType6, &exists);
	if (max>0 && exists) {
		m_unitsInfo[m_numUnitsInfo].minUnits = min;
		m_unitsInfo[m_numUnitsInfo].maxUnits = max;
		m_unitsInfo[m_numUnitsInfo].unitThingName = templateName;
		m_numUnitsInfo++;
	}

	min = d->getInt(TheKey_teamUnitMinCount7, &exists);
	max = d->getInt(TheKey_teamUnitMaxCount7, &exists);
	templateName = d->getAsciiString(TheKey_teamUnitType7, &exists);
	if (max>0 && exists) {
		m_unitsInfo[m_numUnitsInfo].minUnits = min;
		m_unitsInfo[m_numUnitsInfo].maxUnits = max;
		m_unitsInfo[m_numUnitsInfo].unitThingName = templateName;
		m_numUnitsInfo++;
	}

	AsciiString waypoint = d->getAsciiString(TheKey_teamHome, &exists);
	m_homeLocation.x = m_homeLocation.y = 0;
	m_homeLocation.z = 0;
	m_hasHomeLocation = false;
	if (exists) {
		for (Waypoint *way = TheTerrainLogic->getFirstWaypoint(); way; way = way->getNext()) {
			if (way->getName() == waypoint) {
				m_homeLocation = *way->getLocation();
				m_hasHomeLocation = true;
			}
		}
	}

	m_scriptOnCreate = d->getAsciiString(TheKey_teamOnCreateScript, &exists);
	m_isAIRecruitable	= d->getBool(TheKey_teamIsAIRecruitable, &exists);
	if (!exists) {
		m_isAIRecruitable = false;
	}
	m_isBaseDefense	= d->getBool(TheKey_teamIsBaseDefense, &exists);
	m_isPerimeterDefense	= d->getBool(TheKey_teamIsPerimeterDefense, &exists);
	m_automaticallyReinforce = d->getBool(TheKey_teamAutoReinforce, &exists);

	Int interact	= d->getInt(TheKey_teamAggressiveness, &exists);
	m_initialTeamAttitude = ATTITUDE_NORMAL;
	if (exists) {
		m_initialTeamAttitude = (AttitudeType) interact;
	}

	m_transportsReturn	= d->getBool(TheKey_teamTransportsReturn, &exists);
	m_avoidThreats = d->getBool(TheKey_teamAvoidThreats, &exists);

	m_attackCommonTarget = d->getBool(TheKey_teamAttackCommonTarget, &exists);

	m_maxInstances = d->getInt(TheKey_teamMaxInstances, &exists);

	m_scriptOnIdle = d->getAsciiString(TheKey_teamOnIdleScript, &exists);
	m_initialIdleFrames = d->getInt(TheKey_teamInitialIdleFrames, &exists);
	m_scriptOnEnemySighted = d->getAsciiString(TheKey_teamEnemySightedScript, &exists);
	m_scriptOnAllClear = d->getAsciiString(TheKey_teamAllClearScript, &exists);
	m_scriptOnDestroyed = d->getAsciiString(TheKey_teamOnDestroyedScript, &exists);
	m_destroyedThreshold = d->getReal(TheKey_teamDestroyedThreshold, &exists);
	m_scriptOnUnitDestroyed = d->getAsciiString(TheKey_teamOnUnitDestroyedScript, &exists);

	m_productionPriority = d->getInt(TheKey_teamProductionPriority, &exists);
	m_productionPrioritySuccessIncrease = d->getInt(TheKey_teamProductionPrioritySuccessIncrease, &exists);
	m_productionPriorityFailureDecrease = d->getInt(TheKey_teamProductionPriorityFailureDecrease, &exists);

	// Production scripts stuff
	m_productionCondition = d->getAsciiString(TheKey_teamProductionCondition, &exists);
	m_executeActions = d->getBool(TheKey_teamExecutesActionsOnCreate, &exists);

	
	// Which scripts to attempt during run?
	for (int i = 0; i < MAX_GENERIC_SCRIPTS; ++i) {
		AsciiString keyName;
		keyName.format("%s%d", TheNameKeyGenerator->keyToName(TheKey_teamGenericScriptHook).str(), i);			
		m_teamGenericScripts[i] = d->getAsciiString(NAMEKEY(keyName), &exists);
		if (!exists) {
			m_teamGenericScripts[i].clear();
		}
	}


	// reinforcement team info.
	m_transportUnitType = d->getAsciiString(TheKey_teamTransport, &exists);
	m_transportsExit = d->getBool(TheKey_teamTransportsExit, &exists);
	m_teamStartsFull = d->getBool(TheKey_teamStartsFull, &exists);
	m_startReinforceWaypoint = d->getAsciiString(TheKey_teamReinforcementOrigin, &exists);
	m_veterancy = (VeterancyLevel)d->getInt(TheKey_teamVeterancy, &exists);
}

// ------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------
void TeamTemplateInfo::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------
void TeamTemplateInfo::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// xfer the production priority
	xfer->xferInt( &m_productionPriority );

}  // end xfer

// ------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------
void TeamTemplateInfo::loadPostProcess( void )
{

}  // end loadPostProcess

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
TeamPrototype::TeamPrototype( TeamFactory *tf, 
															const AsciiString &name, 
															Player *ownerPlayer, 
															Bool isSingleton, 
															Dict *d, 
															TeamPrototypeID id ) : 
	m_id(id),
	m_factory(tf), 
	m_name(name), 
	m_owningPlayer(ownerPlayer), 
	m_flags(isSingleton ? TeamPrototype::TEAM_SINGLETON : 0),
	m_teamTemplate(d), 
	m_productionConditionAlwaysFalse(false),
	m_productionConditionScript(NULL)
{
	DEBUG_ASSERTCRASH(!(m_owningPlayer == NULL), ("bad args to TeamPrototype ctor"));
	if (m_factory)
		m_factory->addTeamPrototypeToList(this);

	if (m_owningPlayer)
		m_owningPlayer->addTeamToList(this);
	
	m_retrievedGenericScripts = false;
	for (Int i = 0; i < MAX_GENERIC_SCRIPTS; ++i) {
		m_genericScriptsToRun[i] = NULL;
	}
}

// ------------------------------------------------------------------------

	static void deleteTeamCallback(Team* o)
	{
		if (o)
		{
			TheTeamFactory->teamAboutToBeDeleted(o);
			deleteInstance(o);
		}
	}

TeamPrototype::~TeamPrototype()
{
	removeAll_TeamInstanceList(deleteTeamCallback);

	if (m_owningPlayer)
		m_owningPlayer->removeTeamFromList(this);

	if (m_factory)
		m_factory->removeTeamPrototypeFromList(this);

	if (m_productionConditionScript) 
	{
		deleteInstance(m_productionConditionScript);
	}
	m_productionConditionScript = NULL;

	for (Int i = 0; i < MAX_GENERIC_SCRIPTS; ++i) 
	{
		if (m_genericScriptsToRun[i]) 
		{
			deleteInstance(m_genericScriptsToRun[i]);
			m_genericScriptsToRun[i] = NULL;
		}
	}
}

// ------------------------------------------------------------------------
Player *TeamPrototype::getControllingPlayer() const
{
	return m_owningPlayer;
}

// ------------------------------------------------------------------------
Team *TeamPrototype::findTeamByID( TeamID teamID )
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		if( iter.cur()->getID() == teamID )
			return iter.cur();
	}
	return NULL;
}

// ------------------------------------------------------------------------
void TeamPrototype::setControllingPlayer(Player *newController) 
{
	DEBUG_ASSERTCRASH(newController, ("Attempted to set NULL player as team-owner, illegal."));
	if (!newController) {
		return;
	}

	if (m_owningPlayer)
		m_owningPlayer->removeTeamFromList(this);

	m_owningPlayer = newController;

	// impossible to get here with a NULL pointer.
	m_owningPlayer->addTeamToList(this);
}

// ------------------------------------------------------------------------
void TeamPrototype::countObjectsByThingTemplate(Int numTmplates, const ThingTemplate* const* things, Bool ignoreDead, Int *counts, Bool ignoreUnderConstruction) const
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		iter.cur()->countObjectsByThingTemplate(numTmplates, things, ignoreDead, counts, ignoreUnderConstruction);
	}
}

// ------------------------------------------------------------------------
void TeamPrototype::teamAboutToBeDeleted(Team* team)
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
//		iter.cur()->removeOverrideTeamRelationship(team);
		iter.cur()->removeOverrideTeamRelationship(team ? team->getID() : TEAM_ID_INVALID );
	}
}

// ------------------------------------------------------------------------
Script *TeamPrototype::getGenericScript(Int scriptToRetrieve)
{
	if (!m_retrievedGenericScripts) {
		m_retrievedGenericScripts = TRUE;	// set this to true so we won't do the lookup again.
		// Go get them from the script engine, and duplicate each one.
		for (Int i = 0; i < MAX_GENERIC_SCRIPTS; ++i) {
			const Script *tmpScript = NULL;
			Script *scriptToSave = NULL;
			if (!m_teamTemplate.m_teamGenericScripts[i].isEmpty()) {
				tmpScript = TheScriptEngine->findScriptByName(m_teamTemplate.m_teamGenericScripts[i]);
				if (tmpScript) {
					scriptToSave = tmpScript->duplicate();
				} else {
					DEBUG_CRASH(("We attempted to find a generic script, but couldn't. ('%s')", m_teamTemplate.m_teamGenericScripts[i].str()));
				}
			}

			// I now own this one. I'm responsible for cleaning it up on destruction.
			m_genericScriptsToRun[i] = scriptToSave;
		}
	}

	return m_genericScriptsToRun[scriptToRetrieve];
}

// ------------------------------------------------------------------------
// Make a team more likely to be selected by the ai for building due to success.
void TeamPrototype::increaseAIPriorityForSuccess(void) const
{
	m_teamTemplate.m_productionPriority += m_teamTemplate.m_productionPrioritySuccessIncrease;
}

// ------------------------------------------------------------------------
// Make a team more likely to be selected by the ai for building due to success.
void TeamPrototype::decreaseAIPriorityForFailure(void) const
{
	m_teamTemplate.m_productionPriority -= m_teamTemplate.m_productionPriorityFailureDecrease;
}

// ------------------------------------------------------------------------
Int TeamPrototype::countBuildings(void)
{
	int retVal = 0;
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
		retVal += iter.cur()->countBuildings();
	}
	return retVal;
}

// ------------------------------------------------------------------------
Int TeamPrototype::countObjects(KindOfMaskType setMask, KindOfMaskType clearMask)
{
	int retVal = 0;
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
		retVal += iter.cur()->countObjects(setMask, clearMask);
	}
	return retVal;
}

// ------------------------------------------------------------------------
void TeamPrototype::healAllObjects(void)
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance()) 
	{
		iter.cur()->healAllObjects();
	}
}

// ------------------------------------------------------------------------
void TeamPrototype::iterateObjects( ObjectIterateFunc func, void *userData )
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance()) 
	{
		iter.cur()->iterateObjects( func, userData );
	}
}

// ------------------------------------------------------------------------
/**
 * Count the number of teams that have been instanced by this prototype
 */
Int TeamPrototype::countTeamInstances( void )
{
	Int count = 0;
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
		count++;

	return count;
}


// ------------------------------------------------------------------------
Bool TeamPrototype::hasAnyBuildings() const
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->hasAnyBuildings()) return true;
	}
	return false;
}

// ------------------------------------------------------------------------
Bool TeamPrototype::hasAnyBuildings(KindOfMaskType kindOf) const
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->hasAnyBuildings(kindOf)) return true;
	}
	return false;
}

// ------------------------------------------------------------------------
Bool TeamPrototype::hasAnyUnits() const
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->hasAnyUnits()) return true;
	}
	return false;
}

// ------------------------------------------------------------------------
Bool TeamPrototype::hasAnyObjects() const
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->hasAnyObjects()) return true;
	}
	return false;
}

// ------------------------------------------------------------------------
void TeamPrototype::updateState(void) 
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		iter.cur()->updateState();
	}
	/* remove empty teams. */
	Bool done = false;
	while (!done) {
		done = true;
		for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
		{	
			if (iter.cur()->getFirstItemIn_TeamMemberList() == NULL) 
			{ 
				// Team has no members.
				if (this->getIsSingleton())	
				{
					continue; // Don't delete singleton teams, even if they are empty.
				}

				if (iter.cur()->getControllingPlayer() && iter.cur()->getControllingPlayer()->getDefaultTeam() == iter.cur()) 
				{
					// This is the player's default team, so don't remove it.
					continue;
				}

				// don't delete inactive teams - they are under construction
				if (iter.cur()->isActive() == false)
				{
					continue;
				}

				// So remove it
				TheTeamFactory->teamAboutToBeDeleted(iter.cur());
				deleteInstance(iter.cur());

				done = false;
				break; // Not sure what state the iterator is in after deleting a member of the list. jba
			}
		}
	}
}

// ------------------------------------------------------------------------
Bool TeamPrototype::hasAnyBuildFacility() const
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->hasAnyBuildFacility()) return true;
	}
	return false;
}

// ------------------------------------------------------------------------
void TeamPrototype::damageTeamMembers(Real amount)
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		iter.cur()->damageTeamMembers(amount);
	}
}


// ------------------------------------------------------------------------
void TeamPrototype::moveTeamTo(Coord3D destination)
{
	for (DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance())
	{
		iter.cur()->moveTeamTo(destination);
	}
}

// ------------------------------------------------------------------------
Bool TeamPrototype::evaluateProductionCondition(void)
{
	if (m_productionConditionAlwaysFalse) {
		// Set if we don't have a script.
		return false;
	}
	if (m_productionConditionScript) {
		// If we are doing peridic evaluation, check the frame.
		if (TheGameLogic->getFrame()<m_productionConditionScript->getFrameToEvaluate()) {
			return false;
		}
		Int delaySeconds = m_productionConditionScript->getDelayEvalSeconds();

		if (delaySeconds>0) {
			m_productionConditionScript->setFrameToEvaluate(TheGameLogic->getFrame()+delaySeconds*LOGICFRAMES_PER_SECOND);
		}
		return TheScriptEngine->evaluateConditions(m_productionConditionScript, NULL, getControllingPlayer());
	}
	// We don't have a script yet, so check for one.
	if (m_teamTemplate.m_productionCondition.isEmpty()) {
		// No script defined.
		m_productionConditionAlwaysFalse = true;
		return false;
	}
	const Script *pScript = TheScriptEngine->findScriptByName(m_teamTemplate.m_productionCondition);
	if (pScript) {
		// Check difficulty.
		switch (getControllingPlayer()->getPlayerDifficulty() ) {
			case DIFFICULTY_EASY: 
				if (!pScript->isEasy()) {
					m_productionConditionAlwaysFalse = true;
					return false;
				}
				break;
			case DIFFICULTY_NORMAL:
				if (!pScript->isNormal()) {
					m_productionConditionAlwaysFalse = true;
					return false;
				}
				break;
			case DIFFICULTY_HARD:
				if (!pScript->isHard()) {
					m_productionConditionAlwaysFalse = true;
					return false;
				}
				break;
		}

		// Make a copy of the script locally, just for paranoia's sake.  We can't be sure
		// exactly what order the teams & scripts will get reset, so be safe.
		m_productionConditionScript = pScript->duplicate();
		return TheScriptEngine->evaluateConditions(m_productionConditionScript, NULL, getControllingPlayer());
	}
	// Couldn't find a script.
	m_productionConditionAlwaysFalse = true;
	return false;
}

// ------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------
void TeamPrototype::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------
void TeamPrototype::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// owning player index
	Int owningPlayerIndex;
	if( xfer->getXferMode() == XFER_SAVE )
		owningPlayerIndex = m_owningPlayer->getPlayerIndex();
	xfer->xferInt( &owningPlayerIndex );
	m_owningPlayer = ThePlayerList->getNthPlayer( owningPlayerIndex );

	if (version>=2) {
		xfer->xferAsciiString(&m_attackPriorityName);
	}

	// production condition
	xfer->xferBool( &m_productionConditionAlwaysFalse );

	// team template information
	xfer->xferSnapshot( &m_teamTemplate );

	// xfer team instance count
	UnsignedShort teamInstanceCount = 0;
	for( DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance() ) 
		teamInstanceCount++;
	xfer->xferUnsignedShort( &teamInstanceCount );

	// xfer team instances
	Team *teamInstance;
	TeamID teamID;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// xfer each team instance
		for( DLINK_ITERATOR<Team> iter = iterate_TeamInstanceList(); !iter.done(); iter.advance() ) 
		{

			// get the team
			teamInstance = iter.cur();

			// write team id
			teamID = teamInstance->getID();
			xfer->xferUser( &teamID, sizeof( TeamID ) );

			// write team data
			xfer->xferSnapshot( teamInstance );

		}  // end for

	}  // end if, save
	else
	{

		//
		// there is no need to check to make sure the instance list is empty here ... see
		// the large bock comment below where we find a team given an id
		//

		// read each block
		for( UnsignedShort i = 0; i < teamInstanceCount; ++i )
		{

			// read team id
			xfer->xferUser( &teamID, sizeof( TeamID ) );

			//
			// find this team, if it's not there create it ... note that there will in fact
			// be some teams in the instance when were are loading as they are creating during
			// the map load.  But since the team ids are reset on the map load, they are
			// created with exactly the same team IDs they had before
			//
			teamInstance = TheTeamFactory->findTeamByID( teamID );
			if( teamInstance == NULL )
			{

				// create team
				teamInstance = TheTeamFactory->createTeamOnPrototype( this );

				// restore original ID we read from the file
				teamInstance->setID( teamID );

			}  // end if

			// xfer team data
			xfer->xferSnapshot( teamInstance );

		}  // end for, i

	}  // end else, load
	
}  // end xfer

// ------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------
void TeamPrototype::loadPostProcess( void )
{

}  // end loadPostProcess

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
Team::Team(TeamPrototype *proto, TeamID id ) : 
  m_id( id ),
	m_proto(proto), 
	m_enteredOrExited(false), 
	m_active(false), 
	m_seeEnemy(false), 
	m_prevSeeEnemy(false), 
	m_checkEnemySighted(false),
	m_isRecruitablitySet(false),
	m_isRecruitable(false),
	m_destroyThreshold(0), 
	m_curUnits(0), 
	m_wasIdle(false)
{
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_created = FALSE;
	
	//
	m_commonAttackTarget = INVALID_ID;

	// allocate new relation map pools
	m_playerRelations = newInstance(PlayerRelationMap);
	m_teamRelations = newInstance(TeamRelationMap);

	if (proto)
	{
		proto->prependTo_TeamInstanceList(this);
		if (!proto->getTemplateInfo()->m_scriptOnAllClear.isEmpty() ||
				!proto->getTemplateInfo()->m_scriptOnEnemySighted.isEmpty())
		{
			m_checkEnemySighted = true;	 // Only keep track of enemy sighted if there is a script that cares.
		}
		
		AsciiString teamName = proto->getName();
		teamName.concat(" - creating team instance.");
		TheScriptEngine->AppendDebugMessage(teamName, false);
	}

	for (Int i = 0; i < MAX_GENERIC_SCRIPTS; ++i) 
	{
		m_shouldAttemptGenericScript[i] = true;
	}

	
}

// ------------------------------------------------------------------------
Team::~Team()
{
//	DEBUG_ASSERTCRASH(getFirstItemIn_TeamMemberList() == NULL, ("Team still has members in existence"));

	TheScriptEngine->notifyOfTeamDestruction(this);

	// Tell the players a team is going away.
	Int i;
	for (i=0; i<ThePlayerList->getPlayerCount(); i++) {
		Player *plyr = ThePlayerList->getNthPlayer(i);
		if (plyr) {
			plyr->preTeamDestroy(this);
		}
	}

	Object* tm;
	while ((tm = getFirstItemIn_TeamMemberList()) != NULL)
	{
		tm->setTeam(NULL);
	}
//this test is valid, but will generate a 'false positive' during game teardown
//DEBUG_ASSERTCRASH(!(getControllingPlayer() && getControllingPlayer()->getDefaultTeam()==this),("I am still someones default team -- sure you want to delete me?"));

	DEBUG_ASSERTCRASH(m_proto, ("proto should not be null"));
	if (m_proto && m_proto->isInList_TeamInstanceList(this))
		m_proto->removeFrom_TeamInstanceList(this);

	// delete the relation maps (the destructor clears the actual map if any data is present)
	deleteInstance(m_teamRelations);
	deleteInstance(m_playerRelations);

	// make sure the xfer list is clear
	m_xferMemberIDList.clear();

}

// ------------------------------------------------------------------------
Player *Team::getControllingPlayer() const
{
	return m_proto->getControllingPlayer();
}

// ------------------------------------------------------------------------
void Team::setControllingPlayer(Player *newController)
{
	// NULL is not allowed, but is caught by TeamPrototype::setControllingPlayer()
	m_proto->setControllingPlayer(newController);

	// This function is used by one script, and it is kind of odd.  The actual units
	// are not getting captured, the team they are on is being reassigned to a new player.  
	// The Team doesn't change, it just starts to return a different answer when you ask for
	// the controlling player.  I don't want to make the major change of onCapture on everyone,
	// so I will do the minor fix for the specific bug, which is harmless even when misused.

	// Tell all members to redo their looking status, as their Player has changed, but they don't know.
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) 
	{
		Object *obj = iter.cur();
		if (!obj) 
			continue;

		obj->handlePartitionCellMaintenance();		
	}

}

// ------------------------------------------------------------------------
void Team::setAttackPriorityName(AsciiString name)
{
	if (m_proto) m_proto->setAttackPriorityName(name);
}

// ------------------------------------------------------------------------
void Team::getTeamAsAIGroup(AIGroup *pAIGroup)
{
	if (!pAIGroup) {
		return;
	}

	// Should this clear out the pAIGroup that it receives? I don't think so, but that 
	// would go here if so. jkmcd

	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		if (iter.cur()) {
			pAIGroup->add(iter.cur());
		}
	}
}


// ------------------------------------------------------------------------
Int Team::getTargetableCount() const
{
	Int retVal = 0;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *obj = iter.cur();
		if (!obj) {
			continue;
		}
		
		if (obj->isEffectivelyDead() || (obj->getAIUpdateInterface() == NULL && !obj->isKindOf(KINDOF_STRUCTURE))) {
			continue;
		}

		++retVal;
	}

	return retVal;
}

// ------------------------------------------------------------------------
Relationship Team::getRelationship(const Team *that) const
{
	// do we have an override for that particular team? if so, return it.
	if (!m_teamRelations->m_map.empty() && that != NULL)
	{
		TeamRelationMapType::const_iterator it = m_teamRelations->m_map.find(that->getID());
		if (it != m_teamRelations->m_map.end())
		{
			return (*it).second;
		}
	}

	// hummm... well, do we have an override for that team's player?
	if (!m_playerRelations->m_map.empty() && that != NULL)
	{
		Player* thatPlayer = that->getControllingPlayer();
		if (thatPlayer != NULL)
		{
			PlayerRelationMapType::const_iterator it = m_playerRelations->m_map.find(thatPlayer->getPlayerIndex());
			if (it != m_playerRelations->m_map.end())
			{
				return (*it).second;
			}
		}
	}

	// nope -- go with our Player's view on the matter.
	return getControllingPlayer()->getRelationship(that);
}

// ------------------------------------------------------------------------
void Team::setTeamTargetObject(const Object *target)
{
	if (target==NULL) {
		m_commonAttackTarget = INVALID_ID;
		return;
	}
	// Only ai players do common attack.
	if (getControllingPlayer()->getPlayerType() == PLAYER_COMPUTER) {
		if (getControllingPlayer()->getPlayerDifficulty() == DIFFICULTY_EASY) {
			return; // we don't do this for easy.  jba.
		}
		m_commonAttackTarget = target->getID();
	}
}

// ------------------------------------------------------------------------
Object *Team::getTeamTargetObject(void)
{
	if (m_commonAttackTarget == INVALID_ID) {
		return NULL;
	}			 
	Object *target = TheGameLogic->findObjectByID(m_commonAttackTarget);
	if (target) {
		//If the enemy unit is stealthed and not detected, then we can't attack it!
	if( target->testStatus( OBJECT_STATUS_STEALTHED ) && 
			!target->testStatus( OBJECT_STATUS_DETECTED ) )
		{
			target = NULL;
		}
	}
	if (target && target->isEffectivelyDead()) {
		target = NULL;
	}
	if (target && target->getContainedBy()) {
		target = NULL; // target entered a building or vehicle, so stop targeting.
	}
	if (target == NULL) {
		m_commonAttackTarget = INVALID_ID;
	}
	return target;
}

// ------------------------------------------------------------------------
 void Team::setOverrideTeamRelationship( TeamID teamID, Relationship r)
{
	if (teamID != TEAM_ID_INVALID )
	{
		// note that this creates the entry if it doesn't exist.
		m_teamRelations->m_map[teamID] = r;
	}
}

// ------------------------------------------------------------------------
Bool Team::removeOverrideTeamRelationship( TeamID teamID )
{
	if (!m_teamRelations->m_map.empty())
	{
		if (teamID == TEAM_ID_INVALID)
		{
			m_teamRelations->m_map.clear();
			return true;
		}
		else
		{
			TeamRelationMapType::iterator it = m_teamRelations->m_map.find(teamID);
			if (it != m_teamRelations->m_map.end())
			{
				m_teamRelations->m_map.erase(it);
				return true;
			}
		}
	}
	return false;
}

// ------------------------------------------------------------------------
void Team::setOverridePlayerRelationship( Int playerIndex, Relationship r )
{
	if (playerIndex != PLAYER_INDEX_INVALID)
	{
		// note that this creates the entry if it doesn't exist.
		m_playerRelations->m_map[playerIndex] = r;
	}
}

// ------------------------------------------------------------------------
Bool Team::removeOverridePlayerRelationship( Int playerIndex )
{
	if (!m_playerRelations->m_map.empty())
	{
		if (playerIndex == PLAYER_INDEX_INVALID)
		{
			m_playerRelations->m_map.clear();
			return true;
		}
		else
		{
			PlayerRelationMapType::iterator it = m_playerRelations->m_map.find(playerIndex);
			if (it != m_playerRelations->m_map.end())
			{
				m_playerRelations->m_map.erase(it);
				return true;
			}
		}
	}
	return false;
}

// ------------------------------------------------------------------------
void Team::countObjectsByThingTemplate(Int numTmplates, const ThingTemplate* const* things, Bool ignoreDead, Int *counts, Bool ignoreUnderConstruction) const
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		const ThingTemplate *objtmpl = iter.cur()->getTemplate();
		for (Int i = 0; i < numTmplates; ++i)
		{
			//Kris: Compare
			if( !objtmpl->isEquivalentTo( things[i] ) )
			{
				continue;
			}

			if (ignoreDead && iter.cur()->isEffectivelyDead())
				continue;

			if( ignoreUnderConstruction && iter.cur()->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) )
				continue;

			counts[i] += 1;
			break;	// from 'next i', NOT 'next object'

		}
	}
}

// ------------------------------------------------------------------------
Int Team::countBuildings(void)
{
	int retVal = 0;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		const ThingTemplate* objtmpl = iter.cur()->getTemplate();
		if (!objtmpl) {
			continue;
		}
		if (objtmpl->isKindOf(KINDOF_STRUCTURE)) {
			++retVal;
		}
	}
	return retVal;
}

// ------------------------------------------------------------------------
Int Team::countObjects(KindOfMaskType setMask, KindOfMaskType clearMask)
{
	int retVal = 0;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		const ThingTemplate* objtmpl = iter.cur()->getTemplate();
		if (!objtmpl) {
			continue;
		}
		if (objtmpl->isKindOfMulti(setMask, clearMask)) {
			++retVal;
		}
	}
	return retVal;
}

// ------------------------------------------------------------------------
void Team::healAllObjects(void)
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) 
	{
		iter.cur()->healCompletely();
	}
}

// ------------------------------------------------------------------------
void Team::iterateObjects( ObjectIterateFunc func, void *userData )
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) 
	{
		func( iter.cur(), userData );
	}
}

// ------------------------------------------------------------------------
Bool Team::hasAnyBuildings() const
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isDestroyed()) 
			continue;

		if (iter.cur()->isKindOf(KINDOF_STRUCTURE))
			return true;
	}
	return false;
}

// ------------------------------------------------------------------------
Bool Team::hasAnyBuildings(KindOfMaskType kindOf) const
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isDestroyed()) 
			continue;

		kindOf.set(KINDOF_STRUCTURE);
		if (iter.cur()->isKindOfMulti(kindOf, KINDOFMASK_NONE))
			return true;
	}
	return false;
}

// ------------------------------------------------------------------------
Bool Team::hasAnyUnits() const
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isDestroyed()) 
			continue;

		// If it's a structure, it's not  a unit.
		if (iter.cur()->isKindOf(KINDOF_STRUCTURE)) continue;

		// If it's a projectile, it's not  a unit.
		if (iter.cur()->isKindOf(KINDOF_PROJECTILE)) continue;

		// If it's a mine, it's not  a unit.
		if (iter.cur()->isKindOf(KINDOF_MINE)) continue;

		return true;
	}
	return false;
}

// ------------------------------------------------------------------------
Bool Team::isIdle() const
{
	Bool idle = true; // assume idle.
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (!ai) continue; // Non-ai things are idle.

		if (iter.cur()->isEffectivelyDead())
			continue;

		if (!ai->isIdle()) 
		{
			idle = false;
			break;
		}
	}
	return idle;
}

// ------------------------------------------------------------------------
Bool Team::hasAnyObjects() const
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isDestroyed()) 
			continue;

		if (iter.cur()->isKindOf(KINDOF_PROJECTILE)) {
			continue; // shell & missiles don't count. jba.
		}

		if (iter.cur()->isKindOf(KINDOF_INERT)) {
			// inert stuff doesn't count.  This is for radiation fields, which are living
			// so they can be attacked by ambulances.
			continue;
		}

		if (iter.cur()->isKindOf(KINDOF_MINE)) {
			// Mines don't count.
			continue;
		}

		return true;
	}
	return false;
}

// ------------------------------------------------------------------------
/** Clears m_enteredExited, checks & clears m_created. */
void Team::updateState(void) 
{
	m_enteredOrExited = false;
	if (!m_active) {
		return; 
	}
	const TeamTemplateInfo *pInfo = m_proto->getTemplateInfo();
	if (m_created) 
	{
		m_created = false;
		// Run the on create script, if any.
		if (!pInfo->m_scriptOnCreate.isEmpty()) 
		{
			TheScriptEngine->runScript(pInfo->m_scriptOnCreate, this);
		}

		// Set up info for the onDestroyed script, if needed.
		if (!pInfo->m_scriptOnDestroyed.isEmpty() )
		{
			for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
			{
				m_curUnits++;
			}
			m_destroyThreshold = m_curUnits - (m_curUnits * pInfo->m_destroyedThreshold);
			if (m_destroyThreshold>m_curUnits-1) m_destroyThreshold = m_curUnits-1;
			if (m_destroyThreshold<0) m_destroyThreshold = 0;
		}
	}

	// Do enemy sighted/on clear checks.
	if (m_checkEnemySighted) {
		m_prevSeeEnemy = m_seeEnemy;
		m_seeEnemy = false;
		Bool anyAliveInTeam = false; // If we're all dead, don't do all clear.
		// only consider enemies.
		for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
		{
			if (iter.cur()->isEffectivelyDead())
				continue;

			PartitionFilterRelationship	filterTeam(iter.cur(), PartitionFilterRelationship::ALLOW_ENEMIES);

			// and only stuff that is not dead
			PartitionFilterAlive filterAlive;
			PartitionFilterSameMapStatus filterMapStatus(iter.cur());

			PartitionFilter *filters[] = { &filterTeam, &filterAlive, &filterMapStatus, NULL };
			Real visionRange = iter.cur()->getVisionRange();
			anyAliveInTeam = true;
			Object *pObj = ThePartitionManager->getClosestObject( iter.cur(), visionRange, 
				FROM_CENTER_2D, filters );
			if (pObj) {
				m_seeEnemy = true;
				break;
			}
		}
		if (anyAliveInTeam) {
			if (m_prevSeeEnemy != m_seeEnemy) 
			{
				if (m_seeEnemy) 
				{
					// fire onEnemySighted
					TheScriptEngine->runScript(pInfo->m_scriptOnEnemySighted, this);
				} else {
					// fire on all clear.
					TheScriptEngine->runScript(pInfo->m_scriptOnAllClear, this);
				}
			}
		}
	}

	// Do onDestroyed checks.
	if (!pInfo->m_scriptOnDestroyed.isEmpty()) 
	{
		Int prevUnits = m_curUnits;
		m_curUnits = 0;
		for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
		{
			if (iter.cur()->isEffectivelyDead())
				continue;

			m_curUnits++;
		}
		if (m_curUnits != prevUnits && m_curUnits <= m_destroyThreshold) 
		{
			TheScriptEngine->runScript(pInfo->m_scriptOnDestroyed, this);
			m_destroyThreshold = -1; // don't trigger again.
		}
	}

	// Do onIdle checks.
	if (!pInfo->m_scriptOnIdle.isEmpty()) 
	{
		Bool isIdle = true;
		Bool anyAliveInTeam = false; // If we're all dead, don't do all clear.
		for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
		{
			if (iter.cur()->isEffectivelyDead()) {
				continue;
			}
			AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
			if (!ai) continue;
			anyAliveInTeam = true;
			if (!ai->isIdle()) {
				isIdle = false;
			}
		}
		if (anyAliveInTeam && isIdle && m_wasIdle) 
		{
			TheScriptEngine->runScript(pInfo->m_scriptOnIdle, this);
		}
		m_wasIdle = isIdle;
	}
}

// ------------------------------------------------------------------------
void Team::notifyTeamOfObjectDeath( void )
{
	const TeamTemplateInfo *pInfo = m_proto->getTemplateInfo();
	if (!pInfo) {
		return;
	}

	if (pInfo->m_scriptOnUnitDestroyed.isEmpty()) {
		return;
	}

	TheScriptEngine->runScript(pInfo->m_scriptOnUnitDestroyed, this);
}

// ------------------------------------------------------------------------
Bool Team::didAllEnter(PolygonTrigger *pTrigger, UnsignedInt whichToConsider) const
{
	// If any units entered or exited, they set this flag.
	if (!m_enteredOrExited) return false;

	Bool anyConsidered = false;
	Bool entered = false;
	Bool outside = false;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (ai) {
			const LocomotorSet& locoSet = ai->getLocomotorSet();
			{
				if (!locoSetMatches(locoSet.getValidSurfaces(), whichToConsider)) {
					continue;
				}
			}
		} else {
			// things without ai should consider themselves ground units
			if (!locoSetMatches(LOCOMOTORSURFACE_GROUND, whichToConsider)) {
				continue;
			}
		}

		if (iter.cur()->isEffectivelyDead())
			continue;
		
		if (iter.cur()->isKindOf(KINDOF_INERT))
			continue;

		if (iter.cur()->didEnter(pTrigger)) {
			entered = true;
		} else {
			if (!iter.cur()->isInside(pTrigger)) {
				outside = true;
			}
		}

		// We need this in order to prevent this from returning a false positive
		anyConsidered = true;
	}
	return entered && !outside;
}

// ------------------------------------------------------------------------
Bool Team::didPartialEnter(PolygonTrigger *pTrigger, UnsignedInt whichToConsider) const
{
	// If any units entered or exited, they set this flag.
	if (!m_enteredOrExited) return false;

	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (ai) {
			const LocomotorSet& locoSet = ai->getLocomotorSet();
			{
				if (!locoSetMatches(locoSet.getValidSurfaces(), whichToConsider)) {
					continue;
				}
			}
		} else {
			// things without ai should consider themselves ground units
			if (!locoSetMatches(LOCOMOTORSURFACE_GROUND, whichToConsider)) {
				continue;
			}
		}

		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isKindOf(KINDOF_INERT))
			continue;

		if (iter.cur()->didEnter(pTrigger)) {
			return true;
		} 
	}
	return false;
}

// ------------------------------------------------------------------------
Bool Team::didPartialExit(PolygonTrigger *pTrigger, UnsignedInt whichToConsider) const
{
	// If any units entered or exited, they set this flag.
	if (!m_enteredOrExited) return false;

	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (ai) {
			const LocomotorSet& locoSet = ai->getLocomotorSet();
			{
				if (!locoSetMatches(locoSet.getValidSurfaces(), whichToConsider)) {
					continue;
				}
			}
		} else {
			// things without ai should consider themselves ground units
			if (!locoSetMatches(LOCOMOTORSURFACE_GROUND, whichToConsider)) {
				continue;
			}
		}

		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isKindOf(KINDOF_INERT))
			continue;

		if (iter.cur()->didExit(pTrigger)) {
			return true;
		} 
	}
	return false;
}

// ------------------------------------------------------------------------
Bool Team::didAllExit(PolygonTrigger *pTrigger, UnsignedInt whichToConsider) const
{
	// If any units entered or exited, they set this flag.
	if (!m_enteredOrExited) 
		return false;

	Bool anyConsidered = false;
	Bool exited = false;
	Bool inside = false;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (ai) {
			const LocomotorSet& locoSet = ai->getLocomotorSet();
			{
				if (!locoSetMatches(locoSet.getValidSurfaces(), whichToConsider)) {
					continue;
				}
			}
		} else {
			// things without ai should consider themselves ground units
			if (!locoSetMatches(LOCOMOTORSURFACE_GROUND, whichToConsider)) {
				continue;
			}
		}

		if (iter.cur()->isEffectivelyDead())
			continue;
		
		if (iter.cur()->isKindOf(KINDOF_INERT))
			continue;

		if (iter.cur()->didExit(pTrigger)) {
			exited = true;
		} else {
			if (iter.cur()->isInside(pTrigger)) {
				inside = true;
			}
		}

		// We need this in order to prevent this from returning a false positive
		anyConsidered = true;
	}
	return anyConsidered && exited && !inside;
}

// ------------------------------------------------------------------------
Bool Team::allInside(PolygonTrigger *pTrigger, UnsignedInt whichToConsider) const
{
	// empty teams are not inside.
	if (!hasAnyObjects()) {
		return false;
	}

	Bool anyConsidered = false;
	Bool anyOutside = false;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (ai) {
			const LocomotorSet& locoSet = ai->getLocomotorSet();
			{
				if (!locoSetMatches(locoSet.getValidSurfaces(), whichToConsider)) {
					continue;
				}
			}
		} else {
			// things without ai should consider themselves ground units
			if (!locoSetMatches(LOCOMOTORSURFACE_GROUND, whichToConsider)) {
				continue;
			}
		}

		if (iter.cur()->isEffectivelyDead()) {
			continue;
		}

		if (iter.cur()->isKindOf(KINDOF_INERT))
			continue;

		if (!iter.cur()->isInside(pTrigger)) {
			anyOutside = true;
		}

		// We need this in order to prevent this from returning a false positive
		anyConsidered = true;
	}
	return anyConsidered && !anyOutside;
}

// ------------------------------------------------------------------------
Bool Team::noneInside(PolygonTrigger *pTrigger, UnsignedInt whichToConsider) const
{
	Bool anyConsidered = false;
	Bool anyInside = false;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (ai) {
			const LocomotorSet& locoSet = ai->getLocomotorSet();
			{
				if (!locoSetMatches(locoSet.getValidSurfaces(), whichToConsider)) {
					continue;
				}
			}
		} else {
			// things without ai should consider themselves ground units
			if (!locoSetMatches(LOCOMOTORSURFACE_GROUND, whichToConsider)) {
				continue;
			}
		}

		// don't consider dead things.
		if (iter.cur()->isEffectivelyDead()) {
			continue;
		}

		if (iter.cur()->isKindOf(KINDOF_INERT))
			continue;

		if (iter.cur()->isInside(pTrigger)) {
			anyInside = true;
		}

		// We need this in order to prevent this from returning a false positive
		anyConsidered = true;
	}
	return anyConsidered && !anyInside;
}

// ------------------------------------------------------------------------
Bool Team::someInsideSomeOutside(PolygonTrigger *pTrigger, UnsignedInt whichToConsider) const
{
	Bool anyConsidered = false;
	Bool anyInside = false;
	Bool anyOutside = false;
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		AIUpdateInterface *ai = iter.cur()->getAIUpdateInterface();
		if (ai) {
			const LocomotorSet& locoSet = ai->getLocomotorSet();
			{
				if (!locoSetMatches(locoSet.getValidSurfaces(), whichToConsider)) {
					continue;
				}
			}
		} else {
			// things without ai should consider themselves ground units
			if (!locoSetMatches(LOCOMOTORSURFACE_GROUND, whichToConsider)) {
				continue;
			}
		}

		// don't consider dead things.
		if (iter.cur()->isEffectivelyDead()) {
			continue;
		}

		if (iter.cur()->isKindOf(KINDOF_INERT))
			continue;

		if (iter.cur()->isInside(pTrigger)) {
			anyInside = true;
		} else {
			anyOutside = true;
		}

		// In this particular case, this is unnecessary. However, unless it is a performance hit, please 
		// leave it.
		anyConsidered = true;
	}
	return anyConsidered && anyInside && anyOutside;
}

const Coord3D* Team::getEstimateTeamPosition(void)
{
	// this doesn't actually calculate the team position, but rather estimates it by
	// returning the position of the first member of the team
	DLINK_ITERATOR<Object> iter = iterate_TeamMemberList();
	Object *obj = iter.cur();
	if (!obj)
		return NULL;

	const Coord3D *pos = iter.cur()->getPosition();
	if (!pos)
		return NULL;

	return pos;
}

// ------------------------------------------------------------------------
void Team::deleteTeam(Bool ignoreDead)
{
	// First off, if this Team is the Player's default team, we need to Evacuate everyone or else
	// Garrisoned buildings will fall victim to this deletion as well, since they were added to the
	// Default when captured.  Design intends with this script to kill the people out from inside.
	// If the thing is a transport, everything will still work, as the issue at hand is the container's
	// wanting to change sides when emptied.  The bug is that the people in the Garrisoned building
	// are deleted, and that changes the Team of the building, and then the DLink walks down the new team
	// and deletes the wrong stuff.  Like every tree and civialian building on the map.
	// Of course, to prevent the exact same DLINK jumping bug, I must first record what guys I am going to
	// Evacuate, or else after the first occupied building is emptied, he will move his Next into the
	// same damn wrong team.
	if( this == getControllingPlayer()->getDefaultTeam() )
	{
		std::list<Object *> guysToMakeEvacuate;
		for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) 
		{
			Object *obj = iter.cur();
			if (!obj) 
				continue;

			if( obj->getContain()  &&  (obj->getContain()->getContainCount() > 0) )
			{
				// Write them all down, so the DLINK track jumping doesn't screw me up here as well.
				guysToMakeEvacuate.push_back( obj );
			}
		}

		for( std::list<Object *>::iterator it = guysToMakeEvacuate.begin(); it != guysToMakeEvacuate.end(); /*nothing*/ )
		{
			Object *obj = *it;
			it++;
			if( obj->getContain() )
			{
				obj->getContain()->removeAllContained();
			}
		}
	}

	// this doesn't actually delete the team, it deletes the members of the team.
	// the team itself will be deleted in updateState.
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) 
	{
		Object *obj = iter.cur();
		if (!obj) {
			continue;
		}

		// The reason the comment says the team will be deleted is because it will see that it is empty.
		// So yes, the Team will survive for a while, but it was going to anyway.  This is a script flag, so if
		// they use it on a team with things that can't die, then yeah, the Team will last forever.  But then it is
		// user error.
		if( ignoreDead && obj->isEffectivelyDead() )
			continue; 

		TheGameLogic->destroyObject(obj);
	}
}

// ------------------------------------------------------------------------
/* Transfer our units to new team. */
void Team::transferUnitsTo(Team *newTeam)
{
	if (this == newTeam) return;
	if (newTeam == NULL) return;
	Object *obj;
	while ((obj = getFirstItemIn_TeamMemberList()) != 0) 
	{
		obj->setTeam(newTeam);
	} 
}

// ------------------------------------------------------------------------
static Bool isInBuildVariations(const ThingTemplate* ttWithVariations, const ThingTemplate* b)
{
	const std::vector<AsciiString>& bv = ttWithVariations->getBuildVariations();
	if (bv.empty())
		return false;

	for (std::vector<AsciiString>::const_iterator it = bv.begin(); it != bv.end(); ++it)
	{
		if (b->getName() == *it)
			return true;
	}
	return false;
}

// ------------------------------------------------------------------------
/* Try to recruit a unit from other teams of this player. */
Object *Team::tryToRecruit(const ThingTemplate *tTemplate, const Coord3D *teamHome, Real maxDist)
{
	Player *myPlayer = getControllingPlayer();
	Object *obj=NULL;
	Real distSqr = maxDist*maxDist;
	Object *recruit = NULL;
	for( obj = TheGameLogic->getFirstObject(); obj; obj = obj->getNextObject() )
	{
		if (!obj->getTemplate()->isEquivalentTo(tTemplate)) 
		{
			// it might be ok, if tTemplate is really just a "build-variations" template...
			if (!isInBuildVariations(tTemplate, obj->getTemplate()))
				continue;
		}
		if (obj->getControllingPlayer() != myPlayer) 
			continue;
		Team *team = obj->getTeam();
		Bool isDefaultTeam = false;
		if (team == myPlayer->getDefaultTeam()) {
			isDefaultTeam = true;
		}
		if (!team->isActive()) {
			continue; // Team is building, so don't steal it's members yet.
		}
		if (team->getPrototype()->getTemplateInfo()->m_productionPriority>=getPrototype()->getTemplateInfo()->m_productionPriority) {
			continue;
		}
		Bool teamIsRecruitable = isDefaultTeam;	 // Default team always recruitable.
		if (team->getPrototype()->getTemplateInfo()->m_isAIRecruitable) {
			teamIsRecruitable = true; 
		}
		// Check & see if individual team has been marked for recruitability.
		if (team->m_isRecruitablitySet) {
			teamIsRecruitable = team->m_isRecruitable;
		}
		if (!teamIsRecruitable) {
			continue;
		}
		if (obj->getAIUpdateInterface() && !obj->getAIUpdateInterface()->isRecruitable()) {
			continue; // can't recruit this unit.
		}
		if( obj->isDisabledByType( DISABLED_HELD ) ) 
		{
			continue; // Don't recruit held units.
		}
		Real dx, dy;
		dx = teamHome->x - obj->getPosition()->x;
		dy = teamHome->y - obj->getPosition()->y;

		if (isDefaultTeam && recruit == NULL) {
			recruit = obj;
			distSqr = dx*dx+dy*dy;
		}

		if (dx*dx+dy*dy > distSqr) {
			continue;
		}
		distSqr = dx*dx+dy*dy;
		recruit = obj;
	}
	if (recruit!=NULL) {
		return recruit;
	}
 	return NULL;	 
}

// ------------------------------------------------------------------------
void Team::evacuateTeam(void)
{
	std::list<Object *> objectsToProcess;

	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *obj = iter.cur();
		if (!obj || obj->isDestroyed() || obj->isEffectivelyDead()) {
			continue;
		}

		ContainModuleInterface *cmi = obj->getContain();
		UnsignedInt numContained = 0;
		if (cmi != NULL) {
			numContained = cmi->getContainCount();
		}
		if (numContained > 0) {
			objectsToProcess.push_back(obj);
		}
	}

	// evacuate any containers
	std::list<Object *>::iterator objIt;
	for (objIt = objectsToProcess.begin(); objIt != objectsToProcess.end(); ++objIt)
	{
		Object *obj = *objIt;
		ContainModuleInterface *cmi = obj->getContain();
		if (cmi)
		{
			cmi->removeAllContained();
		}
	}
	objectsToProcess.clear();
}

// ------------------------------------------------------------------------
void Team::killTeam(void)
{
	std::list<Object *> objectsToProcess;

	evacuateTeam();

	// beacons are effectively dead, so we need to destroy via a non-kill() method
	const ThingTemplate *beaconTemplate = TheThingFactory->findTemplate( getControllingPlayer()->getPlayerTemplate()->getBeaconTemplate() );

	// now find objects to kill
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *obj = iter.cur();
		if (!obj || obj->isDestroyed() || (obj->isEffectivelyDead() && !obj->getTemplate()->isEquivalentTo(beaconTemplate)))
		{
			continue;
		}
		Team *objTeam = obj->getTeam();
		// the object's team could change after having all the passengers get out.
		if (objTeam == this) {
			objectsToProcess.push_back(obj);
		}
	}

	// and finally, kill things
	std::list<Object *>::iterator objIt;
	for (objIt = objectsToProcess.begin(); objIt != objectsToProcess.end(); ++objIt)
	{
		Object *obj = *objIt;
		if (obj->isKindOf(KINDOF_TECH_BUILDING))
			obj->setTeam(ThePlayerList->getNeutralPlayer()->getDefaultTeam());
		else
			obj->kill();
	}
	objectsToProcess.clear();
}

// ------------------------------------------------------------------------
Bool Team::damageTeamMembers(Real amount) 
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isDestroyed()) 
			continue;

		// do max amount of damage to object
		if (amount < 0.0) {
			iter.cur()->kill();
		} else {
			DamageInfo damageInfo;

			damageInfo.in.m_damageType = DAMAGE_UNRESISTABLE;
			damageInfo.in.m_deathType = DEATH_NORMAL;
			damageInfo.in.m_sourceID = INVALID_ID;
			damageInfo.in.m_amount = amount;
			iter.cur()->attemptDamage( &damageInfo );
		}
		
	}
	return false;
}

// ------------------------------------------------------------------------
/// @todo This should give a "team move" command, not individual move orders (MSB)
void Team::moveTeamTo(Coord3D destination) 
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		if (iter.cur()->isEffectivelyDead())
			continue;

		if (iter.cur()->isDestroyed()) 
			continue;
	}
}

// ------------------------------------------------------------------------
Bool Team::hasAnyBuildFacility() const
{
	for (DLINK_ITERATOR<Object> iter = iterate_TeamMemberList(); !iter.done(); iter.advance())
	{
		const ThingTemplate *objtmpl = iter.cur()->getTemplate();
		if (objtmpl->isBuildFacility()) 
			return true;
	}
	return false;
}

// ------------------------------------------------------------------------
//DECLARE_PERF_TIMER(updateGenericScripts)
void Team::updateGenericScripts(void)
{
	//USE_PERF_TIMER(updateGenericScripts)
	for (Int i = 0; i < MAX_GENERIC_SCRIPTS; ++i) {
		if (m_shouldAttemptGenericScript[i]) {
			// Does the condition succeed? If so, run it. If it is a run once script, also mark that we 
			// shouldn't run it again.
			Script *script = m_proto->getGenericScript(i);
			if (script) {
				if (TheScriptEngine->evaluateConditions(script, this)) {
					// It was successful.
					if (script->isOneShot()) {
						m_shouldAttemptGenericScript[i] = false;
					}
					TheScriptEngine->friend_executeAction(script->getAction(), this);
					AsciiString msg = "Generic script '";
					msg.concat(script->getName());
					msg.concat("' run on team ");
					msg.concat(getName());
					TheScriptEngine->AppendDebugMessage(msg, false);
				}
			} else { 
				m_shouldAttemptGenericScript[i] = false;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void Team::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Team::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// xfer id, this is a sanity check as team id m_id should always be valid at this point
	TeamID teamID = m_id;
	xfer->xferUser( &teamID, sizeof( TeamID ) );
	if( teamID != m_id )
	{

		DEBUG_CRASH(( "Team::xfer - TeamID mismatch.  Xfered '%d' but should be '%d'\n",
									teamID, m_id ));
		throw SC_INVALID_DATA;

	}  // end if
		
	// member list count and data
	ObjectID memberID;
	UnsignedShort memberCount = 0;
	for( DLINK_ITERATOR< Object > objIt = iterate_TeamMemberList(); 
			 objIt.done() == FALSE; 
			 objIt.advance() )
		memberCount++;
	xfer->xferUnsignedShort( &memberCount );
	if( xfer->getXferMode() == XFER_SAVE )
	{
		Object *obj;
		
		// save all member info
		for( DLINK_ITERATOR< Object > objIt = iterate_TeamMemberList(); 
				 objIt.done() == FALSE; 
				 objIt.advance() )
		{

			// get object
			obj = objIt.cur();

			// save id
			memberID = obj->getID();
			xfer->xferObjectID( &memberID );

		}  // end for

	}  // end if, save
	else
	{

		// load all members
		for( UnsignedShort i = 0; i < memberCount; ++i )
		{

			// read ID
			xfer->xferObjectID( &memberID );

			// put on pending list for later processing
			m_xferMemberIDList.push_back( memberID );

		}  // end for, i

	}  // end else, load

	// state
	xfer->xferAsciiString( &m_state );

	// entered or exited
	xfer->xferBool( &m_enteredOrExited );

	// active status
	xfer->xferBool( &m_active );

	// created flag
	xfer->xferBool( &m_created );

	// check enemy sighted
	xfer->xferBool( &m_checkEnemySighted );

	// see enemy
	xfer->xferBool( &m_seeEnemy );

	// previous see enemy
	xfer->xferBool( &m_prevSeeEnemy );

	// was idle
	xfer->xferBool( &m_wasIdle );

	// destroy threshold
	xfer->xferInt( &m_destroyThreshold );

	// current units
	xfer->xferInt( &m_curUnits );

	// waypoint
	UnsignedInt currentWaypointID = m_currentWaypoint ? m_currentWaypoint->getID() : 0;
	xfer->xferUnsignedInt( &currentWaypointID );
	if( xfer->getXferMode() == XFER_LOAD )
		m_currentWaypoint = TheTerrainLogic->getWaypointByID( currentWaypointID );

	UnsignedShort shouldAttemptGenericScriptCount = MAX_GENERIC_SCRIPTS;
	xfer->xferUnsignedShort( &shouldAttemptGenericScriptCount );
	if ( shouldAttemptGenericScriptCount != MAX_GENERIC_SCRIPTS )
	{
		DEBUG_CRASH(("Team::xfer - The number of allowable Generic scripts has changed, and this chunk needs to be versioned."));
		throw SC_INVALID_DATA;
	}

	for (Int i = 0; i < shouldAttemptGenericScriptCount; ++i)
		xfer->xferBool(&m_shouldAttemptGenericScript[i]);

	// recruitability set
	xfer->xferBool( &m_isRecruitablitySet );
	
	// is recruitable
	xfer->xferBool( &m_isRecruitable );

	// Common attack target.
	xfer->xferObjectID( &m_commonAttackTarget );

	// team relations
	xfer->xferSnapshot( m_teamRelations );

	// player relations
	xfer->xferSnapshot( m_playerRelations );
			
}  // ene xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void Team::loadPostProcess( void )
{

	//
	// now that all objects have actually been loaded, populate the member list with
	// real object pointers
	//
	Object *obj;
	std::list< ObjectID >::const_iterator it;
	for( it = m_xferMemberIDList.begin(); it != m_xferMemberIDList.end(); ++it )
	{

		// find object
		obj = TheGameLogic->findObjectByID( *it );
		if( obj == NULL )
		{

			DEBUG_CRASH(( "Team::loadPostProcess - Unable to post process object to member list, object ID = '%d'\n", *it ));
			throw SC_INVALID_DATA;

		}  // end if

		//
		// we are now disabling this code since the objects set their team during their
		// own xfer function which will actually put it on the team ... however, we will sanity
		// check everything here to make sure that all the objects that should be on the team,
		// are in fact on the team
		//
		if( isInList_TeamMemberList( obj ) == FALSE )
		{

			DEBUG_CRASH(( "Team::loadPostProcess - Object '%s'(%d) should be in team list but is not\n",
										obj->getTemplate()->getName().str(), obj->getID() ));
			throw SC_INVALID_DATA;

		}  // end if

	}  // end for

	// since we prepended the object member pointers, reverse that list so it's just like before
//	reverse_TeamMemberList();

	// we're done with the xfer list now
	m_xferMemberIDList.clear();

}  // end loadPostProcess



// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

