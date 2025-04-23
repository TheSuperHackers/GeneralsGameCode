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

// FILE: ArmorDamageScalarUpdate.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002-2003 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	March 2025
//
//	Filename: 	ArmorDamageScalarUpdate.h
//
//	author:		Andi W
//	
//	purpose:	Apply a temporary multiplier to damage taken to units in an area
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ARMOR_DAMAGE_SCALAR_UPDATE_H_
#define __ARMOR_DAMAGE_SCALAR_UPDATE_H_

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameLogic/Module/UpdateModule.h"
//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// enum WeaponBonusConditionType;
class DamageFX;

//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class ArmorDamageScalarUpdateModuleData : public UpdateModuleData
{
public:

	ArmorDamageScalarUpdateModuleData();

	KindOfMaskType						m_allowAffectKindOf;						///< Must be set on target
	KindOfMaskType						m_forbiddenAffectKindOf;	///< Must be clear on target
	Int									m_targetsMask;				///< ALLIES, ENEMIES or NEUTRALS
	Bool								m_isAffectAirborne;					///< Affect Airborne targets
	UnsignedInt							m_bonusDuration;					///< How long a hit lasts on target
	UnsignedInt							m_initialDelay;							///< Delay before effect
	Real								m_bonusRange;							///< How far to affect
	Real								m_armorDamageScalar;					///< damage (i.e. armor) scalar effect magnitude 1.0 = full damage; < 1.0 = less damage; > 1.0 = more damage
	DamageFX*							m_damageFx;								///< override the objects damageFX during effect	
	const ParticleSystemTemplate*		m_effectParticleSystem;					///< particles attached to random points of the object
	Bool								m_scaleParticleCount;					///< scales emission rate of attached particle based on object's size
	// Real								m_sparksPerCubicFoot;						///< just like it sounds
	static void buildFieldParse(MultiIniFieldParse& p);
};


//-------------------------------------------------------------------------------------------------
class ArmorDamageScalarUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ArmorDamageScalarUpdate, "ArmorDamageScalarUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(ArmorDamageScalarUpdate, ArmorDamageScalarUpdateModuleData )

public:

	ArmorDamageScalarUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	UnsignedInt getDieFrame() { return m_dieFrame; }

	virtual UpdateSleepTime update( void );
	void applyEffect(void);
	void removeEffect(void);

	void applyEffectToObject(Object* obj);
	void removeEffectFromObject(Object* obj);


protected:

	ObjectIDVector m_affectedObjects;
	UnsignedInt m_dieFrame;

	Bool m_effectApplied;
};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

#endif
