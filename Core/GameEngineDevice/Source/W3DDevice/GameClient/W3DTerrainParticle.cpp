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

enum CPP_11( : UnsignedByte)
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

struct W3DTerrainParticle::ParticleContext
{
	Vector3 loc;
	IRegion2D bounds;
	UnsignedInt diffuse;
	Real size;
	Real cosine;
	Real sine;
};

W3DTerrainParticle::W3DTerrainParticle()
  : m_vertexData(MAX_VERTICES)
  , m_indexData(MAX_INDICES)
  , m_outcodes(MAX_VERTICES)
  , m_vertexLookup(MAX_VERTICES)
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
	REF_PTR_RELEASE(m_texture);
	REF_PTR_RELEASE(m_pointLoc);
	REF_PTR_RELEASE(m_pointDiffuse);
	REF_PTR_RELEASE(m_pointSize);
	REF_PTR_RELEASE(m_pointOrientation);
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

		const Real angle = orientation / 255.0f * 2.0f * WWMATH_PI;
		const Real cosine = WWMath::Fast_Cos(angle);
		const Real sine = WWMath::Fast_Sin(angle);
		const Real projectedRadius = size * (fabsf(cosine) + fabsf(sine));

		const IRegion2D bounds = calcTerrainBounds(*map, loc, projectedRadius);
		if (bounds.width() < 2 || bounds.height() < 2)
			continue;

		ParticleContext particle;
		particle.loc = loc;
		particle.bounds = bounds;
		particle.diffuse = diffuse;
		particle.size = size;
		particle.cosine = cosine;
		particle.sine = sine;

		resetVertexLookup();
		drawRegion(*map, particle, bounds);
	}

	flushBatch();

	// Restore the texture state.
	if (m_texture)
	{
		m_texture->Get_Filter().Apply(0);
	}
}

void W3DTerrainParticle::drawRegion(WorldHeightMap& map, const ParticleContext& particle, const IRegion2D& bounds)
{
	if (bounds.width() == 2 && bounds.height() == 2 || map.isTerrainFlat(bounds))
	{
		drawQuad(map, particle, bounds);
		return;
	}

	// Subdivide the current region into four sub-regions, or two sub-regions if we can't split one of the sides.
	const Bool splitX = bounds.width() > 2;
	const Bool splitY = bounds.height() > 2;
	const Int midX = bounds.lo.x + (bounds.width() - 1) / 2;
	const Int midY = bounds.lo.y + (bounds.height() - 1) / 2;
	for (Int y = 0; y < (splitY ? 2 : 1); y++)
	{
		for (Int x = 0; x < (splitX ? 2 : 1); x++)
		{
			IRegion2D child;
			child.lo.x = x == 0 ? bounds.lo.x : midX;
			child.hi.x = splitX && x == 0 ? midX + 1 : bounds.hi.x;
			child.lo.y = y == 0 ? bounds.lo.y : midY;
			child.hi.y = splitY && y == 0 ? midY + 1 : bounds.hi.y;
			drawRegion(map, particle, child);
		}
	}
}

void W3DTerrainParticle::drawQuad(WorldHeightMap& map, const ParticleContext& particle, const IRegion2D& bounds)
{
	if (m_numVertices + 4 > MAX_VERTICES || m_numIndices + 6 > MAX_INDICES)
	{
		flushBatch();
	}

	const UnsignedShort bottomLeft = addVertex(map, particle, bounds.lo.x, bounds.lo.y);
	const UnsignedShort bottomRight = addVertex(map, particle, bounds.hi.x - 1, bounds.lo.y);
	const UnsignedShort topLeft = addVertex(map, particle, bounds.lo.x, bounds.hi.y - 1);
	const UnsignedShort topRight = addVertex(map, particle, bounds.hi.x - 1, bounds.hi.y - 1);

	const Bool flipped = map.getQuickFlipState(bounds.lo.x + map.getBorderSizeInline(),
	                                           bounds.lo.y + map.getBorderSizeInline());
	if (flipped)
	{
		addTriangle(bottomRight, topLeft, bottomLeft);
		addTriangle(bottomRight, topRight, topLeft);
	}
	else
	{
		addTriangle(bottomLeft, topRight, topLeft);
		addTriangle(bottomLeft, bottomRight, topRight);
	}
}

