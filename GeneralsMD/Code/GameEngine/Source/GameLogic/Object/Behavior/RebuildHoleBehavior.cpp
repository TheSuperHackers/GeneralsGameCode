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

// FILE: RebuildHoleBehavior.cpp //////////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   GLA Hole behavior that reconstructs a building after death
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/GameState.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/RebuildHoleBehavior.h"
#include "GameLogic/Module/StickyBombUpdate.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RebuildHoleBehaviorModuleData::RebuildHoleBehaviorModuleData( void )
{

	m_workerRespawnDelay = 0.0f;
	m_holeHealthRegenPercentPerSecond = 0.1f;

}  // end RebuildHoleBehaviorModuleData

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void RebuildHoleBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p ) 
{

  UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{
	  { "WorkerObjectName", INI::parseAsciiString, NULL, offsetof( RebuildHoleBehaviorModuleData, m_workerTemplateName ) },
		{ "WorkerRespawnDelay", INI::parseDurationReal,	NULL, offsetof( RebuildHoleBehaviorModuleData, m_workerRespawnDelay ) },
		{ "HoleHealthRegen%PerSecond", INI::parsePercentToReal, NULL, offsetof( RebuildHoleBehaviorModuleData, m_holeHealthRegenPercentPerSecond ) },
		{ 0, 0, 0, 0 }
	};

  p.add( dataFieldParse );

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RebuildHoleBehavior::RebuildHoleBehavior( Thing *thing, const ModuleData* moduleData ) 
									 : UpdateModule( thing, moduleData )
{

	m_workerID = INVALID_ID;
	m_reconstructingID = INVALID_ID;
	m_spawnerObjectID = INVALID_ID;
	m_workerWaitCounter = 0;
	m_workerTemplate = NULL;
	m_rebuildTemplate = NULL;

}  // end RebuildHoleBehavior

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RebuildHoleBehavior::~RebuildHoleBehavior( void )
{
	// ensure that our generated worker is destroyed,
	// just in case someone decides to destroy (not kill) us...
	if( m_workerID != INVALID_ID )
	{
		Object *worker = TheGameLogic->findObjectByID(m_workerID);
		if( worker )
		{
			TheGameLogic->destroyObject(worker);
			m_workerID = INVALID_ID;
		}
	}

}  // end ~RebuildHoleBehavior

// ------------------------------------------------------------------------------------------------
/** we need to start all the timers and ID ties to make a new worker at the correct time */
// ------------------------------------------------------------------------------------------------
void RebuildHoleBehavior::newWorkerRespawnProcess( Object *existingWorker )
{
	const RebuildHoleBehaviorModuleData *modData = getRebuildHoleBehaviorModuleData();

	// if we have an existing worker, get rid of it
	if( existingWorker )
	{
		DEBUG_ASSERTCRASH(existingWorker->getID() == m_workerID, ("m_workerID mismatch in RebuildHole"));
		TheGameLogic->destroyObject( existingWorker );
	}
	m_workerID = INVALID_ID;
	
	// set the timer for the next worker respawn
	m_workerWaitCounter = modData->m_workerRespawnDelay;

	//
	// this method is called when a worker needs to be respawned from the hole.  One of those
	// situations is where the building was killed.  Since during building reconstruction
	// we made the hole "effectively not here" we will always want to make the hole
	// "here again" (able to be selected, targeted by the AI etc) because it's the
	// "focus" of this small area again
	//
	getObject()->maskObject( FALSE );
			
}  // end newWorkerRespawnProcess

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RebuildHoleBehavior::startRebuildProcess( const ThingTemplate *rebuild, ObjectID spawnerID )
{

	// save what we're gonna do
	m_rebuildTemplate = rebuild; 

	// store the object that spawned this hole (even though it's likely being destroyed)
	m_spawnerObjectID = spawnerID;

	// start the spawning process for a worker
	Object *worker = TheGameLogic->findObjectByID(m_workerID);
	newWorkerRespawnProcess( worker ); //Kill the worker if we have one.

} /// end startRebuildProcess


//----------------------------------------------------------------------------------------------
void RebuildHoleBehavior::transferBombs( Object *reconstruction )
{

	Object *self = getObject();

	Object *obj = TheGameLogic->getFirstObject();
	while( obj )
	{
		if( obj->isKindOf( KINDOF_MINE ) )
		{
			static NameKeyType key_StickyBombUpdate = NAMEKEY( "StickyBombUpdate" );
			StickyBombUpdate *update = (StickyBombUpdate*)obj->findUpdateModule( key_StickyBombUpdate );
			if( update && update->getTargetObject() == self )
			{
				update->setTargetObject( reconstruction );
			}
		}
		obj = obj->getNextObject();
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime RebuildHoleBehavior::update( void )
{
	const RebuildHoleBehaviorModuleData *modData = getRebuildHoleBehaviorModuleData();
	Object *hole = getObject();
	Object *reconstructing = NULL;
	Object *worker = NULL;

	// get the worker object if we have one
	if( m_workerID != 0 )
	{

		// get the worker
		worker = TheGameLogic->findObjectByID( m_workerID );

		// if the worker is no longer there, start the respawning process for a worker again
		if( worker == NULL )
			newWorkerRespawnProcess( NULL );

	}  // end if

	// if we have a reconstructing object built, get the actual object pointer
	if( m_reconstructingID != 0 )
	{

		// get object pointer
		reconstructing = TheGameLogic->findObjectByID( m_reconstructingID );

		//
		// if that object does not exist anymore, we need to kill a worker if we have one
		// and start the spawning process over again
		//
		if( reconstructing == NULL )
		{
			newWorkerRespawnProcess( worker );
			m_reconstructingID = INVALID_ID;
		}  // end if

	}  // end if

	// see if it's time for us to spawn a worker
	if( worker == NULL && m_workerWaitCounter > 0 )
	{

		// decrement counter and respawn if it's time
		if( --m_workerWaitCounter == 0 )
		{

			// resolve the worker template pointer if necessary
			if( m_workerTemplate == NULL )
				m_workerTemplate = TheThingFactory->findTemplate( modData->m_workerTemplateName );

			// create a worker
			worker = TheThingFactory->newObject( m_workerTemplate, hole->getTeam() );
			if( worker )
			{

				// set the position of the worker to that of the hole
				worker->setPosition( hole->getPosition() );

				// save the ID of the worker spawned
				m_workerID = worker->getID();

				//
				// tell the worker to begin construction of a building if one does not
				// exist yet.  If one does, have construction resume
				//
				AIUpdateInterface *ai = worker->getAIUpdateInterface();
				if( ai )
				{

					if( reconstructing == NULL )
						reconstructing = ai->construct( m_rebuildTemplate, 
																						hole->getPosition(), 
																						hole->getOrientation(), 
																						hole->getControllingPlayer(),
																						TRUE );
					else
						ai->aiResumeConstruction( reconstructing, CMD_FROM_AI );
					
					for ( Object *obj = TheGameLogic->getFirstObject(); obj; obj = obj->getNextObject() )
					{
						// Just like the building transfers attackers to the hole when it creates us, we need to transfer
						// attackers to our replacement building before we mask ourselves.
						AIUpdateInterface* ai = obj->getAI();
						if (!ai)
							continue;

						ai->transferAttack(hole->getID(), reconstructing->getID());
					}
											
					// save the id of what we are reconstructing
					m_reconstructingID = reconstructing->getID();

					//Kris: Hacking the building to set the hole as the producer... so if the site dies, we 
					//can transfer the attack back to the hole. The object has OBJECT_STATUS_RECONSTRUCTING
					//which we check when the object dies.
					reconstructing->setProducer( hole );

					// we want to prevent the player from selecting and doing things with this worker
					worker->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_UNSELECTABLE ) );

					//
					// we want to prevent the player and the AI from selecting or targeting the hole
					// cause the focus in this area (while reconstruction is happening) is the
					// actual reconstructing building
					//
					hole->maskObject( TRUE );

					transferBombs( reconstructing );
					
				}  // end if

			}  // end if, worker

		}  // end if, time to spawn a worker

	}  // end if, check for working respawn

	// holes get auto-healed when they're sittin around
	BodyModuleInterface *body = hole->getBodyModule();
	if( body->getHealth() < body->getMaxHealth() )
	{
		DamageInfo healingInfo;

		// do some healing
		healingInfo.in.m_amount = (modData->m_holeHealthRegenPercentPerSecond / LOGICFRAMES_PER_SECOND) * 
															body->getMaxHealth();
		healingInfo.in.m_sourceID = hole->getID();
		healingInfo.in.m_damageType = DAMAGE_HEALING;
		healingInfo.in.m_deathType = DEATH_NONE;
		body->attemptHealing( &healingInfo );

	}  // end if

	// when re-construction is complete, we remove this hole and worker
	if( reconstructing && !reconstructing->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) )
	{
		// Transfer hole name to new building
		TheScriptEngine->transferObjectName( hole->getName(), reconstructing );

		// make the worker go away
		if( worker )
			TheGameLogic->destroyObject( worker );

		// make the hole go away
		TheGameLogic->destroyObject( hole );
		
	}  // end if

	return UPDATE_SLEEP_NONE;

}  // end update

