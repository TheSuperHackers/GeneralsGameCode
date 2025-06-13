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

// RingSizePropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "RingSizePropPage.h"
#include "ColorUtils.h"
#include "Utils.h"
#include "ScaleDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(RingSizePropPageClass, CPropertyPage)


/////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////
static bool Is_LERP (float last_value, float last_time, float curr_value, float curr_time, float next_value, float next_time);


/////////////////////////////////////////////////////////////
//
//	RingSizePropPageClass
//
/////////////////////////////////////////////////////////////
RingSizePropPageClass::RingSizePropPageClass (RingRenderObjClass *ring)
	:	m_RenderObj (ring),
		m_bValid (true),
		m_InnerScaleXBar (NULL),
		m_InnerScaleYBar (NULL),
		m_OuterScaleXBar (NULL),
		m_OuterScaleYBar (NULL),
		m_InnerSize (0.5F, 0.5F),
		m_OuterSize (1.0F, 1.0F),
		CPropertyPage(RingSizePropPageClass::IDD)
{
	//{{AFX_DATA_INIT(RingSizePropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	~RingSizePropPageClass
//
/////////////////////////////////////////////////////////////
RingSizePropPageClass::~RingSizePropPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
/////////////////////////////////////////////////////////////
void
RingSizePropPageClass::DoDataExchange (CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RingSizePropPageClass)
	DDX_Control(pDX, IDC_INNER_SIZE_X_SPIN, m_InnerSizeXSpin);
	DDX_Control(pDX, IDC_INNER_SIZE_Y_SPIN, m_InnerSizeYSpin);
	DDX_Control(pDX, IDC_OUTER_SIZE_X_SPIN, m_OuterSizeXSpin);
	DDX_Control(pDX, IDC_OUTER_SIZE_Y_SPIN, m_OuterSizeYSpin);	
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(RingSizePropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(RingSizePropPageClass)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
RingSizePropPageClass::Initialize (void)
{
	m_InnerScaleChannel.Reset ();
	m_OrigInnerScaleChannel.Reset ();
	m_OuterScaleChannel.Reset ();
	m_OrigOuterScaleChannel.Reset ();

	if (m_RenderObj != NULL) {
		m_InnerSize		= m_RenderObj->Get_Inner_Extent ();
		m_OuterSize		= m_RenderObj->Get_Outer_Extent ();

		m_InnerScaleChannel		= m_RenderObj->Get_Inner_Scale_Channel ();
		m_OrigInnerScaleChannel	= m_RenderObj->Get_Inner_Scale_Channel ();

		m_OuterScaleChannel		= m_RenderObj->Get_Outer_Scale_Channel ();
		m_OrigOuterScaleChannel	= m_RenderObj->Get_Outer_Scale_Channel ();

		if (m_OrigInnerScaleChannel.Get_Key_Count () == 0) {
			m_InnerScaleChannel.Add_Key (m_RenderObj->Get_Inner_Scale (), 0);
			m_OrigInnerScaleChannel.Add_Key (m_RenderObj->Get_Inner_Scale (), 0);
		}

		if (m_OrigOuterScaleChannel.Get_Key_Count () == 0) {
			m_OuterScaleChannel.Add_Key (m_RenderObj->Get_Outer_Scale (), 0);
			m_OrigOuterScaleChannel.Add_Key (m_RenderObj->Get_Outer_Scale (), 0);
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
RingSizePropPageClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();
	
	m_InnerScaleXBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_INNER_SCALE_BAR_X));
	m_InnerScaleYBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_INNER_SCALE_BAR_Y));
	m_OuterScaleXBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_OUTER_SCALE_BAR_X));
	m_OuterScaleYBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_OUTER_SCALE_BAR_Y));

	//
	//	Setup the spinners
	//
	::Initialize_Spinner (m_InnerSizeXSpin, m_InnerSize.X, 0, 10000);
	::Initialize_Spinner (m_InnerSizeYSpin, m_InnerSize.Y, 0, 10000);

	::Initialize_Spinner (m_OuterSizeXSpin, m_OuterSize.X, 0, 10000);
	::Initialize_Spinner (m_OuterSizeYSpin, m_OuterSize.Y, 0, 10000);


	//
	// Setup the timelines
	//
	m_InnerScaleXBar->Set_Range (0, 1);
	m_InnerScaleYBar->Set_Range (0, 1);
	m_OuterScaleXBar->Set_Range (0, 1);
	m_OuterScaleYBar->Set_Range (0, 1);

	//
	//	Insert the starting points
	//
	m_InnerScaleXBar->Modify_Point (0, 0, 0, 0, 0);
	m_InnerScaleYBar->Modify_Point (0, 0, 0, 0, 0);
	m_OuterScaleXBar->Modify_Point (0, 0, 0, 0, 0);
	m_OuterScaleYBar->Modify_Point (0, 0, 0, 0, 0);
	m_InnerScaleXBar->Set_Graph_Percent (0, m_OrigInnerScaleChannel[0].Get_Value ().X);
	m_InnerScaleYBar->Set_Graph_Percent (0, m_OrigInnerScaleChannel[0].Get_Value ().Y);
	m_OuterScaleXBar->Set_Graph_Percent (0, m_OrigOuterScaleChannel[0].Get_Value ().X);
	m_OuterScaleYBar->Set_Graph_Percent (0, m_OrigOuterScaleChannel[0].Get_Value ().Y);

	//
	//	Set-up the inner-scale timelines
	//
	int x_index = 1;
	int y_index = 1;
	int index;
	for (index = 1; index < m_OrigInnerScaleChannel.Get_Key_Count (); index ++) {
		const LERPAnimationChannelClass<Vector2>::KeyClass &prev_value = m_OrigInnerScaleChannel.Get_Key (index - 1);
		const LERPAnimationChannelClass<Vector2>::KeyClass &curr_value = m_OrigInnerScaleChannel.Get_Key (index);
		
		//
		//	Find out which channels are unique (we toss the others)
		//
		bool unique_x = false;
		bool unique_y = false;
		if (index == (m_OrigInnerScaleChannel.Get_Key_Count () - 1)) {
			unique_x = (curr_value.Get_Value ().X != prev_value.Get_Value ().X);
			unique_y = (curr_value.Get_Value ().Y != prev_value.Get_Value ().Y);
		} else {
			const LERPAnimationChannelClass<Vector2>::KeyClass &next_value = m_OrigInnerScaleChannel[index + 1];

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
		}

		//
		//	Insert a key for each unique axis
		//
		if (unique_x) {
			m_InnerScaleXBar->Modify_Point (x_index, curr_value.Get_Time (), 0, 0, 0);
			m_InnerScaleXBar->Set_Graph_Percent (x_index, curr_value.Get_Value ().X);
			x_index ++;
		}

		if (unique_y) {
			m_InnerScaleYBar->Modify_Point (y_index, curr_value.Get_Time (), 0, 0, 0);
			m_InnerScaleYBar->Set_Graph_Percent (y_index, curr_value.Get_Value ().Y);
			y_index ++;
		}

		// One of the keys MUST be unique...
		ASSERT (unique_x || unique_y);
	}

	//
	//	Set-up the outer-scale timelines
	//
	x_index = 1;
	y_index = 1;
	for (index = 1; index < m_OrigOuterScaleChannel.Get_Key_Count (); index ++) {
		const LERPAnimationChannelClass<Vector2>::KeyClass &prev_value = m_OrigOuterScaleChannel.Get_Key (index - 1);
		const LERPAnimationChannelClass<Vector2>::KeyClass &curr_value = m_OrigOuterScaleChannel.Get_Key (index);
		
		//
		//	Find out which channels are unique (we toss the others)
		//
		bool unique_x = false;
		bool unique_y = false;
		if (index == (m_OrigOuterScaleChannel.Get_Key_Count () - 1)) {
			unique_x = (curr_value.Get_Value ().X != prev_value.Get_Value ().X);
			unique_y = (curr_value.Get_Value ().Y != prev_value.Get_Value ().Y);
		} else {
			const LERPAnimationChannelClass<Vector2>::KeyClass &next_value = m_OrigOuterScaleChannel[index + 1];

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
		}

		//
		//	Insert a key for each unique axis
		//
		if (unique_x) {
			m_OuterScaleXBar->Modify_Point (x_index, curr_value.Get_Time (), 0, 0, 0);
			m_OuterScaleXBar->Set_Graph_Percent (x_index, curr_value.Get_Value ().X);
			x_index ++;
		}

		if (unique_y) {
			m_OuterScaleYBar->Modify_Point (y_index, curr_value.Get_Time (), 0, 0, 0);
			m_OuterScaleYBar->Set_Graph_Percent (y_index, curr_value.Get_Value ().Y);
			y_index ++;
		}

		// One of the keys MUST be unique...
		ASSERT (unique_x || unique_y);
	}

	//
	//	Ensure our initial 'current' values are up-to-date
	//
	Update_Inner_Scale_Array ();
	Update_Outer_Scale_Array ();
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL
RingSizePropPageClass::OnApply (void)
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
RingSizePropPageClass::OnDestroy (void)
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
RingSizePropPageClass::OnNotify
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
		case IDC_INNER_SCALE_BAR_X:
		case IDC_INNER_SCALE_BAR_Y:
		case IDC_OUTER_SCALE_BAR_X:
		case IDC_OUTER_SCALE_BAR_Y:
		{			
			//
			//	Determine the timeline bar which sent the notification
			//
			ColorBarClass *timeline = NULL;
			if (color_bar_hdr->hdr.idFrom == IDC_INNER_SCALE_BAR_X) {
				timeline = m_InnerScaleXBar;
			} else if (color_bar_hdr->hdr.idFrom == IDC_INNER_SCALE_BAR_Y) {
				timeline = m_InnerScaleYBar;
			} else if (color_bar_hdr->hdr.idFrom == IDC_OUTER_SCALE_BAR_X) {
				timeline = m_OuterScaleXBar;
			} else if (color_bar_hdr->hdr.idFrom == IDC_OUTER_SCALE_BAR_Y) {
				timeline = m_OuterScaleYBar;
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

				if (	color_bar_hdr->hdr.idFrom == IDC_INNER_SCALE_BAR_X ||
						color_bar_hdr->hdr.idFrom == IDC_INNER_SCALE_BAR_Y)
				{
					Update_Inner_Scale_Array ();
				} else if (	color_bar_hdr->hdr.idFrom == IDC_OUTER_SCALE_BAR_X ||
								color_bar_hdr->hdr.idFrom == IDC_OUTER_SCALE_BAR_Y)
				{
					Update_Outer_Scale_Array ();
				}

				SetModified ();
			}
		}
		break;

		case IDC_INNER_SIZE_X_SPIN:
		case IDC_INNER_SIZE_Y_SPIN:
		{
			// Update the object
			m_InnerSize.X = ::GetDlgItemFloat (m_hWnd, IDC_INNER_SIZE_X_EDIT);
			m_InnerSize.Y = ::GetDlgItemFloat (m_hWnd, IDC_INNER_SIZE_Y_EDIT);
			m_RenderObj->Set_Inner_Extent (m_InnerSize);
			SetModified ();
		}
		break;

		case IDC_OUTER_SIZE_X_SPIN:
		case IDC_OUTER_SIZE_Y_SPIN:
		{
			// Update the object
			m_OuterSize.X = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_SIZE_X_EDIT);
			m_OuterSize.Y = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_SIZE_Y_EDIT);
			m_RenderObj->Set_Outer_Extent (m_OuterSize);
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
RingSizePropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_INNER_SIZE_X_EDIT:
		case IDC_INNER_SIZE_Y_EDIT:
		{
			// Update the object
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY))
			{
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				// Update the object
				m_InnerSize.X = ::GetDlgItemFloat (m_hWnd, IDC_INNER_SIZE_X_EDIT);
				m_InnerSize.Y = ::GetDlgItemFloat (m_hWnd, IDC_INNER_SIZE_Y_EDIT);
				m_RenderObj->Set_Inner_Extent (m_InnerSize);
				SetModified ();
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;

		case IDC_OUTER_SIZE_X_EDIT:
		case IDC_OUTER_SIZE_Y_EDIT:
		{
			// Update the object
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY))
			{
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				// Update the object
				m_OuterSize.X = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_SIZE_X_EDIT);
				m_OuterSize.Y = ::GetDlgItemFloat (m_hWnd, IDC_OUTER_SIZE_Y_EDIT);
				m_RenderObj->Set_Outer_Extent (m_OuterSize);
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
RingSizePropPageClass::OnCancel (void)
{
	//
	//	Reset the object to its original state
	//
	m_RenderObj->Set_Inner_Scale_Channel (m_OrigInnerScaleChannel);
	m_RenderObj->Set_Outer_Scale_Channel (m_OrigOuterScaleChannel);

	CPropertyPage::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Inner_Scale_Array
//
/////////////////////////////////////////////////////////////
void
RingSizePropPageClass::Update_Inner_Scale_Array (void)
{
	m_InnerScaleChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;	

	//
	//	Allocate arrays we can store the 3 separate timelines in
	//
	int max_x = m_InnerScaleXBar->Get_Point_Count ();
	int max_y = m_InnerScaleYBar->Get_Point_Count ();
	LERPAnimationChannelClass<float> x_values;
	LERPAnimationChannelClass<float> y_values;
	
	//
	//	Build the X-axis timline
	//
	int index;
	for (index = 0; index < max_x; index++) {
		m_InnerScaleXBar->Get_Point (index, &position, &red, &green, &blue);		
		x_values.Add_Key (m_InnerScaleXBar->Get_Graph_Percent (index), position);
	}

	//
	//	Build the Y-axis timline
	//
	for (index = 0; index < max_y; index++) {		
		m_InnerScaleYBar->Get_Point (index, &position, &red, &green, &blue);		
		y_values.Add_Key (m_InnerScaleYBar->Get_Graph_Percent (index), position);
	}

	//
	//	Combine the 3 separate time lines into one time line
	//
	int x_index		= 0;
	int y_index		= 0;
	int list_index	= 0;
	float x_val = x_values[0].Get_Value ();
	float y_val = y_values[0].Get_Value ();
	while (	x_index < max_x ||
				y_index < max_y)
	{		
		//
		//	Find the smallest time
		//
		float time = 2.0F;

		float x_time = time;
		float y_time = time;

		if (x_index < max_x) {
			x_time = x_values[x_index].Get_Time ();
		}

		if (y_index < max_y) {
			y_time = y_values[y_index].Get_Time ();
		}

		time = min (x_time, time);
		time = min (y_time, time);

		if (x_time == time) {
			x_index ++;
		}

		if (y_time == time) {
			y_index ++;
		}

		//
		// Evaluate X
		//
		x_val = x_values.Evaluate (time);
		y_val = y_values.Evaluate (time);

		//
		//	Add this scalar to the list
		//
		m_InnerScaleChannel.Add_Key (Vector2 (x_val, y_val), time);
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Inner_Scale_Channel (m_InnerScaleChannel);
	m_RenderObj->Restart_Animation ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Outer_Scale_Array
//
/////////////////////////////////////////////////////////////
void
RingSizePropPageClass::Update_Outer_Scale_Array (void)
{
	m_OuterScaleChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;	

	//
	//	Allocate arrays we can store the 3 separate timelines in
	//
	int max_x = m_OuterScaleXBar->Get_Point_Count ();
	int max_y = m_OuterScaleYBar->Get_Point_Count ();
	LERPAnimationChannelClass<float> x_values;
	LERPAnimationChannelClass<float> y_values;
	
	//
	//	Build the X-axis timline
	//
	int index;
	for (index = 0; index < max_x; index++) {
		m_OuterScaleXBar->Get_Point (index, &position, &red, &green, &blue);		
		x_values.Add_Key (m_OuterScaleXBar->Get_Graph_Percent (index), position);
	}

	//
	//	Build the Y-axis timline
	//
	for (index = 0; index < max_y; index++) {		
		m_OuterScaleYBar->Get_Point (index, &position, &red, &green, &blue);		
		y_values.Add_Key (m_OuterScaleYBar->Get_Graph_Percent (index), position);
	}

	//
	//	Combine the 3 separate time lines into one time line
	//
	int x_index		= 0;
	int y_index		= 0;
	int list_index	= 0;
	float x_val = x_values[0].Get_Value ();
	float y_val = y_values[0].Get_Value ();
	while (	x_index < max_x ||
				y_index < max_y)
	{		
		//
		//	Find the smallest time
		//
		float time = 2.0F;

		float x_time = time;
		float y_time = time;

		if (x_index < max_x) {
			x_time = x_values[x_index].Get_Time ();
		}

		if (y_index < max_y) {
			y_time = y_values[y_index].Get_Time ();
		}

		time = min (x_time, time);
		time = min (y_time, time);

		if (x_time == time) {
			x_index ++;
		}

		if (y_time == time) {
			y_index ++;
		}

		//
		// Evaluate X
		//
		x_val = x_values.Evaluate (time);
		y_val = y_values.Evaluate (time);

		//
		//	Add this scalar to the list
		//
		m_OuterScaleChannel.Add_Key (Vector2 (x_val, y_val), time);
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Outer_Scale_Channel (m_OuterScaleChannel);
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
