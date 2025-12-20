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

// FILE: CreateCrateDie.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, February 2002
// Desc:   A chance to create a crate on death according to certain condition checks
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/RandomValue.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/CrateSystem.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/CreateCrateDie.h"
#include "GameLogic/Module/AIUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CreateCrateDie::CreateCrateDie( Thing *thing, const ModuleData* moduleData ) : DieModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CreateCrateDie::~CreateCrateDie( void )
{
}

void CreateCrateDieModuleData::parseCrateData( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ )
{
	CreateCrateDieModuleData* self = (CreateCrateDieModuleData*)instance;

	AsciiString crateName = ini->getNextToken();

	self->m_crateNameList.push_back( crateName );
}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void CreateCrateDie::onDie( const DamageInfo * damageInfo )
{
	if (!isDieApplicable(damageInfo))
		return;

	CrateTemplate const *currentCrateData = NULL;
	Object *killer = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );
	Object *me = getObject();

	if( killer && killer->getRelationship( me ) == ALLIES )
		return; //Nope, no crate for killing ally at all.

	for( AsciiStringListConstIterator iter = getCreateCrateDieModuleData()->m_crateNameList.begin();
				iter != getCreateCrateDieModuleData()->m_crateNameList.end();
				iter++
			)
	{
		currentCrateData = TheCrateSystem->findCrateTemplate( *iter );
		if( currentCrateData )
		{
			if( ! testCreationChance( currentCrateData ) )
				continue;// always test this

			if( (currentCrateData->m_veterancyLevel != LEVEL_INVALID) && ! testVeterancyLevel( currentCrateData ) )
				continue; //If this is set up to test and it fails

			if( KINDOFMASK_ANY_SET(currentCrateData->m_killedByTypeKindof) && !testKillerType( currentCrateData, killer ) )
				continue; //If this is set up to test and it fails

			if( (currentCrateData->m_killerScience != SCIENCE_INVALID)  &&  !testKillerScience( currentCrateData, killer ) )
				continue; //If this is set up to test and it fails

			Object *crate = createCrate( currentCrateData );//Make the crate
			if( crate )
			{
				// Design needs to set ownership of crates sometimes
				if( currentCrateData->m_isOwnedByMaker )
				{
					crate->setTeam( me->getControllingPlayer()->getDefaultTeam() );
				}

				if (killer)
				{
					// If the killer is a computer controlled player, notify that the crate exists.
					if (killer->getControllingPlayer() &&
						killer->getControllingPlayer()->getPlayerType()==PLAYER_COMPUTER)
					{
						AIUpdateInterface *ai = killer->getAIUpdateInterface();
						if (ai)
						{
							ai->notifyCrate( crate->getID() );
						}
					}
				}
			}
		}
	}
}

Bool CreateCrateDie::testCreationChance( CrateTemplate const *currentCrateData )
{
	Real testAgainst = currentCrateData->m_creationChance;
	Real testWith = GameLogicRandomValueReal( 0, 1 );

	return testWith < testAgainst;
}

Bool CreateCrateDie::testVeterancyLevel( CrateTemplate const *currentCrateData )
{
	VeterancyLevel testAgainst = currentCrateData->m_veterancyLevel;
	VeterancyLevel testWith = getObject()->getVeterancyLevel();

	return testAgainst == testWith;
}

Bool CreateCrateDie::testKillerType( CrateTemplate const *currentCrateData, Object *killer )
{
	if( killer == NULL )
		return FALSE;

	// Must match the whole group of bits set in the KilledBy description (most likely One).
	if( ! killer->getTemplate()->isKindOfMulti( currentCrateData->m_killedByTypeKindof, KINDOFMASK_NONE ) )
		return FALSE;

	return TRUE;
}

Bool CreateCrateDie::testKillerScience( CrateTemplate const *currentCrateData, Object *killer )
{
	if( killer == NULL )
		return FALSE;

	// killer's player must have the listed science
	Player *killerPlayer = killer->getControllingPlayer();

	if( killerPlayer == NULL )
		return FALSE;

	if( ! killerPlayer->hasScience( currentCrateData->m_killerScience ) )
		return FALSE;

	return TRUE;
}

Object *CreateCrateDie::createCrate( CrateTemplate const *currentCrateData )
{
	Coord3D centerPoint = *getObject()->getPosition();
	PathfindLayerEnum layer = getObject()->getLayer();

	// CreationChance is used for the success of this block, but this block can have any number of potential actual crates
	Real multipleCratePick = GameLogicRandomValueReal( 0, 1 );
	Real multipleCrateRunningTotal = 0;
	AsciiString crateName;

	for( crateCreationEntryConstIterator iter = currentCrateData->m_possibleCrates.begin();
				iter != currentCrateData->m_possibleCrates.end();
				iter++
			)
	{
		multipleCrateRunningTotal += (*iter).crateChance;
		if( multipleCrateRunningTotal > multipleCratePick )
		{
			// Run through the list of possibles, and if the sum of the chances is greater than my random pick,
			// then this is the correct one.  (This simulates contiguous %, or weighted distribution)
			crateName = (*iter).crateName;
			break;
		}
	}
	// At this point, I could very well have a "" for the type, if the Designer didn't make the sum of chances 1

	ThingTemplate const *crateType = TheThingFactory->findTemplate( crateName );
	if( crateType == NULL )
		return NULL;

	Bool spotFound = FALSE;
	Coord3D creationPoint;
	FindPositionOptions fpOptions;

	fpOptions.minRadius = 0.0f;
	fpOptions.maxRadius = 5.0f;
	fpOptions.relationshipObject = getObject();
	fpOptions.flags = FPF_IGNORE_ALLY_OR_NEUTRAL_UNITS; // So the dead guy won't block, nor will his dead hulk.
	if (layer != LAYER_GROUND) {
		creationPoint = centerPoint;
		spotFound = true;
	} else if( ThePartitionManager->findPositionAround( &centerPoint, &fpOptions, &creationPoint ) )
	{
		spotFound = TRUE;
	}
	else
	{
		// If the tight ignore units scan fails, then try a great big scan so we appear on the edge
		// of the large dead thing (building rubble)
		fpOptions.minRadius = 0.0f;
		fpOptions.maxRadius = 125.0f;
		fpOptions.relationshipObject = NULL;
		fpOptions.flags = FPF_NONE;
		if( ThePartitionManager->findPositionAround( &centerPoint, &fpOptions, &creationPoint ) )
		{
			spotFound = TRUE;
		}
	}

	if( spotFound )
	{
		Object *newCrate = TheThingFactory->newObject( crateType, NULL );
		newCrate->setPosition( &creationPoint );
		newCrate->setOrientation( GameLogicRandomValueReal( 0, 2*PI ) );
		newCrate->setLayer(layer);

		Drawable *crateDrawable = newCrate->getDrawable();

		if( crateDrawable )
		{
			crateDrawable->setTerrainDecal(TERRAIN_DECAL_CRATE);
			crateDrawable->setTerrainDecalSize(2.5f * newCrate->getGeometryInfo().getMajorRadius(),
																2.5f * newCrate->getGeometryInfo().getMajorRadius() )	;
			crateDrawable->setTerrainDecalFadeTarget(1.0f, 0.03f);
		}

		return newCrate;
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CreateCrateDie::crc( Xfer *xfer )
{

	// extend base class
	DieModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CreateCrateDie::xfer( Xfer *xfer )
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
void CreateCrateDie::loadPostProcess( void )
{

	// extend base class
	DieModule::loadPostProcess();

}
