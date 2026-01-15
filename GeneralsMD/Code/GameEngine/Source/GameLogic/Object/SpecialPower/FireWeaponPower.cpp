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

// FILE: FireWeaponPower.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	Created:	August 2003
//
//	Filename: FireWeaponPower.cpp
//
//	Author:		Kris Morness
//
//  Purpose:	Simply loads and fires a specific weapon controlled by a superweapon timer.
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_WEAPONSLOTTYPE_NAMES //For access to TheWeaponSlotTypeNames

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/WeaponSet.h"

#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/FireWeaponPower.h"


FireWeaponPowerModuleData::FireWeaponPowerModuleData( void )
{
	m_maxShotsToFire = 1;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void FireWeaponPowerModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	SpecialPowerModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "MaxShotsToFire", INI::parseUnsignedInt, nullptr, offsetof( FireWeaponPowerModuleData, m_maxShotsToFire ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add(dataFieldParse);

}


// ------------------------------------------------------------------------------------------------
FireWeaponPower::FireWeaponPower( Thing *thing, const ModuleData *moduleData )
												: SpecialPowerModule( thing, moduleData )
{

}

// ------------------------------------------------------------------------------------------------
FireWeaponPower::~FireWeaponPower( void )
{

}

// ------------------------------------------------------------------------------------------------
void FireWeaponPower::doSpecialPower( UnsignedInt commandOptions )
{
	Object *self = getObject();
	const FireWeaponPowerModuleData *data = getFireWeaponPowerModuleData();

	if( self->isDisabled() )
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPower( commandOptions );

	self->reloadAllAmmo( TRUE );

	AIUpdateInterface *ai = self->getAI();
	if( ai )
	{
		// TheSuperHackers @bugfix Caball009 09/08/2025 Position should be irrelevant, but aiAttackPosition requires a valid position pointer to avoid a crash.
		ai->aiAttackPosition( self->getPosition(), data->m_maxShotsToFire, CMD_FROM_AI );

		//Order any turrets to attack as well.
		for( Int i = 0; i < MAX_TURRETS; i++ )
		{
			ai->setTurretTargetPosition( (WhichTurretType)i, self->getPosition() );
		}
	}
}

// ------------------------------------------------------------------------------------------------
void FireWeaponPower::doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions )
{
	Object *self = getObject();
	const FireWeaponPowerModuleData *data = getFireWeaponPowerModuleData();

	if( self->isDisabled() )
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPowerAtLocation( loc, angle, commandOptions );

	self->reloadAllAmmo( TRUE );

	AIUpdateInterface *ai = self->getAI();
	if( ai )
	{
		ai->aiAttackPosition( loc, data->m_maxShotsToFire, CMD_FROM_AI );

		//Order any turrets to attack as well.
		for( Int i = 0; i < MAX_TURRETS; i++ )
		{
			ai->setTurretTargetPosition( (WhichTurretType)i, loc );
		}
	}

}

// ------------------------------------------------------------------------------------------------
void FireWeaponPower::doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions )
{
	Object *self = getObject();
	const FireWeaponPowerModuleData *data = getFireWeaponPowerModuleData();

	if( self->isDisabled() )
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPowerAtObject( obj, commandOptions );

	self->reloadAllAmmo( TRUE );

	AIUpdateInterface *ai = self->getAI();
	if( ai )
	{
		ai->aiAttackObject( obj, data->m_maxShotsToFire, CMD_FROM_AI );

		//Order any turrets to attack as well.
		for( Int i = 0; i < MAX_TURRETS; i++ )
		{
			ai->setTurretTargetObject( (WhichTurretType)i, obj );
		}
	}

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FireWeaponPower::crc( Xfer *xfer )
{

	// extend base class
	SpecialPowerModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FireWeaponPower::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	SpecialPowerModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FireWeaponPower::loadPostProcess( void )
{

	// extend base class
	SpecialPowerModule::loadPostProcess();

}
