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

// FILE: AITNGuard.cpp
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  AITNGuard.cpp                                                   */
/* Created:    John Ahlquist., 12/20/2002                              */
/* Desc:       // Set up guard tunnel network states for AI                                 */
/* Revision History:                                                         */
/*		12/20/2002 : Initial creation - modified from AIGuard.cpp                                           */
/*---------------------------------------------------------------------------*/
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/PerfTimer.h"
#include "Common/Team.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/AITNGuard.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/CollideModule.h"
#include "Common/TunnelTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/PolygonTrigger.h"

const Real CLOSE_ENOUGH = (25.0f);


static Bool hasAttackedMeAndICanReturnFire( State *thisState, void* /*userData*/ )
{
	Object *obj = thisState->getMachineOwner();
	BodyModuleInterface *bmi = obj ? obj->getBodyModule() : NULL;

	if (!(obj && bmi)) {
		return FALSE;
	}

	if (bmi->getClearableLastAttacker() == INVALID_ID) {
		return FALSE;
	}

	// K. It appears we have a valid aggressor. Find it, and determine if we can attack it, etc.
	Object *target = TheGameLogic->findObjectByID(bmi->getClearableLastAttacker());
	bmi->clearLastAttacker();

	// We use the clearable last attacker because we should continue attacking the guy. But if he
	// stops attacking us, then we want our timer to kick us off of him and make us go attack
	// other units instead.


	if (!target) {
		return FALSE;
	}

	if (obj->getRelationship(target) != ENEMIES) {
		return FALSE;
	}

	// This is a quick test on the target. It will be duplicated in getAbleToAttackSpecificObject,
	// but the payoff is worth the duplication.
	if (target->isEffectivelyDead()) {
		return FALSE;
	}

	CanAttackResult result = obj->getAbleToAttackSpecificObject(ATTACK_NEW_TARGET, target, CMD_FROM_AI);
	if( result == ATTACKRESULT_POSSIBLE || result == ATTACKRESULT_POSSIBLE_AFTER_MOVING )
	{
		return TRUE;
	}
	return FALSE;
}

static Object *findBestTunnel(Player *ownerPlayer, const Coord3D *pos)
{
	if (!ownerPlayer) return NULL; // should never happen, but hey.  jba.
	TunnelTracker *tunnels = ownerPlayer->getTunnelSystem();
	Object *bestTunnel = NULL;
	Real bestDistSqr = 0;
	const std::list<ObjectID> *allTunnels = tunnels->getContainerList();
	for( std::list<ObjectID>::const_iterator iter = allTunnels->begin(); iter != allTunnels->end(); iter++ ) {
		// For each ID, look it up and change its team.  We all get captured together.
		Object *currentTunnel = TheGameLogic->findObjectByID( *iter );
		if( currentTunnel ) {
			Real dx = currentTunnel->getPosition()->x-pos->x;
			Real dy = currentTunnel->getPosition()->y-pos->y;
			Real distSqr = dx*dx+dy*dy;
			if (bestTunnel==NULL || distSqr<bestDistSqr) {
				bestDistSqr = distSqr;
				bestTunnel = currentTunnel;
			}
		}
	}
	return bestTunnel;
}


//-- ExitConditions -------------------------------------------------------------------------------
/**
 * This returns true if the conditions specified have been met, false otherwise.
 */
Bool TunnelNetworkExitConditions::shouldExit(const StateMachine* machine) const
{

		if (TheGameLogic->getFrame() >= m_attackGiveUpFrame)
		{
			return true;
		}

	return false;
}


