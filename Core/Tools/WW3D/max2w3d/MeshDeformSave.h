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

/* $Header: /Commando/Code/Tools/max2w3d/MeshDeformSave.h 4     7/07/99 11:52a Patrick $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : MeshDeformSafe.H
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

#ifndef __MESH_DEFORM_SAVE_H
#define __MESH_DEFORM_SAVE_H

#include <max.h>
#include "Vector.H"

// Forward declarations
class ChunkSaveClass;
class MeshDeformModData;
class MeshDeformSaveSetClass;
class MeshBuilderClass;
class MeshDeformSaveSetClass;


///////////////////////////////////////////////////////////////////////////
//
//	Typdefs
//
///////////////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<MeshDeformSaveSetClass *> DEFORM_SAVE_LIST;


///////////////////////////////////////////////////////////////////////////
//
//	MeshDeformSaveClass
//
///////////////////////////////////////////////////////////////////////////
class MeshDeformSaveClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		MeshDeformSaveClass (void)
			:	m_AlphaPasses (0)			{ }
		~MeshDeformSaveClass (void)	{ Reset (); }

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////
		void					Initialize (MeshBuilderClass &builder, Object *object, Mesh &mesh, Matrix3 *transform = NULL);
		void					Initialize (MeshBuilderClass &builder, Mesh &mesh, MeshDeformModData &mod_data, Matrix3 *transform = NULL);

		//void					Re_Index (MeshBuilderClass &builder);
		bool					Export (ChunkSaveClass &chunk_save);

		void					Reset (void);
		bool					Is_Empty (void) const					{ return m_DeformSets.Count () == 0; }

		bool					Does_Deformer_Modify_DCG (void);

		unsigned int		Get_Alpha_Passes (void) const					{ return m_AlphaPasses; }
		void					Set_Alpha_Passes (unsigned int pass_mask)	{ m_AlphaPasses = pass_mask; }

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		bool					Export_Sets (ChunkSaveClass &chunk_save);
		bool					Export_Keyframes (ChunkSaveClass &chunk_save, MeshDeformSaveSetClass &set_save);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		DEFORM_SAVE_LIST	m_DeformSets;
		unsigned int		m_AlphaPasses;
};

#endif //__MESH_DEFORM_SAVE_H
