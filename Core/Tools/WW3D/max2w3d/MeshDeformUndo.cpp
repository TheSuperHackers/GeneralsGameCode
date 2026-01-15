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
 *                    File Name : MeshDeformUndo.h                                             *
 *                                                                                             *
 *                   Programmer : Patrick Smith                                                *
 *                                                                                             *
 *                   Start Date : 06/08/99                                                     *
 *                                                                                             *
 *                  Last Update :
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "MeshDeformUndo.h"
#include "util.h"
#include "MeshDeformData.h"
#include "MeshDeformSet.h"
#include "MeshDeform.h"


///////////////////////////////////////////////////////////////////////////
//
//	VertexRestoreClass
//
///////////////////////////////////////////////////////////////////////////
VertexRestoreClass::VertexRestoreClass
(
	Mesh *					mesh,
	MeshDeformClass *		modifier,
	MeshDeformModData *	mod_data
)
	:	m_pModifier (modifier),
		m_pModData (mod_data),
		m_pMesh (mesh),
		m_SetIndex (0),
		m_KeyframeIndex (0)
{
	assert (mesh != nullptr);

	//
	//	Remember the deformer's current settings
	//
	m_SetIndex			= m_pModData->Get_Current_Set ();
	m_KeyframeIndex	= m_pModData->Peek_Set (m_SetIndex).Get_Current_Key_Frame ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Free_Vertex_Array
//
///////////////////////////////////////////////////////////////////////////
void
VertexRestoreClass::Free_Vertex_Array (void)
{
	m_VertexList.Delete_All ();
	m_RedoVertexList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Restore
//
///////////////////////////////////////////////////////////////////////////
void
VertexRestoreClass::Restore (int is_undo)
{
	assert (m_pMesh != nullptr);
	assert (m_pModData != nullptr);
	assert (m_pModifier != nullptr);

	// Is this being called as part of an undo operation?
	if (is_undo != 0) {

		//
		//	Ensure the modifier is in the state it was when
		// the undo operation was recorded
		//
		m_pModData->Set_Current_Set (m_SetIndex);
		m_pModData->Peek_Set (m_SetIndex).Set_Current_Key_Frame (m_KeyframeIndex);

		//
		//	Apply the original vertex positions to the mesh
		//
		Apply_Vertex_Data (m_VertexList);

		//
		//	Notify the mesh of geometry changes
		//
		m_pModifier->NotifyDependents (FOREVER, PART_GEOM | PART_VERTCOLOR, REFMSG_CHANGE);
		m_pModifier->Update_UI (m_pModData);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Redo
//
///////////////////////////////////////////////////////////////////////////
void
VertexRestoreClass::Redo (void)
{
	assert (m_pMesh != nullptr);
	assert (m_pModData != nullptr);
	assert (m_pModifier != nullptr);

	//
	//	Ensure the modifier is in the state it was when
	// the undo operation was recorded
	//
	m_pModData->Set_Current_Set (m_SetIndex);
	m_pModData->Peek_Set (m_SetIndex).Set_Current_Key_Frame (m_KeyframeIndex);

	//
	//	Apply the original vertex positions to the mesh
	//
	Apply_Vertex_Data (m_RedoVertexList);

	//
	//	Notify the mesh of geometry changes
	//
 	m_pModifier->NotifyDependents (FOREVER, PART_GEOM | PART_VERTCOLOR, REFMSG_CHANGE);
	m_pModifier->Update_UI (m_pModData);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	EndHold
//
///////////////////////////////////////////////////////////////////////////
void
VertexRestoreClass::EndHold (void)
{
	//
	//	Record the position of all the verts we are about to change
	// (to support redo).
	//
	Copy_Vertex_State (m_RedoVertexList);
	m_pModifier->ClearAFlag (A_HELD);
	return ;
}


/***************************************************************************************/
/*
/*	End VertexRestoreClass
/*
/***************************************************************************************/

/***************************************************************************************/
/*
/*	Start VertexPositionRestoreClass
/*
/***************************************************************************************/


///////////////////////////////////////////////////////////////////////////
//
//	VertexPositionRestoreClass
//
///////////////////////////////////////////////////////////////////////////
VertexPositionRestoreClass::VertexPositionRestoreClass
(
	Mesh *					mesh,
	MeshDeformClass *		modifier,
	MeshDeformModData *	mod_data
)
	:	VertexRestoreClass (mesh, modifier, mod_data)
{
	//
	//	Make a copy of the vertex positions
	//
	Copy_Vertex_State (m_VertexList);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Copy_Vertex_State
//
///////////////////////////////////////////////////////////////////////////
void
VertexPositionRestoreClass::Copy_Vertex_State (DEFORM_LIST &list)
{
	//
	// Make a copy of each vertex in the current keyframe
	//
	list.Delete_All ();
	MeshDeformSetClass &set_obj = m_pModData->Peek_Set (m_SetIndex);
	int count = set_obj.Get_Vertex_Count (m_KeyframeIndex);
	for (int index = 0; index < count; index ++) {
		const VERT_INFO &data = set_obj.Get_Vertex_Data (m_KeyframeIndex, index);
		list.Add (VERT_INFO (data.index, data.value));
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Vertex_Data
//
///////////////////////////////////////////////////////////////////////////
void
VertexPositionRestoreClass::Apply_Vertex_Data (DEFORM_LIST &list)
{
	m_pModData->Peek_Set (m_SetIndex).Reset_Key_Frame_Verts (m_KeyframeIndex);

	//
	// Apply each vertex in our list
	//
	for (int index = 0; index < list.Count (); index ++) {
		VERT_INFO &info = list[index];
		m_pModData->Set_Vertex_Position (info.index, info.value);
	}

	return ;
}



/***************************************************************************************/
/*
/*	End VertexPositionRestoreClass
/*
/***************************************************************************************/

/***************************************************************************************/
/*
/*	Start VertexColorRestoreClass
/*
/***************************************************************************************/


///////////////////////////////////////////////////////////////////////////
//
//	VertexColorRestoreClass
//
///////////////////////////////////////////////////////////////////////////
VertexColorRestoreClass::VertexColorRestoreClass
(
	Mesh *					mesh,
	MeshDeformClass *		modifier,
	MeshDeformModData *	mod_data
)
	:	VertexRestoreClass (mesh, modifier, mod_data)
{
	//
	//	Make a copy of the vertex positions
	//
	Copy_Vertex_State (m_VertexList);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Copy_Vertex_State
//
///////////////////////////////////////////////////////////////////////////
void
VertexColorRestoreClass::Copy_Vertex_State (DEFORM_LIST &list)
{
	//
	// Make a copy of each vertex color in the current keyframe
	//
	list.Delete_All ();
	MeshDeformSetClass &set_obj = m_pModData->Peek_Set (m_SetIndex);
	int count = set_obj.Get_Color_Count (m_KeyframeIndex);
	for (int index = 0; index < count; index ++) {
		const VERT_INFO &data = set_obj.Get_Color_Data (m_KeyframeIndex, index);
		list.Add (VERT_INFO (data.index, data.value, data.color_index));
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Vertex_Data
//
///////////////////////////////////////////////////////////////////////////
void
VertexColorRestoreClass::Apply_Vertex_Data (DEFORM_LIST &list)
{
	m_pModData->Peek_Set (m_SetIndex).Reset_Key_Frame_Colors (m_KeyframeIndex);

	//
	// Apply each vertex in our list
	//
	for (int index = 0; index < list.Count (); index ++) {
		VERT_INFO &info = list[index];
		m_pModData->Set_Vertex_Color (info.index, info.color_index, info.value);
	}

	return ;
}

