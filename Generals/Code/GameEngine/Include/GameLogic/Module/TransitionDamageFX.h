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

// FILE: TransitionDamageFX.h /////////////////////////////////////////////////////////////////////
// Author: Colin Day, March 2002
// Desc:   Damage module capable of launching various effects on damage transitions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __TRANSITIONDAMAGEFX_H_
#define __TRANSITIONDAMAGEFX_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameClient/ParticleSys.h"
#include "GameLogic/Module/DamageModule.h"
#include "GameLogic/Module/BodyModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class FXList;
class ObjectCreationList;
class ParticleSystemTemplate;

// we can have this many effects of each type per body state
// NOTE: If you change this you should update the dataFieldParse[] table in the FXDamageModuleData
//       to allow for the new indices into the effect arrays
enum { DAMAGE_MODULE_MAX_FX = 12 };

typedef Char FXDamageLocType;
enum
{
	FX_DAMAGE_LOC_TYPE_BONE  = 0,
	FX_DAMAGE_LOC_TYPE_COORD = 1
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct FXLocInfo
{
	FXDamageLocType locType;			// location is bone or coord
	AsciiString boneName;					// bone name to use for effect pos
	Bool randomBone;							// if true, boneName is a prefix and we will do the effect at one of the bones with that prefix
	Coord3D loc;									// location is a specified point in space
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct FXDamageFXListInfo
{
	const FXList *fx;
	FXLocInfo locInfo;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct FXDamageOCLInfo
{
	const ObjectCreationList *ocl;
	FXLocInfo locInfo;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct FXDamageParticleSystemInfo
{
	const ParticleSystemTemplate *particleSysTemplate;
	FXLocInfo locInfo;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class TransitionDamageFXModuleData : public DamageModuleData
{

public:

	TransitionDamageFXModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    DamageModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{

			{ "DamageFXTypes",				INI::parseDamageTypeFlags, NULL, offsetof( TransitionDamageFXModuleData, m_damageFXTypes ) },
			{ "DamageOCLTypes",				INI::parseDamageTypeFlags, NULL, offsetof( TransitionDamageFXModuleData, m_damageOCLTypes ) },
			{ "DamageParticleTypes",	INI::parseDamageTypeFlags, NULL, offsetof( TransitionDamageFXModuleData, m_damageParticleTypes ) },

			{ "DamagedFXList1",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 0 ] ) },
			{ "DamagedFXList2",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 1 ] ) },
			{ "DamagedFXList3",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 2 ] ) },
			{ "DamagedFXList4",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 3 ] ) },
			{ "DamagedFXList5",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 4 ] ) },
			{ "DamagedFXList6",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 5 ] ) },
			{ "DamagedFXList7",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 6 ] ) },
			{ "DamagedFXList8",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 7 ] ) },
			{ "DamagedFXList9",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 8 ] ) },
			{ "DamagedFXList10",							TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 9 ] ) },
			{ "DamagedFXList11",							TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 10 ] ) },
			{ "DamagedFXList12",							TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_DAMAGED ][ 11 ] ) },
			{ "ReallyDamagedFXList1",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 0 ] ) },
			{ "ReallyDamagedFXList2",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 1 ] ) },
			{ "ReallyDamagedFXList3",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 2 ] ) },
			{ "ReallyDamagedFXList4",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 3 ] ) },
			{ "ReallyDamagedFXList5",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 4 ] ) },
			{ "ReallyDamagedFXList6",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 5 ] ) },
			{ "ReallyDamagedFXList7",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 6 ] ) },
			{ "ReallyDamagedFXList8",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 7 ] ) },
			{ "ReallyDamagedFXList9",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 8 ] ) },
			{ "ReallyDamagedFXList10",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 9 ] ) },
			{ "ReallyDamagedFXList11",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 10 ] ) },
			{ "ReallyDamagedFXList12",					TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_REALLYDAMAGED ][ 11 ] ) },
			{ "RubbleFXList1",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 0 ] ) },
			{ "RubbleFXList2",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 1 ] ) },
			{ "RubbleFXList3",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 2 ] ) },
			{ "RubbleFXList4",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 3 ] ) },
			{ "RubbleFXList5",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 4 ] ) },
			{ "RubbleFXList6",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 5 ] ) },
			{ "RubbleFXList7",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 6 ] ) },
			{ "RubbleFXList8",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 7 ] ) },
			{ "RubbleFXList9",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 8 ] ) },
			{ "RubbleFXList10",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 9 ] ) },
			{ "RubbleFXList11",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 10 ] ) },
			{ "RubbleFXList12",								TransitionDamageFXModuleData::parseFXList,							NULL,	offsetof( TransitionDamageFXModuleData, m_fxList[ BODY_RUBBLE ][ 11 ] ) },

			{ "DamagedOCL1",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 0 ] ) },
			{ "DamagedOCL2",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 1 ] ) },
			{ "DamagedOCL3",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 2 ] ) },
			{ "DamagedOCL4",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 3 ] ) },
			{ "DamagedOCL5",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 4 ] ) },
			{ "DamagedOCL6",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 5 ] ) },
			{ "DamagedOCL7",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 6 ] ) },
			{ "DamagedOCL8",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 7 ] ) },
			{ "DamagedOCL9",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 8 ] ) },
			{ "DamagedOCL10",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 9 ] ) },
			{ "DamagedOCL11",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 10 ] ) },
			{ "DamagedOCL12",									TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_DAMAGED ][ 11 ] ) },
			{ "ReallyDamagedOCL1",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 0 ] ) },
			{ "ReallyDamagedOCL2",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 1 ] ) },
			{ "ReallyDamagedOCL3",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 2 ] ) },
			{ "ReallyDamagedOCL4",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 3 ] ) },
			{ "ReallyDamagedOCL5",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 4 ] ) },
			{ "ReallyDamagedOCL6",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 5 ] ) },
			{ "ReallyDamagedOCL7",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 6 ] ) },
			{ "ReallyDamagedOCL8",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 7 ] ) },
			{ "ReallyDamagedOCL9",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 8 ] ) },
			{ "ReallyDamagedOCL10",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 9 ] ) },
			{ "ReallyDamagedOCL11",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 10 ] ) },
			{ "ReallyDamagedOCL12",						TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_REALLYDAMAGED ][ 11 ] ) },
			{ "RubbleOCL1",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 0 ] ) },
			{ "RubbleOCL2",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 1 ] ) },
			{ "RubbleOCL3",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 2 ] ) },
			{ "RubbleOCL4",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 3 ] ) },
			{ "RubbleOCL5",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 4 ] ) },
			{ "RubbleOCL6",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 5 ] ) },
			{ "RubbleOCL7",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 6 ] ) },
			{ "RubbleOCL8",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 7 ] ) },
			{ "RubbleOCL9",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 8 ] ) },
			{ "RubbleOCL10",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 9 ] ) },
			{ "RubbleOCL11",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 10 ] ) },
			{ "RubbleOCL12",										TransitionDamageFXModuleData::parseObjectCreationList,  NULL, offsetof( TransitionDamageFXModuleData, m_OCL[ BODY_RUBBLE ][ 11 ] ) },

			{ "DamagedParticleSystem1",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 0 ] ) },
			{ "DamagedParticleSystem2",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 1 ] ) },
			{ "DamagedParticleSystem3",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 2 ] ) },
			{ "DamagedParticleSystem4",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 3 ] ) },
			{ "DamagedParticleSystem5",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 4 ] ) },
			{ "DamagedParticleSystem6",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 5 ] ) },
			{ "DamagedParticleSystem7",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 6 ] ) },
			{ "DamagedParticleSystem8",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 7 ] ) },
			{ "DamagedParticleSystem9",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 8 ] ) },
			{ "DamagedParticleSystem10",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 9 ] ) },
			{ "DamagedParticleSystem11",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 10 ] ) },
			{ "DamagedParticleSystem12",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_DAMAGED ][ 11 ] ) },
			{ "ReallyDamagedParticleSystem1",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 0 ] ) },
			{ "ReallyDamagedParticleSystem2",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 1 ] ) },
			{ "ReallyDamagedParticleSystem3",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 2 ] ) },
			{ "ReallyDamagedParticleSystem4",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 3 ] ) },
			{ "ReallyDamagedParticleSystem5",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 4 ] ) },
			{ "ReallyDamagedParticleSystem6",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 5 ] ) },
			{ "ReallyDamagedParticleSystem7",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 6 ] ) },
			{ "ReallyDamagedParticleSystem8",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 7 ] ) },
			{ "ReallyDamagedParticleSystem9",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 8 ] ) },
			{ "ReallyDamagedParticleSystem10",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 9 ] ) },
			{ "ReallyDamagedParticleSystem11",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 10 ] ) },
			{ "ReallyDamagedParticleSystem12",	TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_REALLYDAMAGED ][ 11 ] ) },
			{ "RubbleParticleSystem1",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 0 ] ) },
			{ "RubbleParticleSystem2",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 1 ] ) },
			{ "RubbleParticleSystem3",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 2 ] ) },
			{ "RubbleParticleSystem4",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 3 ] ) },
			{ "RubbleParticleSystem5",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 4 ] ) },
			{ "RubbleParticleSystem6",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 5 ] ) },
			{ "RubbleParticleSystem7",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 6 ] ) },
			{ "RubbleParticleSystem8",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 7 ] ) },
			{ "RubbleParticleSystem9",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 8 ] ) },
			{ "RubbleParticleSystem10",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 9 ] ) },
			{ "RubbleParticleSystem11",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 10 ] ) },
			{ "RubbleParticleSystem12",				TransitionDamageFXModuleData::parseParticleSystem,			NULL, offsetof( TransitionDamageFXModuleData, m_particleSystem[ BODY_RUBBLE ][ 11 ] ) },

			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}

	static void parseFXList( INI *ini, void *instance, void *store, const void *userData );
	static void parseObjectCreationList( INI *ini, void *instance, void *store, const void *userData );
	static void parseParticleSystem( INI *ini, void *instance, void *store, const void *userData );

	DamageTypeFlags m_damageFXTypes;		///< flags used to play or not play the effects
	FXDamageFXListInfo m_fxList[ BODYDAMAGETYPE_COUNT ][ DAMAGE_MODULE_MAX_FX ];
	DamageTypeFlags m_damageOCLTypes;		///< flags used to play or not play the effects
	FXDamageOCLInfo m_OCL[ BODYDAMAGETYPE_COUNT ][ DAMAGE_MODULE_MAX_FX ];
	DamageTypeFlags m_damageParticleTypes;		///< flags used to play or not play the effects
	FXDamageParticleSystemInfo m_particleSystem[ BODYDAMAGETYPE_COUNT ][ DAMAGE_MODULE_MAX_FX ];

};

//-------------------------------------------------------------------------------------------------
class TransitionDamageFX : public DamageModule
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( TransitionDamageFX, TransitionDamageFXModuleData );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TransitionDamageFX, "TransitionDamageFX" )

public:

	TransitionDamageFX( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDamage( DamageInfo *damageInfo ) { }
	virtual void onHealing( DamageInfo *damageInfo ) { }
	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState );

protected:

	/// we keep a record of attached particle system so we can detach and kill them when we want to
	ParticleSystemID m_particleSystemID[ BODYDAMAGETYPE_COUNT ][ DAMAGE_MODULE_MAX_FX ];

};

#endif  // end __TRANSITIONDAMAGEFX
