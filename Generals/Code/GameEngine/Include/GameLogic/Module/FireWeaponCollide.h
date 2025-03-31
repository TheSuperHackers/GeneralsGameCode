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

// FILE: FireWeaponCollide.h ///////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood  April 2002
// Desc:   Shoot something that collides with me every frame with my weapon
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __FireWeaponCollide_H_
#define __FireWeaponCollide_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/CollideModule.h"
#include "GameLogic/Weapon.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;

//-------------------------------------------------------------------------------------------------
class FireWeaponCollideModuleData : public CollideModuleData
{
public:
	const WeaponTemplate* m_collideWeaponTemplate;
	ObjectStatusMaskType m_requiredStatus;
	ObjectStatusMaskType m_forbiddenStatus;
	Bool m_fireOnce;

	FireWeaponCollideModuleData()
	{
		m_collideWeaponTemplate = NULL;
		m_requiredStatus = 0;		// nothing required
		m_forbiddenStatus = 0;	// nothing forbidden
		m_fireOnce = FALSE;
	}

	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class FireWeaponCollide : public CollideModule
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FireWeaponCollide, FireWeaponCollideModuleData );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FireWeaponCollide, "FireWeaponCollide" )

public:

	FireWeaponCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

protected:

	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );

	virtual Bool shouldFireWeapon();

private:
	Weapon* m_collideWeapon;
	Bool m_everFired;

};


#endif

