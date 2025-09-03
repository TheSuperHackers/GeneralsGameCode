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

// FILE: ReplaceObjectUpgrade.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, July 2003
// Desc:	 UpgradeModule that creates a new Object in our exact location and then deletes our object
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameLogic/Module/ReplaceObjectUpgrade.h"

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/CreateModule.h"
#include "GameLogic/Object.h"
#include "GameClient/InGameUI.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ReplaceObjectUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpgradeModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "ReplaceObject",	INI::parseAsciiString,	NULL, offsetof( ReplaceObjectUpgradeModuleData, m_replaceObjectName ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ReplaceObjectUpgrade::ReplaceObjectUpgrade( Thing *thing, const ModuleData* moduleData ) : UpgradeModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ReplaceObjectUpgrade::~ReplaceObjectUpgrade( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ReplaceObjectUpgrade::upgradeImplementation( )
{
	const ReplaceObjectUpgradeModuleData *data = getReplaceObjectUpgradeModuleData();
	const ThingTemplate* replacementTemplate = TheThingFactory->findTemplate(data->m_replaceObjectName);

	Bool oldObjectSelected;
	Int oldObjectSquadNumber;
	Matrix3D myMatrix;
	Team* myTeam;

	{
		Object* me = getObject();

		myMatrix = *me->getTransformMatrix();
		myTeam = me->getTeam();// Team implies player.  It is a subset.

		if (replacementTemplate == NULL)
		{
			DEBUG_ASSERTCRASH(replacementTemplate != NULL, ("No such object '%s' in ReplaceObjectUpgrade.", data->m_replaceObjectName.str()));
			return;
		}

		Drawable* selectedDrawable = TheInGameUI->getFirstSelectedDrawable();
		oldObjectSelected = selectedDrawable && selectedDrawable->getID() == me->getDrawable()->getID();
		oldObjectSquadNumber = me->getControllingPlayer()->getSquadNumberForObject(me);

		// Remove us first since occupation of cells is apparently not a refcount, but a flag.  If I don't remove, then the new
		// thing will be placed, and then on deletion I will remove "his" marks.
		TheAI->pathfinder()->removeObjectFromPathfindMap(me);
		TheGameLogic->destroyObject(me);
	}

	Object *replacementObject = TheThingFactory->newObject(replacementTemplate, myTeam);
	replacementObject->setTransformMatrix(&myMatrix);
	TheAI->pathfinder()->addObjectToPathfindMap( replacementObject );

	// Now onCreates were called at the constructor.  This magically created
	// thing needs to be considered as Built for Game specific stuff.
	for (BehaviorModule** m = replacementObject->getBehaviorModules(); *m; ++m)
	{
		CreateModuleInterface* create = (*m)->getCreate();
		if (!create)
			continue;
		create->onBuildComplete();
	}

	if( replacementObject->getControllingPlayer() )
	{
		replacementObject->getControllingPlayer()->onStructureConstructionComplete(NULL, replacementObject, FALSE);

		// TheSuperHackers @bugfix Stubbjax 26/05/2025 If the old object was selected, select the new one.
		if (oldObjectSelected)
		{
			GameMessage* msg = TheMessageStream->appendMessage(GameMessage::MSG_CREATE_SELECTED_GROUP_NO_SOUND);
			msg->appendBooleanArgument(TRUE);
			msg->appendObjectIDArgument(replacementObject->getID());
			TheInGameUI->selectDrawable(replacementObject->getDrawable());
		}

		// TheSuperHackers @bugfix Stubbjax 26/05/2025 If the old object was grouped, group the new one.
		if (oldObjectSquadNumber != NO_HOTKEY_SQUAD)
		{
			if (replacementObject->isLocallyControlled())
			{
				GameMessage* msg = TheMessageStream->appendMessage((GameMessage::Type)(GameMessage::MSG_CREATE_TEAM0 + oldObjectSquadNumber));
				msg->appendObjectIDArgument(replacementObject->getID());
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ReplaceObjectUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ReplaceObjectUpgrade::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpgradeModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ReplaceObjectUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}  // end loadPostProcess
