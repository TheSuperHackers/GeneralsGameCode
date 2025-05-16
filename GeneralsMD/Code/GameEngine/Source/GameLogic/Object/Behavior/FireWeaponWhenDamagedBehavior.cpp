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

// FILE: FireWeaponWhenDamagedBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:  
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#define DEFINE_SLOWDEATHPHASE_NAMES

#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/FireWeaponWhenDamagedBehavior.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Weapon.h"
#include "GameClient/Drawable.h"

const Int MAX_IDX = 32;

const Real BEGIN_MIDPOINT_RATIO = 0.35f;
const Real END_MIDPOINT_RATIO = 0.65f;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponWhenDamagedBehavior::FireWeaponWhenDamagedBehavior( Thing *thing, const ModuleData* moduleData ) : 
	UpdateModule( thing, moduleData ),
	m_reactionWeaponPristine( NULL ),
	m_reactionWeaponDamaged( NULL ),
	m_reactionWeaponReallyDamaged( NULL ),
	m_reactionWeaponRubble( NULL ),
	m_continuousWeaponPristine( NULL ),
	m_continuousWeaponDamaged( NULL ),
	m_continuousWeaponReallyDamaged( NULL ),
	m_continuousWeaponRubble( NULL )
{

	const FireWeaponWhenDamagedBehaviorModuleData *d = getFireWeaponWhenDamagedBehaviorModuleData();
	const Object* obj = getObject();

	if ( d->m_reactionWeaponPristine )
	{
		m_reactionWeaponPristine				= TheWeaponStore->allocateNewWeapon(
			d->m_reactionWeaponPristine,					PRIMARY_WEAPON);
		m_reactionWeaponPristine->reloadAmmo( obj );
	}
	if ( d->m_reactionWeaponDamaged )
	{
		m_reactionWeaponDamaged					= TheWeaponStore->allocateNewWeapon(
			d->m_reactionWeaponDamaged,					PRIMARY_WEAPON);
		m_reactionWeaponDamaged->reloadAmmo( obj );
	}
	if ( d->m_reactionWeaponReallyDamaged )
	{
		m_reactionWeaponReallyDamaged		= TheWeaponStore->allocateNewWeapon(
			d->m_reactionWeaponReallyDamaged,		PRIMARY_WEAPON); 
		m_reactionWeaponReallyDamaged->reloadAmmo( obj );
	}
	if ( d->m_reactionWeaponRubble )
	{
		m_reactionWeaponRubble					= TheWeaponStore->allocateNewWeapon(
			d->m_reactionWeaponRubble,						PRIMARY_WEAPON);
		m_reactionWeaponRubble->reloadAmmo( obj );
	}


	if ( d->m_continuousWeaponPristine )
	{
		m_continuousWeaponPristine			= TheWeaponStore->allocateNewWeapon(
			d->m_continuousWeaponPristine,				PRIMARY_WEAPON);
		m_continuousWeaponPristine->reloadAmmo( obj );
	}
	if ( d->m_continuousWeaponDamaged )
	{
		m_continuousWeaponDamaged				= TheWeaponStore->allocateNewWeapon(
			d->m_continuousWeaponDamaged,				PRIMARY_WEAPON);
		m_continuousWeaponDamaged->reloadAmmo( obj );
	}
	if ( d->m_continuousWeaponReallyDamaged )
	{
		m_continuousWeaponReallyDamaged = TheWeaponStore->allocateNewWeapon(
			d->m_continuousWeaponReallyDamaged,	PRIMARY_WEAPON);
		m_continuousWeaponReallyDamaged->reloadAmmo( obj );
	}
	if ( d->m_continuousWeaponRubble )
	{
		m_continuousWeaponRubble				= TheWeaponStore->allocateNewWeapon(
			d->m_continuousWeaponRubble,					PRIMARY_WEAPON);
		m_continuousWeaponRubble->reloadAmmo( obj );
	}

	if (d->m_initiallyActive)
	{
		giveSelfUpgrade();
	}

	if (isUpgradeActive() &&
			(d->m_continuousWeaponPristine != NULL ||
			d->m_continuousWeaponDamaged != NULL ||
			d->m_continuousWeaponReallyDamaged != NULL ||
			d->m_continuousWeaponRubble != NULL))
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}
	else
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponWhenDamagedBehavior::~FireWeaponWhenDamagedBehavior( void )
{
	if (m_reactionWeaponPristine)
		deleteInstance(m_reactionWeaponPristine);
	if (m_reactionWeaponDamaged)
		deleteInstance(m_reactionWeaponDamaged);
	if (m_reactionWeaponReallyDamaged)
		deleteInstance(m_reactionWeaponReallyDamaged);
	if (m_reactionWeaponRubble)
		deleteInstance(m_reactionWeaponRubble);

	if (m_continuousWeaponPristine)
		deleteInstance(m_continuousWeaponPristine);
	if (m_continuousWeaponDamaged)
		deleteInstance(m_continuousWeaponDamaged);
	if (m_continuousWeaponReallyDamaged)
		deleteInstance(m_continuousWeaponReallyDamaged);
	if (m_continuousWeaponRubble)
		deleteInstance(m_continuousWeaponRubble);
}

