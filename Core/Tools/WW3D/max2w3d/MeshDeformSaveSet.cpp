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

/* $Header: /Commando/Code/Tools/max2w3d/MeshDeformSaveSet.cpp 2     6/16/99 6:56p Patrick $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : MeshDeformSaveSet.CPP
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

#include "MeshDeformSaveSet.h"
#include "util.h"


////////////////////////////////////////////////////////////////////////
//
//	Reset
//
////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveSetClass::Reset (void)
{
	//
	//	Free all the keyframe pointers in our list
	//
	for (int index = 0; index < m_DeformData.Count (); index ++) {
		SAFE_DELETE (m_DeformData[index]);
	}

	m_DeformData.Delete_All ();
	m_CurrentKeyFrame = NULL;
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Begin_Keyframe
//
////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveSetClass::Begin_Keyframe (float state)
{
	//
	//	Allocate a new keyframe structure
	//
	m_CurrentKeyFrame = new KEYFRAME;
	m_CurrentKeyFrame->state = state;

	//
	//	Add this new keyframe to the end of our list
	//
	m_DeformData.Add (m_CurrentKeyFrame);
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	End_Keyframe
//
////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveSetClass::End_Keyframe (void)
{
	m_CurrentKeyFrame = NULL;
	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Add_Vert
//
////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveSetClass::Add_Vert
(
	UINT					vert_index,
	const Point3 &		position,
	const VertColor &	color
)
{
	// State OK?
	assert (m_CurrentKeyFrame != NULL);
	if (m_CurrentKeyFrame != NULL) {

		//
		//	Create a structure that will hold the
		//	vertex information.
		//
		DEFORM_DATA data;
		data.vert_index	= vert_index;
		data.position		= position;
		data.color			= color;

		//
		//	Add this vertex information to the keyframe list
		//
		m_CurrentKeyFrame->deform_list.Add (data);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Replace_Deform_Data
//
////////////////////////////////////////////////////////////////////////
void
MeshDeformSaveSetClass::Replace_Deform_Data
(
	int										keyframe_index,
	DynamicVectorClass<DEFORM_DATA> &list
)
{
	KEYFRAME *key_frame = m_DeformData[keyframe_index];
	if (key_frame != NULL) {

		//
		//	Replace the vertex deformation list for the keyframe
		//
		key_frame->deform_list.Delete_All ();
		key_frame->deform_list = list;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_Deform_Count
//
////////////////////////////////////////////////////////////////////////
/*int
MeshDeformSaveSetClass::Get_Deform_Count (void) const
{
	//
	//	Count up all the deform entries for all the keyframes
	//
	int count = 0;
	for (int index = 0; index < m_DeformData.Count (); index ++) {
		KEYFRAME *key_frame = m_DeformData[index];
		if (key_frame != NULL) {
			count += key_frame->deform_list.Count ();
		}
	}

	return count;
}*/


