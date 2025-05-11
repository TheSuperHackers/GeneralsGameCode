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

// SceneLightDialog.cpp : implementation file
//

#include "stdafx.h"
#include "W3DView.h"
#include "SceneLightDialog.h"
#include "Utils.H"
#include "MainFrm.H"
#include "W3DViewDoc.H"
#include "Scene.H"
#include "Light.H"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////
//
//  CSceneLightDialog
//
CSceneLightDialog::CSceneLightDialog(CWnd* pParent /*=NULL*/)
	: m_CurrentChannel (DIFFUSE),
	  m_InitialStartAtten (0),
	  m_InitialEndAtten (0),
	  m_InitialDistance (0),
	  m_InitialIntensity (0),
	  m_InitialAttenOn (TRUE),
	  CDialog(CSceneLightDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSceneLightDialog)
	//}}AFX_DATA_INIT
    return ;
}


//////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CSceneLightDialog::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSceneLightDialog)
	DDX_Control(pDX, IDC_START_ATTENUATION_SPIN, m_StartAttenSpin);
	DDX_Control(pDX, IDC_END_ATTENUATION_SPIN, m_EndAttenSpin);
	DDX_Control(pDX, IDC_DISTANCE_SPIN, m_DistanceSpin);
	DDX_Control(pDX, IDC_INTENSITY_SLIDER, m_IntensitySlider);
	DDX_Control(pDX, IDC_SLIDER_BLUE, m_blueSlider);
	DDX_Control(pDX, IDC_SLIDER_GREEN, m_greenSlider);
	DDX_Control(pDX, IDC_SLIDER_RED, m_redSlider);
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CSceneLightDialog, CDialog)
	//{{AFX_MSG_MAP(CSceneLightDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_GRAYSCALE_CHECK, OnGrayscaleCheck)
	ON_BN_CLICKED(IDC_CHANNEL_BOTH_RADIO, OnChannelBothRadio)
	ON_BN_CLICKED(IDC_CHANNEL_DIFFUSE_RADIO, OnChannelDiffuseRadio)
	ON_BN_CLICKED(IDC_CHANNEL_SPECULAR_RADIO, OnChannelSpecularRadio)
	ON_BN_CLICKED(IDC_ATTENUATION_CHECK, OnAttenuationCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CSceneLightDialog::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();

	// Center the dialog around the data tree view instead
	// of the direct center of the screen
	::CenterDialogAroundTreeView (m_hWnd);

	// Set the initial ranges for the color sliders
	m_redSlider.SetRange (0, 100);
	m_greenSlider.SetRange (0, 100);
	m_blueSlider.SetRange (0, 100);    

	// Get a pointer to the doc so we can get at the current scene
	// pointer.
	CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	if (pCDoc && pCDoc->GetScene ()) {
		Vector3 diffuse;
		Vector3 specular;
		pCDoc->GetSceneLight ()->Get_Diffuse (&diffuse);
		pCDoc->GetSceneLight ()->Get_Specular (&specular);

		// Remember these initial settings so we can restore them
		// if the user cancels
		m_InitialRedDiffuse = int(diffuse.X * 100.00F);
		m_InitialGreenDiffuse = int(diffuse.Y * 100.00F);
		m_InitialBlueDiffuse = int(diffuse.Z * 100.00F);        
		m_InitialRedSpecular = int(specular.X * 100.00F);
		m_InitialGreenSpecular = int(specular.Y * 100.00F);
		m_InitialBlueSpecular = int(specular.Z * 100.00F);
		Set_Color_Control_State (diffuse);		

		// Get the light's attenuation
		double start = 0;
		double end = 0;
		pCDoc->GetSceneLight ()->Get_Far_Attenuation_Range (start, end);
		BOOL atten_on = pCDoc->GetSceneLight ()->Get_Flag (LightClass::FAR_ATTENUATION);
		SendDlgItemMessage (IDC_ATTENUATION_CHECK, BM_SETCHECK, (WPARAM)atten_on);

		// Get the light's intensity
		float intensity = pCDoc->GetSceneLight ()->Get_Intensity ();			

		// Attempt to calculate the light's distance from the object
		float distance = 0;
		if (pCDoc->GetDisplayedObject () != NULL) {
			
			// Get the position of the light and the displayed object
			Vector3 light_pos = pCDoc->GetSceneLight ()->Get_Position ();
			Vector3 obj_pos = pCDoc->GetDisplayedObject ()->Get_Position ();

			// Compute the distance of the 2 vectors
			distance = (light_pos - obj_pos).Length ();
		}

		// Set-up the edit controls
		::SetDlgItemFloat (m_hWnd, IDC_START_ATTENUATION_EDIT, start);
		::SetDlgItemFloat (m_hWnd, IDC_END_ATTENUATION_EDIT, end);
		::SetDlgItemFloat (m_hWnd, IDC_DISTANCE_EDIT, distance);
		
		// Set-up the spin controls
		m_DistanceSpin.SetRange (0, 1000000L);
		m_DistanceSpin.SetPos ((distance * 100));
		m_StartAttenSpin.SetRange (0, 1000000L);
		m_StartAttenSpin.SetPos ((start * 100));
		m_EndAttenSpin.SetRange (0, 1000000L);
		m_EndAttenSpin.SetPos ((end * 100));

		// Setup the slider control
		m_IntensitySlider.SetRange (0, 100);
		m_IntensitySlider.SetPos (intensity * 100.0F);

		// Record the initial settings so we can restore them on cancel
		m_InitialStartAtten = start;
		m_InitialEndAtten = end;
		m_InitialDistance = distance;
		m_InitialIntensity = intensity;
		m_InitialAttenOn = atten_on;
	}

	// Check the 'Diffuse' channel by default
	SendDlgItemMessage (IDC_CHANNEL_DIFFUSE_RADIO, BM_SETCHECK, (WPARAM)TRUE);
	Update_Attenuation_Controls ();
	return TRUE;
}


