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

#include "Lib/BaseType.h"
#include "WW3D2/shader.h"
#include "WWLib/sharebuf.h"
#include "WWMath/vector3.h"
#include "WWMath/vector4.h"

class AABoxClass;
class TextureClass;
class WorldHeightMap;
struct VertexFormatXYZNDUV2;

// Renders particles as meshes that follow the shape of the terrain below them.
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

	void processParticle(WorldHeightMap& map, const Vector3& loc, UnsignedInt diffuse, Real size, UnsignedByte orientation);
	void drawQuad(const Vector3& loc, UnsignedInt diffuse, Real size, Real cosine, Real sine, Real height, const Vector3& normal);
	void drawTerrainConformingMesh(WorldHeightMap& map, const Vector3& loc, const IRegion2D& bounds, UnsignedInt diffuse, Real size, Real cosine, Real sine);
	void addTriangle(UnsignedShort baseVertex, UnsignedShort offsetA, UnsignedShort offsetB, UnsignedShort offsetC);
	void flushBatch();
	IRegion2D calcBounds(WorldHeightMap& map, const Vector3& loc, Real projectedRadius) const;
	void updateSettings();

	VertexFormatXYZNDUV2* m_vertexData;    ///< Vertices of the current batch.
	UnsignedShort* m_indexData;    ///< Indices defining the triangles of the current batch.
	UnsignedByte* m_outcodes;    ///< UV outcodes to keep track which triangles are fully transparent.
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
