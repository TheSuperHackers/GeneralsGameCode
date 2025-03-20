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

// FILE: W3DRopeDraw.cpp ////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Aug 2002
// Desc:   Rope drawing
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <windows.h>
#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/ClientRandomValue.h"
#include "GameClient/Color.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/Module/W3DRopeDraw.h"
#include "WW3D2/line3d.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "Common/GameState.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DRopeDraw::W3DRopeDraw( Thing *thing, const ModuleData* moduleData ) : DrawModule( thing, moduleData )
{
	m_curLen = 0.0f;
	m_maxLen = 1.0f;
	m_width = 0.5f;
	m_color.red = 0.0f;
	m_color.green = 0.0f;
	m_color.blue = 0.0f;
	m_curSpeed = 0.0f;
	m_maxSpeed = 0.0f;
	m_accel = 0.0f;
	m_wobbleLen = m_maxLen;	// huge
	m_wobbleAmp = 0.0f;
	m_segments.clear();
	m_curWobblePhase = 0.0f;
	m_curZOffset = 0.0f;
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DRopeDraw::buildSegments()
{
	DEBUG_ASSERTCRASH(m_segments.empty(), ("Hmmn, not empty"));
	m_segments.clear();

	Int numSegs = ceil(m_maxLen / m_wobbleLen);
	Real eachLen = m_maxLen / (Real)numSegs;
	Coord3D pos = *getDrawable()->getPosition();
	for (int i = 0; i < numSegs; ++i, pos.z += eachLen)
	{
		SegInfo info;

		Real axis = GameClientRandomValueReal(0, 2*PI);
		info.wobbleAxisX = Cos(axis);
		info.wobbleAxisY = Sin(axis);
		info.line = NEW Line3DClass( Vector3(pos.x,pos.y,pos.z),
																	 Vector3(pos.x,pos.y,pos.z+eachLen),
																	 m_width * 0.5f,  // width
																	 m_color.red,  // red
																	 m_color.green,  // green
																	 m_color.blue,  // blue
																	 1.0f );  // transparency

		info.softLine = NEW Line3DClass( Vector3(pos.x,pos.y,pos.z),
																	 Vector3(pos.x,pos.y,pos.z+eachLen),
																	 m_width,  // width
																	 m_color.red,  // red
																	 m_color.green,  // green
																	 m_color.blue,  // blue
																	 0.5f );  // transparency

		W3DDisplay::m_3DScene->Add_Render_Object( info.line );
		W3DDisplay::m_3DScene->Add_Render_Object( info.softLine );
		m_segments.push_back(info);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DRopeDraw::tossSegments()
{
	// remove tracer from the scene and delete
	for (std::vector<SegInfo>::iterator it = m_segments.begin(); it != m_segments.end(); ++it)
	{
		if (it->line)
		{
			W3DDisplay::m_3DScene->Remove_Render_Object(it->line);
			REF_PTR_RELEASE((it->line));
		}
		if (it->softLine)
		{
			W3DDisplay::m_3DScene->Remove_Render_Object(it->softLine);
			REF_PTR_RELEASE((it->softLine));
		}
	}
	m_segments.clear();
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DRopeDraw::initRopeParms(Real length, Real width, const RGBColor& color, Real wobbleLen, Real wobbleAmp, Real wobbleRate)
{ 
	m_maxLen = max(1.0f, length);
	m_curLen = 0.0f;
	m_width = width;
	m_color = color;
	m_wobbleLen = min(m_maxLen, wobbleLen);
	m_wobbleAmp = wobbleAmp;
	m_wobbleRate = wobbleRate;
	m_curZOffset = 0.0f;

	tossSegments();
	buildSegments();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DRopeDraw::setRopeCurLen(Real length)
{ 
	m_curLen = length; 
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DRopeDraw::setRopeSpeed(Real curSpeed, Real maxSpeed, Real accel)
{ 
	m_curSpeed = curSpeed; 
	m_maxSpeed = maxSpeed; 
	m_accel = accel;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DRopeDraw::~W3DRopeDraw()
{
	tossSegments();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DRopeDraw::doDrawModule(const Matrix3D* transformMtx)
{
	if (m_segments.empty())
	{
		buildSegments();
	}

	if (!m_segments.empty())
	{
		Real deflection = Sin(m_curWobblePhase) * m_wobbleAmp;
		const Coord3D* pos = getDrawable()->getPosition();
		Vector3 start(pos->x, pos->y, pos->z + m_curZOffset);
		Real eachLen = m_curLen / m_segments.size();
		for (std::vector<SegInfo>::iterator it = m_segments.begin(); it != m_segments.end(); ++it)
		{
			Vector3 end(pos->x + deflection*it->wobbleAxisX, pos->y + deflection*it->wobbleAxisY, start.Z - eachLen);
			if (it->line)
				(it->line)->Reset(start, end);
			if (it->softLine)
				(it->softLine)->Reset(start, end);
			start = end;
		}
	}

	m_curWobblePhase += m_wobbleRate;
	if (m_curWobblePhase > 2*PI)
		m_curWobblePhase -= 2*PI;

	m_curZOffset += m_curSpeed;
	m_curSpeed += m_accel;
	if (m_curSpeed > m_maxSpeed)
		m_curSpeed = m_maxSpeed;
	else if (m_curSpeed < -m_maxSpeed)
		m_curSpeed = -m_maxSpeed;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DRopeDraw::crc( Xfer *xfer )
{

	// extend base class
	DrawModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DRopeDraw::xfer( Xfer *xfer )
{

	// version
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DrawModule::xfer( xfer );

	// m_segments is not saved

	// cur len
	xfer->xferReal( &m_curLen );

	// max len
	xfer->xferReal( &m_maxLen );

	// width
	xfer->xferReal( &m_width );

	// color
	xfer->xferRGBColor( &m_color );

	// cur speed
	xfer->xferReal( &m_curSpeed );

	// max speed
	xfer->xferReal( &m_maxSpeed );

	// acceleration
	xfer->xferReal( &m_accel );

	// wobble len
	xfer->xferReal( &m_wobbleLen );

	// wobble amp
	xfer->xferReal( &m_wobbleAmp );

	// wobble rate
	xfer->xferReal( &m_wobbleRate );

	// current wobble phase
	xfer->xferReal( &m_curWobblePhase );

	// cur Z offset
	xfer->xferReal( &m_curZOffset );

	if (xfer->getXferMode() == XFER_LOAD)
		tossSegments();


}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DRopeDraw::loadPostProcess( void )
{

	// extend base class
	DrawModule::loadPostProcess();

}  // end loadPostProcess
