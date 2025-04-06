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

// FILE: GrantUpgradeCreate.h //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, April 2002
// Desc:   GrantUpgrade create module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GRANTUPGRADECREATE_H_
#define __GRANTUPGRADECREATE_H_

#define DEFINE_OBJECT_STATUS_NAMES

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/CreateModule.h"
#include "GameLogic/Object.h"
#include "Common/ObjectStatusTypes.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
/** The GrantUpgrade create module */
//-------------------------------------------------------------------------------------------------

class GrantUpgradeCreateModuleData : public CreateModuleData
{
public:
	AsciiString		m_upgradeName;			///< name of the upgrade to be granted.
	ObjectStatusMaskType m_exemptStatus; ///< do not execute if this status is set in the object

	GrantUpgradeCreateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class GrantUpgradeCreate : public CreateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( GrantUpgradeCreate, "GrantUpgradeCreate" );
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( GrantUpgradeCreate, GrantUpgradeCreateModuleData );


public:

	GrantUpgradeCreate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// the create method
	virtual void onCreate( void );
	virtual void onBuildComplete();	///< This is called when you are a finished game object

protected:

};

#endif // __GRANTUPGRADECREATE_H_

