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
 *                    File Name : MeshDeformPanel.h                                            *
 *                                                                                             *
 *                   Programmer : Patrick Smith                                                *
 *                                                                                             *
 *                   Start Date : 04/22/99                                                     *
 *                                                                                             *
 *                  Last Update :
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>
#include "resource.h"

// Forward declarations
class MeshDeformClass;

///////////////////////////////////////////////////////////////////////////
//
//	MeshDeformPanelClass
//
///////////////////////////////////////////////////////////////////////////
class MeshDeformPanelClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		MeshDeformPanelClass (HWND hwnd)
			:	m_hWnd (hwnd),
				m_pColorSwatch (nullptr),
				m_pMaxSetsSpin (nullptr),
				m_pMeshDeformer (nullptr),
				m_pLockSetsButton (nullptr),
				m_pMaxSetsEdit (nullptr)				{ }
		virtual ~MeshDeformPanelClass (void)	{ }

		//////////////////////////////////////////////////////////////////////
		// Public methods
		//////////////////////////////////////////////////////////////////////

		// Inline accessors
		IColorSwatch *				Get_Color_Swatch (void) const			{ return m_pColorSwatch; }
		COLORREF						Get_Vertex_Color (void) const			{ return m_pColorSwatch->GetColor (); }
		void							Set_Vertex_Color (COLORREF color)	{ m_pColorSwatch->SetColor (color); }
		void							Set_Deformer (MeshDeformClass *obj);
		BOOL							Is_Edit_Mode (void) const				{ return (::SendDlgItemMessage (m_hWnd, IDC_STATE_SLIDER, TBM_GETPOS, 0, 0L) > 0); }
		BOOL							Are_Sets_Tied (void) const				{ return m_pLockSetsButton->IsChecked (); }
		int							Get_Current_Set (void) const			{ return ::SendDlgItemMessage (m_hWnd, IDC_CURRENT_SET_SLIDER, TBM_GETPOS, 0, 0L); }
		void							Set_Current_Set (int set, bool notify = false);
		void							Set_Max_Sets (int max, bool notify = false);
		void							Set_Current_State (float state);
		void							Set_Auto_Apply_Check (bool onoff)	{ ::SendDlgItemMessage (m_hWnd, IDC_MANUALAPPLY, BM_SETCHECK, (WPARAM)(!onoff), 0L); }
		bool							Get_Auto_Apply_Check (void) const	{ return ::SendDlgItemMessage (m_hWnd, IDC_MANUALAPPLY, BM_GETCHECK, 0, 0L) == 0; }

		// Update methods
		void							Update_Vertex_Color (void);

		//////////////////////////////////////////////////////////////////////
		// Static methods
		//////////////////////////////////////////////////////////////////////
		static BOOL WINAPI				Message_Proc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		static MeshDeformPanelClass *	Get_Object (HWND hwnd);

	protected:

		//////////////////////////////////////////////////////////////////////
		// Protected methods
		//////////////////////////////////////////////////////////////////////
		BOOL							On_Message (UINT message, WPARAM wparam, LPARAM lparam);
		void							On_Command (WPARAM wparam, LPARAM lparam);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		HWND							m_hWnd;
		IColorSwatch *				m_pColorSwatch;
		ICustEdit *					m_pMaxSetsEdit;
		ISpinnerControl *			m_pMaxSetsSpin;
		ICustButton *				m_pLockSetsButton;
		MeshDeformClass *			m_pMeshDeformer;
};
