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

// FILE: W3DTreeDraw.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Tree draw
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameClient/Drawable.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"


//-------------------------------------------------------------------------------------------------
W3DTreeDrawModuleData::W3DTreeDrawModuleData() :
m_framesToMoveInward(1),
m_framesToMoveOutward(1),
m_darkening(0.0f),
m_maxOutwardMovement(1.0f)
{

	// Topple parameters. [7/7/2003]
	const Real START_VELOCITY_PERCENT = 0.2f;
	const Real START_ACCEL_PERCENT = 0.01f;
	const Real VELOCITY_BOUNCE_PERCENT = 0.3f;			// multiply the velocity by this when you bounce
  const Real MINIMUM_TOPPLE_SPEED = 0.5f;         // Won't let trees fall slower than this
	m_toppleFX = NULL;
	m_bounceFX = NULL;
	m_stumpName.clear();
	m_killWhenToppled = true;
	m_doTopple = false;
	m_doShadow = false;
	m_initialVelocityPercent = START_VELOCITY_PERCENT;
	m_initialAccelPercent = START_ACCEL_PERCENT;
	m_bounceVelocityPercent = VELOCITY_BOUNCE_PERCENT;
  m_minimumToppleSpeed = MINIMUM_TOPPLE_SPEED;
	m_sinkFrames = 10*LOGICFRAMES_PER_SECOND;
	m_sinkDistance = 20.0f;

}

//-------------------------------------------------------------------------------------------------
W3DTreeDrawModuleData::~W3DTreeDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DTreeDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "ModelName", INI::parseAsciiString, NULL, offsetof(W3DTreeDrawModuleData, m_modelName) },
		{ "TextureName", INI::parseAsciiString, NULL, offsetof(W3DTreeDrawModuleData, m_textureName) },
		{ "MoveOutwardTime", INI::parseDurationUnsignedInt, NULL, offsetof(W3DTreeDrawModuleData, m_framesToMoveOutward) },
		{ "MoveInwardTime", INI::parseDurationUnsignedInt, NULL, offsetof(W3DTreeDrawModuleData, m_framesToMoveInward) },
		{ "MoveOutwardDistanceFactor", INI::parseReal, NULL, offsetof(W3DTreeDrawModuleData, m_maxOutwardMovement) },
		{ "DarkeningFactor", INI::parseReal, NULL, offsetof(W3DTreeDrawModuleData, m_darkening) },

// Topple parameters [7/7/2003]
		{ "ToppleFX",	INI::parseFXList, NULL, offsetof( W3DTreeDrawModuleData, m_toppleFX ) },
		{ "BounceFX",	INI::parseFXList, NULL, offsetof( W3DTreeDrawModuleData, m_bounceFX ) },
		{ "StumpName",	INI::parseAsciiString, NULL, offsetof( W3DTreeDrawModuleData, m_stumpName ) },
		{ "KillWhenFinishedToppling",	INI::parseBool, NULL, offsetof( W3DTreeDrawModuleData, m_killWhenToppled ) },
		{ "DoTopple",	INI::parseBool, NULL, offsetof( W3DTreeDrawModuleData, m_doTopple ) },
		{ "InitialVelocityPercent",	INI::parsePercentToReal, NULL, offsetof( W3DTreeDrawModuleData, m_initialVelocityPercent ) },
		{ "InitialAccelPercent",	INI::parsePercentToReal, NULL, offsetof( W3DTreeDrawModuleData, m_initialAccelPercent ) },
		{ "BounceVelocityPercent",	INI::parsePercentToReal, NULL, offsetof( W3DTreeDrawModuleData, m_bounceVelocityPercent ) },
    { "MinimumToppleSpeed",	INI::parsePositiveNonZeroReal, NULL, offsetof( W3DTreeDrawModuleData, m_minimumToppleSpeed ) },
    { "SinkDistance",	INI::parsePositiveNonZeroReal, NULL, offsetof( W3DTreeDrawModuleData, m_sinkDistance ) },
		{ "SinkTime", INI::parseDurationUnsignedInt, NULL, offsetof(W3DTreeDrawModuleData, m_sinkFrames) },
		{ "DoShadow",	INI::parseBool, NULL, offsetof( W3DTreeDrawModuleData, m_doShadow ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTreeDraw::W3DTreeDraw( Thing *thing, const ModuleData* moduleData ) : DrawModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTreeDraw::~W3DTreeDraw( void )
{
	addToTreeBuffer();
}

//-------------------------------------------------------------------------------------------------
void W3DTreeDraw::addToTreeBuffer()
{
	const W3DTreeDrawModuleData *moduleData = getW3DTreeDrawModuleData();
	const Drawable *draw = getDrawable();

	DEBUG_ASSERTCRASH(draw->getPosition()->x == 0.0f && draw->getPosition()->y == 0.0f,
		("W3DTreeDraw::addToTreeBuffer - Why place tree at x:0 y:0 ?"));

	Real scale = draw->getScale();
	Real scaleRandomness = draw->getTemplate()->getInstanceScaleFuzziness();
	scaleRandomness = 0.0f; // We use the scale fuzziness inside WB to generate random scales, so they don't change at load time. jba. [4/22/2003]
	TheTerrainRenderObject->addTree(draw->getID(), *draw->getPosition(),
		scale, draw->getOrientation(), scaleRandomness, moduleData);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DTreeDraw::crc( Xfer *xfer )
{

	// extend base class
	DrawModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTreeDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DrawModule::xfer( xfer );

	// no data to save here, nobody will ever notice

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DTreeDraw::loadPostProcess( void )
{

	// extend base class
	DrawModule::loadPostProcess();

}

