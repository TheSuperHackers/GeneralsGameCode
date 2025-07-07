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

// FILE: BoneFXUpdate.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:  
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameState.h"
#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/BoneFXUpdate.h"
#include "GameLogic/Module/BoneFXDamage.h"

const Int MAX_IDX = 32;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BoneFXUpdateModuleData::BoneFXUpdateModuleData(void)
{
	Int i, j;
	for (i = 0; i < BODYDAMAGETYPE_COUNT; ++i) {
		for (j = 0; j < BONE_FX_MAX_BONES; ++j) {
			m_fxList[i][j].fx = NULL;
			m_fxList[i][j].onlyOnce = TRUE;
			m_OCL[i][j].ocl = NULL;
			m_OCL[i][j].onlyOnce = TRUE;
			m_particleSystem[i][j].particleSysTemplate = NULL;
			m_particleSystem[i][j].onlyOnce = TRUE;
		}
	}

	m_damageFXTypes = DAMAGE_TYPE_FLAGS_ALL;
	m_damageOCLTypes = DAMAGE_TYPE_FLAGS_ALL;
	m_damageParticleTypes = DAMAGE_TYPE_FLAGS_ALL;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BoneFXUpdate::BoneFXUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	Int i, j;
	for (i = 0; i < BODYDAMAGETYPE_COUNT; ++i) {
		for (j = 0; j < BONE_FX_MAX_BONES; ++j) {
			m_nextFXFrame[i][j] = -1;
			m_nextOCLFrame[i][j] = -1;
			m_nextParticleSystemFrame[i][j] = -1;
			m_FXBonePositions[i][j].zero();
			m_OCLBonePositions[i][j].zero();
			m_PSBonePositions[i][j].zero();
		}
		m_bonesResolved[i] = FALSE;
	}
	m_particleSystemIDs.clear();
	m_active = FALSE;

	//Added By Sadullah Nader
	m_curBodyState = BODY_PRISTINE;
}

