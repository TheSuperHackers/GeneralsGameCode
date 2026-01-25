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

/* $Header: /Commando/Code/Tools/max2w3d/MeshDeformSaveSet.h 2     6/16/99 6:56p Patrick $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : MeshDeformSaveSet.h
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

#pragma once

#include <max.h>
#include "Vector.h"

// Forward declarations
class ChunkSaveClass;


///////////////////////////////////////////////////////////////////////////
//
//	MeshDeformSaveSetClass
//
///////////////////////////////////////////////////////////////////////////
class MeshDeformSaveSetClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public friends
		//////////////////////////////////////////////////////////////////////
		friend class MeshDeformSaveClass;


	protected:

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected data types
		//////////////////////////////////////////////////////////////////////
		typedef struct _DEFORM_DATA
		{
			UINT			vert_index;
			Point3		position;
			VertColor	color;

			// Don't care, DynamicVectorClass needs these
			bool operator== (const _DEFORM_DATA &src) { return false; }
			bool operator!= (const _DEFORM_DATA &src) { return true; }
		} DEFORM_DATA;

		//////////////////////////////////////////////////////////////////////
		//	Protected data types
		//////////////////////////////////////////////////////////////////////
		typedef struct
		{
			float										state;
			DynamicVectorClass<DEFORM_DATA>	deform_list;
		} KEYFRAME;


public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		MeshDeformSaveSetClass (void)
			:	m_Flags (0),
				m_CurrentKeyFrame (nullptr)	{ }
		~MeshDeformSaveSetClass (void)	{ Reset (); }

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////

		// Keyframe management
		void					Begin_Keyframe (float state);
		void					End_Keyframe (void);

		// Vertex management
		void					Add_Vert (UINT vert_index, const Point3 &position, const VertColor &color);

		// Misc
		void					Reset (void);
		bool					Is_Empty (void) const	{ return m_DeformData.Count () == 0; }

		// Flag support
		bool					Get_Flag (unsigned int flag) const				{ return (m_Flags & flag) == flag; }
		void					Set_Flag (unsigned int flag, bool value)		{ if (value) (m_Flags |= flag); else (m_Flags &= ~flag); }
		unsigned int		Get_Flags (void) const								{ return m_Flags; }

		// Enumeration
		float					Get_Deform_State (int key_frame) const			{ return m_DeformData[key_frame]->state; }
		int					Get_Keyframe_Count (void) const					{ return m_DeformData.Count (); }
		int					Get_Deform_Data_Count (int key_frame) const	{ return m_DeformData[key_frame]->deform_list.Count (); }
		DEFORM_DATA &		Get_Deform_Data (int key_frame, int index)	{ return m_DeformData[key_frame]->deform_list[index]; }
		void					Replace_Deform_Data (int keyframe_index, DynamicVectorClass<DEFORM_DATA> &list);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		DynamicVectorClass<KEYFRAME *>		m_DeformData;
		KEYFRAME *									m_CurrentKeyFrame;
		unsigned int								m_Flags;
};
