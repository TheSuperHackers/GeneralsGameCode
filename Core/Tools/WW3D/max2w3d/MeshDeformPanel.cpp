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
 *                    File Name : MeshDeformPanel.cpp                                          *
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


#include "MeshDeformPanel.h"
#include "resource.h"
#include "util.h"
#include "MeshDeform.h"

///////////////////////////////////////////////////////////////////////////
//
//	Local constants
//
///////////////////////////////////////////////////////////////////////////
const char * const PANEL_OBJ_PROP	= "WWPANELOBJ";


///////////////////////////////////////////////////////////////////////////
//
//	Message_Proc
//
///////////////////////////////////////////////////////////////////////////
BOOL WINAPI
MeshDeformPanelClass::Message_Proc
(
	HWND hwnd,
	UINT message,
	WPARAM wparam,
	LPARAM lparam
)
{
	// Lookup the controlling object for this panel
	MeshDeformPanelClass *panel_obj = MeshDeformPanelClass::Get_Object (hwnd);
	BOOL result = FALSE;

	switch (message)
	{
		// Create the controlling panel-object
		case WM_INITDIALOG:
			panel_obj = new MeshDeformPanelClass (hwnd);
			SetProp (hwnd, PANEL_OBJ_PROP, (HANDLE)panel_obj);
			break;

		case WM_DESTROY:
			result = panel_obj->On_Message (message, wparam, lparam);
			RemoveProp (hwnd, PANEL_OBJ_PROP);
			SAFE_DELETE (panel_obj);
			break;
	}

	// Pass the message onto the controlling panel-object
	if (panel_obj != nullptr) {
		result = panel_obj->On_Message (message, wparam, lparam);
	}

	// Return the TRUE/FALSE result code
	return result;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Object
//
///////////////////////////////////////////////////////////////////////////
MeshDeformPanelClass *
MeshDeformPanelClass::Get_Object (HWND hwnd)
{
	return (MeshDeformPanelClass *)::GetProp (hwnd, PANEL_OBJ_PROP);
}


///////////////////////////////////////////////////////////////////////////
//
//	On_Message
//
///////////////////////////////////////////////////////////////////////////
BOOL
MeshDeformPanelClass::On_Message
(
	UINT message,
	WPARAM wparam,
	LPARAM lparam
)
{
	switch (message)
	{
		case WM_INITDIALOG:
			m_pColorSwatch		= ::GetIColorSwatch (::GetDlgItem (m_hWnd, IDC_VERTEX_COLOR), RGB (0, 0, 0), "Vertex Color");
			m_pMaxSetsEdit		= ::GetICustEdit (::GetDlgItem (m_hWnd, IDC_MAX_SETS_EDIT));
			m_pMaxSetsSpin		= ::GetISpinner (::GetDlgItem (m_hWnd, IDC_MAX_SETS_SPIN));
			m_pLockSetsButton = ::GetICustButton (::GetDlgItem (m_hWnd, IDC_LOCK_SETS));

			//
			//	Setup the 'max-sets' controls
			//
			m_pMaxSetsSpin->LinkToEdit (::GetDlgItem (m_hWnd, IDC_MAX_SETS_EDIT), EDITTYPE_INT);
			m_pMaxSetsSpin->SetLimits (1, 20);
			m_pMaxSetsEdit->SetText (1);
			m_pMaxSetsSpin->SetValue (1, FALSE);
			::SetDlgItemInt (m_hWnd, IDC_CURRENT_SET_STATIC, 1, FALSE);

			//
			//	Setup the edit button
			//
			m_pLockSetsButton->SetType (CBT_CHECK);
			m_pLockSetsButton->SetCheck (FALSE);
			m_pLockSetsButton->SetHighlightColor (GREEN_WASH);
			//m_pEditButton->SetType (CBT_CHECK);
			//m_pEditButton->SetCheck (FALSE);
			//m_pEditButton->SetHighlightColor (GREEN_WASH);

			//
			//	Setup the sliders
			//
			::SendDlgItemMessage (m_hWnd, IDC_CURRENT_SET_SLIDER, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG (1, 1));
			::SendDlgItemMessage (m_hWnd, IDC_CURRENT_SET_SLIDER, TBM_SETPOS, (WPARAM)TRUE, 0L);
			::SendDlgItemMessage (m_hWnd, IDC_STATE_SLIDER, TBM_SETRANGE, (WPARAM)FALSE, MAKELONG (0, 10));
			::SendDlgItemMessage (m_hWnd, IDC_STATE_SLIDER, TBM_SETPOS, (WPARAM)FALSE, 9L);

			//
			//	Ensure the sliders are repainted
			//
			//::InvalidateRect (::GetDlgItem (m_hWnd, IDC_STATE_SLIDER), nullptr, TRUE);
			//::InvalidateRect (::GetDlgItem (m_hWnd, IDC_CURRENT_SET_SLIDER), nullptr, TRUE);
			break;

		case WM_DESTROY:
			::ReleaseIColorSwatch (m_pColorSwatch);
			::ReleaseICustEdit (m_pMaxSetsEdit);
			::ReleaseISpinner (m_pMaxSetsSpin);
			//::ReleaseICustButton (m_pEditButton);
			m_pColorSwatch = nullptr;
			m_pMaxSetsEdit = nullptr;
			m_pMaxSetsSpin = nullptr;
			//m_pEditButton = nullptr;
			break;

		case WM_COMMAND:
			On_Command (wparam, lparam);
			break;

		case CC_COLOR_CHANGE:
		{
			// Pass the new color onto the mesh deformer
			COLORREF color_ref = m_pColorSwatch->GetColor ();
			VertColor color;
			color.x = GetRValue (color_ref) / 255.0F;
			color.y = GetGValue (color_ref) / 255.0F;
			color.z = GetBValue (color_ref) / 255.0F;
			m_pMeshDeformer->Set_Vertex_Color (color, HIWORD (wparam) != 0);
		}
		break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_CHANGE:
		{
			Set_Max_Sets (m_pMaxSetsEdit->GetInt (), true);
		}
		break;

		case WM_HSCROLL:
			if ((HWND)lparam == ::GetDlgItem (m_hWnd, IDC_CURRENT_SET_SLIDER)) {
				int pos = ::SendDlgItemMessage (m_hWnd, IDC_CURRENT_SET_SLIDER, TBM_GETPOS, 0, 0L);
				Set_Current_Set (pos - 1, true);
			} else {
				int pos = ::SendDlgItemMessage (m_hWnd, IDC_STATE_SLIDER, TBM_GETPOS, 0, 0L);
				m_pMeshDeformer->Set_Deform_State (((float)pos) / 10.0F);

				if (pos > 0) {
					m_pColorSwatch->Enable ();
				} else {
					m_pColorSwatch->Disable ();
				}
			}

			break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////////////////////////
//
//	On_Command
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformPanelClass::On_Command
(
	WPARAM wparam,
	LPARAM lparam
)
{
	switch (LOWORD (wparam))
	{
		case IDC_MANUALAPPLY:
		{
			m_pMeshDeformer->Auto_Apply (Get_Auto_Apply_Check ());
		}
		break;

		//case IDC_EDIT_BUTTON:
			/*if (m_pEditButton->IsChecked ()) {
				::SendDlgItemMessage (m_hWnd, IDC_STATE_SLIDER, TBM_SETPOS, (WPARAM)TRUE, 100L);
				::EnableWindow (::GetDlgItem (m_hWnd, IDC_STATE_SLIDER), FALSE);
				m_pColorSwatch->Enable ();
				m_pMeshDeformer->Set_Deform_State (1.0F);
			} else {
				::EnableWindow (::GetDlgItem (m_hWnd, IDC_STATE_SLIDER), TRUE);
				m_pColorSwatch->Disable ();
			}*/
			//break;

		case IDC_MAX_SETS_EDIT:
			break;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Deformer
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformPanelClass::Set_Deformer (MeshDeformClass *obj)
{
	if (m_pMeshDeformer != obj) {
		m_pMeshDeformer = obj;

		// Set the slider position based on the current state of the deformer
		float state = m_pMeshDeformer->Get_Deform_State ();
		::SendDlgItemMessage (m_hWnd, IDC_STATE_SLIDER, TBM_SETPOS, (WPARAM)TRUE, LPARAM(state * 10.0F));

		// Now update the current vertex color
		Update_Vertex_Color ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Vertex_Color
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformPanelClass::Update_Vertex_Color (void)
{
	if (m_pMeshDeformer != nullptr) {

		// Update the color swatch with data from the deformer
		Point3 color;
		m_pMeshDeformer->Get_Vertex_Color (color);
		m_pColorSwatch->SetColor (RGB (int(color.x * 255.0F), int(color.y * 255.0F), int(color.z * 255.0F)), FALSE);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Max_Sets
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformPanelClass::Set_Max_Sets
(
	int max,
	bool notify
)
{
	// Update the UI
	::SendDlgItemMessage (m_hWnd, IDC_CURRENT_SET_SLIDER, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG (1, max));
	::SetDlgItemInt (m_hWnd, IDC_CURRENT_SET_STATIC, max, TRUE);

	if (notify == false) {
		m_pMaxSetsSpin->SetValue (max, TRUE);
	} else if (m_pMeshDeformer != nullptr) {

		// Update the deformer
		m_pMeshDeformer->Set_Max_Deform_Sets (max);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Current_Set
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformPanelClass::Set_Current_Set
(
	int set,
	bool notify
)
{
	// Update the UI
	::SetDlgItemInt (m_hWnd, IDC_CURRENT_SET_STATIC, set + 1, TRUE);

	if (notify == false) {
		::SendDlgItemMessage (m_hWnd, IDC_CURRENT_SET_SLIDER, TBM_SETPOS, (WPARAM)TRUE, set + 1);
	} else if (m_pMeshDeformer != nullptr) {

		// Update the deformer
		m_pMeshDeformer->Set_Current_Set (set, true);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Current_State
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformPanelClass::Set_Current_State (float state)
{
	::SendDlgItemMessage (m_hWnd, IDC_STATE_SLIDER, TBM_SETPOS, (WPARAM)TRUE, LPARAM(state * 10.0F));
	return ;
}
