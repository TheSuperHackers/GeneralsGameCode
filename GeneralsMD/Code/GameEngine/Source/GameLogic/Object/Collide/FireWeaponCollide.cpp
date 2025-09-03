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

// FILE: FireWeaponCollide.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood  April 2002
// Desc:   Shoot something that collides with me every frame with my weapon
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#define DEFINE_OBJECT_STATUS_NAMES
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/FireWeaponCollide.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void FireWeaponCollideModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  CollideModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "CollideWeapon",		INI::parseWeaponTemplate,						NULL, offsetof( FireWeaponCollideModuleData, m_collideWeaponTemplate ) },
		{ "FireOnce",					INI::parseBool,											NULL, offsetof( FireWeaponCollideModuleData, m_fireOnce ) },
		{ "RequiredStatus",		ObjectStatusMaskType::parseFromINI,	NULL, offsetof( FireWeaponCollideModuleData, m_requiredStatus ) },
		{ "ForbiddenStatus",	ObjectStatusMaskType::parseFromINI,	NULL, offsetof( FireWeaponCollideModuleData, m_forbiddenStatus ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponCollide::FireWeaponCollide( Thing *thing, const ModuleData* moduleData ) :
	CollideModule( thing, moduleData ),
	m_collideWeapon(NULL)
{
	m_collideWeapon = TheWeaponStore->allocateNewWeapon(getFireWeaponCollideModuleData()->m_collideWeaponTemplate, PRIMARY_WEAPON);
	m_everFired = FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponCollide::~FireWeaponCollide( void )
{
	deleteInstance(m_collideWeapon);
}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void FireWeaponCollide::onCollide( Object *other, const Coord3D *loc, const Coord3D *normal )
{
	if( other == NULL )
		return; //Don't shoot the ground

	Object *me = getObject();

	// This will fire at you every frame, because multiple people could be colliding and we want
	// to hurt them all.  Another solution would be to keep a Map of other->objetIDs and
	// delays for each individually.  However, this solution here is so quick and simple that it
	// warrants the "do it eventually if we need it" clause.
	if( shouldFireWeapon() )
	{
		m_collideWeapon->loadAmmoNow( me );
		m_collideWeapon->fireWeapon( me, other );
	}
}

//-------------------------------------------------------------------------------------------------
Bool FireWeaponCollide::shouldFireWeapon()
{
	const FireWeaponCollideModuleData *d = getFireWeaponCollideModuleData();

	ObjectStatusMaskType status = getObject()->getStatusBits();

	//We need all required status or else we fail
	if( !status.testForAll( d->m_requiredStatus ) )
		return FALSE;

	//If we have any forbidden statii, then fail
	if( status.testForAny( d->m_forbiddenStatus ) )
		return FALSE;

	if( m_everFired && d->m_fireOnce )
		return FALSE;// can only fire once ever

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FireWeaponCollide::crc( Xfer *xfer )
{

	// extend base class
	CollideModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FireWeaponCollide::xfer( Xfer *xfer )
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
											 ("FireWeaponCollide::xfer - m_collideWeapon present mismatch") );
		xfer->xferSnapshot( m_collideWeapon );

	}  // end else
	else
	{

		DEBUG_ASSERTCRASH( m_collideWeapon == NULL,
											 ("FireWeaponCollide::Xfer - m_collideWeapon missing mismatch" ));

	}  // end else

	// ever fired
	xfer->xferBool( &m_everFired );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FireWeaponCollide::loadPostProcess( void )
{

	// extend base class
	CollideModule::loadPostProcess();

}  // end loadPostProcess
