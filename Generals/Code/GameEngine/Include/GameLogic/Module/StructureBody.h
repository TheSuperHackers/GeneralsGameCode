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

// FILE: StructureBody.h //////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:	 Structure bodies are active bodies specifically for structures that are built
//				 and/or interactable (is that a world) with the player.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __STRUCTUREBODY_H_
#define __STRUCTUREBODY_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/ActiveBody.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;

//-------------------------------------------------------------------------------------------------
class StructureBodyModuleData : public ActiveBodyModuleData
{
public:

	StructureBodyModuleData(){}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    ActiveBodyModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
/** Structure body module */
//-------------------------------------------------------------------------------------------------
class StructureBody : public ActiveBody
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StructureBody, "StructureBody" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StructureBody, StructureBodyModuleData )

public:

	StructureBody( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	void setConstructorObject( Object *obj );
	ObjectID getConstructorObjectID( void ) { return m_constructorObjectID; }

protected:

	ObjectID m_constructorObjectID;					///< object that built this structure

};

#endif // __STRUCTUREBODY_H_

