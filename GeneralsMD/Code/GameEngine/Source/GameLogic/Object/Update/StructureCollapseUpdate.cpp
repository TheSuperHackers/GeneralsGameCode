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

// FILE: StructureCollapseUpdate.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:  
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/RandomValue.h"
#include "Common/GlobalData.h"
#include "Common/Xfer.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BoneFXUpdate.h"
#include "GameLogic/Module/StructureCollapseUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"

const Int MAX_IDX = 32;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
static const char *TheStructureCollapsePhaseNames[] = 
{
	"INITIAL",
	"DELAY",
	"BURST",
	"FINAL",

	NULL
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StructureCollapseUpdate::StructureCollapseUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_collapseFrame = 0;
	m_collapseState = COLLAPSESTATE_STANDING;
	m_collapseVelocity = 0.0f;
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_burstFrame = 0;
	m_currentHeight = 0.0f;
	//
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StructureCollapseUpdate::~StructureCollapseUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
static void parseFX( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ )
{
	StructureCollapseUpdateModuleData* self = (StructureCollapseUpdateModuleData*)instance;
	StructureCollapsePhaseType scphase = (StructureCollapsePhaseType)INI::scanIndexList(ini->getNextToken(), TheStructureCollapsePhaseNames);
	for (const char* token = ini->getNextToken(); token != NULL; token = ini->getNextTokenOrNull())
	{
		const FXList *fxl = TheFXListStore->findFXList((token));	// could be null! this is OK!
		self->m_fxs[scphase].push_back(fxl);
	}
}

//-------------------------------------------------------------------------------------------------
static void parseOCL( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ )
{
	StructureCollapseUpdateModuleData* self = (StructureCollapseUpdateModuleData*)instance;
	StructureCollapsePhaseType stphase = (StructureCollapsePhaseType)INI::scanIndexList(ini->getNextToken(), TheStructureCollapsePhaseNames);
	for (const char* token = ini->getNextToken(); token != NULL; token = ini->getNextTokenOrNull())
	{
		const ObjectCreationList *ocl = TheObjectCreationListStore->findObjectCreationList(token);	// could be null! this is OK!
		self->m_ocls[stphase].push_back(ocl);
	}
}

//-------------------------------------------------------------------------------------------------
/*static*/ void StructureCollapseUpdateModuleData::buildFieldParse(MultiIniFieldParse& p) 
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "MinCollapseDelay",						INI::parseDurationUnsignedInt,		NULL, offsetof( StructureCollapseUpdateModuleData, m_minCollapseDelay ) },
		{ "MaxCollapseDelay",						INI::parseDurationUnsignedInt,		NULL, offsetof( StructureCollapseUpdateModuleData, m_maxCollapseDelay ) },
		{ "MinBurstDelay",							INI::parseDurationUnsignedInt,		NULL, offsetof( StructureCollapseUpdateModuleData, m_minBurstDelay ) },
		{ "MaxBurstDelay",							INI::parseDurationUnsignedInt,		NULL, offsetof( StructureCollapseUpdateModuleData, m_maxBurstDelay ) },
		{ "CollapseDamping",						INI::parseReal,										NULL, offsetof( StructureCollapseUpdateModuleData, m_collapseDamping ) },
		{ "MaxShudder",									INI::parseReal,										NULL, offsetof( StructureCollapseUpdateModuleData, m_maxShudder ) },
		{ "BigBurstFrequency",					INI::parseInt,										NULL, offsetof( StructureCollapseUpdateModuleData, m_bigBurstFrequency ) },
		{ "OCL",												parseOCL,													NULL, 0 },
		{ "FXList",											parseFX,													NULL, 0 },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
	p.add(DieMuxData::getFieldParse(), offsetof( StructureCollapseUpdateModuleData, m_dieMuxData ));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureCollapseUpdate::beginStructureCollapse(const DamageInfo *damageInfo)
{
	const StructureCollapseUpdateModuleData *d = getStructureCollapseUpdateModuleData();


	Object *building = getObject();
	UnsignedInt now = TheGameLogic->getFrame();
	// This has to use a game logic random value since the bursts can spawn debris, and debris is sync'd.
	m_collapseFrame = now + GameLogicRandomValue(d->m_minCollapseDelay, d->m_maxCollapseDelay);

	doPhaseStuff(SCPHASE_INITIAL, building->getPosition());

	m_collapseState = COLLAPSESTATE_WAITINGFORCOLLAPSESTART;
	m_currentHeight = 0.0f;

	setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureCollapseUpdate::onDie( const DamageInfo *damageInfo )
{
	const StructureCollapseUpdateModuleData* d = getStructureCollapseUpdateModuleData();
	if (!d->m_dieMuxData.isDieApplicable(getObject(), damageInfo))
		return;

	AIUpdateInterface *ai = getObject()->getAIUpdateInterface();
	if (ai)
		ai->markAsDead();

	// deselect this object for all players.
	TheGameLogic->deselectObject(getObject(), PLAYERMASK_ALL, TRUE);

	beginStructureCollapse(damageInfo);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime StructureCollapseUpdate::update( void )
{
	static const Real COLLAPSE_ACCELERATION_FACTOR = 0.02f;
	const StructureCollapseUpdateModuleData *d = getStructureCollapseUpdateModuleData();

	if (m_collapseState == COLLAPSESTATE_STANDING)
	{
		DEBUG_CRASH(("hmm, what?"));
		return UPDATE_SLEEP_FOREVER;
	}

	// We are in the dramatic pause between when the building has lost all its hit points and
	// when it starts toppling over.
	if (m_collapseState == COLLAPSESTATE_WAITINGFORCOLLAPSESTART) 
	{
		UnsignedInt now = TheGameLogic->getFrame();
		Object *building = getObject();

		const Coord3D *currentPosition = building->getPosition();
		Vector3 shudder;
		shudder.Set(GameClientRandomValueReal(-(d->m_maxShudder), d->m_maxShudder), GameClientRandomValueReal(-(d->m_maxShudder), d->m_maxShudder), 0);

		const Matrix3D *instMatrix = building->getDrawable()->getInstanceMatrix();
		Matrix3D newInstMatrix;
		newInstMatrix = *instMatrix;
		newInstMatrix.Set_Translation(shudder);

		building->getDrawable()->setInstanceMatrix(&newInstMatrix);

		if (now >= m_collapseFrame) 
		{
			m_collapseState = COLLAPSESTATE_COLLAPSING;
			doPhaseStuff(SCPHASE_BURST, currentPosition);
			// This has to use a game logic random value since the bursts can spawn debris, and debris is sync'd.
			m_burstFrame = now + GameLogicRandomValue(d->m_minBurstDelay, d->m_maxBurstDelay);
		}
	}

	// The building is in the process of falling over.
	if (m_collapseState == COLLAPSESTATE_COLLAPSING) 
	{
		Object *building = getObject();
		UnsignedInt now = TheGameLogic->getFrame();
		m_currentHeight -= m_collapseVelocity;
		m_collapseVelocity -= TheGlobalData->m_gravity * (1.0 - d->m_collapseDamping);

		const Coord3D *currentPosition = building->getPosition();
		Vector3 shudder;
		shudder.Set(GameClientRandomValueReal(-(d->m_maxShudder), d->m_maxShudder), GameClientRandomValueReal(-(d->m_maxShudder), d->m_maxShudder), m_currentHeight);
		const Matrix3D *instMatrix = building->getDrawable()->getInstanceMatrix();
		Matrix3D newInstMatrix;
		newInstMatrix = *instMatrix;
		newInstMatrix.Set_Translation(shudder);

		building->getDrawable()->setInstanceMatrix(&newInstMatrix);

		if (now >= m_burstFrame) 
		{
			if (GameLogicRandomValue(1, d->m_bigBurstFrequency) == 1) 
			{
				doPhaseStuff(SCPHASE_BURST, currentPosition);
			} 
			else 
			{
				doPhaseStuff(SCPHASE_DELAY, currentPosition);
			}
			// This has to use a game logic random value since the bursts can spawn debris, and debris is sync'd.
			m_burstFrame += GameLogicRandomValue(d->m_minBurstDelay, d->m_maxBurstDelay);
		}

//		if ((m_currentHeight + building->getGeometryInfo().getMaxHeightAbovePosition()) <= 0) 
		if ((m_currentHeight + building->getTemplate()->getTemplateGeometryInfo().getMaxHeightAbovePosition()) <= 0)
		{
			m_collapseState = COLLAPSESTATE_DONE;
			doPhaseStuff(SCPHASE_FINAL, building->getPosition());
			Drawable *drawable = building->getDrawable();

			doCollapseDoneStuff();

			drawable->clearModelConditionState(MODELCONDITION_RUBBLE);
			drawable->setModelConditionState(MODELCONDITION_POST_COLLAPSE);
			building->setOrientation(building->getOrientation());

			
			// Need to update body particle systems, now
			BodyModuleInterface *body = building->getBodyModule();
			body->updateBodyParticleSystems();


			Vector3 shudder;
			shudder.Set(0, 0, 0);
			const Matrix3D *instMatrix = building->getDrawable()->getInstanceMatrix();
			Matrix3D newInstMatrix;
			newInstMatrix = *instMatrix;
			newInstMatrix.Set_Translation(shudder);
			building->getDrawable()->setInstanceMatrix(&newInstMatrix);

			return UPDATE_SLEEP_FOREVER;
		}
	}

	return UPDATE_SLEEP_NONE;
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
void StructureCollapseUpdate::doPhaseStuff(StructureCollapsePhaseType scphase, const Coord3D *target)
{
	DEBUG_LOG(("Firing phase %d on frame %d", scphase, TheGameLogic->getFrame()));

	const StructureCollapseUpdateModuleData* d = getStructureCollapseUpdateModuleData();
	Int i, idx, count, listSize;
	Int idxList[MAX_IDX];

	listSize = d->m_fxs[scphase].size();
	if (listSize > 0)
	{
		count = d->m_fxCount[scphase];
		buildNonDupRandomIndexList(listSize, count, idxList);
		for (i = 0; i < count; ++i)
		{
			idx = idxList[i];
			const FXVec& v = d->m_fxs[scphase];
			DEBUG_ASSERTCRASH(idx>=0&&idx<v.size(),("bad idx"));
			const FXList* fxl = v[idx];
			FXList::doFXPos(fxl, target);
		}
	}

	listSize = d->m_ocls[scphase].size();
	if (listSize > 0)
	{
		count = d->m_oclCount[scphase];
		buildNonDupRandomIndexList(listSize, count, idxList);
		for (i = 0; i < count; ++i)
		{
			idx = idxList[i];
			const OCLVec& v = d->m_ocls[scphase];
			DEBUG_ASSERTCRASH(idx>=0&&idx<v.size(),("bad idx"));
			const ObjectCreationList* ocl = v[idx];
			ObjectCreationList::create(ocl, getObject(), target, NULL, getObject()->getOrientation() );
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureCollapseUpdate::doCollapseDoneStuff() 
{
	static NameKeyType key_BoneFXUpdate = NAMEKEY("BoneFXUpdate");
	BoneFXUpdate *bfxu = (BoneFXUpdate *)getObject()->findUpdateModule(key_BoneFXUpdate);
	if (bfxu != NULL) 
	{
		bfxu->stopAllBoneFX();
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StructureCollapseUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StructureCollapseUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// collapse frame
	xfer->xferUnsignedInt( &m_collapseFrame );

	// burst frame
	xfer->xferUnsignedInt( &m_burstFrame );

	// collapse state
	xfer->xferUser( &m_collapseState, sizeof( StructureCollapseStateType ) );

	// collapse velocity
	xfer->xferReal( &m_collapseVelocity );

	// current height
	xfer->xferReal( &m_currentHeight );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StructureCollapseUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
