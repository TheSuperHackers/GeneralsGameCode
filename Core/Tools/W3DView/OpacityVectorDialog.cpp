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

// OpacityVectorDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "OpacityVectorDialog.h"
#include "wwmath.h"
#include "vector3.h"
#include "sphereobj.h"
#include "ringobj.h"
#include "ColorBar.h"
#include "euler.h"
#include "matrix3.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// OpacityVectorDialogClass
//
/////////////////////////////////////////////////////////////////////////////
OpacityVectorDialogClass::OpacityVectorDialogClass(CWnd* pParent /*=nullptr*/)
	:	m_OpacityBar (nullptr),
		m_RenderObj (nullptr),
		m_KeyIndex (0),
		CDialog(OpacityVectorDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(OpacityVectorDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
OpacityVectorDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OpacityVectorDialogClass)
	DDX_Control(pDX, IDC_SLIDER_Z, m_SliderZ);
	DDX_Control(pDX, IDC_SLIDER_Y, m_SliderY);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(OpacityVectorDialogClass, CDialog)
	//{{AFX_MSG_MAP(OpacityVectorDialogClass)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
OpacityVectorDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();

	m_OpacityBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_OPACITY_BAR));
	ASSERT (m_OpacityBar);

	//
	// Setup the opacity bar
	//
	m_OpacityBar->Set_Range (0, 10);
	m_OpacityBar->Modify_Point (0, 0, 255, 255, 255);
	m_OpacityBar->Insert_Point (1, 10, 0, 0, 0);

	float value =  ::atan (((m_Value.intensity / 10.0F) * 11.0F)) / DEG_TO_RAD (84.5) * 10.0F;
	m_OpacityBar->Set_Selection_Pos (value);

	//
	//	Setup the sliders
	//
	m_SliderY.SetRange (0, 179);
	m_SliderZ.SetRange (0, 179);

	float log_test = ::log (8.0F);
	float log_test2 = ::_logb (log_test);
	float log_test3 = ::exp (log_test);

	//
	//	Convert the normalized vector to Euler angles...
	//


	/*Vector3 dir_vector (m_Value.X, m_Value.Y, m_Value.Z);

	float x_rot	= ::acos (dir_vector * Vector3 (1, 0, 0));
	float y_rot	= ::acos (dir_vector * Vector3 (0, 1, 0));
	float z_rot	= ::acos (dir_vector * Vector3 (0, 0, 1));
	x_rot			= RAD_TO_DEG (x_rot);
	y_rot			= RAD_TO_DEG (y_rot);
	z_rot			= RAD_TO_DEG (z_rot);
	x_rot			= WWMath::Wrap (x_rot, -180, 180);
	y_rot			= WWMath::Wrap (y_rot, -180, 180);
	z_rot			= WWMath::Wrap (z_rot, -180, 180);*/

	/*Matrix3D rot_90 (1);
	rot_90.Rotate_Z (DEG_TO_RADF (90));

	Vector3 x_axis (m_Value.X, m_Value.Y, m_Value.Z);
	x_axis.Normalize ();

	Vector3 y_axis = rot_90 * x_axis;
	Vector3 z_axis;
	Vector3::Cross_Product (x_axis, y_axis, &z_axis);
	Matrix3D orientation (x_axis, y_axis, z_axis, Vector3 (0, 0, 0));

	EulerAnglesClass euler_angle (orientation, EulerOrderXYZr);
	float x_rot = RAD_TO_DEG (euler_angle.Get_Angle (0));
	float y_rot = RAD_TO_DEG (euler_angle.Get_Angle (1));
	float z_rot = RAD_TO_DEG (euler_angle.Get_Angle (2));

	x_rot = WWMath::Wrap (x_rot, 0, 360);
	y_rot = WWMath::Wrap (y_rot, 0, 360);
	z_rot = WWMath::Wrap (z_rot, 0, 360);*/


#ifdef ALLOW_TEMPORARIES
	Matrix3D rotation = Build_Matrix3D (m_Value.angle);
#else
	Matrix3D rotation;
	Build_Matrix3D (m_Value.angle, rotation);
