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

#include "WWMath/vector3.h"
#include "Common/GameType.h"
#include "Lib/BaseTypeCore.h"

class TextureClass;
class DX8IndexBufferClass;
class DX8VertexBufferClass;
class WorldHeightMap;

class W3DScorchInterface
{
public:
	virtual ~W3DScorchInterface() {}

	virtual void allocateBuffers() = 0;
	virtual void freeBuffers() = 0;
	virtual void clearAllScorches() = 0;
	virtual void invalidateBuffers() = 0;
	virtual void addScorch(Vector3 location, Real radius, Scorches type) = 0;
	virtual void drawScorches(WorldHeightMap& map) = 0;
};

class W3DScorch : public W3DScorchInterface
{
public:
	W3DScorch();
	virtual ~W3DScorch() override;

	virtual void allocateBuffers() override;    ///< allocate static buffers for drawing scorch marks.
	virtual void freeBuffers() override;    ///< frees up scorch buffers.
	virtual void clearAllScorches() override;
	virtual void invalidateBuffers() override;
	virtual void addScorch(Vector3 location, Real radius, Scorches type) override;
	virtual void drawScorches(WorldHeightMap& map) override;    ///< Draws the scorch mark polygons in m_vertexScorch.

private:
	typedef struct
	{
		Vector3 location;
		Real radius;
		Int scorchType;
	} TScorch;

	enum
	{
		MAX_SCORCH_VERTEX = 8194,
		MAX_SCORCH_INDEX = 6 * 8194,
		MAX_SCORCH_MARKS = 500,
		SCORCH_MARKS_IN_TEXTURE = 9,
		SCORCH_PER_ROW = 3
	};

	void updateScorches(WorldHeightMap& map);    ///< Update m_vertexScorch and m_indexScorch so all scorches will be drawn.

	DX8VertexBufferClass* m_vertexScorch;    ///< Scorch vertex buffer.
	DX8IndexBufferClass* m_indexScorch;    ///< indices defining a triangles for the scorch drawing.
	TextureClass* m_scorchTexture;    ///< Scorch mark texture
	Int m_curNumScorchVertices;    ///< number of vertices used in m_vertexScorch.
	Int m_curNumScorchIndices;    ///< number of indices used in m_indexScorch.
	TScorch m_scorches[MAX_SCORCH_MARKS];
	Int m_numScorches;
	Int m_scorchesInBuffer;    ///< how many are in the buffers.  If less than numScorches, we need to update
};

class W3DScorchDummy : public W3DScorchInterface
{
public:
	virtual void allocateBuffers() override {}
	virtual void freeBuffers() override {}
	virtual void clearAllScorches() override {}
	virtual void invalidateBuffers() override {}
	virtual void addScorch(Vector3, Real, Scorches) override {}
	virtual void drawScorches(WorldHeightMap&) override {}
};
