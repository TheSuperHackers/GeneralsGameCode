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

// FILE: W3DScienceModelDraw.cpp ////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, NOVEMBER 2002
// Desc: Draw module just like Model, except it only draws if the local player has the specified science
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "W3DDevice/GameClient/Module/W3DScienceModelDraw.h"

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Science.h"
#include "Common/Xfer.h"

//-------------------------------------------------------------------------------------------------
W3DScienceModelDrawModuleData::W3DScienceModelDrawModuleData()
{
	m_requiredScience = SCIENCE_INVALID;
}

//-------------------------------------------------------------------------------------------------
W3DScienceModelDrawModuleData::~W3DScienceModelDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DScienceModelDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "RequiredScience", INI::parseScience, NULL, offsetof(W3DScienceModelDrawModuleData, m_requiredScience) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DScienceModelDraw::W3DScienceModelDraw( Thing *thing, const ModuleData* moduleData ) : W3DModelDraw( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
W3DScienceModelDraw::~W3DScienceModelDraw()
{
}

//-------------------------------------------------------------------------------------------------
// All this does is stop the call path if we haven't been cleared to draw yet
void W3DScienceModelDraw::doDrawModule(const Matrix3D* transformMtx)
{
	ScienceType science = getW3DScienceModelDrawModuleData()->m_requiredScience;
	if( science == SCIENCE_INVALID )
	{
		DEBUG_ASSERTCRASH(science != SCIENCE_INVALID, ("ScienceModelDraw has invalid science as condition.") );
		setHidden( TRUE );
		return;
	}

	if( !ThePlayerList->getLocalPlayer()->hasScience(science)
			&& ThePlayerList->getLocalPlayer()->isPlayerActive()
		)
	{
		// We just don't draw for people without our science except for Observers
		setHidden( TRUE );
		return;
	}

	W3DModelDraw::doDrawModule(transformMtx);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DScienceModelDraw::crc( Xfer *xfer )
{

	// extend base class
	W3DModelDraw::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DScienceModelDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	W3DModelDraw::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DScienceModelDraw::loadPostProcess( void )
{

	// extend base class
	W3DModelDraw::loadPostProcess();

}


