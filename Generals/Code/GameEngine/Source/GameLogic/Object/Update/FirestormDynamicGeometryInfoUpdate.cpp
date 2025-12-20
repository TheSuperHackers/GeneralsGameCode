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

// FILE: FirestormDynamicGeometryInfoUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:   Update module adds the molestation of a particle system to Geometry changing
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Xfer.h"

#include "GameClient/GameClient.h"
#include "GameClient/InGameUI.h"
#include "GameClient/FXList.h"
#include "GameClient/ParticleSys.h"

#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Module/FirestormDynamicGeometryInfoUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FirestormDynamicGeometryInfoUpdateModuleData::FirestormDynamicGeometryInfoUpdateModuleData() : DynamicGeometryInfoUpdateModuleData()
{

	for( Int i = 0; i < MAX_FIRESTORM_SYSTEMS; i++ )
		m_particleSystem[ i ] = NULL;
	m_fxList = NULL;
	m_particleOffsetZ = 0.0f;
	m_scorchSize = 0.0f;
	m_delayBetweenDamageFrames = 0.0f;
	m_damageAmount = 0.0f;
	m_maxHeightForDamage = 20.0f;

}

//-------------------------------------------------------------------------------------------------
/*static*/ void FirestormDynamicGeometryInfoUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	DynamicGeometryInfoUpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "DelayBetweenDamageFrames", INI::parseDurationReal, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_delayBetweenDamageFrames ) },
		{ "DamageAmount", INI::parseReal, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_damageAmount ) },
		{ "MaxHeightForDamage", INI::parseReal, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_maxHeightForDamage ) },
		{ "ParticleSystem1", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 0 ] ) },
		{ "ParticleSystem2", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 1 ] ) },
		{ "ParticleSystem3", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 2 ] ) },
		{ "ParticleSystem4", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 3 ] ) },
		{ "ParticleSystem5", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 4 ] ) },
		{ "ParticleSystem6", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 5 ] ) },
		{ "ParticleSystem7", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 6 ] ) },
		{ "ParticleSystem8", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 7 ] ) },
		{ "ParticleSystem9", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 8 ] ) },
		{ "ParticleSystem10", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 9 ] ) },
		{ "ParticleSystem11", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 10 ] ) },
		{ "ParticleSystem12", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 11 ] ) },
		{ "ParticleSystem13", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 12 ] ) },
		{ "ParticleSystem14", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 13 ] ) },
		{ "ParticleSystem15", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 14 ] ) },
		{ "ParticleSystem16", INI::parseParticleSystemTemplate, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleSystem[ 15 ] ) },
		{ "FXList",		INI::parseFXList, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_fxList ) },
		{ "ParticleOffsetZ", INI::parseReal, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_particleOffsetZ ) },
		{ "ScorchSize", INI::parseReal, NULL, offsetof( FirestormDynamicGeometryInfoUpdateModuleData, m_scorchSize ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
FirestormDynamicGeometryInfoUpdate::FirestormDynamicGeometryInfoUpdate( Thing *thing, const ModuleData* moduleData ) : DynamicGeometryInfoUpdate( thing, moduleData )
{

	for( Int i = 0; i < MAX_FIRESTORM_SYSTEMS; i++ )
		m_myParticleSystemID[ i ] = INVALID_PARTICLE_SYSTEM_ID;
	m_effectsFired = FALSE;
	m_scorchPlaced = FALSE;
	m_lastDamageFrame = 0;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FirestormDynamicGeometryInfoUpdate::~FirestormDynamicGeometryInfoUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime FirestormDynamicGeometryInfoUpdate::update( void )
{

	// extend functionality
	DynamicGeometryInfoUpdate::update();

	// get out of here if we haven't fired yet
	if( m_startingDelayCountdown > 0 )
		return UPDATE_SLEEP_NONE;

	// get the module data
	const FirestormDynamicGeometryInfoUpdateModuleData *modData = getFirestormDynamicGeometryInfoUpdateModuleData();

	// start effects for the first time
	if( m_effectsFired == FALSE )
	{
		ParticleSystem *sys;

		//
		// construct a position to play the particle effects ... that is on the ground, but
		// note that we add a small amount to it to avoid particles from the system (especially
		// flat XY particles) from popping up through the terrain
		//
		Coord3D pos;
		pos.x = getObject()->getPosition()->x;
		pos.y = getObject()->getPosition()->y;
		pos.z = modData->m_particleOffsetZ + TheTerrainLogic->getGroundHeight( pos.x, pos.y );

		// do the particle systems
		for( Int i = 0; i < MAX_FIRESTORM_SYSTEMS; i++ )
		{

			sys = TheParticleSystemManager->createParticleSystem( modData->m_particleSystem[ i ] );
			if( sys )
			{

				sys->setPosition( &pos );
				m_myParticleSystemID[ i ] = sys->getSystemID();

			}

		}

		// do the FX list
		FXList::doFXObj( modData->m_fxList, getObject() );

		// effects have been fired
		m_effectsFired = TRUE;

	}

	//
	// Parent goes first, and we use part of what he did to alter our bound particle system
	// get and update our running particle system if we have one
	//
	for( Int i = 0; i < MAX_FIRESTORM_SYSTEMS; i++ )
	{

		if( m_myParticleSystemID[ i ] )
		{
			ParticleSystem *sys = TheParticleSystemManager->findParticleSystem( m_myParticleSystemID[ i ] );

			if( sys )
			{
				ParticleSystemInfo::EmissionVolumeType type = sys->getEmisionVolumeType();

				if( type == ParticleSystemInfo::EmissionVolumeType::SPHERE )
					sys->setEmissionVolumeSphereRadius( getObject()->getGeometryInfo().getMajorRadius() );
				else if( type == ParticleSystemInfo::EmissionVolumeType::CYLINDER )
					sys->setEmissionVolumeCylinderRadius( getObject()->getGeometryInfo().getMajorRadius() );

			}
			else
			{

				// this system not found (it probably died)... stop trying to find it in the future
				m_myParticleSystemID[ i ] = INVALID_PARTICLE_SYSTEM_ID;

			}

		}

	}

	// when we first start running backward ... make a scorch mark
	if( m_switchedDirections == TRUE && m_scorchPlaced == FALSE )
	{

		TheGameClient->addScorch( getObject()->getPosition(), modData->m_scorchSize, SCORCH_1 );
		m_scorchPlaced = TRUE;

	}

	// scan and do some damage every once in a while
	if( TheGameLogic->getFrame() - m_lastDamageFrame >= modData->m_delayBetweenDamageFrames )
	{

		doDamageScan();
		m_lastDamageFrame = TheGameLogic->getFrame();

	}

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void FirestormDynamicGeometryInfoUpdate::doDamageScan( void )
{

	// get the module data
	const FirestormDynamicGeometryInfoUpdateModuleData *modData = getFirestormDynamicGeometryInfoUpdateModuleData();

	// get the object and position
	Object *firestorm = getObject();
	const Coord3D *firestormPos = firestorm->getPosition();

	// setup a damage info structure to do some damage
	DamageInfo damageInfo;
	damageInfo.in.m_damageType = DAMAGE_FLAME;
	damageInfo.in.m_deathType = DEATH_BURNED;
	damageInfo.in.m_sourceID = firestorm->getID();
	damageInfo.in.m_amount = modData->m_damageAmount;

	// get the current bounding circle size for the firestorm
	Real boundingCircle = firestorm->getGeometryInfo().getBoundingCircleRadius();

	// scan objects around us and do damage to objects we have "passed over"
	if( boundingCircle )
	{
		ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( firestormPos,
																																			 boundingCircle,
																																			 FROM_BOUNDINGSPHERE_2D,
																																			 NULL );
		MemoryPoolObjectHolder hold( iter );
		Object *other;
		for( other = iter->first(); other; other = iter->next() )
		{
			// it's too high above us. skip it.
			if (other->getPosition()->z > firestorm->getPosition()->z + modData->m_maxHeightForDamage)
				continue;

			// do damage
			other->attemptDamage( &damageInfo );

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FirestormDynamicGeometryInfoUpdate::crc( Xfer *xfer )
{

	// extend base class
	DynamicGeometryInfoUpdate::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FirestormDynamicGeometryInfoUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DynamicGeometryInfoUpdate::xfer( xfer );

	// particle system ids
	xfer->xferUser( m_myParticleSystemID, sizeof( ParticleSystemID ) * MAX_FIRESTORM_SYSTEMS );

	// effects fired
	xfer->xferBool( &m_effectsFired );

	// scorch placed
	xfer->xferBool( &m_scorchPlaced );

	// last damage frame
	xfer->xferUnsignedInt( &m_lastDamageFrame );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FirestormDynamicGeometryInfoUpdate::loadPostProcess( void )
{

	// extend base class
	DynamicGeometryInfoUpdate::loadPostProcess();

}
