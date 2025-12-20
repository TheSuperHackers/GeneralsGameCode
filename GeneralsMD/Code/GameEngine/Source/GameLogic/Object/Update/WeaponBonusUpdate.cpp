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

// FILE: WeaponBonusUpdate.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002-2003 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	July 2003
//
//	Filename: 	WeaponBonusUpdate.cpp
//
//	author:		Graham Smallwood
//
//	purpose:	Like healing in that it can affect just me or people around,
//						except this gives a Weapon Bonus instead of health
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Module/WeaponBonusUpdate.h"

#define DEFINE_WEAPONBONUSCONDITION_NAMES

#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"
#include "GameClient/TintStatus.h"

//-----------------------------------------------------------------------------
WeaponBonusUpdateModuleData::WeaponBonusUpdateModuleData()
{
	m_requiredAffectKindOf.clear();
	m_forbiddenAffectKindOf.clear();
	m_targetsMask = WEAPON_AFFECTS_ALLIES;
	m_isAffectAirborne = true;
	m_bonusDuration = 0;
	m_bonusDelay = 0;
	m_bonusRange = 0;
	m_bonusConditionType = WEAPONBONUSCONDITION_INVALID;
	m_tintStatus = TINT_STATUS_FRENZY;
}

//-----------------------------------------------------------------------------
void WeaponBonusUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "RequiredAffectKindOf",		KindOfMaskType::parseFromINI,		NULL, offsetof( WeaponBonusUpdateModuleData, m_requiredAffectKindOf ) },
		{ "ForbiddenAffectKindOf",	KindOfMaskType::parseFromINI,		NULL, offsetof( WeaponBonusUpdateModuleData, m_forbiddenAffectKindOf ) },
		{ "AffectsTargets", INI::parseBitString32,	TheWeaponAffectsMaskNames, offsetof(WeaponBonusUpdateModuleData, m_targetsMask) },
		{ "AffectAirborne", INI::parseBool, NULL, offsetof(WeaponBonusUpdateModuleData, m_isAffectAirborne) },
		{ "BonusDuration",					INI::parseDurationUnsignedInt,	NULL, offsetof( WeaponBonusUpdateModuleData, m_bonusDuration ) },
		{ "BonusDelay",							INI::parseDurationUnsignedInt,	NULL, offsetof( WeaponBonusUpdateModuleData, m_bonusDelay ) },
		{ "BonusRange",							INI::parseReal,									NULL, offsetof( WeaponBonusUpdateModuleData, m_bonusRange ) },
		{ "BonusConditionType",			INI::parseIndexList,	TheWeaponBonusNames, offsetof( WeaponBonusUpdateModuleData, m_bonusConditionType ) },
		{ "TintStatusType",			TintStatusFlags::parseSingleBitFromINI,	NULL, offsetof( WeaponBonusUpdateModuleData, m_tintStatus ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
WeaponBonusUpdate::WeaponBonusUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WeaponBonusUpdate::~WeaponBonusUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct tempWeaponBonusData // Hey Steven, bite me!  hahahaha  _Lowercase_ since Local to the file.
{
	WeaponBonusConditionType m_type;
	UnsignedInt m_duration;
	KindOfMaskType m_requiredMask;
	KindOfMaskType m_forbiddenMask;
	TintStatus m_tintStatus;
	Bool m_isAffectAirborne;
};
void containIteratingDoTempWeaponBonus( Object *passenger, void *voidData)
{
	tempWeaponBonusData *data = (tempWeaponBonusData *)voidData;

	if (passenger->isKindOfMulti(data->m_requiredMask, data->m_forbiddenMask)) {
		if (data->m_isAffectAirborne || !passenger->isAirborneTarget()) {
			passenger->doTempWeaponBonus(data->m_type, data->m_duration, data->m_tintStatus);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime WeaponBonusUpdate::update( void )
{
	const WeaponBonusUpdateModuleData * data = getWeaponBonusUpdateModuleData();
	Object *me = getObject();

	Int targetFlags = 0;
	if (data->m_targetsMask & WEAPON_AFFECTS_ALLIES) targetFlags |= PartitionFilterRelationship::ALLOW_ALLIES;
	if (data->m_targetsMask & WEAPON_AFFECTS_ENEMIES) targetFlags |= PartitionFilterRelationship::ALLOW_ENEMIES;
	if (data->m_targetsMask & WEAPON_AFFECTS_NEUTRALS) targetFlags |= PartitionFilterRelationship::ALLOW_NEUTRAL;

	PartitionFilterRelationship relationship(me, targetFlags);
	PartitionFilterSameMapStatus filterMapStatus(me);
	PartitionFilterAlive filterAlive;

	// Leaving this here commented out to show that I need to reach valid contents of invalid transports.
	// So these checks are on an individual basis, not in the Partition query
//	PartitionFilterAcceptByKindOf filterKindof(data->m_requiredAffectKindOf,data->m_forbiddenAffectKindOf);
	PartitionFilter *filters[] = { &relationship, &filterAlive, &filterMapStatus, NULL };

	// scan objects in our region
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( me->getPosition(),
																																			data->m_bonusRange,
																																			FROM_CENTER_2D,
																																			filters );
	MemoryPoolObjectHolder hold( iter );
	tempWeaponBonusData weaponBonusData;
	weaponBonusData.m_type = data->m_bonusConditionType;
	weaponBonusData.m_duration = data->m_bonusDuration;
	weaponBonusData.m_requiredMask = data->m_requiredAffectKindOf;
	weaponBonusData.m_forbiddenMask = data->m_forbiddenAffectKindOf;
	weaponBonusData.m_tintStatus = data->m_tintStatus;
	weaponBonusData.m_isAffectAirborne = data->m_isAffectAirborne;

	
	for( Object *currentObj = iter->first(); currentObj != NULL; currentObj = iter->next() )
	{
		if( currentObj->isKindOfMulti(data->m_requiredAffectKindOf, data->m_forbiddenAffectKindOf) )
		{
			if (data->m_isAffectAirborne || !currentObj->isAirborneTarget()) {
				currentObj->doTempWeaponBonus(data->m_bonusConditionType, data->m_bonusDuration, data->m_tintStatus);
			}
		}

		if( currentObj->getContain() )
		{
			currentObj->getContain()->iterateContained(containIteratingDoTempWeaponBonus, &weaponBonusData, FALSE);
		}
	}

	return UPDATE_SLEEP(data->m_bonusDelay); // Only need an internal timer if there are external hooks for wakning us up, or a second thing we can do
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void WeaponBonusUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void WeaponBonusUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void WeaponBonusUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
