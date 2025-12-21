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

// FILE: BattlePlanUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, September 2002
// Desc:   Update module to handle building states and battle plan execution & changes
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_MAXHEALTHCHANGETYPE_NAMES						// for TheMaxHealthChangeTypeNames[]

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/BitFlagsIO.h"
#include "Common/Radar.h"
#include "Common/PlayerList.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Player.h"
#include "Common/Xfer.h"

#include "GameClient/GameClient.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameText.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/FXList.h"
#include "GameClient/ControlBar.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/WeaponSet.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Module/BattlePlanUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/ActiveBody.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/StealthDetectorUpdate.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BattlePlanUpdateModuleData::BattlePlanUpdateModuleData()
{
	m_specialPowerTemplate								= nullptr;
	m_bombardmentPlanAnimationFrames			= 0;
	m_holdTheLinePlanAnimationFrames			= 0;
	m_searchAndDestroyPlanAnimationFrames = 0;
	m_battlePlanParalyzeFrames						= 0;


	m_holdTheLineArmorDamageScalar				= 1.0f;
	m_searchAndDestroySightRangeScalar		= 1.0f;
	m_strategyCenterSearchAndDestroySightRangeScalar = 1.0f;
	m_strategyCenterSearchAndDestroyDetectsStealth = true;
	m_strategyCenterHoldTheLineMaxHealthScalar = 1.0f;
	m_strategyCenterHoldTheLineMaxHealthChangeType = PRESERVE_RATIO;

}

//-------------------------------------------------------------------------------------------------
/*static*/ void BattlePlanUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "SpecialPowerTemplate",									INI::parseSpecialPowerTemplate,	nullptr, offsetof( BattlePlanUpdateModuleData, m_specialPowerTemplate ) },

    { "BombardmentPlanAnimationTime",					INI::parseDurationUnsignedInt,  nullptr, offsetof( BattlePlanUpdateModuleData, m_bombardmentPlanAnimationFrames ) },
    { "HoldTheLinePlanAnimationTime",					INI::parseDurationUnsignedInt,  nullptr, offsetof( BattlePlanUpdateModuleData, m_holdTheLinePlanAnimationFrames ) },
    { "SearchAndDestroyPlanAnimationTime",		INI::parseDurationUnsignedInt,  nullptr, offsetof( BattlePlanUpdateModuleData, m_searchAndDestroyPlanAnimationFrames ) },
		{ "TransitionIdleTime",										INI::parseDurationUnsignedInt,  nullptr, offsetof( BattlePlanUpdateModuleData, m_transitionIdleFrames ) },

		{ "BombardmentPlanUnpackSoundName",				INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_bombardmentUnpackName ) },
		{ "BombardmentPlanPackSoundName",					INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_bombardmentPackName ) },
		{ "BombardmentMessageLabel",							INI::parseAsciiString,					nullptr,	offsetof( BattlePlanUpdateModuleData, m_bombardmentMessageLabel ) },
		{ "BombardmentAnnouncementName",					INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_bombardmentAnnouncementName ) },
		{ "SearchAndDestroyPlanUnpackSoundName",	INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_searchAndDestroyUnpackName ) },
		{ "SearchAndDestroyPlanIdleLoopSoundName",INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_searchAndDestroyIdleName ) },
		{ "SearchAndDestroyPlanPackSoundName",		INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_searchAndDestroyPackName ) },
		{ "SearchAndDestroyMessageLabel",					INI::parseAsciiString,					nullptr,	offsetof( BattlePlanUpdateModuleData, m_searchAndDestroyMessageLabel ) },
		{ "SearchAndDestroyAnnouncementName",			INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_searchAndDestroyAnnouncementName ) },
		{ "HoldTheLinePlanUnpackSoundName",				INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_holdTheLineUnpackName ) },
		{ "HoldTheLinePlanPackSoundName",					INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_holdTheLinePackName ) },
		{ "HoldTheLineMessageLabel",							INI::parseAsciiString,					nullptr,	offsetof( BattlePlanUpdateModuleData, m_holdTheLineMessageLabel ) },
		{ "HoldTheLineAnnouncementName",					INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_holdTheLineAnnouncementName ) },

		{ "ValidMemberKindOf",										KindOfMaskType::parseFromINI,								nullptr, offsetof( BattlePlanUpdateModuleData, m_validMemberKindOf ) },
		{ "InvalidMemberKindOf",									KindOfMaskType::parseFromINI,								nullptr, offsetof( BattlePlanUpdateModuleData, m_invalidMemberKindOf ) },
		{ "BattlePlanChangeParalyzeTime",					INI::parseDurationUnsignedInt,  nullptr, offsetof( BattlePlanUpdateModuleData, m_battlePlanParalyzeFrames ) },
		{ "HoldTheLinePlanArmorDamageScalar",			INI::parseReal,									nullptr, offsetof( BattlePlanUpdateModuleData, m_holdTheLineArmorDamageScalar ) },
		{ "SearchAndDestroyPlanSightRangeScalar",	INI::parseReal,									nullptr, offsetof( BattlePlanUpdateModuleData, m_searchAndDestroySightRangeScalar ) },

		{ "StrategyCenterSearchAndDestroySightRangeScalar", INI::parseReal,				nullptr, offsetof( BattlePlanUpdateModuleData, m_strategyCenterSearchAndDestroySightRangeScalar ) },
		{ "StrategyCenterSearchAndDestroyDetectsStealth",   INI::parseBool,				nullptr, offsetof( BattlePlanUpdateModuleData, m_strategyCenterSearchAndDestroyDetectsStealth ) },
		{ "StrategyCenterHoldTheLineMaxHealthScalar",				INI::parseReal,				nullptr, offsetof( BattlePlanUpdateModuleData, m_strategyCenterHoldTheLineMaxHealthScalar ) },
    { "StrategyCenterHoldTheLineMaxHealthChangeType",		INI::parseIndexList,  TheMaxHealthChangeTypeNames, offsetof( BattlePlanUpdateModuleData, m_strategyCenterHoldTheLineMaxHealthChangeType ) },

		{ "VisionObjectName",											INI::parseAsciiString,					nullptr, offsetof( BattlePlanUpdateModuleData, m_visionObjectName ) },

		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
