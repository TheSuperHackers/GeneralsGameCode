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

// FILE: StealthDetectorUpdate.cpp ////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, May 2002
// Desc:	 An update that checks for a status bit to stealth the owning object
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_STEALTHLEVEL_NAMES

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameUtility.h"
#include "Common/MiscAudio.h"
#include "Common/Radar.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ParticleSys.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/StealthDetectorUpdate.h"
#include "Common/BitFlagsIO.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"


//-------------------------------------------------------------------------------------------------
void StealthDetectorUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "DetectionRate",							INI::parseDurationUnsignedInt,			NULL, offsetof( StealthDetectorUpdateModuleData, m_updateRate ) },
		{ "DetectionRange",							INI::parseReal,											NULL, offsetof( StealthDetectorUpdateModuleData, m_detectionRange ) },
		{ "InitiallyDisabled",					INI::parseBool,											NULL, offsetof( StealthDetectorUpdateModuleData, m_initiallyDisabled ) },
		{ "PingSound",									INI::parseAudioEventRTS,						NULL, offsetof( StealthDetectorUpdateModuleData, m_pingSound ) },
		{ "LoudPingSound",							INI::parseAudioEventRTS,						NULL, offsetof( StealthDetectorUpdateModuleData, m_loudPingSound ) },
		{ "IRBeaconParticleSysName",		INI::parseParticleSystemTemplate,		NULL, offsetof( StealthDetectorUpdateModuleData, m_IRBeaconParticleSysTmpl ) },
		{ "IRParticleSysName",					INI::parseParticleSystemTemplate,		NULL, offsetof( StealthDetectorUpdateModuleData, m_IRParticleSysTmpl ) },
		{ "IRBrightParticleSysName",		INI::parseParticleSystemTemplate,		NULL, offsetof( StealthDetectorUpdateModuleData, m_IRBrightParticleSysTmpl ) },
		{ "IRGridParticleSysName",			INI::parseParticleSystemTemplate,		NULL, offsetof( StealthDetectorUpdateModuleData, m_IRGridParticleSysTmpl ) },
		{ "IRParticleSysBone",					INI::parseAsciiString,							NULL, offsetof( StealthDetectorUpdateModuleData, m_IRParticleSysBone ) },
		{ "ExtraRequiredKindOf",				KindOfMaskType::parseFromINI,				NULL, offsetof( StealthDetectorUpdateModuleData, m_extraDetectKindof ) },
		{ "ExtraForbiddenKindOf",				KindOfMaskType::parseFromINI,				NULL, offsetof( StealthDetectorUpdateModuleData, m_extraDetectKindofNot ) },
		{ "CanDetectWhileGarrisoned",		INI::parseBool,											NULL, offsetof( StealthDetectorUpdateModuleData, m_canDetectWhileGarrisoned ) },
		{ "CanDetectWhileContained",		INI::parseBool,											NULL, offsetof( StealthDetectorUpdateModuleData, m_canDetectWhileTransported ) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StealthDetectorUpdate::StealthDetectorUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	const StealthDetectorUpdateModuleData *data = getStealthDetectorUpdateModuleData();
	m_enabled = !data->m_initiallyDisabled;
	// start these guys with random phasings so that we don't
	// have all of 'em check on the same frame.
	setWakeFrame(getObject(), m_enabled ? UPDATE_SLEEP(GameLogicRandomValue(1, data->m_updateRate)) : UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StealthDetectorUpdate::~StealthDetectorUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StealthDetectorUpdate::setSDEnabled( Bool enabled )
{
	m_enabled = enabled;
	setWakeFrame(getObject(), m_enabled ? UPDATE_SLEEP_NONE : UPDATE_SLEEP_FOREVER);
}





//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class PartitionFilterStealthedOrStealthGarrisoned : public PartitionFilter
{
public:
	PartitionFilterStealthedOrStealthGarrisoned() { }

	virtual Bool allow(Object *objOther);

#if defined(RTS_DEBUG)
	virtual const char* debugGetName() { return "PartitionFilterStealthedOrStealthGarrisoned"; }
#endif
};

Bool PartitionFilterStealthedOrStealthGarrisoned::allow( Object *objOther)
{
	if( ! objOther )
		return FALSE;

	if( objOther->getStatusBits().test( OBJECT_STATUS_STEALTHED ) )
		return TRUE;

	ContainModuleInterface *contain = objOther->getContain();
	if( contain && contain->isGarrisonable() && contain->getStealthUnitsContained() )
		return TRUE;

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime StealthDetectorUpdate::update( void )
{
	const StealthDetectorUpdateModuleData *data = getStealthDetectorUpdateModuleData();
	Object* self = getObject();

	if (self->isEffectivelyDead())
		return UPDATE_SLEEP_FOREVER;

	// We have to wait until we are fully constructed, but we will detect the moment we finish
	if( self->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) )
		return UPDATE_SLEEP_NONE;

	// We turn off forever the moment we are sold.
	if( self->testStatus(OBJECT_STATUS_SOLD) )
		return UPDATE_SLEEP_FOREVER;

	//Are we contained by anything?
	Object *containedBy = self->getContainedBy();
	if( containedBy )
	{
		ContainModuleInterface *contain = containedBy->getContain();
		if( contain )
		{
			//Are we eligible to detect stealth while in a container?
			if( contain->isGarrisonable() )
			{
				//We are in a garrisonable structure.
				if( !data->m_canDetectWhileGarrisoned )
				{
					//But we can't detect stuff while inside.
					return UPDATE_SLEEP(data->m_updateRate);
				}
			}
			else if( !data->m_canDetectWhileTransported )
			{
				//We are in a normal container and can't detect!
				return UPDATE_SLEEP(data->m_updateRate);
			}
		}
	}

	// only consider items that are currently stealthed.
	PartitionFilterStealthedOrStealthGarrisoned		filterStealthOrStealthGarrisoned;
	//PartitionFilterAcceptByObjectStatus		filterStatus(OBJECT_STATUS_STEALTHED, 0);
	PartitionFilterRelationship						filterTeam(self, PartitionFilterRelationship::ALLOW_ENEMIES | PartitionFilterRelationship::ALLOW_NEUTRAL );
	PartitionFilterAcceptByKindOf					filterKindof(data->m_extraDetectKindof, data->m_extraDetectKindofNot);
	PartitionFilterSameMapStatus					filterMapStatus(getObject());
	PartitionFilter*											filters[] = { &filterStealthOrStealthGarrisoned, &filterTeam, &filterKindof, &filterMapStatus, NULL };

	Real visionRange = self->getVisionRange();
	if( data->m_detectionRange > 0.0f )
	{
		visionRange = data->m_detectionRange;
	}
	Bool foundSomeone = FALSE;

	SimpleObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(
								self, visionRange, FROM_CENTER_2D, filters);
	MemoryPoolObjectHolder hold(iter);
	for (Object *them = iter->first(); them; them = iter->next())
	{
		if ( them->isEffectivelyDead() )
			continue;

		StealthUpdate* stealth = them->getStealth();
		if ( stealth )
		{

			// we have found someone
			foundSomeone = TRUE;

			//
			// if this object was not previously detected it is now being revealed and we
			// want to do some UI feedback
			//
			if( them->testStatus( OBJECT_STATUS_DETECTED ) == FALSE )
			{

				// for the player revealing the stealth unit do some UI feedback
				if( rts::getObservedOrLocalPlayer() == self->getControllingPlayer() &&
						self->getRelationship( them ) != ALLIES )
				{
					Bool doFeedback = TRUE;

					//Kris: Aug 18, 2003 -- This whole system is no longer viable with the inclusion of so many
					//stealth units. It's possible to get spammed hundreds of times in a row in MD_GLA02 because
					//we're blindly creating radar events for each detection (unless they are mines). The best thing
					//to do is tryEvent for everything...
					doFeedback = TheRadar->tryEvent( RADAR_EVENT_STEALTH_DISCOVERED, them->getPosition() );
					//OLD CODE:
					/*
					// do a radar event, for mines we only make events if there weren't other
					// mine events within close proximity and time to other mines
					//
					if( them->isKindOf( KINDOF_MINE ) || them->isKindOf( KINDOF_BOOBY_TRAP ) || them->isKindOf( KINDOF_DEMOTRAP ) )
						doFeedback = TheRadar->tryEvent( RADAR_EVENT_STEALTH_DISCOVERED, them->getPosition() );
					else
 						TheRadar->createEvent( them->getPosition(), RADAR_EVENT_STEALTH_DISCOVERED );
					*/

					// do audio and UI message if we need to do feedback
					if( doFeedback )
					{
 						// audio msg
 						static AudioEventRTS discoveredSound = TheAudio->getMiscAudio()->m_stealthDiscoveredSound;
 						discoveredSound.setPlayerIndex( self->getControllingPlayer()->getPlayerIndex() );
 						TheAudio->addAudioEvent( &discoveredSound );
 						// ui msg
 						TheInGameUI->message( TheGameText->fetch( "MESSAGE:StealthDiscovered" ) );

            // If revealing this unit is suppose to cause an Eva event, do it
            EvaMessage message = stealth->getEnemyDetectionEvaEvent();
            if ( message != EVA_Invalid && TheEva != NULL )
            {
              TheEva->setShouldPlay( message );
            }
					}

				}

				// for the unit being revealed, do some UI feedback
				if( rts::getObservedOrLocalPlayer() == them->getControllingPlayer() &&
						self->getRelationship( them ) != ALLIES )
				{
 					Bool doFeedback = TRUE;

					//
					// do a radar event, for mines we only make events if there weren't other
					// mine events within close proximity and time to other mines
					//
					if( them->isKindOf( KINDOF_MINE ) || them->isKindOf( KINDOF_BOOBY_TRAP ) || them->isKindOf( KINDOF_DEMOTRAP ) )
						doFeedback = TheRadar->tryEvent( RADAR_EVENT_STEALTH_NEUTRALIZED, them->getPosition() );
					else
 						TheRadar->createEvent( them->getPosition(), RADAR_EVENT_STEALTH_NEUTRALIZED );

					// do audio and UI message if we need to do feedback
					if( doFeedback )
					{

 						// audio msg
 						static AudioEventRTS neutralizedSound = TheAudio->getMiscAudio()->m_stealthNeutralizedSound;
 						neutralizedSound.setPlayerIndex( them->getControllingPlayer()->getPlayerIndex() );
 						TheAudio->addAudioEvent( &neutralizedSound );
 						// ui msg
 						TheInGameUI->message( TheGameText->fetch( "MESSAGE:StealthNeutralized" ) );

            // If revealing this unit is suppose to cause an Eva event, do it
            EvaMessage message = stealth->getOwnDetectionEvaEvent();
            if ( message != EVA_Invalid && TheEva != NULL )
            {
              TheEva->setShouldPlay( message );
            }
					}

				}

			}

			// updateRate PLUS 1 is necessary to ensure it stays detected 'till we are called again...
			stealth->markAsDetected(data->m_updateRate + 1);

			/** @todo srj -- evil hack here... this whole heat-vision thing is fucked.
				don't want it on mines but no good way to do that. hack for now. */
			Drawable *theirDraw = them->getDrawable();
			if ( theirDraw && !them->isKindOf(KINDOF_MINE))
			{
				theirDraw->setSecondMaterialPassOpacity( 1.0f );
			}

			if (data->m_IRGridParticleSysTmpl)
			{
				const ParticleSystemTemplate *gridTemplate = data->m_IRGridParticleSysTmpl;
				if (gridTemplate)
				{
					ParticleSystem *sys = TheParticleSystemManager->createParticleSystem( gridTemplate );//GRID
					if (sys)
					{
						Coord3D gridPosition = *them->getPosition();
						gridPosition.z = self->getPosition()->z + 17;
						gridPosition.x -= ((Int)gridPosition.x)%12;
						gridPosition.y -= ((Int)gridPosition.y)%12;

						sys->setPosition( &gridPosition );
					}
				}
			}

		}
		else // perhaps they are garrisoning something stealthy, eh?
		{
			ContainModuleInterface *contain = them->getContain();
			if( contain && contain->isGarrisonable() && contain->getStealthUnitsContained() )
			{
				Object* rider = NULL;
				for(ContainedItemsList::const_iterator it = contain->getContainedItemsList()->begin(); it != contain->getContainedItemsList()->end(); ++it)
				{
					rider = *it;

          StealthUpdate *stealth = rider->getStealth();
					if ( stealth )
					{
						// we have found someone
						foundSomeone = TRUE;
						if( self->getControllingPlayer() != rider->getControllingPlayer() && self->getRelationship( rider ) != ALLIES )
						{
							stealth->markAsDetected(data->m_updateRate + 2);
						}

					}
				}
			}
		}
	}


  const Player *localPlayer = rts::getObservedOrLocalPlayer();

	//Make sure the detector is visible to the local player before we add effects or sounds.
	if ( self->getShroudedStatus( localPlayer->getPlayerIndex() ) <= OBJECTSHROUD_PARTIAL_CLEAR )
	{
    if ( self->testStatus( OBJECT_STATUS_STEALTHED ) == FALSE || self->getControllingPlayer() == localPlayer )
    {
		  Drawable *myDraw = self->getDrawable();
		  Coord3D bonePosition = {-1.66f,5.5f,15};
		  if (myDraw)
			  myDraw->getPristineBonePositions( data->m_IRParticleSysBone.str(), 0, &bonePosition, NULL, 1);

		  const ParticleSystemTemplate *pingTemplate;
		  if ( foundSomeone )
			  pingTemplate = data->m_IRBrightParticleSysTmpl;
		  else
			  pingTemplate = data->m_IRParticleSysTmpl;

		  if (pingTemplate)
		  {
			  ParticleSystem *sys = TheParticleSystemManager->createParticleSystem( pingTemplate );
			  if (sys)
			  {
				  if (myDraw)
					  sys->attachToDrawable( myDraw );
				  else
					  sys->attachToObject( self );

				  sys->setPosition( &bonePosition );
			  }
		  }

		  const ParticleSystemTemplate *beaconTemplate = data->m_IRBeaconParticleSysTmpl;
		  if (beaconTemplate)
		  {
			  ParticleSystem *sys = TheParticleSystemManager->createParticleSystem( beaconTemplate );//BEACON
			  if (sys)
			  {
				  if (myDraw)
					  sys->attachToDrawable( myDraw );
				  else
					  sys->attachToObject( self );

				  sys->setPosition( &bonePosition );

			  }
		  }

		  AudioEventRTS IRPingSound;
		  if (foundSomeone)
			  IRPingSound = data->m_loudPingSound;
		  else
			  IRPingSound = data->m_pingSound;

		  IRPingSound.setObjectID( self->getID() );
		  TheAudio->addAudioEvent(&IRPingSound);

    }

	}


	return UPDATE_SLEEP(data->m_updateRate);

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StealthDetectorUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StealthDetectorUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// enabled
	xfer->xferBool( &m_enabled );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StealthDetectorUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
