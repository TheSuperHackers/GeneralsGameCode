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

/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : MeshDeformSet.cpp                                            *
 *                                                                                             *
 *                   Programmer : Patrick Smith                                                *
 *                                                                                             *
 *                   Start Date : 04/26/99                                                     *
 *                                                                                             *
 *                  Last Update :
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "MeshDeformSet.h"
#include "util.h"
#include "MeshDeformSaveSet.h"
#include "meshbuild.h"
#include "MeshDeformSaveDefs.h"
#include "MeshDeformDefs.h"


///////////////////////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////////////////////
const int MAX_DEFORM_KEY_FRAMES			= 10;


///////////////////////////////////////////////////////////////////////////
//
//	~MeshDeformSetClass
//
///////////////////////////////////////////////////////////////////////////
MeshDeformSetClass::~MeshDeformSetClass (void)
{
	SAFE_DELETE (m_pMesh);
	SAFE_DELETE (m_pVertexArray);
	SAFE_DELETE (m_pVertexOPStartArray);
	SAFE_DELETE (m_pVertexColors);
	Free_Key_Frames ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Current_Key_Frame
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Set_Current_Key_Frame (int index)
{
	if (index >= -1 && index < MAX_DEFORM_KEY_FRAMES) {
		m_CurrentKeyFrame = index;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Vertex_Position
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Set_Vertex_Position
(
	int				index,
	const Point3 &	value
)
{
	DEFORM_LIST &vertices		= m_KeyFrames[m_CurrentKeyFrame]->vertices;
	BitArray &affected_verts	= m_KeyFrames[m_CurrentKeyFrame]->affected_verts;

	//
	//	Set the vert's position
	//
	m_pMesh->verts[index] = value;
	vertices.Add (VERT_INFO (index, value));

	//
	//	Make sure we remember that this vert is affected
	//
	affected_verts.Set (index, 1);
	m_SetMembers.Set (index, 1);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Vertex_Color
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Set_Vertex_Color
(
	int					index,
	int					color_index,
	const VertColor &	value
)
{
	DEFORM_LIST &colors			= m_KeyFrames[m_CurrentKeyFrame]->colors;
	BitArray &affected_colors	= m_KeyFrames[m_CurrentKeyFrame]->affected_colors;

	//
	//	Set the vert's color
	//
	m_pMesh->vertCol[color_index] = value;
	colors.Add (VERT_INFO (index, value, color_index));

	//
	//	Make sure we remember that this vert color is affected
	//
	affected_colors.Set (index, 1);
	m_SetMembers.Set (index, 1);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Set_Members
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Update_Set_Members (void)
{
	//
	//	Examine each keyframe
	//
	m_SetMembers.ClearAll ();
	for (int index = 0; index < m_KeyFrames.Count (); index ++) {
		BitArray &affected_verts	= m_KeyFrames[index]->affected_verts;
		BitArray &affected_colors	= m_KeyFrames[index]->affected_colors;

		//
		//	Mark the verts that are affected by this keyframe
		//
		for (int vert = 0; vert < m_VertexCount; vert ++) {
			if (affected_verts[vert] || affected_colors[vert]) {
				m_SetMembers.Set (vert, 1);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Collapse_Keyframe_Data
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Collapse_Keyframe_Data (int keyframe)
{
	DEFORM_LIST &vertices		= m_KeyFrames[keyframe]->vertices;
	DEFORM_LIST &colors			= m_KeyFrames[keyframe]->colors;
	BitArray &affected_verts	= m_KeyFrames[keyframe]->affected_verts;
	BitArray &affected_colors	= m_KeyFrames[keyframe]->affected_colors;

	//
	//	Collapse the vertex position data
	//
	for (int index = 0; index < vertices.Count (); index ++) {
		VERT_INFO &info = vertices[index];

		//
		//	If this vertex is unchanged, then remove it
		// from the list.
		//
		if (m_pVertexArray[index] == info.value) {
			vertices.Delete (index);
			index --;
		} else {
			affected_verts.Set (info.index, 1);
			m_SetMembers.Set (info.index, 1);
		}
	}


	//
	//	Collapse the vertex color data
	//
	for (index = 0; index < colors.Count (); index ++) {
		VERT_INFO &info = colors[index];

		//
		//	If this vertex is unchanged, then remove it
		// from the list.
		//
		if (m_pVertexColors[index] == info.value) {
			vertices.Delete (index);
			index --;
		} else {
			affected_colors.Set (info.index, 1);
			m_SetMembers.Set (info.index, 1);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Reset_Key_Frame_Verts
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Reset_Key_Frame_Verts (int keyframe)
{
	DEFORM_LIST &vertices		= m_KeyFrames[keyframe]->vertices;
	BitArray &affected_verts	= m_KeyFrames[keyframe]->affected_verts;

	//
	//	Reset all data for this keyframe
	//
	affected_verts.ClearAll ();
	vertices.Delete_All ();

	//
	//	Regenerate the list of set members
	//
	Update_Set_Members ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Reset_Key_Frame_Colors
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Reset_Key_Frame_Colors (int keyframe)
{
	DEFORM_LIST &colors			= m_KeyFrames[keyframe]->colors;
	BitArray &affected_colors	= m_KeyFrames[keyframe]->affected_colors;

	//
	//	Reset all data for this keyframe
	//
	affected_colors.ClearAll ();
	colors.Delete_All ();

	//
	//	Regenerate the list of set members
	//
	Update_Set_Members ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Current_Data
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Update_Current_Data (void)
{
	DEFORM_LIST &vertices		= m_KeyFrames[m_CurrentKeyFrame]->vertices;
	DEFORM_LIST &colors			= m_KeyFrames[m_CurrentKeyFrame]->colors;
	BitArray &affected_verts	= m_KeyFrames[m_CurrentKeyFrame]->affected_verts;
	BitArray &affected_colors	= m_KeyFrames[m_CurrentKeyFrame]->affected_colors;

	//
	//	Assume we have no modifications for this keyframe
	//
	affected_verts.ClearAll ();
	affected_colors.ClearAll ();
	vertices.Delete_All ();
	colors.Delete_All ();

	//
	//	Record the vertex position data
	//
	for (int index = 0; index < m_VertexCount; index ++) {

		// Is this vertex's position different than the undeformed mesh?
		Point3 orig		= m_pVertexArray[index];
		Point3 current	= m_pMesh->verts[index];

		//Apply_Position_Changes (index, m_CurrentKeyFrame, orig);

		if (	(orig.x != current.x) ||
				(orig.y != current.y) ||
				(orig.z != current.z)) {

			//
			//	Record this vertex's position in our lists
			//
			affected_verts.Set (index, 1);
			vertices.Add (VERT_INFO (index, m_pMesh->verts[index]));
		}
	}

	// Only do this if the mesh is using vertex coloring
	if (m_pMesh->numCVerts >= m_pMesh->numVerts) {

		//
		//	Record the vertex color data
		//
		for (int face = 0; face < m_pMesh->numFaces; face ++) {
			for (int vert = 0; vert < 3; vert ++) {

				//
				//	Has this vertex color changed?
				//
				int vertex_index	= m_pMesh->faces[face].v[vert];
				int color_index	= m_pMesh->vcFace[face].t[vert];
				VertColor orig		= m_pVertexColors[color_index];
				VertColor current	= m_pMesh->vertCol[color_index];
				if (	(orig.x != current.x) ||
						(orig.y != current.y) ||
						(orig.z != current.z)) {
					affected_colors.Set (vertex_index, 1);
					colors.Add (VERT_INFO (vertex_index, m_pMesh->vertCol[color_index], color_index));
				}
			}
		}
	}

	//
	//	Rebuild the list of vertices this 'set' affects
	//
	Update_Set_Members ();

	//
	//	Collapse all unsused data from the remainder of the keyframes
	//
	for (index = 0; index < m_KeyFrames.Count (); index ++) {
		//Collapse_Keyframe_Data (index);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Key_Frame
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Update_Key_Frame (int key_frame)
{
	DEFORM_LIST &vertices		= m_KeyFrames[key_frame]->vertices;
	DEFORM_LIST &colors			= m_KeyFrames[key_frame]->colors;
	BitArray &affected_verts	= m_KeyFrames[key_frame]->affected_verts;
	BitArray &affected_colors	= m_KeyFrames[key_frame]->affected_colors;

	if ((key_frame == m_CurrentKeyFrame) ||
		 (vertices.Count () > 0) ||
		 (colors.Count () > 0)) {

		// Clear all entries from this keyframe
		vertices.Delete_All ();
		colors.Delete_All ();

		//
		//	Copy the vertex position changes
		//
		for (int vert = 0; vert < m_pMesh->numVerts; vert ++) {
			if (affected_verts[vert]) {
				vertices.Add (VERT_INFO (vert, m_pMesh->verts[vert]));
			}
		}

		//
		//	Copy the vertex color changes
		//

		// Only do this if the mesh is using vertex coloring
		if (m_pMesh->numCVerts >= m_pMesh->numVerts) {
			for (int face = 0; face < m_pMesh->numFaces; face ++) {

				if (affected_colors[m_pMesh->faces[face].v[0]]) {
					int color_index = m_pMesh->vcFace[face].t[0];
					colors.Add (VERT_INFO (m_pMesh->faces[face].v[0], m_pMesh->vertCol[color_index], color_index));
				}

				if (affected_colors[m_pMesh->faces[face].v[1]]) {
					int color_index = m_pMesh->vcFace[face].t[1];
					colors.Add (VERT_INFO (m_pMesh->faces[face].v[1], m_pMesh->vertCol[color_index], color_index));
				}

				if (affected_colors[m_pMesh->faces[face].v[2]]) {
					int color_index = m_pMesh->vcFace[face].t[2];
					colors.Add (VERT_INFO (m_pMesh->faces[face].v[2], m_pMesh->vertCol[color_index], color_index));
				}
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Init_Key_Frames
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Init_Key_Frames (void)
{
	//
	// For now, add all the key frames upfront
	//
	for (int index = 0; index < MAX_DEFORM_KEY_FRAMES; index ++) {
		KEY_FRAME *key_frame = new KEY_FRAME;
		m_KeyFrames.Add (key_frame);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Free_Key_Frames
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Free_Key_Frames (void)
{
	//
	// Loop through and free all the key frames
	//
	for (int index = 0; index < m_KeyFrames.Count (); index ++) {
		KEY_FRAME *key_frame = m_KeyFrames[index];
		SAFE_DELETE (key_frame);
	}

	m_KeyFrames.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Copy_Vertex_Array
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Copy_Vertex_Array (Mesh &mesh)
{
	Resize_Vertex_Array (mesh.numVerts, mesh.numCVerts);

	//
	// Copy the vertex positions from the mesh
	//
	for (int vert = 0; vert < mesh.numVerts; vert ++) {
		m_pVertexArray[vert] = mesh.verts[vert];
	}

	//
	// Copy the vertex colors from the mesh
	//
	for (int index = 0; index < mesh.numCVerts; index ++) {
		m_pVertexColors[index] = mesh.vertCol[index];
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Resize_Vertex_Array
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Resize_Vertex_Array (int count, int color_count)
{
	if (count != m_VertexCount) {

		// Allocate a new array of vertices
		Point3 *vertex_array = new Point3[count];
		Point3 *opstart_array = new Point3[count];

		// Delete the old vertex array and remember the new one
		SAFE_DELETE (m_pVertexArray);
		SAFE_DELETE (m_pVertexOPStartArray);
		m_pVertexArray = vertex_array;
		m_pVertexOPStartArray = opstart_array;
		m_VertexCount = count;

		//
		// Reset the bounds of the 'affected verts' per keyframe arrays
		//
		for (int index = 0; index < m_KeyFrames.Count (); index ++) {
			m_KeyFrames[index]->affected_verts.SetSize (count);
			m_KeyFrames[index]->affected_colors.SetSize (count);
			m_KeyFrames[index]->affected_verts.ClearAll ();
			m_KeyFrames[index]->affected_colors.ClearAll ();

		}

		m_SetMembers.SetSize (count);
		m_SetMembers.ClearAll ();
	}

	if (color_count != m_VertexColorCount) {

		// Recreate the color deltas
		Point3 *color_array = new VertColor[color_count];
		for (int index = 0; index < color_count; index ++) {
			color_array[index].x = 0;
			color_array[index].y = 0;
			color_array[index].z = 0;
		}

		// Delete the old delta array and remeber the new one
		SAFE_DELETE (m_pVertexColors);
		m_VertexColorCount = color_count;
		m_pVertexColors = color_array;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_State
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Set_State (float state)
{
	m_State = state;
	int key_frame = (m_State * MAX_DEFORM_KEY_FRAMES) + 0.5F;
	Set_Current_Key_Frame ((key_frame >= 0) ? (key_frame - 1) : -1);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Determine_Interpolation_Indicies
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Determine_Interpolation_Indicies
(
	int		key_frame,
	bool		position,
	int &		from,
	int &		to,
	float &	state
)
{
	/*state = m_State;
	from = -1;
	to = key_frame;

	//
	//	Determine where we should start interpolation
	//
	for (int index = 0; index <= key_frame; index ++) {
		if (position && m_KeyFrames[index]->vertices.Count () > 0) {
			from = index;
		} else if (!position && m_KeyFrames[index]->colors.Count () > 0) {
			from = index;
		}
	}

	//
	//	Determine where we should end interpolation
	//
	for (index = to; index < MAX_DEFORM_KEY_FRAMES; index ++) {
		if (position && m_KeyFrames[index]->vertices.Count () > 0) {
			to = index;
			break;
		} else if (!position && m_KeyFrames[index]->colors.Count () > 0) {
			to = index;
			break;
		}
	}

	//
	//	Determine the state (deformation percent)
	//
	state = 0;
	if (m_State > 0) {
		state = 1.0F;
		if ((to != from) && (m_CurrentKeyFrame < to)) {
			float value = m_CurrentKeyFrame;//key_frame;
			state = ((value - ((float)from)) / (float)(to-from));
		}
	}*/

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Position_Changes
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Apply_Position_Changes
(
	UINT		vert,
	int		frame_to_check,
	Point3 &	position,
	Matrix3 *transform
)
{
	//
	// Determine where we should start interpolating this vert
	//
	int from = -1;
	for (int key_frame = frame_to_check; (key_frame >= 0) && (from == -1); key_frame --) {
		if (m_KeyFrames[key_frame]->affected_verts[vert]) {
			from = key_frame;
		}
	}

	//
	// Determine where we should end interpolating this vert
	//
	int to = -1;
	if (frame_to_check >= 0) {
		for (key_frame = frame_to_check; (key_frame < m_KeyFrames.Count ()) && (to == -1); key_frame ++) {
			if (m_KeyFrames[key_frame]->affected_verts[vert]) {
				to = key_frame;
			}
		}
	}

	//
	//	Determine the deformation percent
	//
	float state = 0;
	if (m_State > 0) {
		state = 1.0F;
		if ((to != from) && (frame_to_check < to)) {
			float value = frame_to_check;
			state = ((value - ((float)from)) / (float)(to - from));
		}
	}

	if (from != -1) {

		//
		//	Find the vertex value in the 'from' key frame and set the
		// triangle object's vertex to be this value (we will interplate from it).
		//
		DEFORM_LIST &vert_from = m_KeyFrames[from]->vertices;
		for (int index = 0; index < vert_from.Count (); index ++) {
			VERT_INFO &info = vert_from[index];
			if (info.index == vert) {
				Point3 new_pos = info.value;

				// Transform the new position if necessary
				if (transform != nullptr) {
					new_pos = new_pos * (*transform);
				}

				position = new_pos;
			}
		}
	}

	if (to != -1) {

		//
		//	Find the vertex value in the 'to' key frame and interpolate
		// this value from the triangle object's current vertex value.
		//
		DEFORM_LIST &vert_to = m_KeyFrames[to]->vertices;
		for (int index = 0; index < vert_to.Count (); index ++) {
			VERT_INFO &info = vert_to[index];
			if (info.index == vert) {

				Point3 new_pos = info.value;

				// Transform the new position if necessary
				if (transform != nullptr) {
					new_pos = new_pos * (*transform);
				}

				position += state * (new_pos - position);
				//m_pMesh->verts[vert] = position;
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Color_Changes
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Apply_Color_Changes
(
	UINT			vert,
	int			frame_to_check,
	Mesh &		mesh
	//VertColor &	color
)
{
	//
	// Determine where we should start interpolating this vert
	//
	int from = -1;
	for (int key_frame = frame_to_check; (key_frame >= 0) && (from == -1); key_frame --) {
		if (m_KeyFrames[key_frame]->affected_colors[vert]) {
			from = key_frame;
		}
	}

	//
	// Determine where we should end interpolating this vert
	//
	int to = -1;
	if (frame_to_check >= 0) {
		for (key_frame = frame_to_check; (key_frame < m_KeyFrames.Count ()) && (to == -1); key_frame ++) {
			if (m_KeyFrames[key_frame]->affected_colors[vert]) {
				to = key_frame;
			}
		}
	}

	//
	//	Determine the deformation percent
	//
	float state = 0;
	if (m_State > 0) {
		state = 1.0F;
		if ((to != from) && (frame_to_check < to)) {
			float value = frame_to_check;
			state = ((value - ((float)from)) / (float)(to - from));
		}
	}

	if (from != -1) {

		//
		//	Find the color value in the 'from' key frame and set the
		// triangle object's color to be this value (we will interplate from it).
		//
		DEFORM_LIST &color_from = m_KeyFrames[from]->colors;
		for (int index = 0; index < color_from.Count (); index ++) {
			VERT_INFO &info = color_from[index];
			if (info.index == vert) {
				//color = info.value;
				mesh.vertCol[info.color_index] = info.value;
				m_pMesh->vertCol[info.color_index] = info.value;
			}
		}
	}

	if (to != -1) {

		//
		//	Find the color value in the 'to' key frame and interpolate
		// this value from the triangle object's current color value.
		//
		DEFORM_LIST &color_to = m_KeyFrames[to]->colors;
		for (int index = 0; index < color_to.Count (); index ++) {
			VERT_INFO &info = color_to[index];
			if (info.index == vert) {
				mesh.vertCol[info.color_index] = m_pMesh->vertCol[info.color_index] + state * (info.value - m_pMesh->vertCol[info.color_index]);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Color_Changes
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Apply_Color_Changes
(
	UINT			vert_index,
	UINT			vert_color_index,
	int			frame_to_check,
	VertColor &	color
)
{
	//
	// Determine where we should start interpolating this vert
	//
	int from = -1;
	for (int key_frame = frame_to_check; (key_frame >= 0) && (from == -1); key_frame --) {
		if (m_KeyFrames[key_frame]->affected_colors[vert_index]) {
			from = key_frame;
		}
	}

	//
	// Determine where we should end interpolating this vert
	//
	int to = -1;
	for (key_frame = frame_to_check; (key_frame < m_KeyFrames.Count ()) && (to == -1); key_frame ++) {
		if (m_KeyFrames[key_frame]->affected_colors[vert_index]) {
			to = key_frame;
		}
	}

	//
	//	Determine the deformation percent
	//
	float state = 0;
	if (m_State > 0) {
		state = 1.0F;
		if ((to != from) && (frame_to_check < to)) {
			float value = frame_to_check;
			state = ((value - ((float)from)) / (float)(to - from));
		}
	}

	if (from != -1) {

		//
		//	Find the color value in the 'from' key frame and set the
		// triangle object's color to be this value (we will interplate from it).
		//
		DEFORM_LIST &color_from = m_KeyFrames[from]->colors;
		for (int index = 0; index < color_from.Count (); index ++) {
			VERT_INFO &info = color_from[index];
			if (info.color_index == vert_color_index) {
				color = info.value;
				break;
			}
		}
	}

	if (to != -1) {

		//
		//	Find the color value in the 'to' key frame and interpolate
		// this value from the triangle object's current color value.
		//
		DEFORM_LIST &color_to = m_KeyFrames[to]->colors;
		for (int index = 0; index < color_to.Count (); index ++) {
			VERT_INFO &info = color_to[index];
			if (info.color_index == vert_color_index) {
				color += state * (info.value - color);
				break;
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Mesh
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Update_Mesh (TriObject &tri_obj)
{
	Copy_Vertex_Array (tri_obj.mesh);

	// Should we update the mesh or copy it?
	if (m_pMesh != nullptr) {

		//
		//	Copy the vertex colors from the triangle object
		//
		for (int vert_color = 0; vert_color < m_pMesh->numCVerts; vert_color ++) {
			m_pMesh->vertCol[vert_color] = tri_obj.mesh.vertCol[vert_color];
		}

		//
		//	Loop through all the vertices and interpolate their
		// positions and colors based on the current 'deformation state'.
		//
		for (UINT vert = 0; vert < (UINT)m_pMesh->numVerts; vert ++) {

			// Is this vertex affected by any keyframe in the set?
			if (m_SetMembers[vert]) {

				// Interpolate any changes to this vert
				Apply_Position_Changes (vert, m_CurrentKeyFrame, tri_obj.mesh.verts[vert]);
				m_pMesh->verts[vert] = tri_obj.mesh.verts[vert];
				Apply_Color_Changes (vert, m_CurrentKeyFrame, tri_obj.mesh);
			}
		}

		//
		//	Copy the vertex colors from the triangle object
		//
		for (vert_color = 0; vert_color < m_pMesh->numCVerts; vert_color ++) {
			m_pMesh->vertCol[vert_color] = tri_obj.mesh.vertCol[vert_color];
		}


		/*for (int index = 0; index < m_pMesh->numCVerts; index ++) {
			m_pMesh->vertCol[index] = tri_obj.mesh.vertCol[index];
		}

		//
		for (int key_frame = 0; key_frame < m_KeyFrames.Count (); key_frame ++) {

			//
			//	Update the vertices
			//

			int from = 0;
			int to = 0;
			float state = 0;
			Determine_Interpolation_Indicies (key_frame, true, from, to, state);
			DEFORM_LIST &vert_to = m_KeyFrames[to]->vertices;

			if (from <= m_CurrentKeyFrame) {

				if (from >= 0) {
					DEFORM_LIST &vert_from = m_KeyFrames[from]->vertices;
					for (int index = 0; index < vert_from.Count (); index ++) {
						VERT_INFO &info = vert_from[index];
						tri_obj.mesh.verts[info.index] = info.value;
					}
				} else {

					for (int index = 0; index < vert_to.Count (); index ++) {
						VERT_INFO &info = vert_to[index];
						tri_obj.mesh.verts[info.index] = m_pVertexArray[info.index];
					}
				}

				for (int index = 0; index < vert_to.Count (); index ++) {
					VERT_INFO &info = vert_to[index];
					tri_obj.mesh.verts[info.index] += state * (info.value - tri_obj.mesh.verts[info.index]);
					m_pMesh->verts[info.index] = tri_obj.mesh.verts[info.index];
				}
			}

			//
			//	Update the vertex colors
			//
			Determine_Interpolation_Indicies (key_frame, false, from, to, state);
			DEFORM_LIST &color_to = m_KeyFrames[to]->colors;

			if (from <= m_CurrentKeyFrame) {

				if (from >= 0) {
					DEFORM_LIST &color_from = m_KeyFrames[from]->colors;
					for (int index = 0; index < color_from.Count (); index ++) {
						VERT_INFO &info = color_from[index];
						tri_obj.mesh.vertCol[info.index] = info.value;
					}
				} else {

					for (index = 0; index < color_to.Count (); index ++) {
						VERT_INFO &info = color_to[index];
						tri_obj.mesh.vertCol[info.index] = m_pMesh->vertCol[info.index];
					}
				}

				for (index = 0; index < color_to.Count (); index ++) {
					VERT_INFO &info = color_to[index];
					tri_obj.mesh.vertCol[info.index] = m_pMesh->vertCol[info.index] + state * (info.value - m_pMesh->vertCol[info.index]);
					//tri_obj.mesh.vertCol[info.index] += state * (info.value - tri_obj.mesh.vertCol[info.index]);
				}
			}
		}

		for (index = 0; index < m_pMesh->numCVerts; index ++) {
			m_pMesh->vertCol[index] = tri_obj.mesh.vertCol[index];
		}*/





		/*for (int vert = 0; vert < m_VertexCount; vert ++) {
			tri_obj.mesh.verts[vert] += (1.0F * m_pVertexDeltaArray[vert]);
			m_pMesh->verts[vert] = tri_obj.mesh.verts[vert];
		}


		//
		// Transform the vertex colors based on the current deform state
		//
		for (vert = 0; vert < m_VertexColorCount; vert ++) {
			tri_obj.mesh.vertCol[vert] += (1.0F * m_pVertexColors[vert]);
		}*/

		//
		// Pass the new selection onto the mesh
		//
		tri_obj.mesh.vertSel = m_pMesh->vertSel;
		tri_obj.PointsWereChanged ();

	} else {

		//
		// Make a copy of the mesh as it currently exists
		//
		m_pMesh = new Mesh (tri_obj.mesh);
		tri_obj.mesh.DeepCopy (m_pMesh, GEOM_CHANNEL | SELECT_CHANNEL | SUBSEL_TYPE_CHANNEL | VERTCOLOR_CHANNEL);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Members
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Update_Members (DEFORM_CHANNELS flags)
{
	assert (m_CurrentKeyFrame >= 0);

	//
	// Add all the selected verts to the set array
	//
	for (int vert = 0; vert < m_pMesh->numVerts; vert ++) {
		if (m_pMesh->vertSel[vert]) {

			//
			//	Should we add this vertex to the array of deformed verts
			// the current keyframe?
			//
			if (flags & VERT_POSITION) {
				m_KeyFrames[m_CurrentKeyFrame]->affected_verts.Set (vert, 1);
			}

			//
			//	Should we add this vertex to the array of deformed vertex colors
			// the current keyframe?
			//
			if (flags & VERT_COLORS) {

				m_KeyFrames[m_CurrentKeyFrame]->affected_colors.Set (vert, 1);

				//
				// Map indicies in the vertex color array to the vertex array.
				//
				/*if (m_pMesh->numCVerts >= m_pMesh->numVerts) {
					for (int face = 0; face < m_pMesh->numFaces; face ++) {

						if (m_pMesh->faces[face].v[0] == vert) {
							int color_index = m_pMesh->vcFace[face].t[0];
							m_KeyFrames[m_CurrentKeyFrame]->affected_colors.Set (color_index, 1);
						}

						if (m_pMesh->faces[face].v[1] == vert) {
							int color_index = m_pMesh->vcFace[face].t[1];
							m_KeyFrames[m_CurrentKeyFrame]->affected_colors.Set (color_index, 1);
						}

						if (m_pMesh->faces[face].v[2] == vert) {
							int color_index = m_pMesh->vcFace[face].t[2];
							m_KeyFrames[m_CurrentKeyFrame]->affected_colors.Set (color_index, 1);
						}
					}
				}*/
			}

			//
			//	Finally, add this vertex to the list of all
			//	vertices affected by this set.
			//
			m_SetMembers.Set (vert, 1);
		}
	}

	//r (int index = m_CurrentKeyFrame; index < MAX_DEFORM_KEY_FRAMES; index ++) {
		Update_Key_Frame (m_CurrentKeyFrame);
	//

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Select_Members
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Select_Members (void)
{
	//
	// Loop through and select the necessary verts
	//
	for (int vert = 0; vert < m_pMesh->numVerts; vert ++) {
		m_pMesh->vertSel.Set (vert, m_SetMembers[vert]);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Restore_Members
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Restore_Members (void)
{
/*
	//
	// Loop through and select the necessary verts
	//
	for (int vert = 0; vert < m_pMesh->numVerts; vert ++) {

		if (m_SetMembers[vert]) {

			//
			//	Restore the vertex positions
			//
			m_pMesh->verts[vert] = m_pVertexArray[vert];
			m_pVertexDeltaArray[vert].x = 0;
			m_pVertexDeltaArray[vert].y = 0;
			m_pVertexDeltaArray[vert].z = 0;
		}
	}

	//
	//	Restore the vertex colors
	//

	// Only do this if the mesh is using vertex coloring
	if (m_pMesh->numCVerts >= m_pMesh->numVerts) {
		for (int face = 0; face < m_pMesh->numFaces; face ++) {

			if (m_SetMembers[m_pMesh->faces[face].v[0]]) {
				int color_index = m_pMesh->vcFace[face].t[0];
				m_pVertexColors[color_index].x = 0;
				m_pVertexColors[color_index].y = 0;
				m_pVertexColors[color_index].z = 0;
			}

			if (m_SetMembers[m_pMesh->faces[face].v[1]]) {
				int color_index = m_pMesh->vcFace[face].t[1];
				m_pVertexColors[color_index].x = 0;
				m_pVertexColors[color_index].y = 0;
				m_pVertexColors[color_index].z = 0;
			}

			if (m_SetMembers[m_pMesh->faces[face].v[2]]) {
				int color_index = m_pMesh->vcFace[face].t[2];
				m_pVertexColors[color_index].x = 0;
				m_pVertexColors[color_index].y = 0;
				m_pVertexColors[color_index].z = 0;
			}
		}
	}

*/
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Is_Empty
//
///////////////////////////////////////////////////////////////////////////
bool
MeshDeformSetClass::Is_Empty (void) const
{
	bool is_empty = true;

	//
	//	If any vertex is 'selected' then this set is not empty.
	//
	for (int vert = 0; (vert < m_VertexCount) && is_empty; vert ++) {
		is_empty = (m_SetMembers[vert] == 0);
	}

	// Return the true/false result code
	return is_empty;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSetClass::Save
(
	MeshBuilderClass &			builder,
	Mesh &							mesh,
	MeshDeformSaveSetClass &	save_set,
	Matrix3 *						transform
)
{
	// Start fresh
	save_set.Reset ();
	save_set.Set_Flag (DEFORM_SET_MANUAL_DEFORM, (m_bAutoApply == false));

	//
	//	Determine how much each state change (from 0 - 1)
	// is associated with each keyframe.
	//
	int key_frames = m_KeyFrames.Count ();
	float state_inc = 1.0F / ((float)key_frames);
	float old_state = m_State;

	//
	//	Loop through all the keyframes
	//
	for (int key_frame = 0; key_frame < key_frames; key_frame ++) {

		//
		//	Loop through all the vertices and see if this keyframe
		//	modifies any of them
		//
		bool verts_affected = false;
		for (int vert = 0; (vert < m_VertexCount) && !verts_affected; vert ++) {
			verts_affected = (m_KeyFrames[key_frame]->affected_verts[vert] == 1);
			verts_affected |= (m_KeyFrames[key_frame]->affected_colors[vert] == 1);
		}

		//
		//	If the keyframe modifies any of these verts, then save
		// the state of all verts in the set.
		//
		if (verts_affected) {
			m_State = state_inc * (key_frame + 1);
			save_set.Begin_Keyframe (m_State);

			//
			//	Save the absolute state of all the verts and colors
			//	at this keyframe.
			//

			for (int w3d_vert_index = 0; w3d_vert_index < builder.Get_Vertex_Count (); w3d_vert_index ++) {

				const MeshBuilderClass::VertClass &w3d_vert = builder.Get_Vertex(w3d_vert_index);
				int max_vert_index = w3d_vert.Id;

				if (m_SetMembers[max_vert_index]) {

					//
					//	Get the absolute position of this vertex
					//
					Point3 position = mesh.verts[max_vert_index];
					Apply_Position_Changes (max_vert_index, key_frame, position, transform);

					//
					//	Get the absolute color of this vertex
					//
					VertColor color (1, 1, 1);
					if (mesh.vertCol != nullptr) {
						int vert_col_index = w3d_vert.MaxVertColIndex;
						color = mesh.vertCol[vert_col_index];
						Apply_Color_Changes (max_vert_index, vert_col_index, key_frame, color);
					}

					//
					//	Save this vertex's deformations
					//
					save_set.Add_Vert (w3d_vert_index, position, color);
				}
			}

			save_set.End_Keyframe ();
		}
	}

	m_State = old_state;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
IOResult
MeshDeformSetClass::Save (ISave *save_obj)
{
	int key_frames = m_KeyFrames.Count ();
	float state_inc = 1.0F / ((float)key_frames);

	DeformChunkSetInfo set_info = { 0 };
	set_info.KeyframeCount		= key_frames;
	set_info.flags					= 0;
	set_info.NumVerticies		= m_VertexCount;
	set_info.NumVertexColors	= m_VertexColorCount;
	if (m_bAutoApply == false) {
		set_info.flags |= DEFORM_SET_MANUAL_DEFORM;
	}
	DWORD bytes = 0L;

	//
	//	Write the set info to the chunk
	//
	save_obj->BeginChunk (DEFORM_CHUNK_SET_INFO);
	IOResult result = save_obj->Write (&set_info, sizeof (set_info), &bytes);

	//
	//	Now write a chunk for each keyframe
	//
	for (int index = 0; (index < key_frames) && (result == IO_OK); index ++) {
		KEY_FRAME &key_frame = *(m_KeyFrames[index]);

		DeformChunkKeyframeInfo keyframe_info = { 0 };
		keyframe_info.DeformPercent	= state_inc * (index + 1);
		keyframe_info.VertexCount		= key_frame.vertices.Count ();
		keyframe_info.ColorCount		= key_frame.colors.Count ();

		//
		//	Write keyframe info to the chunk
		//
		//save_obj->BeginChunk (DEFORM_CHUNK_KEYFRAME_INFO);
		result = save_obj->Write (&keyframe_info, sizeof (keyframe_info), &bytes);
		//save_obj->EndChunk ();

		//
		//	Loop through the vertices and save their position
		//
		for (	unsigned int pos_index = 0;
				(pos_index < keyframe_info.VertexCount) && (result == IO_OK);
				pos_index ++)
		{
			VERT_INFO &deform_data = key_frame.vertices[pos_index];

			DeformDataChunk data;
			data.VertexIndex	= deform_data.index;
			data.ColorIndex	= deform_data.color_index;
			data.Value			= deform_data.value;

			//
			//	Write vertex position info to the chunk
			//
			//save_obj->BeginChunk (DEFORM_CHUNK_POSITION_DATA);
			result = save_obj->Write (&data, sizeof (data), &bytes);
			//save_obj->EndChunk ();
		}

		//
		//	Loop through the vertices and save their color
		//
		for (	unsigned int color_index = 0;
				(color_index < keyframe_info.ColorCount) && (result == IO_OK);
				color_index ++)
		{
			VERT_INFO &deform_data = key_frame.colors[color_index];

			DeformDataChunk data;
			data.VertexIndex	= deform_data.index;
			data.ColorIndex	= deform_data.color_index;
			data.Value			= deform_data.value;

			//
			//	Write vertex color info to the chunk
			//
			//save_obj->BeginChunk (DEFORM_CHUNK_COLOR_DATA);
			result = save_obj->Write (&data, sizeof (data), &bytes);
			//save_obj->EndChunk ();
		}

		//
		//	Write the list of affected vertex positions to a chunk
		//
		/*if (result == IO_OK) {
			//save_obj->BeginChunk (DEFORM_CHUNK_POSITION_VERTS);
			result = key_frame.affected_verts.Save (save_obj);
			//save_obj->EndChunk ();
		}

		//
		//	Write the list of affected vertex colors to a chunk
		//
		if (result == IO_OK) {
			//save_obj->BeginChunk (DEFORM_CHUNK_COLOR_VERTS);
			result = key_frame.affected_colors.Save (save_obj);
			//save_obj->EndChunk ();
		}*/
	}

	save_obj->EndChunk ();

	// Return IO_OK on success IO_ERROR on failure
	return result;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
IOResult
MeshDeformSetClass::Load (ILoad *load_obj)
{
	// Start fresh
	Free_Key_Frames ();
	Init_Key_Frames ();
	DWORD bytes = 0L;


	//
	//	Is this the chunk we were expecting?
	//
	IOResult result = load_obj->OpenChunk ();
	if (	(result == IO_OK) &&
			(load_obj->CurChunkID () == DEFORM_CHUNK_SET_INFO)) {

		//
		//	Read information about this set from the chunk
		//
		DeformChunkSetInfo set_info = { 0 };
		result = load_obj->Read (&set_info, sizeof (set_info), &bytes);
		m_bAutoApply = !(set_info.flags & DEFORM_SET_MANUAL_DEFORM);

		//
		//	Resize the internal data to fit the saved state
		//
		if (result == IO_OK) {
			Resize_Vertex_Array (set_info.NumVerticies, set_info.NumVertexColors);
		}

		//
		//	Read keyframe information from the chunk
		//
		for (	unsigned int index = 0;
				(index < set_info.KeyframeCount) && (result == IO_OK);
				index ++)
		{
			KEY_FRAME &key_frame = *(m_KeyFrames[index]);

			DeformChunkKeyframeInfo keyframe_info = { 0 };
			result = load_obj->Read (&keyframe_info, sizeof (keyframe_info), &bytes);

			//
			//	Read all the vertex positions from the chunk
			//
			for (	unsigned int pos_index = 0;
					(pos_index < keyframe_info.VertexCount) && (result == IO_OK);
					pos_index ++)
			{
				//
				//	Read vertex position info from the chunk
				//
				DeformDataChunk data;
				result = load_obj->Read (&data, sizeof (data), &bytes);
				if (result == IO_OK) {
					key_frame.vertices.Add (VERT_INFO (data.VertexIndex, data.Value, data.ColorIndex));
					key_frame.affected_verts.Set (data.VertexIndex, 1);
					m_SetMembers.Set (data.VertexIndex, 1);
				}
			}

			//
			//	Read all the vertex colors from the chunk
			//
			for (	unsigned int color_index = 0;
					(color_index < keyframe_info.ColorCount) && (result == IO_OK);
					color_index ++)
			{
				//
				//	Read vertex color info from the chunk
				//
				DeformDataChunk data;
				result = load_obj->Read (&data, sizeof (data), &bytes);
				if (result == IO_OK) {
					key_frame.colors.Add (VERT_INFO (data.VertexIndex, data.Value, data.ColorIndex));
					key_frame.affected_colors.Set (data.VertexIndex, 1);
					m_SetMembers.Set (data.VertexIndex, 1);
				}
			}

			//
			//	Read the list of affected vertex positions to a chunk
			//
			/*if (result == IO_OK) {
				result = key_frame.affected_verts.Load (load_obj);
			}

			//
			//	Read the list of affected vertex colors to a chunk
			//
			if (result == IO_OK) {
				result = key_frame.affected_colors.Load (load_obj);
			}*/
		}
	}

	load_obj->CloseChunk ();

	// Return IO_OK on success IO_ERROR on failure
	return result;
}

