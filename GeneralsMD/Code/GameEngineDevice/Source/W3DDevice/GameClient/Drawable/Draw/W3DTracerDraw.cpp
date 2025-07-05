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

// FILE: W3DTracerDraw.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Tracer drawing
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/Module/W3DTracerDraw.h"
#include "WW3D2/line3d.h"
#include "W3DDevice/GameClient/W3DScene.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTracerDraw::W3DTracerDraw( Thing *thing, const ModuleData* moduleData ) : DrawModule( thing, moduleData )
{

	// set opacity
	m_opacity = 1.0f;
	m_length = 20.0f;
	m_width = 0.5f;
	m_color.red = 0.9f;
	m_color.green = 0.8f;
	m_color.blue = 0.7f;
	m_speedInDistPerFrame = 1.0f;
	m_theTracer = NULL;

}  // end W3DTracerDraw

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTracerDraw::setTracerParms(Real speed, Real length, Real width, const RGBColor& color, Real initialOpacity)
{ 
	m_speedInDistPerFrame = speed; 
	m_length = length;
	m_width = width;
	m_color = color;
	m_opacity = initialOpacity;
	if (m_theTracer)
	{
		Vector3 start( 0.0f, 0.0f, 0.0f );
		Vector3 stop( m_length, 0.0f, 0.0f );
		m_theTracer->Reset(start, stop, m_width);
		m_theTracer->Re_Color(m_color.red, m_color.green, m_color.blue);
		m_theTracer->Set_Opacity( m_opacity );
		// these calls nuke the internal transform, so re-set it here
		m_theTracer->Set_Transform( *getDrawable()->getTransformMatrix() );
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTracerDraw::~W3DTracerDraw( void )
{
	// remove tracer from the scene and delete
	if( m_theTracer )
	{
		W3DDisplay::m_3DScene->Remove_Render_Object( m_theTracer );
		REF_PTR_RELEASE( m_theTracer );
	}
}

//-------------------------------------------------------------------------------------------------
void W3DTracerDraw::reactToTransformChange( const Matrix3D *oldMtx, 
																							 const Coord3D *oldPos, 
																							 Real oldAngle )
{
	if( m_theTracer )
		m_theTracer->Set_Transform( *getDrawable()->getTransformMatrix() );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTracerDraw::doDrawModule(const Matrix3D* transformMtx)
{

	// create tracer
	if( m_theTracer == NULL )
	{

		Vector3 start( 0.0f, 0.0f, 0.0f );
		Vector3 stop( m_length, 0.0f, 0.0f );

		// create tracer render object for us to manipulate
		// poolify
		m_theTracer = NEW Line3DClass( start,
																	 stop,
																	 m_width,  // width
																	 m_color.red,  // red
																	 m_color.green,  // green
																	 m_color.blue,  // blue
																	 m_opacity );  // transparency
		W3DDisplay::m_3DScene->Add_Render_Object( m_theTracer );

		// set the transform for the tracer to that of the drawable
		m_theTracer->Set_Transform( *transformMtx );

	}

	UnsignedInt expDate = getDrawable()->getExpirationDate();
	if (expDate != 0)
	{
		Real decay = m_opacity / (expDate - TheGameLogic->getFrame());
		m_opacity -= decay;
		m_theTracer->Set_Opacity( m_opacity );
	}

	// set the position for the tracer
	if (m_speedInDistPerFrame != 0.0f)
	{
		Matrix3D pos = m_theTracer->Get_Transform();
		pos.Translate( Vector3( m_speedInDistPerFrame, 0.0f, 0.0 ) );
		m_theTracer->Set_Transform( pos );
	}

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DTracerDraw::crc( Xfer *xfer )
{

	// extend base class
	DrawModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTracerDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DrawModule::xfer( xfer );

	// no data to save here, nobody will ever notice

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DTracerDraw::loadPostProcess( void )
{

	// extend base class
	DrawModule::loadPostProcess();

}  // end loadPostProcess
