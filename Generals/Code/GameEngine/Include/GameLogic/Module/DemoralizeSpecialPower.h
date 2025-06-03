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

// FILE: DemoralizeSpecialPower.h /////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   Demoralize
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DEMORALIZE_SPECIAL_POWER_H_
#define __DEMORALIZE_SPECIAL_POWER_H_

#ifdef ALLOW_DEMORALIZE

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/SpecialPowerModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class FXList;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DemoralizeSpecialPowerModuleData : public SpecialPowerModuleData
{

public:

	DemoralizeSpecialPowerModuleData( void );

	static void buildFieldParse( MultiIniFieldParse& p );

	Real m_baseRange;											///< base range for this special power
	Real m_bonusRangePerCaptured;					///< additional range we get for each prisoner
	Real m_maxRange;											///< no matter how many prisoners we have, this is max
	UnsignedInt m_baseDurationInFrames;		///< duration of the demoralization (in frames)
	UnsignedInt m_bonusDurationPerCapturedInFrames;	///< additional duration added for each prisoner we have
	UnsignedInt m_maxDurationInFrames;		///< no matter how many prisoners we have, this is max
	const FXList *m_fxList;								///< fx list to play

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DemoralizeSpecialPower : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DemoralizeSpecialPower, "DemoralizeSpecialPower" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DemoralizeSpecialPower, DemoralizeSpecialPowerModuleData )

public:

	DemoralizeSpecialPower( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool object

	virtual void doSpecialPowerAtObject( const Object *obj, UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions );

protected:

};

#endif // ALLOW_DEMORALIZE


#endif  // end __DEMORALIZE_SPECIAL_POWER_H_
