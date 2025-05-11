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
 *                    File Name : MeshDeformSet.h                                              * 
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


#ifndef __MESH_DEFORM_SET_H
#define __MESH_DEFORM_SET_H

#include <max.h>
#include "Vector.H"
#include "MeshDeformDefs.h"


// Forward declarations
class MeshDeformSaveSetClass;
class MeshBuilderClass;


///////////////////////////////////////////////////////////////////////////
//
//	MeshDeformSetClass
//
///////////////////////////////////////////////////////////////////////////
class MeshDeformSetClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public data types
		//////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		MeshDeformSetClass (void)
			:	m_pMesh (NULL),
				m_pVertexArray (NULL),
				m_pVertexOPStartArray (NULL),
				m_pVertexColors (NULL),
				m_VertexColorCount (0),
				m_State (0),
				m_CurrentKeyFrame (0),
				m_bAutoApply (true),
				m_VertexCount (0)					{ Init_Key_Frames (); }

		virtual ~MeshDeformSetClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////			
		//virtual LocalModData *	Clone (void)	{ return new MeshDeformSetClass; }
		void					Update_Mesh (TriObject &tri_obj);
		void					Set_State (float state);
		
		//	Inline accessors
		Mesh *				Peek_Mesh (void) const						{ return m_pMesh; }
		const Point3 *		Peek_Orig_Vertex_Array (void) const		{ return m_pVertexArray; }
		Point3 *				Peek_Vertex_OPStart_Array (void) const	{ return m_pVertexOPStartArray; }
		VertColor *			Peek_Vertex_Colors (void) const			{ return m_pVertexColors; }

		// Keyframe managment
		void					Set_Current_Key_Frame (int index);
		int					Get_Current_Key_Frame (void) const		{ return m_CurrentKeyFrame; }
		void					Update_Key_Frame (int key_frame);
		void					Update_Current_Data (void);
		void					Update_Set_Members (void);
		void					Collapse_Keyframe_Data (int keyframe);
		void					Reset_Key_Frame_Verts (int keyframe);
		void					Reset_Key_Frame_Colors (int keyframe);
		
		// Data managment
		void					Set_Vertex_Position (int index, const Point3 &value);
		void					Set_Vertex_Color (int index, int color_index, const VertColor &value);

		// Set managment
		void					Select_Members (void);
		void					Update_Members (DEFORM_CHANNELS flags);
		void					Restore_Members (void);

		//	Auto apply
		bool					Does_Set_Auto_Apply (void) const			{ return m_bAutoApply; }
		void					Auto_Apply (bool auto_apply = true)		{ m_bAutoApply = auto_apply; }

		// Information
		bool					Is_Empty (void) const;
		int					Get_Vertex_Count (int keyframe) const				{ return m_KeyFrames[keyframe]->verticies.Count (); }
		int					Get_Color_Count (int keyframe) const				{ return m_KeyFrames[keyframe]->colors.Count (); }
		const VERT_INFO &	Get_Vertex_Data (int keyframe, int index) const	{ return m_KeyFrames[keyframe]->verticies[index]; }
		const VERT_INFO &	Get_Color_Data (int keyframe, int index) const	{ return m_KeyFrames[keyframe]->colors[index]; }

		// Persistent storage
		IOResult				Save (ISave *save_obj);
		IOResult				Load (ILoad *load_obj);

		void					Save (MeshBuilderClass &builder, Mesh &mesh, MeshDeformSaveSetClass &save_set, Matrix3 *transform = NULL);

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		void					Resize_Vertex_Array (int count, int color_count);
		void					Copy_Vertex_Array (Mesh &mesh);
		
		// Keyframe methods
		void					Init_Key_Frames (void);
		void					Free_Key_Frames (void);
		void					Determine_Interpolation_Indicies (int key_frame, bool position, int &from, int &to, float &state);
		
		// Deformation application methods
		void					Apply_Position_Changes (UINT vert, int frame_to_check, Point3 &position, Matrix3 *transform = NULL);
		void					Apply_Color_Changes (UINT vert, int frame_to_check, Mesh &mesh);
		void					Apply_Color_Changes (UINT vert_index, UINT vert_color_index, int frame_to_check, VertColor &color);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private data types
		//////////////////////////////////////////////////////////////////////
		typedef struct
		{
			DEFORM_LIST	verticies;
			DEFORM_LIST	colors;
			BitArray		affected_verts;
			BitArray		affected_colors;
		}	KEY_FRAME;

		typedef DynamicVectorClass<KEY_FRAME *>		KEY_FRAME_LIST;

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////			
		Mesh *				m_pMesh;
		Point3 *				m_pVertexArray;
		Point3 *				m_pVertexOPStartArray;
		VertColor *			m_pVertexColors;
		int					m_VertexCount;
		int					m_VertexColorCount;
		int					m_CurrentKeyFrame;
		float					m_State;
		bool					m_bAutoApply;

		// Array representing which verticies are part of the set
		BitArray				m_SetMembers;

		// List of key frames
		KEY_FRAME_LIST		m_KeyFrames;
};


#endif //__MESH_DEFORM_DATA_H

