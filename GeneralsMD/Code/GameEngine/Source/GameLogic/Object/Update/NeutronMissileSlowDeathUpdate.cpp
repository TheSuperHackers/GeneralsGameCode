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

// FILE: NeutronMissileSlowDeathBehavior.cpp ////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:   Update module for the neutron missile superweapon object
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/GameState.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameClient/FXList.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/FlammableUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/NeutronMissileSlowDeathUpdate.h"
#include "GameLogic/Module/ToppleUpdate.h"
#include "GameLogic/TerrainLogic.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
NeutronMissileSlowDeathBehaviorModuleData::NeutronMissileSlowDeathBehaviorModuleData( void )
{
	
	for( Int i = 0; i < MAX_NEUTRON_BLASTS; ++i )
	{

		m_blastInfo[ i ].enabled = FALSE;
		m_blastInfo[ i ].delay = 0.0f;
		m_blastInfo[ i ].scorchDelay = 0.0f;
		m_blastInfo[ i ].innerRadius = 0.0f;
		m_blastInfo[ i ].outerRadius = 0.0f;
		m_blastInfo[ i ].maxDamage = 0.0f;
		m_blastInfo[ i ].minDamage = 0.0f;
		m_blastInfo[ i ].toppleSpeed = 0.0f;
		m_blastInfo[ i ].pushForceMag = 0.0f;

	}  // end for i
	m_scorchSize = 0.0f;
	m_fxList		 = NULL;

}  // end NeutronMissileSlowDeathBehaviorModuleData

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void NeutronMissileSlowDeathBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{

  SlowDeathBehaviorModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{
		{ "ScorchMarkSize", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_scorchSize ) },	
		{ "FXList",					INI::parseFXList, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_fxList ) },

		{ "Blast1Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].enabled ) },	
		{ "Blast1Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].delay ) },	
		{ "Blast1ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].scorchDelay ) },	
		{ "Blast1InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].innerRadius ) },	
		{ "Blast1OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].outerRadius ) },	
		{ "Blast1MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].maxDamage ) },	
		{ "Blast1MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].minDamage ) },	
		{ "Blast1ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].toppleSpeed ) },
		{ "Blast1PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_1 ].pushForceMag ) },

		{ "Blast2Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].enabled ) },	
		{ "Blast2Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].delay ) },	
		{ "Blast2ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].scorchDelay ) },	
		{ "Blast2InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].innerRadius ) },	
		{ "Blast2OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].outerRadius ) },	
		{ "Blast2MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].maxDamage ) },	
		{ "Blast2MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].minDamage ) },	
		{ "Blast2ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].toppleSpeed ) },
		{ "Blast2PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_2 ].pushForceMag ) },

		{ "Blast3Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].enabled ) },	
		{ "Blast3Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].delay ) },	
		{ "Blast3ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].scorchDelay ) },	
		{ "Blast3InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].innerRadius ) },	
		{ "Blast3OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].outerRadius ) },	
		{ "Blast3MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].maxDamage ) },	
		{ "Blast3MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].minDamage ) },	
		{ "Blast3ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].toppleSpeed ) },
		{ "Blast3PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_3 ].pushForceMag ) },

		{ "Blast4Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].enabled ) },	
		{ "Blast4Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].delay ) },	
		{ "Blast4ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].scorchDelay ) },	
		{ "Blast4InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].innerRadius ) },	
		{ "Blast4OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].outerRadius ) },	
		{ "Blast4MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].maxDamage ) },	
		{ "Blast4MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].minDamage ) },	
		{ "Blast4ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].toppleSpeed ) },
		{ "Blast4PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_4 ].pushForceMag ) },

		{ "Blast5Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].enabled ) },	
		{ "Blast5Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].delay ) },	
		{ "Blast5ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].scorchDelay ) },	
		{ "Blast5InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].innerRadius ) },	
		{ "Blast5OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].outerRadius ) },	
		{ "Blast5MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].maxDamage ) },	
		{ "Blast5MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].minDamage ) },	
		{ "Blast5ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].toppleSpeed ) },
		{ "Blast5PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_5 ].pushForceMag ) },

		{ "Blast6Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].enabled ) },	
		{ "Blast6Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].delay ) },	
		{ "Blast6ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].scorchDelay ) },			
		{ "Blast6InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].innerRadius ) },	
		{ "Blast6OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].outerRadius ) },	
		{ "Blast6MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].maxDamage ) },	
		{ "Blast6MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].minDamage ) },	
		{ "Blast6ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].toppleSpeed ) },
		{ "Blast6PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_6 ].pushForceMag ) },

		{ "Blast7Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].enabled ) },	
		{ "Blast7Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].delay ) },	
		{ "Blast7ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].scorchDelay ) },			
		{ "Blast7InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].innerRadius ) },	
		{ "Blast7OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].outerRadius ) },	
		{ "Blast7MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].maxDamage ) },	
		{ "Blast7MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].minDamage ) },	
		{ "Blast7ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].toppleSpeed ) },
		{ "Blast7PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_7 ].pushForceMag ) },

		{ "Blast8Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].enabled ) },	
		{ "Blast8Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].delay ) },	
		{ "Blast8ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].scorchDelay ) },			
		{ "Blast8InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].innerRadius ) },	
		{ "Blast8OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].outerRadius ) },	
		{ "Blast8MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].maxDamage ) },	
		{ "Blast8MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].minDamage ) },	
		{ "Blast8ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].toppleSpeed ) },
		{ "Blast8PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_8 ].pushForceMag ) },

		{ "Blast9Enabled", INI::parseBool, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].enabled ) },	
		{ "Blast9Delay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].delay ) },	
		{ "Blast9ScorchDelay", INI::parseDurationReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].scorchDelay ) },			
		{ "Blast9InnerRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].innerRadius ) },	
		{ "Blast9OuterRadius", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].outerRadius ) },	
		{ "Blast9MaxDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].maxDamage ) },	
		{ "Blast9MinDamage", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].minDamage ) },	
		{ "Blast9ToppleSpeed", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].toppleSpeed ) },
		{ "Blast9PushForce", INI::parseReal, NULL, offsetof( NeutronMissileSlowDeathBehaviorModuleData, m_blastInfo[ NEUTRON_BLAST_9 ].pushForceMag ) },

		{ 0, 0, 0, 0 }
	};

  p.add(dataFieldParse);

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
NeutronMissileSlowDeathBehavior::NeutronMissileSlowDeathBehavior( Thing *thing, const ModuleData *moduleData ) 
															: SlowDeathBehavior( thing, moduleData )
{
	Int i;

	m_scorchPlaced = FALSE;
	m_activationFrame = 0;
	for( i = 0; i < MAX_NEUTRON_BLASTS; i++ )
	{

		m_completedBlasts[ i ] = FALSE;
		m_completedScorchBlasts[ i ] = FALSE;

	}  // end for i

}  // end NeutronMissileSlowDeathBehavior

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
NeutronMissileSlowDeathBehavior::~NeutronMissileSlowDeathBehavior( void )
{

}  // end ~NeutronMissileSlowDeathBehavior

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime NeutronMissileSlowDeathBehavior::update( void )
{
/// @todo srj use SLEEPY_UPDATE here
	// get the module data
	const NeutronMissileSlowDeathBehaviorModuleData *modData = getNeutronMissileSlowDeathBehaviorModuleData();

	// call the base class cause we're extending functionality
	SlowDeathBehavior::update();

	// get out of here if we're not activated yet
	if( isSlowDeathActivated() == FALSE )
		return UPDATE_SLEEP_NONE;

	// get the current frame
	UnsignedInt currFrame = TheGameLogic->getFrame();

	// when we become activated we want to do a few things
	if( m_activationFrame == 0 )
	{
		Coord3D pos;
		const Coord3D *missilePos = getObject()->getPosition();

		// get the position to play the effect at on the ground
		pos.x = missilePos->x;
		pos.y = missilePos->y;
		pos.z = TheTerrainLogic->getGroundHeight( pos.x, pos.y );

		// record the frame
		m_activationFrame = currFrame;

		FXList::doFXPos( modData->m_fxList, &pos );
	}  // end if

	// see if it's time for any explosions
	for( Int i = 0; i < MAX_NEUTRON_BLASTS; i++ )
	{

		// if this blast is not enabled we will not bother with it
		if( modData->m_blastInfo[ i ].enabled == FALSE )
			continue;

		// has the time of this blast come
		if( m_completedBlasts[ i ] == FALSE &&
				(currFrame - m_activationFrame > modData->m_blastInfo[ i ].delay) ) 
		{

			// do the blast
			doBlast( &modData->m_blastInfo[ i ] );

			// mark this blast as complete now
			m_completedBlasts[ i ] = TRUE;

		}  // end if

		// has the time for a scorch blast come
		if( m_completedScorchBlasts[ i ] == FALSE &&
		    (currFrame - m_activationFrame > modData->m_blastInfo[ i ].scorchDelay) )
		{

			// do the scorch blast
			doScorchBlast( &modData->m_blastInfo[ i ] );

			// mark this scorch blast as complete now
			m_completedScorchBlasts[ i ] = TRUE;

		}  // end if

	}  // end for i

	return UPDATE_SLEEP_NONE;

}  // end update

// ------------------------------------------------------------------------------------------------
/** Do a single blast for the bomb */
// ------------------------------------------------------------------------------------------------
void NeutronMissileSlowDeathBehavior::doBlast( const BlastInfo *blastInfo )
{

	// sanity
	if( blastInfo == NULL )
		return;

	// get the module data
	const NeutronMissileSlowDeathBehaviorModuleData *modData = getNeutronMissileSlowDeathBehaviorModuleData();

	// get the object and position
	Object *missile = getObject();
	const Coord3D *missilePos = missile->getPosition();

	// setup a damage info structure to do some damage
	DamageInfo damageInfo;
	damageInfo.in.m_damageType = DAMAGE_EXPLOSION;
	damageInfo.in.m_deathType = DEATH_EXPLODED;
	damageInfo.in.m_sourceID = missile->getID();
	damageInfo.in.m_amount = blastInfo->minDamage;

	// scan objects around us and do damage to objects we have "passed over" and are behind us
	if( blastInfo->outerRadius )
	{
		ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( missilePos,
																																			 blastInfo->outerRadius,
																																			 FROM_CENTER_2D, 
																																			 NULL );
		MemoryPoolObjectHolder hold( iter );
		Object *other;
		const Coord3D *otherPos;
		Coord3D forceVector;
		Real dist;
		for( other = iter->first(); other; other = iter->next() )
		{

			// get other position
			otherPos = other->getPosition();

			// compute vector from the missile to other object
			forceVector.x = otherPos->x - missilePos->x;
			forceVector.y = otherPos->y - missilePos->y;
			forceVector.z = otherPos->z - missilePos->z;

			// try to topple other object
			other->topple( &forceVector, blastInfo->toppleSpeed, TOPPLE_OPTIONS_NO_BOUNCE | 
																													 TOPPLE_OPTIONS_NO_FX );

			//
			// compute how much damage we're going to do to this object ... if the object
			// is inside the inner radius we do the full damage.  Outside of the inner radius
			// we do a percentage based on how far away from the inner radius it is, but we
			// will always do at least blastInfo->minDamage amount of damage
			//
			dist = forceVector.length();
			if( dist <= blastInfo->innerRadius )
				damageInfo.in.m_amount = blastInfo->maxDamage;
			else
			{
				Real percent;

				percent = 1.0f - ((dist - blastInfo->innerRadius) / (blastInfo->outerRadius - blastInfo->innerRadius + 0.01f));
				damageInfo.in.m_amount = blastInfo->maxDamage * percent;
				if( damageInfo.in.m_amount < blastInfo->minDamage )
					damageInfo.in.m_amount = blastInfo->minDamage;

			}  // end else
					
			// do actual damage
			if( damageInfo.in.m_amount )
			{

				// do damage
				other->attemptDamage( &damageInfo );

				// place a scorch mark if we haven't already
				if( m_scorchPlaced == FALSE )
				{

					TheGameClient->addScorch( missilePos, modData->m_scorchSize, SCORCH_1 );
					m_scorchPlaced = TRUE;

				}  // end if

			}  // end if

/*
			// apply a small force to the object from the shockwave center
			PhysicsBehavior *physics = other->getPhysics();
			if( physics )
			{
				Coord3D physicsForce = forceVector;

				// normalize the physics force
				physicsForce.normalize();

				//
				// change the magnitude of the physics force to the force amount we want to apply from
				// the shockwave
				//
				physicsForce.x *= blastInfo->pushForceMag;
				physicsForce.y *= blastInfo->pushForceMag;
				physicsForce.z *= blastInfo->pushForceMag;

				// apply the force
				physics->applyForce( &physicsForce );

			}  // end if, physics
*/

		}  // end for, other

	}  // end if, an outer radius exists
		
}  // end doBlast

// ------------------------------------------------------------------------------------------------
/** Do a scorch blast event ... this doesn't do actual damage, but it "scorches" things */
// ------------------------------------------------------------------------------------------------
void NeutronMissileSlowDeathBehavior::doScorchBlast( const BlastInfo *blastInfo )
{

	// sanity
	if( blastInfo == NULL )
		return;

	// get the module data
//	const NeutronMissileSlowDeathBehaviorModuleData *modData = getNeutronMissileSlowDeathBehaviorModuleData();

	// get the object and position
	Object *missile = getObject();
	const Coord3D *missilePos = missile->getPosition();

	// scan objects around us and do damage to objects we have "passed over" and are behind us
	if( blastInfo->outerRadius )
	{
		ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( missilePos,
																																			 blastInfo->outerRadius,
																																			 FROM_CENTER_2D, 
																																			 NULL );
		MemoryPoolObjectHolder hold( iter );
		Object *other;
		for( other = iter->first(); other; other = iter->next() )
		{
					
			// set things aflame
			///@todo can't to this now, flames eat too many particles
	//		static NameKeyType flammableKey = NAMEKEY( "FlammableUpdate" );
	//		FlammableUpdate *update = (FlammableUpdate *)other->findUpdateModule( flammableKey );
	//		if( update )
	//			update->tryToIgnite();

			// set model condition as all burnt up
			other->setModelConditionState( MODELCONDITION_BURNED );

			// remove the shadows from trees
			if( other->isKindOf( KINDOF_SHRUBBERY ) )
			{
				Drawable *draw = other->getDrawable();

				if( draw )
					draw->setShadowsEnabled( FALSE );

			}  // end if

		}  // end for, other

	}  // end if, outer radius exists

}  // end doScorchBlast

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void NeutronMissileSlowDeathBehavior::crc( Xfer *xfer )
{

	// extend base class
	SlowDeathBehavior::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void NeutronMissileSlowDeathBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	SlowDeathBehavior::xfer( xfer );

	// activation frame
	xfer->xferUnsignedInt( &m_activationFrame );

	// size of MAX_NEUTRON_BLASTS since we use it for two arrays here
	UnsignedByte maxNeutronBlasts = MAX_NEUTRON_BLASTS;
	xfer->xferUnsignedByte( &maxNeutronBlasts );
	if( maxNeutronBlasts != MAX_NEUTRON_BLASTS )
	{

		DEBUG_CRASH(( "NeutronMissileSlowDeathBehavior::xfer - Size of MAX_NEUTRON_BLASTS has changed, you must version this xfer code and then you can remove this error message" ));
		throw SC_INVALID_DATA;

	}  // end if

	// completed blasts
	UnsignedByte i;
	for( i = 0; i < maxNeutronBlasts; ++i )
		xfer->xferBool( &m_completedBlasts[ i ] );

	// completed scorch blasts
	for( i = 0; i < maxNeutronBlasts; ++i )
		xfer->xferBool( &m_completedScorchBlasts[ i ] );

	// scorch placed
	xfer->xferBool( &m_scorchPlaced );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void NeutronMissileSlowDeathBehavior::loadPostProcess( void )
{

	// extend base class
	SlowDeathBehavior::loadPostProcess();

}  // end loadPostProcess
