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

// FILE: SpecialPowerCompletionDie.h //////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, May 2002
// Desc:   Die method responsible for telling TheScriptEngine that a special power has been completed
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/DieModule.h"

class SpecialPowerTemplate;

//-------------------------------------------------------------------------------------------------
class SpecialPowerCompletionDieModuleData : public DieModuleData
{
public:
	SpecialPowerTemplate *m_specialPowerTemplate;		///< pointer to the special power template

	SpecialPowerCompletionDieModuleData()
	{
		m_specialPowerTemplate = NULL;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    DieModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "SpecialPowerTemplate", INI::parseSpecialPowerTemplate,	NULL, offsetof( SpecialPowerCompletionDieModuleData, m_specialPowerTemplate ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}
};

//-------------------------------------------------------------------------------------------------
/** Special power completion die */
//-------------------------------------------------------------------------------------------------
class SpecialPowerCompletionDie : public DieModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpecialPowerCompletionDie, "SpecialPowerCompletionDie" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpecialPowerCompletionDie, SpecialPowerCompletionDieModuleData )

public:

	SpecialPowerCompletionDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	void setCreator( ObjectID creatorID );
	void notifyScriptEngine( void );

	virtual void onDie( const DamageInfo *damageInfo );

protected:

	ObjectID m_creatorID;
	Bool m_creatorSet;

};

// Creator is stored as ID, so a failed lookup just means that he died first and noone cares that we are going.
