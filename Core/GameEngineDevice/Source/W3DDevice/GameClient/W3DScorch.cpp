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

#include "W3DDevice/GameClient/W3DScorch.h"

#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "WW3D2/dx8wrapper.h"

W3DScorch::W3DScorch()
  : m_vertexScorch(nullptr)
  , m_indexScorch(nullptr)
  , m_scorchTexture(nullptr)
  , m_curNumScorchVertices(0)
  , m_curNumScorchIndices(0)
  , m_numScorches(0)
  , m_scorchesInBuffer(0)
{}

W3DScorch::~W3DScorch() { freeBuffers(); }

void W3DScorch::allocateBuffers()
{
	m_vertexScorch = NEW_REF(DX8VertexBufferClass, (DX8_FVF_XYZDUV1, MAX_SCORCH_VERTEX, DX8VertexBufferClass::USAGE_DEFAULT));
	m_indexScorch = NEW_REF(DX8IndexBufferClass, (MAX_SCORCH_INDEX));
	m_scorchTexture = NEW ScorchTextureClass;
	invalidateBuffers();
#ifdef RTS_DEBUG
	Vector3 loc(4 * MAP_XY_FACTOR, 4 * MAP_XY_FACTOR, 0);
	addScorch(loc, 1 * MAP_XY_FACTOR, SCORCH_1);
	loc.Y += 10 * MAP_XY_FACTOR;
	loc.X += 5 * MAP_XY_FACTOR;
	addScorch(loc, 3 * MAP_XY_FACTOR, SCORCH_1);
#endif
}

void W3DScorch::freeBuffers()
{
	REF_PTR_RELEASE(m_vertexScorch);
	REF_PTR_RELEASE(m_indexScorch);
	REF_PTR_RELEASE(m_scorchTexture);
}

void W3DScorch::clearAllScorches()
{
	m_numScorches = 0;
	invalidateBuffers();
}

void W3DScorch::invalidateBuffers()
{
	m_scorchesInBuffer = 0;
	m_curNumScorchVertices = 0;
	m_curNumScorchIndices = 0;
}

void W3DScorch::addScorch(Vector3 location, Real radius, Scorches type)
{
	if (m_numScorches >= MAX_SCORCH_MARKS)
	{
		Int i;
		for (i = 0; i < MAX_SCORCH_MARKS - 1; i++)
		{
			m_scorches[i] = m_scorches[i + 1];
		}
		m_numScorches--;
	}

	Int i;
	Real limit = radius / 4;
	for (i = 0; i < m_numScorches; i++)
	{
		if (abs(location.X - m_scorches[i].location.X) < limit &&
		    abs(location.Y - m_scorches[i].location.Y) < limit &&
		    abs(radius - m_scorches[i].radius) < limit &&
		    m_scorches[i].scorchType == type)
		{
			return;    // basically a duplicate.
		}
	}

	m_scorches[m_numScorches].location = location;
	m_scorches[m_numScorches].radius = radius;
	m_scorches[m_numScorches].scorchType = type;
	m_numScorches++;
	m_scorchesInBuffer = 0;    // force buffer regenerations.
}

void W3DScorch::drawScorches(WorldHeightMap* map)
{
	updateScorches(map);
	if (m_curNumScorchIndices == 0)
	{
		return;
	}
	DX8Wrapper::Set_Index_Buffer(m_indexScorch, 0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexScorch);
	DX8Wrapper::Set_Shader(ShaderClass::_PresetAlphaShader);

	DX8Wrapper::Set_Texture(0, m_scorchTexture);
	DX8Wrapper::Draw_Triangles(0, m_curNumScorchIndices / 3, 0, m_curNumScorchVertices);
}

static Real getMapHeight(WorldHeightMap* map, Int x, Int y)
{
	x += map->getBorderSizeInline();
	y += map->getBorderSizeInline();
	return map->getDataPtr()[x + y * map->getXExtent()] * MAP_HEIGHT_SCALE;
}

