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

#include "W3DDevice/GameClient/W3DTerrainParticle.h"

#include <algorithm>

#include "GameClient/ParticleSys.h"
#include "GameLogic/TerrainLogic.h"
#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "WW3D2/dx8indexbuffer.h"
#include "WW3D2/dx8vertexbuffer.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/rinfo.h"
#include "WW3D2/texture.h"
#include "WW3D2/vertmaterial.h"
#include "WWLib/refcount.h"
#include "WWMath/vector3.h"
#include "WWMath/vector4.h"
#include "WWMath/wwmath.h"

constexpr const Int MAX_VERTICES = 32768;
constexpr const Int MAX_INDICES = 65535;
constexpr const Int MAX_TILES_IN_BATCH = 32;
constexpr const Int MAX_BATCH_VERTICES = (MAX_TILES_IN_BATCH + 1) * (MAX_TILES_IN_BATCH + 1);
constexpr const Real Z_OFFSET = MAP_HEIGHT_SCALE / 10;
constexpr const Real Z_OFFSET_BRIDGE = MAP_HEIGHT_SCALE;

static_assert(MAX_BATCH_VERTICES <= MAX_VERTICES, "Tile block exceeds the batch vertex buffer");
static_assert(6 * MAX_TILES_IN_BATCH * MAX_TILES_IN_BATCH <= MAX_INDICES, "Tile block exceeds the batch index buffer");

namespace
{

enum CPP_11(: Int)
{
	U_MIN = 1 << 0,
	U_MAX = 1 << 1,
	V_MIN = 1 << 2,
	V_MAX = 1 << 3,
};

UnsignedByte getUVOutcode(const Real u, const Real v)
{
	UnsignedByte outcode = 0;
	if (u < 0.0f)
		outcode |= U_MIN;
	else if (u > 1.0f)
		outcode |= U_MAX;
	if (v < 0.0f)
		outcode |= V_MIN;
	else if (v > 1.0f)
		outcode |= V_MAX;
	return outcode;
}

Real getMapHeight(WorldHeightMap& map, Int x, Int y)
{
	x += map.getBorderSizeInline();
	y += map.getBorderSizeInline();
	return map.getDataPtr()[x + y * map.getXExtent()] * MAP_HEIGHT_SCALE;
}

Bool isOnBridge(const Vector3& loc, Real& height, Vector3& normal)
{
  if(!TheTerrainLogic->getFirstBridge())
		return false;

	Coord3D center;
	center.x = loc.X;
	center.y = loc.Y;
	center.z = loc.Z;

	PathfindLayerEnum layer = TheTerrainLogic->getLayerForDestination(&center);
	Bridge* bridge = TheTerrainLogic->findBridgeLayerAt(&center, layer);
	if (bridge)
	{
		Coord3D bridgeNormal;
		height = bridge->getBridgeHeight(&center, &bridgeNormal) + Z_OFFSET_BRIDGE;
		normal.Set(bridgeNormal.x, bridgeNormal.y, bridgeNormal.z);
		return true;
	}

	return false;
}

Bool isTerrainFlat(WorldHeightMap& map, const IRegion2D& bounds, Real& height)
{
	height = getMapHeight(map, bounds.lo.x, bounds.lo.y);
	for (Int j = bounds.lo.y; j < bounds.hi.y; j++)
		for (Int i = bounds.lo.x; i < bounds.hi.x; i++)
			if (fabsf(getMapHeight(map, i, j) - height) > Z_OFFSET)
				return false;
	height += Z_OFFSET;
	return true;
}

}

W3DTerrainParticle::W3DTerrainParticle()
  : m_vertexData(W3DNEWARRAY VertexFormatXYZNDUV2[MAX_VERTICES])
  , m_indexData(W3DNEWARRAY UnsignedShort[MAX_INDICES])
  , m_outcodes(W3DNEWARRAY UnsignedByte[MAX_BATCH_VERTICES])
  , m_numVertices(0)
  , m_numIndices(0)
  , m_texture(nullptr)
  , m_pointLoc(nullptr)
  , m_pointDiffuse(nullptr)
  , m_pointSize(nullptr)
  , m_pointOrientation(nullptr)
  , m_pointCount(0)
  , m_terrainInViewBounds()
  , m_defaultPointSize(0.0f)
  , m_defaultPointColor(1.0f, 1.0f, 1.0f)
  , m_defaultPointAlpha(1.0f)
  , m_defaultPointOrientation(0)
{
	m_defaultDiffuse = DX8Wrapper::Convert_Color_Clamp(Vector4(m_defaultPointColor.X, m_defaultPointColor.Y, m_defaultPointColor.Z, m_defaultPointAlpha));
}

