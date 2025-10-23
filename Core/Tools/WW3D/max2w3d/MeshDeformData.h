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
 *                    File Name : MeshDeformData.h                                             *
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

#pragma once

#include <max.h>
#include "Vector.H"
#include "MeshDeformSet.h"


///////////////////////////////////////////////////////////////////////////
//
//	Typedefs
//
///////////////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<MeshDeformSetClass *> SETS_LIST;


///////////////////////////////////////////////////////////////////////////
//
//	MeshDeformModData
//
///////////////////////////////////////////////////////////////////////////
class MeshDeformModData : public LocalModData
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		MeshDeformModData (void)
			:	m_CurrentSet (0)		{ }

		virtual ~MeshDeformModData (void);

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////
		virtual LocalModData *	Clone (void)	{ return new MeshDeformModData; }
		void							Record_Mesh_State (TriObject &tri_obj, float state, bool update_all);

		//	Inline accessors
		Mesh *					Peek_Mesh (void) const						{ return m_SetsList[m_CurrentSet]->Peek_Mesh (); }
		const Point3 *			Peek_Orig_Vertex_Array (void) const		{ return m_SetsList[m_CurrentSet]->Peek_Orig_Vertex_Array (); }
		Point3 *					Peek_Vertex_OPStart_Array (void) const	{ return m_SetsList[m_CurrentSet]->Peek_Vertex_OPStart_Array (); }
		VertColor *				Peek_Vertex_Colors (void) const			{ return m_SetsList[m_CurrentSet]->Peek_Vertex_Colors (); }

		// Auto apply
		bool						Is_Auto_Apply (void) const					{ return m_SetsList[m_CurrentSet]->Does_Set_Auto_Apply (); }
		void						Auto_Apply (bool auto_apply = true)		{ m_SetsList[m_CurrentSet]->Auto_Apply (auto_apply); }

		// Data modifiers
		void						Update_Current_Data (void)					{ m_SetsList[m_CurrentSet]->Update_Current_Data (); }
		void						Set_Vertex_Position (int index, const Point3 &value) { m_SetsList[m_CurrentSet]->Set_Vertex_Position (index, value); }
		void						Set_Vertex_Color (int index, int color_index, const VertColor &value) { m_SetsList[m_CurrentSet]->Set_Vertex_Color (index, color_index, value); }

		// Set managment
		void						Set_Max_Deform_Sets (int max);
		void						Set_Current_Set (int set_index)			{ m_CurrentSet = set_index; }
		int						Get_Current_Set (void) const				{ return m_CurrentSet; }
		void						Select_Set (int set_index)					{ m_SetsList[set_index]->Select_Members (); }
		void						Update_Set (int set_index, DEFORM_CHANNELS flags)	{ m_SetsList[set_index]->Update_Members (flags); }
		void						Restore_Set (int set_index = -1);
		MeshDeformSetClass &	Peek_Set (int index)							{ return *(m_SetsList[index]); }
		int						Get_Set_Count (void) const					{ return m_SetsList.Count (); }

		// Persistent storage
		IOResult					Save (ISave *save_obj);
		IOResult					Load (ILoad *load_obj);

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		void						Resize_Vertex_Array (int count, int color_count);
		void						Copy_Vertex_Array (Mesh &mesh);
		void						Free_Sets_List (void);
		void						Util_Restore_Set (int set_index);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////

		// Set managment
		int						m_CurrentSet;
		SETS_LIST				m_SetsList;
};
