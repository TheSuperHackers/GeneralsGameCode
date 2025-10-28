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

// FILE: BuffUpdate.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002-2003 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Oct 25
//
//	Filename: 	BuffUpdate.cpp
//
//	author:		Andi W
//
//	purpose:	apply a buff/debuff effect to units in an area
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameLogic/Module/BuffUpdate.h"
#include "GameLogic/BuffSystem.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/ContainModule.h"

//-----------------------------------------------------------------------------
BuffUpdateModuleData::BuffUpdateModuleData()
{
	m_requiredAffectKindOf.clear();
	m_forbiddenAffectKindOf.clear();
	m_targetsMask = WEAPON_AFFECTS_ALLIES;
	m_isAffectAirborne = true;
	m_buffDuration = 0;
	m_buffDelay = 0;
	m_buffRange = 0;
	m_buffTemplateName.clear();
}

//-----------------------------------------------------------------------------
void BuffUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "RequiredAffectKindOf",		KindOfMaskType::parseFromINI,		NULL, offsetof( BuffUpdateModuleData, m_requiredAffectKindOf ) },
		{ "ForbiddenAffectKindOf",	KindOfMaskType::parseFromINI,		NULL, offsetof( BuffUpdateModuleData, m_forbiddenAffectKindOf ) },
		{ "AffectsTargets", INI::parseBitString32,	TheWeaponAffectsMaskNames, offsetof(BuffUpdateModuleData, m_targetsMask) },
		{ "AffectAirborne", INI::parseBool, NULL, offsetof(BuffUpdateModuleData, m_isAffectAirborne) },
		{ "BuffDuration",					INI::parseDurationUnsignedInt,	NULL, offsetof( BuffUpdateModuleData, m_buffDuration) },
		{ "BuffDelay",							INI::parseDurationUnsignedInt,	NULL, offsetof( BuffUpdateModuleData, m_buffDelay) },
		{ "BuffRange",							INI::parseReal,									NULL, offsetof( BuffUpdateModuleData, m_buffRange) },
		{ "BuffTemplateName",			INI::parseAsciiString,	NULL, offsetof( BuffUpdateModuleData, m_buffTemplateName) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
BuffUpdate::BuffUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BuffUpdate::~BuffUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct tempBuffData // This is used for iterator to apply buff to contained objects
{
	const BuffTemplate* m_template;
	Object* m_sourceObj;
	UnsignedInt m_duration;
	KindOfMaskType m_requiredMask;
	KindOfMaskType m_forbiddenMask;
	Bool m_isAffectAirborne;
};
void containIteratingDoBuff( Object *passenger, void *voidData)
{
	tempBuffData *data = (tempBuffData *)voidData;

	if (passenger->isKindOfMulti(data->m_requiredMask, data->m_forbiddenMask)) {
		if (data->m_isAffectAirborne || !passenger->isAirborneTarget()) {
			passenger->applyBuff(data->m_template, data->m_duration, data->m_sourceObj); // TODO: create function in object
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime BuffUpdate::update( void )
{
	DEBUG_LOG(("BuffUpdate::update 0"));

	const BuffUpdateModuleData * data = getBuffUpdateModuleData();
	const BuffTemplate* buffTemp = TheBuffTemplateStore->findBuffTemplate(data->m_buffTemplateName);

	if (!buffTemp) {
		DEBUG_LOG(("BuffUpdate::update -- Could not find BuffTemplate '%s'", data->m_buffTemplateName.str()));
		return UPDATE_SLEEP_FOREVER;
	}

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
																																			data->m_buffRange,
																																			FROM_CENTER_2D,
																																			filters );


	MemoryPoolObjectHolder hold( iter );
	tempBuffData buffData;
	buffData.m_template = buffTemp;
	buffData.m_duration = data->m_buffDuration;
	buffData.m_requiredMask = data->m_requiredAffectKindOf;
	buffData.m_forbiddenMask = data->m_forbiddenAffectKindOf;
	buffData.m_isAffectAirborne = data->m_isAffectAirborne;
	buffData.m_sourceObj = me; // TODO: Support for projectiles

	
	for( Object *currentObj = iter->first(); currentObj != NULL; currentObj = iter->next() )
	{
		if( currentObj->isKindOfMulti(data->m_requiredAffectKindOf, data->m_forbiddenAffectKindOf) )
		{
			if (data->m_isAffectAirborne || !currentObj->isAirborneTarget()) {
				currentObj->applyBuff(buffTemp, data->m_buffDuration, me);  // TODO
			}
		}

		if( currentObj->getContain() )
		{
			currentObj->getContain()->iterateContained(containIteratingDoBuff, &buffData, FALSE);
		}
	}

	return UPDATE_SLEEP(data->m_buffDelay); // Only need an internal timer if there are external hooks for wakning us up, or a second thing we can do
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BuffUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BuffUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BuffUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
