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

// FILE: DemoTrapUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, August 2002
// Desc:   Update module to handle demo trap proximity triggering.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_WEAPONSLOTTYPE_NAMES

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/BitFlagsIO.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/Module/DemoTrapUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/WeaponSet.h"
#include "GameLogic/Weapon.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DemoTrapUpdateModuleData::DemoTrapUpdateModuleData()
{
	m_defaultsToProximityMode				= false;
	m_friendlyDetonation						= false;
	m_manualModeWeaponSlot					= PRIMARY_WEAPON;
	m_detonationWeaponSlot					= PRIMARY_WEAPON;
	m_proximityModeWeaponSlot				= PRIMARY_WEAPON;
	m_triggerDetonationRange				= 0.0f;
	m_scanFrames										= 0;
	m_detonationWeaponTemplate			= NULL;
	m_detonateWhenKilled						= false;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void DemoTrapUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
    { "DefaultProximityMode",      INI::parseBool,								NULL, offsetof( DemoTrapUpdateModuleData, m_defaultsToProximityMode ) },
    { "DetonationWeaponSlot",      INI::parseLookupList,					TheWeaponSlotTypeNamesLookupList, offsetof( DemoTrapUpdateModuleData, m_detonationWeaponSlot ) },
    { "ProximityModeWeaponSlot",   INI::parseLookupList,					TheWeaponSlotTypeNamesLookupList, offsetof( DemoTrapUpdateModuleData, m_proximityModeWeaponSlot ) },
    { "ManualModeWeaponSlot",      INI::parseLookupList,					TheWeaponSlotTypeNamesLookupList, offsetof( DemoTrapUpdateModuleData, m_manualModeWeaponSlot ) },
    { "TriggerDetonationRange",    INI::parseReal,								NULL, offsetof( DemoTrapUpdateModuleData, m_triggerDetonationRange ) },
    { "IgnoreTargetTypes",         KindOfMaskType::parseFromINI,							NULL, offsetof( DemoTrapUpdateModuleData, m_ignoreKindOf ) },
		{ "ScanRate",									 INI::parseDurationUnsignedInt,	NULL, offsetof( DemoTrapUpdateModuleData, m_scanFrames ) },
		{ "AutoDetonationWithFriendsInvolved", INI::parseBool,				NULL, offsetof( DemoTrapUpdateModuleData, m_friendlyDetonation ) },
		{ "DetonationWeapon",					 INI::parseWeaponTemplate,			NULL, offsetof( DemoTrapUpdateModuleData, m_detonationWeaponTemplate ) },
		{ "DetonateWhenKilled",				 INI::parseBool,								NULL, offsetof( DemoTrapUpdateModuleData, m_detonateWhenKilled ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
DemoTrapUpdate::DemoTrapUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_nextScanFrames = 0;
	m_detonated = false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DemoTrapUpdate::~DemoTrapUpdate( void )
{

}


//-------------------------------------------------------------------------------------------------
// Validate that we have the necessary data from the ini file.
//-------------------------------------------------------------------------------------------------
void DemoTrapUpdate::onObjectCreated()
{

	const DemoTrapUpdateModuleData *data = getDemoTrapUpdateModuleData();

	if( data->m_detonationWeaponSlot == data->m_proximityModeWeaponSlot ||
			data->m_detonationWeaponSlot == data->m_manualModeWeaponSlot ||
			data->m_proximityModeWeaponSlot == data->m_manualModeWeaponSlot )
	{
		DEBUG_CRASH( ("The demo trap requires three weaponslots: One for each of the detonation mode, proximity mode, and manual mode.") );
	}

	getObject()->setWeaponSetFlag( WEAPONSET_VETERAN );
	if( data->m_defaultsToProximityMode )
	{
		// lock it just till the weapon is empty or the attack is "done"
		getObject()->setWeaponLock( data->m_proximityModeWeaponSlot, LOCKED_TEMPORARILY );
	}
	else
	{
		// lock it just till the weapon is empty or the attack is "done"
		getObject()->setWeaponLock( data->m_manualModeWeaponSlot, LOCKED_TEMPORARILY );
	}
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime DemoTrapUpdate::update()
{
/// @todo srj use SLEEPY_UPDATE here
	const DemoTrapUpdateModuleData *data = getDemoTrapUpdateModuleData();

	if( m_detonated )
	{
		return UPDATE_SLEEP_NONE;
	}

	Object *me = getObject();

	if( me->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) || me->testStatus(OBJECT_STATUS_SOLD) )
	{
		return UPDATE_SLEEP_NONE;
	}

	if( me->isEffectivelyDead() )
	{
		if( data->m_detonateWhenKilled )
		{
			detonate();
		}
		return UPDATE_SLEEP_NONE;
	}

	//Get the current weapon slot -- this determines what mode we're in.
	WeaponSlotType weaponSlot = getObject()->getCurrentWeapon()->getWeaponSlot();

	if( weaponSlot == data->m_detonationWeaponSlot )
	{
		//We've been externally triggered by the press of a command button.
		detonate();
		return UPDATE_SLEEP_NONE;
	}

	//Don't scan every frame for performance reasons.
	if( m_nextScanFrames > 0 )
	{
		m_nextScanFrames--;
		return UPDATE_SLEEP_NONE;
	}


	if( weaponSlot == data->m_manualModeWeaponSlot )
	{
		//Don't scan!
		return UPDATE_SLEEP_NONE;
	}

	//Reset timer here -- because if we are in manual mode, and switch, we want instant
	//gratification (if possible).
	m_nextScanFrames = data->m_scanFrames;

	//Scan for a valid enemy in proximity range.

	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( me->getPosition(), data->m_triggerDetonationRange, FROM_CENTER_2D );
	MemoryPoolObjectHolder hold(iter);

	Bool shallDetonate = false;

	//Now iterate through each object in range and check to see if it should detonate us!
	for( Object *other = iter->first(); other; other = iter->next() )
	{
		if( other->isAnyKindOf( data->m_ignoreKindOf ) )
		{
			//Skip specified types to ignore.
			continue;
		}
		if( other->isEffectivelyDead() )
		{
			continue;
		}

		if( other->isKindOf( KINDOF_DOZER ) )
		{
			//If we're dealing with a dozer... check if it's trying to disarm me. If so, don't blow up!
			Weapon *weapon = other->getCurrentWeapon();
			if( weapon && weapon->getDamageType() == DAMAGE_DISARM )
			{
				//Also check if it's attacking, because it seems to stay in disarm mode.
				if( other->testStatus( OBJECT_STATUS_IS_ATTACKING ) )
				{
					continue;
				}
			}
		}

			// order matters: we want to know if I consider it to be an enemy, not vice versa
		if( getObject()->getRelationship( other ) != ENEMIES )
		{
			if( !data->m_friendlyDetonation )
			{
				//Not allowed to proximity detonate with friends nearby
				return UPDATE_SLEEP_NONE;
			}
			//Don't shoot our friends!
			continue;
		}

		if( other->isAboveTerrain() )
		{
			//Don't detonate on anything airborne.
			continue;
		}

		//Anyone close enough?
		Real fDist = ThePartitionManager->getDistanceSquared( me, other, FROM_CENTER_2D );
		if( fDist <= data->m_triggerDetonationRange * data->m_triggerDetonationRange )
		{
			//Yeehaw!
			shallDetonate = true;

			if( data->m_friendlyDetonation )
			{
				//Okay, no need to look for friends because we don't care. All we care
				//about is the fact that there is an enemy nearby!
				break;
			}
		}
	}

	if( shallDetonate )
	{
		//Enemy in proximity and we are in proximity detonation mode, so trigger the explosion
		//and kill them!!! Muwahahaha!
		detonate();
	}
	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void DemoTrapUpdate::detonate()
{
	const DemoTrapUpdateModuleData *data = getDemoTrapUpdateModuleData();
	Object *me = getObject();

	// Only shoot the weapon if not being built or sold.
	if( !me->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) && !me->testStatus(OBJECT_STATUS_SOLD) )
		TheWeaponStore->createAndFireTempWeapon( data->m_detonationWeaponTemplate, me, me->getPosition() );

	me->kill();
	m_detonated = true;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DemoTrapUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void DemoTrapUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// next scan frames
	xfer->xferInt( &m_nextScanFrames );

	// detonated
	xfer->xferBool( &m_detonated );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DemoTrapUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
