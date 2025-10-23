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

// FILE: BaikonurLaunchPower.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	Created:	November 2002
//
//	Filename: BaikonurLaunchPower.h
//
//	Author:		Kris Morness
//
//  Purpose:	Triggers the beginning of the launch for the baikonur launch tower.
//            This is used only by script to trigger the GLA end game.
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameLogic/Module/SpecialPowerModule.h"

class Object;
class SpecialPowerTemplate;
struct FieldParse;
enum ScienceType CPP_11(: Int);

class BaikonurLaunchPowerModuleData : public SpecialPowerModuleData
{

public:

	BaikonurLaunchPowerModuleData( void );

	static void buildFieldParse( MultiIniFieldParse& p );

	AsciiString m_detonationObject;
};


//-------------------------------------------------------------------------------------------------
class BaikonurLaunchPower : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BaikonurLaunchPower, "BaikonurLaunchPower" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BaikonurLaunchPower, BaikonurLaunchPowerModuleData )

public:

	BaikonurLaunchPower( Thing *thing, const ModuleData *moduleData );

	virtual void doSpecialPower( UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions );

protected:

};
