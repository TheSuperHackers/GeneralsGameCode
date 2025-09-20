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

// FILE: ShieldBody.h //////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:	 Structure bodies are active bodies specifically for structures that are built
//				 and/or interactable (is that a world) with the player.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ShieldBody_H_
#define __ShieldBody_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/ActiveBody.h"
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;
class EnergyShieldBehavior;

//-------------------------------------------------------------------------------------------------
class ShieldBodyModuleData : public ActiveBodyModuleData
{
public:

	Bool m_startsActive;  ///< Initially active without upgrade;
	Real m_shieldMaxHealth;  ///< MaxHealth of the shield
	Real m_shieldMaxHealthPercent;  ///< MaxHealth as percentage of activeBody MaxHealth (takes priority)
	ArmorSetType m_shieldArmorSetFlag; ///< armorset to use for damage absorbed by the shield

	DamageTypeFlags m_damageTypesToPassThrough;
	DamageTypeFlags m_defaultDamageTypesToPassThrough;

	ShieldBodyModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

	static void parseShieldHealthPercent(INI* ini, void* instance, void* store, const void* userData);
};

//-------------------------------------------------------------------------------------------------
/** Structure body module */
//-------------------------------------------------------------------------------------------------
class ShieldBody : public ActiveBody //, public UpdateModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ShieldBody, "ShieldBody" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ShieldBody, ShieldBodyModuleData )

public:

	ShieldBody( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	//static Int getInterfaceMask() { return UpdateModule::getInterfaceMask(); }

	//// UpdateModuleInterface
	//virtual UpdateSleepTime update(void);

	//virtual UpdateModuleInterface* getUpdate() { return this; }

	//virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL;  }


	//void setConstructorObject( Object *obj );
	//ObjectID getConstructorObjectID( void ) { return m_constructorObjectID; }

	inline Real getShieldMaxHealth() const { return getShieldBodyModuleData()->m_shieldMaxHealth; }
	inline Real getShieldCurrentHealth() const { return m_currentShieldHealth; }
	Bool rechargeShieldHealth(Real amount);  ///< returns True if on full health;

	inline Bool isActive() const { return m_active; }
	inline void setActive(Bool value) { m_active = value; }

	Real getShieldPercent();

protected:

	virtual void attemptDamage(DamageInfo* damageInfo);		///< try to damage this object
	virtual void doDamageFX(const DamageInfo* damageInfo);

	virtual void onDisabledEdge(Bool nowDisabled);

	//ObjectID m_constructorObjectID;					///< object that built this structure

private:
	Real									m_currentShieldHealth;

	EnergyShieldBehaviorInterface* m_shieldBehaviorModule;

	Bool m_active;

	void enableShieldEffects();  // when the shield hp is < 0
	void disableShieldEffects();  // when the shield hp is 0

	void findShieldBehaviorModule();
};

#endif // __ShieldBody_H_

