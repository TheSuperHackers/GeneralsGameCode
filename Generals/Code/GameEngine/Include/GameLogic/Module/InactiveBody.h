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

// FILE: InactiveBody.h ///////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:	 An inactive body module, they are indestructable and largely cannot be
//				 affected by things in the world.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __INACTIVEBODY_H_
#define __INACTIVEBODY_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BodyModule.h"

//-------------------------------------------------------------------------------------------------
/** Inactive body module */
//-------------------------------------------------------------------------------------------------
class InactiveBody : public BodyModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( InactiveBody, "InactiveBody" )
	MAKE_STANDARD_MODULE_MACRO( InactiveBody )

public:

	InactiveBody( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void attemptDamage( DamageInfo *damageInfo );		///< try to damage this object
	virtual void attemptHealing( DamageInfo *damageInfo );		///< try to heal this object
	virtual Real estimateDamage( DamageInfoInput& damageInfo ) const;
	virtual Real getHealth() const;													///< get current health
	virtual BodyDamageType getDamageState() const;
	virtual void setDamageState( BodyDamageType newState );	///< control damage state directly.  Will adjust hitpoints.
	virtual void setAflame( Bool setting ){}///< This is a major change like a damage state.

	void onVeterancyLevelChanged( VeterancyLevel oldLevel, VeterancyLevel newLevel ) { /* nothing */ }

	virtual void setArmorSetFlag(ArmorSetType ast) { /* nothing */ }
	virtual void clearArmorSetFlag(ArmorSetType ast) { /* nothing */ }

	virtual void internalChangeHealth( Real delta );

private:
	Bool m_dieCalled;
};

#endif // __INACTIVEBODY_H_

