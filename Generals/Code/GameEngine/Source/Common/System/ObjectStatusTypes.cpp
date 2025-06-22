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

// FILE: ObjectStatusTypes.cpp ////////////////////////////////////////////////////////////////////
// Author: Kris, May 2003
// Desc:   Object status types that are stackable using the BitSet system. Used to be ObjectStatusBits
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"

#include "Common/ObjectStatusTypes.h"
#include "Common/BitFlagsIO.h"

template<>
const char* ObjectStatusMaskType::s_bitNameList[] = 
{
	"NONE",
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
	NULL
};

ObjectStatusMaskType OBJECT_STATUS_MASK_NONE;	// inits to all zeroes