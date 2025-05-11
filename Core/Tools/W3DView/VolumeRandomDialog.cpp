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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/VolumeRandomDialog.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "Stdafx.H"
#include "W3DView.H"
#include "VolumeRandomDialog.H"
#include "V3_Rnd.H"
#include "Utils.H"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


////////////////////////////////////////////////////////////////////
//
//	VolumeRandomDialogClass
//
////////////////////////////////////////////////////////////////////
VolumeRandomDialogClass::VolumeRandomDialogClass (Vector3Randomizer *randomizer, CWnd *pParent)
	:	m_Randomizer (randomizer),
		CDialog (VolumeRandomDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(VolumeRandomDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
////////////////////////////////////////////////////////////////////
void
VolumeRandomDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VolumeRandomDialogClass)
	DDX_Control(pDX, IDC_SPHERE_RADIUS_SPIN, m_SphereRadiusSpin);
	DDX_Control(pDX, IDC_CYLINDER_RADIUS_SPIN, m_CylinderRadiusSpin);
	DDX_Control(pDX, IDC_CYLINDER_HEIGHT_SPIN, m_CylinderHeightSpin);
	DDX_Control(pDX, IDC_BOX_Z_SPIN, m_BoxZSpin);
	DDX_Control(pDX, IDC_BOX_Y_SPIN, m_BoxYSpin);
	DDX_Control(pDX, IDC_BOX_X_SPIN, m_BoxXSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(VolumeRandomDialogClass, CDialog)
	//{{AFX_MSG_MAP(VolumeRandomDialogClass)
	ON_BN_CLICKED(IDC_BOX_RADIO, OnBoxRadio)
	ON_BN_CLICKED(IDC_CYLINDER_RADIO, OnCylinderRadio)
	ON_BN_CLICKED(IDC_SPHERE_RADIO, OnSphereRadio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////
//
//	OnOK
//
////////////////////////////////////////////////////////////////////
void
VolumeRandomDialogClass::OnOK (void)
{
	if (SendDlgItemMessage (IDC_BOX_RADIO, BM_GETCHECK) == 1) {
		
		//
		//	Create a box randomizer
		//
		Vector3 extents (0, 0, 0);
		extents.X = ::GetDlgItemFloat (m_hWnd, IDC_BOX_X_EDIT);
		extents.Y = ::GetDlgItemFloat (m_hWnd, IDC_BOX_Y_EDIT);
		extents.Z = ::GetDlgItemFloat (m_hWnd, IDC_BOX_Z_EDIT);
		m_Randomizer = new Vector3SolidBoxRandomizer (extents);
	} else if (SendDlgItemMessage (IDC_SPHERE_RADIO, BM_GETCHECK) == 1) {
		
		//
		//	What type of sphere is this, hollow or solid?
		//
		float radius = ::GetDlgItemFloat (m_hWnd, IDC_SPHERE_RADIUS_EDIT);
		if (SendDlgItemMessage (IDC_SPHERE_HOLLOW_CHECK, BM_GETCHECK) == 1) {
			m_Randomizer = new Vector3HollowSphereRandomizer (radius);
		} else {
			m_Randomizer = new Vector3SolidSphereRandomizer (radius);
		}
	} else if (SendDlgItemMessage (IDC_CYLINDER_RADIO, BM_GETCHECK) == 1) {
		
		//
		//	Create a cylinder randomizer
		//
		float radius = ::GetDlgItemFloat (m_hWnd, IDC_CYLINDER_RADIUS_EDIT);
		float height = ::GetDlgItemFloat (m_hWnd, IDC_CYLINDER_HEIGHT_EDIT);
		m_Randomizer = new Vector3SolidCylinderRandomizer (height, radius);
	}
	
	CDialog::OnOK ();
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
////////////////////////////////////////////////////////////////////
BOOL
VolumeRandomDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Start with some default values
	//
	Vector3 initial_box (1, 1, 1);
	float initial_sphere_radius = 1.0F;
	bool initial_sphere_hollow = false;
	float initial_cylinder_radius = 1.0F;
	float initial_cylinder_height = 1.0F;
	UINT initial_type = IDC_BOX_RADIO;

	//
	//	Initialize from the provided randomizer
	//
	if (m_Randomizer != NULL) {
		
		// What type of randomizer is this?
		switch (m_Randomizer->Class_ID ())
		{		
			case Vector3Randomizer::CLASSID_SOLIDBOX:
				initial_type = IDC_BOX_RADIO;
				initial_box = ((Vector3SolidBoxRandomizer *)m_Randomizer)->Get_Extents ();
				break;

			case Vector3Randomizer::CLASSID_SOLIDSPHERE:
				initial_type = IDC_SPHERE_RADIO;
				initial_sphere_radius = ((Vector3SolidSphereRandomizer *)m_Randomizer)->Get_Radius();
				initial_sphere_hollow = false;
				break;

			case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
				initial_type = IDC_SPHERE_RADIO;
				initial_sphere_radius = ((Vector3HollowSphereRandomizer *)m_Randomizer)->Get_Radius ();
				initial_sphere_hollow = true;
				break;
			
			case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
				initial_type = IDC_CYLINDER_RADIO;
				initial_cylinder_radius = ((Vector3SolidCylinderRandomizer *)m_Randomizer)->Get_Radius ();
				initial_cylinder_height = ((Vector3SolidCylinderRandomizer *)m_Randomizer)->Get_Height ();
				break;
			
			default:
				ASSERT (0);
				break;
		}
	}

	//
	//	Initialize the box controls
	//
	::Initialize_Spinner (m_BoxXSpin, initial_box.X, -10000, 10000);
	::Initialize_Spinner (m_BoxYSpin, initial_box.Y, -10000, 10000);
	::Initialize_Spinner (m_BoxZSpin, initial_box.Z, -10000, 10000);

	//
	//	Initialize the sphere controls
	//
	::Initialize_Spinner (m_SphereRadiusSpin, initial_sphere_radius, 0, 10000);
	SendDlgItemMessage (IDC_SPHERE_HOLLOW_CHECK, BM_SETCHECK, (WPARAM)initial_sphere_hollow);

	//
	//	Initialize the cylinder controls
	//
	::Initialize_Spinner (m_CylinderRadiusSpin, initial_cylinder_radius, 0, 10000);
	::Initialize_Spinner (m_CylinderHeightSpin, initial_cylinder_height, 0, 10000);

	//
	//	Check the appropriate radio
	//
	SendDlgItemMessage (initial_type, BM_SETCHECK, (WPARAM)TRUE);
	Update_Enable_State ();
	return TRUE;
}


////////////////////////////////////////////////////////////////////
//
//	OnBoxRadio
//
////////////////////////////////////////////////////////////////////
void
VolumeRandomDialogClass::OnBoxRadio (void)
{
	Update_Enable_State ();
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnCylinderRadio
//
////////////////////////////////////////////////////////////////////
void
VolumeRandomDialogClass::OnCylinderRadio (void) 
{
	Update_Enable_State ();
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnSphereRadio
//
////////////////////////////////////////////////////////////////////
void
VolumeRandomDialogClass::OnSphereRadio (void) 
{
	Update_Enable_State ();
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Update_Enable_State
//
////////////////////////////////////////////////////////////////////
void
VolumeRandomDialogClass::Update_Enable_State (void) 
{
	bool enable_box_ctrls = (SendDlgItemMessage (IDC_BOX_RADIO, BM_GETCHECK) == 1);
	bool enable_sphere_ctrls = (SendDlgItemMessage (IDC_SPHERE_RADIO, BM_GETCHECK) == 1);
	bool enable_cylinder_ctrls = (SendDlgItemMessage (IDC_CYLINDER_RADIO, BM_GETCHECK) == 1);

	//
	//	Update the box controls
	//
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_BOX_X_EDIT), enable_box_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_BOX_Y_EDIT), enable_box_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_BOX_Z_EDIT), enable_box_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_BOX_X_SPIN), enable_box_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_BOX_Y_SPIN), enable_box_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_BOX_Z_SPIN), enable_box_ctrls);

	//
	//	Update the sphere controls
	//
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_SPHERE_RADIUS_EDIT), enable_sphere_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_SPHERE_RADIUS_SPIN), enable_sphere_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_SPHERE_HOLLOW_CHECK), enable_sphere_ctrls);	

	//
	//	Update the cylinder controls
	//
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_CYLINDER_RADIUS_EDIT), enable_cylinder_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_CYLINDER_RADIUS_SPIN), enable_cylinder_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_CYLINDER_HEIGHT_EDIT), enable_cylinder_ctrls);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_CYLINDER_HEIGHT_SPIN), enable_cylinder_ctrls);
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnNotify
//
////////////////////////////////////////////////////////////////////
BOOL
VolumeRandomDialogClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
) 
{
	//
	//	Update the spinner control if necessary
	//
	NMHDR *pheader = (NMHDR *)lParam;
	if ((pheader != NULL) && (pheader->code == UDN_DELTAPOS)) {
		LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;		
		::Update_Spinner_Buddy (pheader->hwndFrom, pupdown->iDelta);
	}
	
	// Allow the base class to process this message
	return CDialog::OnNotify (wParam, lParam, pResult);
}
