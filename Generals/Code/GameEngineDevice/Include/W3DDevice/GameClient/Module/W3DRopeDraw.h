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

// FILE: W3DRopeDraw.h //////////////////////////////////////////////////////////////////////////
// Author:
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/DrawModule.h"
#include "WW3D2/line3d.h"

//-------------------------------------------------------------------------------------------------
/** W3D rope draw */
//-------------------------------------------------------------------------------------------------
class W3DRopeDraw : public DrawModule, public RopeDrawInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DRopeDraw, "W3DRopeDraw" )
	MAKE_STANDARD_MODULE_MACRO( W3DRopeDraw )

public:

	W3DRopeDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void doDrawModule(const Matrix3D* transformMtx);
	virtual void setShadowsEnabled(Bool enable) { }
	virtual void releaseShadows(void) {};	///< we don't care about preserving temporary shadows.
	virtual void allocateShadows(void) {};	///< we don't care about preserving temporary shadows.
	virtual void setFullyObscuredByShroud(Bool fullyObscured) { }
	virtual void reactToTransformChange(const Matrix3D* oldMtx, const Coord3D* oldPos, Real oldAngle) { }
	virtual void reactToGeometryChange() { }

	virtual void initRopeParms(Real length, Real width, const RGBColor& color, Real wobbleLen, Real wobbleAmp, Real wobbleRate);
	virtual void setRopeCurLen(Real length);
	virtual void setRopeSpeed(Real curSpeed, Real maxSpeed, Real accel);

	virtual RopeDrawInterface* getRopeDrawInterface() { return this; }
	virtual const RopeDrawInterface* getRopeDrawInterface() const { return this; }

private:

	struct SegInfo
	{
		Line3DClass* line;
		Line3DClass* softLine;
		Real wobbleAxisX;
		Real wobbleAxisY;
	};

	std::vector<SegInfo> m_segments;			///< the rope render object in the W3D scene
	Real m_curLen;								///< length of rope
	Real m_maxLen;								///< length of rope
	Real m_width;									///< width of rope
	RGBColor m_color;							///< color of rope
	Real m_curSpeed;
	Real m_maxSpeed;
	Real m_accel;
	Real m_wobbleLen;
	Real m_wobbleAmp;
	Real m_wobbleRate;
	Real m_curWobblePhase;
	Real m_curZOffset;

	void tossSegments();
	void buildSegments();

};
