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

// EmitterColorPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "emittercolorproppage.h"
#include "part_emt.h"
#include "utils.h"
#include "opacitysettingsdialog.h"
#include "colorutils.h"
#include "emitterinstancelist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterColorPropPageClass property page

IMPLEMENT_DYNCREATE(EmitterColorPropPageClass, CPropertyPage)


/////////////////////////////////////////////////////////////
//
//  EmitterColorPropPageClass
//
/////////////////////////////////////////////////////////////
EmitterColorPropPageClass::EmitterColorPropPageClass (EmitterInstanceListClass *pemitter)
	: m_pEmitterList (NULL),
	  m_bValid (true),
	  m_ColorBar (NULL),
	  m_OpacityBar (NULL),
	  m_Lifetime (0),
	  CPropertyPage (EmitterColorPropPageClass::IDD)
{

  ::memset (&m_OrigColors, 0, sizeof (m_OrigColors));
  ::memset (&m_OrigOpacities, 0, sizeof (m_OrigOpacities));
  ::memset (&m_CurrentColors, 0, sizeof (m_CurrentColors));
  ::memset (&m_CurrentOpacities, 0, sizeof (m_CurrentOpacities));

	//{{AFX_DATA_INIT(EmitterColorPropPageClass)
	//}}AFX_DATA_INIT
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  ~EmitterColorPropPageClass
//
/////////////////////////////////////////////////////////////
EmitterColorPropPageClass::~EmitterColorPropPageClass (void)
{
	// Free the original setting arrays
	SAFE_DELETE_ARRAY (m_OrigColors.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigColors.Values);
	SAFE_DELETE_ARRAY (m_OrigOpacities.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigOpacities.Values);
	
	// Free the current setting arrays
	SAFE_DELETE_ARRAY (m_CurrentColors.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentColors.Values);
	SAFE_DELETE_ARRAY (m_CurrentOpacities.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentOpacities.Values);
	return;
}


/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
/////////////////////////////////////////////////////////////
void
EmitterColorPropPageClass::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterColorPropPageClass)
	DDX_Control(pDX, IDC_OPACITY_RANDOM_SPIN, m_OpacityRandomSpin);
	DDX_Control(pDX, IDC_RED_RANDOM_SPIN, m_RedRandomSpin);
	DDX_Control(pDX, IDC_GREEN_RANDOM_SPIN, m_GreenRandomSpin);
	DDX_Control(pDX, IDC_BLUE_RANDOM_SPIN, m_BlueRandomSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EmitterColorPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterColorPropPageClass)
	ON_WM_DESTROY()
	ON_NOTIFY(UDN_DELTAPOS, IDC_RED_RANDOM_SPIN, OnDeltaposRedRandomSpin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
EmitterColorPropPageClass::Initialize (void)
{
	SAFE_DELETE_ARRAY (m_OrigColors.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigColors.Values);
	SAFE_DELETE_ARRAY (m_OrigOpacities.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigOpacities.Values);
	SAFE_DELETE_ARRAY (m_CurrentColors.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentColors.Values);
	SAFE_DELETE_ARRAY (m_CurrentOpacities.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentOpacities.Values);

	if (m_pEmitterList != NULL) {

		m_Lifetime = m_pEmitterList->Get_Lifetime ();

		//
		//	Get the initial values from the emitter
		//
		m_pEmitterList->Get_Color_Keyframes (m_OrigColors);
		m_pEmitterList->Get_Color_Keyframes (m_CurrentColors);
		m_pEmitterList->Get_Opacity_Keyframes (m_OrigOpacities);
		m_pEmitterList->Get_Opacity_Keyframes (m_CurrentOpacities);				
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
/////////////////////////////////////////////////////////////
BOOL
EmitterColorPropPageClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();
	
	m_ColorBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_COLOR_BAR));
	m_OpacityBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_OPACITY_BAR));

	//
	// Setup the color bars
	//
	m_ColorBar->Set_Range (0, 1);
	m_OpacityBar->Set_Range (0, 1);
	m_OpacityBar->Modify_Point (0, 0, 255, 255, 255);

	//
	// Setup the spinners
	//
	m_OpacityRandomSpin.SetRange (0, 100);
	m_RedRandomSpin.SetRange (0, 255);
	m_GreenRandomSpin.SetRange (0, 255);
	m_BlueRandomSpin.SetRange (0, 255);

	m_OpacityRandomSpin.SetPos (m_OrigOpacities.Rand * 100);
	m_RedRandomSpin.SetPos (m_OrigColors.Rand.X * 255);
	m_GreenRandomSpin.SetPos (m_OrigColors.Rand.Y * 255);
	m_BlueRandomSpin.SetPos (m_OrigColors.Rand.Z * 255);

	//
	//	Reset the color bars
	//
	m_ColorBar->Clear_Points ();
	m_OpacityBar->Clear_Points ();
	m_ColorBar->Modify_Point (0, 0, m_OrigColors.Start.X * 255, m_OrigColors.Start.Y * 255, m_OrigColors.Start.Z * 255);
	m_OpacityBar->Modify_Point (0, 0, m_OrigOpacities.Start * 255, m_OrigOpacities.Start * 255, m_OrigOpacities.Start * 255);

	//
	//	Setup the ranges
	//
	m_ColorBar->Set_Range (0, 1);
	m_OpacityBar->Set_Range (0, 1);

	//
	//	Set-up the color bar
	//
	for (UINT index = 0; index < m_OrigColors.NumKeyFrames; index ++) {
		m_ColorBar->Modify_Point (index + 1,
											m_OrigColors.KeyTimes[index] / m_Lifetime,
											m_OrigColors.Values[index].X * 255,
											m_OrigColors.Values[index].Y * 255,
											m_OrigColors.Values[index].Z * 255);
	}

	//
	//	Set-up the opacity bar
	//
	for (index = 0; index < m_OrigOpacities.NumKeyFrames; index ++) {
		m_OpacityBar->Modify_Point (index + 1,
											m_OrigOpacities.KeyTimes[index] / m_Lifetime,
											m_OrigOpacities.Values[index] * 255,
											m_OrigOpacities.Values[index] * 255,
											m_OrigOpacities.Values[index] * 255);
	}

	//
	//	Ensure our initial 'current' values are up-to-date
	//
	m_CurrentColors.Rand = m_OrigColors.Rand;
	m_CurrentOpacities.Rand = m_OrigOpacities.Rand;
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
EmitterColorPropPageClass::OnApply (void)
{
	/*SAFE_DELETE_ARRAY (m_OrigColors.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigColors.Values);
	SAFE_DELETE_ARRAY (m_OrigOpacities.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigOpacities.Values);

	//
	//	Make the current values from the emitter the default values.
	//
	m_pEmitter->Get_Color_Key_Frames (m_OrigColors);
	m_pEmitter->Get_Opacity_Key_Frames (m_OrigOpacities);*/

	// Allow the base class to process this message
	return CPropertyPage::OnApply ();
}