void W3DScorch::updateScorches(WorldHeightMap* map)
{
	if (m_scorchesInBuffer > 1)
	{
		return;
	}
	if (m_numScorches == 0)
	{
		return;
	}
	if (!m_indexScorch || !m_vertexScorch)
	{
		return;
	}
	if (!map)
		return;

	m_curNumScorchVertices = 0;
	m_curNumScorchIndices = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexScorch);
	UnsignedShort* ib = lockIdxBuffer.Get_Index_Array();
	UnsignedShort* curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexScorch);
	VertexFormatXYZDUV1* vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1* curVb = vb;

	Int curScorch;
	Real shadeR, shadeG, shadeB;
	shadeR = TheGlobalData->m_terrainAmbient[0].red;
	shadeG = TheGlobalData->m_terrainAmbient[0].green;
	shadeB = TheGlobalData->m_terrainAmbient[0].blue;
	shadeR += TheGlobalData->m_terrainDiffuse[0].red / 2;
	shadeG += TheGlobalData->m_terrainDiffuse[0].green / 2;
	shadeB += TheGlobalData->m_terrainDiffuse[0].blue / 2;
	shadeR *= 255.0f;
	shadeG *= 255.0f;
	shadeB *= 255.0f;
	Int diffuse = REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16) | ((int)255 << 24);
	m_scorchesInBuffer = 0;
	for (curScorch = m_numScorches - 1; curScorch >= 0; curScorch--)
	{
		m_scorchesInBuffer++;
		Real radius = m_scorches[curScorch].radius;
		Vector3 loc = m_scorches[curScorch].location;
		Int type = m_scorches[curScorch].scorchType;
		if (type < 0)
		{
			type = 0;
		}
		if (type >= SCORCH_MARKS_IN_TEXTURE)
		{
			type = 0;
		}
		Real amtToFloat = 0;
		amtToFloat = MAP_HEIGHT_SCALE / 10;

		Int minX = REAL_TO_INT_FLOOR((loc.X - radius) / MAP_XY_FACTOR);
		Int minY = REAL_TO_INT_FLOOR((loc.Y - radius) / MAP_XY_FACTOR);
		if (minX < -map->getBorderSizeInline())
			minX = -map->getBorderSizeInline();
		if (minY < -map->getBorderSizeInline())
			minY = -map->getBorderSizeInline();
		Int maxX = REAL_TO_INT_CEIL((loc.X + radius) / MAP_XY_FACTOR);
		Int maxY = REAL_TO_INT_CEIL((loc.Y + radius) / MAP_XY_FACTOR);
		maxX++;
		maxY++;
		if (maxX > map->getXExtent() - map->getBorderSizeInline())
		{
			maxX = map->getXExtent() - map->getBorderSizeInline();
		}
		if (maxY > map->getYExtent() - map->getBorderSizeInline())
		{
			maxY = map->getYExtent() - map->getBorderSizeInline();
		}
		Int startVertex = m_curNumScorchVertices;
		Int i, j;
		for (j = minY; j < maxY; j++)
		{
			for (i = minX; i < maxX; i++)
			{
				if (m_curNumScorchVertices >= MAX_SCORCH_VERTEX)
					return;
				curVb->diffuse = diffuse;
				Real theZ = amtToFloat + getMapHeight(map, i, j);
				// The scorchmarks are spaced out by 1.5 in the texture.
				Real uOffset = (type % SCORCH_PER_ROW) * 1.5f;
				Real vOffset = (type / SCORCH_PER_ROW) * 1.5f;
				Real X = i * MAP_XY_FACTOR;
				Real Y = j * MAP_XY_FACTOR;
				curVb->u1 = (uOffset + 0.5f + (X - loc.X) / (2 * radius)) / (SCORCH_PER_ROW + 1);
				curVb->v1 = (vOffset + 0.5f + (Y - loc.Y) / (2 * radius)) / (SCORCH_PER_ROW + 1);
				curVb->x = X;
				curVb->y = Y;
				curVb->z = theZ;
				curVb++;
				m_curNumScorchVertices++;
			}
		}
		Int yOffset = maxX - minX;
		for (j = 0; j < maxY - minY - 1; j++)
		{
			for (i = 0; i < maxX - minX - 1; i++)
			{
				if (m_curNumScorchIndices + 6 > MAX_SCORCH_INDEX)
					return;
				Int xNdx = i + minX + map->getBorderSizeInline();
				Int yNdx = j + minY + map->getBorderSizeInline();
				Bool flipForBlend = map->getFlipState(xNdx, yNdx);
#if 0
				UnsignedByte alpha[4];
				float UA[4], VA[4];
				m_map->getAlphaUVData(xNdx, yNdx, UA, VA, alpha, &flipForBlend);
#endif
				if (flipForBlend)
				{
					*curIb++ = startVertex + j * yOffset + i + 1;
					*curIb++ = startVertex + j * yOffset + i + yOffset;
					*curIb++ = startVertex + j * yOffset + i;
					*curIb++ = startVertex + j * yOffset + i + 1;
					*curIb++ = startVertex + j * yOffset + i + 1 + yOffset;
					*curIb++ = startVertex + j * yOffset + i + yOffset;
				}
				else
				{
					*curIb++ = startVertex + j * yOffset + i;
					*curIb++ = startVertex + j * yOffset + i + 1 + yOffset;
					*curIb++ = startVertex + j * yOffset + i + yOffset;
					*curIb++ = startVertex + j * yOffset + i;
					*curIb++ = startVertex + j * yOffset + i + 1;
					*curIb++ = startVertex + j * yOffset + i + 1 + yOffset;
				}
				m_curNumScorchIndices += 6;
			}
		}
	}
}
