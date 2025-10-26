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

// FILE: BattleBusSlowDeathBehavior.h ///////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June 2003
// Desc:   Death for the Battle Bus.  Can do a fake slow death before the real one if triggered intentionally
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/SlowDeathBehavior.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class FXList;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class BattleBusSlowDeathBehaviorModuleData : public SlowDeathBehaviorModuleData
{

public:

	BattleBusSlowDeathBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	const FXList *m_fxStartUndeath;								///< fx list executed on start of throw
	const ObjectCreationList *m_oclStartUndeath;	///< ocl list executed on start of throw

	const FXList *m_fxHitGround;									///< FXList for when we land
	const ObjectCreationList *m_oclHitGround;			///< OCL for when we land

	Real m_throwForce;														///< How hard we are thrown in to the air
	Real m_percentDamageToPassengers;							///< At the moment we throw up, how hard the people inside are hit
	UnsignedInt m_emptyHulkDestructionDelay;			///< Another reason this is a BattleBus module, and not a generic two stage death.  If non-zero, time empty before we kill ourselves.

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BattleBusSlowDeathBehavior : public SlowDeathBehavior
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BattleBusSlowDeathBehavior, "BattleBusSlowDeathBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BattleBusSlowDeathBehavior, BattleBusSlowDeathBehaviorModuleData )

public:

	BattleBusSlowDeathBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// slow death methods
	virtual void onDie( const DamageInfo *damageInfo );
	virtual void beginSlowDeath( const DamageInfo *damageInfo );
	virtual UpdateSleepTime update( void );

protected:

	Bool m_isRealDeath;								///< Flag set by onDie that prevents the alternate processing caused by beginSlowDeath.
	Bool m_isInFirstDeath;						///< Flag that controls lifetime of alternate processing.  Prevented by RealDeath flag.
	UnsignedInt m_groundCheckFrame;		///< Don't check for bouncing until at least here.
	UnsignedInt m_penaltyDeathFrame;	///< If non zero, kill us with Penalty damage at this frame

};
