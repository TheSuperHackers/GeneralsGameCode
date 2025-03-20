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

/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D engine                                       * 
 *                                                                                             * 
 *                    File Name : MeshDeformData.cpp                                           * 
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


#include "MeshDeformData.h"
#include "util.h"
#include "MeshDeformSaveDefs.h"


///////////////////////////////////////////////////////////////////////////
//
//	~MeshDeformModData
//
///////////////////////////////////////////////////////////////////////////
MeshDeformModData::~MeshDeformModData (void)
{
	Free_Sets_List ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Record_Mesh_State
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformModData::Record_Mesh_State (TriObject &tri_obj, float state, bool update_all)
{
	//
	//	Ask each set to update its state
	//

	for (int index = 0; index < m_SetsList.Count (); index ++) {
		if (index != m_CurrentSet) {
			if (update_all) {
				m_SetsList[index]->Set_State (state);
			}
			m_SetsList[index]->Update_Mesh (tri_obj);
		}
	}

	if (m_CurrentSet < m_SetsList.Count ()) {
		m_SetsList[m_CurrentSet]->Set_State (state);
		m_SetsList[m_CurrentSet]->Update_Mesh (tri_obj);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Free_Sets_List
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformModData::Free_Sets_List (void)
{
	//
	//	Delete all the object pointers in the set list
	//
	for (int index = 0; index < m_SetsList.Count (); index ++) {
		MeshDeformSetClass *set = m_SetsList[index];
		SAFE_DELETE (set);
	}

	// Remove all the entries from the list
	m_SetsList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Max_Deform_Sets
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformModData::Set_Max_Deform_Sets (int max)
{
	int current_max = m_SetsList.Count ();
	if (max > current_max) {
		
		//
		//	Add the new sets to the list
		//
		int sets_to_add = max - current_max;
		for (int index = 0; index < sets_to_add; index ++) {
			MeshDeformSetClass *set = new MeshDeformSetClass;
			m_SetsList.Add (set);
		}

	} else if (max < current_max) {
		
		//
		//	Remove the obsolete sets from the list
		//
		int sets_to_remove = current_max - max;
		for (int index = 0; index < sets_to_remove; index ++) {
			
			// Restore the set before we delete it
			Restore_Set (max);

			// Delete the set
			MeshDeformSetClass *set = m_SetsList[max];
			SAFE_DELETE (set);
			m_SetsList.Delete (max);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Restore_Set
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformModData::Restore_Set (int set_index)
{
	if (set_index == -1) {

		// Restore ALL the set
		for (int index = 0; index < m_SetsList.Count (); index ++) {
			m_SetsList[index]->Restore_Members ();
		}			

	} else {
		m_SetsList[set_index]->Restore_Members ();
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////
IOResult
MeshDeformModData::Save (ISave *save_obj)
{
	DWORD bytes = 0L;
	save_obj->BeginChunk (DEFORM_CHUNK_INFO);

	//
	//	Write the set count info to the chunk
	//
	DeformChunk info = { 0 };
	info.SetCount = m_SetsList.Count ();
	IOResult result = save_obj->Write (&info, sizeof (info), &bytes);
	
	save_obj->EndChunk ();

	//
	//	Now write a chunk for each set
	//
	for (int index = 0; (index < m_SetsList.Count ()) && (result == IO_OK); index ++) {
		result = m_SetsList[index]->Save (save_obj);
	}
	
	// Return IO_OK on success IO_ERROR on failure
	return result;
}


///////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////
IOResult
MeshDeformModData::Load (ILoad *load_obj)
{
	Free_Sets_List ();
	DWORD bytes = 0L;

	//
	//	Is this the chunk we were expecting?
	//
	IOResult result = load_obj->OpenChunk ();
	if (	(result == IO_OK) &&
			(load_obj->CurChunkID () == DEFORM_CHUNK_INFO)) {

		DeformChunk info = { 0 };
		result = load_obj->Read (&info, sizeof (info), &bytes);
		load_obj->CloseChunk ();
		
		//
		//	Read the set information from the chunk
		//
		for (unsigned int index = 0; (index < info.SetCount) && (result == IO_OK); index ++) {
			MeshDeformSetClass *set = new MeshDeformSetClass;
			m_SetsList.Add (set);
			result = set->Load (load_obj);
		}		
	}	

	// Return IO_OK on success IO_ERROR on failure
	return result;
}

