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

// FILE: BeaconClientUpdate.cpp //////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, August 2002
// Desc:   Beacon client update module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h" // This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Module/BeaconClientUpdate.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Radar.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"

//-------------------------------------------------------------------------------------------------
BeaconClientUpdateModuleData::BeaconClientUpdateModuleData() :
	m_framesBetweenRadarPulses(30),
	m_radarPulseDuration(15)
{
}

//-------------------------------------------------------------------------------------------------
BeaconClientUpdateModuleData::~BeaconClientUpdateModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void BeaconClientUpdateModuleData::buildFieldParse(MultiIniFieldParse& p) 
{
  ClientUpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "RadarPulseFrequency",	INI::parseDurationUnsignedInt, NULL, offsetof(BeaconClientUpdateModuleData, m_framesBetweenRadarPulses) },
		{ "RadarPulseDuration",		INI::parseDurationUnsignedInt, NULL, offsetof(BeaconClientUpdateModuleData, m_radarPulseDuration) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BeaconClientUpdate::BeaconClientUpdate( Thing *thing, const ModuleData* moduleData ) :
	ClientUpdateModule( thing, moduleData ),
	m_particleSystemID(INVALID_PARTICLE_SYSTEM_ID),
	m_lastRadarPulse(TheGameLogic->getFrame())
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BeaconClientUpdate::~BeaconClientUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
static ParticleSystem* createParticleSystem( Drawable *draw )
{
	ParticleSystem *system = NULL;
	if (draw)
	{
		Object *obj = draw->getObject();
		if (obj)
		{
			AsciiString templateName;
			templateName.format("BeaconSmoke%6.6X", (0xffffff & obj->getIndicatorColor()));
			const ParticleSystemTemplate *particleTemplate = TheParticleSystemManager->findTemplate( templateName );

			DEBUG_ASSERTCRASH(particleTemplate, ("Could not find particle system %s", templateName.str()));
			
			if (particleTemplate)
			{
				system = TheParticleSystemManager->createParticleSystem( particleTemplate );
				if (system)
					system->attachToDrawable( draw );
			}
			else// This is a failsafe... if someone has monkeyed with the particle system names, or the MP house colors
			{// THis this will whip up a new particle system to match the house color provided
				templateName.format("BeaconSmokeFFFFFF");
				const ParticleSystemTemplate *failsafeTemplate = TheParticleSystemManager->findTemplate( templateName );
				DEBUG_ASSERTCRASH(failsafeTemplate, ("Doh, this is bad \n I Could not even find the white particle system to make a failsafe system out of."));
				if (failsafeTemplate)
				{
					system = TheParticleSystemManager->createParticleSystem( failsafeTemplate );
					if (system)
					{
						system->attachToDrawable( draw );
						system->tintAllColors( obj->getIndicatorColor() );
					}
				}
			}
		}
	}
	return system;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BeaconClientUpdate::hideBeacon( void )
{
	Drawable *draw = getDrawable();
	if (draw)
	{
		draw->setDrawableHidden( true );
		draw->setShadowsEnabled( false );
	}

	ParticleSystem *system;
	if (draw && m_particleSystemID == INVALID_PARTICLE_SYSTEM_ID)
	{
		system = createParticleSystem( draw );
		if (system)
			m_particleSystemID = system->getSystemID();
	}

	// clean up particle system
	if (m_particleSystemID != INVALID_PARTICLE_SYSTEM_ID)
	{
		ParticleSystem *system = TheParticleSystemManager->findParticleSystem( m_particleSystemID );

		if( system )
			system->stop();

	}  // end if
	
//	DEBUG_LOG(("in hideBeacon(): draw=%d, m_particleSystemID=%d", draw, m_particleSystemID));

}

//-------------------------------------------------------------------------------------------------
/** The client update callback. */
//-------------------------------------------------------------------------------------------------
void BeaconClientUpdate::clientUpdate( void )
{
	Drawable *draw = getDrawable();
	if (!draw)
		return;

	if (m_particleSystemID == INVALID_PARTICLE_SYSTEM_ID)
	{
		ParticleSystem *system = createParticleSystem( draw );
		if( system )
			m_particleSystemID = system->getSystemID();
	}

	if (!draw->isDrawableEffectivelyHidden())
	{
		BeaconClientUpdateModuleData *moduleData = (BeaconClientUpdateModuleData *)getModuleData();
		if (TheGameLogic->getFrame() > m_lastRadarPulse + moduleData->m_framesBetweenRadarPulses)
		{
			TheRadar->createEvent( draw->getPosition(), RADAR_EVENT_BEACON_PULSE, moduleData->m_radarPulseDuration * SECONDS_PER_LOGICFRAME_REAL );
			m_lastRadarPulse = TheGameLogic->getFrame();
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BeaconClientUpdate::crc( Xfer *xfer )
{
	
	// extend base class
	ClientUpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BeaconClientUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	ClientUpdateModule::xfer( xfer );

	// particle system ID
	xfer->xferUser( &m_particleSystemID, sizeof( ParticleSystemID ) );

	// last radar pulse
	xfer->xferUnsignedInt( &m_lastRadarPulse );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BeaconClientUpdate::loadPostProcess( void )
{

	// extend base class
	ClientUpdateModule::loadPostProcess();

}  // end loadPostProcess