W3DTerrainParticle::~W3DTerrainParticle()
{
	delete[] m_vertexData;
	delete[] m_indexData;
	delete[] m_outcodes;
	REF_PTR_RELEASE(m_texture);
	REF_PTR_RELEASE(m_pointLoc);
	REF_PTR_RELEASE(m_pointDiffuse);
	REF_PTR_RELEASE(m_pointSize);
	REF_PTR_RELEASE(m_pointOrientation);
}

void W3DTerrainParticle::setTexture(TextureClass* texture)
{
	REF_PTR_SET(m_texture, texture);
}

void W3DTerrainParticle::setShader(ShaderClass shader)
{
	m_shader = shader;
}

void W3DTerrainParticle::setArrays(
  ShareBufferClass<Vector3>* locs,
  ShareBufferClass<Vector4>* diffuse,
  ShareBufferClass<Real>* sizes,
  ShareBufferClass<UnsignedByte>* orientations,
  Int activePointCount)
{
	WWASSERT(locs);
	WWASSERT(activePointCount <= locs->Get_Count());

	// Ensure lengths of all arrays are the same
	WWASSERT(!diffuse || locs->Get_Count() == diffuse->Get_Count());
	WWASSERT(!sizes || locs->Get_Count() == sizes->Get_Count());
	WWASSERT(!orientations || locs->Get_Count() == orientations->Get_Count());

	REF_PTR_SET(m_pointLoc, locs);
	REF_PTR_SET(m_pointDiffuse, diffuse);
	REF_PTR_SET(m_pointSize, sizes);
	REF_PTR_SET(m_pointOrientation, orientations);

	m_pointCount = activePointCount >= 0 ? activePointCount : locs->Get_Count();
}

void W3DTerrainParticle::setBoundingBox(const AABoxClass& worldBoundingBox)
{
	m_terrainInViewBounds.lo.x = REAL_TO_INT_FLOOR((worldBoundingBox.Center.X - worldBoundingBox.Extent.X) / MAP_XY_FACTOR);
	m_terrainInViewBounds.hi.x = REAL_TO_INT_FLOOR((worldBoundingBox.Center.X + worldBoundingBox.Extent.X) / MAP_XY_FACTOR);
	m_terrainInViewBounds.lo.y = REAL_TO_INT_FLOOR((worldBoundingBox.Center.Y - worldBoundingBox.Extent.Y) / MAP_XY_FACTOR);
	m_terrainInViewBounds.hi.y = REAL_TO_INT_FLOOR((worldBoundingBox.Center.Y + worldBoundingBox.Extent.Y) / MAP_XY_FACTOR);
}

void W3DTerrainParticle::render()
{
	if (m_pointCount <= 0 || !m_pointLoc || !TheTerrainRenderObject)
		return;

	WorldHeightMap* map = TheTerrainRenderObject->getMap();
	if (!map)
		return;

	updateSettings();

	for (Int p = 0; p < m_pointCount; p++)
	{
		Vector3 loc = m_pointLoc->Get_Array()[p];
		UnsignedInt diffuse = m_pointDiffuse ? DX8Wrapper::Convert_Color_Clamp(m_pointDiffuse->Get_Array()[p]) : m_defaultDiffuse;
		Real size = m_pointSize ? m_pointSize->Get_Array()[p] : m_defaultPointSize;
		UnsignedByte orientation = m_pointOrientation ? m_pointOrientation->Get_Array()[p] : m_defaultPointOrientation;

		processParticle(*map, loc, diffuse, size, orientation);
	}

	flushBatch();

	// Restore the texture state.
	if (m_texture)
	{
		m_texture->Get_Filter().Apply(0);
	}
}

void W3DTerrainParticle::processParticle(WorldHeightMap& map, const Vector3& loc, UnsignedInt diffuse, Real size, UnsignedByte orientation)
{
	const Real angle = orientation / 255.0f * 2.0f * WWMATH_PI;
	const Real cosine = WWMath::Fast_Cos(angle);
	const Real sine = WWMath::Fast_Sin(angle);
	const Real projectedRadius = size * (fabsf(cosine) + fabsf(sine));

	IRegion2D bounds = calcBounds(map, loc, projectedRadius);
	if (bounds.width() < 2 || bounds.height() < 2)
		return;

	Real z;
	Vector3 normal = Vector3(0.0f, 0.0f, 1.0f);
	if (isOnBridge(loc, z, normal) || isTerrainFlat(map, bounds, z))
	{
		drawQuad(loc, diffuse, size, cosine, sine, z, normal);
	}
	else
	{
		drawTerrainConformingMesh(map, loc, bounds, diffuse, size, cosine, sine);
	}
}

