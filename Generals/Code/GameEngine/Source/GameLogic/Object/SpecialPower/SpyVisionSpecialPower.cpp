/*
**	Command & Conquer Generals(tm)
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

// FILE: SpyVisionSpecialPower.cpp /////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September 2002
// Desc:   Special Power will spy on the vision of all enemy players.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/SpyVisionSpecialPower.h"
#include "GameLogic/Module/SpyVisionUpdate.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SpyVisionSpecialPowerModuleData::SpyVisionSpecialPowerModuleData( void )
{

	m_baseDurationInFrames = 0;
	m_bonusDurationPerCapturedInFrames = 0;
	m_maxDurationInFrames = 0;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void SpyVisionSpecialPowerModuleData::buildFieldParse( MultiIniFieldParse &p )
{
	SpecialPowerModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "BaseDuration",								INI::parseDurationUnsignedInt,	nullptr,   offsetof( SpyVisionSpecialPowerModuleData, m_baseDurationInFrames ) },
		{ "BonusDurationPerCaptured",		INI::parseDurationUnsignedInt,	nullptr,   offsetof( SpyVisionSpecialPowerModuleData, m_bonusDurationPerCapturedInFrames ) },
		{ "MaxDuration",								INI::parseDurationUnsignedInt,	nullptr,   offsetof( SpyVisionSpecialPowerModuleData, m_maxDurationInFrames ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add( dataFieldParse );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SpyVisionSpecialPower::SpyVisionSpecialPower( Thing *thing, const ModuleData *moduleData )
												: SpecialPowerModule( thing, moduleData )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SpyVisionSpecialPower::~SpyVisionSpecialPower( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void SpyVisionSpecialPower::doSpecialPower( UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPower( commandOptions );


	// get the source of this power
	Object *source = getObject();

	// get module data
	const SpyVisionSpecialPowerModuleData *modData = getSpyVisionSpecialPowerModuleData();

	//
	// the SpyVision special power increases in range and duration of effect the more
	// units we have captured within us
	//
	UnsignedInt duration = modData->m_baseDurationInFrames;
	ContainModuleInterface *contain = source->getContain();
	if( contain )
	{
		// for every captured unit we get a bonus
		duration += contain->getContainCount() * modData->m_bonusDurationPerCapturedInFrames;

		// cap at the max
		if( duration > modData->m_maxDurationInFrames )
			duration = modData->m_maxDurationInFrames;
	}

	static const NameKeyType key_SpyVisionUpdate = NAMEKEY( "SpyVisionUpdate" );
	SpyVisionUpdate *update = (SpyVisionUpdate*)source->findUpdateModule( key_SpyVisionUpdate );
	if( !update )
	{
		return;
	}

	update->activateSpyVision( duration );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SpyVisionSpecialPower::crc( Xfer *xfer )
{

	// extend base class
	SpecialPowerModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SpyVisionSpecialPower::xfer( Xfer *xfer )
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
void SpyVisionSpecialPower::loadPostProcess( void )
{

	// extend base class
	SpecialPowerModule::loadPostProcess();

}
