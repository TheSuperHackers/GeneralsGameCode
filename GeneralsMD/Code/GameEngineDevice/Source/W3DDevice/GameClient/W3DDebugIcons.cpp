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
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include <vector>
import Graphics.Frame.Runtime;
import Graphics.Scene.Debug.Renderer;

#include "Common/GlobalData.h"
#include "GameLogic/GameLogic.h"
#include "Common/MapObject.h"
import Graphics.Materials.State;


#if defined(RTS_DEBUG)

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_ALWAYS, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_ONE, \
	Graphics::MaterialState::DSTBLEND_ZERO, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_DISABLE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_DISABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_ALWAYS, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_SRC_ALPHA, \
	Graphics::MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_MODULATE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_ENABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA_Z ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_LEQUAL, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_SRC_ALPHA, \
	Graphics::MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_MODULATE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_DISABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE_Z ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_LEQUAL, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_ONE, \
	Graphics::MaterialState::DSTBLEND_ZERO, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_DISABLE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_DISABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )


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

DebugIcon	*W3DDebugIcons::m_debugIcons = nullptr;
Int				 W3DDebugIcons::m_numDebugIcons = 0;
Int				 W3DDebugIcons::m_maxDebugIcons = 0;

W3DDebugIcons::~W3DDebugIcons()
{
	Graphics::Get_Surface_Renderer().Destroy_Mesh(m_mesh);
	delete[] m_debugIcons;
	m_debugIcons = nullptr;
	m_numDebugIcons = 0;
}

W3DDebugIcons::W3DDebugIcons(Int mapWidth, Int mapHeight)
{
	m_maxDebugIcons = mapWidth * mapHeight;
	//go with a preset material for now.
	
	allocateIconsArray();
}


bool W3DDebugIcons::Cast_Ray(W3DRayCastQuery & raytest)
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

Int W3DDebugIcons::Class_ID() const
{
	return W3DRenderObject::CLASSID_UNKNOWN;
}

W3DRenderObject * W3DDebugIcons::Clone() const
{
	return NEW W3DDebugIcons(*this);	// poolify
}


void W3DDebugIcons::allocateIconsArray()
{
	DEBUG_ASSERTCRASH(m_debugIcons == nullptr, ("debugIcons array already allocated!"));
	m_debugIcons = NEW DebugIcon[m_maxDebugIcons];
	m_numDebugIcons = 0;
}


void W3DDebugIcons::compressIconsArray()
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
	if (pos==nullptr) {
		if (m_numDebugIcons > maxIcons) {
			DEBUG_LOG(("Max icons %d", m_numDebugIcons));
			maxIcons = m_numDebugIcons;
		}
		m_numDebugIcons = 0;
		return;
	}
	if (m_numDebugIcons>= m_maxDebugIcons) return;
	if (m_debugIcons==nullptr) return;
	m_debugIcons[m_numDebugIcons].position = *pos;
	m_debugIcons[m_numDebugIcons].width = width;
	m_debugIcons[m_numDebugIcons].color = color;
	m_debugIcons[m_numDebugIcons].endFrame = TheGameLogic->getFrame()+numFramesDuration;
	m_numDebugIcons++;
}

/** Render draws into the current 3d context. */
void W3DDebugIcons::Render(W3DRenderContext& info)
{
    if (Graphics::Get_Scene_Draw_Queue().Is_Enabled()) {
        Graphics::Get_Scene_Draw_Queue().Enqueue<Extract_Ordered_Draw>(1, *this); return;
    }
    auto* device=Graphics::Shared_Frame_Device();
    if (!device || !m_numDebugIcons) return;
    std::vector<Graphics::SurfaceVertex> vertices;
    std::vector<std::uint32_t> indices;
    bool vanished=false;
    for (int k=0;k<m_numDebugIcons;++k) {
        const auto& icon=m_debugIcons[k];
        const int frames=icon.endFrame-TheGameLogic->getFrame();
        if (frames<1) { vanished=true; continue; }
        const unsigned alpha=frames<100 ? static_cast<unsigned>(64.0f*frames/100) : 64;
        const unsigned color=icon.color.getAsInt() | (alpha<<24);
        const float width=icon.width*0.5f;
        const std::array<Vector3,4> corners{
            Vector3(icon.position.x-width,icon.position.y-width,icon.position.z),
            Vector3(icon.position.x+width,icon.position.y-width,icon.position.z),
            Vector3(icon.position.x+width,icon.position.y+width,icon.position.z),
            Vector3(icon.position.x-width,icon.position.y+width,icon.position.z)};
        const auto base=static_cast<std::uint32_t>(vertices.size());
        for (const auto& corner : corners) {
            Vector3 point; Matrix3D::Transform_Vector(Get_Transform(),corner,&point);
            Graphics::SurfaceVertex vertex;
            vertex.position={point.X,point.Y,point.Z};
            vertex.color={float((color>>16)&255)/255,float((color>>8)&255)/255,float(color&255)/255,float(color>>24)/255};
            vertices.push_back(vertex);
        }
        indices.insert(indices.end(),{base,base+1,base+2,base,base+2,base+3});
    }
    if (!indices.empty()) Graphics::Draw_Debug_Geometry(Graphics::Get_Surface_Renderer(),
        device->Immediate_Command_List(),m_mesh,vertices,indices,Make_Surface_Parameters(info.Camera));
    if (vanished) compressIconsArray();
}

#endif // RTS_DEBUG
