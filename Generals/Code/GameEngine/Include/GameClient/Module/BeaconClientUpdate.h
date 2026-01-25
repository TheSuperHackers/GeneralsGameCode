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

// FILE: BeaconClientUpdate.h ////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, August 2002
// Desc:   Beacon client update module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ClientUpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class ParticleSystem;

//-------------------------------------------------------------------------------------------------
class BeaconClientUpdateModuleData : public ClientUpdateModuleData
{
public:
	UnsignedInt m_framesBetweenRadarPulses;
	UnsignedInt m_radarPulseDuration;

	BeaconClientUpdateModuleData();
	~BeaconClientUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
/** The tree way client update module */
//-------------------------------------------------------------------------------------------------
class BeaconClientUpdate : public ClientUpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BeaconClientUpdate, "BeaconClientUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BeaconClientUpdate, BeaconClientUpdateModuleData );

public:

	BeaconClientUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// the client update callback
	virtual void clientUpdate( void );
	void hideBeacon( void );

protected:

	ParticleSystemID m_particleSystemID;
	UnsignedInt m_lastRadarPulse;

};
