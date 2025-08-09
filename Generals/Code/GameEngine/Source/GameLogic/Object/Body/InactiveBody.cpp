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

// FILE: InactiveBody.cpp /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:	 An Inactive body module doesn't have any of the data storage for
//				 health and damage etc ... it's an "Inactive" object that isn't
//				 affected by matters of the body ... it's all in the mind!!!!
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/InactiveBody.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
InactiveBody::InactiveBody( Thing *thing, const ModuleData* moduleData )
						: BodyModule( thing, moduleData ), m_dieCalled(false)
{
	getObject()->setEffectivelyDead(true);
}  // end InactiveBody

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
InactiveBody::~InactiveBody( void )
{

}  // end ~InactiveBody

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Real InactiveBody::estimateDamage( DamageInfoInput& damageInfo ) const
{
	// Inactive bodies have no health so no damage can really be done
	Real amount = 0.0f;

	// exception!
	if (damageInfo.m_damageType == DAMAGE_UNRESISTABLE)
	{
		amount = damageInfo.m_amount;
	}

	return amount;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void InactiveBody::attemptDamage( DamageInfo *damageInfo )
{
	if( damageInfo == NULL )
		return;

	if( damageInfo->in.m_damageType == DAMAGE_HEALING )
	{
		// Healing and Damage are separate, so this shouldn't happen
		attemptHealing( damageInfo );
		return;
	}

	// Inactive bodies have no health so no damage can really be done
	damageInfo->out.m_actualDamageDealt = 0.0f;
	damageInfo->out.m_actualDamageClipped = 0.0f;
	damageInfo->out.m_noEffect = true;

	// exception: damage type KILL always wipes us out
	if (damageInfo->in.m_damageType == DAMAGE_UNRESISTABLE)
	{

		DEBUG_ASSERTCRASH(!getObject()->getTemplate()->isPrerequisite(), ("Prerequisites should not have InactiveBody"));

		damageInfo->out.m_noEffect = false;

		// since we have no Health, we do not call DamageModules, nor do DamageFX.
		// however, we DO process DieModules.
		if (!m_dieCalled)
		{
			getObject()->onDie( damageInfo );
			m_dieCalled = true;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void InactiveBody::attemptHealing( DamageInfo *damageInfo )
{
	if( damageInfo == NULL )
		return;

	if( damageInfo->in.m_damageType != DAMAGE_HEALING )
	{
		// Healing and Damage are separate, so this shouldn't happen
		attemptDamage( damageInfo );
		return;
	}

	// Inactive bodies have no health so no damage can really be done
	damageInfo->out.m_actualDamageDealt = 0.0f;
	damageInfo->out.m_actualDamageClipped = 0.0f;
	damageInfo->out.m_noEffect = true;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void InactiveBody::internalChangeHealth( Real delta )
{

	// Inactive bodies have no health to increase or decrease

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Real InactiveBody::getHealth() const
{

	// Inactive bodies have no health to get
	return 0.0f;

}  // end getHealth

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BodyDamageType InactiveBody::getDamageState() const
{
	return BODY_PRISTINE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void InactiveBody::setDamageState( BodyDamageType  )	///< control damage state directly.  Will adjust hitpoints.
{

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void InactiveBody::crc( Xfer *xfer )
{

	// extend base class
	BodyModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void InactiveBody::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// base class
	BodyModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void InactiveBody::loadPostProcess( void )
{

	// extend base class
	BodyModule::loadPostProcess();

}  // end loadPostProcess


