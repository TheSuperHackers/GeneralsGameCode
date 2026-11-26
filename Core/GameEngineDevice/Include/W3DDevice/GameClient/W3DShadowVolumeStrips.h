/*
**	Command & Conquer Generals(tm)
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

#pragma once

#include <utility>

struct ShadowVolumeStripLayout
{
	int vertexCount;
	int triangleCount;
};

// Order paired silhouette endpoints in place and size the extruded strips.
// The caller provides a nonempty list of complete pairs or an empty list.
inline ShadowVolumeStripLayout PrepareShadowVolumeStrips(short *silhouette, int indexCount)
{
	if (indexCount == 0)
		return {0, 0};

	int vertexCount = 2;
	short stripStart = silhouette[0];
	for (int edge = 0; edge < indexCount; edge += 2)
	{
		const short edgeEnd = silhouette[edge + 1];
		const int nextEdge = edge + 2;
		int continuation = nextEdge;
		while (continuation < indexCount && silhouette[continuation] != edgeEnd)
			continuation += 2;

		if (continuation < indexCount)
		{
			std::swap(silhouette[nextEdge], silhouette[continuation]);
			std::swap(silhouette[nextEdge + 1], silhouette[continuation + 1]);
			vertexCount += 2;
		}
		else
		{
			if (edgeEnd != stripStart)
				vertexCount += 2;
			if (nextEdge < indexCount)
			{
				stripStart = silhouette[nextEdge];
				vertexCount += 2;
			}
		}
	}

	// Each silhouette edge produces two triangles.
	return {vertexCount, indexCount};
}
