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

// FILE: AdvancedCollide.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Andi W, Oct 25
// Desc:   Do something if we collide with objects or the ground
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#define DEFINE_OBJECT_STATUS_NAMES
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameClient/FXList.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Module/AdvancedCollide.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void AdvancedCollideModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  CollideModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "CollideWeapon",		INI::parseWeaponTemplate,				NULL, offsetof( AdvancedCollideModuleData, m_collideWeaponTemplate ) },
		{ "OCL",	            INI::parseObjectCreationList,		NULL,	offsetof(AdvancedCollideModuleData, m_ocl) },
		{ "FX",							  INI::parseFXList,		            NULL, offsetof(AdvancedCollideModuleData, m_fxlist) },

		{ "FireOnce",					INI::parseBool,											NULL, offsetof( AdvancedCollideModuleData, m_fireOnce ) },
		{ "CollideWithGround",					INI::parseBool,											NULL, offsetof( AdvancedCollideModuleData, m_collideWithGround) },
		{ "CollideWithObjects",					INI::parseBool,											NULL, offsetof( AdvancedCollideModuleData, m_collideWithObjects) },

		{ "RequiredStatus",		ObjectStatusMaskType::parseFromINI,	NULL, offsetof( AdvancedCollideModuleData, m_requiredStatus ) },
		{ "ForbiddenStatus",	ObjectStatusMaskType::parseFromINI,	NULL, offsetof( AdvancedCollideModuleData, m_forbiddenStatus ) },
		{ "TargetRequiredStatus",		ObjectStatusMaskType::parseFromINI,	NULL, offsetof(AdvancedCollideModuleData, m_targetRequiredStatus) },
		{ "TargetForbiddenStatus",	ObjectStatusMaskType::parseFromINI,	NULL, offsetof(AdvancedCollideModuleData, m_targetForbiddenStatus) },
		{ "RequiredKindOf",   KindOfMaskType::parseFromINI, NULL, offsetof(AdvancedCollideModuleData, m_kindof) },
		{ "ForbiddenKindOf",  KindOfMaskType::parseFromINI, NULL, offsetof(AdvancedCollideModuleData, m_kindofnot) },

		{ "ChanceToTriggerPercent",  INI::parsePercentToReal, NULL, offsetof(AdvancedCollideModuleData, m_triggerChance) },
		{ "RollOnceForTrigger",  INI::parseBool, NULL, offsetof(AdvancedCollideModuleData, m_rollOnce) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AdvancedCollide::AdvancedCollide( Thing *thing, const ModuleData* moduleData ) :
	CollideModule( thing, moduleData ),
	m_collideWeapon(NULL)
{
	const AdvancedCollideModuleData* d = getAdvancedCollideModuleData();
	if (d->m_collideWeaponTemplate != NULL) {
		m_collideWeapon = TheWeaponStore->allocateNewWeapon(getAdvancedCollideModuleData()->m_collideWeaponTemplate, PRIMARY_WEAPON);
	}
	m_everFired = FALSE;
	m_roll = -1;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AdvancedCollide::~AdvancedCollide( void )
{
	if (m_collideWeapon)
		deleteInstance(m_collideWeapon);
}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void AdvancedCollide::onCollide( Object *other, const Coord3D *loc, const Coord3D *normal )
{
	const AdvancedCollideModuleData* d = getAdvancedCollideModuleData();

	if( other == NULL && !d->m_collideWithGround)
		return;
	if (other != NULL && !d->m_collideWithObjects)
		return;

	if(shouldTrigger(other))
	{
		Object* me = getObject();

		if (m_collideWeapon) {
			m_collideWeapon->loadAmmoNow(me);
			m_collideWeapon->fireWeapon(me, other);
		}

		FXList::doFXObj(d->m_fxlist, me, other);

		ObjectCreationList::create(d->m_ocl, me, other);

	}
}

//-------------------------------------------------------------------------------------------------
Bool AdvancedCollide::shouldTrigger(Object* other)
{
	const AdvancedCollideModuleData *d = getAdvancedCollideModuleData();

	if (m_everFired && d->m_fireOnce)
		return FALSE; // can only trigger once ever

	// Check Source Object
	ObjectStatusMaskType status = getObject()->getStatusBits();

	//We need all required status or else we fail
	if( !status.testForAll( d->m_requiredStatus ) )
		return FALSE;

	//If we have any forbidden statii, then fail
	if( status.testForAny( d->m_forbiddenStatus ) )
		return FALSE;

	// -----------------------
	// Check Target Object
	if (other) {
		status = other->getStatusBits();

		//We need all required status or else we fail
		if (!status.testForAll(d->m_targetRequiredStatus))
			return FALSE;

		//If we have any forbidden statii, then fail
		if (status.testForAny(d->m_targetForbiddenStatus))
			return FALSE;

		// must match our kindof flags (if any)
		if (!other->isKindOfMulti(d->m_kindof, d->m_kindofnot))
			return FALSE;
	}

	if (d->m_triggerChance < 1.0) {
		if (!d->m_rollOnce || m_roll < 0) {
			m_roll = GameLogicRandomValueReal(0, 1);
		}

		if (m_roll > d->m_triggerChance)
			return FALSE;
	}

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AdvancedCollide::crc( Xfer *xfer )
{

	// extend base class
	CollideModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void AdvancedCollide::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CollideModule::xfer( xfer );

	// weapon
	Bool collideWeaponPresent = m_collideWeapon ? TRUE : FALSE;
	xfer->xferBool( &collideWeaponPresent );
	if( collideWeaponPresent )
	{

		DEBUG_ASSERTCRASH( m_collideWeapon != NULL,
											 ("AdvancedCollide::xfer - m_collideWeapon present mismatch") );
		xfer->xferSnapshot( m_collideWeapon );

	}  // end else
	else
	{

		DEBUG_ASSERTCRASH( m_collideWeapon == NULL,
											 ("AdvancedCollide::Xfer - m_collideWeapon missing mismatch" ));

	}  // end else

	// ever fired
	xfer->xferBool( &m_everFired );

	// trigger chance roll
	xfer->xferReal(&m_roll);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AdvancedCollide::loadPostProcess( void )
{

	// extend base class
	CollideModule::loadPostProcess();

}  // end loadPostProcess
