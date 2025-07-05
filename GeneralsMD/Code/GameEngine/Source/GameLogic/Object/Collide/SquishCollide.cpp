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

// FILE: SquishCollide.cpp //////////////////////////////////////////////////////////////////////////
// Author:
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/SquishCollide.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/HijackerUpdate.h"
#include "GameLogic/Module/SpecialAbilityUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/PartitionManager.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SquishCollide::SquishCollide( Thing *thing, const ModuleData* moduleData ) : CollideModule( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SquishCollide::~SquishCollide( void )
{

}

//-------------------------------------------------------------------------------------------------
/** Do the collision */
//-------------------------------------------------------------------------------------------------
void SquishCollide::onCollide( Object *other, const Coord3D *loc, const Coord3D *normal )
{
	// Note that other == null means "collide with ground"
	if (other == NULL)
		return;
	
	Object *self = getObject();
	AIUpdateInterface *ai = self->getAI();
	if( ai && ai->getGoalObject() == other )
	{
		//We are actually targeting the other object to do something to! Don't allow them to crush us in the following
		//special circumstances:

		//Hijacking!
		static NameKeyType key_HijackerUpdate = NAMEKEY( "HijackerUpdate" );
		HijackerUpdate *hijackUpdate = (HijackerUpdate*)self->findUpdateModule( key_HijackerUpdate );
		if( hijackUpdate )
		{
			return;
		}

		//TNT hunter placing a charge!
		SpecialAbilityUpdate *saUpdate = self->findSpecialAbilityUpdate( SPECIAL_TANKHUNTER_TNT_ATTACK );
		if( saUpdate && saUpdate->isActive() )
		{
			return;
		}
	}

	// order matters: we want to know if IT considers ME to be an ally (a reversal of the usual situation)
	if( other->getCrusherLevel() > 0 && other->getRelationship(getObject()) != ALLIES)
	{
		PhysicsBehavior *otherPhysics = other->getPhysics();
		if (otherPhysics == NULL)
			return;

		// use a 1.0 crush radius so the tank has to actually hit the infantry.
 		GeometryInfo myGeom = self->getGeometryInfo();
		myGeom.setMajorRadius(1.0f);
		myGeom.setMinorRadius(1.0f);
		if (!ThePartitionManager->geomCollidesWithGeom(other->getPosition(), other->getGeometryInfo(), other->getOrientation(), 
			self->getPosition(), myGeom, self->getOrientation())) {
			return;
		}

		// only squish if tank is moving toward victim
		const Coord3D *vel = otherPhysics->getVelocity();
		const Coord3D *pos = other->getPosition();
		const Coord3D *myPos = getObject()->getPosition();
		Coord3D to;

		to.x = myPos->x - pos->x;
		to.y = myPos->y - pos->y;
		to.z = myPos->z - pos->z;

		if (to.x * vel->x + to.y * vel->y > 0.0f)
		{
			DamageInfo damageInfo;
			damageInfo.in.m_damageType = DAMAGE_CRUSH;
			damageInfo.in.m_deathType = DEATH_CRUSHED;
			damageInfo.in.m_sourceID = other->getID();
			damageInfo.in.m_amount = HUGE_DAMAGE_AMOUNT;			// make sure they die
			getObject()->attemptDamage( &damageInfo );
			getObject()->friend_setUndetectedDefector( FALSE );// My secret is out
		}


  }
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SquishCollide::crc( Xfer *xfer )
{

	// extend base class
	CollideModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SquishCollide::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CollideModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SquishCollide::loadPostProcess( void )
{

	// extend base class
	CollideModule::loadPostProcess();

}  // end loadPostProcess
