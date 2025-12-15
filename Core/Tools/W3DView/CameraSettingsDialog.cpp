/*
**	Command & Conquer Renegade(tm)
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

// CameraSettingsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "W3DViewDoc.h"
#include "GraphicView.h"
#include "CameraSettingsDialog.h"
#include "Utils.h"
#include "camera.h"
#include "ViewerScene.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CameraSettingsDialogClass
//
/////////////////////////////////////////////////////////////////////////////
CameraSettingsDialogClass::CameraSettingsDialogClass(CWnd* pParent /*=nullptr*/)
	: CDialog(CameraSettingsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CameraSettingsDialogClass)
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
CameraSettingsDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CameraSettingsDialogClass)
	DDX_Control(pDX, IDC_LENS_SPIN, m_LensSpin);
	DDX_Control(pDX, IDC_FAR_CLIP_SPIN, m_FarClipSpin);
	DDX_Control(pDX, IDC_VFOV_SPIN, m_VFOVSpin);
	DDX_Control(pDX, IDC_NEAR_CLIP_SPIN, m_NearClipSpin);
	DDX_Control(pDX, IDC_HFOV_SPIN, m_HFOVSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(CameraSettingsDialogClass, CDialog)
	//{{AFX_MSG_MAP(CameraSettingsDialogClass)
	ON_BN_CLICKED(IDC_FOV_CHECK, OnFovCheck)
	ON_BN_CLICKED(IDC_CLIP_PLANE_CHECK, OnClipPlaneCheck)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CameraSettingsDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	CW3DViewDoc *doc				= ::GetCurrentDocument ();
	CGraphicView *graphic_view = doc->GetGraphicView ();
	CameraClass *camera			= graphic_view->GetCamera ();

	//
	//	Enable/disable the group boxes
	//
	SendDlgItemMessage (IDC_FOV_CHECK, BM_SETCHECK, (WPARAM)doc->Is_FOV_Manual ());
	SendDlgItemMessage (IDC_CLIP_PLANE_CHECK, BM_SETCHECK, (WPARAM)doc->Are_Clip_Planes_Manual ());

	float znear = 0;
	float zfar = 0;
	camera->Get_Clip_Planes (znear, zfar);
	::Initialize_Spinner (m_NearClipSpin, znear, 0.0F, 999999.0F);
	::Initialize_Spinner (m_FarClipSpin, zfar, 1.0F, 999999.0F);

	//
	//	Setup the FOV controls
	//
	int hfov_deg = (int)RAD_TO_DEG (camera->Get_Horizontal_FOV ());
	int vfov_deg = (int)RAD_TO_DEG (camera->Get_Vertical_FOV ());
	::Initialize_Spinner (m_HFOVSpin, hfov_deg, 0.0F, 180.0F);
	::Initialize_Spinner (m_VFOVSpin, vfov_deg, 0.0F, 180.0F);

	//
	//	Setup the camera lens controls
	//
	float hfov = camera->Get_Horizontal_FOV ();
	const float constant	= (18.0F / 1000.0F);
	float lens				= (constant / (::tan (hfov / 2))) * 1000.0F;
	::Initialize_Spinner (m_LensSpin, lens, 1.0F, 200.0F);

	OnFovCheck ();
	OnClipPlaneCheck ();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
CameraSettingsDialogClass::OnOK (void)
{
	CW3DViewDoc *doc				= ::GetCurrentDocument ();
	CGraphicView *graphic_view = doc->GetGraphicView ();
	CameraClass *camera			= graphic_view->GetCamera ();

	bool manual_fov		= (SendDlgItemMessage (IDC_FOV_CHECK, BM_GETCHECK) == 1);
	bool manual_planes	= (SendDlgItemMessage (IDC_CLIP_PLANE_CHECK, BM_GETCHECK) == 1);

	doc->Set_Manual_FOV (manual_fov);
	doc->Set_Manul_Clip_Planes (manual_planes);
	if (manual_fov == false) {
		graphic_view->Reset_FOV ();
	} else {

		//
		//	Update the camera's FOV
		//
		float hfov_deg = ::GetDlgItemFloat (m_hWnd, IDC_HFOV_EDIT);
		float vfov_deg = ::GetDlgItemFloat (m_hWnd, IDC_VFOV_EDIT);
		camera->Set_View_Plane (DEG_TO_RAD (hfov_deg), DEG_TO_RAD (vfov_deg));
	}

	//
	//	Update the camera's clip planes
	//
	float znear = ::GetDlgItemFloat (m_hWnd, IDC_NEAR_CLIP_EDIT);
	float zfar = ::GetDlgItemFloat (m_hWnd, IDC_FAR_CLIP_EDIT);
	camera->Set_Clip_Planes (znear, zfar);
	doc->Save_Camera_Settings ();

	//
	// Update the fog settings. The fog near clip plane should always be equal
	// to the camera near clip plane, but the fog far clip plane is scene
	// dependant. We will be sure to modify only the near clip plane here.
	//
	float fog_near, fog_far;
	doc->GetScene()->Get_Fog_Range(&fog_near, &fog_far);
	doc->GetScene()->Set_Fog_Range(znear, fog_far);
	doc->GetScene()->Recalculate_Fog_Planes();

	//
	//	Refresh the camera settings
	//
	RenderObjClass *render_obj = doc->GetDisplayedObject ();
	if (render_obj != nullptr) {
		graphic_view->Reset_Camera_To_Display_Object (*render_obj);
	}

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnFovCheck
//
/////////////////////////////////////////////////////////////////////////////
void
CameraSettingsDialogClass::OnFovCheck (void)
{
	bool manual_fov = (SendDlgItemMessage (IDC_FOV_CHECK, BM_GETCHECK) == 1);
	::EnableWindow (m_VFOVSpin, manual_fov);
	::EnableWindow (m_HFOVSpin, manual_fov);
	::EnableWindow (m_LensSpin, manual_fov);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_VFOV_EDIT), manual_fov);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_HFOV_EDIT), manual_fov);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LENS_EDIT), manual_fov);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnClipPlaneCheck
