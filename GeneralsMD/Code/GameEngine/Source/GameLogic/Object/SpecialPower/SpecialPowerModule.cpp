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

// FILE: SpecialPowerModule.cpp ///////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:	 Special power module interface
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameAudio.h"
#include "Common/GlobalData.h"
#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Science.h"
#include "Common/SpecialPower.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/DeletionUpdate.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Module/SpecialPowerUpdateModule.h"
#include "GameLogic/ScriptEngine.h"

#include "GameClient/Eva.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ControlBar.h"


#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerModuleData::SpecialPowerModuleData()
{

	m_specialPowerTemplate = NULL;
	m_updateModuleStartsAttack = false;
	m_startsPaused = FALSE;
	m_scriptedSpecialPowerOnly = FALSE;

}  // end SpecialPowerModuleData

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/* static */ void SpecialPowerModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	BehaviorModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{
		{ "SpecialPowerTemplate",			INI::parseSpecialPowerTemplate, NULL, offsetof( SpecialPowerModuleData, m_specialPowerTemplate ) },
		{ "UpdateModuleStartsAttack", INI::parseBool,									NULL, offsetof( SpecialPowerModuleData, m_updateModuleStartsAttack ) },
		{ "StartsPaused",							INI::parseBool,									NULL, offsetof( SpecialPowerModuleData, m_startsPaused ) },
		{ "InitiateSound",						INI::parseAudioEventRTS,				NULL, offsetof( SpecialPowerModuleData, m_initiateSound ) },
		{ "ScriptedSpecialPowerOnly", INI::parseBool,									NULL, offsetof( SpecialPowerModuleData, m_scriptedSpecialPowerOnly ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerModule::SpecialPowerModule( Thing *thing, const ModuleData *moduleData )
									: BehaviorModule( thing, moduleData )
{

	m_availableOnFrame = 0;
	m_pausedCount = 0;
	m_pausedOnFrame = 0;
	m_pausedPercent = 0.0f;

	// we won't be able to use the power for X number of frames now

	// if we're pre-built, start counting down
	if( !getObject()->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) )
	{
		//A sharedNSync special only startPowerRecharges when first scienced or when executed,
		//Since a new modue with same SPTemplates may construct at any time.
		if ( getSpecialPowerTemplate()->isSharedNSync() == FALSE )
			startPowerRecharge();
	}
	// WE USED TO DO THE POLL-EVERYBODY-AND-VOTE-ON-WHO-TO-SYNC-TO THING HERE,
	// BUT NO MORE, NOW IT IS HANDLED IN PLAYER

	// Some Special powers need to be activated by an Upgrade, so prevent the timer from going until then
	const SpecialPowerModuleData *md = (const SpecialPowerModuleData *)moduleData;
	if( md->m_startsPaused )
		pauseCountdown( TRUE );
	
	resolveSpecialPower();

	// Now, if we find that we have just come into being, 
	// but there is already a science granted for our shared superweapon,
	// lets make sure TheIngameUI knows about our public timer
	// add this weapon to the UI if it has a public timer for all to see
	if( m_pausedCount == 0 &&
			getSpecialPowerTemplate()->isSharedNSync() == TRUE &&
			getSpecialPowerTemplate()->hasPublicTimer() == TRUE &&
			getObject()->getControllingPlayer() &&
			getObject()->isKindOf( KINDOF_STRUCTURE ) )
	{
		TheInGameUI->addSuperweapon( getObject()->getControllingPlayer()->getPlayerIndex(), 
																 getPowerName(), 
																 getObject()->getID(), 
																 getSpecialPowerModuleData()->m_specialPowerTemplate );
	}


}  // end SpecialPowerModule

//-------------------------------------------------------------------------------------------------
const AudioEventRTS& SpecialPowerModule::getInitiateSound() const
{
	return getSpecialPowerModuleData()->m_initiateSound;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerModule::~SpecialPowerModule()
{

 	if( getSpecialPowerModuleData()->m_specialPowerTemplate->hasPublicTimer() == TRUE &&
			getObject()->getControllingPlayer() )
 		TheInGameUI->removeSuperweapon( getObject()->getControllingPlayer()->getPlayerIndex(), 
																		getPowerName(), 
																		getObject()->getID(), 
																		getSpecialPowerModuleData()->m_specialPowerTemplate );

}  // end ~SpecialPowerModule

//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::setReadyFrame( UnsignedInt frame )
{ 
	m_availableOnFrame = frame; 

	//If a script should change the ready frame, we need to update the paused frame. This value isn't
	//used directly to determine if paused or not... it uses m_pausedCount.
	m_pausedOnFrame = TheGameLogic->getFrame();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::resolveSpecialPower( void )
{
	/*

	// if we're pre-built, and not from a command center, and a building register us with the UI
	if( getSpecialPowerModuleData()->m_specialPowerTemplate->hasPublicTimer() == TRUE &&
			TheGameLogic->getFrame() == 0 && getObject()->getControllingPlayer() &&
		  getObject()->isKindOf( KINDOF_COMMANDCENTER ) == FALSE &&
			getObject()->isKindOf( KINDOF_STRUCTURE ) )
	{
		//KM: The KINDOF_STRUCTURE check was made to prevent scripted bombers from registering their
		//    special powers as public timers.
		TheInGameUI->addSuperweapon( getObject()->getControllingPlayer()->getPlayerIndex(), 
																 getPowerName(), 
																 getObject()->getID(), 
																 getSpecialPowerModuleData()->m_specialPowerTemplate );
	}
	*/
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::onSpecialPowerCreation( void )
{
	// THIS gets called by addScience(), that is, when the General has purchased a new special power, 
	// and this module is thus activated.

	// start a power recharge going
	startPowerRecharge();

	// Dustin wants these special powers to start ready to fire, 
	// so here (and only here) we will expressly set them to ready-now.
	if ( getSpecialPowerTemplate()->isSharedNSync())
	{
		Player *player = getObject()->getControllingPlayer();
		if ( player )
		{
			player->expressSpecialPowerReadyFrame( getSpecialPowerTemplate(), TheGameLogic->getFrame() );
			m_availableOnFrame = player->getOrStartSpecialPowerReadyFrame( getSpecialPowerTemplate() );
		}
	}

	// Some Special powers need to be activated by an Upgrade, so prevent the timer from going until then
	const SpecialPowerModuleData *md = getSpecialPowerModuleData();
	if( md->m_startsPaused )
		pauseCountdown( TRUE );

	// add this weapon to the UI if it has a public timer for all to see
	if( getSpecialPowerModuleData()->m_specialPowerTemplate->hasPublicTimer() == TRUE &&
			getObject()->getControllingPlayer() &&
			getObject()->isKindOf( KINDOF_STRUCTURE ) )
	{
		TheInGameUI->addSuperweapon( getObject()->getControllingPlayer()->getPlayerIndex(), 
																 getPowerName(), 
																 getObject()->getID(), 
																 getSpecialPowerModuleData()->m_specialPowerTemplate );
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ScienceType SpecialPowerModule::getRequiredScience( void ) const
{

	return getSpecialPowerModuleData()->m_specialPowerTemplate->getRequiredScience();
}  // end ~SpecialPowerModule

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const SpecialPowerTemplate * SpecialPowerModule::getSpecialPowerTemplate( void ) const
{

	return getSpecialPowerModuleData()->m_specialPowerTemplate;
}  // end ~SpecialPowerModule

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AsciiString SpecialPowerModule::getPowerName( void ) const
{

	return getSpecialPowerModuleData()->m_specialPowerTemplate->getName();
}  // end ~SpecialPowerModule

//-------------------------------------------------------------------------------------------------
/** Is this module designed for the power identier template passed in? */
//-------------------------------------------------------------------------------------------------
Bool SpecialPowerModule::isModuleForPower( const SpecialPowerTemplate *specialPowerTemplate ) const
{
	
	// get the module data
	const SpecialPowerModuleData *modData = getSpecialPowerModuleData();

	//
	// if the special power template defined in the module data matches the template we want
	// to check then we are for it!
	//
	if( modData->m_specialPowerTemplate == specialPowerTemplate )
	{
		//We match templates.
		return TRUE;
	}
	//We don't match templates.
	return FALSE;
		
}  // end canExecutePower

//-------------------------------------------------------------------------------------------------
/** Is this special power ready to use */
//-------------------------------------------------------------------------------------------------
Bool SpecialPowerModule::isReady() const
{
#if defined(RTS_DEBUG) || defined(_INTERNAL) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
	// this is a cheat ... remove this for release!
	if( TheGlobalData->m_specialPowerUsesDelay == FALSE )
		return TRUE;
#endif

	const Object* obj = getObject();
	const SpecialPowerModuleData *modData = getSpecialPowerModuleData();

	if ( obj && modData )
	{
		Player *player = getObject()->getControllingPlayer();
		if ( player )
		{
			if ( modData->m_specialPowerTemplate->isSharedNSync())
				return (TheGameLogic->getFrame() >= player->getOrStartSpecialPowerReadyFrame( modData->m_specialPowerTemplate ) );
		}
	}
	
	return (m_pausedCount == 0) && (TheGameLogic->getFrame() >= m_availableOnFrame);

}  // end isReady

//-------------------------------------------------------------------------------------------------
/** Get the percentage ready a special power is to use
	* 1.0f = ready now
	* 0.5f = 50% ready
	* 0.2f = 20% ready
	* etc ... */
//-------------------------------------------------------------------------------------------------
Real SpecialPowerModule::getPercentReady() const
{
	if( m_pausedCount > 0 && m_pausedPercent == 1.0f )
	{
			//Don't consider it ready if paused.
		return 0.99999f;
	}

#if defined(RTS_DEBUG) || defined(_INTERNAL) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
	if( TheGlobalData->m_specialPowerUsesDelay == FALSE ) 
		return 1.0f;
#endif
	
	// easy case ... is ready
	if( isReady() )
		return 1.0f;

	if( m_pausedCount > 0 )
	{
		return m_pausedPercent;
	}

	// get the module data
	const SpecialPowerModuleData *modData = getSpecialPowerModuleData();

	// sanity
	if( modData->m_specialPowerTemplate == NULL )
		return 0.0f;

	UnsignedInt readyFrame = m_availableOnFrame;

	//unless
	const Object* obj = getObject();
	if ( obj )
	{
		Player *player = getObject()->getControllingPlayer();
		if ( player )
		{
			if ( modData->m_specialPowerTemplate->isSharedNSync())
			{
				readyFrame = player->getOrStartSpecialPowerReadyFrame( getSpecialPowerTemplate() );
			}
		}
	}

	// calculate the percent	
	Real percent = 1.0f - ((readyFrame - TheGameLogic->getFrame()) / 
												 (Real)modData->m_specialPowerTemplate->getReloadTime());

	return percent;
}

//-------------------------------------------------------------------------------------------------
// A special power module that is only supposed to be fired via scripts. An example of this
// are the various cargo plane units we have. Scripters can launch specials from them after
// specifying a waypoint path for them to follow them.
//-------------------------------------------------------------------------------------------------
Bool SpecialPowerModule::isScriptOnly() const
{
	const SpecialPowerModuleData *modData = getSpecialPowerModuleData();
	return modData->m_scriptedSpecialPowerOnly;
}


//-------------------------------------------------------------------------------------------------
/** A special power has been used ... start the recharge process by computing the frame
	* we will become fully available on in the future again */
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::startPowerRecharge()
{
#if defined(RTS_DEBUG) || defined(_INTERNAL) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
	// this is a cheat ... remove this for release!
	if( TheGlobalData->m_specialPowerUsesDelay == FALSE ) 
		return;
#endif

	const SpecialPowerModuleData *modData = getSpecialPowerModuleData();

	// sanity
	if( modData->m_specialPowerTemplate == NULL )
	{
		DEBUG_CRASH(("special power not found"));
		return;
	}

	Object* obj = getObject();
	if (!obj)
		return;

	Player* player = getObject()->getControllingPlayer();
	if (!player)
		return;

	//Here, we make sure that general specials work as one between command centers
	// only factory type faction buildings should do this, and only with generals powers (in general)
	// but there are no restrictions on the use of SharedNSync on specialPowerTemplates at large
	if ( modData->m_specialPowerTemplate->isSharedNSync() )
	{
		player->resetOrStartSpecialPowerReadyFrame( modData->m_specialPowerTemplate );
	}
	else
	{
		// set the frame we will be 100% available on now
		m_availableOnFrame = TheGameLogic->getFrame() + getSpecialPowerTemplate()->getReloadTime();
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool SpecialPowerModule::initiateIntentToDoSpecialPower( const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions )
{
	Bool valid = false;
	// tell our update modules that we intend to do this special power.
	for( BehaviorModule** u = getObject()->getBehaviorModules(); *u; ++u )
	{
		SpecialPowerUpdateInterface* spu = (*u)->getSpecialPowerUpdateInterface();
		if( spu )
		{
			//Validate that we are calling the correct module!
			if( isModuleForPower( getSpecialPowerModuleData()->m_specialPowerTemplate ) )
			{
				if( spu->doesSpecialPowerUpdatePassScienceTest() )
				{
					if( spu->initiateIntentToDoSpecialPower( getSpecialPowerModuleData()->m_specialPowerTemplate, targetObj, targetPos, way, commandOptions ) )
					{
						//Kris: Aug 2003
						//We have executed the special power, so don't try to execute any more. This logic
						//was changed for multi-level spectres. Before, multiple modules would get launched
						//causing 2 or 3 spectres to be created.
						valid = true;
						break;
					}
				}
			}
		}
	}

	getObject()->getControllingPlayer()->getAcademyStats()->recordSpecialPowerUsed( getSpecialPowerModuleData()->m_specialPowerTemplate );
	
	//If we depend on our update module to trigger the special power, make sure we have the
	//appropriate update module!
	if( !valid && getSpecialPowerModuleData()->m_updateModuleStartsAttack )
	{
		DEBUG_CRASH( ("Object does not contain a special power module to execute.  Did you forget to add it to the object INI?\n"));
		//DEBUG_CRASH(( "Object does not contain special power module (%s) to execute.  Did you forget to add it to the object INI?\n",
		//							command->m_specialPower->getName().str() ));
	}

	return valid;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::triggerSpecialPower( const Coord3D *location )
{
	aboutToDoSpecialPower( location );	// do BEFORE recharge

	createViewObject(location);
	
	// we won't be able to use the power for X number of frames now
	startPowerRecharge();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::createViewObject( const Coord3D *location )
{
	const SpecialPowerModuleData *modData = getSpecialPowerModuleData();
	const SpecialPowerTemplate *powerTemplate = modData->m_specialPowerTemplate;

	if( modData == NULL  ||  powerTemplate == NULL )
		return;

	Real visionRange = powerTemplate->getViewObjectRange();
	UnsignedInt visionDuration = powerTemplate->getViewObjectDuration();

	if( visionRange == 0 || visionDuration == 0 )
		return; // We don't want a view object at all.

	AsciiString objectName = TheGlobalData->m_specialPowerViewObjectName;
	if( objectName.isEmpty() )
		return;

	const ThingTemplate *viewObjectTemplate = TheThingFactory->findTemplate( objectName );
	if( viewObjectTemplate == NULL )
		return;

	Object *viewObject = TheThingFactory->newObject( viewObjectTemplate, getObject()->getControllingPlayer()->getDefaultTeam() );

	if( viewObject == NULL )
		return;

	viewObject->setPosition( location );
	viewObject->setShroudClearingRange( visionRange );

	static NameKeyType key_DeletionUpdate = NAMEKEY("DeletionUpdate");
	DeletionUpdate* dup = (DeletionUpdate*)viewObject->findUpdateModule(key_DeletionUpdate);
	if( dup )
	{
		dup->setLifetimeRange( visionDuration, visionDuration );
	}	
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::markSpecialPowerTriggered( const Coord3D *location )
{
	triggerSpecialPower( location );
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::aboutToDoSpecialPower( const Coord3D *location )
{
	// Tell the scripting engine!
	TheScriptEngine->notifyOfTriggeredSpecialPower(
		getObject()->getControllingPlayer()->getPlayerIndex(), 
		getSpecialPowerModuleData()->m_specialPowerTemplate->getName(),
		getObject()->getID());

	// Let EVA do her thing
	SpecialPowerType type = getSpecialPowerModuleData()->m_specialPowerTemplate->getSpecialPowerType();

  Player *localPlayer = ThePlayerList->getLocalPlayer();

  // Only play the EVA sounds if this is not the local player, and the local player doesn't consider the 
	// person an enemy.
	// Kris: Actually, all players need to hear these warnings.
  // Ian: But now there are different Eva messages depending on who launched
	//if (localPlayer != getObject()->getControllingPlayer() && localPlayer->getRelationship(getObject()->getTeam()) != ENEMIES) 
  {
		if( type == SPECIAL_PARTICLE_UPLINK_CANNON || type == SUPW_SPECIAL_PARTICLE_UPLINK_CANNON || type == LAZR_SPECIAL_PARTICLE_UPLINK_CANNON )
    {
      if ( localPlayer == getObject()->getControllingPlayer() )
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Own_ParticleCannon);
      }
      else if ( localPlayer->getRelationship(getObject()->getTeam()) != ENEMIES )
      {
        // Note: counting relationship NEUTRAL as ally. Not sure if this makes a difference???
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Ally_ParticleCannon);
      }
      else
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Enemy_ParticleCannon);
      }
    }
    else if( type == SPECIAL_NEUTRON_MISSILE || type == NUKE_SPECIAL_NEUTRON_MISSILE || type == SUPW_SPECIAL_NEUTRON_MISSILE )
    {
      if ( localPlayer == getObject()->getControllingPlayer() )
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Own_Nuke);
      }
      else if ( localPlayer->getRelationship(getObject()->getTeam()) != ENEMIES )
      {
        // Note: counting relationship NEUTRAL as ally. Not sure if this makes a difference???
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Ally_Nuke);
      }
      else
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Enemy_Nuke);
      }
    }
		else if (type == SPECIAL_SCUD_STORM)
    {
      if ( localPlayer == getObject()->getControllingPlayer() )
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Own_ScudStorm);
      }
      else if ( localPlayer->getRelationship(getObject()->getTeam()) != ENEMIES )
      {
        // Note: counting relationship NEUTRAL as ally. Not sure if this makes a difference???
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Ally_ScudStorm);
      }
      else
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Enemy_ScudStorm);
      }
    }
		else if (type == SPECIAL_GPS_SCRAMBLER || type == SLTH_SPECIAL_GPS_SCRAMBLER )
    {
			// This is Ghetto.  Voices should be ini lines in the special power entry.  You shouldn't have to 
			// add to an enum to get a new voice
      if ( localPlayer == getObject()->getControllingPlayer() )
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Own_GPS_Scrambler);
      }
      else if ( localPlayer->getRelationship(getObject()->getTeam()) != ENEMIES )
      {
        // Note: counting relationship NEUTRAL as ally. Not sure if this makes a difference???
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Ally_GPS_Scrambler);
      }
      else
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Enemy_GPS_Scrambler);
      }
    }
		else if (type == SPECIAL_SNEAK_ATTACK)
    {
      if ( localPlayer == getObject()->getControllingPlayer() )
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Own_Sneak_Attack);
      }
      else if ( localPlayer->getRelationship(getObject()->getTeam()) != ENEMIES )
      {
        // Note: counting relationship NEUTRAL as ally. Not sure if this makes a difference???
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Ally_Sneak_Attack);
      }
      else
      {
        TheEva->setShouldPlay(EVA_SuperweaponLaunched_Enemy_Sneak_Attack);
      }
    }
	}

	// get module data
	const SpecialPowerModuleData *modData = getSpecialPowerModuleData();

	// play our initiate sound if we have one
	AudioEventRTS audioEvent = *modData->m_specialPowerTemplate->getInitiateSound();
	audioEvent.setObjectID(getObject()->getID());
	TheAudio->addAudioEvent( &audioEvent );

	// play sound at target location if specified
	if( location )
	{

		AudioEventRTS soundAtLocation = *modData->m_specialPowerTemplate->getInitiateAtTargetSound();
		soundAtLocation.setPosition( location );
		soundAtLocation.setPlayerIndex(getObject()->getControllingPlayer()->getPlayerIndex());
		TheAudio->addAudioEvent( &soundAtLocation );

	}  // end if

}

