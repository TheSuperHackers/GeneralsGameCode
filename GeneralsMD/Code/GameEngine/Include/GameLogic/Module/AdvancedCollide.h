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

// FILE: AdvancedCollide.h ///////////////////////////////////////////////////////////////////////////
// Author: Andi W, Oct 25
// Desc:   Do something if we collide with objects or the ground
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __AdvancedCollide_H_
#define __AdvancedCollide_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/CollideModule.h"
#include "GameLogic/Weapon.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;
class FXList;
class ObjectCreationList;

//-------------------------------------------------------------------------------------------------
class AdvancedCollideModuleData : public CollideModuleData
{
public:
	const WeaponTemplate* m_collideWeaponTemplate;  ///< Weapon To Fire
	const FXList* m_fxlist;   ///< FX to play
	const ObjectCreationList* m_ocl;			///< object creaton list to make

	KindOfMaskType	m_kindof;				///< the kind(s) of units that can be collided with
	KindOfMaskType	m_kindofnot;		///< the kind(s) of units that CANNOT be collided with

  ObjectStatusMaskType m_requiredStatus;
	ObjectStatusMaskType m_forbiddenStatus;
	ObjectStatusMaskType m_targetRequiredStatus;
	ObjectStatusMaskType m_targetForbiddenStatus;

	Bool m_fireOnce;     ///< trigger effects only once
	Bool m_collideWithGround;    ///< trigger on collide with Ground
	Bool m_collideWithObjects;   ///< trigger on collide with Objects

	Real m_triggerChance;  ///< chance to trigger effects on a valid collission
	Bool m_rollOnce;  ///< Only Roll once to trigger effects; Or roll every frame.


	AdvancedCollideModuleData()
	{
		m_collideWeaponTemplate = NULL;
		m_fxlist = NULL;
		m_ocl = NULL;
		m_fireOnce = FALSE;
		m_collideWithGround = FALSE;
		m_collideWithObjects = TRUE;
		m_triggerChance = 1.0;
		m_rollOnce = FALSE;
	}

	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class AdvancedCollide : public CollideModule
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( AdvancedCollide, AdvancedCollideModuleData );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AdvancedCollide, "AdvancedCollide" )

public:

	AdvancedCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

protected:

	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );

	virtual Bool shouldTrigger(Object* other);

private:
	Weapon* m_collideWeapon;
	Bool m_everFired;
	Real m_roll;

};


#endif

