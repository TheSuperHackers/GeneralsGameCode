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

// FILE: StickyBombUpdate.h ////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2002
// Desc:   Similar to ParachuteContain, this module is used essentially to attach a bomb to an object
//         moving around. The sticky bomb position simply updates to the specified bone until it explodes.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __STICK_BOMB_UPDATE_H
#define __STICK_BOMB_UPDATE_H

#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
class StickyBombUpdateModuleData : public UpdateModuleData
{
public:
	AsciiString		m_attachToBone;
	Real					m_offsetZ;

	StickyBombUpdateModuleData()
	{
		m_offsetZ = 10.0f;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "AttachToTargetBone",	INI::parseAsciiString,						NULL, offsetof( StickyBombUpdateModuleData, m_attachToBone ) },
			{ "OffsetZ",						INI::parseReal,						NULL, offsetof( StickyBombUpdateModuleData, m_offsetZ ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class StickyBombUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StickyBombUpdate, "StickyBombUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StickyBombUpdate, StickyBombUpdateModuleData )

public:

	StickyBombUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onObjectCreated();

	virtual UpdateSleepTime update();							///< called once per frame

	void init( const Object *object, const Object *bomber );
	void detonate();
	Bool isTimedBomb() const { return m_dieFrame > 0; }
	UnsignedInt getDetonationFrame() const { return m_dieFrame; }
	Object* getTargetObject() const;
	void setTargetObject( Object *obj );

private:

	ObjectID			m_targetID;
	UnsignedInt		m_dieFrame;
	UnsignedInt   m_nextPingFrame;
};

#endif // __STICK_BOMB_UPDATE_H

