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

// FILE: ScriptConditions.cpp /////////////////////////////////////////////////////////////////////////
// The game scripting conditions.  Evaluates conditions.
// Author: John Ahlquist, Nov. 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/MapObject.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/SpecialPower.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Team.h"
#include "Common/Player.h"
#include "Common/ObjectStatusTypes.h"

#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/InGameUI.h"
#include "GameClient/View.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Module/SupplyWarehouseDockUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectTypes.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/ScriptConditions.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Scripts.h"
#include "GameLogic/VictoryConditions.h"


class ObjectTypesTemp
{
public:
	ObjectTypes* m_types;

	ObjectTypesTemp() : m_types(NULL)
	{
		m_types = newInstance(ObjectTypes);
	}

	~ObjectTypesTemp()
	{
		deleteInstance(m_types);
	}
};

// STATICS ////////////////////////////////////////////////////////////////////////////////////////
namespace rts
{
	template<typename T>
		T sum(std::vector<T>& vecOfValues )
	{
		T retVal = 0;
		typename std::vector<T>::iterator it;
		for (it = vecOfValues.begin(); it != vecOfValues.end(); ++it) {
			retVal += (*it);
		}
		return retVal;
	}
};

// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
ScriptConditionsInterface *TheScriptConditions = NULL;

class TransportStatus : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TransportStatus, "TransportStatus")
public:
	TransportStatus *	m_nextStatus;
	ObjectID					m_objID;
	UnsignedInt				m_frameNumber;
	Int								m_unitCount;

public:
	TransportStatus() : m_objID(INVALID_ID), m_frameNumber(0), m_unitCount(0), m_nextStatus(NULL) {}
	//~TransportStatus();
};

//-------------------------------------------------------------------------------------------------
TransportStatus::~TransportStatus()
{
	deleteInstance(m_nextStatus);
}

//-------------------------------------------------------------------------------------------------
static TransportStatus *s_transportStatuses;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ScriptConditions::ScriptConditions()
{

}  // end ScriptConditions

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ScriptConditions::~ScriptConditions()
{
	reset(); // just in case.
}  // end ~ScriptConditions

//-------------------------------------------------------------------------------------------------
/** Init */
//-------------------------------------------------------------------------------------------------
void ScriptConditions::init( void )
{

	reset();

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void ScriptConditions::reset( void )
{

	deleteInstance(s_transportStatuses);
	s_transportStatuses = NULL;
	// Empty for now.  jba.
}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void ScriptConditions::update( void )
{

	// Empty for now. jba
}  // end update


//-------------------------------------------------------------------------------------------------
/** Finds the player by the name in the parameter, and if found caches the player mask in the
parameter so we don't have to do a name search.  May return null if the player doesn't exist.*/
//-------------------------------------------------------------------------------------------------
Player *ScriptConditions::playerFromParam(Parameter *pSideParm)
{
	DEBUG_ASSERTCRASH(Parameter::SIDE == pSideParm->getParameterType(), ("Wrong parameter type."));
	Player *pPlayer=NULL;
	UnsignedInt mask = (UnsignedInt)pSideParm->getInt();
	if (mask) {
		pPlayer = ThePlayerList->getPlayerFromMask(mask);
	} else {
		pPlayer = TheScriptEngine->getPlayerFromAsciiString(pSideParm->getString());
		if (pPlayer) {
			// Enemy player can change dynamically, so don't cache the player mask.  jba.
			if (pSideParm->getString()!=THIS_PLAYER_ENEMY) {
				mask = pPlayer->getPlayerMask();
			}
		} else {
			mask = 0xFFFF0000;
		}
		pSideParm->friend_setInt((Int)mask);
	}
	DEBUG_ASSERTCRASH(pPlayer, ("Couldn't find player %s", pSideParm->getString().str()));
	return pPlayer;
}

//-------------------------------------------------------------------------------------------------
/** objectTypesFromParam */
//-------------------------------------------------------------------------------------------------
void ScriptConditions::objectTypesFromParam(Parameter *pTypeParm, ObjectTypes *outObjectTypes)
{
	if (!(pTypeParm && outObjectTypes)) {
		return;
	}

	AsciiString str = pTypeParm->getString();

	if (str.isEmpty()) {
		return;
	}

	ObjectTypes *types = TheScriptEngine->getObjectTypes(str);
	if (!types) {
		(*outObjectTypes).addObjectType(str);
	} else {
		(*outObjectTypes) = (*types);
	}
}


//-------------------------------------------------------------------------------------------------
/** evaluateAllDestroyed */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateAllDestroyed(Parameter *pSideParm )
{
	Player *pPlayer = playerFromParam(pSideParm);
	if (pPlayer) {
		return (!pPlayer->hasAnyObjects());
	}
	return true; // Non existent player is all destroyed. :)
}

//-------------------------------------------------------------------------------------------------
/** evaluateAllBuildFacilitiesDestroyed */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateAllBuildFacilitiesDestroyed(Parameter *pSideParm )
{
	Player *pPlayer = playerFromParam(pSideParm);
	if (pPlayer) {
		return (!pPlayer->hasAnyBuildFacility());
	}
	return true; // Non existent player is all destroyed. :)
}

//-------------------------------------------------------------------------------------------------
/** evaluateIsDestroyed */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateIsDestroyed(Parameter *pTeamParm)
{
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	// The team is the team based on the name, and the calling team (if any) and the team that
	// is being considered for the condition.  jba. :)
	if (theTeam) {
		return (!theTeam->hasAnyObjects());
	}
	return false; // Non existent team is not destroyed.
}

//-------------------------------------------------------------------------------------------------
/** evaluateBridgeBroken */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateBridgeBroken(Parameter *pBridgeParm)
{
	if (!TheTerrainLogic->anyBridgesDamageStatesChanged()) {
		// Don't bother checking if no bridges changed damage states.
		return false;
	}
	Object *theBridge = TheScriptEngine->getUnitNamed( pBridgeParm->getString() );
	if (theBridge) {
		return (TheTerrainLogic->isBridgeBroken(theBridge));
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateBridgeRepaired */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateBridgeRepaired(Parameter *pBridgeParm)
{
	if (!TheTerrainLogic->anyBridgesDamageStatesChanged()) {
		// Don't bother checking if no bridges changed damage states.
		return false;
	}
	Object *theBridge = TheScriptEngine->getUnitNamed( pBridgeParm->getString() );
	if (theBridge) {
		return (TheTerrainLogic->isBridgeRepaired(theBridge));
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedUnitDestroyed */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedUnitDestroyed(Parameter *pUnitParm)
{
	Object *theUnit = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (theUnit)
	{
		return theUnit->isEffectivelyDead();
	}

	if (TheScriptEngine->didUnitExist(pUnitParm->getString())) {
		return true;
	}
	return false; // Non existent unit is not destroyed.
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedUnitExists */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedUnitExists(Parameter *pUnitParm)
{
	Object *theUnit = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (theUnit)
	{
		return !theUnit->isEffectivelyDead();
	}

	return false; // Doesn't exist.
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedUnitDying */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedUnitDying(Parameter *pUnitParm)
{
	Object *theUnit = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (theUnit)
	{
		return theUnit->isEffectivelyDead();
	}

	if (TheScriptEngine->didUnitExist(pUnitParm->getString()))
	{
		return false; // already totally killed
	}
	return false; // Non existent unit is not dying.
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedUnitTotallyDead */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedUnitTotallyDead(Parameter *pUnitParm)
{
	Object *theUnit = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (theUnit) {
		return false; // if the unit still exists, it isn't totally dead.
	}

	if (TheScriptEngine->didUnitExist(pUnitParm->getString())) {
		// Did exist, now it doesnt.  So it is really, really dead.
		return true; // totally killed
	}
	return false; // Non existent unit is not dead.
}

//-------------------------------------------------------------------------------------------------
/** evaluateHasUnits */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateHasUnits(Parameter *pTeamParm)
{
	AsciiString desiredTeamName = pTeamParm->getString();
	// If they are calling a <this team> condition, do it.
	if (desiredTeamName == THIS_TEAM) {
		Team *theTeam = TheScriptEngine->getTeamNamed( desiredTeamName );
		// The team is the team based on the name, and the calling team (if any) and the team that
		// is being considered for the condition.  jba. :)
		if (theTeam) {
			return (theTeam->hasAnyUnits());
		}
		return false; // Non existent team has no units.
	}
	Team *thisTeam = TheScriptEngine->getTeamNamed(THIS_TEAM);
	if (thisTeam && thisTeam->getName()==desiredTeamName)	{
		return thisTeam->hasAnyUnits();
	}

	// It isn't THIS_TEAM, and doesn't match the THIS_TEAM, so check if any team with this name
	// has units.
	TeamPrototype *pProto = NULL;
	pProto = TheTeamFactory->findTeamPrototype(desiredTeamName);

	if (pProto) {
		// We have a team referred to in the conditions.  Iterate over the instances of the team,
		// applying the script conditions (and possibly actions) to each instance of the team.
		for (DLINK_ITERATOR<Team> iter = pProto->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			if (iter.cur()->hasAnyUnits()) {
				return true;
			}
		}
	}
	return false; // Non existent team has no units.
}

//-------------------------------------------------------------------------------------------------
/** evaluateUnitsEntered */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamInsideAreaPartially(Parameter *pTeamParm, Parameter *pTriggerAreaParm, Parameter *pTypeParm)
{
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	// The team is the team based on the name, and the calling team (if any) and the team that
	// is being considered for the condition.  jba. :)
	AsciiString triggerName = pTriggerAreaParm->getString();
	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerAreaParm->getString());

	if (pTrig == NULL) return false;
	if (theTeam) {
		return (theTeam->someInsideSomeOutside(pTrig, (UnsignedInt) pTypeParm->getInt()) ||
						theTeam->allInside(pTrig, (UnsignedInt) pTypeParm->getInt()));
	}
	return false; // Non existent team isn't in trigger area. :)
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedInsideArea */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedInsideArea(Parameter *pUnitParm, Parameter *pTriggerAreaParm )
{
	Object *theObj = TheScriptEngine->getUnitNamed( pUnitParm->getString() );

	if (!theObj) {
		return false;
	}

	AsciiString triggerName = pTriggerAreaParm->getString();
	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerAreaParm->getString());
	if (pTrig == NULL) return false;
	if (theObj) {
		Coord3D pCoord = *theObj->getPosition();
		ICoord3D iCoord;
		iCoord.x = pCoord.x; iCoord.y = pCoord.y; iCoord.z = pCoord.z;
		return pTrig->pointInTrigger(iCoord);
	}
	return false; // Non existent team isn't in trigger area. :)
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasUnitTypeInArea */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasUnitTypeInArea(Condition *pCondition, Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm, Parameter *pTypeParm, Parameter *pTriggerParm )
{
	AsciiString triggerName = pTriggerParm->getString();
	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());
	if (pTrig == NULL) return false;

	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}
	Player::PlayerTeamList::const_iterator it;
	Bool anyChanges = false;


	if (pCondition->getCustomData() == 0) anyChanges = true;

	for (it = pPlayer->getPlayerTeams()->begin(); it != pPlayer->getPlayerTeams()->end(); ++it) {
		if (anyChanges) break;
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			if (anyChanges) break;
			Team *team = iter.cur();
			if (!team) {
				continue;
			}
			if (team->didEnterOrExit()) {
				anyChanges = true;
			}
		}
	}

	if (TheScriptEngine->getFrameObjectCountChanged()+1 >= TheGameLogic->getFrame()) {
		anyChanges = true; // Objects were added/deleted last frame, so count could have changed.  jba.
	}

	if (!anyChanges) {
		if (pCondition->getCustomData()==-1) return false;
		if (pCondition->getCustomData()==1) return true;
	}

	ObjectTypesTemp types;
	objectTypesFromParam(pTypeParm, types.m_types);

	Int count = 0;
	for (it = pPlayer->getPlayerTeams()->begin(); it != pPlayer->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) {
				continue;
			}
			for (DLINK_ITERATOR<Object> iter = team->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
				Object *pObj = iter.cur();
				if (!pObj) {
					continue;
				}

				if (types.m_types->isInSet(pObj->getTemplate())) {
					if (pObj->isInside(pTrig)) {

						//
						// dead objects will not be considered, except crates ... they are "dead" cause
						// they have no body and health, but are a class of object we want to
						// trigger this stuff
						//
						if (!(pObj->isEffectivelyDead() || pObj->isKindOf(KINDOF_INERT)) || pObj->isKindOf( KINDOF_CRATE ) ) {
							count++;
						}
					}
				}
			}
		}
	}

	Bool comparison = false;
	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN :			comparison = (count < pCountParm->getInt()); break;
		case Parameter::LESS_EQUAL :		comparison = (count <= pCountParm->getInt()); break;
		case Parameter::EQUAL :					comparison = (count == pCountParm->getInt()); break;
		case Parameter::GREATER_EQUAL :	comparison = (count >= pCountParm->getInt()); break;
		case Parameter::GREATER :				comparison = (count > pCountParm->getInt()); break;
		case Parameter::NOT_EQUAL :			comparison = (count != pCountParm->getInt()); break;
	}
	pCondition->setCustomData(-1); // false.
	if (comparison) {
		pCondition->setCustomData(1); // true.
	}
	return comparison;
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasUnitKindInArea */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasUnitKindInArea(Condition *pCondition, Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm, Parameter *pKindParm, Parameter *pTriggerParm )
{
	AsciiString triggerName = pTriggerParm->getString();
	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());
	if (pTrig == NULL) return false;

	KindOfType kind = (KindOfType)pKindParm->getInt();

	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	Player::PlayerTeamList::const_iterator it;
	Bool anyChanges = false;
	if (pCondition->getCustomData() == 0) anyChanges = true;


	for (it = pPlayer->getPlayerTeams()->begin(); it != pPlayer->getPlayerTeams()->end(); ++it) {
		if (anyChanges) break;
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			if (anyChanges) break;
			Team *team = iter.cur();
			if (!team) {
				continue;
			}
			if (team->didEnterOrExit()) {
				anyChanges = true;
			}
		}
	}
	if (TheScriptEngine->getFrameObjectCountChanged()+1 >= TheGameLogic->getFrame()) {
		anyChanges = true; // Objects were added/deleted last frame, so count could have changed.  jba.
	}
	if (!anyChanges) {
		if (pCondition->getCustomData()==-1) return false;
		if (pCondition->getCustomData()==1) return true;

	}


	Int count = 0;
	for (it = pPlayer->getPlayerTeams()->begin(); it != pPlayer->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) {
				continue;
			}
			for (DLINK_ITERATOR<Object> iter = team->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
				Object *pObj = iter.cur();
				if (!pObj) {
					continue;
				}
				if (pObj->isKindOf(kind)) {
					if (pObj->isInside(pTrig)) {
						if (!(pObj->isEffectivelyDead() || pObj->isKindOf(KINDOF_INERT))) {
							count++;
						}
					}
				}
			}
		}
	}

	Bool comparison = false;
	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN :			comparison = (count < pCountParm->getInt()); break;
		case Parameter::LESS_EQUAL :		comparison = (count <= pCountParm->getInt()); break;
		case Parameter::EQUAL :					comparison = (count == pCountParm->getInt()); break;
		case Parameter::GREATER_EQUAL :	comparison = (count >= pCountParm->getInt()); break;
		case Parameter::GREATER :				comparison = (count > pCountParm->getInt()); break;
		case Parameter::NOT_EQUAL :			comparison = (count != pCountParm->getInt()); break;
	}

	return comparison;
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamStateIs */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamStateIs(Parameter *pTeamParm, Parameter *pStateParm )
{
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	// The team is the team based on the name, and the calling team (if any) and the team that
	// is being considered for the condition.  jba. :)
	AsciiString stateName = pStateParm->getString();
	if (theTeam) {
		return (theTeam->getState() == stateName);
	}
	return false; // Non existent team isn't in any state.
}