//-------------------------------------------------------------------------------------------------
/** Damage has been dealt, this is an opportunity to reach to that damage */
//-------------------------------------------------------------------------------------------------
void FireWeaponWhenDamagedBehavior::onDamage( DamageInfo *damageInfo )
{
	if (!isUpgradeActive())
		return;

	const FireWeaponWhenDamagedBehaviorModuleData* d = getFireWeaponWhenDamagedBehaviorModuleData();

	// right type?
	if (!getDamageTypeFlag(d->m_damageTypes, damageInfo->in.m_damageType))
		return;

	// right amount? (use actual [post-armor] damage dealt)
	if (damageInfo->out.m_actualDamageDealt < d->m_damageAmount)
		return;

	const Object *obj = getObject();
	BodyDamageType bdt = obj->getBodyModule()->getDamageState();

	if ( bdt == BODY_RUBBLE )
	{
		if( m_reactionWeaponRubble && m_reactionWeaponRubble->getStatus() == READY_TO_FIRE )
		{
			m_reactionWeaponRubble->forceFireWeapon( obj, obj->getPosition() );
		}

	}
	else if ( bdt == BODY_REALLYDAMAGED )
	{
		if( m_reactionWeaponReallyDamaged && m_reactionWeaponReallyDamaged->getStatus() == READY_TO_FIRE )
		{
			m_reactionWeaponReallyDamaged->forceFireWeapon( obj, obj->getPosition() );
		}
	}
	else if ( bdt == BODY_DAMAGED )
	{
		if( m_reactionWeaponDamaged && m_reactionWeaponDamaged->getStatus() == READY_TO_FIRE )
		{
			m_reactionWeaponDamaged->forceFireWeapon( obj, obj->getPosition() );
		}
	}
	else // not damaged yet
	{
		if( m_reactionWeaponPristine && m_reactionWeaponPristine->getStatus() == READY_TO_FIRE )
		{
			m_reactionWeaponPristine->forceFireWeapon( obj, obj->getPosition() );
		}
	}

}

//-------------------------------------------------------------------------------------------------
/** if object fires weapon constantly, figure out which one and do it */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime FireWeaponWhenDamagedBehavior::update( void )
{
	if (!isUpgradeActive())
	{
		DEBUG_ASSERTCRASH(isUpgradeActive(), ("hmm, this should not be possible"));
		return UPDATE_SLEEP_FOREVER;
	}

	const Object *obj = getObject();
	BodyDamageType bdt = obj->getBodyModule()->getDamageState();

	if ( bdt == BODY_RUBBLE )
	{
		if( m_continuousWeaponRubble && m_continuousWeaponRubble->getStatus() == READY_TO_FIRE )
		{
			m_continuousWeaponRubble->forceFireWeapon( obj, obj->getPosition() );
		}

	}
	else if ( bdt == BODY_REALLYDAMAGED )
	{
		if( m_continuousWeaponReallyDamaged && m_continuousWeaponReallyDamaged->getStatus() == READY_TO_FIRE )
		{
			m_continuousWeaponReallyDamaged->forceFireWeapon( obj, obj->getPosition() );
		}
	}
	else if ( bdt == BODY_DAMAGED )
	{
		if( m_continuousWeaponDamaged && m_continuousWeaponDamaged->getStatus() == READY_TO_FIRE )
		{
			m_continuousWeaponDamaged->forceFireWeapon( obj, obj->getPosition() );
		}
	}
	else // not damaged yet
	{
		if( m_continuousWeaponPristine && m_continuousWeaponPristine->getStatus() == READY_TO_FIRE )
		{
			m_continuousWeaponPristine->forceFireWeapon( obj, obj->getPosition() );
		}
	}

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FireWeaponWhenDamagedBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

	// extend upgrade mux
	UpgradeMux::upgradeMuxCRC( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FireWeaponWhenDamagedBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// extend upgrade mux
	UpgradeMux::upgradeMuxXfer( xfer );

	Bool weaponPresent;

	// reaction pristine
	weaponPresent = m_reactionWeaponPristine ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_reactionWeaponPristine );

	// reaction damaged
	weaponPresent = m_reactionWeaponDamaged ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_reactionWeaponDamaged );

	// reaction really damaged
	weaponPresent = m_reactionWeaponReallyDamaged ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_reactionWeaponReallyDamaged );

	// reaction rubble
	weaponPresent = m_reactionWeaponRubble ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_reactionWeaponRubble );

	// continuous pristine
	weaponPresent = m_continuousWeaponPristine ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_continuousWeaponPristine );

	// continuous damaged
	weaponPresent = m_continuousWeaponDamaged ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_continuousWeaponDamaged );

	// continuous really damaged
	weaponPresent = m_continuousWeaponReallyDamaged ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_continuousWeaponReallyDamaged );

	// continuous rubble
	weaponPresent = m_continuousWeaponRubble ? TRUE : FALSE;
	xfer->xferBool( &weaponPresent );
	if( weaponPresent )
		xfer->xferSnapshot( m_continuousWeaponRubble );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FireWeaponWhenDamagedBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

	// extend upgrade mux
	UpgradeMux::upgradeMuxLoadPostProcess();

}  // end loadPostProcess
