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

// FILE: Damage.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Damage description
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DAMAGE_H_
#define __DAMAGE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameType.h"
#include "Common/Snapshot.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;
class INI;

//-------------------------------------------------------------------------------------------------
/** Damage types, keep this in sync with TheDamageNames[] */
//-------------------------------------------------------------------------------------------------
enum DamageType CPP_11(: Int)
{	
	DAMAGE_EXPLOSION							= 0,			
	DAMAGE_CRUSH									= 1,					
	DAMAGE_ARMOR_PIERCING					= 2,
	DAMAGE_SMALL_ARMS							= 3,		
	DAMAGE_GATTLING								= 4,				
	DAMAGE_RADIATION							= 5,			
	DAMAGE_FLAME									= 6,					
	DAMAGE_LASER									= 7,					
	DAMAGE_SNIPER									= 8,				
	DAMAGE_POISON									= 9,			
	DAMAGE_HEALING								= 10,	
	DAMAGE_UNRESISTABLE						= 11,		// this is for scripting to cause 'armorproof' damage
	DAMAGE_WATER									= 12,	
	DAMAGE_DEPLOY									= 13,					// for transports to deploy units and order them to all attack.
// this stays, even if ALLOW_SURRENDER is not defed, since flashbangs still use 'em
	DAMAGE_SURRENDER							= 14,				// if something "dies" to surrender damage, they surrender.... duh!
	DAMAGE_HACK										= 15,
	DAMAGE_KILLPILOT							= 16,				// special snipe attack that kills the pilot and renders a vehicle unmanned.
	DAMAGE_PENALTY								= 17,					// from game penalty (you won't receive radar warnings BTW)
	DAMAGE_FALLING								= 18,
	DAMAGE_MELEE									= 19,						// Blades, clubs...
	DAMAGE_DISARM									= 20,	// "special" damage type used for disarming mines, bombs, etc (NOT for "disarming" an opponent!)
	DAMAGE_HAZARD_CLEANUP					= 21,	// special damage type for cleaning up hazards like radiation or bio-poison.
	DAMAGE_PARTICLE_BEAM					= 22,	// Incinerates virtually everything (insanely powerful orbital beam)
	DAMAGE_TOPPLING								= 23,	// damage from getting toppled.
	DAMAGE_INFANTRY_MISSILE				= 24,	
	DAMAGE_AURORA_BOMB						= 25,	
	DAMAGE_LAND_MINE							= 26,	
	DAMAGE_JET_MISSILES						= 27,	
	DAMAGE_STEALTHJET_MISSILES		= 28,	
	DAMAGE_MOLOTOV_COCKTAIL				= 29,	
	DAMAGE_COMANCHE_VULCAN				= 30,	
	DAMAGE_FLESHY_SNIPER					= 31,		// like DAMAGE_SNIPER, but (generally) does no damage to vehicles.

	// Please note: There is a string array below this enum, and when you change them,
	// you need to search on the array names to find all the stuff that generates names
	// based on these strings.  (eg DamageFX does a strcat to make its array of names so 
	// change DamageFX.ini and its Default)


	// !!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	// if you add additional damage types, you will PROBABLY HAVE TO
	// ENLARGE A BITMASK IN WEAPONSET.
	//
	// !!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	DAMAGE_NUM_TYPES			// keep this last
};

#ifdef DEFINE_DAMAGE_NAMES
static const char *TheDamageNames[] = 
{
	"EXPLOSION",			
	"CRUSH",					
	"ARMOR_PIERCING",
	"SMALL_ARMS",		
	"GATTLING",			
	"RADIATION",			
	"FLAME",					
	"LASER",					
	"SNIPER",				
	"POISON",			
	"HEALING",	
	"UNRESISTABLE",	
	"WATER",
	"DEPLOY",	
	"SURRENDER",	
	"HACK",	
	"KILL_PILOT",	
	"PENALTY",	
	"FALLING",	
	"MELEE",	
	"DISARM",	
	"HAZARD_CLEANUP",	
	"PARTICLE_BEAM",
	"TOPPLING",
	"INFANTRY_MISSILE",	
	"AURORA_BOMB",	
	"LAND_MINE",	
	"JET_MISSILES",	
	"STEALTHJET_MISSILES",	
	"MOLOTOV_COCKTAIL",	
	"COMANCHE_VULCAN",	
	"FLESHY_SNIPER",	

	NULL
};
#endif // end DEFINE_DAMAGE_NAMES


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

typedef UnsignedInt DamageTypeFlags;

const DamageTypeFlags DAMAGE_TYPE_FLAGS_ALL = 0xffffffff;
const DamageTypeFlags DAMAGE_TYPE_FLAGS_NONE = 0x00000000;

inline Bool getDamageTypeFlag(DamageTypeFlags flags, DamageType dt)
{
	return (flags & (1UL << (dt - 1))) != 0;
}

inline DamageTypeFlags setDamageTypeFlag(DamageTypeFlags flags, DamageType dt)
{
	return (flags | (1UL << (dt - 1)));
}

inline DamageTypeFlags clearDamageTypeFlag(DamageTypeFlags flags, DamageType dt)
{
	return (flags & ~(1UL << (dt - 1)));
}

