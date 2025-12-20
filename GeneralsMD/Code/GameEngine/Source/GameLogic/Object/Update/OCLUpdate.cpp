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

// FILE: OCLUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, August2002
// Desc:   Update Spits out an OCL on a timer
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "Common/Player.h"
#include "Common/PlayerTemplate.h"
#include "Common/UnicodeString.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/OCLUpdate.h"
#include "GameLogic/TerrainLogic.h"

//-------------------------------------------------------------------------------------------------
void parseFactionObjectCreationList( INI *ini, void *instance, void *store, const void *userData )
{
	OCLUpdateModuleData::FactionOCLInfo info;
	info.m_ocl = NULL;

	const char *token = ini->getNextToken( ini->getSepsColon() );

	if ( stricmp(token, "Faction") == 0 )
	{
		token = ini->getNextTokenOrNull( ini->getSepsColon() );
		if (!token)	throw INI_INVALID_DATA;

		info.m_factionName = token;
	}
	else
		throw INI_INVALID_DATA;


	token = ini->getNextTokenOrNull( ini->getSepsColon() );
	if ( stricmp(token, "OCL") == 0 )
		ini->parseObjectCreationList( ini, instance, &info.m_ocl, NULL );
	else
		throw INI_INVALID_DATA;

	// Insert the info into the ocl hashmap
	OCLUpdateModuleData::FactionOCLList * theList = (OCLUpdateModuleData::FactionOCLList*)store;
	theList->push_back(info);

}

