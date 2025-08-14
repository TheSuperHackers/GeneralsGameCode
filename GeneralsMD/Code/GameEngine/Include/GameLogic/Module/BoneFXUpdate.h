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

// FILE: BoneFXUpdate.h /////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland, April 2002
// Desc:   Update module that will fire off FX lists at bone locations at random intervals
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BONEFXUPDATE_H_
#define __BONEFXUPDATE_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameClient/ParticleSys.h"
#include "GameLogic/Module/UpdateModule.h"

//#include "GameLogic/Module/DamageModule.h" -- Yikes... not necessary to include this! (KM)
class DamageInfo; //Ahhhh much better!

#include "GameLogic/Module/BodyModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class FXList;
class ObjectCreationList;
class ParticleSystemTemplate;

// we can have this many effects of each type per body state
// NOTE: If you change this you should update the dataFieldParse[] table in the FXDamageModuleData
//       to allow for the new indices into the effect arrays
enum { BONE_FX_MAX_BONES = 8 };

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct BoneLocInfo
{
	AsciiString boneName;					// bone name to use for effect pos
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct BaseBoneListInfo
{
	BoneLocInfo locInfo;
	GameClientRandomVariable gameClientDelay;
	GameLogicRandomVariable gameLogicDelay;
	Bool onlyOnce;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct BoneFXListInfo : public BaseBoneListInfo
{
	const FXList *fx;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct BoneOCLInfo : public BaseBoneListInfo
{
	const ObjectCreationList *ocl;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct BoneParticleSystemInfo : public BaseBoneListInfo
{
	const ParticleSystemTemplate *particleSysTemplate;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class BoneFXUpdateModuleData : public UpdateModuleData
{

public:

	BoneFXUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{

			{ "DamageFXTypes",				INI::parseDamageTypeFlags, NULL, offsetof( BoneFXUpdateModuleData, m_damageFXTypes ) },
			{ "DamageOCLTypes",				INI::parseDamageTypeFlags, NULL, offsetof( BoneFXUpdateModuleData, m_damageOCLTypes ) },
			{ "DamageParticleTypes",	INI::parseDamageTypeFlags, NULL, offsetof( BoneFXUpdateModuleData, m_damageParticleTypes ) },

			{ "PristineFXList1",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 0 ] ) },
			{ "PristineFXList2",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 1 ] ) },
			{ "PristineFXList3",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 2 ] ) },
			{ "PristineFXList4",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 3 ] ) },
			{ "PristineFXList5",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 4 ] ) },
			{ "PristineFXList6",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 5 ] ) },
			{ "PristineFXList7",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 6 ] ) },
			{ "PristineFXList8",							BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_PRISTINE ][ 7 ] ) },
			{ "DamagedFXList1",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 0 ] ) },
			{ "DamagedFXList2",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 1 ] ) },
			{ "DamagedFXList3",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 2 ] ) },
			{ "DamagedFXList4",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 3 ] ) },
			{ "DamagedFXList5",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 4 ] ) },
			{ "DamagedFXList6",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 5 ] ) },
			{ "DamagedFXList7",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 6 ] ) },
			{ "DamagedFXList8",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_DAMAGED ][ 7 ] ) },
			{ "ReallyDamagedFXList1",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 0 ] ) },
			{ "ReallyDamagedFXList2",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 1 ] ) },
			{ "ReallyDamagedFXList3",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 2 ] ) },
			{ "ReallyDamagedFXList4",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 3 ] ) },
			{ "ReallyDamagedFXList5",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 4 ] ) },
			{ "ReallyDamagedFXList6",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 5 ] ) },
			{ "ReallyDamagedFXList7",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 6 ] ) },
			{ "ReallyDamagedFXList8",					BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 7 ] ) },
			{ "RubbleFXList1",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 0 ] ) },
			{ "RubbleFXList2",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 1 ] ) },
			{ "RubbleFXList3",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 2 ] ) },
			{ "RubbleFXList4",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 3 ] ) },
			{ "RubbleFXList5",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 4 ] ) },
			{ "RubbleFXList6",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 5 ] ) },
			{ "RubbleFXList7",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 6 ] ) },
			{ "RubbleFXList8",								BoneFXUpdateModuleData::parseFXList,							NULL,	offsetof( BoneFXUpdateModuleData, m_fxList[ BODY_RUBBLE ][ 7 ] ) },

			{ "PristineOCL1",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 0 ] ) },
			{ "PristineOCL2",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 1 ] ) },
			{ "PristineOCL3",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 2 ] ) },
			{ "PristineOCL4",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 3 ] ) },
			{ "PristineOCL5",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 4 ] ) },
			{ "PristineOCL6",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 5 ] ) },
			{ "PristineOCL7",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 6 ] ) },
			{ "PristineOCL8",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_PRISTINE ][ 7 ] ) },
			{ "DamagedOCL1",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 0 ] ) },
			{ "DamagedOCL2",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 1 ] ) },
			{ "DamagedOCL3",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 2 ] ) },
			{ "DamagedOCL4",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 3 ] ) },
			{ "DamagedOCL5",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 4 ] ) },
			{ "DamagedOCL6",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 5 ] ) },
			{ "DamagedOCL7",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 6 ] ) },
			{ "DamagedOCL8",									BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_DAMAGED ][ 7 ] ) },
			{ "ReallyDamagedOCL1",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 0 ] ) },
			{ "ReallyDamagedOCL2",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 1 ] ) },
			{ "ReallyDamagedOCL3",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 2 ] ) },
			{ "ReallyDamagedOCL4",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 3 ] ) },
			{ "ReallyDamagedOCL5",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 4 ] ) },
			{ "ReallyDamagedOCL6",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 5 ] ) },
			{ "ReallyDamagedOCL7",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 6 ] ) },
			{ "ReallyDamagedOCL8",						BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 7 ] ) },
			{ "RubbleOCL1",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 0 ] ) },
			{ "RubbleOCL2",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 1 ] ) },
			{ "RubbleOCL3",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 2 ] ) },
			{ "RubbleOCL4",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 3 ] ) },
			{ "RubbleOCL5",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 4 ] ) },
			{ "RubbleOCL6",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 5 ] ) },
			{ "RubbleOCL7",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 6 ] ) },
			{ "RubbleOCL8",										BoneFXUpdateModuleData::parseObjectCreationList,  NULL, offsetof( BoneFXUpdateModuleData, m_OCL[ BODY_RUBBLE ][ 7 ] ) },

			{ "PristineParticleSystem1",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 0 ] ) },
			{ "PristineParticleSystem2",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 1 ] ) },
			{ "PristineParticleSystem3",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 2 ] ) },
			{ "PristineParticleSystem4",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 3 ] ) },
			{ "PristineParticleSystem5",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 4 ] ) },
			{ "PristineParticleSystem6",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 5 ] ) },
			{ "PristineParticleSystem7",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 6 ] ) },
			{ "PristineParticleSystem8",			BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_PRISTINE ][ 7 ] ) },
			{ "DamagedParticleSystem1",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 0 ] ) },
			{ "DamagedParticleSystem2",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 1 ] ) },
			{ "DamagedParticleSystem3",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 2 ] ) },
			{ "DamagedParticleSystem4",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 3 ] ) },
			{ "DamagedParticleSystem5",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 4 ] ) },
			{ "DamagedParticleSystem6",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 5 ] ) },
			{ "DamagedParticleSystem7",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 6 ] ) },
			{ "DamagedParticleSystem8",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_DAMAGED ][ 7 ] ) },
			{ "ReallyDamagedParticleSystem1",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 0 ] ) },
			{ "ReallyDamagedParticleSystem2",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 1 ] ) },
			{ "ReallyDamagedParticleSystem3",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 2 ] ) },
			{ "ReallyDamagedParticleSystem4",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 3 ] ) },
			{ "ReallyDamagedParticleSystem5",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 4 ] ) },
			{ "ReallyDamagedParticleSystem6",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 5 ] ) },
			{ "ReallyDamagedParticleSystem7",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 6 ] ) },
			{ "ReallyDamagedParticleSystem8",	BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 7 ] ) },
			{ "RubbleParticleSystem1",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 0 ] ) },
			{ "RubbleParticleSystem2",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 1 ] ) },
			{ "RubbleParticleSystem3",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 2 ] ) },
			{ "RubbleParticleSystem4",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 3 ] ) },
			{ "RubbleParticleSystem5",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 4 ] ) },
			{ "RubbleParticleSystem6",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 5 ] ) },
			{ "RubbleParticleSystem7",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 6 ] ) },
			{ "RubbleParticleSystem8",				BoneFXUpdateModuleData::parseParticleSystem,			NULL, offsetof( BoneFXUpdateModuleData, m_particleSystem[ BODY_RUBBLE ][ 7 ] ) },

			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}

	static void parseFXList( INI *ini, void *instance, void *store, const void *userData );
	static void parseObjectCreationList( INI *ini, void *instance, void *store, const void *userData );
	static void parseParticleSystem( INI *ini, void *instance, void *store, const void *userData );

	DamageTypeFlags m_damageFXTypes;		///< flags used to play or not play the effects
	BoneFXListInfo m_fxList[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	DamageTypeFlags m_damageOCLTypes;		///< flags used to play or not play the effects
	BoneOCLInfo m_OCL[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	DamageTypeFlags m_damageParticleTypes;		///< flags used to play or not play the effects
	BoneParticleSystemInfo m_particleSystem[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];

};

//-------------------------------------------------------------------------------------------------
class BoneFXUpdate : public UpdateModule
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BoneFXUpdate, BoneFXUpdateModuleData );

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BoneFXUpdate, "BoneFXUpdate"  )