//////////////////////////////////////////////////////////////
//
//  OnHScroll
//
void
CSceneLightDialog::OnHScroll
(
    UINT nSBCode,
    UINT nPos,
    CScrollBar* pScrollBar
)
{
	// Did the intensity slider send this message or did the color sliders?
	if (pScrollBar == GetDlgItem (IDC_INTENSITY_SLIDER)) {
		
		// Update the light's intensity settings
		float intensity = ((float)m_IntensitySlider.GetPos ()) / 100.0F;
		::GetCurrentDocument ()->GetSceneLight ()->Set_Intensity (intensity);

	} else {

		// Are the 3 colors locked together?
		if (SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_GETCHECK)) {
			
			// Which color sent this message?
			int iCurrentPos = 0;
			if (pScrollBar == GetDlgItem (IDC_SLIDER_RED)) {

				// Use this color as default
				iCurrentPos = m_redSlider.GetPos ();
			}
			else if (pScrollBar == GetDlgItem (IDC_SLIDER_GREEN)) {

				// Use this color as default
				iCurrentPos = m_greenSlider.GetPos ();
			}
			else {

				// Use this color as default
				iCurrentPos = m_blueSlider.GetPos ();
			}

			// Make all the sliders the same pos
			m_redSlider.SetPos (iCurrentPos);
			m_greenSlider.SetPos (iCurrentPos);
			m_blueSlider.SetPos (iCurrentPos);
		}

		// Update the light to reflect the new settings
		Vector3 color;
		color.X = float(m_redSlider.GetPos ()) / 100.00F;
		color.Y = float(m_greenSlider.GetPos ()) / 100.00F;
		color.Z = float(m_blueSlider.GetPos ()) / 100.00F;
		Update_Light (color);
	}

	// Allow the base class to process this message
	CDialog::OnHScroll (nSBCode, nPos, pScrollBar);
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnCancel
//
void
CSceneLightDialog::OnCancel (void)
{
	// Get a pointer to the document so we can change the scene light's
	// settings and position
	CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	if (pCDoc && pCDoc->GetScene ()) {

		Vector3 diffuse;
		diffuse.X = float(m_InitialRedDiffuse) / 100.00F;
		diffuse.Y = float(m_InitialGreenDiffuse) / 100.00F;
		diffuse.Z = float(m_InitialBlueDiffuse) / 100.00F;

		Vector3 specular;
		specular.X = float(m_InitialRedSpecular) / 100.00F;
		specular.Y = float(m_InitialGreenSpecular) / 100.00F;
		specular.Z = float(m_InitialBlueSpecular) / 100.00F;

		// Restore the scene light's color
		pCDoc->GetSceneLight ()->Set_Diffuse (diffuse);
		pCDoc->GetSceneLight ()->Set_Specular (specular);

		// Restore the intensity, attenuation, and distance settings
		pCDoc->GetSceneLight ()->Set_Intensity (m_InitialIntensity);
		pCDoc->GetSceneLight ()->Set_Far_Attenuation_Range (m_InitialStartAtten, m_InitialEndAtten);
		pCDoc->GetSceneLight ()->Set_Flag (LightClass::FAR_ATTENUATION, (m_InitialAttenOn == TRUE));
		Update_Distance (m_InitialDistance);		
	}

	// Allow the base class to process this message
	CDialog::OnCancel ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT
CSceneLightDialog::WindowProc
(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) 
{
	switch (message)
	{
		case WM_NOTIFY:
		{
			// Did this notification come from a spin control?
			NMHDR *pheader = (NMHDR *)lParam;
			if ((pheader != NULL) && (pheader->code == UDN_DELTAPOS)) {
				LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;
				
				// Get the buddy window associated with this spin control
				HWND hbuddy_wnd = (HWND)SendDlgItemMessage ((int)wParam, UDM_GETBUDDY);
				if (::IsWindow (hbuddy_wnd)) {

					// Get the current value, increment it, and put it back into the control
					float value = ::GetWindowFloat (hbuddy_wnd);
					value += (((float)(pupdown->iDelta)) / 100.0F);
					::SetWindowFloat (hbuddy_wnd, value);

					// Force update the light
					Update_Distance (::GetDlgItemFloat (m_hWnd, IDC_DISTANCE_EDIT));
					Update_Attenuation ();
				}
			}
		}
		break;

		case WM_COMMAND:
		{
			if (HIWORD (wParam) == EN_KILLFOCUS) {
				switch (LOWORD (wParam))
				{
					case IDC_DISTANCE_EDIT:
					{
						Update_Distance (::GetDlgItemFloat (m_hWnd, IDC_DISTANCE_EDIT));
					}
					break;
					
					case IDC_START_ATTENUATION_EDIT:
					case IDC_END_ATTENUATION_EDIT:
					{
						Update_Attenuation ();
					}
					break;
				}
			}
		}
		break;

		case WM_PAINT:
		{
			// Paint the gradients for each color
			::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_RED_GRADIENT), 1, 0, 0);
			::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_GREEN_GRADIENT), 0, 1, 0);
			::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_BLUE_GRADIENT), 0, 0, 1);
		}
		break;
	}
	
	// Allow the base class to process this message
	return CDialog::WindowProc (message, wParam, lParam);
}


