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

// FILE: SpyVisionSpecialPower.h /////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September 2002
// Desc:   Special Power will spy on the vision of all enemy players.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/SpecialPowerModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class FXList;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpyVisionSpecialPowerModuleData : public SpecialPowerModuleData
{

public:

	SpyVisionSpecialPowerModuleData( void );

	static void buildFieldParse( MultiIniFieldParse& p );

	UnsignedInt m_baseDurationInFrames;		///< duration of the demoralization (in frames)
	UnsignedInt m_bonusDurationPerCapturedInFrames;	///< additional duration added for each prisoner we have
	UnsignedInt m_maxDurationInFrames;		///< no matter how many prisoners we have, this is max

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpyVisionSpecialPower : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpyVisionSpecialPower, "SpyVisionSpecialPower" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpyVisionSpecialPower, SpyVisionSpecialPowerModuleData )

public:

	SpyVisionSpecialPower( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool object

	virtual void doSpecialPower( UnsignedInt commandOptions );

protected:

};