// ------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RebuildHoleBehavior::onDie( const DamageInfo *damageInfo )
{
	if( m_workerID != INVALID_ID )
	{
		// Our rebuilding building and us the hole can be killed in the same frame, which means we may not have 
		// deleted our generated worker since we do that in our update.
		Object *worker = TheGameLogic->findObjectByID(m_workerID);
		if( worker )
		{
			TheGameLogic->destroyObject(worker);
			m_workerID = INVALID_ID;
		}
	}

	Object *obj = getObject();

	// destroy us
	TheGameLogic->destroyObject( obj );

}  // end onDie

// ------------------------------------------------------------------------------------------------
/** Helper method to get interface given an object */
// ------------------------------------------------------------------------------------------------
/*static*/ RebuildHoleBehaviorInterface* RebuildHoleBehavior::getRebuildHoleBehaviorInterfaceFromObject( Object *obj )
{
	RebuildHoleBehaviorInterface *rhbi = NULL;

	if( obj )
	{

		for( BehaviorModule **i = obj->getBehaviorModules(); *i; ++i )
		{

			rhbi = (*i)->getRebuildHoleBehaviorInterface();
			if( rhbi )
				break;  // exit for

		}  // end for i

	}  // end if, obj

	return rhbi;

}  // end getRebuildHoleBehaviorInterfaceFromObject

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RebuildHoleBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version,
	* 2: Added spawner id */
