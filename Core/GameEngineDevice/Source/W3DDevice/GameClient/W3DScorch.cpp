/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include <array>
#include <span>
#include <vector>
import Graphics.Scene.Scorches.Geometry;
import Graphics.Frame.Runtime;
import Assets.Math;
#include "W3DDevice/GameClient/W3DScorch.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"

#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
import Graphics.Materials.State;


W3DScorch::W3DScorch(bool deduplicateScorches)
  : m_scorchTexture(nullptr)
  , m_curNumScorchVertices(0)
  , m_curNumScorchIndices(0)
  , m_needBufferRecompute(true)
  , m_deduplicateScorches(deduplicateScorches)
{}

W3DScorch::~W3DScorch() { freeBuffers(); }

void W3DScorch::allocateBuffers()
{
	freeBuffers();
	m_scorchTexture = NEW ScorchTextureClass;
	invalidateBuffers();
}

void W3DScorch::freeBuffers()
{
	Graphics::Get_Surface_Renderer().Destroy_Mesh(m_graphicsMesh);
	m_graphicsMesh = {};
	REF_PTR_RELEASE(m_scorchTexture);
}

void W3DScorch::clearAllScorches()
{
	m_scorches.clear();
	invalidateBuffers();
}

void W3DScorch::invalidateBuffers()
{
	m_needBufferRecompute = true;
	m_curNumScorchVertices = 0;
	m_curNumScorchIndices = 0;
}

void W3DScorch::invalidateTexture()
{
	if (m_scorchTexture)
	{
		m_scorchTexture->Invalidate();
	}
}

void W3DScorch::addScorch(Vector3 location, Real radius, Scorches type)
{
	TScorch scorch;
	scorch.location = location;
	scorch.radius = radius;
	if (type >= 0 && (Int)type < SCORCH_MARKS_IN_TEXTURE)
		scorch.scorchType = type;
	else
		scorch.scorchType = SCORCH_1;

	if (m_deduplicateScorches && isDuplicate(scorch))
	{
		return;
	}

	if ((Int)m_scorches.size() >= MAX_SCORCH_MARKS)
	{
		m_scorches.pop_front();
	}
	m_scorches.push_back(scorch);

	invalidateBuffers();
}

Bool W3DScorch::isDuplicate(const TScorch& scorch) const
{
	const Real limit = scorch.radius / 4;
	for (std::deque<TScorch>::const_iterator it = m_scorches.begin(); it != m_scorches.end(); ++it)
	{
		if (it->scorchType == scorch.scorchType &&
		    fabsf(scorch.location.X - it->location.X) < limit &&
		    fabsf(scorch.location.Y - it->location.Y) < limit &&
		    fabsf(scorch.radius - it->radius) < limit)
		{
			return true;
		}
	}
	return false;
}

void W3DScorch::drawScorches(WorldHeightMap& map, W3DCamera& camera)
{
    updateScorches(map);
    auto* device = Graphics::Shared_Frame_Device();
    if (!device || m_curNumScorchIndices == 0) return;
    Graphics::SurfaceStyle style;
    style.cull = Graphics::RHICullMode::Back;
    const auto texture = Resolve_Graphics_Texture(m_scorchTexture);
    Graphics::Get_Surface_Renderer().Draw(device->Immediate_Command_List(), m_graphicsMesh,
        style, Make_Surface_Parameters(camera), std::array<Graphics::RHITextureHandle, 4>{texture, {}, {}, {}});
}

void W3DScorch::updateScorches(WorldHeightMap& map)
{
    if (!m_needBufferRecompute || !m_scorchTexture) return;
    Graphics::ScorchGeometry geometry;
    const Graphics::ScorchGrid grid{map.getXExtent(), map.getYExtent(), map.getBorderSizeInline(),
        MAP_XY_FACTOR, MAP_HEIGHT_SCALE / 10};
    const auto& ambient = TheGlobalData->m_terrainAmbient[0];
    const auto& diffuse = TheGlobalData->m_terrainDiffuse[0];
    const auto packed = Assets::Color_To_ARGB({
        (ambient.red + diffuse.red) / 2, (ambient.green + diffuse.green) / 2,
        (ambient.blue + diffuse.blue) / 2, 1});
    const std::array<float, 4> color{((packed >> 16) & 255) / 255.0f,
        ((packed >> 8) & 255) / 255.0f, (packed & 255) / 255.0f, 1};
    for (auto it = m_scorches.rbegin(); it != m_scorches.rend(); ++it) {
        if (!geometry.Append({{it->location.X, it->location.Y}, it->radius, unsigned(it->scorchType)}, grid, color,
            [&](int x, int y) { return map.getDataPtr()[x + y * map.getXExtent()] * MAP_HEIGHT_SCALE; },
            [&](int x, int y) { return map.getFlipState(x, y); })) break;
    }
    auto& renderer = Graphics::Get_Surface_Renderer();
    if (m_graphicsMesh.Is_Valid()) {
        if (!renderer.Update_Mesh(m_graphicsMesh, geometry.vertices, geometry.indices)) return;
    } else {
        m_graphicsMesh = renderer.Create_Mesh(geometry.vertices, geometry.indices);
        if (!m_graphicsMesh.Is_Valid()) return;
    }
    m_curNumScorchVertices = static_cast<Int>(geometry.vertices.size());
    m_curNumScorchIndices = static_cast<Int>(geometry.indices.size());
    m_needBufferRecompute = false;
}

