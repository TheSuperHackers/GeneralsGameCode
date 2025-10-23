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

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"


//
// W3DStatusCircle: Object generated from 2D Height grid
//
//
class W3DStatusCircle : public RenderObjClass
{

public:

	W3DStatusCircle(void);
	W3DStatusCircle(const W3DStatusCircle & src);
	W3DStatusCircle & operator = (const W3DStatusCircle &);
	~W3DStatusCircle(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
//	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);
//	virtual void 					Set_Transform(const Matrix3D &m);
//	virtual void 					Set_Position(const Vector3 &v);
//TODO: MW: do these later - only needed for collision detection
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
//	virtual Bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
//	virtual Bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
//	virtual Bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
//	virtual Bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
    virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;


//	virtual int					 	Get_Num_Polys(void) const;
//	virtual const char *		 	Get_Name(void) const;
//	virtual void				 	Set_Name(const char * name);

//	unsigned int					Get_Flags(void)  { return Flags; }
//	void								Set_Flags(unsigned int flags) { Flags = flags; }
//	void								Set_Flag(unsigned int flag, Bool onoff) { Flags &= (~flag); if (onoff) Flags |= flag; }

	int updateBlock(void);
	Int freeMapResources(void);
	void static setColor(Int r, Int g, Int b) {m_needUpdate = true; m_diffuse = (b) + (g<<8) + (r<<16);};
protected:
	Int	m_numTriangles;	//dimensions of list
	static Int m_diffuse;
	static Bool			 m_needUpdate;

	DX8IndexBufferClass			*m_indexBuffer;	//indices defining a triangle strip the covers full terrain
	ShaderClass m_shaderClass; //shader or rendering state for heightmap
	VertexMaterialClass	  	  *m_vertexMaterialClass;
	DX8VertexBufferClass	*m_vertexBufferCircle;	//collection of vertexes that make the circle.
	DX8VertexBufferClass	*m_vertexBufferScreen;	//2 triangle quad that covers the screen.

	int initData(void);
	Int updateCircleVB(void);
	Int updateScreenVB(Int diffuse);
};
