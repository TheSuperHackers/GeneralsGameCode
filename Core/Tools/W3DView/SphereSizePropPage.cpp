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

// SphereSizePropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "SphereSizePropPage.h"
#include "ColorUtils.h"
#include "Utils.h"
#include "ScaleDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(SphereSizePropPageClass, CPropertyPage)


/////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////
static bool Is_LERP (float last_value, float last_time, float curr_value, float curr_time, float next_value, float next_time);


/////////////////////////////////////////////////////////////
//
//	SphereSizePropPageClass
//
/////////////////////////////////////////////////////////////
SphereSizePropPageClass::SphereSizePropPageClass (SphereRenderObjClass *sphere)
	:	m_RenderObj (sphere),
		m_bValid (true),
		m_ScaleXBar (nullptr),
		m_ScaleYBar (nullptr),
		m_ScaleZBar (nullptr),
		m_Size (0.5F, 0.5F, 0.5F),
		CPropertyPage(SphereSizePropPageClass::IDD)
{
	//{{AFX_DATA_INIT(SphereSizePropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	~SphereSizePropPageClass
//
/////////////////////////////////////////////////////////////
SphereSizePropPageClass::~SphereSizePropPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
/////////////////////////////////////////////////////////////
void
SphereSizePropPageClass::DoDataExchange (CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SphereSizePropPageClass)
	DDX_Control(pDX, IDC_SIZE_Z_SPIN, m_SizeZSpin);
	DDX_Control(pDX, IDC_SIZE_Y_SPIN, m_SizeYSpin);
	DDX_Control(pDX, IDC_SIZE_X_SPIN, m_SizeXSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(SphereSizePropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(SphereSizePropPageClass)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
SphereSizePropPageClass::Initialize (void)
{
	m_ScaleChannel.Reset ();
	m_OrigScaleChannel.Reset ();

	if (m_RenderObj != nullptr) {
		m_Size					= m_RenderObj->Get_Box ().Extent;
		m_ScaleChannel			= m_RenderObj->Get_Scale_Channel ();
		m_OrigScaleChannel	= m_RenderObj->Get_Scale_Channel ();

		if (m_OrigScaleChannel.Get_Key_Count () == 0) {
			m_ScaleChannel.Add_Key (m_RenderObj->Get_Scale (), 0);
			m_OrigScaleChannel.Add_Key (m_RenderObj->Get_Scale (), 0);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
/////////////////////////////////////////////////////////////
BOOL
SphereSizePropPageClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();

	m_ScaleXBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_SCALE_BAR_X));
	m_ScaleYBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_SCALE_BAR_Y));
	m_ScaleZBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_SCALE_BAR_Z));

	//
	//	Setup the spinners
	//
	::Initialize_Spinner (m_SizeXSpin, m_Size.X, 0, 10000);
	::Initialize_Spinner (m_SizeYSpin, m_Size.Y, 0, 10000);
	::Initialize_Spinner (m_SizeZSpin, m_Size.Z, 0, 10000);

	//
	// Setup the timelines
	//
	m_ScaleXBar->Set_Range (0, 1);
	m_ScaleYBar->Set_Range (0, 1);
	m_ScaleZBar->Set_Range (0, 1);

	//
	//	Insert the starting points
	//
	m_ScaleXBar->Modify_Point (0, 0, 0, 0, 0);
	m_ScaleYBar->Modify_Point (0, 0, 0, 0, 0);
	m_ScaleZBar->Modify_Point (0, 0, 0, 0, 0);
	m_ScaleXBar->Set_Graph_Percent (0, m_OrigScaleChannel[0].Get_Value ().X);
	m_ScaleYBar->Set_Graph_Percent (0, m_OrigScaleChannel[0].Get_Value ().Y);
	m_ScaleZBar->Set_Graph_Percent (0, m_OrigScaleChannel[0].Get_Value ().Z);

	//
	//	Set-up the timelines
	//
	int x_index = 1;
	int y_index = 1;
	int z_index = 1;
	for (int index = 1; index < m_OrigScaleChannel.Get_Key_Count (); index ++) {
		const LERPAnimationChannelClass<Vector3>::KeyClass &prev_value = m_OrigScaleChannel.Get_Key (index - 1);
		const LERPAnimationChannelClass<Vector3>::KeyClass &curr_value = m_OrigScaleChannel.Get_Key (index);

		//
		//	Find out which channels are unique (we toss the others)
		//
		bool unique_x = false;
		bool unique_y = false;
		bool unique_z = false;
		if (index == (m_OrigScaleChannel.Get_Key_Count () - 1)) {
			unique_x = (curr_value.Get_Value ().X != prev_value.Get_Value ().X);
			unique_y = (curr_value.Get_Value ().Y != prev_value.Get_Value ().Y);
			unique_z = (curr_value.Get_Value ().Z != prev_value.Get_Value ().Z);
		} else {
			const LERPAnimationChannelClass<Vector3>::KeyClass &next_value = m_OrigScaleChannel[index + 1];

			//
			//	Check to ensure the X-value isn't just a LERP of the 2 adjacent keys.
			//
			unique_x = (::Is_LERP (	prev_value.Get_Value ().X,
											prev_value.Get_Time (),
											curr_value.Get_Value ().X,
											curr_value.Get_Time (),
											next_value.Get_Value ().X,
											next_value.Get_Time ()) == false);

			//
			//	Check to ensure the Y-value isn't just a LERP of the 2 adjacent keys.
			//
			unique_y = (::Is_LERP (	prev_value.Get_Value ().Y,
											prev_value.Get_Time (),
											curr_value.Get_Value ().Y,
											curr_value.Get_Time (),
											next_value.Get_Value ().Y,
											next_value.Get_Time ()) == false);

			//
			//	Check to ensure the Z-value isn't just a LERP of the 2 adjacent keys.
			//
			unique_z = (::Is_LERP (	prev_value.Get_Value ().Z,
											prev_value.Get_Time (),
											curr_value.Get_Value ().Z,
											curr_value.Get_Time (),
											next_value.Get_Value ().Z,
											next_value.Get_Time ()) == false);
		}

		//
		//	Insert a key for each unique axis
		//
		if (unique_x) {
			m_ScaleXBar->Modify_Point (x_index, curr_value.Get_Time (), 0, 0, 0);
			m_ScaleXBar->Set_Graph_Percent (x_index, curr_value.Get_Value ().X);
			x_index ++;
		}

		if (unique_y) {
			m_ScaleYBar->Modify_Point (y_index, curr_value.Get_Time (), 0, 0, 0);
			m_ScaleYBar->Set_Graph_Percent (y_index, curr_value.Get_Value ().Y);
			y_index ++;
		}

		if (unique_z) {
			m_ScaleZBar->Modify_Point (z_index, curr_value.Get_Time (), 0, 0, 0);
			m_ScaleZBar->Set_Graph_Percent (z_index, curr_value.Get_Value ().Z);
			z_index ++;
		}

		// One of the keys MUST be unique...
		ASSERT (unique_x || unique_y || unique_z);
	}

	//
	//	Ensure our initial 'current' values are up-to-date
	//
	Update_Scale_Array ();
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL
SphereSizePropPageClass::OnApply (void)
{
	// Allow the base class to process this message
	return CPropertyPage::OnApply ();
}


