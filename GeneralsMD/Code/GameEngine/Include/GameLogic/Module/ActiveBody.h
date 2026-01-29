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

// FILE: ActiveBody.h /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Active bodies have health, they can die and are affected by health
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/DamageFX.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Armor.h"
#include "GameLogic/ArmorSet.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class BodyParticleSystem;
class ParticleSystemTemplate;

//-------------------------------------------------------------------------------------------------
/** Active body module */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ActiveBodyModuleData : public BodyModuleData
{
public:
	Real m_maxHealth;
	Real m_initialHealth;

	Real m_subdualDamageCap;								///< Subdual damage will never accumulate past this
	UnsignedInt m_subdualDamageHealRate;		///< Every this often, we drop subdual damage...
	Real m_subdualDamageHealAmount;					///< by this much.

	ActiveBodyModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class ActiveBody : public BodyModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ActiveBody, "ActiveBody" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ActiveBody, ActiveBodyModuleData )

public:

	ActiveBody( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDelete( void );

	virtual void attemptDamage( DamageInfo *damageInfo );		///< try to damage this object
	virtual Real estimateDamage( DamageInfoInput& damageInfo ) const;
	virtual void attemptHealing( DamageInfo *damageInfo );		///< try to heal this object
	virtual Real getHealth() const;													///< get current health
	virtual BodyDamageType getDamageState() const;
	virtual void setDamageState( BodyDamageType newState );	///< control damage state directly.  Will adjust hitpoints.
	virtual void setAflame( Bool setting );///< This is a major change like a damage state.
	virtual UnsignedInt getSubdualDamageHealRate() const;
	virtual Real getSubdualDamageHealAmount() const;
	virtual Bool hasAnySubdualDamage() const;
	virtual Real getCurrentSubdualDamageAmount() const { return m_currentSubdualDamage; }

	virtual const DamageInfo *getLastDamageInfo() const { return &m_lastDamageInfo; }	///< return info on last damage dealt to this object
	virtual UnsignedInt getLastDamageTimestamp() const { return m_lastDamageTimestamp; }	///< return frame of last damage dealt
	virtual UnsignedInt getLastHealingTimestamp() const { return m_lastHealingTimestamp; }	///< return frame of last damage dealt
	virtual ObjectID getClearableLastAttacker() const { return (m_lastDamageCleared ? INVALID_ID : m_lastDamageInfo.in.m_sourceID); }
	virtual void clearLastAttacker() { m_lastDamageCleared = true; }

	void onVeterancyLevelChanged( VeterancyLevel oldLevel, VeterancyLevel newLevel, Bool provideFeedback = TRUE );

	virtual void setArmorSetFlag(ArmorSetType ast) { m_curArmorSetFlags.set(ast, 1); }
	virtual void clearArmorSetFlag(ArmorSetType ast) { m_curArmorSetFlags.set(ast, 0); }
	virtual Bool testArmorSetFlag(ArmorSetType ast) { return m_curArmorSetFlags.test(ast); }

	virtual void setInitialHealth(Int initialPercent); ///< Sets the initial load health %.
	virtual void setMaxHealth( Real maxHealth, MaxHealthChangeType healthChangeType = SAME_CURRENTHEALTH ); ///< Sets the initial max health

	virtual Bool getFrontCrushed() const { return m_frontCrushed; }
	virtual Bool getBackCrushed() const { return m_backCrushed; }

	virtual void setFrontCrushed(Bool v) { m_frontCrushed = v; }
	virtual void setBackCrushed(Bool v) { m_backCrushed = v; }

	virtual Real getMaxHealth() const;  ///< return max health
	virtual Real getInitialHealth() const;  // return initial health

	virtual Real getPreviousHealth() const { return m_prevHealth; }

	virtual void setIndestructible( Bool indestructible );
	virtual Bool isIndestructible( void ) const { return m_indestructible; }

	virtual void internalChangeHealth( Real delta );								///< change health

	virtual void evaluateVisualCondition();
	virtual void updateBodyParticleSystems( void );// made public for topple anf building collapse updates -ML

	// Subdual Damage
	virtual Bool isSubdued() const;
	virtual Bool canBeSubdued() const;
	virtual void onSubdualChange( Bool isNowSubdued );///< Override this if you want a totally different effect than DISABLED_SUBDUED

protected:

	void validateArmorAndDamageFX() const;
	void doDamageFX( const DamageInfo *damageInfo );

	void createParticleSystems( const AsciiString &boneBaseName,
															const ParticleSystemTemplate *systemTemplate,
															Int maxSystems );
	void deleteAllParticleSystems( void );
	void setCorrectDamageState();

	Bool shouldRetaliate(Object *obj);
	Bool shouldRetaliateAgainstAggressor(Object *obj, Object *damager);

	virtual void internalAddSubdualDamage( Real delta );								///< change health

private:

	Real									m_currentHealth;				///< health of the object
	Real									m_prevHealth;						///< previous health value before current health change op
  Real									m_maxHealth;						///< max health this object can have
  Real									m_initialHealth;				///< starting health for this object
	Real									m_currentSubdualDamage;	///< Starts at zero and goes up.  Inherited modules will do something when "subdued".

	BodyDamageType				m_curDamageState;				///< last known damage state
	UnsignedInt						m_nextDamageFXTime;
	DamageType						m_lastDamageFXDone;
	DamageInfo						m_lastDamageInfo;				///< store the last DamageInfo object that we received
	UnsignedInt						m_lastDamageTimestamp; 	///< frame of last damage dealt
	UnsignedInt						m_lastHealingTimestamp; ///< frame of last healing dealt
	Bool									m_frontCrushed;
	Bool									m_backCrushed;
	Bool									m_lastDamageCleared;
	Bool									m_indestructible;				///< is this object indestructible?

	BodyParticleSystem *m_particleSystems;				///< particle systems created and attached to this object

	/*
		Note, you MUST call validateArmorAndDamageFX() before accessing these fields.
	*/
	ArmorSetFlags											m_curArmorSetFlags;
	mutable const ArmorTemplateSet*		m_curArmorSet;
	mutable Armor											m_curArmor;
	mutable const DamageFX*						m_curDamageFX;

};
