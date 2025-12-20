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

// FILE: BridgeTowerBehavior.cpp //////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   Behavior module for the towers attached to bridges that can be targeted
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/BridgeBehavior.h"
#include "GameLogic/Module/BridgeTowerBehavior.h"
#include "GameLogic/TerrainLogic.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BridgeTowerBehavior::BridgeTowerBehavior( Thing *thing, const ModuleData *moduleData )
									 : BehaviorModule( thing, moduleData )
{

	m_bridgeID = INVALID_ID;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BridgeTowerBehavior::~BridgeTowerBehavior( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::setBridge( Object *bridge )
{

	if( bridge == NULL )
		m_bridgeID = INVALID_ID;
	else
		m_bridgeID = bridge->getID();

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ObjectID BridgeTowerBehavior::getBridgeID( void )
{

	return m_bridgeID;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::setTowerType( BridgeTowerType type )
{

	m_type = type;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::onDamage( DamageInfo *damageInfo )
{
	Object *bridge = TheGameLogic->findObjectByID( getBridgeID() );

	// sanity
	if( bridge == NULL )
		return;

	//
	// get our body info so we now how much damage percent is being done to us ... we need this
	// so that we can propagate the same damage percentage amont the towers and the bridge
	//
	BodyModuleInterface *body = getObject()->getBodyModule();
	Real damagePercentage = damageInfo->in.m_amount / body->getMaxHealth();

	// get the bridge behavior module for our bridge
	BehaviorModule **bmi;
	BridgeBehaviorInterface *bridgeInterface = NULL;
	for( bmi = bridge->getBehaviorModules(); *bmi; ++bmi )
	{

		bridgeInterface = (*bmi)->getBridgeBehaviorInterface();
		if( bridgeInterface )
			break;

	}
	DEBUG_ASSERTCRASH( bridgeInterface != NULL, ("BridgeTowerBehavior::onDamage - no 'BridgeBehaviorInterface' found") );
	if( bridgeInterface )
	{

		//
		// damage each of the other towers if the source of this damage isn't from the bridge
		// or other towers
		//
		Object *source = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );
		if( source == NULL ||
			  (source->isKindOf( KINDOF_BRIDGE ) == FALSE &&
				 source->isKindOf( KINDOF_BRIDGE_TOWER ) == FALSE) )
		{

			for( Int i = 0; i < BRIDGE_MAX_TOWERS; ++i )
			{
				Object *tower;

				tower = TheGameLogic->findObjectByID( bridgeInterface->getTowerID( (BridgeTowerType)i ) );
				if( tower && tower != getObject() )
				{
					BodyModuleInterface *towerBody = tower->getBodyModule();
					DamageInfo towerDamage;

					towerDamage.in.m_amount = damagePercentage * towerBody->getMaxHealth();
					towerDamage.in.m_sourceID = getObject()->getID();  // we're now the source
					towerDamage.in.m_damageType = damageInfo->in.m_damageType;
					towerDamage.in.m_deathType = damageInfo->in.m_deathType;
					tower->attemptDamage( &towerDamage );

				}

			}

			//
			// damage bridge object, but make sure it's done through the bridge interface
			// so that it doesn't automatically propagate that damage to the towers
			//
			BodyModuleInterface *bridgeBody = bridge->getBodyModule();
			DamageInfo bridgeDamage;

			bridgeDamage.in.m_amount = damagePercentage * bridgeBody->getMaxHealth();
			bridgeDamage.in.m_sourceID = getObject()->getID();  // we're now the source
			bridgeDamage.in.m_damageType = damageInfo->in.m_damageType;
			bridgeDamage.in.m_deathType = damageInfo->in.m_deathType;
			bridge->attemptDamage( &bridgeDamage );

		}

	}

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::onHealing( DamageInfo *damageInfo )
{
	Object *bridge = TheGameLogic->findObjectByID( getBridgeID() );

	// sanity
	if( bridge == NULL )
		return;

	//
	// get our body info so we now how much healing percent is being done to us ... we need this
	// so that we can propagate the same healing percentage amont the towers and the bridge
	//
	BodyModuleInterface *body = getObject()->getBodyModule();
	Real healingPercentage = damageInfo->in.m_amount / body->getMaxHealth();

	// get the bridge behavior module for our bridge
	BehaviorModule **bmi;
	BridgeBehaviorInterface *bridgeInterface = NULL;
	for( bmi = bridge->getBehaviorModules(); *bmi; ++bmi )
	{

		bridgeInterface = (*bmi)->getBridgeBehaviorInterface();
		if( bridgeInterface )
			break;

	}
	DEBUG_ASSERTCRASH( bridgeInterface != NULL, ("BridgeTowerBehavior::onHealing - no 'BridgeBehaviorInterface' found") );
	if( bridgeInterface )
	{

		//
		// heal each of the other towers if the source of this healing isn't from the bridge
		// or other towers
		//
		Object *source = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );
		if( source == NULL ||
			  (source->isKindOf( KINDOF_BRIDGE ) == FALSE &&
				 source->isKindOf( KINDOF_BRIDGE_TOWER ) == FALSE) )
		{

			for( Int i = 0; i < BRIDGE_MAX_TOWERS; ++i )
			{
				Object *tower;

				tower = TheGameLogic->findObjectByID( bridgeInterface->getTowerID( (BridgeTowerType)i ) );
				if( tower && tower != getObject() )
				{
					BodyModuleInterface *towerBody = tower->getBodyModule();
					tower->attemptHealing(healingPercentage * towerBody->getMaxHealth(), getObject());

				}

			}

			//
			// heal bridge object, but make sure it's done through the bridge interface
			// so that it doesn't automatically propagate that healing to the towers.
			//
			BodyModuleInterface *bridgeBody = bridge->getBodyModule();
			bridge->attemptHealing(healingPercentage * bridgeBody->getMaxHealth(), getObject());

		}

	}

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::onBodyDamageStateChange( const DamageInfo* damageInfo,
																									 BodyDamageType oldState,
																									 BodyDamageType newState )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::onDie( const DamageInfo *damageInfo )
{

	// kill the bridge object, this will kill all the towers
	Object *bridge = TheGameLogic->findObjectByID( getBridgeID() );
	if( bridge )
		bridge->kill();

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
/** Given an object, return a bridge tower interface if that object has one */
// ------------------------------------------------------------------------------------------------
BridgeTowerBehaviorInterface *BridgeTowerBehavior::getBridgeTowerBehaviorInterfaceFromObject( Object *obj )
{

	// sanity
	if( obj == NULL || obj->isKindOf( KINDOF_BRIDGE_TOWER ) == FALSE )
		return NULL;

	BehaviorModule **bmi;
	BridgeTowerBehaviorInterface *bridgeTowerInterface = NULL;
	for( bmi = obj->getBehaviorModules(); *bmi; ++bmi )
	{

		bridgeTowerInterface = (*bmi)->getBridgeTowerBehaviorInterface();
		if( bridgeTowerInterface )
			return bridgeTowerInterface;

	}

	// interface not found
	return NULL;

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::crc( Xfer *xfer )
{

	// extend base class
	BehaviorModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	BehaviorModule::xfer( xfer );

	// xfer bridge object ID
	xfer->xferObjectID( &m_bridgeID );

	// xfer tower type
	xfer->xferUser( &m_type, sizeof( BridgeTowerType ) );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BridgeTowerBehavior::loadPostProcess( void )
{

	// extend base class
	BehaviorModule::loadPostProcess();

}