#endif
	//Vector3 point = m_Value.angle.Rotate_Vector (Vector3 (1, 0, 0));

	EulerAnglesClass euler_angle (rotation, EulerOrderXYZr);
	//float x_rot = RAD_TO_DEG (euler_angle.Get_Angle (0));
	float y_rot = RAD_TO_DEG (euler_angle.Get_Angle (1));
	float z_rot = RAD_TO_DEG (euler_angle.Get_Angle (2));

	//float y_rot = RAD_TO_DEG (rotation.Get_Y_Rotation ());
	//float z_rot = RAD_TO_DEG (rotation.Get_Z_Rotation ());

	y_rot = WWMath::Wrap (y_rot, 0, 360);
	z_rot = WWMath::Wrap (z_rot, 0, 360);

	m_SliderY.SetPos ((int)y_rot);
	m_SliderZ.SetPos ((int)z_rot);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
OpacityVectorDialogClass::OnOK (void)
{
	m_Value = Update_Value ();
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Object
//
/////////////////////////////////////////////////////////////////////////////
void
OpacityVectorDialogClass::Update_Object (void)
{
	Update_Object (Update_Value ());
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Value
//
/////////////////////////////////////////////////////////////////////////////
AlphaVectorStruct
OpacityVectorDialogClass::Update_Value (void)
{
	AlphaVectorStruct value;

	int y_pos = m_SliderY.GetPos ();
	int z_pos = m_SliderZ.GetPos ();

	//float x_rot = DEG_TO_RADF ((float)x_pos);
	float y_rot = DEG_TO_RADF ((float)y_pos);
	float z_rot = DEG_TO_RADF ((float)z_pos);

	Matrix3x3 rot_mat (true);
	//rot_mat.Rotate_X (x_rot);
	rot_mat.Rotate_Y (y_rot);
	rot_mat.Rotate_Z (z_rot);

	value.angle = ::Build_Quaternion (rot_mat);

	float percent = ::tan ((m_OpacityBar->Get_Selection_Pos () / 10.0F) * DEG_TO_RAD (84.5)) / 11.0F;
	percent = min (1.0F, percent);
	percent = max (0.0F, percent);

	value.intensity = 10.0F * percent;
	return value;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Object
//
/////////////////////////////////////////////////////////////////////////////
void
OpacityVectorDialogClass::Update_Object (const AlphaVectorStruct &value)
{
	if (m_RenderObj != nullptr) {

		//
		//	Determine what type of object this is
		//
		switch (m_RenderObj->Class_ID ())
		{
			case RenderObjClass::CLASSID_SPHERE:
			{
				//
				//	Update the key with the new vector
				//

				SphereVectorChannelClass &vector_channel = ((SphereRenderObjClass *)m_RenderObj)->Get_Vector_Channel ();
				vector_channel.Set_Key_Value (m_KeyIndex, value);

				//
				//	Force the animation to restart
				//
				((SphereRenderObjClass *)m_RenderObj)->Restart_Animation ();
			}
			break;
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
/////////////////////////////////////////////////////////////////////////////
void
OpacityVectorDialogClass::OnCancel (void)
{
	Update_Object (m_Value);
	CDialog::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnHScroll
//
/////////////////////////////////////////////////////////////////////////////
void
OpacityVectorDialogClass::OnHScroll
(
	UINT				nSBCode,
	UINT				nPos,
	CScrollBar *	pScrollBar
)
{
	//
	//	Update the object
	//
	Update_Object ();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL
OpacityVectorDialogClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
)
{
	CBR_NMHDR *color_bar_hdr = (CBR_NMHDR *)lParam;

	//
	//	Which control sent the notification?
	//
	switch (color_bar_hdr->hdr.idFrom)
	{
		case IDC_OPACITY_BAR:
		{
			//
			// Update the object
			//
			if (color_bar_hdr->hdr.code == CBRN_SEL_CHANGED) {
				Update_Object ();
			}
		}
		break;
	}

	return CDialog::OnNotify (wParam, lParam, pResult);
}