//////////////////////////////////////////////////////////////
//
//  OnGrayscaleCheck
//
void
CSceneLightDialog::OnGrayscaleCheck (void)
{
	if (SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_GETCHECK)) {
		
		// Make the green and blue sliders the same as red
		m_greenSlider.SetPos (m_redSlider.GetPos ());
		m_blueSlider.SetPos (m_redSlider.GetPos ());

		Vector3 color;
		color.X = float(m_redSlider.GetPos ()) / 100.00F;
		color.Y = float(m_greenSlider.GetPos ()) / 100.00F;
		color.Z = float(m_blueSlider.GetPos ()) / 100.00F;
		Update_Light (color);
	}

	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnChannelBothRadio
//
void
CSceneLightDialog::OnChannelBothRadio (void)
{
	m_CurrentChannel = BOTH;
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnChannelDiffuseRadio
//
void
CSceneLightDialog::OnChannelDiffuseRadio (void)
{
	// Reset the UI to reflect the current diffuse color
	Vector3 color;
	::GetCurrentDocument ()->GetSceneLight ()->Get_Diffuse (&color);
	Set_Color_Control_State (color);
	m_CurrentChannel = DIFFUSE;
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnChannelSpecularRadio
//
void
CSceneLightDialog::OnChannelSpecularRadio (void)
{
	// Reset the UI to reflect the current specular color
	Vector3 color;
	::GetCurrentDocument ()->GetSceneLight ()->Get_Specular (&color);
	Set_Color_Control_State (color);
	m_CurrentChannel = SPECULAR;
	return ;
}


//////////////////////////////////////////////////////////////
//
//  Update_Light
//
void
CSceneLightDialog::Update_Light (const Vector3 &color)
{
	// Get a pointer to the document so we can change the scene light's
	// settings
	CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	if (pCDoc && pCDoc->GetScene ()) {

		// Update the diffuse setting of the scene light if necessary
		if (m_CurrentChannel & DIFFUSE) {
			pCDoc->GetSceneLight ()->Set_Diffuse (color);
		}

		// Update the specular setting of the scene light if necessary
		if (m_CurrentChannel & SPECULAR) {
			pCDoc->GetSceneLight ()->Set_Specular (color);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////
//
//  Set_Color_Control_State
//
void
CSceneLightDialog::Set_Color_Control_State (const Vector3 &color)
{
	// Should we 'lock' the color sliders together?
	if ((color.X == color.Y) &&
		 (color.X == color.Z)) {
		SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_SETCHECK, (WPARAM)TRUE);
	} else {		
		SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_SETCHECK, (WPARAM)FALSE);
	}

	// Set the color slider positions
	m_redSlider.SetPos (int(color.X * 100.0F));
	m_greenSlider.SetPos (int(color.Y * 100.0F));
	m_blueSlider.SetPos (int(color.Z * 100.0F));
	return ;
}


//////////////////////////////////////////////////////////////
//
//  Update_Attenuation
//
void
CSceneLightDialog::Update_Attenuation (void)
{
	// Get a pointer to the document so we can change the scene light's
	// settings
	CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	if (pCDoc && pCDoc->GetScene ()) {

		// Update the attenuation settings
		float start = ::GetDlgItemFloat (m_hWnd, IDC_START_ATTENUATION_EDIT);
		float end = ::GetDlgItemFloat (m_hWnd, IDC_END_ATTENUATION_EDIT);
		pCDoc->GetSceneLight ()->Set_Far_Attenuation_Range (start, end);
	}

	return ;
}


//////////////////////////////////////////////////////////////
//
//  Update_Distance
//
void
CSceneLightDialog::Update_Distance (float distance)
{
	// Get a pointer to the document so we can change the scene light's
	// settings
	CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	if (pCDoc && pCDoc->GetScene ()) {

		// Get the position of the displayed object
		Vector3 obj_pos (0,0,0);		
		if (pCDoc->GetDisplayedObject ()) {
			obj_pos = pCDoc->GetDisplayedObject ()->Get_Position ();
		}

		// Get the position of the light and the displayed object
		Vector3 light_pos = pCDoc->GetSceneLight ()->Get_Position ();		
		Vector3 new_pos = (light_pos - obj_pos);
		new_pos.Normalize ();
		new_pos = new_pos * distance;

		// Update the attenuation settings
		pCDoc->GetSceneLight ()->Set_Position (new_pos);
	}

	return ;
}


//////////////////////////////////////////////////////////////
//
//  Update_Attenuation_Controls
//
void
CSceneLightDialog::Update_Attenuation_Controls (void)
{
	// Enable or disable the attenuation controls based on the group's checkstate
	BOOL enable = (SendDlgItemMessage (IDC_ATTENUATION_CHECK, BM_GETCHECK) == 1);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_START_ATTENUATION_EDIT), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_START_ATTENUATION_SPIN), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_END_ATTENUATION_EDIT), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_END_ATTENUATION_SPIN), enable);
	return ;
}


//////////////////////////////////////////////////////////////
//
//  OnAttenuationCheck
//
void
CSceneLightDialog::OnAttenuationCheck (void) 
{
	// Update the scene light to reflect the new setting	
	bool enable = (SendDlgItemMessage (IDC_ATTENUATION_CHECK, BM_GETCHECK) == 1);
	CW3DViewDoc *pdoc = ::GetCurrentDocument ();
	pdoc->GetSceneLight ()->Set_Flag (LightClass::FAR_ATTENUATION, enable);

	// Update the dialog controls to reflect the new setting
	Update_Attenuation_Controls ();
	return ;
}
