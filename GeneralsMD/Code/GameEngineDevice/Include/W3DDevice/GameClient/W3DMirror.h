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


#pragma once

#ifndef __W3DMirror_H_
#define __W3DMirror_H_

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"
#include "Common/GameType.h"

/// Custom render object that draws mirrors, water, and skies.
/**
This render object handles drawing reflected W3D scenes.  It will only work
with rectangular planar surfaces and was tuned with an emphasis on water.
Since skies are only visible in reflections, this code will also
render clouds and sky bodies.
*/
class MirrorRenderObjClass : public RenderObjClass
{

public:

	MirrorRenderObjClass(void);
	~MirrorRenderObjClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
/// @todo: Add methods for collision detection with mirror surface
//	virtual Bool					Cast_Ray(RayCollisionTestClass & raytest);
//	virtual Bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
//	virtual Bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
//	virtual Bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
//	virtual Bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
    virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;

	///allocate W3D resources needed to render mirror
	Int init(Real waterLevel, Real dx, Real dy, SceneClass *parentScene);
	static void setTimeOfDay(TimeOfDay tod)	{m_tod=tod;}	///<change sky/water for time of day
	void toggleCloudLayer(Bool state)	{	m_useCloudLayer=state;}	///<enables/disables the cloud layer

protected:
	DX8IndexBufferClass			*m_indexBuffer;	///<indices defining quad
	SceneClass							*m_parentScene;	///<scene to be reflected
	ShaderClass m_shaderClass; ///<shader or rendering state for heightmap
	VertexMaterialClass	  		*m_vertexMaterialClass;	///<vertex lighting material
	TextureClass *m_alphaClippingTexture;	///<used for faked clipping using alpha
	Real	m_dx;	///<x extent of mirror surface (offset from local center)
	Real	m_dy;	///<y extent of mirror surface (offset from local center)
	Vector3 m_planeNormal;		///<mirror plane normal
	Real		m_planeDistance;	///<mirror plane distance
	Real		m_level;			///<level of mirror (hack for water)
	Real		m_uOffset;			///<current texture offset on u axis
	Real		m_vOffset;			///<current texture offset on v axis
	Real		m_uScrollPerMs;		///<texels per/ms scroll rate in u direction
	Real		m_vScrollPerMs;		///<texels per/ms scroll rate in v direction
	Int			m_LastUpdateTime;	///<time of last cloud update
	Bool		m_useCloudLayer;	///<flag if clouds are on/off

	static TimeOfDay m_tod;	///<time of day setting for reflected cloud layer

	struct skySetting
	{
		TextureClass	*skyTexture;
		TextureClass	*waterTexture;
		Int				waterRepeatCount;
		Int				skyRepeatCount;
		DWORD			vertex00Diffuse;
		DWORD			vertex10Diffuse;
		DWORD			vertex11Diffuse;
		DWORD			vertex01Diffuse;
		DWORD			waterDiffuse;
		Real			uScrollPerMs;
		Real			vScrollPerMs;
	};

	skySetting m_skySettings[TIME_OF_DAY_COUNT];	///< settings for each time of day
	void renderSky(void);	///<draw the sky layer (clouds, stars, etc.)
	void renderSkyBody(Matrix3D *mat);	///<draw the sky body (sun, moon, etc.)
	void renderWater(void);
	void renderWaterMesh(void);
};

#endif  // end __W3DMirror_H_