//-- AITNGuardMachine -------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
AITNGuardMachine::AITNGuardMachine( Object *owner ) :
	StateMachine(owner, "AITNGuardMachine"),
	m_nemesisToAttack(INVALID_ID),
	m_guardMode(GUARDMODE_NORMAL)
{
	m_positionToGuard.zero();

	static const StateConditionInfo attackAggressors[] =
	{
		StateConditionInfo(hasAttackedMeAndICanReturnFire, AI_TN_GUARD_ATTACK_AGGRESSOR, NULL),
		StateConditionInfo(NULL, NULL, NULL)	// keep last
	};

	// order matters: first state is the default state.
// srj sez: I made "return" the start state, so that if ordered to guard a position
// that isn't the unit's current position, it moves to that position first.
	defineState( AI_TN_GUARD_RETURN,						newInstance(AITNGuardReturnState)( this ), AI_TN_GUARD_IDLE, AI_TN_GUARD_INNER, attackAggressors );
	defineState( AI_TN_GUARD_IDLE,							newInstance(AITNGuardIdleState)( this ), AI_TN_GUARD_INNER, AI_TN_GUARD_RETURN );
	defineState( AI_TN_GUARD_INNER,						newInstance(AITNGuardInnerState)( this ), AI_TN_GUARD_OUTER, AI_TN_GUARD_OUTER , attackAggressors);
	defineState( AI_TN_GUARD_OUTER,						newInstance(AITNGuardOuterState)( this ), AI_TN_GUARD_GET_CRATE, AI_TN_GUARD_GET_CRATE );
	defineState( AI_TN_GUARD_GET_CRATE,				newInstance(AITNGuardPickUpCrateState)( this ), AI_TN_GUARD_RETURN, AI_TN_GUARD_RETURN );
	defineState( AI_TN_GUARD_ATTACK_AGGRESSOR, newInstance(AITNGuardAttackAggressorState)( this ), AI_TN_GUARD_RETURN, AI_TN_GUARD_RETURN  );
#ifdef STATE_MACHINE_DEBUG
	setDebugOutput(true);
#endif
}

//--------------------------------------------------------------------------------------
AITNGuardMachine::~AITNGuardMachine()
{
}

//--------------------------------------------------------------------------------------
/*static*/ Real AITNGuardMachine::getStdGuardRange(const Object* obj)
{
	Real visionRange = TheAI->getAdjustedVisionRangeForObject(obj,
		AI_VISIONFACTOR_OWNERTYPE | AI_VISIONFACTOR_MOOD | AI_VISIONFACTOR_GUARDINNER);

	return visionRange;
}

