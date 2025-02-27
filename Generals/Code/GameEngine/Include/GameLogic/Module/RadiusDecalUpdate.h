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

// FILE: RadiusDecalUpdate.h /////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "GameClient/RadiusDecal.h"

//-------------------------------------------------------------------------------------------------
class RadiusDecalUpdateModuleData : public UpdateModuleData
{
public:
	//RadiusDecalTemplate	m_deliveryDecalTemplate;
	//Real								m_deliveryDecalRadius;

	RadiusDecalUpdateModuleData()
	{
		//m_deliveryDecalRadius = 0.0f;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			//{ "DeliveryDecal",						RadiusDecalTemplate::parseRadiusDecalTemplate,	NULL, offsetof( RadiusDecalUpdateModuleData, m_deliveryDecalTemplate ) },
			//{ "DeliveryDecalRadius",			INI::parseReal,									NULL,	offsetof( RadiusDecalUpdateModuleData, m_deliveryDecalRadius ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class RadiusDecalUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RadiusDecalUpdate, "RadiusDecalUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RadiusDecalUpdate, RadiusDecalUpdateModuleData )

public:

	RadiusDecalUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	//void createRadiusDecal( const Coord3D& pos );
	void createRadiusDecal( const RadiusDecalTemplate& tmpl, Real radius, const Coord3D& pos );
	void killWhenNoLongerAttacking(Bool v) { m_killWhenNoLongerAttacking = v; }
	void killRadiusDecal();

	virtual UpdateSleepTime update( void );

private:

	RadiusDecal m_deliveryDecal;
	Bool m_killWhenNoLongerAttacking;
};
