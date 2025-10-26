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

// FILE: FirestormDynamicGeometryInfoUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:   Update module adds the molestation of a particle system to Geometry changing
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Geometry.h"
#include "GameLogic/Module/DynamicGeometryInfoUpdate.h"

// ------------------------------------------------------------------------------------------------
enum { MAX_FIRESTORM_SYSTEMS = 16 };

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class ParticleSystemTemplate;

class FirestormDynamicGeometryInfoUpdateModuleData : public DynamicGeometryInfoUpdateModuleData
{
public:

	FirestormDynamicGeometryInfoUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

	const FXList *m_fxList;
	const ParticleSystemTemplate *m_particleSystem[ MAX_FIRESTORM_SYSTEMS ];
	Real m_particleOffsetZ;
	Real m_scorchSize;
	Real m_delayBetweenDamageFrames;
	Real m_damageAmount;
	Real m_maxHeightForDamage;	// things higher than this above us take no damage

};

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class FirestormDynamicGeometryInfoUpdate : public DynamicGeometryInfoUpdate
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FirestormDynamicGeometryInfoUpdate, "FirestormDynamicGeometryInfoUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FirestormDynamicGeometryInfoUpdate, FirestormDynamicGeometryInfoUpdateModuleData );

public:

	FirestormDynamicGeometryInfoUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

protected:

	void doDamageScan( void );

	ParticleSystemID m_myParticleSystemID[ MAX_FIRESTORM_SYSTEMS ];
	Bool m_effectsFired;							///< TRUE once the effects have been fired off
	Bool m_scorchPlaced;							///< TRUE once we have placed the scorch mark
	UnsignedInt m_lastDamageFrame;		///< frame we last did damage on
};
