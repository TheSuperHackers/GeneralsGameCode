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

// FILE: BodyModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BODYMODULE_H_
#define __BODYMODULE_H_

#include "Common/Module.h"
#include "GameLogic/Damage.h"
#include "GameLogic/ArmorSet.h"
#include "GameLogic/Module/BehaviorModule.h"

//-------------------------------------------------------------------------------------------------
/** OBJECT BODY MODULE base class */
//-------------------------------------------------------------------------------------------------

class WeaponTemplate;

//-------------------------------------------------------------------------------------------------
/** Damage states for structures
	*
	* NOTE: the macros below for IS_CONDITION_WORSE and IS_CONDITION_BETTER depend on this
	* enumeration being in sequential order
	*/
//-------------------------------------------------------------------------------------------------
enum BodyDamageType CPP_11(: Int)
{
	BODY_PRISTINE,				///< unit should appear in pristine condition
	BODY_DAMAGED,					///< unit has been damaged
	BODY_REALLYDAMAGED,		///< unit is extremely damaged / nearly destroyed
	BODY_RUBBLE,					///< unit has been reduced to rubble/corpse/exploded-hulk, etc

	BODYDAMAGETYPE_COUNT
};

#ifdef DEFINE_BODYDAMAGETYPE_NAMES
static const char* const TheBodyDamageTypeNames[] =
{
	"PRISTINE",
	"DAMAGED",
	"REALLYDAMAGED",
	"RUBBLE",

	NULL
};
static_assert(ARRAY_SIZE(TheBodyDamageTypeNames) == BODYDAMAGETYPE_COUNT + 1, "Incorrect array size");
#endif

enum MaxHealthChangeType CPP_11(: Int)
{
	SAME_CURRENTHEALTH,
	PRESERVE_RATIO,
	ADD_CURRENT_HEALTH_TOO,
	FULLY_HEAL,

	MAX_HEALTH_CHANGE_COUNT
};

#ifdef DEFINE_MAXHEALTHCHANGETYPE_NAMES
static const char* const TheMaxHealthChangeTypeNames[] =
{
	"SAME_CURRENTHEALTH",
	"PRESERVE_RATIO",
	"ADD_CURRENT_HEALTH_TOO",
	"FULLY_HEAL",
	NULL
};
static_assert(ARRAY_SIZE(TheMaxHealthChangeTypeNames) == MAX_HEALTH_CHANGE_COUNT + 1, "Incorrect array size");
#endif


//
// is condition A worse than condition B  ... NOTE: this assumes the conditions
// in BodyDamageType are in sequential order
//
// is a worse than b
#define IS_CONDITION_WORSE( a, b ) ( a > b )
// is a better than b
#define IS_CONDITION_BETTER( a, b ) ( a < b )

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class BodyModuleData : public BehaviorModuleData
{
public:
	BodyModuleData()
	{
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    BehaviorModuleData::buildFieldParse(p);
	}
};

//-------------------------------------------------------------------------------------------------
class BodyModuleInterface
{
public:
	/**
		Try to damage this Object. The module's Armor
		will be taken into account, so the actual damage done may vary
		considerably from what you requested. Also note that (if damage is done)
		the DamageFX will be invoked to provide a/v fx as appropriate.
	*/
	virtual void attemptDamage( DamageInfo *damageInfo ) = 0;

	/**
		Instead of having negative damage count as healing, or allowing access to the private
		changeHealth Method, we will use this parallel to attemptDamage to do healing without hack.
	*/
	virtual void attemptHealing( DamageInfo *healingInfo ) = 0;

	/**
		Estimate the (unclipped) damage that would be done to this object
		by the given damage (taking bonuses, armor, etc into account),
		but DO NOT alter the body in any way. (This is used by the AI system
		to choose weapons.)
	*/
	virtual Real estimateDamage( DamageInfoInput& damageInfo ) const = 0;

	virtual Real getHealth() const = 0;													///< get current health

	virtual Real getMaxHealth() const = 0;

	virtual Real getInitialHealth() const = 0;

	virtual Real getPreviousHealth() const = 0;

	virtual UnsignedInt getSubdualDamageHealRate() const = 0;
	virtual Real getSubdualDamageHealAmount() const = 0;
	virtual Bool hasAnySubdualDamage() const = 0;
	virtual Real getCurrentSubdualDamageAmount() const = 0;

	virtual BodyDamageType getDamageState() const = 0;
	virtual void setDamageState( BodyDamageType newState ) = 0;	///< control damage state directly.  Will adjust hitpoints.
	virtual void setAflame( Bool setting ) = 0;///< This is a major change like a damage state.

	virtual void onVeterancyLevelChanged( VeterancyLevel oldLevel, VeterancyLevel newLevel, Bool provideFeedback ) = 0;	///< I just achieved this level right this moment

	virtual void setArmorSetFlag(ArmorSetType ast) = 0;
	virtual void clearArmorSetFlag(ArmorSetType ast) = 0;
	virtual Bool testArmorSetFlag(ArmorSetType ast) = 0;

	virtual const DamageInfo *getLastDamageInfo() const = 0;
	virtual UnsignedInt getLastDamageTimestamp() const = 0;
	virtual UnsignedInt getLastHealingTimestamp() const = 0;
	virtual ObjectID getClearableLastAttacker() const = 0;
	virtual void clearLastAttacker() = 0;
	virtual Bool getFrontCrushed() const = 0;
	virtual Bool getBackCrushed() const = 0;

	virtual void setInitialHealth(Int initialPercent)  = 0;
	virtual void setMaxHealth( Real maxHealth, MaxHealthChangeType healthChangeType = SAME_CURRENTHEALTH )  = 0;

