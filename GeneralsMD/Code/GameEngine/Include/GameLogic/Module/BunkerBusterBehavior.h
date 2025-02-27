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

// FILE: BunkerBusterBehavior.h ////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, June 2003
// Desc:   Behavior module for Bunker Buster... it kills garrisoned objects
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameClient/FXList.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class FXList;
class UpgradeTemplate;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BunkerBusterBehaviorModuleData : //public UpdateModuleData
                                       public BehaviorModuleData
{

public:

	BunkerBusterBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	AsciiString m_upgradeRequired;///< Upgrade required to kill garrisoned units
	const FXList *m_detonationFX;						///< FXList to play upon detonation
  const FXList *m_crashThroughBunkerFX;  ///< FXList to play as the bomb goes smashing through the bunker
  UnsignedInt m_crashThroughBunkerFXFrequency;  ///< How often to play the above FX

  Real m_seismicEffectRadius;
  Real m_seismicEffectMagnitude;

  WeaponTemplate *m_shockwaveWeaponTemplate; ///< a weapon that gets fired when the buster gets busted, intended to generate a shockwave effect only
  WeaponTemplate *m_occupantDamageWeaponTemplate; ///< a weapon to provide a damegeInfo that gets applied to each occupant of container struck as they are getting kicked out


};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BunkerBusterBehavior : public UpdateModule,
 														 public DieModuleInterface
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BunkerBusterBehavior, BunkerBusterBehaviorModuleData );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BunkerBusterBehavior, "BunkerBusterBehavior" )

public:

	BunkerBusterBehavior( Thing *thing, const ModuleData *modData );
	// virtual destructor prototype provided by MemoryPoolObject

	// module methods
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// update module methods
	virtual UpdateSleepTime update( void );
  virtual void onObjectCreated( void );

	// die module methods
	virtual DieModuleInterface *getDie( void ) { return this; }
	virtual void onDie( const DamageInfo *damageInfo );



protected:
  void bustTheBunker( void );

	const UpgradeTemplate *m_upgradeRequired;			///< Upgrade required to use the upgraded pulse FX

  ObjectID m_victimID;
};
