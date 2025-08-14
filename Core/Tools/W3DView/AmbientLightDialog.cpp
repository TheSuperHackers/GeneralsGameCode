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

// AmbientLightDialog.cpp : implementation file
//

#include "StdAfx.h"

#include "W3DView.h"
#include "AmbientLightDialog.h"
#include "MainFrm.h"
#include "W3DViewDoc.h"
#include "ViewerScene.h"
#include "Utils.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAmbientLightDialog dialog


CAmbientLightDialog::CAmbientLightDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAmbientLightDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAmbientLightDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAmbientLightDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAmbientLightDialog)
	DDX_Control(pDX, IDC_SLIDER_BLUE, m_blueSlider);
	DDX_Control(pDX, IDC_SLIDER_GREEN, m_greenSlider);
	DDX_Control(pDX, IDC_SLIDER_RED, m_redSlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAmbientLightDialog, CDialog)
	//{{AFX_MSG_MAP(CAmbientLightDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_GRAYSCALE_CHECK, OnGrayscaleCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CAmbientLightDialog::OnInitDialog (void)
{
	// Allow the base class to process this message
    CDialog::OnInitDialog ();

    // Center the dialog around the data tree view instead
    // of the direct center of the screen
    ::CenterDialogAroundTreeView (m_hWnd);

    m_redSlider.SetRange (0, 100);
    m_greenSlider.SetRange (0, 100);
    m_blueSlider.SetRange (0, 100);

    // Get a pointer to the doc so we can get at the current scene
    // pointer.
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc && pCDoc->GetScene ())
    {
        Vector3 lightSettings = pCDoc->GetScene ()->Get_Ambient_Light ();

        // Remember these initial settings so we can restore them
        // if the user cancels
        m_initialRed = int(lightSettings.X * 100.00F);
        m_initialGreen = int(lightSettings.Y * 100.00F);
        m_initialBlue = int(lightSettings.Z * 100.00F);
    }

    if ((m_initialRed == m_initialGreen) &&
        (m_initialRed == m_initialBlue))
    {
        // Check the grayscale checkbox
        SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_SETCHECK, (WPARAM)TRUE);
    }

    // Set the initial slider position
    m_redSlider.SetPos (m_initialRed);
    m_greenSlider.SetPos (m_initialGreen);
    m_blueSlider.SetPos (m_initialBlue);
	return TRUE;
}

//////////////////////////////////////////////////////////////
//
//  OnHScroll
//
void
CAmbientLightDialog::OnHScroll
(
    UINT nSBCode,
    UINT nPos,
    CScrollBar* pScrollBar
)
{
    if (SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_GETCHECK))
    {
        int iCurrentPos = 0;
        if (pScrollBar == GetDlgItem (IDC_SLIDER_RED))
        {
            iCurrentPos = m_redSlider.GetPos ();
        }
        else if (pScrollBar == GetDlgItem (IDC_SLIDER_GREEN))
        {
            iCurrentPos = m_greenSlider.GetPos ();
        }
        else
        {
            iCurrentPos = m_blueSlider.GetPos ();
        }

        // Make all the sliders the same pos
        m_redSlider.SetPos (iCurrentPos);
        m_greenSlider.SetPos (iCurrentPos);
        m_blueSlider.SetPos (iCurrentPos);
    }

    Vector3 lightSettings;
    lightSettings.X = float(m_redSlider.GetPos ()) / 100.00F;
    lightSettings.Y = float(m_greenSlider.GetPos ()) / 100.00F;
    lightSettings.Z = float(m_blueSlider.GetPos ()) / 100.00F;

    // Get a pointer to the document so we can change the scene's light
    // settings
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc && pCDoc->GetScene ())
    {
        // Modify the ambient light for this scene
        pCDoc->GetScene ()->Set_Ambient_Light (lightSettings);
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
CAmbientLightDialog::OnCancel (void)
{
    Vector3 lightSettings;
    lightSettings.X = float(m_initialRed) / 100.00F;
    lightSettings.Y = float(m_initialGreen) / 100.00F;
    lightSettings.Z = float(m_initialBlue) / 100.00F;

    // Get a pointer to the document so we can change the scene's light
    // settings
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc && pCDoc->GetScene ())
    {
        // Modify the ambient light for this scene
        pCDoc->GetScene ()->Set_Ambient_Light (lightSettings);
    }

	// Allow the base class to process this message
    CDialog::OnCancel();
    return ;
}

//////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT
CAmbientLightDialog::WindowProc
(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    if (message == WM_PAINT)
    {
        // Paint the gradients for each color
        ::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_RED_GRADIENT), 1, 0, 0);
        ::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_GREEN_GRADIENT), 0, 1, 0);
        ::Paint_Gradient (::GetDlgItem (m_hWnd, IDC_BLUE_GRADIENT), 0, 0, 1);
    }

	// Allow the base class to process this message
    return CDialog::WindowProc (message, wParam, lParam);
}

//////////////////////////////////////////////////////////////
//
//  OnGrayscaleCheck
//
void
CAmbientLightDialog::OnGrayscaleCheck (void)
{
    if (SendDlgItemMessage (IDC_GRAYSCALE_CHECK, BM_GETCHECK))
    {
        // Make the green and blue sliders the same as red
        m_greenSlider.SetPos (m_redSlider.GetPos ());
        m_blueSlider.SetPos (m_redSlider.GetPos ());

        Vector3 lightSettings;
        lightSettings.X = float(m_redSlider.GetPos ()) / 100.00F;
        lightSettings.Y = float(m_greenSlider.GetPos ()) / 100.00F;
        lightSettings.Z = float(m_blueSlider.GetPos ()) / 100.00F;

        // Get a pointer to the document so we can change the scene's light
        // settings
        CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
        if (pCDoc && pCDoc->GetScene ())
        {
            // Modify the ambient light for this scene
            pCDoc->GetScene ()->Set_Ambient_Light (lightSettings);
        }
    }

    return ;
}

