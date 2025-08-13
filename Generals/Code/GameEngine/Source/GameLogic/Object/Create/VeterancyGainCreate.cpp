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

// FILE: VeterancyGainCreate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, August 2002
// Desc:   On creation, will set Object's veterancy level if required Science is present.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_VETERANCY_NAMES				// for TheVeterancyNames[]
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/VeterancyGainCreate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
VeterancyGainCreateModuleData::VeterancyGainCreateModuleData()
{
	m_startingLevel = LEVEL_REGULAR;
	m_scienceRequired = SCIENCE_INVALID;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VeterancyGainCreateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  CreateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "StartingLevel",		INI::parseIndexList,	TheVeterancyNames,	offsetof( VeterancyGainCreateModuleData, m_startingLevel ) },
		{ "ScienceRequired",	INI::parseScience,		NULL,								offsetof( VeterancyGainCreateModuleData, m_scienceRequired ) },
		{ 0, 0, 0, 0 }
	};

  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
VeterancyGainCreate::VeterancyGainCreate( Thing *thing, const ModuleData* moduleData ) : CreateModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
VeterancyGainCreate::~VeterancyGainCreate( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The create callback. */
//-------------------------------------------------------------------------------------------------
void VeterancyGainCreate::onCreate( void )
{

	// When produced normally, this Object will ask the Player if the correct Science is known for it
	// to set its level to the given level

	const VeterancyGainCreateModuleData *md = getVeterancyGainCreateModuleData();
	Player *myPlayer = getObject()->getControllingPlayer();
	if( myPlayer && (md->m_scienceRequired == SCIENCE_INVALID ||
									 myPlayer->hasScience( md->m_scienceRequired )) )
	{
		ExperienceTracker* myExp = getObject()->getExperienceTracker();
		if( myExp  &&  myExp->isTrainable() )
		{
			// srj sez: use "setMin" here so that we never lose levels
			myExp->setMinVeterancyLevel( md->m_startingLevel );// sVL can override isTrainable, but this module should not.
		}
	}

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void VeterancyGainCreate::crc( Xfer *xfer )
{

	// extend base class
	CreateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void VeterancyGainCreate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CreateModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void VeterancyGainCreate::loadPostProcess( void )
{

	// extend base class
	CreateModule::loadPostProcess();

}  // end loadPostProcess
