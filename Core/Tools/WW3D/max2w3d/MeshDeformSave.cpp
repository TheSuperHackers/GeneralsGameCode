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

/* $Header: /Commando/Code/Tools/max2w3d/MeshDeformSave.cpp 6     11/12/99 11:12a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : MeshDeformSafe.CPP
 *                                                                                             *
 *                   Programmer : Patrick Smith                                                *
 *                                                                                             *
 *                   Start Date : 05/28/99                                                     *
 *                                                                                             *
 *                  Last Update :
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "MeshDeform.h"
#include "MeshDeformSave.h"
#include "MeshDeformData.h"
#include "MeshDeformSet.h"
#include "MeshDeformSaveSet.h"
#include "util.h"
#include "modstack.h"
#include "meshbuild.h"
#include "meshsave.h"

///////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveClass::Initialize
(
	MeshBuilderClass &builder,
	Object *				object,
	Mesh &				mesh,
	Matrix3 *			transform
)
{
	// Start fresh
	Reset ();

	//
	//	Attempt to gain access to the IDerivedObject this node references
	//
	int test = object->SuperClassID ();
	int test2 = GEN_DERIVOB_CLASS_ID;
	if ((object != NULL) &&
		 (object->SuperClassID () == GEN_DERIVOB_CLASS_ID)) {

		//
		//	Loop through all the modifiers and see if we can find the
		// Westwood Damage Mesh modifier.
		//
		IDerivedObject *derived_object = static_cast<IDerivedObject *> (object);
		int modifier_count = derived_object->NumModifiers ();
		bool found = false;
		for (int index = 0; (index < modifier_count) && !found; index ++) {

			//
			//	If this is the right modifier, then initialize using the
			//	data it contains.
			//
			Modifier *modifier = derived_object->GetModifier (index);
			if ((modifier != NULL) && (modifier->ClassID () == _MeshDeformClassID)) {

				//
				//	Attempt to get at the modifier data for this context
				//
				ModContext *mod_context = derived_object->GetModContext (index);
				if ((mod_context != NULL) && (mod_context->localData != NULL)) {
					MeshDeformModData *mod_data = static_cast<MeshDeformModData *> (mod_context->localData);
					Initialize (builder, mesh, *mod_data, transform);
				}

				// Found it!
				found = true;
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveClass::Initialize
(
	MeshBuilderClass &	builder,
	Mesh &					mesh,
	MeshDeformModData &	mod_data,
	Matrix3 *				transform
)
{
	//
	//	Loop through all the sets in the modifier
	//
	for (int index = 0; index < mod_data.Get_Set_Count (); index ++) {

		//
		//	If this set isn't empty then add its data to our list
		//
		MeshDeformSetClass &deform_set = mod_data.Peek_Set (index);
		if (deform_set.Is_Empty () == false) {

			//
			//	Add this set to our list
			//
			MeshDeformSaveSetClass *save_set = new MeshDeformSaveSetClass;
			deform_set.Save (builder, mesh, *save_set, transform);
			m_DeformSets.Add (save_set);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Reset
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveClass::Reset (void)
{
	//
	//	Delete all the damage sets
	//
	for (int index = 0; index < m_DeformSets.Count (); index ++) {
		SAFE_DELETE (m_DeformSets[index]);
	}

	m_DeformSets.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Export
//
///////////////////////////////////////////////////////////////////////////
bool
MeshDeformSaveClass::Export (ChunkSaveClass &chunk_save)
{
	bool retval = true;

	if (m_DeformSets.Count() > 0) {

		retval = chunk_save.Begin_Chunk (W3D_CHUNK_DEFORM);
		if (retval) {

			//
			//	Write the deform header to the file
			//
			W3dMeshDeform header = { 0 };
			header.SetCount		= m_DeformSets.Count ();
			header.AlphaPasses	= m_AlphaPasses;
			retval &= (chunk_save.Write (&header, sizeof (header)) == sizeof (header));
			if (retval) {

				//
				//	Export all the sets in the deformation
				//
				retval &= Export_Sets (chunk_save);
			}

			retval &= chunk_save.End_Chunk ();
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Export_Sets
//
///////////////////////////////////////////////////////////////////////////
bool
MeshDeformSaveClass::Export_Sets (ChunkSaveClass &chunk_save)
{
	bool retval = true;

	//
	//	Loop through all the sets and write them to the file
	//
	for (int set_index = 0; (set_index < m_DeformSets.Count ()) && retval; set_index ++) {
		retval &= chunk_save.Begin_Chunk (W3D_CHUNK_DEFORM_SET);
		if (retval) {

			//
			//	Write a chunk of information out for this set
			//
			MeshDeformSaveSetClass *set_save = m_DeformSets[set_index];
			W3dDeformSetInfo set_info = { 0 };
			set_info.KeyframeCount = set_save->Get_Keyframe_Count ();
			set_info.flags = set_save->Get_Flags ();
			retval &= (chunk_save.Write (&set_info, sizeof (set_info)) == sizeof (set_info));
			if (retval) {

				//
				//	Export all the keyframes for this chunk
				//
				retval &= Export_Keyframes (chunk_save, *set_save);
			}

			retval &= chunk_save.End_Chunk ();
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Export_Keyframes
//
///////////////////////////////////////////////////////////////////////////
bool
MeshDeformSaveClass::Export_Keyframes
(
	ChunkSaveClass &			chunk_save,
	MeshDeformSaveSetClass &set_save
)
{
	bool retval = true;

	//
	//	Loop through all the keyframes in the set
	//
	int count = set_save.Get_Keyframe_Count ();
	for (int keyframe_index = 0; (keyframe_index < count) && retval; keyframe_index ++) {

		//
		//	Write a chunk of information out for this keyframe
		//
		retval &= chunk_save.Begin_Chunk (W3D_CHUNK_DEFORM_KEYFRAME);
		if (retval) {
			W3dDeformKeyframeInfo keyframe_info = { 0 };
			keyframe_info.DeformPercent	= set_save.Get_Deform_State (keyframe_index);
			keyframe_info.DataCount			= set_save.Get_Deform_Data_Count (keyframe_index);

			retval &= (chunk_save.Write (&keyframe_info, sizeof (keyframe_info)) == sizeof (keyframe_info));
			if (retval) {

				//
				//	Loop through all the verticies in this keyframe
				//
				int data_count = set_save.Get_Deform_Data_Count (keyframe_index);
				for (int index = 0; (index < data_count) && retval; index ++) {
					MeshDeformSaveSetClass::DEFORM_DATA &data = set_save.Get_Deform_Data (keyframe_index, index);

					//
					//	Write a chunk of information out for this vertex
					//
					retval &= chunk_save.Begin_Chunk (W3D_CHUNK_DEFORM_DATA);
					if (retval) {
						W3dDeformData data_struct = { 0 };
						data_struct.VertexIndex	= data.vert_index;
						data_struct.Position.X	= data.position.x;
						data_struct.Position.Y	= data.position.y;
						data_struct.Position.Z	= data.position.z;
						data_struct.Color.R		= data.color.x * 255;
						data_struct.Color.G		= data.color.y * 255;
						data_struct.Color.B		= data.color.z * 255;

						// If we are using vertex alpha instead of vertex color, then convert
						// the v-color into an alpha setting
						data_struct.Color.A		= 255;
						if (m_AlphaPasses != 0) {
							data_struct.Color.A	= (data_struct.Color.R + data_struct.Color.G + data_struct.Color.B) / 3.0F;
						}

						retval &= (chunk_save.Write (&data_struct, sizeof (data_struct)) == sizeof (data_struct));
						retval &= chunk_save.End_Chunk ();
					}
				}
			}

			retval &= chunk_save.End_Chunk ();
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Re_Index
//
///////////////////////////////////////////////////////////////////////////
/*void
MeshDeformSaveClass::Re_Index (MeshBuilderClass &builder)
{
	DynamicVectorClass<MeshDeformSaveSetClass::DEFORM_DATA> temp_list;

	//
	//	Reindex each set of deform data
	//
	for (int set_index = 0; set_index < m_DeformSets.Count (); set_index ++) {
		MeshDeformSaveSetClass *set_save = m_DeformSets[set_index];

		//
		//	Loop through all the deform entries in this set
		//
		for (int keyframe_index = 0; keyframe_index < set_save->Get_Keyframe_Count (); keyframe_index ++) {
			temp_list.Delete_All ();
			for (int index = 0; index < set_save->Get_Deform_Data_Count (keyframe_index); index ++) {
				MeshDeformSaveSetClass::DEFORM_DATA &data = set_save->Get_Deform_Data (keyframe_index, index);

				//
				//	Now try to find the 'W3D' index of this vertex (its different than the max version).
				//
				//bool found = false;
				for (int vert_index = 0; vert_index < builder.Get_Vertex_Count (); vert_index++) {
					MeshBuilderClass::VertClass &vert = builder.Get_Vertex (vert_index);

					//
					//	Reindex this vertex if its the one we are looking for.
					//
					if (vert.Id == (int)data.vert_index) {
						MeshDeformSaveSetClass::DEFORM_DATA new_data = data;
						new_data.vert_index = vert_index;
						temp_list.Add (new_data);
						//data.vert_index = vert_index;
						//found = true;
					}
				}
			}

			set_save->Replace_Deform_Data (keyframe_index, temp_list);
		}
	}

	return ;
}*/


