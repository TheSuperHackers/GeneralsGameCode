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

#include "W3DDevice/GameClient/W3DPoly.h"
#include "Lib/BaseType.h"

//-------------------------------------------------------------------------------------------------
/** Delete all vertices in polygon */
//-------------------------------------------------------------------------------------------------
void ClipPolyClass::Reset(void)
{
	Verts.Delete_All(false);
}

//-------------------------------------------------------------------------------------------------
/** Add a new vertex to polygon */
//-------------------------------------------------------------------------------------------------
void ClipPolyClass::Add_Vertex(const Vector3 & point)
{
	Verts.Add(point);
}

//-------------------------------------------------------------------------------------------------
/** Clip polygon to given plane, returning new polygon in dest. */
//-------------------------------------------------------------------------------------------------
void ClipPolyClass::Clip(const PlaneClass & plane,ClipPolyClass & dest) const
{
	dest.Reset();

	// temporary variables used in clipping
	Int i = 0;
	Int vcount = Verts.Count();
	Int iprev = vcount - 1;
	Bool cur_point_in_front;
	Bool prev_point_in_front;
	Real alpha;
	Vector3 int_point;

	if (vcount <= 2) return;

	// perform clipping
	prev_point_in_front = !plane.In_Front(Verts[iprev]);		// Note, plane normal is outward so we invert this test
	for (Int j=0; j<vcount; j++) {

		cur_point_in_front = !plane.In_Front(Verts[i]);			// Note, plane nomral is out so we invert this test
		if (prev_point_in_front) {

			if (cur_point_in_front) {

				// Previous vertex was in front of plane and this vertex is in
				// front of the plane so we emit this vertex.
				dest.Add_Vertex(Verts[i]);

			} else {

				// Previous vert was in front, this vert is behind, compute
				// the intersection and emit the point.
				plane.Compute_Intersection(Verts[iprev],Verts[i],&alpha);
				Vector3::Lerp(Verts[iprev],Verts[i],alpha,&int_point);
				dest.Add_Vertex(int_point);

			}

		} else {

			if (cur_point_in_front) {

				// segment is going from the back halfspace to the front halfspace
				// compute the intersection and emit it, then continue
				// the edge into the front halfspace and emit the end point.
				plane.Compute_Intersection(Verts[iprev],Verts[i],&alpha);
				Vector3::Lerp(Verts[iprev],Verts[i],alpha,&int_point);
				dest.Add_Vertex(int_point);
				dest.Add_Vertex(Verts[i]);

			}
		}

		prev_point_in_front = cur_point_in_front;
		iprev = i;

		//i = (i+1)%(Verts.Count());
		i++;
		if (i>=vcount) {
			i = 0;
		}
	}
}
