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

// FILE: RebuildHoleExposeDie.h ///////////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   When a structure dies with this module, a rebuild hole will be created in place
//				 of the structure that will automatically rebuild the structure
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/DieModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
class RebuildHoleExposeDieModuleData : public DieModuleData
{

public:

	RebuildHoleExposeDieModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	AsciiString m_holeName;			///< name of hole object to create
	Real m_holeMaxHealth;				///< max health of hole to create
	Bool m_transferAttackers;		///< if true, people attacking us should switch attacks to the hole when we die

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class RebuildHoleExposeDie : public DieModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RebuildHoleExposeDie, "RebuildHoleExposeDie"  )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RebuildHoleExposeDie, RebuildHoleExposeDieModuleData );

public:

	RebuildHoleExposeDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDie( const DamageInfo *damageInfo );

};
