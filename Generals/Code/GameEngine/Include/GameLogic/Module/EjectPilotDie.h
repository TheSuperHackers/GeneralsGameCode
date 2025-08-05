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

// FILE: EjectPilotDie.h /////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, April 2002
// Desc:   Create object at current object's death
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _EjectPilotDie_H_
#define _EjectPilotDie_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/DieModule.h"
#include "Common/INI.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class ObjectCreationList;

//-------------------------------------------------------------------------------------------------
class EjectPilotDieModuleData : public DieModuleData
{
public:
	const ObjectCreationList* m_oclInAir;
	const ObjectCreationList* m_oclOnGround;
	UnsignedInt m_invulnerableTime;

	EjectPilotDieModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
/** When this object dies, create another object in its place */
//-------------------------------------------------------------------------------------------------
class EjectPilotDie : public DieModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( EjectPilotDie, "EjectPilotDie"  )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( EjectPilotDie, EjectPilotDieModuleData );

public:

	EjectPilotDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static void ejectPilot(const ObjectCreationList* ocl, const Object* dyingObject, const Object* damageDealer);

	virtual void onDie( const DamageInfo *damageInfo );
	virtual DieModuleInterface* getEjectPilotDieInterface( void ) {return this; }

};

#endif // _EjectPilotDie_H_

