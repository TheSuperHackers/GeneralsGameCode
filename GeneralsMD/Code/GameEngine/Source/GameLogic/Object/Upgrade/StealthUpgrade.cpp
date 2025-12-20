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

// FILE: StealthUpgrade.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, May 2002
// Desc:	 An upgrade that set's the owner's OBJECT_STATUS_CAN_STEALTH Status
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_STEALTHLEVEL_NAMES

#include "Common/Xfer.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/StealthUpgrade.h"
#include "GameLogic/Module/SpawnBehavior.h"
#include "GameLogic/Object.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void StealthUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpgradeModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "EnableStealth", INI::parseBool, NULL, offsetof(StealthUpgradeModuleData, m_enableStealth) },
		{ "OverrideStealthForbiddenConditions", INI::parseBitString32, TheStealthLevelNames, offsetof(StealthUpgradeModuleData, m_stealthLevel) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StealthUpgrade::StealthUpgrade( Thing *thing, const ModuleData* moduleData ) : UpgradeModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StealthUpgrade::~StealthUpgrade( void )
{
}

//-------------------------------------------------------------------------------------------------
void StealthUpgrade::upgradeImplementation()
{

	const StealthUpgradeModuleData* d = getStealthUpgradeModuleData();

	if (d->m_stealthLevel > 0) {
		StealthUpdate* stealth = getObject()->getStealth();
		if (stealth) {  // we should always have a stealth update module
			stealth->setStealthLevelOverride(d->m_stealthLevel);
		}
		return;  // Note AW: There should be no reason to enable/disable stealth if you change the stealthLevel
	}

	// The logic that does the stealthupdate will notice this and start stealthing
	Object* me = getObject();
	me->setStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_CAN_STEALTH), d->m_enableStealth);

	//Grant stealth to spawns if applicable.
	if (me->isKindOf(KINDOF_SPAWNS_ARE_THE_WEAPONS))
	{
		SpawnBehaviorInterface* sbInterface = me->getSpawnBehaviorInterface();
		if (sbInterface)
		{
			sbInterface->giveSlavesStealthUpgrade(d->m_enableStealth);
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StealthUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StealthUpgrade::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpgradeModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StealthUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}