void W3DTerrainParticle::drawQuad(const Vector3& loc,
                                  UnsignedInt diffuse,
                                  Real size,
                                  Real cosine,
                                  Real sine,
                                  Real height,
                                  const Vector3& normal)
{
	if (m_numVertices + 4 > MAX_VERTICES || m_numIndices + 6 > MAX_INDICES)
	{
		flushBatch();
	}

	static constexpr const Real cornerU[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
	static constexpr const Real cornerV[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	const UnsignedShort baseVertex = m_numVertices;
	for (Int index = 0; index < 4; index++)
	{
		const Real localX = size * (1.0f - 2.0f * cornerU[index]);
		const Real localY = size * (1.0f - 2.0f * cornerV[index]);
		VertexFormatXYZNDUV2 vertex;
		vertex.diffuse = diffuse;
		vertex.x = loc.X + cosine * localX + sine * localY;
		vertex.y = loc.Y - sine * localX + cosine * localY;
		vertex.z = height;
		if (fabsf(normal.Z) > WWMATH_EPSILON)
		{
			const Real deltaX = vertex.x - loc.X;
			const Real deltaY = vertex.y - loc.Y;
			vertex.z -= (normal.X * deltaX + normal.Y * deltaY) / normal.Z;
		}
		vertex.nx = normal.X;
		vertex.ny = normal.Y;
		vertex.nz = normal.Z;
		vertex.u1 = cornerU[index];
		vertex.v1 = cornerV[index];
		vertex.u2 = 0.0f;
		vertex.v2 = 0.0f;
		m_outcodes[index] = 0;
		m_vertexData[m_numVertices++] = vertex;
	}

	addTriangle(baseVertex, 0, 1, 2);
	addTriangle(baseVertex, 0, 2, 3);
}

void W3DTerrainParticle::drawTerrainConformingMesh(WorldHeightMap& map, const Vector3& loc, const IRegion2D& bounds, UnsignedInt diffuse, Real size, Real cosine, Real sine)
{
	// Split the drawing into blocks of at most MAX_TILES_IN_BATCH cells per axis.
	// This lets large particles that exceed the batch buffers be drawn over multiple flushes.
	for (Int batchMinY = bounds.lo.y; batchMinY < bounds.hi.y - 1; batchMinY += MAX_TILES_IN_BATCH)
	{
		const Int batchMaxY = std::min(batchMinY + MAX_TILES_IN_BATCH + 1, bounds.hi.y);
		for (Int batchMinX = bounds.lo.x; batchMinX < bounds.hi.x - 1; batchMinX += MAX_TILES_IN_BATCH)
		{
			const Int batchMaxX = std::min(batchMinX + MAX_TILES_IN_BATCH + 1, bounds.hi.x);
			const Int batchWidth = batchMaxX - batchMinX;
			const Int batchHeight = batchMaxY - batchMinY;

			// The buffer is full and a draw to screen is needed.
			// Note that this estimate is conservative as it does not account for triangles filtered by outcodes.
			if (m_numVertices + batchWidth * batchHeight > MAX_VERTICES ||
			    m_numIndices + 6 * (batchWidth - 1) * (batchHeight - 1) > MAX_INDICES)
			{
				flushBatch();
			}

			Int i, j;
			const UnsignedShort baseVertex = m_numVertices;
			for (j = batchMinY; j < batchMaxY; j++)
			{
				for (i = batchMinX; i < batchMaxX; i++)
				{
					VertexFormatXYZNDUV2 vertex;
					vertex.diffuse = diffuse;
					vertex.x = i * MAP_XY_FACTOR;
					vertex.y = j * MAP_XY_FACTOR;
					vertex.z = getMapHeight(map, i, j) + Z_OFFSET;
					// The vertex normal is not used by the renderer and does not align with the terrain.
					vertex.nx = 0.0f;
					vertex.ny = 0.0f;
					vertex.nz = 1.0f;
					const Real deltaX = vertex.x - loc.X;
					const Real deltaY = vertex.y - loc.Y;
					const Real localX = cosine * deltaX - sine * deltaY;
					const Real localY = sine * deltaX + cosine * deltaY;
					vertex.u1 = 0.5f - localX / (2.0f * size);
					vertex.v1 = 0.5f - localY / (2.0f * size);
					vertex.u2 = 0.0f;
					vertex.v2 = 0.0f;
					m_outcodes[(j - batchMinY) * batchWidth + i - batchMinX] = getUVOutcode(vertex.u1, vertex.v1);
					m_vertexData[m_numVertices++] = vertex;
				}
			}

			for (j = 0; j < batchHeight - 1; j++)
			{
				for (i = 0; i < batchWidth - 1; i++)
				{
					const UnsignedShort topLeftOffset = j * batchWidth + i;
					const UnsignedShort topRightOffset = topLeftOffset + 1;
					const UnsignedShort bottomLeftOffset = topLeftOffset + batchWidth;
					const UnsignedShort bottomRightOffset = bottomLeftOffset + 1;
					const Int mapCellX = batchMinX + i + map.getBorderSizeInline();
					const Int mapCellY = batchMinY + j + map.getBorderSizeInline();
					if (map.getQuickFlipState(mapCellX, mapCellY))
					{
						addTriangle(baseVertex, topRightOffset, bottomLeftOffset, topLeftOffset);
						addTriangle(baseVertex, topRightOffset, bottomRightOffset, bottomLeftOffset);
					}
					else
					{
						addTriangle(baseVertex, topLeftOffset, bottomRightOffset, bottomLeftOffset);
						addTriangle(baseVertex, topLeftOffset, topRightOffset, bottomRightOffset);
					}
				}
			}
		}
	}
}

inline void W3DTerrainParticle::addTriangle(UnsignedShort baseVertex,
	                                            UnsignedShort offsetA,
	                                            UnsignedShort offsetB,
	                                            UnsignedShort offsetC)
{
	if ((m_outcodes[offsetA] & m_outcodes[offsetB] & m_outcodes[offsetC]) != 0)
		return;

	m_indexData[m_numIndices++] = baseVertex + offsetA;
	m_indexData[m_numIndices++] = baseVertex + offsetB;
	m_indexData[m_numIndices++] = baseVertex + offsetC;
}

void W3DTerrainParticle::flushBatch()
{
	if (m_numIndices > 0 && m_numVertices > 0)
	{
		DynamicVBAccessClass vertexAccess(BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, m_numVertices);
		{
			DynamicVBAccessClass::WriteLockClass vertexLock(&vertexAccess);
			memcpy(vertexLock.Get_Formatted_Vertex_Array(),
			       m_vertexData,
			       m_numVertices * sizeof(VertexFormatXYZNDUV2));
		}

		DynamicIBAccessClass indexAccess(BUFFER_TYPE_DYNAMIC_DX8, m_numIndices);
		{
			DynamicIBAccessClass::WriteLockClass indexLock(&indexAccess);
			memcpy(indexLock.Get_Index_Array(),
			       m_indexData,
			       m_numIndices * sizeof(UnsignedShort));
		}

		DX8Wrapper::Set_Index_Buffer(indexAccess, 0);
		DX8Wrapper::Set_Vertex_Buffer(vertexAccess);
		DX8Wrapper::Draw_Triangles(0,
		                           m_numIndices / 3,
		                           0,
		                           m_numVertices);
	}

	m_numVertices = 0;
	m_numIndices = 0;
}

IRegion2D W3DTerrainParticle::calcBounds(WorldHeightMap& map, const Vector3& loc, Real projectedRadius) const
{
	IRegion2D bounds;

	bounds.lo.x = REAL_TO_INT_FLOOR((loc.X - projectedRadius) / MAP_XY_FACTOR);
	bounds.lo.y = REAL_TO_INT_FLOOR((loc.Y - projectedRadius) / MAP_XY_FACTOR);
	bounds.hi.x = REAL_TO_INT_CEIL((loc.X + projectedRadius) / MAP_XY_FACTOR) + 1;
	bounds.hi.y = REAL_TO_INT_CEIL((loc.Y + projectedRadius) / MAP_XY_FACTOR) + 1;

	bounds.intersect(map.getLogicalBounds());
	bounds.intersect(m_terrainInViewBounds);

	return bounds;
}

void W3DTerrainParticle::updateSettings()
{
	// If there is a color or alpha array enable gradient in shader - otherwise disable.
	const Real value255 = 0.9961f;    // 254 / 255
	const Bool defaultWhiteOpaque = m_defaultPointColor.X > value255 &&
	                                m_defaultPointColor.Y > value255 &&
	                                m_defaultPointColor.Z > value255 &&
	                                m_defaultPointAlpha > value255;

	// The reason we check for lack of texture here is that SR seems to render black triangles
	// rather than white triangles as would be expected) when there is no texture AND no gradient.
	if (m_pointDiffuse || !defaultWhiteOpaque || !m_texture)
	{
		m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
	}
	else
	{
		m_shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
	}

	// If m_texture is non-null enable texturing in shader - otherwise disable.
	if (m_texture)
	{
		m_shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
	}
	else
	{
		m_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
	}
	m_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_ENABLE);

	DX8Wrapper::Set_World_Identity();
	VertexMaterialClass* material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(material);
	REF_PTR_RELEASE(material);
	DX8Wrapper::Set_Shader(m_shader);
	DX8Wrapper::Set_Texture(0, m_texture);

	// To prevent visual glitches on overdraw we clamp the texture to a transparent black pixel.
	DX8Wrapper::Apply_Render_State_Changes();
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_BORDERCOLOR, 0x00000000);
}
