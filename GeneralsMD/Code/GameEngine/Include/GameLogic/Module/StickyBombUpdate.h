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

// FILE: StickyBombUpdate.h ////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2002
// Desc:   Similar to ParachuteContain, this module is used essentially to attach a bomb to an object
//         moving around. The sticky bomb position simply updates to the specified bone until it explodes.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameLogic/Module/UpdateModule.h"
#include "GameClient/Anim2D.h"

class WeaponTemplate;
class FXList;

//-------------------------------------------------------------------------------------------------
class StickyBombUpdateModuleData : public UpdateModuleData
{
public:
	AsciiString			m_attachToBone;
	Real						m_offsetZ;
	WeaponTemplate*	m_geometryBasedDamageWeaponTemplate;
	FXList*					m_geometryBasedDamageFX;

	AsciiString m_animBaseTemplate;
	AsciiString m_animTimedTemplate;
	Bool m_showTimer;     ///< if this is disabled, only use animBase for timed bombs

	Bool m_hideAnimBase;      ///< will be set automatically if String is Null
	Bool m_hideAnimTimed;      ///< will be set automatically if String is Null

	StickyBombUpdateModuleData()
	{
		m_offsetZ = 10.0f;
		m_geometryBasedDamageWeaponTemplate = NULL;
		m_geometryBasedDamageFX = NULL;
		m_animBaseTemplate = AsciiString::TheEmptyString;
		m_animTimedTemplate = AsciiString::TheEmptyString;
		m_showTimer = TRUE;
	}

	static void parseAnimBaseName(INI* ini, void* instance, void* store, const void* userData);
	static void parseAnimTimedName(INI* ini, void* instance, void* store, const void* userData);

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "AttachToTargetBone",				INI::parseAsciiString,		NULL, offsetof( StickyBombUpdateModuleData, m_attachToBone ) },
			{ "OffsetZ",									INI::parseReal,						NULL, offsetof( StickyBombUpdateModuleData, m_offsetZ ) },
			{ "GeometryBasedDamageWeapon",INI::parseWeaponTemplate, NULL, offsetof( StickyBombUpdateModuleData, m_geometryBasedDamageWeaponTemplate ) },
			{ "GeometryBasedDamageFX",		INI::parseFXList,					NULL, offsetof( StickyBombUpdateModuleData, m_geometryBasedDamageFX ) },
			{ "Animation2DBase",		parseAnimBaseName,					NULL, 0 },
			{ "Animation2DTimed",		parseAnimTimedName,					NULL, 0 },
			{ "ShowTimer",		INI::parseBool,					NULL, offsetof( StickyBombUpdateModuleData, m_showTimer) },
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

	void initStickyBomb( Object *object, const Object *bomber, const Coord3D *specificPos = NULL );
	void detonate();
	Bool isTimedBomb() const { return (m_dieFrame > 0) && getStickyBombUpdateModuleData()->m_showTimer; }
	UnsignedInt getDetonationFrame() const { return m_dieFrame; }
	Object* getTargetObject() const;
	void setTargetObject( Object *obj );

	//AsciiString getAnimBaseTemplate() { return getStickyBombUpdateModuleData()->m_animBaseTemplate; }
	//AsciiString getAnimTimedTemplate() { return getStickyBombUpdateModuleData()->m_animTimedTemplate; }

	Anim2DTemplate* getAnimBaseTemplate();
	Anim2DTemplate* getAnimTimedTemplate();

	inline Bool showAnimBaseTemplate() { return !getStickyBombUpdateModuleData()->m_hideAnimBase; }
	inline Bool showAnimTimedTemplate() { return !getStickyBombUpdateModuleData()->m_hideAnimTimed; }

private:

	ObjectID			m_targetID;
	UnsignedInt		m_dieFrame;
	UnsignedInt   m_nextPingFrame;

	Anim2DTemplate* m_animBaseTemplate;
	Anim2DTemplate* m_animTimedTemplate;

};