BattlePlanUpdate::BattlePlanUpdate( Thing *thing, const ModuleData* moduleData ) :
	UpdateModule( thing, moduleData ),
	m_bonuses(nullptr)
{
	const BattlePlanUpdateModuleData *data = getBattlePlanUpdateModuleData();

	m_status								= TRANSITIONSTATUS_IDLE;
	m_currentPlan						= PLANSTATUS_NONE;
	m_desiredPlan						= PLANSTATUS_NONE;
	m_planAffectingArmy			= PLANSTATUS_NONE;
	m_nextReadyFrame				= 0;
	m_invalidSettings				= false;
	m_centeringTurret				= false;

	//Default the bonuses to no change.
	m_bonuses = newInstance(BattlePlanBonuses);
	m_bonuses->m_armorScalar					= 1.0f;
	m_bonuses->m_sightRangeScalar		= 1.0f;
	m_bonuses->m_bombardment					= 0;
	m_bonuses->m_searchAndDestroy		= 0;
	m_bonuses->m_holdTheLine					= 0;
	m_bonuses->m_validKindOf					= data->m_validMemberKindOf;
	m_bonuses->m_invalidKindOf				= data->m_invalidMemberKindOf;

	m_visionObjectID = INVALID_ID;

	m_specialPowerModule   = nullptr;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BattlePlanUpdate::~BattlePlanUpdate( void )
{
	TheAudio->removeAudioEvent( m_bombardmentUnpack.getPlayingHandle() );
	TheAudio->removeAudioEvent( m_bombardmentPack.getPlayingHandle() );
	TheAudio->removeAudioEvent( m_searchAndDestroyUnpack.getPlayingHandle() );
	TheAudio->removeAudioEvent( m_searchAndDestroyIdle.getPlayingHandle() );
	TheAudio->removeAudioEvent( m_searchAndDestroyPack.getPlayingHandle() );
	TheAudio->removeAudioEvent( m_holdTheLineUnpack.getPlayingHandle() );
	TheAudio->removeAudioEvent( m_holdTheLinePack.getPlayingHandle() );

}

// ------------------------------------------------------------------------------------------------
/** On delete */
// ------------------------------------------------------------------------------------------------
void BattlePlanUpdate::onDelete()
{

	// extend base class
	UpdateModule::onDelete();

	// delete our vision object, if it exists
	Object *obj;
	if( m_visionObjectID != INVALID_ID )
	{
		obj = TheGameLogic->findObjectByID( m_visionObjectID );
		if( obj )
			TheGameLogic->destroyObject( obj );
	}

	// If we get destroyed, then make sure we remove our bonus!
	// srj sez: we can't do this in the dtor because our team
	// (and thus controlling player) has already been nulled by then...
	Player* player = getObject()->getControllingPlayer();
	// however, player CAN legitimately be null during game reset cycles
	// (and which point it doesn't really matter if we can remove the bonus or not)
	//DEBUG_ASSERTCRASH(player != nullptr, ("Hmm, controller is null"));
	if( player && m_planAffectingArmy != PLANSTATUS_NONE )
	{
		player->changeBattlePlan( m_planAffectingArmy, -1, m_bonuses );
	}

}

//-------------------------------------------------------------------------------------------------
// Validate that we have the necessary data from the ini file.
//-------------------------------------------------------------------------------------------------
void BattlePlanUpdate::onObjectCreated()
{
	const BattlePlanUpdateModuleData *data = getBattlePlanUpdateModuleData();
	Object *obj = getObject();

	if( !data->m_specialPowerTemplate )
	{
		DEBUG_CRASH( ("%s object's BattlePlanUpdate lacks access to the SpecialPowerTemplate. Needs to be specified in ini.", obj->getTemplate()->getName().str() ) );
		m_invalidSettings = true;
		return;
	}

	m_specialPowerModule = obj->getSpecialPowerModule( data->m_specialPowerTemplate );

	//Create instances of the sounds required.
	m_bombardmentUnpack.setEventName( data->m_bombardmentUnpackName );
	m_bombardmentPack.setEventName(	data->m_bombardmentPackName );
	m_bombardmentAnnouncement.setEventName( data->m_bombardmentAnnouncementName );
	m_searchAndDestroyUnpack.setEventName( data->m_searchAndDestroyUnpackName );
	m_searchAndDestroyIdle.setEventName( data->m_searchAndDestroyIdleName );
	m_searchAndDestroyPack.setEventName( data->m_searchAndDestroyPackName );
	m_searchAndDestroyAnnouncement.setEventName( data->m_searchAndDestroyAnnouncementName );
	m_holdTheLineUnpack.setEventName( data->m_holdTheLineUnpackName );
	m_holdTheLinePack.setEventName(	data->m_holdTheLinePackName );
	m_holdTheLineAnnouncement.setEventName( data->m_holdTheLineAnnouncementName );
	TheAudio->getInfoForAudioEvent( &m_bombardmentUnpack );
	TheAudio->getInfoForAudioEvent( &m_bombardmentPack );
	TheAudio->getInfoForAudioEvent( &m_bombardmentAnnouncement );
	TheAudio->getInfoForAudioEvent( &m_searchAndDestroyUnpack );
	TheAudio->getInfoForAudioEvent( &m_searchAndDestroyIdle );
	TheAudio->getInfoForAudioEvent( &m_searchAndDestroyPack );
	TheAudio->getInfoForAudioEvent( &m_searchAndDestroyAnnouncement );
	TheAudio->getInfoForAudioEvent( &m_holdTheLineUnpack );
	TheAudio->getInfoForAudioEvent( &m_holdTheLinePack );
	TheAudio->getInfoForAudioEvent( &m_holdTheLineAnnouncement );

	getObject()->setWeaponSetFlag( WEAPONSET_VETERAN );
	AIUpdateInterface *ai = obj->getAI();
	if( ai )
	{
		// lock it just till the weapon is empty or the attack is "done"
		obj->setWeaponLock( PRIMARY_WEAPON, LOCKED_TEMPORARILY );
	}
	enableTurret( false );
}

//-------------------------------------------------------------------------------------------------
Bool BattlePlanUpdate::initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions )
{
	if( m_specialPowerModule->getSpecialPowerTemplate() != specialPowerTemplate )
	{
		//Check to make sure our modules are connected.
		return FALSE;
	}

	//Set the desired status based on the command button option!
	if( BitIsSet( commandOptions, OPTION_ONE ) )
	{
		m_desiredPlan = PLANSTATUS_BOMBARDMENT;
	}
	else if( BitIsSet( commandOptions, OPTION_TWO ) )
	{
		m_desiredPlan = PLANSTATUS_HOLDTHELINE;
	}
	else if( BitIsSet( commandOptions, OPTION_THREE ) )
	{
		m_desiredPlan = PLANSTATUS_SEARCHANDDESTROY;
	}
	else
	{
		DEBUG_CRASH( ("Selected an unsupported strategy for strategy center.") );
		return FALSE;
	}

	return TRUE;
}