//-------------------------------------------------------------------------------------------------
/** Death types, keep this in sync with TheDeathNames[] */
//-------------------------------------------------------------------------------------------------
enum DeathType CPP_11(: Int)
{	
	// note that these DELIBERATELY have (slightly) different names from the damage names,
	// since there isn't necessarily a one-to-one correspondence. e.g., DEATH_BURNED
	// can come from DAMAGE_FLAME but also from DAMAGE_PARTICLE_BEAM.
	DEATH_NORMAL		= 0,
	DEATH_NONE			= 1,					///< this is a "special case" that can't normally cause death			
  DEATH_CRUSHED		= 2,
  DEATH_BURNED		= 3,
  DEATH_EXPLODED	= 4,
	DEATH_POISONED	= 5,
	DEATH_TOPPLED		= 6,
	DEATH_FLOODED		= 7,
	DEATH_SUICIDED	= 8,
	DEATH_LASERED		= 9,
	DEATH_DETONATED	= 10,		/**< this is the "death" that occurs when a missile/warhead/etc detonates normally,
														as opposed to being shot down, etc */
	DEATH_SPLATTED	= 11,		/**< the death that results from DAMAGE_FALLING */
	DEATH_POISONED_BETA	= 12,	

	// these are the "extra" types for yet-to-be-defined stuff. Don't bother renaming or adding
	// or removing these; they are reserved for modders :-)
	DEATH_EXTRA_2		= 13,	
	DEATH_EXTRA_3		= 14,	
	DEATH_EXTRA_4		= 15,	
	DEATH_EXTRA_5		= 16,	
	DEATH_EXTRA_6		= 17,	
	DEATH_EXTRA_7		= 18,	
	DEATH_EXTRA_8		= 19,	
	
	DEATH_NUM_TYPES			// keep this last
};

#ifdef DEFINE_DEATH_NAMES
static const char *TheDeathNames[] = 
{
	"NORMAL",			
	"NONE",			
	"CRUSHED",					
	"BURNED",		
	"EXPLODED",
	"POISONED",
	"TOPPLED",
	"FLOODED",
	"SUICIDED",
	"LASERED",
	"DETONATED",
	"SPLATTED",
	"POISONED_BETA",	

	"EXTRA_2",	
	"EXTRA_3",	
	"EXTRA_4",	
	"EXTRA_5",	
	"EXTRA_6",	
	"EXTRA_7",	
	"EXTRA_8",	

	NULL
};
#endif // end DEFINE_DEATH_NAMES


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

typedef UnsignedInt DeathTypeFlags;

const DeathTypeFlags DEATH_TYPE_FLAGS_ALL = 0xffffffff;
const DeathTypeFlags DEATH_TYPE_FLAGS_NONE = 0x00000000;

inline Bool getDeathTypeFlag(DeathTypeFlags flags, DeathType dt)
{
	return (flags & (1UL << (dt - 1))) != 0;
}

inline DeathTypeFlags setDeathTypeFlag(DeathTypeFlags flags, DeathType dt)
{
	return (flags | (1UL << (dt - 1)));
}

inline DeathTypeFlags clearDeathTypeFlag(DeathTypeFlags flags, DeathType dt)
{
	return (flags & ~(1UL << (dt - 1)));
}

//-------------------------------------------------------------------------------------------------
/** Damage info inputs */
//-------------------------------------------------------------------------------------------------
class DamageInfoInput : public Snapshot
{

public:

	DamageInfoInput( void ) 
	{ 
		m_sourceID = INVALID_ID; 
		m_sourcePlayerMask = 0;
		m_damageType = DAMAGE_EXPLOSION; 
		m_deathType = DEATH_NORMAL; 
		m_amount = 0; 
	}

	ObjectID		   m_sourceID;							///< source of the damage
	PlayerMaskType m_sourcePlayerMask;			///< Player mask of m_sourceID.
	DamageType		 m_damageType;						///< type of damage
	DeathType			 m_deathType;						///< if this kills us, death type to be used
	Real					 m_amount;								///< # value of how much damage to inflict

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void ) { }
	
};

const Real HUGE_DAMAGE_AMOUNT = 999999.0f;

//-------------------------------------------------------------------------------------------------
/** Damage into outputs */
//-------------------------------------------------------------------------------------------------
class DamageInfoOutput : public Snapshot
{

public:

	DamageInfoOutput( void ) 
	{ 
		m_actualDamageDealt = 0; 
		m_actualDamageClipped = 0; 
		m_noEffect = false;
	}

	/** 
		m_actualDamageDealt is the damage we tried to apply to object (after multipliers and such). 
		m_actualDamageClipped is the value of m_actualDamageDealt, but clipped to the max health remaining of the obj.
		example:
			a mammoth tank fires a round at a small tank, attempting 100 damage.
			the small tank has a damage multiplier of 50%, meaning that only 50 damage is applied.
			furthermore, the small tank has only 30 health remaining.
			so: m_actualDamageDealt = 50, m_actualDamageClipped = 30.

		this distinction is useful, since visual fx really wants to do the fx for "50 damage", 
		even though it was more than necessary to kill this object; game logic, on the other hand,
		may want to know the "clipped" damage for ai purposes.
	*/
	Real m_actualDamageDealt;
	Real m_actualDamageClipped;			///< (see comment for m_actualDamageDealt)
	Bool m_noEffect;								///< if true, no damage was done at all (generally due to being InactiveBody)

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void ) { }

};

//-------------------------------------------------------------------------------------------------
/** DamageInfo is a descriptor of damage we're trying to inflict.  The structure
	* is divided up into two parts, inputs and outputs.
	*
	* INPUTS: You must provide valid values for these fields in order for damage
	*					calculation to correctly take place
	* OUTPUT: Upon returning from damage issuing functions, the output fields
	*					will be filled with the results of the damage occurrence
	*/
//-------------------------------------------------------------------------------------------------
class DamageInfo : public Snapshot
{

public:

	DamageInfoInput		in;					///< inputs for the damage info
	DamageInfoOutput	out;				///< results for the damage occurrence

protected:

	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void ){ }

};

#endif // __DAMAGE_H_