//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::onObjectCreated()
{
	static NameKeyType key_BoneFXDamage = NAMEKEY("BoneFXDamage");
	BoneFXDamage* bfxd = (BoneFXDamage*)getObject()->findDamageModule(key_BoneFXDamage);
	if (bfxd == NULL)
	{
		DEBUG_CRASH(("BoneFXUpdate requires BoneFXDamage"));
		throw INI_INVALID_DATA;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BoneFXUpdate::~BoneFXUpdate( void )
{
	killRunningParticleSystems();
}

//-------------------------------------------------------------------------------------------------
/** Parse fx location info ... that is a named bone */
//-------------------------------------------------------------------------------------------------
static void parseFXLocInfo( INI *ini, void *instance, BoneLocInfo *locInfo )
{
	const char *token = ini->getNextToken( ini->getSepsColon() );

	if( stricmp( token, "bone" ) == 0 )
	{

		// save bone name and location type
		locInfo->boneName = ini->getNextToken();

	}  // end if
	else
	{

		// error
		throw INI_INVALID_DATA;

	}  // end else

}  // end parseFXLocInfo

//-------------------------------------------------------------------------------------------------
/** Parse a random delay.  This is a number pair, where the numbers are a min and max time in miliseconds. */
//-------------------------------------------------------------------------------------------------
static void parseGameClientRandomDelay( INI *ini, void *instance, GameClientRandomVariable *delay)
{
	Real min, max;
	INI::parseDurationReal(ini, instance, &min, NULL);
	INI::parseDurationReal(ini, instance, &max, NULL);

	delay->setRange(min, max, GameClientRandomVariable::DistributionType::UNIFORM);
}

static void parseGameLogicRandomDelay( INI *ini, void *instance, GameLogicRandomVariable *delay)
{
	Real min, max;
	INI::parseDurationReal(ini, instance, &min, NULL);
	INI::parseDurationReal(ini, instance, &max, NULL);

	delay->setRange(min, max, GameLogicRandomVariable::DistributionType::UNIFORM);
}

//-------------------------------------------------------------------------------------------------
/** In the form of:
	* <BodyDamageState>FXList<index> = Bone:<BoneName> OnlyOnce:<Yes|No> <Min delay> <Max delay> FXList:<FXListName> */
//-------------------------------------------------------------------------------------------------
void BoneFXUpdateModuleData::parseFXList( INI *ini, void *instance, 
																								void *store, const void *userData )
{
	const char *token;
	BoneFXListInfo *info = (BoneFXListInfo *)store;

	// parse the location bone or location
	parseFXLocInfo( ini, instance, &info->locInfo );

	// make sure we have an "OnlyOnce:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if (stricmp( token, "onlyonce" ) != 0)
	{

		// error
		throw INI_INVALID_DATA;

	} // end if

	ini->parseBool( ini, instance, &info->onlyOnce, NULL);

	parseGameLogicRandomDelay( ini, instance, &info->gameLogicDelay);

	// make sure we have an "FXList:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if( stricmp( token, "fxlist" ) != 0 )
	{

		// error
		throw INI_INVALID_DATA;

	}  // end if

	// parse the fx list name
	ini->parseFXList( ini, instance, &info->fx, NULL );

}  // end parseFXList

//-------------------------------------------------------------------------------------------------
/** In the form of:
	* <BodyDamageState>OCL<index> = Bone:<BoneName> OnlyOnce:<Yes|No> <Min delay> <Max delay> OCL:<OCLName> */
//-------------------------------------------------------------------------------------------------
void BoneFXUpdateModuleData::parseObjectCreationList( INI *ini, void *instance, 
																														void *store, const void *userData )
{
	const char *token;
	BoneOCLInfo *info = (BoneOCLInfo *)store;

	// parse the location bone or location
	parseFXLocInfo( ini, instance, &info->locInfo );

	// make sure we have an "OnlyOnce:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if (stricmp( token, "onlyonce" ) != 0)
	{

		// error
		throw INI_INVALID_DATA;

	} // end if

	ini->parseBool( ini, instance, &info->onlyOnce, NULL );

	parseGameLogicRandomDelay(ini, instance, &info->gameLogicDelay);

	// make sure we have an "OCL:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if( stricmp( token, "ocl" ) != 0 )
	{

		// error
		throw INI_INVALID_DATA;

	}  // end if

	// parse the ocl name
	ini->parseObjectCreationList( ini, instance, &info->ocl, NULL );

}  // end parseObjectCreationList

//-------------------------------------------------------------------------------------------------
/** In the form of:
	* <BodyDamageState>ParticleSystem<index> = <Bone:BoneName> OnlyOnce:<Yes|No> <Min delay> <Max delay> PSys:<PSysName> */
//-------------------------------------------------------------------------------------------------
void BoneFXUpdateModuleData::parseParticleSystem( INI *ini, void *instance, 
																												void *store, const void *userData )
{
	const char *token;
	BoneParticleSystemInfo *info = (BoneParticleSystemInfo *)store;

	// parse the location bone or location
	parseFXLocInfo( ini, instance, &info->locInfo );

	// make sure we have an "OnlyOnce:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if (stricmp( token, "onlyonce" ) != 0)
	{

		// error
		throw INI_INVALID_DATA;

	} // end if

	ini->parseBool( ini, instance, &info->onlyOnce, NULL );

	parseGameClientRandomDelay(ini, instance, &info->gameClientDelay);

	// make sure we have an "PSys:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if( stricmp( token, "psys" ) != 0 )
	{

		// error
		throw INI_INVALID_DATA;

	}  // end if

	// parse the particle system name
	ini->parseParticleSystemTemplate( ini, instance, &info->particleSysTemplate, NULL );

}  // end parseParticleSystem

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime BoneFXUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here
	const BoneFXUpdateModuleData *d = getBoneFXUpdateModuleData();
	Int now = TheGameLogic->getFrame();

	if (m_active == FALSE) {
		initTimes();
		m_active = TRUE;
	}

	for (Int i = 0; i < BONE_FX_MAX_BONES; ++i) {
		//Check to see if its time to fire off any cool stuff.
		if ((m_nextFXFrame[m_curBodyState][i] != -1) && (m_nextFXFrame[m_curBodyState][i] <= now)) {
			doFXListAtBone(d->m_fxList[m_curBodyState][i].fx, &(m_FXBonePositions[m_curBodyState][i]));
			computeNextLogicFXTime(&(d->m_fxList[m_curBodyState][i]), m_nextFXFrame[m_curBodyState][i]);
		}
		if ((m_nextOCLFrame[m_curBodyState][i] != -1) && (m_nextOCLFrame[m_curBodyState][i] <= now)) {
			doOCLAtBone(d->m_OCL[m_curBodyState][i].ocl, &(m_OCLBonePositions[m_curBodyState][i]));
			computeNextLogicFXTime(&(d->m_OCL[m_curBodyState][i]), m_nextOCLFrame[m_curBodyState][i]);
		}
		if ((m_nextParticleSystemFrame[m_curBodyState][i] != -1) && (m_nextParticleSystemFrame[m_curBodyState][i] <= now)) {
			doParticleSystemAtBone(d->m_particleSystem[m_curBodyState][i].particleSysTemplate, &(m_PSBonePositions[m_curBodyState][i]));
			computeNextClientFXTime(&(d->m_particleSystem[m_curBodyState][i]), m_nextParticleSystemFrame[m_curBodyState][i]);
		}
	}
	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::initTimes() {
	Int i;
	const BoneFXUpdateModuleData *d = getBoneFXUpdateModuleData();
	Int now = TheGameLogic->getFrame();

	for (i = 0; i < BONE_FX_MAX_BONES; ++i) {
		if (d->m_fxList[m_curBodyState][i].locInfo.boneName.compare(AsciiString::TheEmptyString) != 0) {
			m_nextFXFrame[m_curBodyState][i] = now + REAL_TO_INT(d->m_fxList[m_curBodyState][i].gameLogicDelay.getValue());
		} else {
			m_nextFXFrame[m_curBodyState][i] = -1;
		}
		if (d->m_OCL[m_curBodyState][i].locInfo.boneName.compare(AsciiString::TheEmptyString) != 0) {
			m_nextOCLFrame[m_curBodyState][i] = now + REAL_TO_INT(d->m_OCL[m_curBodyState][i].gameLogicDelay.getValue());
		} else {
			m_nextOCLFrame[m_curBodyState][i] = -1;
		}
		if (d->m_particleSystem[m_curBodyState][i].locInfo.boneName.compare(AsciiString::TheEmptyString) != 0) {
			m_nextParticleSystemFrame[m_curBodyState][i] = now + REAL_TO_INT(d->m_particleSystem[m_curBodyState][i].gameClientDelay.getValue());
		} else {
			m_nextParticleSystemFrame[m_curBodyState][i] = -1;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
inline Bool inList(Int value, Int count, const Int idxList[])
{
	for (Int j = 0; j < count; ++j)
	{
		if (idxList[j] == value)
			return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
static void buildNonDupRandomIndexList(Int range, Int count, Int idxList[])
{
	for (Int i = 0; i < count; ++i)
	{
		Int idx;
		do
		{
			idx = GameLogicRandomValue(0, range-1);
		} 
		while (inList(idx, i, idxList));
		idxList[i] = idx;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::changeBodyDamageState(BodyDamageType oldState, BodyDamageType newState)
{
	m_curBodyState = newState;
	killRunningParticleSystems();
	initTimes();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::doFXListAtBone(const FXList *fxList, const Coord3D *bonePosition)
{
	if (m_bonesResolved[m_curBodyState] == FALSE) {
		resolveBoneLocations();
	}

	// if we are restricted by the damage type executing effect, bail out of here
	const BoneFXUpdateModuleData *d = getBoneFXUpdateModuleData();
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();
	if( lastDamageInfo && getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) == FALSE )
		return;

	// the bonePosition variable will have been made right by the call to
	// resolveBoneLocations.  Either that or it was correct to begin with.
	Object *building = getObject();

	// Convert the bone's position relative to the origin of the building to the current
	// bone position in the world.
	Coord3D newPos;
	building->convertBonePosToWorldPos(bonePosition, NULL, &newPos, NULL);

	// execute the fx list at the calculated bone position.
	FXList::doFXPos(fxList, &newPos, NULL);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::doOCLAtBone(const ObjectCreationList *ocl, const Coord3D *bonePosition)
{
	if (m_bonesResolved[m_curBodyState] == FALSE) {
		resolveBoneLocations();
	}

	// if we are restricted by the damage type executing effect, bail out of here
	const BoneFXUpdateModuleData *d = getBoneFXUpdateModuleData();
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();
	if( lastDamageInfo && getDamageTypeFlag( d->m_damageOCLTypes, lastDamageInfo->in.m_damageType ) == FALSE )
		return;

	// the bonePosition variable will have been made right by the call to
	// resolveBoneLocations.  Either that or it was correct to begin with.
	Object *building = getObject();

	Coord3D newPos;
	building->convertBonePosToWorldPos(bonePosition, NULL, &newPos, NULL);

	ObjectCreationList::create( ocl, building, &newPos, NULL );

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::doParticleSystemAtBone(const ParticleSystemTemplate *particleSystemTemplate, const Coord3D *bonePosition)
{
	if (m_bonesResolved[m_curBodyState] == FALSE) {
		resolveBoneLocations();
	}

	// if we are restricted by the damage type executing effect, bail out of here
	const BoneFXUpdateModuleData *d = getBoneFXUpdateModuleData();
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();
	if( lastDamageInfo && getDamageTypeFlag( d->m_damageParticleTypes, lastDamageInfo->in.m_damageType ) == FALSE )
		return;

	Object *building = getObject();

	ParticleSystem *psys = TheParticleSystemManager->createParticleSystem(particleSystemTemplate);
	if (psys != NULL) 
	{
		m_particleSystemIDs.push_back(psys->getSystemID());
		psys->setPosition(bonePosition);
		psys->attachToObject(building);
		Drawable *drawable = building->getDrawable();
		if (drawable && drawable->isDrawableEffectivelyHidden()) 
		{
			psys->stop();
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::computeNextClientFXTime(const BaseBoneListInfo *info, Int &nextFrame)
{
	if (info->onlyOnce) {
		nextFrame = -1;
		return;
	}
	nextFrame = TheGameLogic->getFrame() + REAL_TO_INT(info->gameClientDelay.getValue());
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::computeNextLogicFXTime(const BaseBoneListInfo *info, Int &nextFrame)
{
	if (info->onlyOnce) {
		nextFrame = -1;
		return;
	}
	nextFrame = TheGameLogic->getFrame() + REAL_TO_INT(info->gameLogicDelay.getValue());
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::killRunningParticleSystems() {
	for (std::vector<ParticleSystemID>::iterator it = m_particleSystemIDs.begin(); it != m_particleSystemIDs.end(); ++it)
	{
		ParticleSystem *sys = TheParticleSystemManager->findParticleSystem(*it);
		if( sys )
			sys->destroy();
	}

	m_particleSystemIDs.clear();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// This function is going to suck lots of time, should only be called once.
void BoneFXUpdate::resolveBoneLocations() {
	Int i;
	const BoneFXUpdateModuleData *d = getBoneFXUpdateModuleData();
	Object *building = getObject();
	if (building == NULL) {
		DEBUG_ASSERTCRASH(building != NULL, ("There is no object?"));
		return;
	}

	Drawable *drawable = building->getDrawable();
	if (drawable == NULL) {
		DEBUG_ASSERTCRASH(drawable != NULL, ("There is no drawable?"));
		return;
	}

	for (i = 0; i < BONE_FX_MAX_BONES; ++i) {
		if (d->m_fxList[m_curBodyState][i].locInfo.boneName.compare(AsciiString::TheEmptyString) != 0) 
		{
			const BoneFXListInfo *info = &(d->m_fxList[m_curBodyState][i]);
			drawable->getPristineBonePositions(info->locInfo.boneName.str(), 0, &m_FXBonePositions[m_curBodyState][i], NULL, 1);
		}

		if (d->m_OCL[m_curBodyState][i].locInfo.boneName.compare(AsciiString::TheEmptyString) != 0) 
		{
			const BoneOCLInfo *info = &(d->m_OCL[m_curBodyState][i]);
			drawable->getPristineBonePositions(info->locInfo.boneName.str(), 0, &m_OCLBonePositions[m_curBodyState][i], NULL, 1);
		}

		if (d->m_particleSystem[m_curBodyState][i].locInfo.boneName.compare(AsciiString::TheEmptyString) != 0) 
		{
			const BoneParticleSystemInfo *info = &(d->m_particleSystem[m_curBodyState][i]);
			drawable->getPristineBonePositions(info->locInfo.boneName.str(), 0, &m_PSBonePositions[m_curBodyState][i], NULL, 1);
		}
	}
	m_bonesResolved[m_curBodyState] = TRUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BoneFXUpdate::stopAllBoneFX() {
	int i, j;
	for (i = 0; i < BODYDAMAGETYPE_COUNT; ++i) {
		for (j = 0; j < BONE_FX_MAX_BONES; ++j) {
			m_nextFXFrame[i][j] = -1;
			m_nextOCLFrame[i][j] = -1;
			m_nextParticleSystemFrame[i][j] = -1;
		}
	}
	killRunningParticleSystems();
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BoneFXUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BoneFXUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// particle system vector count and data
	UnsignedShort particleSystemCount = m_particleSystemIDs.size();
	xfer->xferUnsignedShort( &particleSystemCount );
	ParticleSystemID systemID;
	if( xfer->getXferMode() == XFER_SAVE )
	{
		std::vector<ParticleSystemID>::const_iterator it;

		for( it = m_particleSystemIDs.begin(); it != m_particleSystemIDs.end(); ++it )
		{

			systemID = *it;
			xfer->xferUser( &systemID, sizeof( ParticleSystemID ) );

		}  // end for

	}  // end if, save
	else
	{

		// the list should be emtpy right now
		if( m_particleSystemIDs.empty() == FALSE )
		{

			DEBUG_CRASH(( "BoneFXUpdate::xfer - m_particleSystemIDs should be empty but is not" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read all data
		for( UnsignedShort i = 0; i < particleSystemCount; ++i )
		{

			// read id
			xfer->xferUser( &systemID, sizeof( ParticleSystemID ) );

			// put at end of vector
			m_particleSystemIDs.push_back( systemID );

		}  // end for, i

	}  // end else

	// next fx frame
	xfer->xferUser( m_nextFXFrame, sizeof( Int ) * BODYDAMAGETYPE_COUNT * BONE_FX_MAX_BONES );
	
	// next OCL farme
	xfer->xferUser( m_nextOCLFrame, sizeof( Int ) * BODYDAMAGETYPE_COUNT * BONE_FX_MAX_BONES );

	// next particle system frame
	xfer->xferUser( m_nextParticleSystemFrame, sizeof( Int ) * BODYDAMAGETYPE_COUNT * BONE_FX_MAX_BONES );

	// fx bone positions
	xfer->xferUser( m_FXBonePositions, sizeof( Coord3D ) * BODYDAMAGETYPE_COUNT * BONE_FX_MAX_BONES );

	// ocl bone positions
	xfer->xferUser( m_OCLBonePositions, sizeof( Coord3D ) * BODYDAMAGETYPE_COUNT * BONE_FX_MAX_BONES );

	// particle system bone positions
	xfer->xferUser( m_PSBonePositions, sizeof( Coord3D ) * BODYDAMAGETYPE_COUNT * BONE_FX_MAX_BONES );

	// current body state
	xfer->xferUser( &m_curBodyState, sizeof( BodyDamageType ) );

	// bones resolved
	xfer->xferUser( m_bonesResolved, sizeof( Bool ) * BODYDAMAGETYPE_COUNT );

	// active
	xfer->xferBool( &m_active );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BoneFXUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
