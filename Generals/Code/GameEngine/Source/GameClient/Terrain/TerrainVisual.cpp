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

// FILE: TerrainVisual.cpp ////////////////////////////////////////////////////////////////////////
// Interface for visual representation of terrain on the client
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameClient/TerrainVisual.h"





// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
TerrainVisual *TheTerrainVisual = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINITIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainVisual::TerrainVisual()
{

}  // end TerrainVisual

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainVisual::~TerrainVisual()
{

}  // end ~TerrainVisual

//-------------------------------------------------------------------------------------------------
/** initialize the device independent functionality of the visual terrain */
//-------------------------------------------------------------------------------------------------
void TerrainVisual::init( void )
{

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void TerrainVisual::reset( void )
{

	m_filenameString.clear();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void TerrainVisual::update( void )
{
	// All the interesting stuff happens in load.  jba.
}  // end update

//-------------------------------------------------------------------------------------------------
/** device independent implementation for common terrain visual systems */
//-------------------------------------------------------------------------------------------------
Bool TerrainVisual::load( AsciiString filename )
{

	// save the filename
	if (filename.isEmpty())
		return FALSE;

	m_filenameString = filename;

	return TRUE;;  // success

}  // end load

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TerrainVisual::crc( Xfer *xfer )
{

}  // end CRC

// ------------------------------------------------------------------------------------------------
/** Xfer
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void TerrainVisual::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TerrainVisual::loadPostProcess( void )
{

}  // end loadPostProcess
