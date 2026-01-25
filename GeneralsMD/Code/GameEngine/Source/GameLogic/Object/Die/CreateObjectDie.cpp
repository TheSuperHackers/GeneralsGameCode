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

// FILE: CreateObjectDie.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Michael S. Booth, January 2002
// Desc:   Create an object upon this object's death
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_OBJECT_STATUS_NAMES
#include "GameLogic/Module/AIUpdate.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/CreateObjectDie.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameClient/InGameUI.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
CreateObjectDieModuleData::CreateObjectDieModuleData()
{

	m_ocl = nullptr;
	m_transferPreviousHealth = FALSE;
	m_transferSelection = FALSE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void CreateObjectDieModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	DieModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "CreationList",	INI::parseObjectCreationList,		nullptr,											offsetof( CreateObjectDieModuleData, m_ocl ) },
		{ "TransferPreviousHealth", INI::parseBool, nullptr	,offsetof( CreateObjectDieModuleData, m_transferPreviousHealth ) },
		{ "TransferSelection", INI::parseBool, nullptr, offsetof( CreateObjectDieModuleData, m_transferSelection ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add(dataFieldParse);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CreateObjectDie::CreateObjectDie( Thing *thing, const ModuleData* moduleData ) : DieModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CreateObjectDie::~CreateObjectDie( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void CreateObjectDie::onDie( const DamageInfo * damageInfo )
{
	const CreateObjectDieModuleData *data = getCreateObjectDieModuleData();
	if (!isDieApplicable(damageInfo))
		return;

	Object *damageDealer = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );

	Object *newObject = ObjectCreationList::create( data->m_ocl, getObject(), damageDealer );
	if (!newObject)
		return;

	//If we're transferring previous health, we're transfering the last known
	//health before we died. In the case of the sneak attack tunnel network, it
	//is killed after the lifetime update expires.
	if( data->m_transferPreviousHealth )
	{
		//Convert old health to new health.
		Object *oldObject = getObject();
		BodyModuleInterface *oldBody = oldObject->getBodyModule();
		BodyModuleInterface *newBody = newObject->getBodyModule();
		if( oldBody && newBody )
		{
			//First transfer subdual damage
			DamageInfo damInfo;
			Real subdualDamageAmount = oldBody->getCurrentSubdualDamageAmount();
			if( subdualDamageAmount > 0.0f )
			{
				damInfo.in.m_amount = subdualDamageAmount;
				damInfo.in.m_damageType = DAMAGE_SUBDUAL_UNRESISTABLE;
				damInfo.in.m_sourceID = INVALID_ID;
				newBody->attemptDamage( &damInfo );
			}

			//Now transfer the previous health from the old object to the new.
			damInfo.in.m_amount = oldBody->getMaxHealth() - oldBody->getPreviousHealth();
			damInfo.in.m_damageType = DAMAGE_UNRESISTABLE;
			damInfo.in.m_sourceID = oldBody->getLastDamageInfo()->in.m_sourceID;
			if( damInfo.in.m_amount > 0.0f )
			{
				newBody->attemptDamage( &damInfo );
			}

		}

		//Transfer attackers.
		for( Object *obj = TheGameLogic->getFirstObject(); obj; obj = obj->getNextObject() )
		{
			AIUpdateInterface* ai = obj->getAI();
			if (!ai)
				continue;

			ai->transferAttack( oldObject->getID(), newObject->getID() );
		}
	}

	// TheSuperHackers @bugfix Stubbjax 02/10/2025 If the old object was selected, select the new one.
	// This is important for the Sneak Attack, which is spawned via a CreateObjectDie module.
	if (data->m_transferSelection)
	{
		Object* oldObject = getObject();
		Drawable* selectedDrawable = TheInGameUI->getFirstSelectedDrawable();
		Bool oldObjectSelected = selectedDrawable && selectedDrawable->getID() == oldObject->getDrawable()->getID();

		if (oldObjectSelected)
		{
			GameMessage* msg = TheMessageStream->appendMessage(GameMessage::MSG_CREATE_SELECTED_GROUP_NO_SOUND);
			msg->appendBooleanArgument(TRUE);
			msg->appendObjectIDArgument(newObject->getID());
			TheInGameUI->selectDrawable(newObject->getDrawable());
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CreateObjectDie::crc( Xfer *xfer )
{

	// extend base class
	DieModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CreateObjectDie::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DieModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CreateObjectDie::loadPostProcess( void )
{

	// extend base class
	DieModule::loadPostProcess();

}
