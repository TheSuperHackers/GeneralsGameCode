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

// FILE: W3DProjectileStreamDraw.h ////////////////////////////////////////////////////////////
// Tile a texture strung between Projectiles
// Graham Smallwood, May 2002
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _W3D_PROJECTILE_STREAM_DRAW_H_
#define _W3D_PROJECTILE_STREAM_DRAW_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/DrawModule.h"
#include "GameLogic/Module/ProjectileStreamUpdate.h" // I am the draw module for this update.  Very tight.

class SegmentedLineClass;
class TextureClass;
class Vector3;

//-------------------------------------------------------------------------------------------------
class W3DProjectileStreamDrawModuleData : public ModuleData
{
public:

	AsciiString m_textureName;
	Real m_width;
	Real m_tileFactor;
	Real m_scrollRate;
	Int m_maxSegments;

	W3DProjectileStreamDrawModuleData();
	~W3DProjectileStreamDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class W3DProjectileStreamDraw : public DrawModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DProjectileStreamDraw, "W3DProjectileStreamDraw" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( W3DProjectileStreamDraw, W3DProjectileStreamDrawModuleData )

public:

	W3DProjectileStreamDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void doDrawModule(const Matrix3D* transformMtx);
	virtual void releaseShadows(void) {};	///< we don't care about preserving temporary shadows.
	virtual void allocateShadows(void) {};	///< we don't care about preserving temporary shadows.
	virtual void setShadowsEnabled(Bool ) { }
	virtual void setFullyObscuredByShroud(Bool);
	virtual void reactToTransformChange(const Matrix3D* oldMtx, const Coord3D* oldPos, Real oldAngle) { }
	virtual void reactToGeometryChange() { }

protected:
	void makeOrUpdateLine( Vector3 *points, UnsignedInt pointCount, Int lineIndex );

	TextureClass *m_texture;
	SegmentedLineClass *m_allLines[MAX_PROJECTILE_STREAM];	///< Persist, so I can ensure they live a full cycle, and minimize re-creates by holding on
	Int m_linesValid;
};

#endif

