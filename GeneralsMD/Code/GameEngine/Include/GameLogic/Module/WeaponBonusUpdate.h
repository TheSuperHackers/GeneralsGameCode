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

// FILE: WeaponBonusUpdate.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002-2003 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	July 2003
//
//	Filename: 	WeaponBonusUpdate.cpp
//
//	author:		Graham Smallwood
//
//	purpose:	Like healing in that it can affect just me or people around,
//						except this gives a Weapon Bonus instead of health
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameLogic/Module/UpdateModule.h"
//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
enum WeaponBonusConditionType CPP_11(: Int);

//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class WeaponBonusUpdateModuleData : public UpdateModuleData
{
public:

	WeaponBonusUpdateModuleData();

	KindOfMaskType						m_requiredAffectKindOf;						///< Must be set on target
	KindOfMaskType						m_forbiddenAffectKindOf;	///< Must be clear on target
	UnsignedInt								m_bonusDuration;					///< How long a hit lasts on target
	UnsignedInt								m_bonusDelay;							///< How often to pulse
	Real											m_bonusRange;							///< How far to affect
	WeaponBonusConditionType	m_bonusConditionType;			///< Status to give

	static void buildFieldParse(MultiIniFieldParse& p);
};


//-------------------------------------------------------------------------------------------------
class WeaponBonusUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WeaponBonusUpdate, "WeaponBonusUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( WeaponBonusUpdate, WeaponBonusUpdateModuleData )

public:

	WeaponBonusUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update( void );

protected:

};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
