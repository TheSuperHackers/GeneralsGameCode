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

// FILE: BoneFXDamage.h /////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland, April 2002
// Desc:   Damage module for the boneFX update module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BONEFXDAMAGE_H_
#define __BONEFXDAMAGE_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////

#include "GameLogic/Module/DamageModule.h" 

//#include "GameLogic/Module/BodyModule.h" -- Yikes... not necessary to include this! (KM)
enum BodyDamageType CPP_11(: Int); //Ahhhh much better!


// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
class BoneFXDamage : public DamageModule
{

	MAKE_STANDARD_MODULE_MACRO( BoneFXDamage );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BoneFXDamage, "BoneFXDamage" )

public:

	BoneFXDamage( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// damage module methods
	virtual void onDamage( DamageInfo *damageInfo ) { }
	virtual void onHealing( DamageInfo *damageInfo ) { }
	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo, 
																				BodyDamageType oldState, 
																				BodyDamageType newState );

protected:

	virtual void onObjectCreated();

};

#endif  // end __BONEFXDAMAGE_H_
