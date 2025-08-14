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

// FILE: W3DLaserDraw.h ///////////////////////////////////////////////////////////////////////////
// Author:
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DLASERDRAW_H_
#define __W3DLASERDRAW_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/DrawModule.h"
//#include "WW3D2/Line3D.h"
#include "GameClient/Color.h"

class SegmentedLineClass;
class TextureClass;

class W3DLaserDrawModuleData : public ModuleData
{
public:

  Color m_innerColor;
  Color m_outerColor;
	Real m_innerBeamWidth;
	Real m_outerBeamWidth;
	Real m_scrollRate;
	Bool m_tile;
  UnsignedInt m_numBeams;
  UnsignedInt m_maxIntensityFrames;
  UnsignedInt m_fadeFrames;
	AsciiString m_textureName;
	UnsignedInt m_segments;
	Real m_arcHeight;
	Real m_segmentOverlapRatio;
	Real m_tilingScalar;

	W3DLaserDrawModuleData();
	~W3DLaserDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
/** W3D laser draw */
//-------------------------------------------------------------------------------------------------
class W3DLaserDraw : public DrawModule, public LaserDrawInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DLaserDraw, "W3DLaserDraw" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( W3DLaserDraw, W3DLaserDrawModuleData )

public:

	W3DLaserDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void doDrawModule(const Matrix3D* transformMtx);
	virtual void releaseShadows(void) {};	///< we don't care about preserving temporary shadows.
	virtual void allocateShadows(void) {};	///< we don't care about preserving temporary shadows.
	virtual void setShadowsEnabled(Bool enable) { }
	virtual void setFullyObscuredByShroud(Bool fullyObscured) { };
	virtual void reactToTransformChange(const Matrix3D* oldMtx, const Coord3D* oldPos, Real oldAngle) { }
	virtual void reactToGeometryChange() { }
	virtual Bool isLaser() const { return true; }
	Real getLaserTemplateWidth() const;

	virtual LaserDrawInterface* getLaserDrawInterface() { return this; }
	virtual const LaserDrawInterface* getLaserDrawInterface() const { return this; }

protected:

	SegmentedLineClass **m_line3D;  ///< line 3D for effect
	TextureClass *m_texture;
	Real m_textureAspectRatio;			///< aspect ratio of texture
	Bool m_selfDirty;								// not saved

};

#endif // __W3DLASERDRAW_H_