// ------------------------------------------------------------------------------------------------
void RebuildHoleBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// worker ID
	xfer->xferObjectID( &m_workerID );

	// reconstructing id
	xfer->xferObjectID( &m_reconstructingID );

	// spawner ID
	if( version >= 2 )
		xfer->xferObjectID( &m_spawnerObjectID );

	// worker wait counter
	xfer->xferUnsignedInt( &m_workerWaitCounter );

	// worker template
	AsciiString workerName = m_workerTemplate ? m_workerTemplate->getName() : AsciiString::TheEmptyString;
	xfer->xferAsciiString( &workerName );
	if( xfer->getXferMode() == XFER_LOAD )
	{

		if( workerName != AsciiString::TheEmptyString )
		{

			m_workerTemplate = TheThingFactory->findTemplate( workerName );
			if( m_workerTemplate == NULL )
			{

				DEBUG_CRASH(( "RebuildHoleBehavior::xfer - Unable to find template '%s'\n",
											workerName.str() ));
				throw SC_INVALID_DATA;

			}  // end if

		}  // end if
		else
			m_workerTemplate = NULL;

	}  // end if

	// rebuild template
	AsciiString rebuildName = m_rebuildTemplate ? m_rebuildTemplate->getName() : AsciiString::TheEmptyString;
	xfer->xferAsciiString( &rebuildName );
	if( xfer->getXferMode() == XFER_LOAD )
	{

		if( rebuildName != AsciiString::TheEmptyString )
		{

			m_rebuildTemplate = TheThingFactory->findTemplate( rebuildName );
			if( m_rebuildTemplate == NULL )
			{

				DEBUG_CRASH(( "RebuildHoleBehavior::xfer - Unable to find template '%s'\n",
											rebuildName.str() ));
				throw SC_INVALID_DATA;

			}  // end if

		}  // end if
		else
			m_rebuildTemplate = NULL;

	}  // end if

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RebuildHoleBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
