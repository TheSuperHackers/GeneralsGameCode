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

// FILE: ObjectStatusTypes.h /////////////////////////////////////////////////////////////////////////
// Author: Kris, May 2003
// Desc:   Object status types that are stackable using the BitSet system. Used to be ObjectStatusBits
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __OBJECT_STATUS_TYPES_H
#define __OBJECT_STATUS_TYPES_H

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/BitFlags.h"
#include "Common/BitFlagsIO.h"

//-------------------------------------------------------------------------------------------------
/** Object status types */
//-------------------------------------------------------------------------------------------------
enum ObjectStatusTypes CPP_11(: Int)
{
	//These are saved. Do not insert or remove any!

	OBJECT_STATUS_NONE,									///< no status bit
	OBJECT_STATUS_DESTROYED,						///< has been destroyed, pending delete
	OBJECT_STATUS_CAN_ATTACK,						///< used by garrissoned buildings, is OR'ed with KINDOF_CAN_ATTACK in isAbleToAttack()
	OBJECT_STATUS_UNDER_CONSTRUCTION,		///< object is being constructed and is not yet complete
	OBJECT_STATUS_UNSELECTABLE,					///< This is a negative condition since these statuses are overrides.  ie their presence forces the condition, but their absence means nothing
	OBJECT_STATUS_NO_COLLISIONS,				///< object should be ignored for object-object collisions (but not object-ground); used for thing like collapsing parachutes that are intangible
	OBJECT_STATUS_NO_ATTACK,						///< Absolute override to being able to attack
	OBJECT_STATUS_AIRBORNE_TARGET,			///< InTheAir as far as AntiAir weapons are concerned only.
	OBJECT_STATUS_PARACHUTING,					///< object is on a parachute
	OBJECT_STATUS_REPULSOR,							///< object repulses "KINDOF_CAN_BE_REPULSED" objects.
	OBJECT_STATUS_HIJACKED,  						///< unit is in the possesion of an enemy criminal, call the authorities
	OBJECT_STATUS_AFLAME,								///< This object is on fire.
	OBJECT_STATUS_BURNED,								///< This object has already burned as much as it can.
	OBJECT_STATUS_WET,									///< object has been soaked with water
	OBJECT_STATUS_IS_FIRING_WEAPON,			///< Object is firing a weapon, now. Not true for special attacks. --Lorenzen
	OBJECT_STATUS_BRAKING,							///< Object is braking, and subverts the physics.
	OBJECT_STATUS_STEALTHED,						///< Object is currently "stealthed"
	OBJECT_STATUS_DETECTED,							///< Object is in range of a stealth-detector unit (meaningless if STEALTHED not set)
	OBJECT_STATUS_CAN_STEALTH,					///< Object has ability to stealth allowing the stealth update module to run.
	OBJECT_STATUS_SOLD,									///< Object is being sold
	OBJECT_STATUS_UNDERGOING_REPAIR,		///< Object is awaiting/undergoing a repair order that has been issued
	OBJECT_STATUS_RECONSTRUCTING,				///< Reconstructing
	OBJECT_STATUS_MASKED,								///< Masked objects are not selectable and targetable by players or AI
	OBJECT_STATUS_IS_ATTACKING,					///< Object is in the general Attack state (incl. aim, approach, etc.). Note that IS_FIRING_WEAPON and IS_AIMING_WEAPON is a subset of this!
	OBJECT_STATUS_IS_USING_ABILITY,			///< Object is in the process of preparing or firing a special ability.
	OBJECT_STATUS_IS_AIMING_WEAPON,			///< Object is aiming a weapon, now. Not true for special attacks. 
	OBJECT_STATUS_NO_ATTACK_FROM_AI,		///< attacking this object may not be done from commandSource == CMD_FROM_AI
	OBJECT_STATUS_IGNORING_STEALTH,			///< temporarily ignoring all stealth bits. (used only for some special-case mine clearing stuff.)
	OBJECT_STATUS_IS_CARBOMB,						///< Object is now a carbomb.
	// add more status types here and don't forget to add to the string table ObjectStatusMaskType::s_bitNameList[]

	OBJECT_STATUS_COUNT

};

typedef BitFlags<OBJECT_STATUS_COUNT>	ObjectStatusMaskType;

#define MAKE_OBJECT_STATUS_MASK(k) ObjectStatusMaskType(ObjectStatusMaskType::kInit, (k))
#define MAKE_OBJECT_STATUS_MASK2(k,a) ObjectStatusMaskType(ObjectStatusMaskType::kInit, (k), (a))
#define MAKE_OBJECT_STATUS_MASK3(k,a,b) ObjectStatusMaskType(ObjectStatusMaskType::kInit, (k), (a), (b))
#define MAKE_OBJECT_STATUS_MASK4(k,a,b,c) ObjectStatusMaskType(ObjectStatusMaskType::kInit, (k), (a), (b), (c))
#define MAKE_OBJECT_STATUS_MASK5(k,a,b,c,d) ObjectStatusMaskType(ObjectStatusMaskType::kInit, (k), (a), (b), (c), (d))

inline Bool TEST_OBJECT_STATUS_MASK( const ObjectStatusMaskType& m, ObjectStatusTypes t ) 
{ 
	return m.test( t ); 
}

inline Bool TEST_OBJECT_STATUS_MASK_ANY( const ObjectStatusMaskType& m, const ObjectStatusMaskType& mask ) 
{ 
	return m.anyIntersectionWith( mask );
}

inline Bool TEST_OBJECT_STATUS_MASK_MULTI( const ObjectStatusMaskType& m, const ObjectStatusMaskType& mustBeSet, const ObjectStatusMaskType& mustBeClear )
{
	return m.testSetAndClear( mustBeSet, mustBeClear );
}

inline Bool OBJECT_STATUS_MASK_ANY_SET( const ObjectStatusMaskType& m) 
{ 
	return m.any(); 
}

inline void CLEAR_OBJECT_STATUS_MASK( ObjectStatusMaskType& m ) 
{ 
	m.clear(); 
}

inline void SET_ALL_OBJECT_STATUS_MASK_BITS( ObjectStatusMaskType& m )
{
	m.clear( );
	m.flip( );
}

inline void FLIP_OBJECT_STATUS_MASK( ObjectStatusMaskType& m )
{
	m.flip();
}

// defined in Common/System/ObjectStatusTypes.cpp
extern ObjectStatusMaskType OBJECT_STATUS_MASK_NONE;	// inits to all zeroes

#endif /* __OBJECT_STATUS_TYPES_H */
