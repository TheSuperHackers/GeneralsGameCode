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

// FILE: SupplyWarehouseCreate.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood Feb 2002
// Desc:   When a Supply Center is created, it needs to update all the Resource brains in all players
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ResourceGatheringManager.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/SupplyWarehouseCreate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SupplyWarehouseCreate::SupplyWarehouseCreate( Thing *thing, const ModuleData* moduleData ) : CreateModule( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SupplyWarehouseCreate::~SupplyWarehouseCreate( void )
{

}

//-------------------------------------------------------------------------------------------------
void SupplyWarehouseCreate::onCreate( void )
{
	// Warehouses are never Built.
	if( ThePlayerList == nullptr )
		return;

	for( Int playerIndex = ThePlayerList->getPlayerCount() - 1; playerIndex >= 0; playerIndex-- )
	{
		Player *currentPlayer = ThePlayerList->getNthPlayer( playerIndex );
		if( currentPlayer == nullptr )
			continue;
		ResourceGatheringManager *manager = currentPlayer->getResourceGatheringManager();
		if( manager == nullptr )
			continue;
		manager->addSupplyWarehouse( getObject() );
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCreate::crc( Xfer *xfer )
{

	// extend base class
	CreateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCreate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CreateModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCreate::loadPostProcess( void )
{

	// extend base class
	CreateModule::loadPostProcess();

}