//--------------------------------------------------------------------------------------
Bool AITNGuardMachine::lookForInnerTarget(void)
{
	Object* owner = getOwner();

	// Check if team auto targets same victim.
	Object *teamVictim = NULL;
	if (owner->getTeam()->getPrototype()->getTemplateInfo()->m_attackCommonTarget)
	{
		teamVictim = owner->getTeam()->getTeamTargetObject();
		if (teamVictim)
		{
			setNemesisID(teamVictim->getID());
			return true;	// Transitions to AITNGuardInnerState.
		}
	}

	// Find tunnel network to defend.
	// Scan my tunnels.
	Player *ownerPlayer = getOwner()->getControllingPlayer();
	if (!ownerPlayer) return false; // should never happen, but hey.  jba.
	TunnelTracker *tunnels = ownerPlayer->getTunnelSystem();
	if (tunnels==NULL) return false;
	if (tunnels->getCurNemesis()) {
		setNemesisID(tunnels->getCurNemesis()->getID());
		return true;	// Transitions to AITNGuardInnerState.
	}
	const std::list<ObjectID> *allTunnels = tunnels->getContainerList();
	for( std::list<ObjectID>::const_iterator iter = allTunnels->begin(); iter != allTunnels->end(); iter++ ) {
		Object *currentTunnel = TheGameLogic->findObjectByID( *iter );
		if( currentTunnel ) {
			// Check for attacking.
			if (currentTunnel->getAI()) {
				Object *victim = currentTunnel->getAI()->getGoalObject();
				if (owner->getRelationship(victim) == ENEMIES) {
					setNemesisID(victim->getID());
					return true;
				}
			}
			// check for attacked.
			BodyModuleInterface *body = currentTunnel->getBodyModule();
			if (body) {
				const DamageInfo *info = body->getLastDamageInfo();
				if (info) {
					if (info->out.m_noEffect) {
						continue;
					}
					if (body->getLastDamageTimestamp() + TheAI->getAiData()->m_guardEnemyScanRate > TheGameLogic->getFrame()) {
						// winner.
						ObjectID attackerID = info->in.m_sourceID;
						Object *attacker = TheGameLogic->findObjectByID(attackerID);

						if( attacker )
						{
							if (owner->getRelationship(attacker) != ENEMIES) {
								continue;
							}
							CanAttackResult result = getOwner()->getAbleToAttackSpecificObject(ATTACK_TUNNEL_NETWORK_GUARD, attacker, CMD_FROM_AI);
							if( result == ATTACKRESULT_POSSIBLE || result == ATTACKRESULT_POSSIBLE_AFTER_MOVING )
							{
								setNemesisID(attackerID);
								owner->getTeam()->setTeamTargetObject(attacker);
								tunnels->updateNemesis(attacker);
								return true;	// Transitions to AITNGuardInnerState.
							}
						}
					}
				}
			}
		}
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AITNGuardMachine::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void AITNGuardMachine::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 2;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

	if (version>=2) {
		StateMachine::xfer(xfer);	// Forgot this in initial implementation.  jba.
	}

	xfer->xferObjectID(&m_nemesisToAttack);
	xfer->xferCoord3D(&m_positionToGuard);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AITNGuardMachine::loadPostProcess( void )
{
}  // end loadPostProcess

//-- AITNGuardInnerState ----------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AITNGuardInnerState::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void AITNGuardInnerState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AITNGuardInnerState::loadPostProcess( void )
{
	onEnter();
}  // end loadPostProcess

// ------------------------------------------------------------------------------------------------
AITNGuardInnerState::~AITNGuardInnerState(void)
{
	deleteInstance(m_attackState);
}

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardInnerState::onEnter( void )
{
	Object* nemesis = TheGameLogic->findObjectByID(getGuardMachine()->getNemesisID()) ;
	if (nemesis == NULL)
	{
		DEBUG_LOG(("Unexpected NULL nemesis in AITNGuardInnerState."));
		return STATE_SUCCESS;
	}
	m_exitConditions.m_attackGiveUpFrame = TheGameLogic->getFrame() + TheAI->getAiData()->m_guardChaseUnitFrames;

	m_attackState = newInstance(AIAttackState)(getMachine(), false, true, false, &m_exitConditions);

	m_attackState->getMachine()->setGoalObject(nemesis);

	StateReturnType returnVal = m_attackState->onEnter();
	if (returnVal == STATE_CONTINUE) {
		return STATE_CONTINUE;
	}

	// if we had no one to attack, we were successful, so go to the next state.
	return STATE_SUCCESS;
}

static Object *TunnelNetworkScan(Object *owner)
{
		PartitionFilterRelationship					f1(owner, PartitionFilterRelationship::ALLOW_ENEMIES);
		PartitionFilterPossibleToAttack			f2(ATTACK_NEW_TARGET, owner, CMD_FROM_AI);
		PartitionFilterSameMapStatus				filterMapStatus(owner);

		PartitionFilter *filters[16];
		Int count = 0;

		filters[count++] = &f1;
		filters[count++] = &f2;
		filters[count++] = &filterMapStatus;

		Real visionRange = AITNGuardMachine::getStdGuardRange(owner);

		filters[count++] = NULL;

		Object* target = ThePartitionManager->getClosestObject(owner->getPosition(), visionRange, FROM_CENTER_2D, filters);
		return target;
}

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardInnerState::update( void )
{
	Object* nemesis = TheGameLogic->findObjectByID(getGuardMachine()->getNemesisID()) ;
	Player *ownerPlayer = getMachineOwner()->getControllingPlayer();
	TunnelTracker *tunnels = NULL;
	if (ownerPlayer) {
		tunnels = ownerPlayer->getTunnelSystem();
	}

	Object* owner = getMachineOwner();
	// killed him.
	Object *teamVictim = owner->getTeam()->getTeamTargetObject();
	if (nemesis == NULL)
	{
		if (teamVictim)
		{
			getGuardMachine()->setNemesisID(teamVictim->getID());
			m_exitConditions.m_attackGiveUpFrame = TheGameLogic->getFrame() + TheAI->getAiData()->m_guardChaseUnitFrames;
			return STATE_CONTINUE;
		}

		// Check tunnel.
		if (tunnels) {
			nemesis = tunnels->getCurNemesis();
			if (nemesis) {
				getGuardMachine()->setNemesisID(nemesis->getID());
				m_exitConditions.m_attackGiveUpFrame = TheGameLogic->getFrame() + TheAI->getAiData()->m_guardChaseUnitFrames;
				return STATE_CONTINUE;
			}
		}

		if (m_scanForEnemy) {
			m_scanForEnemy = false; // we just do 1 scan.
			nemesis = TunnelNetworkScan(owner);
			if (nemesis) {
				m_attackState->onExit(EXIT_RESET);
				m_attackState->getMachine()->setGoalObject(nemesis);
				if (tunnels) {
					tunnels->updateNemesis(nemesis);
				}
				StateReturnType returnVal = m_attackState->onEnter();
				return returnVal;
			}
		}
	} else {
		if (nemesis != teamVictim && teamVictim != NULL) {
			tunnels->updateNemesis(nemesis);
			getGuardMachine()->setNemesisID(teamVictim->getID());
		}
	}
	return m_attackState->update();
}

//--------------------------------------------------------------------------------------
void AITNGuardInnerState::onExit( StateExitType status )
{
	if (m_attackState)
	{
		m_attackState->onExit(status);
		deleteInstance(m_attackState);
		m_attackState = NULL;
	}
}

//-- AITNGuardOuterState ----------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AITNGuardOuterState::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void AITNGuardOuterState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AITNGuardOuterState::loadPostProcess( void )
{						 AITNGuardOuterState
	onEnter();
}  // end loadPostProcess

// ------------------------------------------------------------------------------------------------
AITNGuardOuterState::~AITNGuardOuterState(void)
{
	deleteInstance(m_attackState);
}

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardOuterState::onEnter( void )
{
	if (getGuardMachine()->getGuardMode() == GUARDMODE_GUARD_WITHOUT_PURSUIT)
	{
		// "patrol" mode does not follow targets outside the guard area.
		return STATE_SUCCESS;
	}

	Object* nemesis = TheGameLogic->findObjectByID(getGuardMachine()->getNemesisID()) ;
	if (nemesis == NULL)
	{
		DEBUG_LOG(("Unexpected NULL nemesis in AITNGuardOuterState."));
		return STATE_SUCCESS;
	}

	m_exitConditions.m_attackGiveUpFrame = TheGameLogic->getFrame() + TheAI->getAiData()->m_guardChaseUnitFrames;
	m_attackState = newInstance(AIAttackState)(getMachine(), false, true, false, &m_exitConditions);
	m_attackState->getMachine()->setGoalObject(nemesis);

	StateReturnType returnVal = m_attackState->onEnter();
	if (returnVal == STATE_CONTINUE) {
		return STATE_CONTINUE;
	}

	// if we had no one to attack, we were successful, so go to the next state.
	return STATE_SUCCESS;
}

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardOuterState::update( void )
{
	Object *owner = getMachineOwner();
	Object* goalObj = m_attackState->getMachineGoalObject();
	if (goalObj)
	{
	}	else {
		Object* nemesis = TheGameLogic->findObjectByID(getGuardMachine()->getNemesisID()) ;
		if (nemesis) {
			goalObj = nemesis;
		}
		// Check if team auto targets same victim.
		Object *teamVictim = NULL;
		if (goalObj == NULL && owner->getTeam()->getPrototype()->getTemplateInfo()->m_attackCommonTarget)
		{
			teamVictim = owner->getTeam()->getTeamTargetObject();
			if (teamVictim)
			{
				goalObj = teamVictim;
			}
			m_attackState->getMachine()->setGoalObject(goalObj);
			return m_attackState->onEnter();
		}
	}

	return m_attackState->update();
}

//--------------------------------------------------------------------------------------
void AITNGuardOuterState::onExit( StateExitType status )
{
	if (m_attackState)
	{
		m_attackState->onExit(status);
		deleteInstance(m_attackState);
		m_attackState = NULL;
	}
}

//-- AITNGuardReturnState ----------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AITNGuardReturnState::crc( Xfer *xfer )
{
	AIEnterState::crc(xfer);
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void AITNGuardReturnState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

	AIEnterState::xfer(xfer);

	xfer->xferUnsignedInt(&m_nextReturnScanTime);
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AITNGuardReturnState::loadPostProcess( void )
{
	AIEnterState::loadPostProcess();
}  // end loadPostProcess

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardReturnState::onEnter( void )
{
	UnsignedInt now = TheGameLogic->getFrame();
	m_nextReturnScanTime = now + GameLogicRandomValue(0, TheAI->getAiData()->m_guardEnemyReturnScanRate);

	if (getMachineOwner()->getContainedBy()) {
		// we are already inside our tunnel.  Return success. jba [8/24/2003]
		return STATE_SUCCESS;
	}
	if (getMachineOwner()->getTeam()) {
		Object *teamVictim = getMachineOwner()->getTeam()->getTeamTargetObject();
		if (teamVictim)	{
			// We have a team target.  Go attack it rather than returning to the tunel. jba [8/24/2003]
			getGuardMachine()->setNemesisID(teamVictim->getID());
			return STATE_FAILURE; // Fail to return goes to inner attack state.
		}
	}
// no, no, no, don't do this in onEnter, unless you like really slow maps. (srj)
//	if (getGuardMachine()->lookForInnerTarget())
//		return STATE_FAILURE; // early termination because we found a target.

	// Find tunnel network to enter.
	// Scan my tunnels.
	Object *bestTunnel = findBestTunnel(getMachineOwner()->getControllingPlayer(), getMachineOwner()->getPosition());
	if (bestTunnel==NULL) return STATE_FAILURE;

	getMachine()->setGoalObject(bestTunnel);
	getMachineOwner()->getAI()->friend_setGoalObject(bestTunnel);

	return AIEnterState::onEnter();
}

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardReturnState::update( void )
{
	Player *ownerPlayer = getMachineOwner()->getControllingPlayer();
	if (getMachineOwner()->getTeam()) {
		Object *teamVictim = getMachineOwner()->getTeam()->getTeamTargetObject();
		if (teamVictim)	{
			getGuardMachine()->setNemesisID(teamVictim->getID());
			return STATE_FAILURE; // Fail to return goes to inner attack state.
		}
	}
	// Check tunnel for target.
	TunnelTracker *tunnels = NULL;
	if (ownerPlayer) {
		tunnels = ownerPlayer->getTunnelSystem();
	}

	if (tunnels) {
		Object *nemesis = tunnels->getCurNemesis();
		if (nemesis) {
			// Check distance.
			//Coord3D dist;
			//Coord3D curPos;
			//dist.set()
			getGuardMachine()->setNemesisID(nemesis->getID());
			return STATE_FAILURE; // Fail to return goes to inner attack state.
		}
	}

	// Just let the return movement finish.
	StateReturnType ret = AIEnterState::update();
	if (ret==STATE_CONTINUE) return STATE_CONTINUE;
	return STATE_SUCCESS;

}

//--------------------------------------------------------------------------------------
void AITNGuardReturnState::onExit( StateExitType status )
{
	AIEnterState::onExit( status );
}

//-- AITNGuardIdleState ----------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AITNGuardIdleState::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void AITNGuardIdleState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

	xfer->xferUnsignedInt(&m_nextEnemyScanTime);
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AITNGuardIdleState::loadPostProcess( void )
{
}  // end loadPostProcess

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardIdleState::onEnter( void )
{
	// first time thru, use a random amount so that everyone doesn't scan on the same frame,
	// to avoid "spikes".
	UnsignedInt now = TheGameLogic->getFrame();
	m_nextEnemyScanTime = now + GameLogicRandomValue(0, TheAI->getAiData()->m_guardEnemyScanRate);

	getMachineOwner()->getAI()->friend_setGoalObject(NULL);
	return STATE_CONTINUE;
}

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardIdleState::update( void )
{
	//DEBUG_LOG(("AITNGuardIdleState frame %d: %08lx",TheGameLogic->getFrame(),getMachineOwner()));

	UnsignedInt now = TheGameLogic->getFrame();
	if (now < m_nextEnemyScanTime)
		return STATE_SLEEP(m_nextEnemyScanTime - now);

	m_nextEnemyScanTime = now + TheAI->getAiData()->m_guardEnemyScanRate;

	getMachineOwner()->getAI()->friend_setGoalObject(NULL);

#ifdef STATE_MACHINE_DEBUG
	//getMachine()->setDebugOutput(true);
#endif
	Object *owner = getMachineOwner();
	AIUpdateInterface *ai = owner->getAIUpdateInterface();
	// Check to see if we have created a crate we need to pick up.
	if (ai->getCrateID() != INVALID_ID)
	{
		getMachine()->setState(AI_TN_GUARD_GET_CRATE);
		return STATE_SLEEP(m_nextEnemyScanTime - now);
	}

	// if anyone is in the inner area, return success.
	if (getGuardMachine()->lookForInnerTarget())
	{
		Object *nemesis = TheGameLogic->findObjectByID(getGuardMachine()->getNemesisID());
		if (nemesis == NULL)
		{
			DEBUG_LOG(("Unexpected NULL nemesis in AITNGuardAttackAggressorState."));
			return STATE_SLEEP(0);
		}
		if (getMachineOwner()->getContainedBy()) {
			Object *bestTunnel = findBestTunnel(owner->getControllingPlayer(), nemesis->getPosition());
			ExitInterface* goalExitInterface = bestTunnel->getContain() ? bestTunnel->getContain()->getContainExitInterface() : NULL;
			if( goalExitInterface == NULL )
				return STATE_FAILURE;

			if( goalExitInterface->isExitBusy() )
				return STATE_SLEEP(0);// Just wait a sec.
			goalExitInterface->exitObjectInAHurry(getMachineOwner());
			return STATE_SLEEP(0);
		}
		return STATE_SUCCESS;	// Transitions to AITNGuardInnerState.
	}

	if (!owner->getContainedBy() && findBestTunnel(owner->getControllingPlayer(), owner->getPosition())) {
		return STATE_FAILURE;	 // go to AITNGuardReturnState, & enter a tunnel.
	}

	return STATE_SLEEP(m_nextEnemyScanTime - now);
}

//--------------------------------------------------------------------------------------
void AITNGuardIdleState::onExit( StateExitType status )
{
}

//-- AITNGuardPickUpCrateState ----------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AITNGuardPickUpCrateState::AITNGuardPickUpCrateState( StateMachine *machine )	: AIPickUpCrateState(machine)
{
#ifdef STATE_MACHINE_DEBUG
		setName("AITNGuardPickUpCrateState");
#endif
}
//--------------------------------------------------------------------------------------
StateReturnType AITNGuardPickUpCrateState::onEnter( void )
{
	Object *owner = getMachineOwner();
	AIUpdateInterface *ai = owner->getAIUpdateInterface();

	// Check to see if we have created a crate we need to pick up.
	Object* crate = ai->checkForCrateToPickup();
	if (crate)
	{
		getMachine()->setGoalObject(crate);
		return AIPickUpCrateState::onEnter();
	}

	return STATE_SUCCESS; // no crate, so we're done.
}

//--------------------------------------------------------------------------------------
StateReturnType AITNGuardPickUpCrateState::update( void )
{
	return AIPickUpCrateState::update();
}

//--------------------------------------------------------------------------------------
void AITNGuardPickUpCrateState::onExit( StateExitType status )
{

}

//-- AITNGuardAttackAggressorState ------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AITNGuardAttackAggressorState::AITNGuardAttackAggressorState( StateMachine *machine ) :
	State( machine, "AITNGuardAttackAggressorState" )
{
	m_attackState = NULL;
}

// ------------------------------------------------------------------------------------------------
AITNGuardAttackAggressorState::~AITNGuardAttackAggressorState(void)
{
	deleteInstance(m_attackState);
}

//-------------------------------------------------------------------------------------------------
StateReturnType AITNGuardAttackAggressorState::onEnter( void )
{
	Object *obj = getMachineOwner();
	ObjectID nemID = INVALID_ID;

	if (obj->getBodyModule() && obj->getBodyModule()->getLastDamageInfo()->in.m_sourceID) {
		nemID = obj->getBodyModule()->getLastDamageInfo()->in.m_sourceID;
		getGuardMachine()->setNemesisID(nemID);

	}

	Object *nemesis = TheGameLogic->findObjectByID(getGuardMachine()->getNemesisID());
	if (nemesis == NULL)
	{
		DEBUG_LOG(("Unexpected NULL nemesis in AITNGuardAttackAggressorState."));
		return STATE_SUCCESS;
	}

	Player *ownerPlayer = getMachineOwner()->getControllingPlayer();
	TunnelTracker *tunnels = NULL;
	if (ownerPlayer) {
		tunnels = ownerPlayer->getTunnelSystem();
	}
	if (tunnels) tunnels->updateNemesis(nemesis);

	m_exitConditions.m_attackGiveUpFrame = TheGameLogic->getFrame() + TheAI->getAiData()->m_guardChaseUnitFrames;
	m_attackState = newInstance(AIAttackState)(getMachine(), true, true, false, &m_exitConditions);
	m_attackState->getMachine()->setGoalObject(nemesis);

	StateReturnType returnVal = m_attackState->onEnter();
	if (returnVal == STATE_CONTINUE) {
		return STATE_CONTINUE;
	}

	// if we had no one to attack, we were successful, so go to the next state.
	return STATE_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
StateReturnType AITNGuardAttackAggressorState::update( void )
{
	if (m_attackState->getMachine()->getCurrentStateID() == AttackStateMachine::FIRE_WEAPON) {
		Object *nemesis = TheGameLogic->findObjectByID(getGuardMachine()->getNemesisID());
		Player *ownerPlayer = getMachineOwner()->getControllingPlayer();
		TunnelTracker *tunnels = NULL;
		if (ownerPlayer) {
			tunnels = ownerPlayer->getTunnelSystem();
		}
		if (tunnels) tunnels->updateNemesis(nemesis);
	}
	return m_attackState->update();
}

//-------------------------------------------------------------------------------------------------
void AITNGuardAttackAggressorState::onExit( StateExitType status )
{
	Object *obj = getMachineOwner();
	if (m_attackState)
	{
		m_attackState->onExit(status);
		deleteInstance(m_attackState);
		m_attackState = NULL;
	}

	if (obj->getTeam())
	{
		obj->getTeam()->setTeamTargetObject(NULL); // clear the target.
	}
}

//-------------------------------------------------------------------------------------------------
void AITNGuardAttackAggressorState::crc( Xfer *xfer )
{

}

//-------------------------------------------------------------------------------------------------
void AITNGuardAttackAggressorState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

}

//-------------------------------------------------------------------------------------------------
void AITNGuardAttackAggressorState::loadPostProcess()
{
	onEnter();
}