public:

	BoneFXUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	void changeBodyDamageState( BodyDamageType oldState, BodyDamageType newState);
	void stopAllBoneFX();

	virtual UpdateSleepTime update();

protected:

	virtual void onObjectCreated();

	virtual void resolveBoneLocations();

	void doFXListAtBone(const FXList *fxList, const Coord3D *bonePosition);
	void doOCLAtBone(const ObjectCreationList *ocl, const Coord3D *bonePosition);
	void doParticleSystemAtBone(const ParticleSystemTemplate *particleSystemTemplate, const Coord3D *bonePosition);
	void killRunningParticleSystems();

	void computeNextClientFXTime(const BaseBoneListInfo *info, Int &nextFrame);
	void computeNextLogicFXTime(const BaseBoneListInfo *info, Int &nextFrame);
	void initTimes();



	typedef std::vector<ParticleSystemID> ParticleSystemIDVec;

	/// we keep a record of attached particle system so we can detach and kill them when we want to
	ParticleSystemIDVec m_particleSystemIDs;
	Int m_nextFXFrame[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	Int m_nextOCLFrame[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	Int m_nextParticleSystemFrame[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	Coord3D m_FXBonePositions[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	Coord3D m_OCLBonePositions[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	Coord3D m_PSBonePositions[ BODYDAMAGETYPE_COUNT ][ BONE_FX_MAX_BONES ];
	BodyDamageType m_curBodyState;
	Bool m_bonesResolved[BODYDAMAGETYPE_COUNT];
	Bool m_active;
};

#endif  // end __BONEFXUPDATE_H_
