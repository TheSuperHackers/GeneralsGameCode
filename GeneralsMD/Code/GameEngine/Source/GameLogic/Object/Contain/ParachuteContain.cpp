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

// FILE: ParachuteContain.cpp //////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, March 2002
// Desc:   Contain module for transport units.
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/CRCDebug.h"
#include "Common/Player.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameLogic/AIPathfind.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ParachuteContain.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

#include "GameClient/Drawable.h"


const Real NO_START_Z = 1e10;




// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
ParachuteContainModuleData::ParachuteContainModuleData() :
	m_pitchRateMax(0),
	m_rollRateMax(0),
	m_lowAltitudeDamping(0.2f),
	m_paraOpenDist(0.0f),
	m_freeFallDamagePercent(0.5f),
	m_killWhenLandingInWaterSlop(10.0f)
{
}

//-------------------------------------------------------------------------------------------------
void ParachuteContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  OpenContainModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "PitchRateMax",	INI::parseAngularVelocityReal,		NULL, offsetof( ParachuteContainModuleData, m_pitchRateMax ) },
		{ "RollRateMax",	INI::parseAngularVelocityReal,		NULL, offsetof( ParachuteContainModuleData, m_rollRateMax ) },
		{ "LowAltitudeDamping",	INI::parseReal,		NULL, offsetof( ParachuteContainModuleData, m_lowAltitudeDamping ) },
		{ "ParachuteOpenDist",	INI::parseReal,		NULL, offsetof( ParachuteContainModuleData, m_paraOpenDist ) },
		{ "KillWhenLandingInWaterSlop",	INI::parseReal,		NULL, offsetof( ParachuteContainModuleData, m_killWhenLandingInWaterSlop ) },
		{ "FreeFallDamagePercent",	INI::parsePercentToReal,		NULL, offsetof( ParachuteContainModuleData, m_freeFallDamagePercent ) },
		{ "ParachuteOpenSound", INI::parseAudioEventRTS, NULL, offsetof( ParachuteContainModuleData, m_parachuteOpenSound ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

// PUBLIC /////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ParachuteContain::ParachuteContain( Thing *thing, const ModuleData *moduleData ) :
								 OpenContain( thing, moduleData )
{
	m_opened = false;
	m_needToUpdateParaBones = true;
	m_needToUpdateRiderBones = true;
	m_pitch = 0;
	m_roll = 0;
	m_pitchRate = 0;
	m_rollRate = 0;
	m_isLandingOverrideSet = FALSE;
	m_startZ = NO_START_Z;

	//Added By Sadullah Nader
	//Initializations
	m_landingOverride.zero();
	m_paraAttachBone.zero();
	m_paraAttachOffset.zero();
	m_paraSwayBone.zero();
	m_paraSwayOffset.zero();
	m_riderAttachBone.zero();
	m_riderAttachOffset.zero();
	m_riderSwayBone.zero();
	m_riderSwayOffset.zero();

	//
	const ParachuteContainModuleData* d = getParachuteContainModuleData();
	if (d)
	{
		m_pitchRate = GameLogicRandomValueReal(-d->m_pitchRateMax, d->m_pitchRateMax);
		m_rollRate = GameLogicRandomValueReal(-d->m_rollRateMax, d->m_rollRateMax);
	}

	getObject()->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_PARACHUTING ) );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ParachuteContain::~ParachuteContain( void )
{
}

//-------------------------------------------------------------------------------------------------
/**
	this is called whenever a drawable is bound to the object.
	drawable is NOT guaranteed to be non-null.
*/
void ParachuteContain::onDrawableBoundToObject()
{
	Drawable* draw = getObject()->getDrawable();
	if (draw)
		draw->setDrawableHidden(!m_opened);
}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::calcSwayMtx(const Coord3D* offset, Matrix3D* mtx)
{
	mtx->Make_Identity();
	mtx->Translate(offset->x, offset->y, offset->z);
	mtx->In_Place_Pre_Rotate_X(m_roll);
	mtx->In_Place_Pre_Rotate_Y(m_pitch);
	mtx->Translate(-offset->x, -offset->y, -offset->z);
}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::updateBonePositions()
{
	if (m_needToUpdateParaBones)
	{
		m_needToUpdateParaBones = false;	// yeah, even if not found.

		Drawable* parachuteDraw = getObject()->getDrawable();
		if (parachuteDraw)
		{
			if (parachuteDraw->getPristineBonePositions( "PARA_COG", 0, &m_paraSwayBone, NULL, 1) != 1)
			{
				DEBUG_CRASH(("PARA_COG not found"));
				m_paraSwayBone.zero();
			}

			if (parachuteDraw->getPristineBonePositions( "PARA_ATTCH", 0, &m_paraAttachBone, NULL, 1 ) != 1)
			{
				DEBUG_CRASH(("PARA_ATTCH not found"));
				m_paraAttachBone.zero();
			}
		}
		//DEBUG_LOG(("updating para bone positions %d...",TheGameLogic->getFrame()));
	}

	if (m_needToUpdateRiderBones)
	{

		m_needToUpdateRiderBones = false;	// yeah, even if not found.

		Object* rider = (getContainCount() > 0) ? getContainList().front() : NULL;
		Drawable* riderDraw = rider ? rider->getDrawable() : NULL;
		if (riderDraw)
		{
			if (riderDraw->getPristineBonePositions( "PARA_MAN", 0, &m_riderAttachBone, NULL, 1) != 1)
			{
				//DEBUG_LOG(("*** No parachute-attach bone... using object height!"));
				m_riderAttachBone.zero();
				m_riderAttachBone.z += riderDraw->getDrawableGeometryInfo().getMaxHeightAbovePosition();
			}
		}

		//DEBUG_LOG(("updating rider bone positions %d...",TheGameLogic->getFrame()));
	}
}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::updateOffsetsFromBones()
{
	const Coord3D* objPos = getObject()->getPosition();

	getObject()->convertBonePosToWorldPos(&m_paraSwayBone, NULL, &m_paraSwayOffset, NULL);
	m_paraSwayOffset.x -= objPos->x;
	m_paraSwayOffset.y -= objPos->y;
	m_paraSwayOffset.z -= objPos->z;

	getObject()->convertBonePosToWorldPos(&m_paraAttachBone, NULL, &m_paraAttachOffset, NULL);
	m_paraAttachOffset.x -= objPos->x;
	m_paraAttachOffset.y -= objPos->y;
	m_paraAttachOffset.z -= objPos->z;

	Object* rider = (getContainCount() > 0) ? getContainList().front() : NULL;
	if (rider)
	{
		const Coord3D* riderPos = rider->getPosition();

		rider->convertBonePosToWorldPos(&m_riderAttachBone, NULL, &m_riderAttachOffset, NULL);
		m_riderAttachOffset.x -= riderPos->x;
		m_riderAttachOffset.y -= riderPos->y;
		m_riderAttachOffset.z -= riderPos->z;

		m_riderAttachOffset.x = m_paraAttachOffset.x - m_riderAttachOffset.x;
		m_riderAttachOffset.y = m_paraAttachOffset.y - m_riderAttachOffset.y;
		m_riderAttachOffset.z = m_paraAttachOffset.z - m_riderAttachOffset.z;

		m_riderSwayOffset.x = m_paraSwayOffset.x - m_riderAttachOffset.x;
		m_riderSwayOffset.y = m_paraSwayOffset.y - m_riderAttachOffset.y;
		m_riderSwayOffset.z = m_paraSwayOffset.z - m_riderAttachOffset.z;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
	can this container contain this kind of object?
	and, if checkCapacity is TRUE, does this container have enough space left to hold the given unit?
*/
Bool ParachuteContain::isValidContainerFor(const Object* rider, Bool checkCapacity) const
{
	if (!rider)
		return false;

	// extend functionality
	if( OpenContain::isValidContainerFor( rider, checkCapacity ) == false )
		return false;

	Int transportSlotCount = rider->getTransportSlotCount();

	// if 0, this object isn't transportable.
	// (exception: infantry are always transportable by parachutes, regardless
	// of this.... this allows us to paradrop pilots, but not transport them
	// by other means)
	if (transportSlotCount == 0 && !rider->isKindOf(KINDOF_INFANTRY) && !rider->isKindOf(KINDOF_PARACHUTABLE))
		return false;

	// we can only "hold" one item at a time.
	if (getContainCount() > 0)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ParachuteContain::containReactToTransformChange()
{
	// a bit of a cheese festival here... hidden is a flag, not a counter, so when we are dumped
	// from a plane, we might get drawn for a frame before our update is called, meaning we
	// should be briefly visible. put this here to ensure we stay hidden appropriately.
	Drawable* draw = getObject()->getDrawable();
	if (draw)
		draw->setDrawableHidden(!m_opened);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ParachuteContain::update( void )
{
	OpenContain::update();

	Object* parachute = getObject();
	if( parachute->isDisabledByType( DISABLED_HELD ) )
	{
		return UPDATE_SLEEP_NONE; // my, that was easy
	}

	AIUpdateInterface *parachuteAI = parachute->getAI();

	Drawable* draw = parachute->getDrawable();
	const ParachuteContainModuleData* d = getParachuteContainModuleData();
	Object* rider = (getContainCount() > 0) ? getContainList().front() : NULL;

	if (m_startZ == NO_START_Z)	{
		m_startZ = parachute->getPosition()->z;
		Real groundHeight = TheTerrainLogic->getGroundHeight(parachute->getPosition()->x, parachute->getPosition()->y);
		if (m_startZ-groundHeight < 2*d->m_paraOpenDist) {
			// Oh dear - we ejected too close to the ground, and there isn't enough
			// room to open the chute.  Well, since it's only a game, we'll fudge
			// a little so that the pilot doesn't slam into the ground & stick.
			m_startZ = groundHeight+2*d->m_paraOpenDist;
		}
	}

	if (!m_opened)
	{
		// see if we need to open.
		if (fabs(m_startZ - parachute->getPosition()->z) >= d->m_paraOpenDist)
		{
			m_opened = true;
			parachute->clearAndSetModelConditionState(MODELCONDITION_FREEFALL, MODELCONDITION_PARACHUTING);
			m_needToUpdateParaBones = true;
			if (rider)
			{
				rider->clearAndSetModelConditionState(MODELCONDITION_FREEFALL, MODELCONDITION_PARACHUTING);
				m_needToUpdateRiderBones = true;

				AudioEventRTS soundToPlay = d->m_parachuteOpenSound;
				soundToPlay.setObjectID( rider->getID() );
				TheAudio->addAudioEvent( &soundToPlay );
			}


			// When a parachute opens, it should look for a good place to land.  This could be explicitly set
			// by a DeliverPayload, otherwise any place clear is good.
			if( parachuteAI )
			{
				Coord3D target = *parachute->getPosition();
				if( m_isLandingOverrideSet )
				{
					target = m_landingOverride;
					if( parachuteAI->getCurLocomotor() )
						parachuteAI->getCurLocomotor()->setUltraAccurate( TRUE );
				}
				else
				{
					FindPositionOptions fpOptions;
					fpOptions.minRadius = 0.0f;
					fpOptions.maxRadius = 100.0f;
					fpOptions.relationshipObject = NULL;
					fpOptions.flags = FPF_NONE;
					ThePartitionManager->findPositionAround( &target, &fpOptions, &target );
				}
				parachuteAI->aiMoveToPosition( &target, CMD_FROM_AI );
			}
		}
    else if ( rider )
			rider->clearAndSetModelConditionState( MODELCONDITION_PARACHUTING, MODELCONDITION_FREEFALL );

  }
	draw->setDrawableHidden(!m_opened);

	if (!m_opened || getContainCount() == 0)
	{
		// unopened, or empty, chutes, don't collide with anything, to simplify
		// ejections, paradrops, landings, etc...
		parachute->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_NO_COLLISIONS ) );
		if (rider)
			rider->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_NO_COLLISIONS ) );
	}
	else
	{
		// opened/nonempty chutes DO collide...
		parachute->clearStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_NO_COLLISIONS ) );
		if (rider)
			rider->clearStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_NO_COLLISIONS ) );
	}

	AIUpdateInterface* ai = parachute->getAIUpdateInterface();
	if (ai && !parachute->isEffectivelyDead())
	{
		ai->chooseLocomotorSet(m_opened ? LOCOMOTORSET_NORMAL : LOCOMOTORSET_FREEFALL);

		Locomotor* locomotor = ai->getCurLocomotor();
		if (locomotor)
		{
			// damp the swaying a bunch when we get close, so that things land vertically (or nearly so)
			Real altitudeDamping = 0;
			if (getContainCount() > 0)
			{
				Object* rider = getContainList().front();
				const Real ALTITUDE_DAMP_START = 20.0f;
				if (rider->getHeightAboveTerrain() <= ALTITUDE_DAMP_START)
					altitudeDamping = d->m_lowAltitudeDamping;
			}

			if (m_opened)
			{
				const Real PITCH_STIFFNESS = locomotor->getPitchStiffness();
				const Real ROLL_STIFFNESS =  locomotor->getRollStiffness();
				const Real PITCH_DAMPING = locomotor->getPitchDamping() + altitudeDamping;
				const Real ROLL_DAMPING = locomotor->getRollDamping() + altitudeDamping;
				m_pitchRate += ((-PITCH_STIFFNESS * m_pitch) + (-PITCH_DAMPING * m_pitchRate));		// spring/damper
				m_rollRate += ((-ROLL_STIFFNESS * m_roll) + (-ROLL_DAMPING * m_rollRate));		// spring/damper

				m_pitch += m_pitchRate;
				m_roll += m_rollRate;

				if( m_isLandingOverrideSet )
				{
					// Need to wait until after opening to do this, or else we are sending the message to the Freefall locomotor.
					locomotor->setCloseEnoughDist( 10.0 );
					locomotor->setCloseEnoughDist3D( FALSE );
				}
			}

			if (draw)
			{
				updateBonePositions();
				updateOffsetsFromBones();

				Matrix3D tmp;
				calcSwayMtx(&m_paraSwayOffset, &tmp);
				draw->setInstanceMatrix(&tmp);
			}

			positionContainedObjectsRelativeToContainer();
		}
	}

	// allow us to land on bridges!
	const Coord3D* paraPos = getObject()->getPosition();
	PathfindLayerEnum newLayer = TheTerrainLogic->getHighestLayerForDestination(paraPos);
	getObject()->setLayer(newLayer);
	if (rider)
		rider->setLayer(newLayer);

	// If we have lost our passenger for whatever reason, die early.  Otherwise we just sit around forever.
	if( getContainCount() == 0 )
		getObject()->kill();

	// the collide system doesn't always collide us with the ground if we fall into water.
	// so force the issue.
	Real waterZ;
	if (!getObject()->isEffectivelyDead()
			&& getObject()->getLayer() == LAYER_GROUND
			&& TheTerrainLogic->isUnderwater(paraPos->x, paraPos->y, &waterZ)
			&& (paraPos->z - waterZ) < d->m_killWhenLandingInWaterSlop)
	{
		getObject()->kill();
	}


	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ParachuteContain::onContaining( Object *rider, Bool wasSelected )
{
	OpenContain::onContaining( rider, wasSelected );

	// objects inside a transport are held
	rider->setDisabled( DISABLED_HELD );
	rider->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_PARACHUTING ) );

	rider->clearAndSetModelConditionState(MODELCONDITION_PARACHUTING, MODELCONDITION_FREEFALL);
	m_needToUpdateRiderBones = true;

	// position him correctly.
	positionRider(rider);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ParachuteContain::onRemoving( Object *rider )
{
	OpenContain::onRemoving(rider);

	const ParachuteContainModuleData* d = getParachuteContainModuleData();

	// object is no longer held inside a transport
	rider->clearDisabled( DISABLED_HELD );
	rider->clearStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_PARACHUTING ) );

	// mark parachute as "no-collisions"... it is just ephemeral at this point,
	// and having the chute collide with the soldier (and both bounce apart) is
	// just dumb-lookin'...
	getObject()->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_NO_COLLISIONS ) );

	// position him correctly.
	positionRider(rider);

	rider->clearModelConditionFlags(MAKE_MODELCONDITION_MASK2(MODELCONDITION_FREEFALL, MODELCONDITION_PARACHUTING));
	m_needToUpdateRiderBones = true;

	// temporarily mark the guy as being allowed to fall
	// (overriding his locomotor's stick-to-ground attribute).
	// this will be reset (by PhysicsBehavior) when he touches the ground.
	PhysicsBehavior* physics = rider->getPhysics();
	if (physics)
	{
		physics->setAllowToFall(true);

		Coord3D force;
		force.zero();
		physics->applyForce(&force);	// force its physics to wake up... should be done when DISABLED_HELD is cleared, but it not, and scared to do it now.
	}


	AIUpdateInterface* riderAI = rider->getAIUpdateInterface();
	if (riderAI)
	{
		Player* controller = rider->getControllingPlayer();
		if (controller && controller->isSkirmishAIPlayer())
		{
			riderAI->aiHunt(CMD_FROM_AI);	// hunt, as per Dustin's request.
		}
		else
		{
			bool hasRallyPoint = false;
			// Get the transport of the rider
			Object *transport = TheGameLogic->findObjectByID(rider->getProducerID());
			if (transport)
			{
				// Get the building that produced the transport
				Object *transportProducer = TheGameLogic->findObjectByID(transport->getProducerID());
				if (transportProducer)
				{
					// See if we need to set a rally point for the object being parachuted
					ExitInterface* exitInterface = transportProducer->getObjectExitInterface();
					if (exitInterface && exitInterface->useSpawnRallyPoint())
					{
						exitInterface->exitObjectViaDoor(rider, DOOR_1);
						hasRallyPoint = true;
					}
				}
			}

			if (!hasRallyPoint)
				riderAI->aiIdle(CMD_FROM_AI); // become idle.
		}
	}

	// if we land in the water, we die. alas.
	const Coord3D* riderPos = rider->getPosition();
	Real waterZ, terrainZ;
	if (TheTerrainLogic->isUnderwater(riderPos->x, riderPos->y, &waterZ, &terrainZ)
			&& riderPos->z <= waterZ + d->m_killWhenLandingInWaterSlop
			&& rider->getLayer() == LAYER_GROUND)
	{
		// don't call kill(); do it manually, so we can specify DEATH_FLOODED
		DamageInfo damageInfo;
		damageInfo.in.m_damageType = DAMAGE_WATER;	// use this instead of UNRESISTABLE so we don't get a dusty damage effect
		damageInfo.in.m_deathType = DEATH_FLOODED;
		damageInfo.in.m_sourceID = INVALID_ID;
		damageInfo.in.m_amount = HUGE_DAMAGE_AMOUNT;
		rider->attemptDamage( &damageInfo );
	}

	// Kill if we landed on impassable ground
	Int cellX = REAL_TO_INT( rider->getPosition()->x / PATHFIND_CELL_SIZE );
	Int cellY = REAL_TO_INT( rider->getPosition()->y / PATHFIND_CELL_SIZE );

	PathfindCell* cell = TheAI->pathfinder()->getCell( rider->getLayer(), cellX, cellY );
	PathfindCell::CellType cellType = cell ? cell->getType() : PathfindCell::CELL_IMPASSABLE;

	// If we land outside the map from a faulty parachute, we die too.
	// Otherwise we exist outside the PartitionManger like a cheater.
	if( rider->isOffMap()
    || (cellType == PathfindCell::CELL_CLIFF)
    || (cellType == PathfindCell::CELL_WATER)
    || (cellType == PathfindCell::CELL_IMPASSABLE) )
	{
		// The Paradrop command was legal, the parachute destination was legal, but the parachute
		// can still fail to adjust back on the map.  SO this is the place to cap the cheater.
		rider->kill();
	}

  // Note: for future enhancement of this feature, we should test the object against the cell type he is on,
  // using obj->getAI()->hasLocomotorForSurface( __ ). We cshould not assume here that the parachutist can not
  // find happiness on cliffs or water or whatever.


}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::positionRider(Object* rider)
{
	updateBonePositions();
	updateOffsetsFromBones();

	Coord3D pos = *getObject()->getPosition();
	///DUMPCOORD3D(&pos);
	pos.x += m_riderAttachOffset.x;
	pos.y += m_riderAttachOffset.y;
	pos.z += m_riderAttachOffset.z;
	//DUMPCOORD3D(&pos);
	rider->setPosition(&pos);

	Real alt = rider->getHeightAboveTerrain();
	if (alt < 0.0f)
	{
		// don't let him go below ground.
		pos.z -= alt;
		rider->setPosition(&pos);
	}

	rider->setOrientation(getObject()->getOrientation());

	Drawable* draw = rider->getDrawable();
	if (draw)
	{
		if( rider->isDisabledByType( DISABLED_HELD ) )
		{
			Matrix3D tmp;
			calcSwayMtx(&m_riderSwayOffset, &tmp);
			draw->setInstanceMatrix(&tmp);
		}
		else
		{
			draw->setInstanceMatrix(NULL);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::positionContainedObjectsRelativeToContainer()
{
	for(ContainedItemsList::const_iterator it = getContainList().begin(); it != getContainList().end(); ++it)
	{
		positionRider(*it);
	}
}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::setOverrideDestination( const Coord3D *override )
{
	// Instead of trying to float straight down, I am going to nail this spot.
	m_landingOverride = *override;
	m_isLandingOverrideSet = TRUE;
}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::onDie( const DamageInfo * damageInfo )
{
	// if we are airborne when killed, the guy falls screaming to his death...
	if (getObject()->isSignificantlyAboveTerrain())
	{
		Object* rider = (getContainCount() > 0) ? getContainList().front() : NULL;
		if (rider)
		{
			removeAllContained();
			const ParachuteContainModuleData* d = getParachuteContainModuleData();
			if (d->m_freeFallDamagePercent > 0.0f)
			{
				// do some damage just for losing your parachute.
				// not very realistic, but practical to help ensure that
				// you really do die from going "splat" on the ground.
				DamageInfo extraDamageInfo;
				extraDamageInfo.in.m_damageType = DAMAGE_FALLING;
				extraDamageInfo.in.m_deathType = DEATH_SPLATTED;
				extraDamageInfo.in.m_sourceID = damageInfo->in.m_sourceID;
				extraDamageInfo.in.m_amount = rider->getBodyModule()->getMaxHealth() * d->m_freeFallDamagePercent;
				rider->attemptDamage(&extraDamageInfo);
			}
			PhysicsBehavior* physics = rider->getPhysics();
			if (physics)
			{
				physics->setAllowToFall(true);
				physics->setIsInFreeFall(true);	// bwah ha ha

				Coord3D force;
				force.zero();
				physics->applyForce(&force);	// force its physics to wake up... should be done when DISABLED_HELD is cleared, but it not, and scared to do it now.
			}
		}
	}


	OpenContain::onDie(damageInfo);
}

//-------------------------------------------------------------------------------------------------
void ParachuteContain::onCollide( Object *other, const Coord3D *loc, const Coord3D *normal )
{
	// Note that other == null means "collide with ground"
	if( other == NULL )
	{
		// if we're in a container (eg, a transport plane), just ignore this...
		if( getObject()->getContainedBy() != NULL )
			return;

		removeAllContained();

		// TheGameLogic->destroyObject(obj);
		// kill it, so that the chute's SlowDeath will trigger!
		getObject()->kill();
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ParachuteContain::crc( Xfer *xfer )
{

	// extend base class
	OpenContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ParachuteContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	OpenContain::xfer( xfer );

	// pitch
	xfer->xferReal( &m_pitch );

	// roll
	xfer->xferReal( &m_roll );

	// pitch rage
	xfer->xferReal( &m_pitchRate );

	// roll rate
	xfer->xferReal( &m_rollRate );

	// start Z
	xfer->xferReal( &m_startZ );

	// is landing override set
	xfer->xferBool( &m_isLandingOverrideSet );

	// landing override
	xfer->xferCoord3D( &m_landingOverride );

	// rider attach bone
	xfer->xferCoord3D( &m_riderAttachBone );

	// rider sway bone
	xfer->xferCoord3D( &m_riderSwayBone );

	// para attach bone
	xfer->xferCoord3D( &m_paraAttachBone );

	// para sway bone
	xfer->xferCoord3D( &m_paraSwayBone );

	// rider attach offset
	xfer->xferCoord3D( &m_riderAttachOffset );

	// rider sway offset
	xfer->xferCoord3D( &m_riderSwayOffset );

	// para attach offset
	xfer->xferCoord3D( &m_paraAttachOffset );

	// para sway offset
	xfer->xferCoord3D( &m_paraSwayOffset );

	// need to update rider bones
	xfer->xferBool( &m_needToUpdateRiderBones );

	// need to update para bones
	xfer->xferBool( &m_needToUpdateParaBones );

	// opened
	xfer->xferBool( &m_opened );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ParachuteContain::loadPostProcess( void )
{

	// extend base class
	OpenContain::loadPostProcess();

}
