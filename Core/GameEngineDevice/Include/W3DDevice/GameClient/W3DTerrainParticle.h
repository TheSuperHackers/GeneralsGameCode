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

#pragma once

#include <vector>

#include "Common/MapObject.h"
#include "Lib/BaseType.h"
#include "WW3D2/shader.h"
#include "WWLib/sharebuf.h"
#include "WWMath/vector3.h"
#include "WWMath/vector4.h"

constexpr const UnsignedShort MAX_VERTICES = 32768;
constexpr const UnsignedShort MAX_INDICES = 65535;
constexpr const UnsignedShort INVALID_VERTEX = MAX_VERTICES + 1;
constexpr const Real Z_OFFSET = MAP_HEIGHT_SCALE / 10;

static_assert(MAX_VERTICES < INVALID_VERTEX, "Vertex indices must leave room for the INVALID_VERTEX sentinel value.");

class AABoxClass;
class TextureClass;
class WorldHeightMap;
struct VertexFormatXYZNDUV2;

/**
 * Render particles as terrain conforming overlays. For each particle, form an initial region by calculating the bounds
 * of its rotated square and intersecting that with the map bounds and visible-terrain bounds. Recursively subdivide this
 * region until each sub-region is either a single terrain cell or is on perfectly flat terrain. Flat regions become one
 * large quad, while non-flat regions must match the terrain's topology exactly.
 */
class W3DTerrainParticle
{
public:
	W3DTerrainParticle();
	~W3DTerrainParticle();

	void setTexture(TextureClass* texture);
	void setShader(ShaderClass shader);
	void setArrays(ShareBufferClass<Vector3>* locs,
	               ShareBufferClass<Vector4>* diffuse = nullptr,
	               ShareBufferClass<Real>* sizes = nullptr,
	               ShareBufferClass<UnsignedByte>* orientations = nullptr,
	               Int activePointCount = -1);
	void setBoundingBox(const AABoxClass& worldBoundingBox);
	void render();

private:

	struct ParticleContext;

	void drawRegion(WorldHeightMap& map, const ParticleContext& particle, const IRegion2D& bounds);
	void drawQuad(WorldHeightMap& map, const ParticleContext& particle, const IRegion2D& bounds);
	UnsignedShort addVertex(WorldHeightMap& map, const ParticleContext& particle, Int x, Int y);
	void addTriangle(UnsignedShort a, UnsignedShort b, UnsignedShort c);void resetVertexLookup();
	void flushBatch();
	IRegion2D calcTerrainBounds(WorldHeightMap& map, const Vector3& loc, Real projectedRadius) const;
	void updateSettings();

	std::vector<VertexFormatXYZNDUV2> m_vertexData;    ///< Vertices of the current batch.
	std::vector<UnsignedShort> m_indexData;    ///< Indices defining the triangles of the current batch.
	std::vector<UnsignedByte> m_outcodes;    ///< UV outcodes indexed by batch vertex.
	std::vector<UnsignedShort> m_vertexLookup;    ///< Map grid location to index in m_vertexData.
	UnsignedShort m_numVertices;    ///< Number of vertices used in m_vertexData.
	UnsignedShort m_numIndices;    ///< Number of indices used in m_indexData.

	TextureClass* m_texture;
	ShaderClass m_shader;

	ShareBufferClass<Vector3>* m_pointLoc;    ///< World space point locations.
	ShareBufferClass<Vector4>* m_pointDiffuse;    ///< RGBA values (nullptr if not used).
	ShareBufferClass<Real>* m_pointSize;    ///< Size override table (nullptr if not used).
	ShareBufferClass<UnsignedByte>* m_pointOrientation;    ///< Orientation indices (nullptr if not used).
	Int m_pointCount;    ///< Total point count.
	IRegion2D m_terrainInViewBounds;    ///< Bounding box of which terrain cell indices are on the screen.

	Real m_defaultPointSize;    ///< Point size (size array overrides if present).
	Vector3 m_defaultPointColor;    ///< Point color (color array overrides if present).
	Real m_defaultPointAlpha;    ///< Point alpha (alpha array overrides if present).
	UnsignedByte m_defaultPointOrientation;    ///< Point orientation (orientation array overrides if present).
	UnsignedInt m_defaultDiffuse;    ///< Diffuse built from m_defaultPointColor and m_defaultPointAlpha.
};
