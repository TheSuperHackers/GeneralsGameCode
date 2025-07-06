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

// FILE: InstantDeathBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:  
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#define DEFINE_SLOWDEATHPHASE_NAMES

#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/RandomValue.h"
#include "Common/GameLOD.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/InstantDeathBehavior.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Weapon.h"
#include "GameClient/Drawable.h"

//-------------------------------------------------------------------------------------------------
InstantDeathBehaviorModuleData::InstantDeathBehaviorModuleData()
{
	// redundant.
	//m_fx.clear();
	//m_ocls.clear();
	//m_weapons.clear();
}

//-------------------------------------------------------------------------------------------------
static void parseFX( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ )
{
	InstantDeathBehaviorModuleData* self = (InstantDeathBehaviorModuleData*)instance;
	for (const char* token = ini->getNextToken(); token != NULL; token = ini->getNextTokenOrNull())
	{
		const FXList *fxl = TheFXListStore->findFXList((token));	// could be null! this is OK!
		self->m_fx.push_back(fxl);
	}
}

//-------------------------------------------------------------------------------------------------
static void parseOCL( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ )
{
	InstantDeathBehaviorModuleData* self = (InstantDeathBehaviorModuleData*)instance;
	for (const char* token = ini->getNextToken(); token != NULL; token = ini->getNextTokenOrNull())
	{
		const ObjectCreationList *ocl = TheObjectCreationListStore->findObjectCreationList(token);	// could be null! this is OK!
		self->m_ocls.push_back(ocl);
	}
}

//-------------------------------------------------------------------------------------------------
static void parseWeapon( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ )
{
	InstantDeathBehaviorModuleData* self = (InstantDeathBehaviorModuleData*)instance;
	for (const char* token = ini->getNextToken(); token != NULL; token = ini->getNextTokenOrNull())
	{
		const WeaponTemplate *wt = TheWeaponStore->findWeaponTemplate(token);	// could be null! this is OK!
		self->m_weapons.push_back(wt);
	}
}

//-------------------------------------------------------------------------------------------------
/*static*/ void InstantDeathBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p) 
{
  DieModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "FX",										parseFX,													NULL, 0 },
		{ "OCL",									parseOCL,													NULL, 0 },
		{ "Weapon",								parseWeapon,											NULL, 0 },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
InstantDeathBehavior::InstantDeathBehavior( Thing *thing, const ModuleData* moduleData ) : DieModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
InstantDeathBehavior::~InstantDeathBehavior( void )
{
}

//-------------------------------------------------------------------------------------------------
void InstantDeathBehavior::onDie( const DamageInfo *damageInfo )
{
	if (!isDieApplicable(damageInfo))
		return;

	AIUpdateInterface* ai = getObject()->getAIUpdateInterface();
	if (ai)
	{
		// has another AI already handled us. (hopefully another InstantDeathBehavior)
		if (ai->isAiInDeadState())
			return;
		ai->markAsDead();
	}

	const InstantDeathBehaviorModuleData* d = getInstantDeathBehaviorModuleData();

	size_t idx, listSize;

	listSize = d->m_fx.size();
	if (listSize > 0)
	{
		idx = (size_t)GameLogicRandomValue(0, listSize-1);
		const FXListVec& v = d->m_fx;
		DEBUG_ASSERTCRASH(idx>=0&&idx<v.size(),("bad idx"));
		const FXList* fxl = v[idx];
		FXList::doFXObj(fxl, getObject(), NULL);
	}

	listSize = d->m_ocls.size();
	if (listSize > 0)
	{
		idx = (size_t)GameLogicRandomValue(0, listSize-1);
		const OCLVec& v = d->m_ocls;
		DEBUG_ASSERTCRASH(idx>=0&&idx<v.size(),("bad idx"));
		const ObjectCreationList* ocl = v[idx];
		ObjectCreationList::create(ocl, getObject(), NULL);
	}

	listSize = d->m_weapons.size();
	if (listSize > 0)
	{
		idx = (size_t)GameLogicRandomValue(0, listSize-1);
		const WeaponTemplateVec& v = d->m_weapons;
		DEBUG_ASSERTCRASH(idx>=0&&idx<v.size(),("bad idx"));
		const WeaponTemplate* wt = v[idx];
		if (wt)
		{
			TheWeaponStore->createAndFireTempWeapon(wt, getObject(), getObject()->getPosition());
		}
	}

	TheGameLogic->destroyObject(getObject());
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void InstantDeathBehavior::crc( Xfer *xfer )
{

	// extend base class
	DieModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void InstantDeathBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DieModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void InstantDeathBehavior::loadPostProcess( void )
{

	// extend base class
	DieModule::loadPostProcess();

}  // end loadPostProcess
