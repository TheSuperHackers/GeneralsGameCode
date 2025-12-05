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

// FILE: CreateObjectDie.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Michael S. Booth, January 2002
// Desc:   Create an object upon this object's death
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_OBJECT_STATUS_NAMES
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/CreateObjectDie.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
CreateObjectDieModuleData::CreateObjectDieModuleData()
{

	m_ocl = nullptr;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void CreateObjectDieModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	DieModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "CreationList",	INI::parseObjectCreationList,		nullptr,											offsetof( CreateObjectDieModuleData, m_ocl ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add(dataFieldParse);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CreateObjectDie::CreateObjectDie( Thing *thing, const ModuleData* moduleData ) : DieModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CreateObjectDie::~CreateObjectDie( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void CreateObjectDie::onDie( const DamageInfo * damageInfo )
{
	if (!isDieApplicable(damageInfo))
		return;

	Object *damageDealer = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );

	ObjectCreationList::create(getCreateObjectDieModuleData()->m_ocl, getObject(), damageDealer);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CreateObjectDie::crc( Xfer *xfer )
{

	// extend base class
	DieModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CreateObjectDie::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DieModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CreateObjectDie::loadPostProcess( void )
{

	// extend base class
	DieModule::loadPostProcess();

}
