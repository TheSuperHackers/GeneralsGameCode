#include <algorithm>
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

// FILE: W3DBibBuffer.cpp ////////////////////////////////////////////////
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
// File name: W3DBibBuffer.cpp
//
// Created:   John Ahlquist, May 2001
//
// Desc:      Draw buffer to handle all the bibs in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes
//-----------------------------------------------------------------------------

#include "W3DDevice/GameClient/W3DBibBuffer.h"

#include <W3DDevice/GameClient/W3DTextureHandle.h>
#include "Common/GlobalData.h"
#include "Common/RandomValue.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DCamera.h"

#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"

#include <array>
#include <cstddef>
#include <span>
#include <vector>

import Graphics.Scene.Bibs.Renderer;
import Graphics.Frame.Runtime;
#include "W3DDevice/GameClient/W3DGraphicsResources.h"

//-----------------------------------------------------------------------------
//         Private Data
//-----------------------------------------------------------------------------
W3DBibBuffer* W3DBibBuffer::s_current=nullptr;

//-----------------------------------------------------------------------------
//         Private Functions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// W3DBibBuffer::~W3DBibBuffer
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DBibBuffer::~W3DBibBuffer()
{
	if (s_current == this)
		s_current = nullptr;
	freeBibBuffers();
    REF_PTR_RELEASE(m_bibTexture);
    REF_PTR_RELEASE(m_highlightBibTexture);
}

//=============================================================================
// W3DBibBuffer::W3DBibBuffer
//=============================================================================
/** Retains the normal and highlighted bib texture assets. */
//=============================================================================
W3DBibBuffer::W3DBibBuffer()
{
	s_current = this;
	clearAllBibs();
    m_bibTexture=NEW_REF(W3DTextureHandle,("TBBib.tga"));
    m_highlightBibTexture=NEW_REF(W3DTextureHandle,("TBRedBib.tga"));
}

void W3DBibBuffer::Release_Graphics_Bibs() noexcept
{
    if (s_current) s_current->freeBibBuffers();
}
void W3DBibBuffer::freeBibBuffers()
{
    for (auto& mesh : m_graphicsMeshes) {
        Graphics::Get_Surface_Renderer().Destroy_Mesh(mesh);
        mesh={};
    }
}
void W3DBibBuffer::allocateBibBuffers() {}

//=============================================================================
// W3DBibBuffer::clearAllBibs
//=============================================================================
/** Removes all bibs. */
//=============================================================================
void W3DBibBuffer::clearAllBibs()
{
	m_numBibs=0;
	m_anythingChanged = true;
/* test bib
	Vector3 corners[4];
	corners[0].Set(0, 0, 20);
	corners[1].Set(100, 0, 20);
	corners[2].Set(100,100,20);
	corners[3].Set(0,100,20);
	addBib(corners, 1, false);
*/
}

//=============================================================================
// W3DBibBuffer::removeHighlighting
//=============================================================================
/** Clears highlighting flag.   */
//=============================================================================
void W3DBibBuffer::removeHighlighting()
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		m_bibs[bibIndex].m_highlight = false;
	}
}

//=============================================================================
// W3DBibBuffer::addBib
//=============================================================================
/** Adds a bib.   */
//=============================================================================
void W3DBibBuffer::addBib(Vector3 corners[4], ObjectID id, Bool highlight)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (!m_bibs[bibIndex].m_unused && m_bibs[bibIndex].m_objectID == id) {
			break;
		}
	}
	if (bibIndex==m_numBibs) {
		for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
			if (m_bibs[bibIndex].m_unused) {
				break;
			}
		}
	}
	if (bibIndex==m_numBibs) {
		if (m_numBibs >= MAX_BIBS) {
			return;
		}
		m_numBibs++;
	}
	m_anythingChanged = true;
	m_bibs[bibIndex].m_corners[0] = corners[0];
	m_bibs[bibIndex].m_corners[1] = corners[1];
	m_bibs[bibIndex].m_corners[2] = corners[2];
	m_bibs[bibIndex].m_corners[3] = corners[3];
	m_bibs[bibIndex].m_highlight = highlight;
	m_bibs[bibIndex].m_color = 0; // for now.
	m_bibs[bibIndex].m_unused = false; // for now.
	m_bibs[bibIndex].m_objectID = id;
	m_bibs[bibIndex].m_drawableID = INVALID_DRAWABLE_ID;
}

