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

// FILE: SpyVisionUpdate.cpp /////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September 2002
// Desc:   Special Power will spy on the vision of all enemy players.
//				Putting a SpecialPower in a behavior takes a big huge amount of code duplication and
//				has no precedent.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/SpyVisionUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpyVisionUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpyVisionUpdate::SpyVisionUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_deactivateFrame = 0;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpyVisionUpdate::~SpyVisionUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpyVisionUpdate::activateSpyVision( UnsignedInt duration )
{
	UnsignedInt now = TheGameLogic->getFrame();
	m_deactivateFrame = now + duration;

	doActivationWork( TRUE );

	setWakeFrame( getObject(), UPDATE_SLEEP(duration) );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime SpyVisionUpdate::update( void )
{
	if( m_deactivateFrame && (m_deactivateFrame <= TheGameLogic->getFrame()) )
	{
		// Turn off SpyVision.
		doActivationWork( FALSE );

		m_deactivateFrame = 0;
	}

	return UPDATE_SLEEP_FOREVER;
}

void SpyVisionUpdate::doActivationWork( Bool setting )
{
	Player *ourPlayer = getObject()->getControllingPlayer();
	if( ourPlayer == NULL  ||  ThePlayerList == NULL )
		return;

	for (Int i=0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if( ourPlayer->getRelationship(player->getDefaultTeam()) == ENEMIES )
		{
			player->setUnitsVisionSpied( setting, ourPlayer->getPlayerIndex() );
		}
	}
}

void SpyVisionUpdate::onDelete( void )
{
	// If I was left on at the time of death, then turn me off.
	if( m_deactivateFrame )
		doActivationWork( FALSE );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SpyVisionUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SpyVisionUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// deactivate frame
	xfer->xferUnsignedInt( &m_deactivateFrame );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SpyVisionUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