/////////////////////////////////////////////////////////////
//
//  OnDestroy
//
/////////////////////////////////////////////////////////////
void
EmitterColorPropPageClass::OnDestroy (void)
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
EmitterColorPropPageClass::OnNotify
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
					m_OpacityBar->Modify_Point (color_bar_hdr->key_index,
														color_bar_hdr->position,
														dialog.Get_Opacity () * 255,
														dialog.Get_Opacity () * 255,
														dialog.Get_Opacity () * 255);

					// Update the emitter
					Update_Opacities ();
					m_pEmitterList->Set_Opacity_Keyframes (m_CurrentOpacities);
					SetModified ();					
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT)) {
				
				// Update the emitter
				Update_Opacities ();
				m_pEmitterList->Set_Opacity_Keyframes (m_CurrentOpacities);
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
					m_ColorBar->Modify_Point (color_bar_hdr->key_index,
														color_bar_hdr->position,
														red,
														green,
														blue);

					// Update the emitter
					Update_Colors ();
					m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);
					SetModified ();
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT)) {

				// Update the emitter
				Update_Colors ();
				m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);
				SetModified ();
			}
		}
		break;

		case IDC_RED_RANDOM_SPIN:
		{
			// Update the emitter
			NMUPDOWN *pudnotif = (NMUPDOWN *)lParam;
			if (pudnotif->hdr.code == UDN_DELTAPOS) {
				float pos = (pudnotif->iPos + pudnotif->iDelta);
				m_CurrentColors.Rand.X = pos / 255.0F;
				m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);
				SetModified ();
			}
		}
		break;

		case IDC_GREEN_RANDOM_SPIN:
		{
			// Update the emitter
			NMUPDOWN *pudnotif = (NMUPDOWN *)lParam;
			if (pudnotif->hdr.code == UDN_DELTAPOS) {
				float pos = (pudnotif->iPos + pudnotif->iDelta);
				m_CurrentColors.Rand.Y = pos / 255.0F;
				m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);
				SetModified ();
			}
		}
		break;

		case IDC_BLUE_RANDOM_SPIN:
		{
			// Update the emitter
			NMUPDOWN *pudnotif = (NMUPDOWN *)lParam;
			if (pudnotif->hdr.code == UDN_DELTAPOS) {
				float pos = (pudnotif->iPos + pudnotif->iDelta);
				m_CurrentColors.Rand.Z = pos / 255.0F;
				m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);
				SetModified ();
			}
		}
		break;

		case IDC_OPACITY_RANDOM_SPIN:
		{
			// Update the emitter
			NMUPDOWN *pudnotif = (NMUPDOWN *)lParam;
			if (pudnotif->hdr.code == UDN_DELTAPOS) {
				float pos = (pudnotif->iPos + pudnotif->iDelta);
				m_CurrentOpacities.Rand = pos / 100.0F;
				m_pEmitterList->Set_Opacity_Keyframes (m_CurrentOpacities);
				SetModified ();
			}
		}
		break;
	}
		
	return CPropertyPage::OnNotify(wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  OnCancel
//
/////////////////////////////////////////////////////////////
void
EmitterColorPropPageClass::OnCancel (void)
{
	//
	//	Reset the emitter to its original state
	//
	m_pEmitterList->Set_Color_Keyframes (m_OrigColors);
	m_pEmitterList->Set_Opacity_Keyframes (m_OrigOpacities);

	CPropertyPage::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Opacities
//
/////////////////////////////////////////////////////////////
void
EmitterColorPropPageClass::Update_Opacities (void)
{
	float position = 0;
	float red = 0;
	float green = 0;
	float blue = 0;
	m_OpacityBar->Get_Point (0, &position, &red, &green, &blue);

	//
	//	Setup the initial or 'starting' opacity
	//
	m_CurrentOpacities.Start = red / 255;

	//
	// Free the current setting arrays
	//
	SAFE_DELETE_ARRAY (m_CurrentOpacities.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentOpacities.Values);

	//
	//	Determine if we need to build the array of key frames or not
	//
	int count = m_OpacityBar->Get_Point_Count ();
	m_CurrentOpacities.NumKeyFrames = count - 1;
	if (count > 1) {
		m_CurrentOpacities.KeyTimes = new float[count - 1];
		m_CurrentOpacities.Values = new float[count - 1];

		//
		//	Get all the opacity key frames and add them to our structure
		//	
		for (int index = 1; index < count; index ++) {
			m_OpacityBar->Get_Point (index, &position, &red, &green, &blue);
			m_CurrentOpacities.KeyTimes[index - 1] = position * m_Lifetime;
			m_CurrentOpacities.Values[index - 1] = red / 255;
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Colors
//
/////////////////////////////////////////////////////////////
void
EmitterColorPropPageClass::Update_Colors (void)
{		
	float position = 0;
	float red = 0;
	float green = 0;
	float blue = 0;
	m_ColorBar->Get_Point (0, &position, &red, &green, &blue);

	//
	//	Setup the initial or 'starting' color
	//
	m_CurrentColors.Start.X = red / 255;
	m_CurrentColors.Start.Y = green / 255;
	m_CurrentColors.Start.Z = blue / 255;

	//
	// Free the current setting arrays
	//
	SAFE_DELETE_ARRAY (m_CurrentColors.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentColors.Values);

	//
	//	Determine if we need to build the array of key frames or not
	//
	int count = m_ColorBar->Get_Point_Count ();
	m_CurrentColors.NumKeyFrames = count - 1;
	if (count > 1) {
		m_CurrentColors.KeyTimes = new float[count - 1];
		m_CurrentColors.Values = new Vector3[count - 1];

		//
		//	Get all the color points and add them to our structure
		//	
		for (int index = 1; index < count; index ++) {
			m_ColorBar->Get_Point (index, &position, &red, &green, &blue);
			m_CurrentColors.KeyTimes[index-1] = position * m_Lifetime;
			m_CurrentColors.Values[index-1].X	= red / 255;
			m_CurrentColors.Values[index-1].Y	= green / 255;
			m_CurrentColors.Values[index-1].Z	= blue / 255;
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnCommand
//
/////////////////////////////////////////////////////////////
BOOL
EmitterColorPropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_RED_RANDOM_EDIT:
		{			
			if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}

			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				m_CurrentColors.Rand.X = ((float)GetDlgItemInt (IDC_RED_RANDOM_EDIT)) / 255;
				m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);
				SetModified ();				
			}
		}
		break;

		case IDC_GREEN_RANDOM_EDIT:
		{
			if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}

			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				m_CurrentColors.Rand.Y = ((float)GetDlgItemInt (IDC_GREEN_RANDOM_EDIT)) / 255;
				m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);
				SetModified ();
			}
		}
		break;

		case IDC_BLUE_RANDOM_EDIT:
		{
			if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}

			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				m_CurrentColors.Rand.Z = ((float)GetDlgItemInt (IDC_BLUE_RANDOM_EDIT)) / 255;
				m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);				
				SetModified ();
			}
		}
		break;

		case IDC_OPACITY_RANDOM_EDIT:
		{
			if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}

			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				m_CurrentOpacities.Rand = ((float)GetDlgItemInt (IDC_OPACITY_RANDOM_EDIT)) / 100;
				m_pEmitterList->Set_Opacity_Keyframes (m_CurrentOpacities);
				SetModified ();
			}
		}
		break;
	}

	return CPropertyPage::OnCommand (wParam, lParam);
}

void EmitterColorPropPageClass::OnDeltaposRedRandomSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	int test = 0;
	
	*pResult = 0;
	return ;
}


/////////////////////////////////////////////////////////////
//
//  On_Lifetime_Changed
//
/////////////////////////////////////////////////////////////
void
EmitterColorPropPageClass::On_Lifetime_Changed (float lifetime)
{
	if (m_Lifetime != lifetime) {
		float conversion = lifetime / m_Lifetime;

		//
		//	Rescale the colors
		//
		for (UINT index = 0; index < m_CurrentColors.NumKeyFrames; index ++) {
			m_CurrentColors.KeyTimes[index] *= conversion;
		}
		
		//
		//	Rescale the opacities
		//
		for (index = 0; index < m_CurrentOpacities.NumKeyFrames; index ++) {
			m_CurrentOpacities.KeyTimes[index] *= conversion;
		}

		//
		//	Update the emitter
		//
		m_pEmitterList->Set_Color_Keyframes (m_CurrentColors);		
		m_pEmitterList->Set_Opacity_Keyframes (m_CurrentOpacities);		
		m_Lifetime = lifetime;
		/*if (m_hWnd != NULL) {
			Update_Colors ();
			Update_Opacities ();
		}*/
	}

	return ;
}