/////////////////////////////////////////////////////////////
//
//  OnDestroy
//
/////////////////////////////////////////////////////////////
void
SphereSizePropPageClass::OnDestroy (void)
{
	CPropertyPage::OnDestroy();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL
SphereSizePropPageClass::OnNotify
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
		case IDC_SCALE_BAR_X:
		case IDC_SCALE_BAR_Y:
		case IDC_SCALE_BAR_Z:
		{
			//
			//	Determine the timeline bar which sent the notification
			//
			ColorBarClass *timeline = nullptr;
			if (color_bar_hdr->hdr.idFrom == IDC_SCALE_BAR_X) {
				timeline = m_ScaleXBar;
			} else if (color_bar_hdr->hdr.idFrom == IDC_SCALE_BAR_Y) {
				timeline = m_ScaleYBar;
			} else {
				timeline = m_ScaleZBar;
			}

			bool update =	(color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
								(color_bar_hdr->hdr.code == CBRN_DELETED_POINT);

			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {

				//
				//	Allow the user to edit the keyframe
				//
				float scale = timeline->Get_Graph_Percent (color_bar_hdr->key_index);
				ScaleDialogClass dialog (scale, this);
				if (dialog.DoModal () == IDOK) {

					//
					//	Update the timeline
					//
					timeline->Set_Graph_Percent (color_bar_hdr->key_index, dialog.Get_Scale ());
					update = true;
				}
			}

			//
			//	Update the object
			//
			if (update) {
				Update_Scale_Array ();
				SetModified ();
			}
		}
		break;

		case IDC_SIZE_X_SPIN:
		case IDC_SIZE_Y_SPIN:
		case IDC_SIZE_Z_SPIN:
		{
			// Update the object
			m_Size.X = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_X_EDIT);
			m_Size.Y = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_Y_EDIT);
			m_Size.Z = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_Z_EDIT);
			m_RenderObj->Set_Extent (m_Size);
			SetModified ();
		}
		break;
	}

	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  OnCommand