//-------------------------------------------------------------------------------------------------
/** evaluateTeamStateIsNot */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamStateIsNot(Parameter *pTeamParm, Parameter *pStateParm )
{
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	// The team is the team based on the name, and the calling team (if any) and the team that
	// is being considered for the condition.  jba. :)
	AsciiString stateName = pStateParm->getString();
	if (theTeam) {
		return (!(theTeam->getState() == stateName));
	}
	return false; // Non existent team isn't in any state.
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedOutsideArea */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedOutsideArea(Parameter *pUnitParm, Parameter *pTriggerParm)
{// This is actually NamedUnitInside(...)

	return !evaluateNamedInsideArea(pUnitParm, pTriggerParm);
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamInsideAreaEntirely */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamInsideAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm)
{// This is actually TeamInside(...)
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	// The team is the team based on the name, and the calling team (if any) and the team that
	// is being considered for the condition.  jba. :)
	AsciiString triggerName = pTriggerParm->getString();
	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (pTrig == NULL)
		return false;

	if (theTeam) {
		return theTeam->allInside(pTrig, (UnsignedInt)pTypeParm->getInt());
	}
	return false; // Non existent team isn't in trigger area. :)
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamOutsideAreaEntirely */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamOutsideAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm)
{
	return !(evaluateTeamInsideAreaEntirely(pTeamParm, pTriggerParm, pTypeParm) ||
					 evaluateTeamInsideAreaPartially(pTeamParm, pTriggerParm, pTypeParm));
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedAttackedByType */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedAttackedByType(Parameter *pUnitParm, Parameter *pTypeParm)
{
	Object *theObj = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (!theObj) {
		return false;
	}

	BodyModuleInterface* theBodyModule = theObj->getBodyModule();
	if (!theBodyModule) {
		return false;
	}

	const DamageInfo* lastDamageInfo = theBodyModule->getLastDamageInfo();

	if (!lastDamageInfo) {
		return false;
	}

	ObjectID id = lastDamageInfo->in.m_sourceID;
	Object* pAttacker = TheGameLogic->findObjectByID(id);
	if (!pAttacker || !pAttacker->getTemplate()) {
		return false;
	}

	ObjectTypesTemp types;
	objectTypesFromParam(pTypeParm, types.m_types);

	return (types.m_types->isInSet(pAttacker->getTemplate()->getName()));
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamAttackedByType */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamAttackedByType(Parameter *pTeamParm, Parameter *pTypeParm)
{
	Team *theTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!theTeam) {
		return FALSE;
	}

	ObjectTypesTemp types;
	objectTypesFromParam(pTypeParm, types.m_types);

	for (DLINK_ITERATOR<Object> iter = theTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *pCur = iter.cur();
		if (!pCur) {
			continue;
		}

		BodyModuleInterface* theBodyModule = pCur->getBodyModule();
		if (!theBodyModule) {
			continue;
		}

		const DamageInfo* lastDamageInfo = theBodyModule->getLastDamageInfo();

		if (!lastDamageInfo) {
			continue;
		}

		ObjectID id = lastDamageInfo->in.m_sourceID;
		Object* pAttacker = TheGameLogic->findObjectByID(id);
		if (!pAttacker || !pAttacker->getTemplate()) {
			continue;
		}

		if (types.m_types->isInSet(pAttacker->getTemplate()->getName())) {
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedAttackedByPlayer */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedAttackedByPlayer(Parameter *pUnitParm, Parameter *pPlayerParm)
{
	Object *theObj = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (!theObj) {
		return false;
	}

	BodyModuleInterface* theBodyModule = theObj->getBodyModule();
	if (!theBodyModule) {
		return false;
	}

	const DamageInfo* lastDamageInfo = theBodyModule->getLastDamageInfo();

	if (!lastDamageInfo) {
		return false;
	}

	ObjectID id = lastDamageInfo->in.m_sourceID;
	Object* pAttacker = TheGameLogic->findObjectByID(id);
	Player *pPlayer = NULL;
	if (lastDamageInfo->in.m_sourcePlayerMask) {
		pPlayer = ThePlayerList->getPlayerFromMask(lastDamageInfo->in.m_sourcePlayerMask);
	}
	if (pPlayer || pAttacker) {
		Player *victimPlayer = playerFromParam(pPlayerParm);
		if (pPlayer == victimPlayer) {
			return true;
		}
		if (!pAttacker) return false; // wasn't attacked.
		return (pAttacker->getControllingPlayer() == victimPlayer);
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamAttackedByPlayer */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamAttackedByPlayer(Parameter *pTeamParm, Parameter *pPlayerParm)
{
	Team *theTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!theTeam) {
		return false;
	}

	for (DLINK_ITERATOR<Object> iter = theTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *pCur = iter.cur();
		if (!pCur) {
			continue;
		}
		BodyModuleInterface* theBodyModule = pCur->getBodyModule();
		if (!theBodyModule) {
			continue;
		}

		const DamageInfo* lastDamageInfo = theBodyModule->getLastDamageInfo();

		if (!lastDamageInfo) {
			continue;
		}

		ObjectID id = lastDamageInfo->in.m_sourceID;
		Object* pAttacker = TheGameLogic->findObjectByID(id);
		if (!pAttacker) {
			continue;
		}

		if (pAttacker->getControllingPlayer() == playerFromParam(pPlayerParm)) {
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateBuiltByPlayer */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateBuiltByPlayer(Condition *pCondition, Parameter* pTypeParm, Parameter* pPlayerParm)
{
	if (pCondition->getCustomData()!=0) {
		// We have a cached value.
		if (TheScriptEngine->getFrameObjectCountChanged()+1 < TheGameLogic->getFrame()) {
			// object count hasn't changed recently.  Use cached value.
			if (pCondition->getCustomData()==1) return true;
			if (pCondition->getCustomData()==-1) return false;
		}
	}
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	const ThingTemplate* pTemplate = TheThingFactory->findTemplate(pTypeParm->getString());
	if (!pTemplate) {
		return false;
	}

	ObjectTypesTemp types;
	objectTypesFromParam(pTypeParm, types.m_types);

	std::vector<Int> counts;
	std::vector<const ThingTemplate *> templates;

	Int numTemplates = types.m_types->prepForPlayerCounting(templates, counts);
	if (numTemplates > 0) {
		pPlayer->countObjectsByThingTemplate(numTemplates, &(*templates.begin()), false, &(*counts.begin()));
	} else {
		return 0;
	}

	Int sumOfObjs = rts::sum(counts);
	pCondition->setCustomData(-1); // false.
	if (sumOfObjs != 0) {
		pCondition->setCustomData(1); // true.
	}
	return (sumOfObjs != 0);
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedCreated */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedCreated(Parameter* pUnitParm)
{
	// This is actually evaluateNamedExists(...)
	///@todo - evaluate created, not exists...
	return (TheScriptEngine->getUnitNamed(pUnitParm->getString()) != NULL);
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamCreated */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamCreated(Parameter* pTeamParm)
{
	Team *pTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (pTeam) {
		return pTeam->isCreated();
	}
	return ( false );
}

//-------------------------------------------------------------------------------------------------
/** evaluateUnitHealth */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateUnitHealth(Parameter *pUnitParm, Parameter* pComparisonParm, Parameter *pHealthPercent)
{
	Object *theObj = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (!theObj) {
		return false;
	}
	if (!theObj->getBodyModule()) return false;

	Real curHealth = theObj->getBodyModule()->getHealth();
	Real initialHealth = theObj->getBodyModule()->getInitialHealth();
	Int curPercent = (curHealth*100 + initialHealth/2)/initialHealth;

	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN :			return (curPercent < pHealthPercent->getInt());
		case Parameter::LESS_EQUAL :		return (curPercent <= pHealthPercent->getInt());
		case Parameter::EQUAL :					return (curPercent == pHealthPercent->getInt());
		case Parameter::GREATER_EQUAL :	return (curPercent >= pHealthPercent->getInt());
		case Parameter::GREATER :				return (curPercent > pHealthPercent->getInt());
		case Parameter::NOT_EQUAL :			return (curPercent != pHealthPercent->getInt());
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasCredits */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasCredits(Parameter *pCreditsParm, Parameter* pComparisonParm, Parameter *pPlayerParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	if (pPlayer && pPlayer->getMoney()) {
		switch (pComparisonParm->getInt())
		{
			case Parameter::LESS_THAN :			return (pCreditsParm->getInt() < pPlayer->getMoney()->countMoney()); break;
			case Parameter::LESS_EQUAL :		return (pCreditsParm->getInt() <= pPlayer->getMoney()->countMoney()); break;
			case Parameter::EQUAL :					return (pCreditsParm->getInt() == pPlayer->getMoney()->countMoney()); break;
			case Parameter::GREATER_EQUAL :	return (pCreditsParm->getInt() >= pPlayer->getMoney()->countMoney()); break;
			case Parameter::GREATER :				return (pCreditsParm->getInt() > pPlayer->getMoney()->countMoney()); break;
			case Parameter::NOT_EQUAL :			return (pCreditsParm->getInt() != pPlayer->getMoney()->countMoney()); break;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateBuildingEntered */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateBuildingEntered( Parameter *pPlayerParm, Parameter *pItemParm )
{
	Object *theObj = TheScriptEngine->getUnitNamed( pItemParm->getString() );
	if (!theObj) {
		return false;
	}

	ContainModuleInterface *contain = theObj->getContain();
	if( !contain )
	{
		DEBUG_CRASH( ("evaluateBuildingEntered script condition -- building doesn't have a container.") );
		return false;
	}
	PlayerMaskType playerMask = theObj->getContain()->getPlayerWhoEntered();
	if (playerMask==0) {
		return false;
	}

	Player *pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}
	if (playerMask == pPlayer->getPlayerMask()) {
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateIsBuildingEmpty */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateIsBuildingEmpty( Parameter *pItemParm )
{

	Object *theBuilding = TheScriptEngine->getUnitNamed(pItemParm->getString());
	if (!theBuilding) {
		return false;
	}

	ContainModuleInterface* contain = theBuilding->getContain();
	if (!contain) {
		return false;
	}
	if (contain->getContainCount() > 0) {
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------
/** evaluateEnemySighted */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateEnemySighted(Parameter *pItemParm, Parameter *pAllianceParm, Parameter* pPlayerParm)
{

	Object *theObj = TheScriptEngine->getUnitNamed( pItemParm->getString() );
	if (!theObj) {
		return false;
	}

	Player *pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	// filter out appropriately based on alliances
	Int relationDescriber;
	switch (pAllianceParm->getInt()) {
		case Parameter::REL_NEUTRAL:
			relationDescriber = PartitionFilterRelationship::ALLOW_NEUTRAL;
			break;
		case Parameter::REL_FRIEND:
			relationDescriber = PartitionFilterRelationship::ALLOW_ALLIES;
			break;
		case Parameter::REL_ENEMY:
			relationDescriber = PartitionFilterRelationship::ALLOW_ENEMIES;
			break;
		default:
			DEBUG_CRASH(("Unhandled case in ScriptConditions::evaluateEnemySighted()"));
			relationDescriber = 0;
			break;
	}
	PartitionFilterRelationship	filterTeam(theObj, relationDescriber);

	// and only stuff that is not dead
	PartitionFilterAlive filterAlive;

	// and only nonstealthed items.
	PartitionFilterRejectByObjectStatus filterStealth( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_STEALTHED ),
																										 MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_DETECTED ) );

	// and only on-map (or not)
	PartitionFilterSameMapStatus filterMapStatus(theObj);

	PartitionFilter *filters[] = { &filterTeam, &filterAlive, &filterStealth, &filterMapStatus, NULL };

	Real visionRange = theObj->getVisionRange();

	SimpleObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(
								theObj, visionRange, FROM_CENTER_2D, filters);
	MemoryPoolObjectHolder hold(iter);
	for (Object *them = iter->first(); them; them = iter->next())
	{
		if (them->getControllingPlayer() == pPlayer) {
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateTypeSighted */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTypeSighted(Parameter *pItemParm, Parameter *pTypeParm, Parameter* pPlayerParm)
{

	Object *theObj = TheScriptEngine->getUnitNamed( pItemParm->getString() );
	if (!theObj) {
		return false;
	}

	Player *pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	ObjectTypesTemp types;
	objectTypesFromParam(pTypeParm, types.m_types);

	// and only stuff that is not dead
	PartitionFilterAlive filterAlive;

	// and only nonstealthed items.
	PartitionFilterRejectByObjectStatus filterStealth( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_STEALTHED ),
																										 MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_DETECTED ) );

	// and only on-map (or not)
	PartitionFilterSameMapStatus filterMapStatus(theObj);

	PartitionFilter *filters[] = { &filterAlive, &filterStealth, &filterMapStatus, NULL };

	Real visionRange = theObj->getVisionRange();

	SimpleObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(
								theObj, visionRange, FROM_CENTER_2D, filters);
	MemoryPoolObjectHolder hold(iter);
	for (Object *them = iter->first(); them; them = iter->next())
	{
		if (them->getControllingPlayer() == pPlayer) {
			if (types.m_types->isInSet(them->getTemplate()->getName()))
				return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedDiscovered */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedDiscovered(Parameter *pItemParm, Parameter* pPlayerParm)
{
	Object *theObj = TheScriptEngine->getUnitNamed( pItemParm->getString() );
	if (!theObj) {
		return false;
	}

	Player *pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	// We are held, so we are not visible.
	if( theObj->isDisabledByType( DISABLED_HELD ) )
	{
		return false;
	}

	// If we are stealthed we are not visible.
	if( theObj->getStatusBits().test( OBJECT_STATUS_STEALTHED ) &&
			!theObj->getStatusBits().test( OBJECT_STATUS_DETECTED ) )
	{
		return false;
	}
	ObjectShroudStatus shroud = theObj->getShroudedStatus(pPlayer->getPlayerIndex());
	return (shroud == OBJECTSHROUD_CLEAR || shroud == OBJECTSHROUD_PARTIAL_CLEAR);
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamDiscovered */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamDiscovered(Parameter *pTeamParm, Parameter *pPlayerParm)
{
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	if (!theTeam) {
		return false;
	}

	Player *pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	for (DLINK_ITERATOR<Object> iter = theTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *pObj = iter.cur();
		if (!pObj) {
			continue;
		}

		// We are held, so we are not visible.
		if( pObj->isDisabledByType( DISABLED_HELD ) )
		{
			continue;
		}

		// If we are stealthed we are not visible.
		if( pObj->getStatusBits().test( OBJECT_STATUS_STEALTHED ) &&
				!pObj->getStatusBits().test( OBJECT_STATUS_DETECTED ) )
		{
			continue;
		}
		ObjectShroudStatus shroud = pObj->getShroudedStatus(pPlayer->getPlayerIndex());

		if (shroud == OBJECTSHROUD_CLEAR || shroud == OBJECTSHROUD_PARTIAL_CLEAR) {
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateMissionAttempts */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateMissionAttempts(Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pAttemptsParm)
{
//Player* pPlayer = playerFromParam(pPlayerParm);
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedOwnedByPlayer */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedOwnedByPlayer(Parameter *pUnitParm, Parameter *pPlayerParm)
{

	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	Object* pObj = TheScriptEngine->getUnitNamed(pUnitParm->getString());
	if (!pObj) {
		return false;
	}

	return (pObj->getControllingPlayer() == pPlayer);
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamOwnedByPlayer */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamOwnedByPlayer(Parameter *pTeamParm, Parameter *pPlayerParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	Team* pTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!pTeam) {
		return false;
	}

	return (pTeam->getControllingPlayer() == pPlayer);
}


//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasNOrFewerBuildings */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasNOrFewerBuildings(Parameter *pBuildingCountParm, Parameter *pPlayerParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	return (pBuildingCountParm->getInt() >= pPlayer->countBuildings());
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasNOrFewerFactionBuildings */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasNOrFewerFactionBuildings(Parameter *pBuildingCountParm, Parameter *pPlayerParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	KindOfMaskType mask;
	mask.set(KINDOF_MP_COUNT_FOR_VICTORY);
	mask.set(KINDOF_STRUCTURE);
	return (pBuildingCountParm->getInt() >= pPlayer->countObjects(mask, KINDOFMASK_NONE));
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasPower */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasPower(Parameter *pPlayerParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	Energy* pPlayersEnergy = pPlayer->getEnergy();
	if (!pPlayersEnergy) {
		return false;
	}
	return pPlayersEnergy->hasSufficientPower();
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedReachedWaypointsEnd */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedReachedWaypointsEnd(Parameter *pUnitParm, Parameter* pWaypointPathParm)
{
	Object *theObj = TheScriptEngine->getUnitNamed( pUnitParm->getString() );
	if (!theObj) {
		return false;
	}

	AIUpdateInterface *ai = theObj->getAIUpdateInterface();
	if (!ai)
		return false;

	const Waypoint *targetWay = ai->getCompletedWaypoint();

	if (!targetWay) return false;

	AsciiString	pathName = pWaypointPathParm->getString();

	if (targetWay->getPathLabel1() == pathName) return true;
	if (targetWay->getPathLabel2() == pathName) return true;
	if (targetWay->getPathLabel3() == pathName) return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamReachedWaypointsEnd */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamReachedWaypointsEnd(Parameter *pTeamParm, Parameter* pWaypointPathParm)
{
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	if (!theTeam) {
		return false;
	}

	AsciiString	pathName = pWaypointPathParm->getString();
	Bool anyAtEnd = false;
	Bool anyNotAtEnd = false;
	// Note - This returns true if any of the team completed the path.  This is as the current
	// implementation tends to do group pathfinding by default, so we trigger when the leader actually thinks
	// that he has reached the end of the waypoint path.
//int i = 0;
	for (DLINK_ITERATOR<Object> iter = theTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *pObj = iter.cur();
		if (!pObj) {
			continue;
		}
		AIUpdateInterface *ai = pObj->getAIUpdateInterface();
		if (!ai) continue; // in case there are any rocks or trees in the team :)

		const Waypoint *targetWay = ai->getCompletedWaypoint();

		if (!targetWay) {
			anyNotAtEnd = true;
			continue;
		}
		Bool found = false;
		if (targetWay->getPathLabel1() == pathName) found = true;
		if (targetWay->getPathLabel2() == pathName) found = true;
		if (targetWay->getPathLabel3() == pathName) found = true;
		if (found) {
			anyAtEnd = true;
		} else {
			anyNotAtEnd = true;
		}
	}
	return anyAtEnd;
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedSelected */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedSelected(Condition *pCondition, Parameter *pUnitParm)
{
	if (TheGameEngine->isMultiplayerSession())
	{
		return false;
	}


	Bool anyChanges = false;
	if (pCondition->getCustomData() == 0) anyChanges = true;


	if (TheInGameUI->getFrameSelectionChanged()+1 >= TheGameLogic->getFrame()) {
		anyChanges = true; // Objects were selected last frame, so count could have changed.  jba.
	}
	if (!anyChanges) {
		if (pCondition->getCustomData()==-1) return false;
		if (pCondition->getCustomData()==1) return true;
	}

	Bool isSelected = false;
	const DrawableList *selected = TheInGameUI->getAllSelectedDrawables();

	// loop through all the selected drawables
	Drawable *draw;
	for( DrawableListCIt it = selected->begin(); it != selected->end(); ++it )
	{
		draw = *it;

		if (draw->getObject()->getName() == (pUnitParm->getString())) {
			isSelected = true;
			break;
		}

	}

	pCondition->setCustomData(-1); // false.
	if (isSelected) {
		pCondition->setCustomData(1); // true.
	}
	return isSelected;
}

//-------------------------------------------------------------------------------------------------
/** evaluateVideoHasCompleted */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateVideoHasCompleted(Parameter *pVideoParm)
{
	return TheScriptEngine->isVideoComplete(pVideoParm->getString(), true);
}

//-------------------------------------------------------------------------------------------------
/** evaluateSpeechHasCompleted */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSpeechHasCompleted(Parameter *pSpeechParm)
{
	return TheScriptEngine->isSpeechComplete(pSpeechParm->getString(), true);
}

//-------------------------------------------------------------------------------------------------
/** evaluateAudioHasCompleted */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateAudioHasCompleted(Parameter *pAudioParm)
{
	return TheScriptEngine->isAudioComplete(pAudioParm->getString(), true);
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerSpecialPowerFromUnitTriggered */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerSpecialPowerFromUnitTriggered(Parameter *pPlayerParm, Parameter *pSpecialPowerParm, Parameter* pUnitParm)
{
	Player *pPlayer = playerFromParam(pPlayerParm);
	ObjectID sourceID = INVALID_ID;
	if (pUnitParm)
	{
		Object* pUnit = TheScriptEngine->getUnitNamed(pUnitParm->getString());
		if (!pUnit)
		{
			// we cared about the source object, but it is dead.  No sense checking anymore, since we don't know it's objectID anymore. :P
			return FALSE;
		}
		sourceID = pUnit->getID();
	}

	if (pPlayer)
	{
		return TheScriptEngine->isSpecialPowerTriggered(pPlayer->getPlayerIndex(), pSpecialPowerParm->getString(), TRUE, sourceID);
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerSpecialPowerFromUnitMidway */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerSpecialPowerFromUnitMidway(Parameter *pPlayerParm, Parameter *pSpecialPowerParm, Parameter* pUnitParm)
{
	Player *pPlayer = playerFromParam(pPlayerParm);
	ObjectID sourceID = INVALID_ID;
	if (pUnitParm)
	{
		Object* pUnit = TheScriptEngine->getUnitNamed(pUnitParm->getString());
		if (!pUnit)
		{
			// we cared about the source object, but it is dead.  No sense checking anymore, since we don't know it's objectID anymore. :P
			return FALSE;
		}
		sourceID = pUnit->getID();
	}

	if (pPlayer)
	{
		return TheScriptEngine->isSpecialPowerMidway(pPlayer->getPlayerIndex(), pSpecialPowerParm->getString(), TRUE, sourceID);
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerSpecialPowerFromUnitComplete */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerSpecialPowerFromUnitComplete(Parameter *pPlayerParm, Parameter *pSpecialPowerParm, Parameter* pUnitParm)
{
	Player *pPlayer = playerFromParam(pPlayerParm);
	ObjectID sourceID = INVALID_ID;
	if (pUnitParm)
	{
		Object* pUnit = TheScriptEngine->getUnitNamed(pUnitParm->getString());
		if (!pUnit)
		{
			// we cared about the source object, but it is dead.  No sense checking anymore, since we don't know it's objectID anymore. :P
			return FALSE;
		}
		sourceID = pUnit->getID();
	}

	if (pPlayer)
	{
		return TheScriptEngine->isSpecialPowerComplete(pPlayer->getPlayerIndex(), pSpecialPowerParm->getString(), TRUE, sourceID);
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluateUpgradeFromUnitComplete */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateUpgradeFromUnitComplete(Parameter *pPlayerParm, Parameter *pUpgradeParm, Parameter* pUnitParm)
{
	Player *pPlayer = playerFromParam(pPlayerParm);
	ObjectID sourceID = INVALID_ID;
	if (pUnitParm)
	{
		Object* pUnit = TheScriptEngine->getUnitNamed(pUnitParm->getString());
		if (!pUnit)
		{
			// we cared about the source object, but it is dead.  No sense checking anymore, since we don't know it's objectID anymore. :P
			return FALSE;
		}
		sourceID = pUnit->getID();
	}

	if (pPlayer)
	{
		return TheScriptEngine->isUpgradeComplete(pPlayer->getPlayerIndex(), pUpgradeParm->getString(), TRUE, sourceID);
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluateScienceAcquired */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateScienceAcquired(Parameter *pPlayerParm, Parameter *pScienceParm)
{
	Player *pPlayer = playerFromParam(pPlayerParm);
	if (pPlayer)
	{
		ScienceType science = TheScienceStore->getScienceFromInternalName(pScienceParm->getString());
		if (science == SCIENCE_INVALID)
			return FALSE;
		return TheScriptEngine->isScienceAcquired(pPlayer->getPlayerIndex(), science, TRUE);
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluateCanPurchaseScience */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateCanPurchaseScience(Parameter *pPlayerParm, Parameter *pScienceParm)
{
	Player *pPlayer = playerFromParam(pPlayerParm);
	if (pPlayer)
	{
		ScienceType science = TheScienceStore->getScienceFromInternalName(pScienceParm->getString());
		if (science == SCIENCE_INVALID)
			return FALSE;
		return pPlayer->isCapableOfPurchasingScience(science);
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluateSciencePurchasePoints */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSciencePurchasePoints(Parameter *pPlayerParm, Parameter *pSciencePointParm)
{
	Player *pPlayer = playerFromParam(pPlayerParm);
	if (pPlayer)
	{
		Int pointsNeeded = pSciencePointParm->getInt();
		return pPlayer->getSciencePurchasePoints() >= pointsNeeded;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedEnteredArea */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedEnteredArea(Parameter *pUnitParm, Parameter *pTriggerParm)
{
	Object* pUnit = TheScriptEngine->getUnitNamed(pUnitParm->getString());
	if (!pUnit) {
		return false;
	}

	if (pUnit->isKindOf(KINDOF_INERT)) {
		return false;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (!pTrig) {
		return false;
	}

	return (pUnit->didEnter(pTrig));
}

//-------------------------------------------------------------------------------------------------
/** evaluateNamedExitedArea */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateNamedExitedArea(Parameter *pUnitParm, Parameter *pTriggerParm)
{
	Object* pUnit = TheScriptEngine->getUnitNamed(pUnitParm->getString());
	if (!pUnit) {
		return false;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (!pTrig) {
		return false;
	}

	return (pUnit->didExit(pTrig));
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamEnteredAreaEntirely */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamEnteredAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm)
{
	Team* pTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!pTeam) {
		return false;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (pTrig) {
		return pTeam->didAllEnter(pTrig, (UnsignedInt)pTypeParm->getInt());
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamEnteredAreaPartially */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamEnteredAreaPartially(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm)
{
	Team* pTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!pTeam) {
		return false;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (pTrig) {
		return pTeam->didPartialEnter(pTrig, (UnsignedInt)pTypeParm->getInt());
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamExitedAreaEntirely */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamExitedAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm)
{
	Team* pTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!pTeam) {
		return false;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (!pTrig) {
		return false;
	}

	return (pTeam->didAllExit(pTrig, (UnsignedInt)pTypeParm->getInt()));
}

//-------------------------------------------------------------------------------------------------
/** evaluateTeamExitedAreaPartially */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamExitedAreaPartially(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm)
{
	Team* pTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!pTeam) {
		return false;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (!pTrig) {
		return false;
	}

	return (pTeam->didPartialExit(pTrig, (UnsignedInt)pTypeParm->getInt()));
}

//-------------------------------------------------------------------------------------------------
/** evaluateMultiplayerAlliedVictory */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateMultiplayerAlliedVictory(void)
{
	return TheVictoryConditions->isLocalAlliedVictory();
}

//-------------------------------------------------------------------------------------------------
/** evaluateMultiplayerAlliedDefeat */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateMultiplayerAlliedDefeat(void)
{
	return TheVictoryConditions->isLocalAlliedDefeat();
}

//-------------------------------------------------------------------------------------------------
/** evaluateMultiplayerPlayerDefeat */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateMultiplayerPlayerDefeat(void)
{
	return TheVictoryConditions->isLocalDefeat() && !TheVictoryConditions->isLocalAlliedDefeat();
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerUnitCondition */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerUnitCondition(Condition *pCondition, Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm, Parameter *pUnitTypeParm)
{
	if (pCondition->getCustomData()!=0) {
		// We have a cached value.
		if (TheScriptEngine->getFrameObjectCountChanged()+1 < TheGameLogic->getFrame()) {
			// object count hasn't changed recently.  Use cached value.
			if (pCondition->getCustomData()==1) return true;
			if (pCondition->getCustomData()==-1) return false;
		}
	}

	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}

	ObjectTypesTemp types;
	objectTypesFromParam(pUnitTypeParm, types.m_types);

	std::vector<Int> counts;
	std::vector<const ThingTemplate *> templates;

	Int numObjs = types.m_types->prepForPlayerCounting(templates, counts);
	Int count = 0;

	if (numObjs > 0) {
		pPlayer->countObjectsByThingTemplate(numObjs, &(*templates.begin()), false, &(*counts.begin()));
		count = rts::sum(counts);
	}

	Bool comparison = false;
	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN :			comparison = (count < pCountParm->getInt()); break;
		case Parameter::LESS_EQUAL :		comparison = (count <= pCountParm->getInt()); break;
		case Parameter::EQUAL :					comparison = (count == pCountParm->getInt()); break;
		case Parameter::GREATER_EQUAL :	comparison = (count >= pCountParm->getInt()); break;
		case Parameter::GREATER :				comparison = (count > pCountParm->getInt()); break;
		case Parameter::NOT_EQUAL :			comparison = (count != pCountParm->getInt()); break;
		default:
			DEBUG_CRASH(("ScriptConditions::evaluatePlayerUnitCondition: Invalid comparison type. (jkmcd)"));
			break;
	}
	pCondition->setCustomData(-1); // false.
	if (comparison) {
		pCondition->setCustomData(1); // true.
	}
	return comparison;
}

//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasComparisonPercentPower */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasComparisonPercentPower(Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pPercentParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}
	Real powerRatio = pPlayer->getEnergy()->getEnergySupplyRatio();
	Real testRatio = pPercentParm->getInt()/100.0f;
	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN :			return (powerRatio < testRatio);
		case Parameter::LESS_EQUAL :		return (powerRatio <= testRatio);
		case Parameter::EQUAL :					return (powerRatio == testRatio);
		case Parameter::GREATER_EQUAL :	return (powerRatio >= testRatio);
		case Parameter::GREATER :				return (powerRatio > testRatio);
		case Parameter::NOT_EQUAL:			return (powerRatio != testRatio);
	}
	return false;
}
//-------------------------------------------------------------------------------------------------
/** evaluatePlayerHasComparisonValueExcessPower */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerHasComparisonValueExcessPower(Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pKWHParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	if (!pPlayer) {
		return false;
	}
	Int desiredKilowattExcess = pKWHParm->getInt();
	Int actualKilowats = pPlayer->getEnergy()->getProduction() - pPlayer->getEnergy()->getConsumption();
	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN :			return (actualKilowats < desiredKilowattExcess);
		case Parameter::LESS_EQUAL :		return (actualKilowats <= desiredKilowattExcess);
		case Parameter::EQUAL :					return (actualKilowats == desiredKilowattExcess);
		case Parameter::GREATER_EQUAL :	return (actualKilowats >= desiredKilowattExcess);
		case Parameter::GREATER :				return (actualKilowats > desiredKilowattExcess);
		case Parameter::NOT_EQUAL:			return (actualKilowats != desiredKilowattExcess);
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** evaluateSkirmishSpecialPowerIsReady - does any unit have this special power ready to use? */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishSpecialPowerIsReady(Parameter *pSkirmishPlayerParm, Parameter *pPower)
{
	if (pPower->getInt() == -1) return false;
	if (pPower->getInt()>0 && pPower->getInt()>TheGameLogic->getFrame()) {
		return false;
	}
	Int nextFrame = TheGameLogic->getFrame() + 10*LOGICFRAMES_PER_SECOND;
	const SpecialPowerTemplate *power = TheSpecialPowerStore->findSpecialPowerTemplate(pPower->getString());
	if (power==NULL) {
		pPower->friend_setInt(-1); // flag as never true.
		return false;
	}
	Bool found = false;
	Player::PlayerTeamList::const_iterator it;
	Player *pPlayer = playerFromParam(pSkirmishPlayerParm);
	if (pPlayer==NULL)
		return false;

	for (it = pPlayer->getPlayerTeams()->begin(); it != pPlayer->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) continue;
			for (DLINK_ITERATOR<Object> iter = team->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
				Object *pObj = iter.cur();
				if (!pObj) continue;
				if( pObj->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) || pObj->isDisabled() )
				{
					continue; // can't fire if under construction or disabled.
				}
				SpecialPowerModuleInterface *mod = pObj->getSpecialPowerModule(power);
				if (mod)
				{
					if (!TheSpecialPowerStore->canUseSpecialPower(pObj, power)) {
						continue;
					}
					found = true;
					if (mod->isReady()) return true;
					if (mod->getReadyFrame()<nextFrame) nextFrame = mod->getReadyFrame();
				}

			}
		}
	}
	pPower->friend_setInt(nextFrame);
	return false;
}


//-------------------------------------------------------------------------------------------------
/** evaluatePlayerDestroyedNOrMoreBuildings */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerDestroyedNOrMoreBuildings(Parameter *pPlayerParm, Parameter *pNumParm, Parameter *pOpponentParm)
{
	Player* pPlayer = playerFromParam(pPlayerParm);
	Player* pOpponent = playerFromParam(pOpponentParm);
//Int N = pNumParm->getInt();
	if (!pPlayer || !pOpponent) {
		return false;
	}

	/// @todo CLH implement me!
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** evaluateUnitHasEmptied */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateUnitHasEmptied(Parameter *pUnitParm)
{
	Object *object = TheScriptEngine->getUnitNamed(pUnitParm->getString());
	if (!object) {
		return false;
	}

	// have we checked this one before?
	TransportStatus *stats = s_transportStatuses;
	while (stats) {
		if (stats->m_objID == object->getID()) {
			break;
		}

		stats = stats->m_nextStatus;
	}

	ContainModuleInterface *cmi = object->getContain();
	Int numPeeps = cmi ? cmi->getContainCount() : 0;

	UnsignedInt frameNum = TheGameLogic->getFrame();


	if (stats == NULL)
	{
		TransportStatus *transportStatus = newInstance(TransportStatus);
		transportStatus->m_objID = object->getID();
		transportStatus->m_frameNumber = frameNum;
		transportStatus->m_unitCount = numPeeps;
		transportStatus->m_nextStatus = s_transportStatuses;
		s_transportStatuses = transportStatus;
		return false;
	}

	if (stats->m_frameNumber == frameNum - 1) {
		if (stats->m_unitCount > 0 && numPeeps == 0) {
			// don't actually update the info on this round, because we want to make sure that
			// multiple calls to this in the same frame actually work.
			return true;
		}
	}

	// perform the update.
	stats->m_frameNumber = frameNum;
	stats->m_unitCount = numPeeps;
	return false;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamIsContained(Parameter *pTeamParm, Bool allContained)
{
	Team* pTeam = TheScriptEngine->getTeamNamed(pTeamParm->getString());
	if (!pTeam) {
		return false;
	}

	Bool anyConsidered = FALSE;
	for (DLINK_ITERATOR<Object> iter = pTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *obj = iter.cur();
		if (!obj) {
			continue;
		}

		Bool isContained = (obj->getContainedBy() != NULL);
		if (!isContained) {
			// we could still be exiting, in which case we should pretend like we are contained.

			AIUpdateInterface *ai = obj->getAIUpdateInterface();
			if (ai) {
				isContained = (isContained && (ai->getCurrentStateID() == AI_EXIT));
			}
		}

		if (isContained) {
			if (!allContained) {
				return TRUE;
			}
		} else {
			if (allContained) {
				return FALSE;
			}
		}

		anyConsidered = TRUE;
	}

	if (anyConsidered) {
		return allContained;
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateUnitHasObjectStatus(Parameter *pUnitParm, Parameter *pObjectStatus)
{
	Object *object = TheScriptEngine->getUnitNamed(pUnitParm->getString());
	if (!object) {
		return false;
	}

	return( object->getStatusBits().testForAny( pObjectStatus->getStatus() ) );
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateTeamHasObjectStatus(Parameter *pTeamParm, Parameter *pObjectStatus, Bool entireTeam)
{
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	if (!theTeam) {
		return false;
	}

	for (DLINK_ITERATOR<Object> iter = theTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *pObj = iter.cur();
		if (!pObj) {
			return false;
		}

		ObjectStatusMaskType objStatus = pObjectStatus->getStatus();
		Bool currObjHasStatus = pObj->getStatusBits().testForAny( objStatus );

		if( entireTeam && !currObjHasStatus )
		{
			return false;
		}
		else if( !entireTeam && currObjHasStatus )
		{
			return true;
		}

	}

	if (entireTeam) {
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
// @todo: PERF_EVALUATE Get a perf timer on this. Should we adjust this function so that it runs like
// evaluatePlayerHasUnitKindInArea
// ?
Bool ScriptConditions::evaluateSkirmishValueInArea(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pComparisonParm, Parameter *pMoneyParm, Parameter *pTriggerParm)
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return false;
	}

	AsciiString triggerName = pTriggerParm->getString();
	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());

	if (!pTrig) {
		return false;
	}

	Player::PlayerTeamList::const_iterator it;
	Bool anyChanges = false;
	if (pCondition->getCustomData() == 0) anyChanges = true;


	for (it = player->getPlayerTeams()->begin(); it != player->getPlayerTeams()->end(); ++it) {
		if (anyChanges) break;
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			if (anyChanges) break;
			Team *team = iter.cur();
			if (!team) {
				continue;
			}
			if (team->didEnterOrExit()) {
				anyChanges = true;
			}
		}
	}
	if (TheScriptEngine->getFrameObjectCountChanged()+1 >= TheGameLogic->getFrame()) {
		anyChanges = true; // Objects were added/deleted last frame, so count could have changed.  jba.
	}
	if (!anyChanges) {
		if (pCondition->getCustomData()==-1) return false;
		if (pCondition->getCustomData()==1) return true;
	}
	Int totalCost = 0;
	for (it = player->getPlayerTeams()->begin(); it != player->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) {
				continue;
			}
			for (DLINK_ITERATOR<Object> iter = team->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
				Object *pObj = iter.cur();
				if (!pObj) {
					continue;
				}
				if (!pObj->isKindOf(KINDOF_INERT) && pObj->isInside(pTrig)) {
					if (!pObj->isEffectivelyDead()) {
						const ThingTemplate *tt = pObj->getTemplate();
						if (!tt) {
							continue;
						}
						totalCost += tt->friend_getBuildCost();
					}
				}
			}
		}
	}

	Bool comparison = false;
	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN :			comparison = (totalCost < pMoneyParm->getInt()); break;
		case Parameter::LESS_EQUAL :		comparison = (totalCost <= pMoneyParm->getInt()); break;
		case Parameter::EQUAL :					comparison = (totalCost == pMoneyParm->getInt()); break;
		case Parameter::GREATER_EQUAL :	comparison = (totalCost >= pMoneyParm->getInt()); break;
		case Parameter::GREATER :				comparison = (totalCost > pMoneyParm->getInt()); break;
		case Parameter::NOT_EQUAL :			comparison = (totalCost != pMoneyParm->getInt()); break;
		default:
			DEBUG_CRASH(("ScriptConditions::evaluateSkirmishValueInArea: Invalid comparison type. (jkmcd)"));
			break;
	}
	pCondition->setCustomData(-1); // false.
	if (comparison) {
		pCondition->setCustomData(1); // true.
	}
	return comparison;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerIsFaction(Parameter *pSkirmishPlayerParm, Parameter *pFactionParm)
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return false;
	}

	return (player->getSide() == pFactionParm->getString());
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishSuppliesWithinDistancePerimeter(Parameter *pSkirmishPlayerParm, Parameter *pDistanceParm, Parameter *pLocationParm, Parameter *pValueParm)
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return false;
	}

	PolygonTrigger *trigger = TheScriptEngine->getQualifiedTriggerAreaByName(pLocationParm->getString());
	if (!trigger) {
		return false;
	}

	Coord3D center;
	trigger->getCenterPoint(&center);
	Real distance = trigger->getRadius() + pDistanceParm->getReal();

	Real compareToValue = pValueParm->getReal();

	PartitionFilterAcceptByKindOf f1(MAKE_KINDOF_MASK(KINDOF_STRUCTURE), KINDOFMASK_NONE);
	PartitionFilterPlayerAffiliation f2(player, ALLOW_NEUTRAL, true);
	PartitionFilterOnMap filterMapStatus;

	PartitionFilter *filters[] = { &f1, &f2, &filterMapStatus, 0 };

	SimpleObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(&center, distance, FROM_CENTER_2D, filters, ITER_FASTEST);
	MemoryPoolObjectHolder hold(iter);

	Real maxValue = 0;
	for (Object *them = iter->first(); them; them = iter->next()) {
		static const NameKeyType key_warehouseUpdate = NAMEKEY("SupplyWarehouseDockUpdate");
		SupplyWarehouseDockUpdate *warehouseModule = (SupplyWarehouseDockUpdate*) them->findUpdateModule( key_warehouseUpdate );
		if (!warehouseModule) {
			continue;
		}

		Real value = player->getSupplyBoxValue() * warehouseModule->getBoxesStored();
		if (value > maxValue) {
			maxValue = value;
		}
	}

	return maxValue > compareToValue;
}

//-------------------------------------------------------------------------------------------------
// @todo: PERF_EVALUATE PERF_WARNING
// If this is called multiple times per frame, the cost could add up. This will infrequently (read:
// never) change, so it shouldn't be called very often. jkmcd
Bool ScriptConditions::evaluateSkirmishPlayerTechBuildingWithinDistancePerimeter(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pDistanceParm, Parameter *pLocationParm)
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return false;
	}

	if (pCondition->getCustomData()==1) return true;
	if (pCondition->getCustomData()==-1) return false;

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pLocationParm->getString());
	if (!pTrig) {
		return false;
	}

	Coord3D center;
	pTrig->getCenterPoint(&center);
	Real radius = pTrig->getRadius() + pDistanceParm->getReal();

	PartitionFilterAcceptByKindOf f1(MAKE_KINDOF_MASK(KINDOF_TECH_BUILDING), KINDOFMASK_NONE);
	PartitionFilterPlayerAffiliation f2(player, ALLOW_ALLIES, false);
	PartitionFilterPlayer f3(player, false);	// Don't find your own units, as our affiliation to self is neutral.
	PartitionFilterOnMap filterMapStatus;


	PartitionFilter *filters[] = { &f1, &f2, &f3, &filterMapStatus, 0 };

	Bool comparison = ThePartitionManager->getClosestObject(&center, radius, FROM_CENTER_2D, filters) != NULL;
	pCondition->setCustomData(-1); // false.
	if (comparison) {
		pCondition->setCustomData(1); // true.
	}
	return comparison;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishCommandButtonIsReady( Parameter * /* pSkirmishPlayerParm */, Parameter *pTeamParm, Parameter *pCommandButtonParm, Bool allReady )
{
	// In this one case, the pSkirmishPlayerParm isn't used.
	Team *theTeam = TheScriptEngine->getTeamNamed( pTeamParm->getString() );
	if (!theTeam) {
		return false;
	}

	const CommandButton *commandButton = TheControlBar->findCommandButton( pCommandButtonParm->getString() );
	if( !commandButton ) {
		return false;
	}

	for (DLINK_ITERATOR<Object> iter = theTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *pObj = iter.cur();
		if (commandButton->getSpecialPowerTemplate()) {
			if( !pObj->hasSpecialPower( commandButton->getSpecialPowerTemplate()->getSpecialPowerType() ) ) {
				continue;
			}
		} else if (!commandButton->getUpgradeTemplate()) {
			continue;
		}

		if (commandButton->isReady(pObj)) {
			if (!allReady) {
				return true;
			}
		} else {
			if (allReady) {
				return false;
			}
		}
	}

	return allReady;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishUnownedFactionUnitComparison( Parameter * /*pSkirmishPlayerParm*/, Parameter *pComparisonParm, Parameter *pCountParm )
{
	Player *player = ThePlayerList->getNeutralPlayer();
	if (!player) {
		return FALSE;
	}

	// Note: This looks slow, but in practice it shouldn't be. The neutral player shouldn't really ever
	// have more than one team.
	Int numFactionUnits = 0;
	Player::PlayerTeamList::const_iterator it;
	for (it = player->getPlayerTeams()->begin(); it != player->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) {
				continue;
			}

			for (DLINK_ITERATOR<Object> objIter = team->iterate_TeamMemberList(); !objIter.done(); objIter.advance()) {
				Object *obj = objIter.cur();
				if (!obj) {
					continue;
				}

				if( obj->isDisabledByType( DISABLED_UNMANNED ) )
				{
					++numFactionUnits;
				}
			}
		}
	}

	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN			:	return numFactionUnits < pCountParm->getInt();	break;
		case Parameter::LESS_EQUAL		:	return numFactionUnits <= pCountParm->getInt(); break;
		case Parameter::EQUAL					:	return numFactionUnits == pCountParm->getInt(); break;
		case Parameter::GREATER_EQUAL :	return numFactionUnits >= pCountParm->getInt(); break;
		case Parameter::GREATER				:	return numFactionUnits > pCountParm->getInt();	break;
		case Parameter::NOT_EQUAL			:	return numFactionUnits != pCountParm->getInt();	break;
	}

	DEBUG_CRASH(("ScriptConditions::evaluateSkirmishUnownedFactionUnitComparison: Invalid comparison type. (jkmcd)"));
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerHasPrereqsToBuild( Parameter *pSkirmishPlayerParm, Parameter *pObjectTypeParm )
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}

	ObjectTypesTemp types;
	objectTypesFromParam(pObjectTypeParm, types.m_types);

	return types.m_types->canBuildAny(player);
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerHasComparisonGarrisoned(Parameter *pSkirmishPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm )
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}

	// Note: This looks slow, and probably is.
	// @todo: PERF_EVALUATE
	Int numGarrisonedBuildings = 0;
	Player::PlayerTeamList::const_iterator it;
	for (it = player->getPlayerTeams()->begin(); it != player->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) {
				continue;
			}

			for (DLINK_ITERATOR<Object> objIter = team->iterate_TeamMemberList(); !objIter.done(); objIter.advance()) {
				Object *obj = objIter.cur();
				if (!obj) {
					continue;
				}

				ContainModuleInterface *cmi = obj->getContain();
				if (!cmi) {
					continue;
				}

				if (cmi->isGarrisonable() && cmi->getContainCount() > 0) {
					++numGarrisonedBuildings;
				}
			}
		}
	}

	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN			:	return numGarrisonedBuildings < pCountParm->getInt();	break;
		case Parameter::LESS_EQUAL		:	return numGarrisonedBuildings <= pCountParm->getInt(); break;
		case Parameter::EQUAL					:	return numGarrisonedBuildings == pCountParm->getInt(); break;
		case Parameter::GREATER_EQUAL :	return numGarrisonedBuildings >= pCountParm->getInt(); break;
		case Parameter::GREATER				:	return numGarrisonedBuildings > pCountParm->getInt();	break;
		case Parameter::NOT_EQUAL			:	return numGarrisonedBuildings != pCountParm->getInt();	break;
	}

	DEBUG_CRASH(("ScriptConditions::evaluateSkirmishPlayerHasComparisonGarrisoned: Invalid comparison type. (jkmcd)"));
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerHasComparisonCapturedUnits(Parameter *pSkirmishPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm )
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}

	// Note: This looks slow, and probably is.
	// @todo: PERF_EVALUATE
	Int numCapturedUnits = 0;
	Player::PlayerTeamList::const_iterator it;
	for (it = player->getPlayerTeams()->begin(); it != player->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) {
				continue;
			}

			for (DLINK_ITERATOR<Object> objIter = team->iterate_TeamMemberList(); !objIter.done(); objIter.advance()) {
				Object *obj = objIter.cur();
				if (!obj) {
					continue;
				}

				if (obj->isCaptured()) {
					++numCapturedUnits;
				}
			}
		}
	}

	switch (pComparisonParm->getInt())
	{
		case Parameter::LESS_THAN			:	return numCapturedUnits < pCountParm->getInt();	break;
		case Parameter::LESS_EQUAL		:	return numCapturedUnits <= pCountParm->getInt(); break;
		case Parameter::EQUAL					:	return numCapturedUnits == pCountParm->getInt(); break;
		case Parameter::GREATER_EQUAL :	return numCapturedUnits >= pCountParm->getInt(); break;
		case Parameter::GREATER				:	return numCapturedUnits > pCountParm->getInt();	break;
		case Parameter::NOT_EQUAL			:	return numCapturedUnits != pCountParm->getInt();	break;
	}

	DEBUG_CRASH(("ScriptConditions::evaluateSkirmishPlayerHasComparisonCapturedUnits: Invalid comparison type. (jkmcd)"));
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishNamedAreaExists(Parameter *, Parameter *pTriggerParm)
{
	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());
	return (pTrig != NULL);
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerHasUnitsInArea(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pTriggerParm )
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());
	if (!pTrig) {
		return FALSE;
	}

	Player::PlayerTeamList::const_iterator it;
	Bool anyChanges = false;


	if (pCondition->getCustomData() == 0) anyChanges = true;

	for (it = player->getPlayerTeams()->begin(); it != player->getPlayerTeams()->end(); ++it) {
		if (anyChanges) break;
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			if (anyChanges) break;
			Team *team = iter.cur();
			if (!team) {
				continue;
			}
			if (team->didEnterOrExit()) {
				anyChanges = true;
			}
		}
	}

	if (TheScriptEngine->getFrameObjectCountChanged()+1 >= TheGameLogic->getFrame()) {
		anyChanges = true; // Objects were added/deleted last frame, so count could have changed.  jba.
	}

	if (!anyChanges) {
		if (pCondition->getCustomData()==-1) return false;
		if (pCondition->getCustomData()==1) return true;
	}

	Coord3D center;
	Real radius = pTrig->getRadius();
	pTrig->getCenterPoint(&center);

	PartitionFilterPlayer f1(player, true);
	PartitionFilterPolygonTrigger f2(pTrig);
	PartitionFilterOnMap filterMapStatus;

	PartitionFilter *filters[] = { &f1, &f2, &filterMapStatus, 0 };

	Object *obj = ThePartitionManager->getClosestObject(&center, radius, FROM_CENTER_2D, filters);
	Bool condition = (obj!=NULL);
	pCondition->setCustomData(-1); // false.
	if (condition) {
		pCondition->setCustomData(1); // true.
	}
	return condition;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishSupplySourceSafe(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pMinSupplyAmount )
{
	// Trigger every 32 frames on the 31st frame. jba.
	Bool anyChanges = (TheGameLogic->getFrame() & 0x1F) == 0x1F;
	if (!anyChanges) {
		if (pCondition->getCustomData()==-1) return false;
		if (pCondition->getCustomData()==1) return true;
	}
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}
	Bool isSafe = player->isSupplySourceSafe(pMinSupplyAmount->getInt());
	pCondition->setCustomData(-1); // false.
	if (isSafe) {
		pCondition->setCustomData(1); // true.
	}
	return isSafe;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishSupplySourceAttacked(Parameter *pSkirmishPlayerParm)
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}
	return player->isSupplySourceAttacked( );
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishStartPosition(Parameter *pSkirmishPlayerParm, Parameter *pStartNdx)
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}
	Int ndx = pStartNdx->getInt()-1;  // externally 1, 2, 3, internally 0, 1, 2.
	Int startNdx = player->getMpStartIndex();
	return ndx == startNdx;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerHasBeenAttackedByPlayer(Parameter *pSkirmishPlayerParm, Parameter *pAttackedByParm )
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}

	Player *srcPlayer = playerFromParam(pAttackedByParm);
	if (!srcPlayer ) {
		return FALSE;
	}

	return player->getAttackedBy(srcPlayer->getPlayerIndex());
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerIsOutsideArea(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pTriggerParm )
{
	// Even though these will be prechecked in the other function, we want to preflight here as well.
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}

	PolygonTrigger *pTrig = TheScriptEngine->getQualifiedTriggerAreaByName(pTriggerParm->getString());
	if (!pTrig) {
		return FALSE;
	}

	return !evaluateSkirmishPlayerHasUnitsInArea(pCondition, pSkirmishPlayerParm, pTriggerParm);
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateSkirmishPlayerHasDiscoveredPlayer(Parameter *pSkirmishPlayerParm, Parameter *pDiscoveredByParm )
{
	Player *player = playerFromParam(pSkirmishPlayerParm);
	if (!player) {
		return FALSE;
	}

	Player *discoveredBy = playerFromParam(pDiscoveredByParm);
	if (!discoveredBy) {
		return FALSE;
	}

	Int discoveredByIndex = discoveredBy->getPlayerIndex();

	Player::PlayerTeamList::const_iterator it;
	for (it = player->getPlayerTeams()->begin(); it != player->getPlayerTeams()->end(); ++it) {
		for (DLINK_ITERATOR<Team> iter = (*it)->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			Team *team = iter.cur();
			if (!team) {
				continue;
			}

			for (DLINK_ITERATOR<Object> objIter = team->iterate_TeamMemberList(); !objIter.done(); objIter.advance()) {
				Object *obj = objIter.cur();
				if (!obj) {
					continue;
				}

				ObjectShroudStatus shroudStatus = obj->getShroudedStatus(discoveredByIndex);
				if (shroudStatus == OBJECTSHROUD_CLEAR || shroudStatus == OBJECTSHROUD_PARTIAL_CLEAR) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateMusicHasCompleted(Parameter *pMusicParm, Parameter *pIntParm)
{
	AsciiString str = pMusicParm->getString();
	return TheAudio->hasMusicTrackCompleted(str, pIntParm->getInt());
}

//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluatePlayerLostObjectType(Parameter *pPlayerParm, Parameter *pTypeParm)
{
	Player *player = playerFromParam(pPlayerParm);
	if (!player) {
		return FALSE;
	}

	ObjectTypesTemp objs;
	objectTypesFromParam(pTypeParm, objs.m_types);

	std::vector<Int> counts;
	std::vector<const ThingTemplate *> templates;

	Int numTemplates = objs.m_types->prepForPlayerCounting(templates, counts);
	if (numTemplates > 0) {
		player->countObjectsByThingTemplate(numTemplates, &(*templates.begin()), true, &(*counts.begin()));
	} else {
		return FALSE;
	}

	Int sumOfObjs = rts::sum(counts);
	Int currentCount = TheScriptEngine->getObjectCount(player->getPlayerIndex(), pTypeParm->getString());

	if (sumOfObjs != currentCount) {
		TheScriptEngine->setObjectCount(player->getPlayerIndex(), pTypeParm->getString(), sumOfObjs);
	}

	return (sumOfObjs < currentCount);
}

//-------------------------------------------------------------------------------------------------
/** Evaluate a condition */
//-------------------------------------------------------------------------------------------------
Bool ScriptConditions::evaluateCondition( Condition *pCondition )
{
	switch (pCondition->getConditionType()) {
		default:
			DEBUG_CRASH(("Unknown ScriptCondition type %d", pCondition->getConditionType()));
			return false;
		case Condition::PLAYER_ALL_DESTROYED:
			return evaluateAllDestroyed(pCondition->getParameter(0));
		case Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED:
			return evaluateAllBuildFacilitiesDestroyed(pCondition->getParameter(0));
		case Condition::TEAM_INSIDE_AREA_PARTIALLY:
			return evaluateTeamInsideAreaPartially(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::NAMED_INSIDE_AREA:
			return evaluateNamedInsideArea(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_DESTROYED:
			return evaluateIsDestroyed(pCondition->getParameter(0));
		case Condition::NAMED_DESTROYED:
			return evaluateNamedUnitDestroyed(pCondition->getParameter(0));
		case Condition::NAMED_DYING:
			return evaluateNamedUnitDying(pCondition->getParameter(0));
		case Condition::NAMED_TOTALLY_DEAD:
			return evaluateNamedUnitTotallyDead(pCondition->getParameter(0));
		case Condition::NAMED_NOT_DESTROYED:
			return evaluateNamedUnitExists(pCondition->getParameter(0));
		case Condition::TEAM_HAS_UNITS:
			return evaluateHasUnits(pCondition->getParameter(0));
		case Condition::CAMERA_MOVEMENT_FINISHED:
			return TheTacticalView->isCameraMovementFinished();
		case Condition::TEAM_STATE_IS:
			return evaluateTeamStateIs(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_STATE_IS_NOT:
			return evaluateTeamStateIsNot(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::NAMED_OUTSIDE_AREA:
			return evaluateNamedOutsideArea(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_INSIDE_AREA_ENTIRELY:
			return evaluateTeamInsideAreaEntirely(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::TEAM_OUTSIDE_AREA_ENTIRELY:
			return evaluateTeamOutsideAreaEntirely(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::NAMED_ATTACKED_BY_OBJECTTYPE:
			return evaluateNamedAttackedByType(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_ATTACKED_BY_OBJECTTYPE:
			return evaluateTeamAttackedByType(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::NAMED_ATTACKED_BY_PLAYER:
			return evaluateNamedAttackedByPlayer(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_ATTACKED_BY_PLAYER:
			return evaluateTeamAttackedByPlayer(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::BUILT_BY_PLAYER:
			return evaluateBuiltByPlayer(pCondition, pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::NAMED_CREATED:
			return evaluateNamedCreated(pCondition->getParameter(0));
		case Condition::TEAM_CREATED:
			return evaluateTeamCreated(pCondition->getParameter(0));
		case Condition::PLAYER_HAS_CREDITS:
			return evaluatePlayerHasCredits(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::BRIDGE_REPAIRED:
			return evaluateBridgeRepaired(pCondition->getParameter(0));
		case Condition::BRIDGE_BROKEN:
			return evaluateBridgeBroken(pCondition->getParameter(0));
		case Condition::NAMED_DISCOVERED:
			return evaluateNamedDiscovered(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_DISCOVERED:
			return evaluateTeamDiscovered(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::BUILDING_ENTERED_BY_PLAYER:
			return evaluateBuildingEntered(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::ENEMY_SIGHTED:
		{
			Int numParameters = pCondition->getNumParameters();
			DEBUG_ASSERTCRASH(numParameters == 3, ("'Condition: [Unit] Unit has sighted a(n) friendly/neutral/enemy unit belonging to a side.' has too few parameters. Please fix in WB. (jkmcd)"));

			if (numParameters < 3) {
				return false;
			}

			return evaluateEnemySighted(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		}
		case Condition::TYPE_SIGHTED:
			return evaluateTypeSighted(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::NAMED_BUILDING_IS_EMPTY:
			return evaluateIsBuildingEmpty(pCondition->getParameter(0));
		case Condition::MISSION_ATTEMPTS:
			return evaluateMissionAttempts(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::NAMED_OWNED_BY_PLAYER:
			return evaluateNamedOwnedByPlayer(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_OWNED_BY_PLAYER:
			return evaluateTeamOwnedByPlayer(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS:
			return evaluatePlayerHasNOrFewerBuildings(pCondition->getParameter(1), pCondition->getParameter(0));
		case Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS:
			return evaluatePlayerHasNOrFewerFactionBuildings(pCondition->getParameter(1), pCondition->getParameter(0));
		case Condition::PLAYER_HAS_POWER:
			return evaluatePlayerHasPower(pCondition->getParameter(0));
		case Condition::PLAYER_HAS_NO_POWER:
			return !evaluatePlayerHasPower(pCondition->getParameter(0));
		case Condition::NAMED_REACHED_WAYPOINTS_END:
			return evaluateNamedReachedWaypointsEnd(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_REACHED_WAYPOINTS_END:
			return evaluateTeamReachedWaypointsEnd(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::NAMED_SELECTED:
			return evaluateNamedSelected(pCondition, pCondition->getParameter(0));
		case Condition::NAMED_ENTERED_AREA:
			return evaluateNamedEnteredArea(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::NAMED_EXITED_AREA:
			return evaluateNamedExitedArea(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::TEAM_ENTERED_AREA_ENTIRELY:
			return evaluateTeamEnteredAreaEntirely(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::TEAM_ENTERED_AREA_PARTIALLY:
			return evaluateTeamEnteredAreaPartially(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::TEAM_EXITED_AREA_ENTIRELY:
			return evaluateTeamExitedAreaEntirely(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::TEAM_EXITED_AREA_PARTIALLY:
			return evaluateTeamExitedAreaPartially(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::MULTIPLAYER_ALLIED_VICTORY:
			return evaluateMultiplayerAlliedVictory();
		case Condition::MULTIPLAYER_ALLIED_DEFEAT:
			return evaluateMultiplayerAlliedDefeat();
		case Condition::MULTIPLAYER_PLAYER_DEFEAT:
			return evaluateMultiplayerPlayerDefeat();
		case Condition::HAS_FINISHED_VIDEO:
			return evaluateVideoHasCompleted(pCondition->getParameter(0));
		case Condition::HAS_FINISHED_SPEECH:
			return evaluateSpeechHasCompleted(pCondition->getParameter(0));
		case Condition::HAS_FINISHED_AUDIO:
			return evaluateAudioHasCompleted(pCondition->getParameter(0));
		case Condition::UNIT_HEALTH:
			return evaluateUnitHealth(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::PLAYER_TRIGGERED_SPECIAL_POWER:
			return evaluatePlayerSpecialPowerFromUnitTriggered(pCondition->getParameter(0), pCondition->getParameter(1), NULL);
		case Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED:
			return evaluatePlayerSpecialPowerFromUnitTriggered(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::PLAYER_MIDWAY_SPECIAL_POWER:
			return evaluatePlayerSpecialPowerFromUnitMidway(pCondition->getParameter(0), pCondition->getParameter(1), NULL);
		case Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED:
			return evaluatePlayerSpecialPowerFromUnitMidway(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::PLAYER_COMPLETED_SPECIAL_POWER:
			return evaluatePlayerSpecialPowerFromUnitComplete(pCondition->getParameter(0), pCondition->getParameter(1), NULL);
		case Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED:
			return evaluatePlayerSpecialPowerFromUnitComplete(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::PLAYER_ACQUIRED_SCIENCE:
			return evaluateScienceAcquired(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::PLAYER_CAN_PURCHASE_SCIENCE:
			return evaluateCanPurchaseScience(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS:
			return evaluateSciencePurchasePoints(pCondition->getParameter(0), pCondition->getParameter(1));
		case Condition::DEFUNCT_PLAYER_SELECTED_GENERAL:
		case Condition::DEFUNCT_PLAYER_SELECTED_GENERAL_FROM_NAMED:
			DEBUG_CRASH(("PLAYER_SELECTED_GENERAL script conditions are no longer in use"));
			return false;
		case Condition::PLAYER_BUILT_UPGRADE:
			return evaluateUpgradeFromUnitComplete(pCondition->getParameter(0), pCondition->getParameter(1), NULL);
		case Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED:
			return evaluateUpgradeFromUnitComplete(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::PLAYER_HAS_OBJECT_COMPARISON:
			return evaluatePlayerUnitCondition(pCondition, pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2), pCondition->getParameter(3));
		case Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER:
			return evaluatePlayerDestroyedNOrMoreBuildings(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));
		case Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA:
		{
			Int numParameters = pCondition->getNumParameters();
			DEBUG_ASSERTCRASH(numParameters == 5, ("'Condition: [Player] has (comparison) unit type in an area' has too few parameters. Please fix in WB. (jkmcd)"));

			if (numParameters < 5) {
				return false;
			}

			return evaluatePlayerHasUnitTypeInArea(pCondition, pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2), pCondition->getParameter(3), pCondition->getParameter(4));
		}
		case Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA:
		{
			Int numParameters = pCondition->getNumParameters();
			DEBUG_ASSERTCRASH(numParameters == 5, ("'Condition: [Player] has (comparison) kind of unit or structure in an area' has too few parameters. Please fix in WB. (jkmcd)"));

			if (numParameters < 5) {
				return false;
			}

			return evaluatePlayerHasUnitKindInArea(pCondition, pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2), pCondition->getParameter(3), pCondition->getParameter(4));
		}
		case Condition::UNIT_EMPTIED:
			return evaluateUnitHasEmptied(pCondition->getParameter(0));

		case Condition::PLAYER_POWER_COMPARE_PERCENT:
			return evaluatePlayerHasComparisonPercentPower(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));

		case Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE:
			return evaluatePlayerHasComparisonValueExcessPower(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));

		case Condition::SKIRMISH_SPECIAL_POWER_READY:
			return evaluateSkirmishSpecialPowerIsReady(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::UNIT_HAS_OBJECT_STATUS:
			return evaluateUnitHasObjectStatus(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::TEAM_ALL_HAS_OBJECT_STATUS:
			return evaluateTeamHasObjectStatus(pCondition->getParameter(0), pCondition->getParameter(1), true);

		case Condition::TEAM_SOME_HAVE_OBJECT_STATUS:
			return evaluateTeamHasObjectStatus(pCondition->getParameter(0), pCondition->getParameter(1), false);

		case Condition::SKIRMISH_VALUE_IN_AREA:
			return evaluateSkirmishValueInArea(pCondition, pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2), pCondition->getParameter(3));

		case Condition::SKIRMISH_PLAYER_FACTION:
			return evaluateSkirmishPlayerIsFaction(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE:
			return evaluateSkirmishSuppliesWithinDistancePerimeter(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2), pCondition->getParameter(3));

		case Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE:
			return evaluateSkirmishPlayerTechBuildingWithinDistancePerimeter(pCondition, pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));

		case Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL:
			return evaluateSkirmishCommandButtonIsReady(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2), true);

		case Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL:
			return evaluateSkirmishCommandButtonIsReady(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2), false);

		case Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS:
			return evaluateSkirmishUnownedFactionUnitComparison(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));

		case Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD:
			return evaluateSkirmishPlayerHasPrereqsToBuild(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED:
			return evaluateSkirmishPlayerHasComparisonGarrisoned(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));

		case Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS:
			return evaluateSkirmishPlayerHasComparisonCapturedUnits(pCondition->getParameter(0), pCondition->getParameter(1), pCondition->getParameter(2));

		case Condition::SKIRMISH_NAMED_AREA_EXIST:
			return evaluateSkirmishNamedAreaExists(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA:
			return evaluateSkirmishPlayerHasUnitsInArea(pCondition, pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER:
			return evaluateSkirmishPlayerHasBeenAttackedByPlayer(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA:
			return evaluateSkirmishPlayerIsOutsideArea(pCondition, pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER:
			return evaluateSkirmishPlayerHasDiscoveredPlayer(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::MUSIC_TRACK_HAS_COMPLETED:
			return evaluateMusicHasCompleted(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SUPPLY_SOURCE_SAFE:
			return evaluateSkirmishSupplySourceSafe(pCondition, pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::SUPPLY_SOURCE_ATTACKED:
			return evaluateSkirmishSupplySourceAttacked(pCondition->getParameter(0));

		case Condition::START_POSITION_IS:
			return evaluateSkirmishStartPosition(pCondition->getParameter(0), pCondition->getParameter(1));

		case Condition::PLAYER_LOST_OBJECT_TYPE:
			return evaluatePlayerLostObjectType(pCondition->getParameter(0), pCondition->getParameter(1));


	}
}