UnsignedShort W3DTerrainParticle::addVertex(WorldHeightMap& map, const ParticleContext& particle, Int x, Int y)
{
	const Int gridLocation = (y - particle.bounds.lo.y) * particle.bounds.width() + x - particle.bounds.lo.x;
	UnsignedShort& index = m_vertexLookup[gridLocation];

	// Vertex may not exist yet, or got removed in the last flush.
	if (index != INVALID_VERTEX)
		return index;

	index = m_numVertices++;
	VertexFormatXYZNDUV2& vertex = m_vertexData[index];
	vertex.diffuse = particle.diffuse;
	vertex.x = x * MAP_XY_FACTOR;
	vertex.y = y * MAP_XY_FACTOR;
	vertex.z = map.getQuickHeight(x + map.getBorderSizeInline(), y + map.getBorderSizeInline()) * MAP_HEIGHT_SCALE + Z_OFFSET;
	const Real deltaX = vertex.x - particle.loc.X;
	const Real deltaY = vertex.y - particle.loc.Y;
	const Real localX = particle.cosine * deltaX - particle.sine * deltaY;
	const Real localY = particle.sine * deltaX + particle.cosine * deltaY;
	vertex.u1 = 0.5f - localX / (2.0f * particle.size);
	vertex.v1 = 0.5f - localY / (2.0f * particle.size);
	m_outcodes[index] = getUVOutcode(vertex.u1, vertex.v1);
	return index;
}

inline void W3DTerrainParticle::addTriangle(UnsignedShort a, UnsignedShort b, UnsignedShort c)
{
	// Skip the triangle when all UV's are outside (transparent).
	if ((m_outcodes[a] & m_outcodes[b] & m_outcodes[c]) != 0)
		return;

	m_indexData[m_numIndices++] = a;
	m_indexData[m_numIndices++] = b;
	m_indexData[m_numIndices++] = c;
}

void W3DTerrainParticle::flushBatch()
{
	if (m_numIndices > 0 && m_numVertices > 0)
	{
		DynamicVBAccessClass vertexAccess(BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, m_numVertices);
		{
			DynamicVBAccessClass::WriteLockClass vertexLock(&vertexAccess);
			memcpy(vertexLock.Get_Formatted_Vertex_Array(),
			       m_vertexData.data(),
			       m_numVertices * sizeof(VertexFormatXYZNDUV2));
		}

		DynamicIBAccessClass indexAccess(BUFFER_TYPE_DYNAMIC_DX8, m_numIndices);
		{
			DynamicIBAccessClass::WriteLockClass indexLock(&indexAccess);
			memcpy(indexLock.Get_Index_Array(),
			       m_indexData.data(),
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
	resetVertexLookup();
}

void W3DTerrainParticle::resetVertexLookup()
{
	std::fill(m_vertexLookup.begin(), m_vertexLookup.end(), INVALID_VERTEX);
}

IRegion2D W3DTerrainParticle::calcTerrainBounds(WorldHeightMap& map, const Vector3& loc, Real projectedRadius) const
{
	IRegion2D bounds;
	bounds.lo.x = REAL_TO_INT_FLOOR((loc.X - projectedRadius) / MAP_XY_FACTOR);
	bounds.lo.y = REAL_TO_INT_FLOOR((loc.Y - projectedRadius) / MAP_XY_FACTOR);
	bounds.hi.x = REAL_TO_INT_CEIL((loc.X + projectedRadius) / MAP_XY_FACTOR) + 1;
	bounds.hi.y = REAL_TO_INT_CEIL((loc.Y + projectedRadius) / MAP_XY_FACTOR) + 1;

	bounds.intersectWith(map.getLogicalBounds());
	bounds.intersectWith(m_terrainInViewBounds);

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