Bool BattlePlanUpdate::isPowerCurrentlyInUse( const CommandButton *command ) const
{
	//@todo -- perhaps we may need this one day...
	return false;
}

//-------------------------------------------------------------------------------------------------
CommandOption BattlePlanUpdate::getCommandOption() const
{
	switch( m_desiredPlan )
	{
		case PLANSTATUS_BOMBARDMENT:
			return OPTION_ONE;
		case PLANSTATUS_HOLDTHELINE:
			return OPTION_TWO;
		case PLANSTATUS_SEARCHANDDESTROY:
			return OPTION_THREE;
	}
	return (CommandOption)0;
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime BattlePlanUpdate::update()
{

	if( m_invalidSettings )
	{
		// can't return UPDATE_SLEEP_FOREVER unless we are sleepy...
		return UPDATE_SLEEP_NONE;
		///return UPDATE_SLEEP_FOREVER;
	}

	//const BattlePlanUpdateModuleData *data = getBattlePlanUpdateModuleData();
	//Object *obj = getObject();

	UnsignedInt now = TheGameLogic->getFrame();

	if( m_nextReadyFrame <= now )
	{
		switch( m_status )
		{
			case TRANSITIONSTATUS_IDLE:
				//There's only two cases where we are in an idle status -- upon initialization
				//when no plan has yet been selected. The other case is after we've finished
				//packing the previous plan up and are waiting to unpack the new state.
				if( m_desiredPlan != PLANSTATUS_NONE )
				{
					m_currentPlan = m_desiredPlan;
					setStatus( TRANSITIONSTATUS_UNPACKING );
				}
				break;
			case TRANSITIONSTATUS_UNPACKING:
				//If we're unpacking, we are forcing the user to wait until the plan is unpacked
				//before allowing him to select a new plan. The plan doesn't become active until
				//we're finished unpacking.
				setStatus( TRANSITIONSTATUS_ACTIVE );
				if( m_currentPlan == PLANSTATUS_BOMBARDMENT )
				{
					enableTurret( true );
				}
				break;
			case TRANSITIONSTATUS_ACTIVE:
				//If we're active and the user has selected a different plan, then we need to
				//pack up.
				if( m_currentPlan != m_desiredPlan )
				{
					if( m_currentPlan == PLANSTATUS_BOMBARDMENT )
					{
						//Special case situation -- in bombardment status, we need to center
						//the turret prior to packing up, so handle it here.
						AIUpdateInterface *ai = getObject()->getAI();
						if( ai )
						{
							if( isTurretInNaturalPosition() )
							{
								//It's centered, so pack
								setStatus( TRANSITIONSTATUS_PACKING );
								m_centeringTurret = false;
								enableTurret( false );
							}
							else if( !m_centeringTurret )
							{
								//It's not centered, and not trying to center, so order it to center.
								ai->aiIdle( CMD_FROM_AI );
								recenterTurret();
								m_centeringTurret = true;
							}
						}
					}
					else
					{
						setStatus( TRANSITIONSTATUS_PACKING );
					}
				}
				break;
			case TRANSITIONSTATUS_PACKING:
				//If we finished packing, then go idle until we can switch to our new plan.
				setStatus( TRANSITIONSTATUS_IDLE );
				break;
		}
	}

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
/** Create vision objects for all players revealing this building to all */
// ------------------------------------------------------------------------------------------------
void BattlePlanUpdate::createVisionObject()
{
	if (m_visionObjectID != INVALID_ID) // don't want two.
		return;

	const BattlePlanUpdateModuleData *data = getBattlePlanUpdateModuleData();
	Object *obj = getObject();

	// get template of object to create
	const ThingTemplate *tt = TheThingFactory->findTemplate( data->m_visionObjectName );
	DEBUG_ASSERTCRASH( tt, ("BattlePlanUpdate::setStatus - Invalid vision object name '%s'",
													data->m_visionObjectName.str()) );

	if (!tt)
		return;

	Player *pPlayer = ThePlayerList->getNeutralPlayer();
	// sanity
	if(!pPlayer)
		return;

	Object *visionObject;

	// create object for this player
	visionObject = TheThingFactory->newObject( tt, pPlayer->getDefaultTeam() );
	if( visionObject )
	{

		// record we have an object
		m_visionObjectID = visionObject->getID();

		// set position
		visionObject->setPosition( obj->getPosition() );

		// set the shroud clearing range
		visionObject->setShroudClearingRange( obj->getGeometryInfo().getBoundingSphereRadius() );

	}

}

//-------------------------------------------------------------------------------------------------
void BattlePlanUpdate::setStatus( TransitionStatus newStatus )
{
	const BattlePlanUpdateModuleData *data = getBattlePlanUpdateModuleData();
	Object *obj = getObject();

	if( m_status == newStatus )
	{
		return;
	}

	TransitionStatus oldStatus = m_status;

	//Turn off old defining states and sounds
	switch( oldStatus )
	{
		case TRANSITIONSTATUS_IDLE:
			break;

		case TRANSITIONSTATUS_UNPACKING:
			switch( m_currentPlan )
			{
				case PLANSTATUS_BOMBARDMENT:
					obj->clearModelConditionState( MODELCONDITION_DOOR_1_OPENING );
					TheAudio->removeAudioEvent( m_bombardmentUnpack.getPlayingHandle() );
					break;
				case PLANSTATUS_HOLDTHELINE:
					obj->clearModelConditionState( MODELCONDITION_DOOR_2_OPENING );
					TheAudio->removeAudioEvent( m_holdTheLineUnpack.getPlayingHandle() );
					break;
				case PLANSTATUS_SEARCHANDDESTROY:
					obj->clearModelConditionState( MODELCONDITION_DOOR_3_OPENING );
					TheAudio->removeAudioEvent( m_searchAndDestroyUnpack.getPlayingHandle() );
					break;
			}
			break;

		case TRANSITIONSTATUS_ACTIVE:
			switch( m_currentPlan )
			{
				case PLANSTATUS_BOMBARDMENT:
					obj->clearModelConditionState( MODELCONDITION_DOOR_1_WAITING_TO_CLOSE );
					break;
				case PLANSTATUS_HOLDTHELINE:
					obj->clearModelConditionState( MODELCONDITION_DOOR_2_WAITING_TO_CLOSE );
					break;
				case PLANSTATUS_SEARCHANDDESTROY:
					obj->clearModelConditionState( MODELCONDITION_DOOR_3_WAITING_TO_CLOSE );
					TheAudio->removeAudioEvent( m_searchAndDestroyIdle.getPlayingHandle() );
					break;
			}
			break;

		case TRANSITIONSTATUS_PACKING:
			switch( m_currentPlan )
			{
				case PLANSTATUS_BOMBARDMENT:
					obj->clearModelConditionState( MODELCONDITION_DOOR_1_CLOSING );
					TheAudio->removeAudioEvent( m_bombardmentPack.getPlayingHandle() );
					break;
				case PLANSTATUS_HOLDTHELINE:
					obj->clearModelConditionState( MODELCONDITION_DOOR_2_CLOSING );
					TheAudio->removeAudioEvent( m_holdTheLinePack.getPlayingHandle() );
					break;
				case PLANSTATUS_SEARCHANDDESTROY:
					obj->clearModelConditionState( MODELCONDITION_DOOR_3_CLOSING );
					TheAudio->removeAudioEvent( m_searchAndDestroyPack.getPlayingHandle() );
					break;
			}
			break;
	}

	UnsignedInt now = TheGameLogic->getFrame();

	//Handle entering new status
	switch( newStatus )
	{
		case TRANSITIONSTATUS_IDLE:
		{
			m_currentPlan = PLANSTATUS_NONE;
			m_nextReadyFrame = now + data->m_transitionIdleFrames;
			break;
		}

		case TRANSITIONSTATUS_UNPACKING:
		{

			// play a radar blip showing the battle plan change in the color of the
			TheRadar->createPlayerEvent( obj->getControllingPlayer(),
																	 obj->getPosition(),
																	 RADAR_EVENT_BATTLE_PLAN );

			createVisionObject();

			switch( m_currentPlan )
			{
				case PLANSTATUS_BOMBARDMENT:
					obj->setModelConditionState( MODELCONDITION_DOOR_1_OPENING );
					obj->getDrawable()->setAnimationLoopDuration( data->m_bombardmentPlanAnimationFrames );
					m_nextReadyFrame = now + data->m_bombardmentPlanAnimationFrames;
					if( m_bombardmentUnpack.getEventName().isNotEmpty() )
					{
						m_bombardmentUnpack.setObjectID( obj->getID() );
						m_bombardmentUnpack.setPlayingHandle( TheAudio->addAudioEvent( &m_bombardmentUnpack ) );
					}

					// display a message to *all* users
					TheInGameUI->message( TheGameText->fetch( data->m_bombardmentMessageLabel ) );
					if( m_bombardmentAnnouncement.getEventName().isEmpty() == FALSE )
					{
						m_bombardmentAnnouncement.setPosition( obj->getPosition() );
						TheAudio->addAudioEvent( &m_bombardmentAnnouncement );
					}
					break;

				case PLANSTATUS_HOLDTHELINE:
					obj->setModelConditionState( MODELCONDITION_DOOR_2_OPENING );
					obj->getDrawable()->setAnimationLoopDuration( data->m_holdTheLinePlanAnimationFrames );
					m_nextReadyFrame = now + data->m_holdTheLinePlanAnimationFrames;
					if( m_holdTheLineUnpack.getEventName().isNotEmpty() )
					{
						m_holdTheLineUnpack.setObjectID( obj->getID() );
						m_holdTheLineUnpack.setPlayingHandle( TheAudio->addAudioEvent( &m_holdTheLineUnpack ) );
					}

					// display a message to *all* users
					TheInGameUI->message( TheGameText->fetch( data->m_holdTheLineMessageLabel ) );
					if( m_holdTheLineAnnouncement.getEventName().isEmpty() == FALSE )
					{
						m_holdTheLineAnnouncement.setPosition( obj->getPosition() );
						TheAudio->addAudioEvent( &m_holdTheLineAnnouncement );
					}
					break;

				case PLANSTATUS_SEARCHANDDESTROY:
					obj->setModelConditionState( MODELCONDITION_DOOR_3_OPENING );
					obj->getDrawable()->setAnimationLoopDuration( data->m_searchAndDestroyPlanAnimationFrames );
					m_nextReadyFrame = now + data->m_searchAndDestroyPlanAnimationFrames;
					if( m_searchAndDestroyUnpack.getEventName().isNotEmpty() )
					{
						m_searchAndDestroyUnpack.setObjectID( obj->getID() );
						m_searchAndDestroyUnpack.setPlayingHandle( TheAudio->addAudioEvent( &m_searchAndDestroyUnpack ) );
					}

					// display a message to *all* users
					TheInGameUI->message( TheGameText->fetch( data->m_searchAndDestroyMessageLabel ) );
					if( m_searchAndDestroyAnnouncement.getEventName().isEmpty() == FALSE )
					{
						m_searchAndDestroyAnnouncement.setPosition( obj->getPosition() );
						TheAudio->addAudioEvent( &m_searchAndDestroyAnnouncement );
					}
					break;

			}

			break;

		}

		case TRANSITIONSTATUS_ACTIVE:

			setBattlePlan( m_currentPlan );

			switch( m_currentPlan )
			{
				case PLANSTATUS_BOMBARDMENT:
					obj->setModelConditionState( MODELCONDITION_DOOR_1_WAITING_TO_CLOSE );
					break;
				case PLANSTATUS_HOLDTHELINE:
					obj->setModelConditionState( MODELCONDITION_DOOR_2_WAITING_TO_CLOSE );
					break;
				case PLANSTATUS_SEARCHANDDESTROY:
					obj->setModelConditionState( MODELCONDITION_DOOR_3_WAITING_TO_CLOSE );
					if( m_searchAndDestroyIdle.getEventName().isNotEmpty() )
					{
						m_searchAndDestroyIdle.setObjectID( obj->getID() );
						m_searchAndDestroyIdle.setPlayingHandle( TheAudio->addAudioEvent( &m_searchAndDestroyIdle ) );
					}
					break;
			}
			break;

		case TRANSITIONSTATUS_PACKING:

			setBattlePlan( PLANSTATUS_NONE );

			switch( m_currentPlan )
			{
				case PLANSTATUS_BOMBARDMENT:
					obj->setModelConditionState( MODELCONDITION_DOOR_1_CLOSING );
					obj->getDrawable()->setAnimationLoopDuration( data->m_bombardmentPlanAnimationFrames );
					m_nextReadyFrame = now + data->m_bombardmentPlanAnimationFrames;
					if( m_bombardmentUnpack.getEventName().isNotEmpty() )
					{
						m_bombardmentPack.setObjectID( obj->getID() );
						m_bombardmentPack.setPlayingHandle( TheAudio->addAudioEvent( &m_bombardmentPack ) );
					}
					break;
				case PLANSTATUS_HOLDTHELINE:
					obj->setModelConditionState( MODELCONDITION_DOOR_2_CLOSING );
					obj->getDrawable()->setAnimationLoopDuration( data->m_holdTheLinePlanAnimationFrames );
					m_nextReadyFrame = now + data->m_holdTheLinePlanAnimationFrames;
					if( m_holdTheLineUnpack.getEventName().isNotEmpty() )
					{
						m_holdTheLinePack.setObjectID( obj->getID() );
						m_holdTheLinePack.setPlayingHandle( TheAudio->addAudioEvent( &m_holdTheLinePack ) );
					}
					break;
				case PLANSTATUS_SEARCHANDDESTROY:
					obj->setModelConditionState( MODELCONDITION_DOOR_3_CLOSING );
					obj->getDrawable()->setAnimationLoopDuration( data->m_searchAndDestroyPlanAnimationFrames );
					m_nextReadyFrame = now + data->m_searchAndDestroyPlanAnimationFrames;
					if( m_searchAndDestroyUnpack.getEventName().isNotEmpty() )
					{
						m_searchAndDestroyPack.setObjectID( obj->getID() );
						m_searchAndDestroyPack.setPlayingHandle( TheAudio->addAudioEvent( &m_searchAndDestroyPack ) );
					}
					break;
			}
			break;
	}

	//Set new status.
	m_status = newStatus;
}

//------------------------------------------------------------------------------------------------
void BattlePlanUpdate::enableTurret( Bool enable )
{
	AIUpdateInterface *ai = getObject()->getAI();
	if( ai )
	{
		WhichTurretType tur = ai->getWhichTurretForCurWeapon();
		if( tur != TURRET_INVALID )
		{
			ai->setTurretEnabled( tur, enable );
		}
	}
}

//------------------------------------------------------------------------------------------------
void BattlePlanUpdate::recenterTurret()
{
	AIUpdateInterface *ai = getObject()->getAI();
	if( ai )
	{
		WhichTurretType tur = ai->getWhichTurretForCurWeapon();
		if( tur != TURRET_INVALID )
		{
			ai->recenterTurret( tur );
		}
	}
}

//------------------------------------------------------------------------------------------------
Bool BattlePlanUpdate::isTurretInNaturalPosition()
{
	AIUpdateInterface *ai = getObject()->getAI();
	if( ai )
	{
		WhichTurretType tur = ai->getWhichTurretForCurWeapon();
		if( tur != TURRET_INVALID )
		{
			return ai->isTurretInNaturalPosition( tur );
		}
	}
	return false;
}

//------------------------------------------------------------------------------------------------
static void paralyzeTroop( Object *obj, void *userData )
{
	const BattlePlanUpdateModuleData *data = (BattlePlanUpdateModuleData*)userData;
	if( obj->isAnyKindOf( data->m_validMemberKindOf ) )
	{
		if( !obj->isAnyKindOf( data->m_invalidMemberKindOf ) )
		{
			obj->setDisabledUntil( DISABLED_PARALYZED, TheGameLogic->getFrame() + data->m_battlePlanParalyzeFrames );
		}
	}
}

//------------------------------------------------------------------------------------------------
void BattlePlanUpdate::setBattlePlan( BattlePlanStatus plan )
{
	const BattlePlanUpdateModuleData *data = getBattlePlanUpdateModuleData();
	Object *obj = getObject();
	Player *player = obj->getControllingPlayer();
	if( player )
	{
		switch( m_planAffectingArmy )
		{
			case PLANSTATUS_BOMBARDMENT:
			{
				//Remove the previous plan!
				player->changeBattlePlan( PLANSTATUS_BOMBARDMENT, -1, m_bonuses );

				//The only building bonus is actually the turret, and that's already handled!
				break;
			}
			case PLANSTATUS_HOLDTHELINE:
			{
				//Remove the previous plan!
				player->changeBattlePlan( PLANSTATUS_HOLDTHELINE, -1, m_bonuses );

				//Remove building health bonuses
				if( data->m_strategyCenterHoldTheLineMaxHealthScalar != 1.0f )
				{
					BodyModuleInterface *body = obj->getBodyModule();
					body->setMaxHealth( body->getMaxHealth() * 1.0f / data->m_strategyCenterHoldTheLineMaxHealthScalar, data->m_strategyCenterHoldTheLineMaxHealthChangeType );
				}
				break;
			}
			case PLANSTATUS_SEARCHANDDESTROY:
			{
				//Remove the previous plan!
				player->changeBattlePlan( PLANSTATUS_SEARCHANDDESTROY, -1, m_bonuses );

				//Remove sight range bonus
				if( data->m_strategyCenterSearchAndDestroySightRangeScalar != 1.0f )
				{
					obj->setVisionRange( obj->getVisionRange() * 1.0f / data->m_strategyCenterSearchAndDestroySightRangeScalar );
					obj->setShroudClearingRange( obj->getShroudClearingRange() * 1.0f / data->m_strategyCenterSearchAndDestroySightRangeScalar );
				}

				//Remove stealth detection
				if( data->m_strategyCenterSearchAndDestroyDetectsStealth )
				{
					static NameKeyType key_StealthDetectorUpdate = NAMEKEY( "StealthDetectorUpdate" );
					StealthDetectorUpdate *update = (StealthDetectorUpdate*)obj->findUpdateModule( key_StealthDetectorUpdate );
					if( update )
					{
						update->setSDEnabled( false );
					}
				}

				break;
			}
		}

		//Revert to default no-bonuses!
		m_bonuses->m_armorScalar					= 1.0f;
		m_bonuses->m_sightRangeScalar		= 1.0f;
		m_bonuses->m_bombardment					= 0;
		m_bonuses->m_searchAndDestroy		= 0;
		m_bonuses->m_holdTheLine					= 0;

		//Add new bonuses!
		switch( plan )
		{
			case PLANSTATUS_NONE:
				//Paralyze troops!
				player->iterateObjects( paralyzeTroop, (void*)data );
				break;

			case PLANSTATUS_BOMBARDMENT:
				//Set the bombardment bonuses
				m_bonuses->m_bombardment	= 1; //for weapon bonuses

				//Add the new plan!
				player->changeBattlePlan( PLANSTATUS_BOMBARDMENT, 1, m_bonuses );
				break;

			case PLANSTATUS_HOLDTHELINE:
				//Add building health bonuses
				if( data->m_strategyCenterHoldTheLineMaxHealthScalar )
				{
					BodyModuleInterface *body = obj->getBodyModule();
					body->setMaxHealth( body->getMaxHealth() * data->m_strategyCenterHoldTheLineMaxHealthScalar, data->m_strategyCenterHoldTheLineMaxHealthChangeType );
				}

				//Set the hold-the-line bonuses
				m_bonuses->m_armorScalar = data->m_holdTheLineArmorDamageScalar;
				m_bonuses->m_holdTheLine	= 1; //for weapon bonuses

				//Add the new plan!
				player->changeBattlePlan( PLANSTATUS_HOLDTHELINE, 1, m_bonuses );
				break;
			case PLANSTATUS_SEARCHANDDESTROY:
				//Add sight range bonus
				if( data->m_strategyCenterSearchAndDestroySightRangeScalar != 1.0f )
				{
					obj->setVisionRange( obj->getVisionRange() * data->m_strategyCenterSearchAndDestroySightRangeScalar );
					obj->setShroudClearingRange( obj->getShroudClearingRange() * data->m_strategyCenterSearchAndDestroySightRangeScalar );
				}

				//Enable stealth detection
				if( data->m_strategyCenterSearchAndDestroyDetectsStealth )
				{
					static NameKeyType key_StealthDetectorUpdate = NAMEKEY( "StealthDetectorUpdate" );
					StealthDetectorUpdate *update = (StealthDetectorUpdate*)obj->findUpdateModule( key_StealthDetectorUpdate );
					if( update )
					{
						update->setSDEnabled( true );
					}
				}

				//Set the search-and-destroy bonuses
				m_bonuses->m_searchAndDestroy	= 1; //for weapon bonuses
				m_bonuses->m_sightRangeScalar	= data->m_searchAndDestroySightRangeScalar;

				//Add the new plan!
				player->changeBattlePlan( PLANSTATUS_SEARCHANDDESTROY, 1, m_bonuses );
				break;
		}
		m_planAffectingArmy = plan;
	}
}

//------------------------------------------------------------------------------------------------
//Returns the currently active battle plan -- unpacked and ready... returns PLANSTATUS_NONE if in
//transition!
//------------------------------------------------------------------------------------------------
BattlePlanStatus BattlePlanUpdate::getActiveBattlePlan() const
{
	if( m_status == TRANSITIONSTATUS_ACTIVE )
	{
		return m_planAffectingArmy;
	}
	return PLANSTATUS_NONE;
}

//------------------------------------------------------------------------------------------------
void BattlePlanUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

//------------------------------------------------------------------------------------------------
// Xfer method
//	Version Info:
//	1: Initial version
//------------------------------------------------------------------------------------------------
void BattlePlanUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// current plan
	xfer->xferUser( &m_currentPlan, sizeof( BattlePlanStatus ) );

	// desired plan
	xfer->xferUser( &m_desiredPlan, sizeof( BattlePlanStatus ) );

	// plan affecting army
	xfer->xferUser( &m_planAffectingArmy, sizeof( BattlePlanStatus ) );

	// status
	xfer->xferUser( &m_status, sizeof( TransitionStatus ) );

	// next ready frame
	xfer->xferUnsignedInt( &m_nextReadyFrame );

	// don't need to save this interface, it's retrived on object creation
	// SpecialPowerModuleInterface *m_specialPowerModule;

	// invalid settings
	xfer->xferBool( &m_invalidSettings );

	// centering turret
	xfer->xferBool( &m_centeringTurret );

	// bonuses
	xfer->xferReal( &m_bonuses->m_armorScalar );
	xfer->xferInt( &m_bonuses->m_bombardment );
	xfer->xferInt( &m_bonuses->m_searchAndDestroy );
	xfer->xferInt( &m_bonuses->m_holdTheLine );
	xfer->xferReal( &m_bonuses->m_sightRangeScalar );
	m_bonuses->m_validKindOf.xfer( xfer );
	m_bonuses->m_invalidKindOf.xfer( xfer );

	// vision object data
	xfer->xferObjectID( &m_visionObjectID );

}

//------------------------------------------------------------------------------------------------
void BattlePlanUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
