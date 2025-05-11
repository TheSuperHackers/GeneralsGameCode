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

// FILE: FireWeaponPower.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	Created:	August 2003
//
//	Filename: FireWeaponPower.h
//
//	Author:		Kris Morness
//
//  Purpose:	Simply loads and fires a specific weapon controlled by a superweapon timer.
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __FIRE_WEAPON_POWER_H
#define __FIRE_WEAPON_POWER_H

#include "GameLogic/Module/SpecialPowerModule.h"

class Object;
class SpecialPowerTemplate;
struct FieldParse;
enum ScienceType CPP_11(: Int);

class FireWeaponPowerModuleData : public SpecialPowerModuleData
{

public:

	FireWeaponPowerModuleData( void );

	static void buildFieldParse( MultiIniFieldParse& p );

	UnsignedInt m_maxShotsToFire;
};


//-------------------------------------------------------------------------------------------------
class FireWeaponPower : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FireWeaponPower, "FireWeaponPower" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FireWeaponPower, FireWeaponPowerModuleData )

public:

	FireWeaponPower( Thing *thing, const ModuleData *moduleData );

	virtual void doSpecialPower( UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions );
	virtual void doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions );

protected:

};

#endif // __FIRE_WEAPON_POWER_H