//-------------------------------------------------------------------------------------------------
//By default, special powers are not triggered by it's update module -- in which case
//it triggers it and resets its timer immediately. When the update module triggers it,
//then all we do is initiate the special power, and trust that the update module will 
//do the rest.
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::doSpecialPower( UnsignedInt commandOptions )
{
	if (m_pausedCount > 0 || getObject()->isDisabled()) {
		return;
	}

	//This tells the update module that we want to do our special power. The update modules
	//will then start processing each frame.
	initiateIntentToDoSpecialPower( NULL, NULL, NULL, commandOptions );

	//Only trigger the special power immediately if the updatemodule doesn't start the attack.
	//An example of a case that wouldn't trigger immediately is for a unit that needs to 
	//close to range before firing the special attack. A case that would trigger immediately
	//is the napalm strike. If we don't call this now, it's up to the update module to do so.
	if( !getSpecialPowerModuleData()->m_updateModuleStartsAttack )
	{
		triggerSpecialPower( NULL );// Location-less trigger
	}
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions )
{
	if (m_pausedCount > 0 || getObject()->isDisabled()) {
		return;
	}

	//This tells the update module that we want to do our special power. The update modules
	//will then start processing each frame.
	initiateIntentToDoSpecialPower( obj, NULL, NULL, commandOptions );

	//Only trigger the special power immediately if the updatemodule doesn't start the attack.
	//An example of a case that wouldn't trigger immediately is for a unit that needs to 
	//close to range before firing the special attack. A case that would trigger immediately
	//is the napalm strike. If we don't call this now, it's up to the update module to do so.
	if( !getSpecialPowerModuleData()->m_updateModuleStartsAttack )
	{
		triggerSpecialPower( obj->getPosition() );
	}
}  

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions )
{
	if (m_pausedCount > 0 || getObject()->isDisabled()) {
		return;
	}

	//This tells the update module that we want to do our special power. The update modules
	//will then start processing each frame.
	initiateIntentToDoSpecialPower( NULL, loc, NULL, commandOptions );

	//Only trigger the special power immediately if the updatemodule doesn't start the attack.
	//An example of a case that wouldn't trigger immediately is for a unit that needs to 
	//close to range before firing the special attack. A case that would trigger immediately
	//is the napalm strike. If we don't call this now, it's up to the update module to do so.
	if( !getSpecialPowerModuleData()->m_updateModuleStartsAttack )
	{
		triggerSpecialPower( loc );
	}
}  

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::doSpecialPowerUsingWaypoints( const Waypoint *way, UnsignedInt commandOptions )
{
	if (m_pausedCount > 0 || getObject()->isDisabled()) {
		return;
	}

	//This tells the update module that we want to do our special power. The update modules
	//will then start processing each frame.
	initiateIntentToDoSpecialPower( NULL, NULL, way, commandOptions );

	//Only trigger the special power immediately if the updatemodule doesn't start the attack.
	//An example of a case that wouldn't trigger immediately is for a unit that needs to 
	//close to range before firing the special attack. A case that would trigger immediately
	//is the napalm strike. If we don't call this now, it's up to the update module to do so.
	if( !getSpecialPowerModuleData()->m_updateModuleStartsAttack )
	{
		triggerSpecialPower( NULL );// This type doesn't create view objects
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerModule::pauseCountdown( Bool pause )
{
	if (pause)// If pausing
	{
		if( m_pausedCount == 0 )
		{
			// Only record this with the first pausing, otherwise upon final unpausing you would get credited the time
			// between pauses as time served.
			m_pausedOnFrame = TheGameLogic->getFrame();
			m_pausedPercent = getPercentReady();
		}

		++m_pausedCount;
	}
	else if( m_pausedCount > 0 )//Else if unpausing, but only if I am in fact paused, so multiple unpauses don't break.
	{
		--m_pausedCount;

		// And only update the ready time if we are fully unpaused now.
		if( m_pausedCount == 0 )	
		{
			m_availableOnFrame += (TheGameLogic->getFrame() - m_pausedOnFrame);
		}
	}
}  // end pauseCountdown

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnsignedInt SpecialPowerModule::getReadyFrame( void ) const
{
	if ( getSpecialPowerTemplate()->isSharedNSync() )
	{
		const Object* obj = getObject();
		if ( obj )
		{
			Player *player = getObject()->getControllingPlayer();
			if ( player )
				return player->getOrStartSpecialPowerReadyFrame( getSpecialPowerTemplate() );
		}
	}

	if (m_pausedCount > 0 || getObject()->isDisabled())
	{
		Int pausedFrames = TheGameLogic->getFrame() - m_pausedOnFrame;
		return m_availableOnFrame + pausedFrames;
	}
	else
	{
		return m_availableOnFrame;
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SpecialPowerModule::crc( Xfer *xfer )
{

	// extend base class
	BehaviorModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SpecialPowerModule::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	BehaviorModule::xfer( xfer );

	// available on frame
	xfer->xferUnsignedInt( &m_availableOnFrame );

	// paused by script
	xfer->xferInt( &m_pausedCount );

	// paused on frame
	xfer->xferUnsignedInt( &m_pausedOnFrame );

	// paused percent
	xfer->xferReal( &m_pausedPercent );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SpecialPowerModule::loadPostProcess( void )
{

	// extend base class
	BehaviorModule::loadPostProcess();



	// Now, if we find that we have just come into being, 
	// but there is already a science granted for our shared superweapon,
	// lets make sure TheIngameUI knows about our public timer
	// add this weapon to the UI if it has a public timer for all to see
	if( m_pausedCount == 0 &&
			getSpecialPowerTemplate()->isSharedNSync() == TRUE &&
			getSpecialPowerTemplate()->hasPublicTimer() == TRUE &&
			getObject()->getControllingPlayer() &&
			getObject()->isKindOf( KINDOF_STRUCTURE ) )
	{
		TheInGameUI->addSuperweapon( getObject()->getControllingPlayer()->getPlayerIndex(), 
																 getPowerName(), 
																 getObject()->getID(), 
																 getSpecialPowerModuleData()->m_specialPowerTemplate );
	}






}  // end loadPostProcess