//=============================================================================
// W3DBibBuffer::addBib
//=============================================================================
/** Adds a bib.   */
//=============================================================================
void W3DBibBuffer::addBibDrawable(Vector3 corners[4], DrawableID id, Bool highlight)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (!m_bibs[bibIndex].m_unused && m_bibs[bibIndex].m_drawableID == id) {
			break;
		}
	}
	if (bibIndex==m_numBibs) {
		for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
			if (m_bibs[bibIndex].m_unused) {
				break;
			}
		}
	}
	if (bibIndex==m_numBibs) {
		if (m_numBibs >= MAX_BIBS) {
			return;
		}
		m_numBibs++;
	}
	m_anythingChanged = true;
	m_bibs[bibIndex].m_corners[0] = corners[0];
	m_bibs[bibIndex].m_corners[1] = corners[1];
	m_bibs[bibIndex].m_corners[2] = corners[2];
	m_bibs[bibIndex].m_corners[3] = corners[3];
	m_bibs[bibIndex].m_highlight = highlight;
	m_bibs[bibIndex].m_color = 0; // for now.
	m_bibs[bibIndex].m_unused = false; // for now.
	m_bibs[bibIndex].m_objectID = INVALID_ID;
	m_bibs[bibIndex].m_drawableID = id;
}

//=============================================================================
// W3DBibBuffer::removeBib
//=============================================================================
/** Removes a bib.  */
//=============================================================================
void W3DBibBuffer::removeBib(ObjectID id)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (m_bibs[bibIndex].m_objectID == id) {
			m_bibs[bibIndex].m_unused = true;
			m_bibs[bibIndex].m_objectID = INVALID_ID;
			m_bibs[bibIndex].m_drawableID = INVALID_DRAWABLE_ID;
			m_anythingChanged = true;
		}
	}
}

//=============================================================================
// W3DBibBuffer::removeBib
//=============================================================================
/** Removes a bib.  */
//=============================================================================
void W3DBibBuffer::removeBibDrawable(DrawableID id)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (m_bibs[bibIndex].m_drawableID == id) {
			m_bibs[bibIndex].m_unused = true;
			m_bibs[bibIndex].m_objectID = INVALID_ID;
			m_bibs[bibIndex].m_drawableID = INVALID_DRAWABLE_ID;
			m_anythingChanged = true;
		}
	}
}


//=============================================================================
// W3DBibBuffer::drawBibs
//=============================================================================
/** Draws the bibs.  Uses camera to cull. */
//=============================================================================
void W3DBibBuffer::renderBibs(W3DCamera& camera)
{
    auto* device=Graphics::Shared_Frame_Device();
    if (!device || !TheGlobalData) return;
    const auto parameters=Make_Surface_Parameters(camera);
    std::array<float,4> color{1,1,1,1};
    color[0]=TheGlobalData->m_terrainAmbient[0].red+TheGlobalData->m_terrainDiffuse[0].red;
    color[1]=TheGlobalData->m_terrainAmbient[0].green+TheGlobalData->m_terrainDiffuse[0].green;
    color[2]=TheGlobalData->m_terrainAmbient[0].blue+TheGlobalData->m_terrainDiffuse[0].blue;
    for (unsigned i=0;i<3;++i) color[i]=REAL_TO_INT(std::min(color[i],1.0f)*255)/255.0f;
    for (int highlight=0;highlight<2;++highlight) {
        std::vector<Graphics::BibQuad> quads;
        for (int i=0;i<m_numBibs;++i) {
            const auto& bib=m_bibs[i];
            if (bib.m_unused || bib.m_highlight!=Bool(highlight)) continue;
            Graphics::BibQuad quad;
            for (unsigned corner=0;corner<4;++corner) {
                const auto& position=bib.m_corners[corner];
                quad.corners[corner]={position.X,position.Y,position.Z};
            }
            quads.push_back(quad);
        }
        Graphics::Draw_Bibs(Graphics::Get_Surface_Renderer(),device->Immediate_Command_List(),
            m_graphicsMeshes[highlight],quads,color,parameters,
            Resolve_Graphics_Texture(highlight ? m_highlightBibTexture : m_bibTexture));
    }
}
