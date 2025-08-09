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

// FILE: OverchargeBehavior.h /////////////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   Objects with this behavior module will get the ability to produce more power
//				 for a short amount of time, during this "overcharge" state object health is
//				 slowly reduced
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __OVERCHARGE_BEHAVIOR_H_
#define __OVERCHARGE_BEHAVIOR_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DamageModule.h"
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class OverchargeBehaviorModuleData : public UpdateModuleData
{

public:

	OverchargeBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	Real m_healthPercentToDrainPerSecond;			///< when active, this much health is drained
	Real m_notAllowedWhenHealthBelowPercent;	///< you cannot overcharge when object is below this health %

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class OverchargeBehaviorInterface
{

public:

	virtual void toggle( void ) = 0;
	virtual void enable( Bool enable ) = 0;
	virtual Bool isOverchargeActive( void ) = 0;

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class OverchargeBehavior : public UpdateModule,
													 public DamageModuleInterface,
													 public OverchargeBehaviorInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( OverchargeBehavior, "OverchargeBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( OverchargeBehavior, OverchargeBehaviorModuleData )

public:

	OverchargeBehavior( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// interface housekeeping
	virtual OverchargeBehaviorInterface* getOverchargeBehaviorInterface() { return this; }
	static Int getInterfaceMask( void ) { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DAMAGE); }

	// BehaviorModule
	virtual DamageModuleInterface* getDamage( void ) { return this; }

	// UpdateModuleInterface
	virtual UpdateSleepTime update( void );

	// DamageModuleInterface
	virtual void onDamage( DamageInfo *damageInfo );
	virtual void onHealing( DamageInfo *damageInfo ) { }
	virtual void onBodyDamageStateChange( const DamageInfo *damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState ) { }


	// specific methods
	virtual void toggle( void );						///< toggle overcharge on/off
	virtual void enable( Bool enable );			///< turn overcharge on/off
	virtual Bool isOverchargeActive( void ) { return m_overchargeActive; }

	void onDelete( void );																///< we have some work to do when this module goes away
	void onCapture( Player *oldOwner, Player *newOwner );	///< object containing upgrade has changed teams

protected:

	Bool m_overchargeActive;				///< Overcharge is currently on/off for this object

};

#endif  // end __OVERCHARGE_BEHAVIOR_H_