	virtual void setFrontCrushed(Bool v) = 0;
	virtual void setBackCrushed(Bool v) = 0;

	virtual void applyDamageScalar( Real scalar ) = 0;
	virtual Real getDamageScalar() const = 0;

	/**
		Change the module's health by the given delta. Note that
		the module's DamageFX and Armor are NOT taken into
		account, so you should think about what you're bypassing when you
		call this directly (especially when when decreasing health, since
		you probably want "attemptDamage" or "attemptHealing")
	*/
	virtual void internalChangeHealth( Real delta ) = 0;

	virtual void setIndestructible( Bool indestructible ) = 0;
	virtual Bool isIndestructible( void ) const = 0;

	virtual void evaluateVisualCondition() = 0;
	virtual void updateBodyParticleSystems() = 0; // made public for topple and building collapse updates -ML

};

//-------------------------------------------------------------------------------------------------
class BodyModule : public BehaviorModule, public BodyModuleInterface
{

	MEMORY_POOL_GLUE_ABC( BodyModule )

public:

	BodyModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	static Int getInterfaceMask() { return MODULEINTERFACE_BODY; }

	// BehaviorModule
	virtual BodyModuleInterface* getBody() { return this; }

	/**
		Try to damage this Object. The module's Armor
		will be taken into account, so the actual damage done may vary
		considerably from what you requested. Also note that (if damage is done)
		the DamageFX will be invoked to provide a/v fx as appropriate.
	*/
	virtual void attemptDamage( DamageInfo *damageInfo ) = 0;

	/**
		Instead of having negative damage count as healing, or allowing access to the private
		changeHealth Method, we will use this parallel to attemptDamage to do healing without hack.
	*/
	virtual void attemptHealing( DamageInfo *healingInfo ) = 0;

	/**
		Estimate the (unclipped) damage that would be done to this object
		by the given damage (taking bonuses, armor, etc into account),
		but DO NOT alter the body in any way. (This is used by the AI system
		to choose weapons.)
	*/
	virtual Real estimateDamage( DamageInfoInput& damageInfo ) const = 0;

	virtual Real getHealth() const = 0;													///< get current health

	virtual Real getMaxHealth() const {return 0.0f;}  ///< return max health
	virtual Real getPreviousHealth() const { return 0.0f; } ///< return previous health

	virtual UnsignedInt getSubdualDamageHealRate() const {return 0;}
	virtual Real getSubdualDamageHealAmount() const {return 0.0f;}
	virtual Bool hasAnySubdualDamage() const{return FALSE;}
	virtual Real getCurrentSubdualDamageAmount() const { return 0.0f; }

	virtual Real getInitialHealth() const {return 0.0f;}  // return initial health

	virtual BodyDamageType getDamageState() const = 0;
	virtual void setDamageState( BodyDamageType newState ) = 0;	///< control damage state directly.  Will adjust hitpoints.
	virtual void setAflame( Bool setting ) = 0;///< This is a major change like a damage state.

	virtual void onVeterancyLevelChanged( VeterancyLevel oldLevel, VeterancyLevel newLevel, Bool provideFeedback = FALSE ) = 0;	///< I just achieved this level right this moment

	virtual void setArmorSetFlag(ArmorSetType ast) = 0;
	virtual void clearArmorSetFlag(ArmorSetType ast) = 0;
	virtual Bool testArmorSetFlag(ArmorSetType ast) = 0;

	virtual const DamageInfo *getLastDamageInfo() const { return NULL; }	///< return info on last damage dealt to this object
	virtual UnsignedInt getLastDamageTimestamp() const { return 0; }	///< return frame of last damage dealt
	virtual UnsignedInt getLastHealingTimestamp() const { return 0; }	///< return frame of last healing dealt
	virtual ObjectID getClearableLastAttacker() const { return INVALID_ID; }
	virtual void clearLastAttacker() { }
	virtual Bool getFrontCrushed() const { return false; }
	virtual Bool getBackCrushed() const { return false; }

	virtual void setInitialHealth(Int initialPercent)  {  } ///< Sets the inital load health %.
	virtual void setMaxHealth(Real maxHealth, MaxHealthChangeType healthChangeType = SAME_CURRENTHEALTH )  {  } ///< Sets the max health.

	virtual void setFrontCrushed(Bool v) { DEBUG_CRASH(("you should never call this for generic Bodys")); }
	virtual void setBackCrushed(Bool v) { DEBUG_CRASH(("you should never call this for generic Bodys")); }


	virtual void setIndestructible( Bool indestructible ) { }
	virtual Bool isIndestructible( void ) const { return TRUE; }

	//Allows outside systems to apply defensive bonuses or penalties (they all stack as a multiplier!)
	virtual void applyDamageScalar( Real scalar ) { m_damageScalar *= scalar; }
	virtual Real getDamageScalar() const { return m_damageScalar; }

	/**
		Change the module's health by the given delta. Note that
		the module's DamageFX and Armor are NOT taken into
		account, so you should think about what you're bypassing when you
		call this directly (especially when when decreasing health, since
		you probably want "attemptDamage" or "attemptHealing")
	*/
	virtual void internalChangeHealth( Real delta ) = 0;

	virtual void evaluateVisualCondition() { }
	virtual void updateBodyParticleSystems() { };// made public for topple anf building collapse updates -ML

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Real	m_damageScalar;

};
inline BodyModule::BodyModule( Thing *thing, const ModuleData* moduleData ) : BehaviorModule( thing, moduleData ), m_damageScalar(1.0f) { }
inline BodyModule::~BodyModule() { }

#endif