///////////////////////////////////////////////////////////////////////////
//
//	Does_Deformer_Modify_DCG
//
///////////////////////////////////////////////////////////////////////////
bool
MeshDeformSaveClass::Does_Deformer_Modify_DCG (void)
{
	bool retval = false;

	//
	//	Loop through all the sets
	//
	for (int set_index = 0; (set_index < m_DeformSets.Count ()) && !retval; set_index ++) {
		MeshDeformSaveSetClass *set_save = m_DeformSets[set_index];
		if (set_save) {

			//
			//	Loop through all the keyframes in this set
			//
			int count = set_save->Get_Keyframe_Count ();
			for (int keyframe_index = 0; (keyframe_index < count) && !retval; keyframe_index ++) {

				//
				//	Loop through all the entries in this keyframe
				//
				int data_count = set_save->Get_Deform_Data_Count (keyframe_index);
				for (int index = 0; (index < data_count) && !retval; index ++) {
					MeshDeformSaveSetClass::DEFORM_DATA &data = set_save->Get_Deform_Data (keyframe_index, index);

					//
					//	If the color is not 'white' then we will
					// modify the DCG array.
					//
					if ((data.color.x != 1) ||
						 (data.color.y != 1) ||
						 (data.color.z != 1)) {
						retval = true;
					}
				}
			}
		}
	}

	// Return the true/false result code
	return retval;
}
