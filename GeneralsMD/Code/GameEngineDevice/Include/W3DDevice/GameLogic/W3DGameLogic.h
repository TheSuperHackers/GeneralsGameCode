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

// FILE: W3DGameLogic.h ///////////////////////////////////////////////////////
//
// W3D game logic class, there shouldn't be a lot of new functionality
// in this class, but there are certain things that need to have close
// knowledge of each other like ther logical and visual terrain
//
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DGAMELOGIC_H_
#define __W3DGAMELOGIC_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameLogic/W3DTerrainLogic.h"
#include "W3DDevice/GameLogic/W3DGhostObject.h"

class W3DGhostObjectManager;
///////////////////////////////////////////////////////////////////////////////
// PROTOTYPES /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// W3DGameLogic
//-----------------------------------------------------------------------------
/** W3D specific functionality for game logic */
//-----------------------------------------------------------------------------
class W3DGameLogic : public GameLogic
{

public:

protected:

	/// factory for TheTerrainLogic, called from init()
	virtual TerrainLogic *createTerrainLogic( void ) { return NEW W3DTerrainLogic; };
	virtual GhostObjectManager *createGhostObjectManager(void) { return NEW W3DGhostObjectManager; }

};  // end class W3DGameLogic

#endif  // end __W3DGAMELOGIC_H_
