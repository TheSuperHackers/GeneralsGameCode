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

// FILE: MissionStats.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: MissionStats.cpp
//
// Created:   Steven Johnson, October 2001
//
// Desc:      @todo
//
//-----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/MissionStats.h"
#include "Common/Player.h"
#include "Common/Xfer.h"

//-----------------------------------------------------------------------------
MissionStats::MissionStats()
{
	init();
}

//-----------------------------------------------------------------------------
void MissionStats::init()
{
	Int i;

	for (i = 0; i < MAX_PLAYER_COUNT; ++i)
	{
		m_unitsKilled[i] = 0;
		m_buildingsKilled[i] = 0;
	}
	m_unitsLost = 0;
	m_buildingsLost = 0;
	//m_whoLastHurtMe = PLAYER_NONE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void MissionStats::crc( Xfer *xfer )
{

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info;
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void MissionStats::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// units killed
	xfer->xferUser( m_unitsKilled, sizeof( Int ) * MAX_PLAYER_COUNT );

	// units lost
	xfer->xferInt( &m_unitsLost );

	// buidings killed
	xfer->xferUser( m_buildingsKilled, sizeof( Int ) * MAX_PLAYER_COUNT );

	// buildings lost
	xfer->xferInt( &m_buildingsLost );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void MissionStats::loadPostProcess( void )
{

}
