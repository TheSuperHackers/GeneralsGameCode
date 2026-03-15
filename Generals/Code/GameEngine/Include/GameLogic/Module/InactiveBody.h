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

	virtual void attemptDamage( DamageInfo *damageInfo ) override;		///< try to damage this object
	virtual void attemptHealing( DamageInfo *damageInfo ) override;		///< try to heal this object
	virtual Real estimateDamage( DamageInfoInput& damageInfo ) const override;
	virtual Real getHealth() const override;													///< get current health
	virtual BodyDamageType getDamageState() const override;
	virtual void setDamageState( BodyDamageType newState ) override;	///< control damage state directly.  Will adjust hitpoints.
	virtual void setAflame( Bool setting ) override {}///< This is a major change like a damage state.

	virtual void onVeterancyLevelChanged( VeterancyLevel oldLevel, VeterancyLevel newLevel, Bool provideFeedback ) override { /* nothing */ }

	virtual void setArmorSetFlag(ArmorSetType ast) override { /* nothing */ }
	virtual void clearArmorSetFlag(ArmorSetType ast) override { /* nothing */ }

	virtual void internalChangeHealth( Real delta ) override;

private:
	Bool m_dieCalled;
};
