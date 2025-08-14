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

// FILE: W3DDebugIcons.cpp ////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: Heightmap.cpp
//
// Created:   John Ahlquist, March 2002
//
// Desc:      Draws huge numbers of debug icons for pathfinding quickly.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes
//-----------------------------------------------------------------------------

#include "W3DDevice/GameClient/W3DDebugIcons.h"

#include "Common/GlobalData.h"
#include "GameLogic/GameLogic.h"
#include "Common/MapObject.h"
#include "WW3D2/dx8wrapper.h"

#if defined(RTS_DEBUG)

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_DISABLE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA_Z ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE_Z ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_DISABLE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )


void addIcon(const Coord3D *pos, Real width, Int numFramesDuration, RGBColor color)
{
	W3DDebugIcons::addIcon(pos, width, numFramesDuration, color);
}


struct DebugIcon {
	Coord3D position;
	Real		width; // all are squares centered about pos.
	RGBColor color;
	Int			endFrame; // Frame when this disappears.
};

DebugIcon	*W3DDebugIcons::m_debugIcons = NULL;
Int				 W3DDebugIcons::m_numDebugIcons = 0;

W3DDebugIcons::~W3DDebugIcons(void)
{
	REF_PTR_RELEASE(m_vertexMaterialClass);
	if (m_debugIcons) delete m_debugIcons;
	m_debugIcons = NULL;
	m_numDebugIcons = 0;
}

W3DDebugIcons::W3DDebugIcons(void)

{
	//go with a preset material for now.
	m_vertexMaterialClass=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	allocateIconsArray();
}


bool W3DDebugIcons::Cast_Ray(RayCollisionTestClass & raytest)
{

	return false;

}


//@todo: MW Handle both of these properly!!
W3DDebugIcons::W3DDebugIcons(const W3DDebugIcons & src)
{
	*this = src;
}

W3DDebugIcons & W3DDebugIcons::operator = (const W3DDebugIcons & that)
{
	DEBUG_CRASH(("oops"));
	return *this;
}

void W3DDebugIcons::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	Vector3	ObjSpaceCenter(TheGlobalData->m_waterExtentX,TheGlobalData->m_waterExtentY,50*MAP_XY_FACTOR);
	float length = ObjSpaceCenter.Length();

	sphere.Init(ObjSpaceCenter, length);
}

void W3DDebugIcons::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	Vector3	minPt(-2*TheGlobalData->m_waterExtentX,-2*TheGlobalData->m_waterExtentY,0);
	Vector3	maxPt(2*TheGlobalData->m_waterExtentX,2*TheGlobalData->m_waterExtentY,100*MAP_XY_FACTOR);
	box.Init(minPt,maxPt);
}

Int W3DDebugIcons::Class_ID(void) const
{
	return RenderObjClass::CLASSID_UNKNOWN;
}

RenderObjClass * W3DDebugIcons::Clone(void) const
{
	return NEW W3DDebugIcons(*this);	// poolify
}


void W3DDebugIcons::allocateIconsArray(void)
{
	m_debugIcons = NEW DebugIcon[MAX_ICONS];
	m_numDebugIcons = 0;
}


void W3DDebugIcons::compressIconsArray(void)
{
	if (m_debugIcons && m_numDebugIcons > 0) {
		Int newNum = 0;
		Int i;
		for (i=0; i<m_numDebugIcons; i++) {
			if (m_debugIcons[i].endFrame >= TheGameLogic->getFrame() && i>newNum) {
				m_debugIcons[newNum] = m_debugIcons[i];
				newNum++;
			}
		}
		m_numDebugIcons = newNum;
	}
}

static Int maxIcons = 0;

void W3DDebugIcons::addIcon(const Coord3D *pos, Real width, Int numFramesDuration, RGBColor color)
{
	if (pos==NULL) {
		if (m_numDebugIcons > maxIcons) {
			DEBUG_LOG(("Max icons %d", m_numDebugIcons));
			maxIcons = m_numDebugIcons;
		}
		m_numDebugIcons = 0;
		return;
	}
	if (m_numDebugIcons>= MAX_ICONS) return;
	if (m_debugIcons==NULL) return;
	m_debugIcons[m_numDebugIcons].position = *pos;
	m_debugIcons[m_numDebugIcons].width = width;
	m_debugIcons[m_numDebugIcons].color = color;
	m_debugIcons[m_numDebugIcons].endFrame = TheGameLogic->getFrame()+numFramesDuration;
	m_numDebugIcons++;
}

