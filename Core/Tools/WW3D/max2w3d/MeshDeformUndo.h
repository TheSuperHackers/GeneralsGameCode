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

#pragma once

#include <max.h>
#include "Vector.h"
#include "MeshDeformDefs.h"

// Forward declarations
class MeshDeformClass;
class MeshDeformModData;



///////////////////////////////////////////////////////////////////////////
//
//	VertexRestoreClass
//
///////////////////////////////////////////////////////////////////////////
class VertexRestoreClass : public RestoreObj
{
	public:

		//////////////////////////////////////////////////////////////////
		//	Public constructors/destructor
		//////////////////////////////////////////////////////////////////
		VertexRestoreClass (Mesh *mesh, MeshDeformClass *modifier, MeshDeformModData *mod_data);
		virtual ~VertexRestoreClass (void)	{ Free_Vertex_Array (); };

		//////////////////////////////////////////////////////////////////
		//	RestoreObj overrides
		//////////////////////////////////////////////////////////////////
		virtual void			Restore (int is_undo);
		virtual void			Redo (void);
		virtual int				Size (void)				{ return (m_VertexList.Count () * sizeof (Point3) * 2) + sizeof (VertexRestoreClass); }
		virtual void			EndHold (void);

	protected:

		//////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////
		virtual void			Copy_Vertex_State (DEFORM_LIST &list) = 0;
		virtual void			Apply_Vertex_Data (DEFORM_LIST &list) = 0;
		void						Free_Vertex_Array (void);

		//////////////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////////////
		Mesh *					m_pMesh;
		MeshDeformClass *		m_pModifier;
		MeshDeformModData *	m_pModData;
		DEFORM_LIST				m_VertexList;
		DEFORM_LIST				m_RedoVertexList;
		int						m_SetIndex;
		int						m_KeyframeIndex;
};


///////////////////////////////////////////////////////////////////////////
//
//	VertexPositionRestoreClass
//
///////////////////////////////////////////////////////////////////////////
class VertexPositionRestoreClass : public VertexRestoreClass
{
	public:

		//////////////////////////////////////////////////////////////////
		//	Public constructors/destructor
		//////////////////////////////////////////////////////////////////
		VertexPositionRestoreClass (Mesh *mesh, MeshDeformClass *modifier, MeshDeformModData *mod_data);
		virtual ~VertexPositionRestoreClass (void)	{ };

		//////////////////////////////////////////////////////////////////
		//	RestoreObj overrides
		//////////////////////////////////////////////////////////////////
		TSTR				Description (void)	{ return TSTR(_T("Vertex Position")); }

	protected:

		//////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////
		virtual void	Copy_Vertex_State (DEFORM_LIST &list);
		virtual void	Apply_Vertex_Data (DEFORM_LIST &list);
};


///////////////////////////////////////////////////////////////////////////
//
//	VertexColorRestoreClass
//
///////////////////////////////////////////////////////////////////////////
class VertexColorRestoreClass : public VertexRestoreClass
{
	public:

		//////////////////////////////////////////////////////////////////
		//	Public constructors/destructor
		//////////////////////////////////////////////////////////////////
		VertexColorRestoreClass (Mesh *mesh, MeshDeformClass *modifier, MeshDeformModData *mod_data);
		virtual ~VertexColorRestoreClass (void)	{ };

		//////////////////////////////////////////////////////////////////
		//	RestoreObj overrides
		//////////////////////////////////////////////////////////////////
		TSTR				Description (void)	{ return TSTR(_T("Vertex Color")); }

	protected:

		//////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////
		virtual void	Copy_Vertex_State (DEFORM_LIST &list);
		virtual void	Apply_Vertex_Data (DEFORM_LIST &list);
};