//
/////////////////////////////////////////////////////////////////////////////
void
CameraSettingsDialogClass::OnClipPlaneCheck (void)
{
	bool manual_planes = (SendDlgItemMessage (IDC_CLIP_PLANE_CHECK, BM_GETCHECK) == 1);
	::EnableWindow (m_NearClipSpin, manual_planes);
	::EnableWindow (m_FarClipSpin, manual_planes);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_NEAR_CLIP_EDIT), manual_planes);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_FAR_CLIP_EDIT), manual_planes);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnReset
//
/////////////////////////////////////////////////////////////////////////////
void
CameraSettingsDialogClass::OnReset (void)
{
	CW3DViewDoc *doc				= ::GetCurrentDocument ();
	CGraphicView *graphic_view = doc->GetGraphicView ();
	CameraClass *camera			= graphic_view->GetCamera ();

	doc->Set_Manual_FOV (false);
	doc->Set_Manul_Clip_Planes (false);

	graphic_view->Reset_FOV ();
	RenderObjClass *render_obj = doc->GetDisplayedObject ();
	if (render_obj != nullptr) {
		graphic_view->Reset_Camera_To_Display_Object (*render_obj);
	}

	//
	//	Update the clip plane controls
	//
	float znear = 0;
	float zfar = 0;
	camera->Get_Clip_Planes (znear, zfar);
	::SetDlgItemFloat (m_hWnd, IDC_NEAR_CLIP_EDIT, znear);
	::SetDlgItemFloat (m_hWnd, IDC_FAR_CLIP_EDIT, zfar);

	//
	//	Update the FOV controls
	//
	int hfov_deg = (int)RAD_TO_DEG (camera->Get_Horizontal_FOV ());
	int vfov_deg = (int)RAD_TO_DEG (camera->Get_Vertical_FOV ());
	::SetDlgItemFloat (m_hWnd, IDC_HFOV_EDIT, hfov_deg);
	::SetDlgItemFloat (m_hWnd, IDC_VFOV_EDIT, vfov_deg);

	//
	//	Setup the camera lens controls
	//
	float vfov = camera->Get_Vertical_FOV ();
	float lens = ((::atan ((18.0F / 1000.0F)) / vfov) * 2.0F) * 1000.0F;
	::SetDlgItemFloat (m_hWnd, IDC_LENS_EDIT, lens);
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnNotify
//
////////////////////////////////////////////////////////////////////
BOOL
CameraSettingsDialogClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
)
{
	//
	//	Update the spinner control if necessary
	//
	NMHDR *header = (NMHDR *)lParam;
	if ((header != nullptr) && (header->code == UDN_DELTAPOS)) {
		LPNMUPDOWN updown_info = (LPNMUPDOWN)lParam;
		::Update_Spinner_Buddy (header->hwndFrom, updown_info->iDelta);

		//
		//	Update the FOV settings (they are dependent on each other)
		//
		if (updown_info->hdr.idFrom == IDC_LENS_SPIN) {
			Update_FOV ();
		} else if (updown_info->hdr.idFrom == IDC_HFOV_SPIN) {
			Update_Camera_Lens ();
		}
	}

	// Allow the base class to process this message
	return CDialog::OnNotify (wParam, lParam, pResult);
}


////////////////////////////////////////////////////////////////////
//
//	Update_Camera_Lens
//
////////////////////////////////////////////////////////////////////
void
CameraSettingsDialogClass::Update_Camera_Lens (void)
{
	//
	//	Get the current vertical FOV settings
	//
	float hfov = ::GetDlgItemFloat (m_hWnd, IDC_HFOV_EDIT);

	//
	//	Convert the vertical FOV to a camera lens setting
	//
	if (hfov > 0) {
		const float constant	= (18.0F / 1000.0F);
		float lens				= (constant / (::tan (DEG_TO_RAD (hfov) / 2))) * 1000.0F;
		::SetDlgItemFloat (m_hWnd, IDC_LENS_EDIT, lens);
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Update_FOV
//
////////////////////////////////////////////////////////////////////
void
CameraSettingsDialogClass::Update_FOV (void)
{
	//
	//	Get the current camera lens setting
	//
	float lens = (::GetDlgItemFloat (m_hWnd, IDC_LENS_EDIT) / 1000.0F);

	//
	//	Convert the camera lens to a FOV
	//
	if (lens > 0) {
		const float constant	= (18.0F / 1000.0F);
		float hfov				= (::atan (constant / lens) * 2.0F);
		float vfov				= (3 * hfov / 4);

		//
		//	Pass the new FOV settings onto the dialog
		//
		::SetDlgItemFloat (m_hWnd, IDC_HFOV_EDIT, RAD_TO_DEG (hfov));
		::SetDlgItemFloat (m_hWnd, IDC_VFOV_EDIT, RAD_TO_DEG (vfov));
	}
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Update_FOV
//
////////////////////////////////////////////////////////////////////
BOOL
CameraSettingsDialogClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	static bool updating = false;
	if (updating == false) {

		//
		//	Update the FOV settings if necessary
		//
		if (	LOWORD (wParam) == IDC_HFOV_EDIT &&
				HIWORD (wParam) == EN_UPDATE)
		{
			updating = true;
			Update_Camera_Lens ();
			updating = false;
		} else if (	LOWORD (wParam) == IDC_LENS_EDIT &&
						HIWORD (wParam) == EN_UPDATE)
		{
			updating = true;
			Update_FOV ();
			updating = false;
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}
