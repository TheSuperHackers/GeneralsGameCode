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

// FILE: DemoralizeSpecialPower.cpp ///////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   Demoralize
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#ifdef ALLOW_DEMORALIZE

#include "Common/Xfer.h"
#include "GameClient/FXList.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/DemoralizeSpecialPower.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
DemoralizeSpecialPowerModuleData::DemoralizeSpecialPowerModuleData( void )
{

	m_baseRange = 0.0f;
	m_bonusRangePerCaptured = 0.0f;
	m_maxRange = 0.0f;
	m_baseDurationInFrames = 0;
	m_bonusDurationPerCapturedInFrames = 0;
	m_maxDurationInFrames = 0;
	m_fxList = NULL;

}  // end DemoralizeSpecialPowerModuleData

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void DemoralizeSpecialPowerModuleData::buildFieldParse( MultiIniFieldParse &p )
{
	SpecialPowerModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "BaseRange",									INI::parseReal,			NULL,			offsetof( DemoralizeSpecialPowerModuleData, m_baseRange ) },
		{ "BonusRangePerCaptured",			INI::parseReal,			NULL,			offsetof( DemoralizeSpecialPowerModuleData, m_bonusRangePerCaptured ) },
		{ "MaxRange",										INI::parseReal,			NULL,			offsetof( DemoralizeSpecialPowerModuleData, m_maxRange ) },
		{ "BaseDuration",								INI::parseDurationUnsignedInt,	NULL,   offsetof( DemoralizeSpecialPowerModuleData, m_baseDurationInFrames ) },
		{ "BonusDurationPerCaptured",		INI::parseDurationUnsignedInt,	NULL,   offsetof( DemoralizeSpecialPowerModuleData, m_bonusDurationPerCapturedInFrames ) },
		{ "MaxDuration",								INI::parseDurationUnsignedInt,	NULL,   offsetof( DemoralizeSpecialPowerModuleData, m_maxDurationInFrames ) },
		{ "FXList",											INI::parseFXList,								NULL,		offsetof( DemoralizeSpecialPowerModuleData, m_fxList ) },
		{ 0, 0, 0, 0 }
	};
	p.add( dataFieldParse );

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
DemoralizeSpecialPower::DemoralizeSpecialPower( Thing *thing, const ModuleData *moduleData )
												: SpecialPowerModule( thing, moduleData )
{

}  // end DemoralizeSpecialPower

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
DemoralizeSpecialPower::~DemoralizeSpecialPower( void )
{

}  // end ~DemoralizeSpecialPower

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void DemoralizeSpecialPower::doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	// sanity
	if( loc == NULL )
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPowerAtLocation( loc, angle, commandOptions );

	// the source of my fiendish power
	Object *source = getObject();

	// get module data
	const DemoralizeSpecialPowerModuleData *modData = getDemoralizeSpecialPowerModuleData();

	//
	// the demoralize special power increases in range and duration of effect the more
	// units we have captured within us
	//
	UnsignedInt duration = modData->m_baseDurationInFrames;
	Real range = modData->m_baseRange;
	ContainModuleInterface *contain = source->getContain();
	if( contain )
	{

		// for every captured unit we get a bonus
		duration += contain->getContainCount() * modData->m_bonusDurationPerCapturedInFrames;

		// cap at the max
		if( duration > modData->m_maxDurationInFrames )
			duration = modData->m_maxDurationInFrames;

		// increase the range for every prisoner we got
		range += contain->getContainCount() * modData->m_bonusRangePerCaptured;

		// cap at the max
		if( range > modData->m_maxRange )
			range = modData->m_maxRange;

	}  // end if

	// scan the objects in the area range and demoralize them
	PartitionFilterRelationship filter1( source, PartitionFilterRelationship::ALLOW_ENEMIES |
																							PartitionFilterRelationship::ALLOW_NEUTRAL );
	PartitionFilterAcceptByKindOf filter2( MAKE_KINDOF_MASK( KINDOF_INFANTRY ),
																				 KINDOFMASK_NONE );
	PartitionFilterSameMapStatus filterMapStatus(source);
	PartitionFilter *filters[] = { &filter1, &filter2, &filterMapStatus, NULL };
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( loc,
																																		 range,
																																		 FROM_CENTER_2D,
																																		 filters );

	MemoryPoolObjectHolder hold( iter );
	AIUpdateInterface *ai;
	for( Object *obj = iter->first(); obj; obj = iter->next() )
	{

		// demoralize only affects with ai of course
		ai = obj->getAIUpdateInterface();
		if( ai )
			ai->setDemoralized( duration );

	}  // end for obj

	// play an effect at the destination location
	if( modData->m_fxList )
	{

		// execute FX
		FXList::doFXPos( modData->m_fxList, loc );

	}  // end if

}    // end doSpecialPowerAtLocation

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void DemoralizeSpecialPower::doSpecialPowerAtObject( const Object *obj, Real angle, UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	if( obj )
		doSpecialPowerAtLocation( obj->getPosition(), angle, commandOptions );

}  // end doSpecialPowerAtObject

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DemoralizeSpecialPower::crc( Xfer *xfer )
{

	// extend base class
	SpecialPowerModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void DemoralizeSpecialPower::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	SpecialPowerModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DemoralizeSpecialPower::loadPostProcess( void )
{

	// extend base class
	SpecialPowerModule::loadPostProcess();

}  // end loadPostProcess

#endif	// #ifdef ALLOW_DEMORALIZE

