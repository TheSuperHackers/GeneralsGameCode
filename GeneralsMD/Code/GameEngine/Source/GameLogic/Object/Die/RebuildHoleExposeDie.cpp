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

// FILE: RebuildHoleExposeDie.cpp /////////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   When a structure dies with this module, a rebuild hole will be created in place
//				 of the structure that will automatically rebuild the structure
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDE FILES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/RebuildHoleBehavior.h"
#include "GameLogic/Module/RebuildHoleExposeDie.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"
#include "GameClient/SelectionXlat.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RebuildHoleExposeDieModuleData::RebuildHoleExposeDieModuleData()
{

	m_holeMaxHealth = 0.0f;
	m_transferAttackers = true;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void RebuildHoleExposeDieModuleData::buildFieldParse( MultiIniFieldParse &p )
{
	DieModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "HoleName", INI::parseAsciiString, NULL, offsetof( RebuildHoleExposeDieModuleData, m_holeName ) },
		{ "HoleMaxHealth", INI::parseReal, NULL, offsetof( RebuildHoleExposeDieModuleData, m_holeMaxHealth ) },
		{ "TransferAttackers", INI::parseBool, NULL, offsetof( RebuildHoleExposeDieModuleData, m_transferAttackers ) },
		{ 0, 0, 0, 0 }
	};

	p.add( dataFieldParse );

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RebuildHoleExposeDie::RebuildHoleExposeDie( Thing *thing, const ModuleData* moduleData )
										: DieModule( thing, moduleData )
{

}  // end RebuildHoleExposeDie

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RebuildHoleExposeDie::~RebuildHoleExposeDie( void )
{

}  // end ~RebuildHoleExposeDie

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void RebuildHoleExposeDie::onDie( const DamageInfo *damageInfo )
{
	if (!isDieApplicable(damageInfo))
		return;
  

#if defined(RTS_DEBUG) || defined(_INTERNAL) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
  if(TheSelectionTranslator->isHandOfGodSelectionMode())
  {
    if ( getObject()->isKindOf( KINDOF_STRUCTURE ) )
    {
      if ( damageInfo->in.m_damageType == DAMAGE_UNRESISTABLE )
        return;
    }
  }
#endif





	const RebuildHoleExposeDieModuleData *modData = getRebuildHoleExposeDieModuleData();
	Object *us = getObject();

	//
	// if we are being constructed from either the first time or from a hole reconstruction
	// we do not "spawn" a hole object
	//
	if( us->getControllingPlayer() != ThePlayerList->getNeutralPlayer() 
		  && us->getControllingPlayer()->isPlayerActive() 
			&& !us->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) )
	{
		Object *hole;

		// create the hole
		hole = TheThingFactory->newObject( TheThingFactory->findTemplate( modData->m_holeName ),
																			 getObject()->getTeam() );

		// put the hole at our position and angle
		hole->setPosition( us->getPosition() );
		hole->setOrientation( us->getOrientation() );

		//
		// modify the hole extents to be the same as ours because we need to preserve the
		// same amount of space for the rebuilding process
		//
		hole->setGeometryInfo( us->getGeometryInfo() );

		//
		// Transfer the building's name to the hole
		//
		TheScriptEngine->transferObjectName( us->getName(), hole );

		//
		// add to pathfind map, this really should be wrapped up somewhere in the creation
		// pipeline of the object automagically!
		//
		TheAI->pathfinder()->addObjectToPathfindMap( hole );

		// set the health of the hole to that defined by our data
		BodyModuleInterface *body = hole->getBodyModule();
		body->setMaxHealth( modData->m_holeMaxHealth );

		// set the information in the hole about what to build
		RebuildHoleBehaviorInterface *rhbi = RebuildHoleBehavior::getRebuildHoleBehaviorInterfaceFromObject( hole );

		// sanity
		DEBUG_ASSERTCRASH( rhbi, ("RebuildHoleExposeDie: No Rebuild Hole Behavior interface on hole\n") );

		// start the rebuild process
		if( rhbi )
			rhbi->startRebuildProcess( us->getTemplate(), us->getID() );

		if (modData->m_transferAttackers)
		{
			for ( Object *obj = TheGameLogic->getFirstObject(); obj; obj = obj->getNextObject() )
			{
				AIUpdateInterface* ai = obj->getAI();
				if (!ai)
					continue;

				ai->transferAttack(us->getID(), hole->getID());
			}
		}

	}  // end if

}  // end onDie

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RebuildHoleExposeDie::crc( Xfer *xfer )
{

	// extend base class
	DieModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RebuildHoleExposeDie::xfer( Xfer *xfer )
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
void RebuildHoleExposeDie::loadPostProcess( void )
{

	// extend base class
	DieModule::loadPostProcess();

}  // end loadPostProcess
