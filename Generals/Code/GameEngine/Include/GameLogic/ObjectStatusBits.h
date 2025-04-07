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

// ObjectStatusBits.h /////////////////////////////////////////////////////////////////////////////
// Part of header detangling
// JKMCD Aug 2002

#pragma once
#ifndef __OBJECTSTATUSBITS_H__
#define __OBJECTSTATUSBITS_H__

//-------------------------------------------------------------------------------------------------
/** Object status bits */
//-------------------------------------------------------------------------------------------------
enum ObjectStatusBits
{
	OBJECT_STATUS_NONE									= 0,					///< no status bit
	OBJECT_STATUS_DESTROYED							=	(1 << 0),		///< has been destroyed, pending delete
	OBJECT_STATUS_CAN_ATTACK						= (1 << 1),		///< used by garrissoned buildings, is OR'ed with KINDOF_CAN_ATTACK in isAbleToAttack()
	OBJECT_STATUS_UNDER_CONSTRUCTION		= (1 << 2),		///< object is being constructed and is not yet complete
	OBJECT_STATUS_UNSELECTABLE					= (1 << 3),		///< This is a negative condition since these statuses are overrides.  ie their presence forces the condition, but their absence means nothing
	OBJECT_STATUS_NO_COLLISIONS					= (1 << 4),		///< object should be ignored for object-object collisions (but not object-ground); used for thing like collapsing parachutes that are intangible
	OBJECT_STATUS_NO_ATTACK							= (1 << 5),		///< Absolute override to being able to attack
	OBJECT_STATUS_AIRBORNE_TARGET				= (1 << 6),		///< InTheAir as far as AntiAir weapons are concerned only.
	OBJECT_STATUS_PARACHUTING						= (1 << 7),		///< object is on a parachute
	OBJECT_STATUS_REPULSOR							= (1 << 8),		///< object repulses "KINDOF_CAN_BE_REPULSED" objects.
	OBJECT_STATUS_HIJACKED  						= (1 << 9),		///< unit is in the possesion of an enemy criminal, call the authorities
	OBJECT_STATUS_AFLAME								= (1 << 10),	///< This object is on fire.
	OBJECT_STATUS_BURNED								= (1 << 11),	///< This object has already burned as much as it can.
	OBJECT_STATUS_WET										= (1 << 12),	///< object has been soaked with water
	OBJECT_STATUS_IS_FIRING_WEAPON			= (1 << 13),  ///< Object is firing a weapon, now. Not true for special attacks. --Lorenzen
	OBJECT_STATUS_BRAKING								= (1 << 14),	///< Object is braking, and subverts the physics.
	OBJECT_STATUS_STEALTHED							= (1 << 15),	///< Object is currently "stealthed"
	OBJECT_STATUS_DETECTED							= (1 << 16),	///< Object is in range of a stealth-detector unit (meaningless if STEALTHED not set)
	OBJECT_STATUS_CAN_STEALTH						= (1 << 17),	///< Object has ability to stealth allowing the stealth update module to run.
	OBJECT_STATUS_SOLD									= (1 << 18),	///< Object is being sold
	OBJECT_STATUS_UNDERGOING_REPAIR			= (1 << 19),	///< Object is awaiting/undergoing a repair order that has been issued
	OBJECT_STATUS_RECONSTRUCTING				= (1 << 20),	///< Reconstructing
	OBJECT_STATUS_MASKED								= (1 << 21),	///< Masked objects are not selectable and targetable by players or AI
	OBJECT_STATUS_IS_ATTACKING					= (1 << 22),  ///< Object is in the general Attack state (incl. aim, approach, etc.). Note that IS_FIRING_WEAPON and IS_AIMING_WEAPON is a subset of this!
	OBJECT_STATUS_IS_USING_ABILITY			= (1 << 23),	///< Object is in the process of preparing or firing a special ability.
	OBJECT_STATUS_IS_AIMING_WEAPON			= (1 << 24),  ///< Object is aiming a weapon, now. Not true for special attacks. 
	OBJECT_STATUS_NO_ATTACK_FROM_AI			= (1 << 25),  ///< attacking this object may not be done from commandSource == CMD_FROM_AI
	OBJECT_STATUS_IGNORING_STEALTH			= (1 << 26),	///< temporarily ignoring all stealth bits. (used only for some special-case mine clearing stuff.)
	OBJECT_STATUS_IS_CARBOMB						= (1 << 27),  ///< Object is now a carbomb.

	// add more status bits here ... don't forget to add to the string table below!!!

};

typedef UnsignedInt	ObjectStatusMaskType;

#ifdef DEFINE_OBJECT_STATUS_NAMES
static const char *TheObjectStatusBitNames[] = 
{
	"DESTROYED",
	"CAN_ATTACK",					
	"UNDER_CONSTRUCTION",	
	"UNSELECTABLE",				
	"NO_COLLISIONS",				
	"NO_ATTACK",						
	"AIRBORNE_TARGET",			
	"PARACHUTING",	
	"REPULSOR",
	"HIJACKED",					
	"AFLAME",							
	"BURNED",							
	"WET",
	"IS_FIRING_WEAPON",
	"IS_BRAKING",
	"STEALTHED",
	"DETECTED",
	"CAN_STEALTH",
	"SOLD",
	"UNDERGOING_REPAIR",
	"RECONSTRUCTING",
	"MASKED",
	"IS_ATTACKING",
	"USING_ABILITY",
	"IS_AIMING_WEAPON",
	"NO_ATTACK_FROM_AI",
	"IGNORING_STEALTH",
	"IS_CARBOMB",

	NULL																///< leave this last please
};
#endif

#endif /* __OBJECTSTATUSBITS_H__ */
