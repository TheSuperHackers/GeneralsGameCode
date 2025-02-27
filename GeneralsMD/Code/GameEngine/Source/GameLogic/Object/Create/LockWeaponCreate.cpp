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

// FILE: LockWeaponCreate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2003
// Desc:   Locks the weapon choice to the slot specified on creation
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_WEAPONSLOTTYPE_NAMES
#include "Common/Xfer.h"
#include "GameLogic/Module/LockWeaponCreate.h"
#include "GameLogic/Object.h"
#include "GameLogic/WeaponSet.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
LockWeaponCreateModuleData::LockWeaponCreateModuleData()
{
	m_slotToLock = PRIMARY_WEAPON;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void LockWeaponCreateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  CreateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "SlotToLock",	INI::parseLookupList,	TheWeaponSlotTypeNamesLookupList, offsetof( LockWeaponCreateModuleData, m_slotToLock ) },
		{ 0, 0, 0, 0 }
	};

  p.add(dataFieldParse);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
LockWeaponCreate::LockWeaponCreate( Thing *thing, const ModuleData* moduleData ) : CreateModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
LockWeaponCreate::~LockWeaponCreate( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The create callback. */
//-------------------------------------------------------------------------------------------------
void LockWeaponCreate::onCreate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void LockWeaponCreate::onBuildComplete( void )
{
	CreateModule::onBuildComplete(); // extend

	Object *me = getObject();
	WeaponSlotType slot = getLockWeaponCreateModuleData()->m_slotToLock;
	me->setWeaponLock( slot, LOCKED_PERMANENTLY );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void LockWeaponCreate::crc( Xfer *xfer )
{

	// extend base class
	CreateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void LockWeaponCreate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CreateModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void LockWeaponCreate::loadPostProcess( void )
{

	// extend base class
	CreateModule::loadPostProcess();

}
