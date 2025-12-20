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

// FILE: CommandButtonHuntUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: John Ahlquist, Sept. 2002
// Desc:   Update module to handle wounded idle infantry finding a heal unit or structure.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class ThingTemplate;
class WeaponTemplate;


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class CommandButtonHuntUpdateModuleData : public ModuleData
{
public:
	UnsignedInt			m_scanFrames;
	Real						m_scanRange;

	CommandButtonHuntUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class CommandButtonHuntUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( CommandButtonHuntUpdate, "CommandButtonHuntUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( CommandButtonHuntUpdate, CommandButtonHuntUpdateModuleData );

public:

	CommandButtonHuntUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onObjectCreated();
	virtual UpdateSleepTime update();

	void setCommandButton(const AsciiString& buttonName);

protected:
	Object* scanClosestTarget(void);
	UpdateSleepTime huntSpecialPower(AIUpdateInterface *ai);
	UpdateSleepTime huntWeapon(AIUpdateInterface *ai);


protected:
	AsciiString		m_commandButtonName;
	const CommandButton *m_commandButton;
};
