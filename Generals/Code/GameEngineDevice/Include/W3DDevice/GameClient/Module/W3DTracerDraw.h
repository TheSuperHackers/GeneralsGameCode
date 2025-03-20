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

// FILE: W3DTracerDraw.h //////////////////////////////////////////////////////////////////////////
// Author: 
// Desc:   
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DTRACERDRAW_H_
#define __W3DTRACERDRAW_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/DrawModule.h"
#include "WW3D2/line3d.h"

//-------------------------------------------------------------------------------------------------
/** W3D tracer draw */
//-------------------------------------------------------------------------------------------------
class W3DTracerDraw : public DrawModule, public TracerDrawInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DTracerDraw, "W3DTracerDraw" )
	MAKE_STANDARD_MODULE_MACRO( W3DTracerDraw )

public:

	W3DTracerDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void doDrawModule(const Matrix3D* transformMtx);
	virtual void setShadowsEnabled(Bool enable) { }
	virtual void releaseShadows(void) {};	///< we don't care about preserving temporary shadows.	
	virtual void allocateShadows(void) {};	///< we don't care about preserving temporary shadows.
	virtual void setFullyObscuredByShroud(Bool fullyObscured) { }
	virtual void reactToTransformChange(const Matrix3D* oldMtx, const Coord3D* oldPos, Real oldAngle);
	virtual void reactToGeometryChange() { }

	virtual void setTracerParms(Real speed, Real length, Real width, const RGBColor& color, Real initialOpacity);

	virtual TracerDrawInterface* getTracerDrawInterface() { return this; }
	virtual const TracerDrawInterface* getTracerDrawInterface() const { return this; }

protected:

	Line3DClass *m_theTracer;			///< the tracer render object in the W3D scene
	Real m_length;								///< length of tracer
	Real m_width;									///< width of tracer
	RGBColor m_color;							///< color of tracer
	Real m_speedInDistPerFrame;		///< speed of tracer (in dist/frame)
	Real m_opacity;								///< opacity of the tracer

};

#endif // __W3DTRACERDRAW_H_

