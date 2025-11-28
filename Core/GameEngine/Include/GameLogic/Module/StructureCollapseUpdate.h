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

// FILE: StructureCollapseUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland, March 2002
// Desc:   Update that will collapse a building.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "Common/RandomValue.h"

class FXList;
class ObjectCreationList;

typedef std::vector<const ObjectCreationList*> OCLVec;
typedef std::vector<const FXList*> FXVec;


//-------------------------------------------------------------------------------------------------
enum StructureCollapsePhaseType CPP_11(: Int)
{
	SCPHASE_INITIAL = 0,
	SCPHASE_DELAY,
	SCPHASE_BURST,
	SCPHASE_FINAL,

	SC_PHASE_COUNT
};

//-------------------------------------------------------------------------------------------------
class StructureCollapseUpdateModuleData : public UpdateModuleData
{
public:
	DieMuxData				m_dieMuxData;
	Int								m_minCollapseDelay;
	Int								m_maxCollapseDelay;
	Int								m_minBurstDelay;
	Int								m_maxBurstDelay;
	Int								m_bigBurstFrequency;
	Real							m_collapseDamping;
	Real							m_maxShudder;
	OCLVec						m_ocls[SC_PHASE_COUNT];
	FXVec							m_fxs[SC_PHASE_COUNT];
	UnsignedInt				m_oclCount[SC_PHASE_COUNT];
	UnsignedInt				m_fxCount[SC_PHASE_COUNT];

	StructureCollapseUpdateModuleData()
	{
		m_minCollapseDelay = 0;
		m_maxCollapseDelay = 0;
		m_minBurstDelay = 9999;
		//Removed by Sadullah Nader
		//Redundancy from above
		//m_minBurstDelay = 9999;
		m_maxShudder = 0;
		m_collapseDamping = 0.0;
		m_bigBurstFrequency = 0;

		for (int i = 0; i < SC_PHASE_COUNT; ++i)
		{
			// init to one, so that if these are omitted, we choose exactly one of each.
			m_oclCount[i] = 1;
			m_fxCount[i] = 1;
		}
	}

	static void buildFieldParse(MultiIniFieldParse& p);

};


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class StructureCollapseUpdate : public UpdateModule,
																public DieModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StructureCollapseUpdate, "StructureCollapseUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StructureCollapseUpdate, StructureCollapseUpdateModuleData )

public:

	StructureCollapseUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual DieModuleInterface* getDie() { return this; }

	// UpdateModuleInterface
	virtual UpdateSleepTime update();

	// DieModuleInterface
	virtual void onDie( const DamageInfo *damageInfo );

protected:

	void applyCrushingDamage(Real theta);
	void beginStructureCollapse( const DamageInfo *damageInfo );
	void doDamageLine(Object *building, const WeaponTemplate* wt, Real jcos, Real jsin, Real facingWidth, Real collapseAngle);
	void doCollapseStartFX(Object *building, const DamageInfo *damageInfo);
	void doCollapseDelayBurstFX();
	void doCollapseDoneStuff();

	void doPhaseStuff(StructureCollapsePhaseType scphase, const Coord3D *target);

	enum StructureCollapseStateType
	{
		COLLAPSESTATE_STANDING,
		COLLAPSESTATE_WAITINGFORCOLLAPSESTART,
		COLLAPSESTATE_COLLAPSING,
		COLLAPSESTATE_DONE
	};

	UnsignedInt									m_collapseFrame;
	UnsignedInt									m_burstFrame;
	StructureCollapseStateType	m_collapseState;
	Real												m_collapseVelocity;
	Real												m_currentHeight;

};
