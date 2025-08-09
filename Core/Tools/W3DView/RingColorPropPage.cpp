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

// RingColorPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "RingColorPropPage.h"
#include "OpacitySettingsDialog.h"
#include "ColorUtils.h"
#include "Utils.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



IMPLEMENT_DYNCREATE(RingColorPropPageClass, CPropertyPage)

/////////////////////////////////////////////////////////////
//
//	RingColorPropPageClass
//
/////////////////////////////////////////////////////////////
RingColorPropPageClass::RingColorPropPageClass (RingRenderObjClass *ring)
	:	m_RenderObj (ring),
		m_bValid (true),
		m_ColorBar (NULL),
		m_OpacityBar (NULL),
		CPropertyPage(RingColorPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(RingColorPropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	~RingColorPropPageClass
//
/////////////////////////////////////////////////////////////
RingColorPropPageClass::~RingColorPropPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
/////////////////////////////////////////////////////////////
void
RingColorPropPageClass::DoDataExchange (CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RingColorPropPageClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(RingColorPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(RingColorPropPageClass)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
RingColorPropPageClass::Initialize (void)
{
	m_ColorChannel.Reset ();
	m_OrigColorChannel.Reset ();
	m_AlphaChannel.Reset ();
	m_OrigAlphaChannel.Reset ();

	if (m_RenderObj != NULL) {

		m_ColorChannel			= m_RenderObj->Get_Color_Channel ();
		m_OrigColorChannel	= m_RenderObj->Get_Color_Channel ();

		m_AlphaChannel			= m_RenderObj->Get_Alpha_Channel ();
		m_OrigAlphaChannel	= m_RenderObj->Get_Alpha_Channel ();

		if (m_ColorChannel.Get_Key_Count () == 0) {
			m_ColorChannel.Add_Key (m_RenderObj->Get_Color (), 0);
			m_OrigColorChannel.Add_Key (m_RenderObj->Get_Color (), 0);
		}

		if (m_AlphaChannel.Get_Key_Count () == 0) {
			m_AlphaChannel.Add_Key (m_RenderObj->Get_Alpha (), 0);
			m_OrigAlphaChannel.Add_Key (m_RenderObj->Get_Alpha(), 0);
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
RingColorPropPageClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();

	m_ColorBar		= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_COLOR_BAR));
	m_OpacityBar	= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_OPACITY_BAR));

	//
	// Setup the color bars
	//
	m_ColorBar->Set_Range (0, 1);
	m_OpacityBar->Set_Range (0, 1);

	//
	//	Set-up the color bar
	//
	int index;
	for (index = 0; index < m_OrigColorChannel.Get_Key_Count (); index ++) {
		m_ColorBar->Modify_Point (	index,
											m_OrigColorChannel[index].Get_Time (),
											m_OrigColorChannel[index].Get_Value ().X * 255,
											m_OrigColorChannel[index].Get_Value ().Y * 255,
											m_OrigColorChannel[index].Get_Value ().Z * 255);
	}

	//
	//	Set-up the opacity bar
	//
	for (index = 0; index < m_OrigAlphaChannel.Get_Key_Count (); index ++) {
		m_OpacityBar->Modify_Point (	index,
												m_OrigAlphaChannel[index].Get_Time (),
												m_OrigAlphaChannel[index].Get_Value () * 255,
												m_OrigAlphaChannel[index].Get_Value () * 255,
												m_OrigAlphaChannel[index].Get_Value () * 255);
	}

	//
	//	Ensure our initial 'current' values are up-to-date
	//
	Update_Colors ();
	Update_Opacities ();
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL
RingColorPropPageClass::OnApply (void)
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
RingColorPropPageClass::OnDestroy (void)
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
RingColorPropPageClass::OnNotify
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
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {

				//
				//	Allow the user to edit the keyframe
				//
				OpacitySettingsDialogClass dialog (color_bar_hdr->red / 255, this);
				if (dialog.DoModal () == IDOK) {
					m_OpacityBar->Modify_Point (	color_bar_hdr->key_index,
															color_bar_hdr->position,
															dialog.Get_Opacity () * 255,
															dialog.Get_Opacity () * 255,
															dialog.Get_Opacity () * 255);

					//
					// Update the object
					//
					Update_Opacities ();
					SetModified ();
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT))
			{
				//
				// Update the object
				//
				Update_Opacities ();
				SetModified ();
			}
		}
		break;

		case IDC_COLOR_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {

				//
				//	Allow the user to edit the keyframe
				//
				int red		= (int)color_bar_hdr->red;
				int green	= (int)color_bar_hdr->green;
				int blue		= (int)color_bar_hdr->blue;
				if (Show_Color_Picker (&red, &green, &blue)) {
					m_ColorBar->Modify_Point (	color_bar_hdr->key_index,
														color_bar_hdr->position,
														red,
														green,
														blue);

					//
					// Update the object
					//
					Update_Colors ();
					SetModified ();
				}

			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT))
			{
				//
				// Update the object
				//
				Update_Colors ();
				SetModified ();
			}
		}
		break;
	}

	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  OnCancel
//
/////////////////////////////////////////////////////////////
void
RingColorPropPageClass::OnCancel (void)
{
	//
	//	Reset the object to its original state
	//
	m_RenderObj->Set_Color_Channel (m_OrigColorChannel);
	m_RenderObj->Set_Alpha_Channel (m_OrigAlphaChannel);

	CPropertyPage::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Opacities
//
/////////////////////////////////////////////////////////////
void
RingColorPropPageClass::Update_Opacities (void)
{
	m_AlphaChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;

	//
	//	Build the channel
	//
	int count = m_OpacityBar->Get_Point_Count ();
	for (int index = 0; index < count; index ++) {
		m_OpacityBar->Get_Point (index, &position, &red, &green, &blue);
		m_AlphaChannel.Add_Key (red / 255, position);
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Alpha_Channel (m_AlphaChannel);
	m_RenderObj->Restart_Animation ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Colors
//
/////////////////////////////////////////////////////////////
void
RingColorPropPageClass::Update_Colors (void)
{
	m_ColorChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;

	//
	//	Build the channel
	//
	int count = m_ColorBar->Get_Point_Count ();
	for (int index = 0; index < count; index ++) {
		m_ColorBar->Get_Point (index, &position, &red, &green, &blue);
		m_ColorChannel.Add_Key (Vector3 (red / 255, green / 255, blue / 255), position);
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Color_Channel (m_ColorChannel);
	m_RenderObj->Restart_Animation ();
	return ;
}