//
/////////////////////////////////////////////////////////////
BOOL
SphereSizePropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_SIZE_X_EDIT:
		case IDC_SIZE_Y_EDIT:
		case IDC_SIZE_Z_EDIT:
		{
			// Update the object
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY))
			{
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				// Update the object
				m_Size.X = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_X_EDIT);
				m_Size.Y = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_Y_EDIT);
				m_Size.Z = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_Z_EDIT);
				m_RenderObj->Set_Extent (m_Size);
				SetModified ();
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;
	}

	return CPropertyPage::OnCommand (wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  OnCancel
//
/////////////////////////////////////////////////////////////
void
SphereSizePropPageClass::OnCancel (void)
{
	//
	//	Reset the object to its original state
	//
	m_RenderObj->Set_Scale_Channel (m_ScaleChannel);

	CPropertyPage::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Scale_Array
//
/////////////////////////////////////////////////////////////
void
SphereSizePropPageClass::Update_Scale_Array (void)
{
	m_ScaleChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;

	//
	//	Allocate arrays we can store the 3 separate timelines in
	//
	int max_x = m_ScaleXBar->Get_Point_Count ();
	int max_y = m_ScaleYBar->Get_Point_Count ();
	int max_z = m_ScaleZBar->Get_Point_Count ();
	LERPAnimationChannelClass<float> x_values;
	LERPAnimationChannelClass<float> y_values;
	LERPAnimationChannelClass<float> z_values;

	//
	//	Build the X-axis timline
	//
	int index;
	for (index = 0; index < max_x; index++) {
		m_ScaleXBar->Get_Point (index, &position, &red, &green, &blue);
		x_values.Add_Key (m_ScaleXBar->Get_Graph_Percent (index), position);
	}

	//
	//	Build the Y-axis timline
	//
	for (index = 0; index < max_y; index++) {
		m_ScaleYBar->Get_Point (index, &position, &red, &green, &blue);
		y_values.Add_Key (m_ScaleYBar->Get_Graph_Percent (index), position);
	}

	//
	//	Build the Z-axis timline
	//
	for (index = 0; index < max_z; index++) {
		m_ScaleZBar->Get_Point (index, &position, &red, &green, &blue);
		z_values.Add_Key (m_ScaleZBar->Get_Graph_Percent (index), position);
	}

	//
	//	Combine the 3 separate time lines into one time line
	//
	int x_index		= 0;
	int y_index		= 0;
	int z_index		= 0;
	int list_index	= 0;
	float x_val = x_values[0].Get_Value ();
	float y_val = y_values[0].Get_Value ();
	float z_val = z_values[0].Get_Value ();
	while (	x_index < max_x ||
				y_index < max_y ||
				z_index < max_z)
	{
		//
		//	Find the smallest time
		//
		float time = 2.0F;

		float x_time = time;
		float y_time = time;
		float z_time = time;

		if (x_index < max_x) {
			x_time = x_values[x_index].Get_Time ();
		}

		if (y_index < max_y) {
			y_time = y_values[y_index].Get_Time ();
		}

		if (z_index < max_z) {
			z_time = z_values[z_index].Get_Time ();
		}

		time = min (x_time, time);
		time = min (y_time, time);
		time = min (z_time, time);

		if (x_time == time) {
			x_index ++;
		}

		if (y_time == time) {
			y_index ++;
		}

		if (z_time == time) {
			z_index ++;
		}

		//
		// Evaluate X
		//
		x_val = x_values.Evaluate (time);
		y_val = y_values.Evaluate (time);
		z_val = z_values.Evaluate (time);

		//
		//	Add this scalar to the list
		//
		m_ScaleChannel.Add_Key (Vector3 (x_val, y_val, z_val), time);
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Scale_Channel (m_ScaleChannel);
	m_RenderObj->Restart_Animation ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Is_LERP
//
/////////////////////////////////////////////////////////////
bool
Is_LERP
(
	float last_value,
	float last_time,
	float curr_value,
	float curr_time,
	float next_value,
	float next_time
)
{
	float percent					= (curr_time - last_time) / (next_time - last_time);
	float interpolated_value	= last_value + ((next_value-last_value) * percent);
	return bool(WWMath::Fabs (interpolated_value - curr_value) < WWMATH_EPSILON);
}