/** Render draws into the current 3d context. */
void W3DDebugIcons::Render(RenderInfoClass & rinfo)
{
	//
	if (WW3D::Are_Static_Sort_Lists_Enabled()) {
		WW3D::Add_To_Static_Sort_List(this, 1);
		return;
	}
	//
	Bool anyVanished = false;
	if (m_numDebugIcons==0) return;
	DX8Wrapper::Apply_Render_State_Changes();

	DX8Wrapper::Set_Material(m_vertexMaterialClass);
	DX8Wrapper::Set_Texture(0, NULL);
	DX8Wrapper::Apply_Render_State_Changes();

	Matrix3D tm(Transform);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);

	Int numRect = m_numDebugIcons;
	static Real offset = 30;
	const Int MAX_RECT = 5000;  // cap drawing n rects.
	if (numRect > MAX_RECT) numRect = MAX_RECT;
	offset+= 0.5f;
	Int k;
	for (k=0; k<m_numDebugIcons;) {
		Int curIndex = 0;
		Int	numVertex = 0;
		DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8,DX8_FVF_XYZNDUV2,numRect*4);
		DynamicIBAccessClass ib_access(BUFFER_TYPE_DYNAMIC_DX8,numRect*6);
		{
		DynamicVBAccessClass::WriteLockClass lock(&vb_access);
		VertexFormatXYZNDUV2* vb= lock.Get_Formatted_Vertex_Array();
		DynamicIBAccessClass::WriteLockClass lockib(&ib_access);
		if (!vb) return;

		UnsignedShort *ib=lockib.Get_Index_Array();
		UnsignedShort *curIb = ib;

//		VertexFormatXYZNDUV2 *curVb = vb;
 		Real shadeR, shadeG, shadeB;
		shadeR = 0;
		shadeG = 0;
		shadeB = 255;
		for(;  numVertex<numRect*4 && k<m_numDebugIcons; k++) {
			Int theAlpha = 64;
			const Int FADE_FRAMES = 100;
			Int framesLeft = m_debugIcons[k].endFrame - TheGameLogic->getFrame();
			if (framesLeft < 1) {
				anyVanished = true;
				continue;
			}
			if (framesLeft<FADE_FRAMES) {
				theAlpha *= (Real)framesLeft/FADE_FRAMES;
			}
			RGBColor clr = m_debugIcons[k].color;
			Real halfWidth = m_debugIcons[k].width/2;
			Int diffuse = clr.getAsInt() | ((int)theAlpha << 24);
			Coord3D pt1 = m_debugIcons[k].position;
			vb->x=	pt1.x-halfWidth;
			vb->y=	pt1.y-halfWidth;
			vb->z=  pt1.z;
			vb->diffuse=diffuse;	 // b g<<8 r<<16 a<<24.
			vb->u1=0 ;
			vb->v1=0 ;
			vb++;
			vb->x=	pt1.x+halfWidth;
			vb->y=	pt1.y-halfWidth;
			vb->z=  pt1.z;
			vb->diffuse=diffuse;	 // b g<<8 r<<16 a<<24.
			vb->u1=0 ;
			vb->v1=0 ;
			vb++;
			vb->x=	pt1.x+halfWidth;
			vb->y=	pt1.y+halfWidth;
			vb->z=  pt1.z;
			vb->diffuse=diffuse;	 // b g<<8 r<<16 a<<24.
			vb->u1=0 ;
			vb->v1=0 ;
			vb++;
			vb->x=	pt1.x-halfWidth;
			vb->y=	pt1.y+halfWidth;
			vb->z=  pt1.z;
			vb->diffuse=diffuse;	 // b g<<8 r<<16 a<<24.
			vb->u1=0 ;
			vb->v1=0 ;
			vb++;
			*curIb++ = numVertex;
			*curIb++ = numVertex+1;
			*curIb++ = numVertex+2;
			*curIb++ = numVertex;
			*curIb++ = numVertex+2;
			*curIb++ = numVertex+3;
			curIndex += 6;
			numVertex += 4;
		}
		}
		if (numVertex == 0) break;
		DX8Wrapper::Set_Shader(ShaderClass(SC_ALPHA));
		DX8Wrapper::Set_Index_Buffer(ib_access,0);
		DX8Wrapper::Set_Vertex_Buffer(vb_access);
		DX8Wrapper::Draw_Triangles(	0,curIndex/3, 0,	numVertex);	//draw a quad, 2 triangles, 4 verts
	}

	if (anyVanished) {
		compressIconsArray();
	}
}

#endif // RTS_DEBUG