//-------------------------------------------------------------------------------------------------
OCLUpdateModuleData::OCLUpdateModuleData()
{
	m_minDelay = 0;
	m_maxDelay = 0;
	m_ocl = NULL;
	m_factionOCL.clear();
	m_isCreateAtEdge = FALSE;
	m_isFactionTriggered = FALSE;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void OCLUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "OCL",					INI::parseObjectCreationList,		NULL, offsetof( OCLUpdateModuleData, m_ocl ) },
		{ "FactionOCL",		parseFactionObjectCreationList,	NULL, offsetof( OCLUpdateModuleData, m_factionOCL ) },
		{ "MinDelay",			INI::parseDurationUnsignedInt,	NULL, offsetof( OCLUpdateModuleData, m_minDelay ) },
		{ "MaxDelay",			INI::parseDurationUnsignedInt,	NULL, offsetof( OCLUpdateModuleData, m_maxDelay ) },
		{ "CreateAtEdge",	INI::parseBool,									NULL, offsetof( OCLUpdateModuleData, m_isCreateAtEdge ) },
		{ "FactionTriggered",	INI::parseBool,							NULL, offsetof( OCLUpdateModuleData, m_isFactionTriggered ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OCLUpdate::OCLUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_nextCreationFrame = 0;
	m_timerStartedFrame = 0;
	m_isFactionNeutral = TRUE;
	m_currentPlayerColor = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OCLUpdate::~OCLUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime OCLUpdate::update( void )
{
#if RETAIL_COMPATIBLE_CRC
	if( getObject()->isDisabled() )
	{
		m_nextCreationFrame++;
		return UPDATE_SLEEP_NONE;
	}
#else
	// TheSuperHackers @bugfix dizzyj/Caball009/Mauller 14/07/2025 prevent triggering supply drop when subdued while under construction
	// When the construction is finished, we allow the timer to be initialized and then start shifting the timer while subdued
	if ( m_timerStartedFrame > 0 && getObject()->isDisabled() )
	{
		m_nextCreationFrame++;
		m_timerStartedFrame++;
		return UPDATE_SLEEP_NONE;
	}
#endif

	const OCLUpdateModuleData *data = getOCLUpdateModuleData();

	// Test if the OCL update is faction dependant. If so, check for faction changes
	if (data->m_isFactionTriggered)
	{
		Player *player = getObject()->getControllingPlayer();

		// Test for when a player captures the building
		if (m_isFactionNeutral)
		{
			if( player && player->isPlayableSide() )
			{
				m_currentPlayerColor = player->getPlayerColor();
				m_isFactionNeutral = FALSE;
				setNextCreationFrame();
			}
		}
		// Test for when the building has been made neutral or when it changes faction
		else
		{
			// If this is no longer under player control, then we set the faction to neutral
			if( !player || !player->isPlayableSide() )
			{
				m_isFactionNeutral = TRUE;
			}
			// If another player has taken control, reset the timer
			else if( player && player->getPlayerColor() != m_currentPlayerColor)
			{
				m_currentPlayerColor = player->getPlayerColor();
				setNextCreationFrame();
			}
		}

		// If the building is neutal, skip futher update
		if (m_isFactionNeutral)
			return UPDATE_SLEEP_NONE;
	}

/// @todo srj use SLEEPY_UPDATE here
	if( shouldCreate() )
	{
		if( m_nextCreationFrame == 0 )
		{
			// You don't get to actually spread the first try, you start on a timer, then go
			setNextCreationFrame();
			return UPDATE_SLEEP_NONE;
		}

		setNextCreationFrame();

		Coord3D creationCoord;
		if( getOCLUpdateModuleData()->m_isCreateAtEdge )
			creationCoord = TheTerrainLogic->findClosestEdgePoint( getObject()->getPosition() );
		else
			creationCoord = *getObject()->getPosition();

		// If this is faction triggered, search through the faction specific OCLs to find the match
		if (data->m_isFactionTriggered)
		{
			std::string playerFactionName;

			Player *player = getObject()->getControllingPlayer();
			if (!player) return UPDATE_SLEEP_NONE;

			const PlayerTemplate *playerT = player->getPlayerTemplate();
			if (!playerT) return UPDATE_SLEEP_NONE;

			// Get and store the faction side to compare with the faction ocl list
			if (playerT->getSide().str()) playerFactionName = playerT->getSide().str();

			// Loop through the list of faction ocls to find the matching faction that triggeres the specific ocls
			for (OCLUpdateModuleData::FactionOCLList::const_iterator it = data->m_factionOCL.begin(); it != data->m_factionOCL.end(); ++it)
			{
				OCLUpdateModuleData::FactionOCLInfo info = *it;
				if (playerFactionName == info.m_factionName)
				{
					ObjectCreationList::create( info.m_ocl, getObject(), &creationCoord, getObject()->getPosition(), getObject()->getOrientation() );
					break;
				}
			}
		}
		// Use the non faction OCL information
		else
		{
			ObjectCreationList::create( data->m_ocl, getObject(), &creationCoord, getObject()->getPosition(), getObject()->getOrientation() );
		}
	}
	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
void OCLUpdate::resetTimer()
{
	setNextCreationFrame();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool OCLUpdate::shouldCreate()
{
	if( TheGameLogic->getFrame() < m_nextCreationFrame )
		return FALSE;//too soon

	if( getObject()->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) )
		return FALSE;// not built yet

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void OCLUpdate::setNextCreationFrame()
{
	UnsignedInt delay = GameLogicRandomValue( getOCLUpdateModuleData()->m_minDelay,
																						getOCLUpdateModuleData()->m_maxDelay );
	m_timerStartedFrame = TheGameLogic->getFrame();
	m_nextCreationFrame = m_timerStartedFrame + delay;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Real OCLUpdate::getCountdownPercent() const
{
	UnsignedInt now = TheGameLogic->getFrame();

	return 1.0f - (( m_nextCreationFrame - now ) / (float)( m_nextCreationFrame - m_timerStartedFrame ));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UnsignedInt OCLUpdate::getRemainingFrames() const
{
	UnsignedInt now = TheGameLogic->getFrame();

	return ( m_nextCreationFrame - now );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void OCLUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void OCLUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// next creation frame
	xfer->xferUnsignedInt( &m_nextCreationFrame );

	// timer stated frame
	xfer->xferUnsignedInt( &m_timerStartedFrame );

	// faction status
	xfer->xferBool( &m_isFactionNeutral );

	// current owning player color
	xfer->xferInt( &m_currentPlayerColor );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void OCLUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
