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

// FILE: StructureToppleUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland, March 2002
// Desc:   Update that will topple over a building and crush and destroy stuff that it hits.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __StructureToppleUpdate_H_
#define __StructureToppleUpdate_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DieModule.h"
#include "Lib/BaseType.h"
#include "Common/RandomValue.h"
#include "GameClient/ParticleSys.h"

class FXList;
class ObjectCreationList;

typedef std::vector<const ObjectCreationList*> OCLVec;

//-------------------------------------------------------------------------------------------------
struct FXBoneInfo
{
	AsciiString boneName;
	const ParticleSystemTemplate* particleSystemTemplate;
};

typedef std::vector<FXBoneInfo> FXBoneInfoVector;

//-------------------------------------------------------------------------------------------------
struct AngleFXInfo
{
	Real angle;
	FXList *fxList;
};

typedef std::vector<AngleFXInfo> AngleFXInfoVector;

//-------------------------------------------------------------------------------------------------
enum StructureTopplePhaseType CPP_11(: Int)
{
	STPHASE_INITIAL = 0,
	STPHASE_DELAY,
	STPHASE_FINAL,

	ST_PHASE_COUNT
};

static const char *TheStructureTopplePhaseNames[] =
{
	"INITIAL",
	"DELAY",
	"FINAL",

	NULL
};

//-------------------------------------------------------------------------------------------------
class StructureToppleUpdateModuleData : public UpdateModuleData
{
public:
	DieMuxData m_dieMuxData;
	Int m_minToppleDelay;
	Int m_maxToppleDelay;
	Real m_structuralIntegrity;
	Real m_structuralDecay;
	DamageTypeFlags m_damageFXTypes;		///< flags used to play or not play the effects
	FXList *m_toppleStartFXList;
	FXList *m_toppleDelayFXList;
	FXList *m_toppleFXList;
	FXList *m_toppleDoneFXList;
	FXList *m_crushingFXList;
	AsciiString	m_crushingWeaponName;
	Int m_minToppleBurstDelay;
	Int m_maxToppleBurstDelay;
	OCLVec m_ocls[ST_PHASE_COUNT];
	UnsignedInt m_oclCount[ST_PHASE_COUNT];
	FXBoneInfoVector fxbones;			///< Bone names and attached particle systems.
	AngleFXInfoVector angleFX;


	StructureToppleUpdateModuleData()
	{
		m_minToppleDelay = 0;
		m_maxToppleDelay = 0;
		m_minToppleBurstDelay = 0;
		m_maxToppleBurstDelay = 0;
		m_structuralIntegrity = 0.1f;
		m_structuralDecay = 0.0f;
		m_damageFXTypes = DAMAGE_TYPE_FLAGS_ALL;
		m_toppleStartFXList = NULL;
		m_toppleDelayFXList = NULL;
		m_toppleDoneFXList = NULL;
		m_toppleFXList = NULL;
		m_crushingFXList = NULL;
		m_crushingWeaponName.set("");

		for (int i = 0; i < ST_PHASE_COUNT; ++i)
		{
			// init to one, so that if these are omitted, we choose exactly one of each.
			m_oclCount[i] = 1;
		}
		fxbones.clear();
		angleFX.clear();
	}

	static void buildFieldParse(MultiIniFieldParse& p);

};


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class StructureToppleUpdate : public UpdateModule,
															public DieModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StructureToppleUpdate, "StructureToppleUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StructureToppleUpdate, StructureToppleUpdateModuleData )

public:

	StructureToppleUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual DieModuleInterface* getDie() { return this; }

	// UpdateModuleInterface
	virtual UpdateSleepTime update();

	// DieModuleInterface
	virtual void onDie( const DamageInfo *damageInfo );

protected:

	void beginStructureTopple( const DamageInfo *damageInfo );

	void applyCrushingDamage(Real theta);
	void doDamageLine(Object *building, const WeaponTemplate* wt, Real jcos, Real jsin, Real facingWidth, Real toppleAngle);
	void doToppleStartFX(Object *building, const DamageInfo *damageInfo);
	void doToppleDelayBurstFX();

	void doToppleDoneStuff();
	void doAngleFX(Real curAngle, Real newAngle);

	void doPhaseStuff(StructureTopplePhaseType stphase, const Coord3D *target);

	enum StructureToppleStateType {
		TOPPLESTATE_STANDING,
		TOPPLESTATE_WAITINGFORTOPPLESTART,
		TOPPLESTATE_TOPPLING,
		TOPPLESTATE_WAITINGFORDONE,
		TOPPLESTATE_DONE
	};

	UnsignedInt m_toppleFrame;
	Coord2D m_toppleDirection;
	StructureToppleStateType m_toppleState;
	Real m_toppleVelocity;
	Real m_accumulatedAngle;
	Real m_structuralIntegrity;
	Real m_lastCrushedLocation;
	Int m_nextBurstFrame;
	Coord3D m_delayBurstLocation;
	Real m_buildingHeight;

};

#endif // __StructureToppleUpdate_H_

