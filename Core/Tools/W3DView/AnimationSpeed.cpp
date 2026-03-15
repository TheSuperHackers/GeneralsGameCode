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

// AnimationSpeed.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "AnimationSpeed.h"
#include "MainFrm.h"
#include "GraphicView.h"
#include "Utils.h"
#include "W3DViewDoc.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//extern bool CompressQ;
//extern int QnBytes;


/////////////////////////////////////////////////////////////////////////////
// CAnimationSpeed dialog

//////////////////////////////////////////////////////////////
//
//  CAnimationSpeed
//
CAnimationSpeed::CAnimationSpeed (CWnd* pParent)
	: m_iInitialPercent (0),
      CDialog(CAnimationSpeed::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimationSpeed)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    return ;
}

//////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CAnimationSpeed::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationSpeed)
	DDX_Control(pDX, IDC_SPEED_SLIDER, m_speedSlider);
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CAnimationSpeed, CDialog)
	//{{AFX_MSG_MAP(CAnimationSpeed)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BLEND, OnBlend)
	ON_BN_CLICKED(IDC_COMPRESSQ, OnCompressq)
	ON_BN_CLICKED(IDC_16BIT, On16bit)
	ON_BN_CLICKED(IDC_8BIT, On8bit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CAnimationSpeed::OnInitDialog (void)
{
	// Allow the base class to process this message
    CDialog::OnInitDialog ();

    // Center the dialog around the data tree view instead
    // of the direct center of the screen
    ::CenterDialogAroundTreeView (m_hWnd);

    // Get a pointer to the doc so we can get at the current scene
    // pointer.
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        SendDlgItemMessage (IDC_BLEND, BM_SETCHECK, (WPARAM)pCDoc->GetAnimationBlend ());
		  CheckDlgButton(IDC_COMPRESSQ, pCDoc->GetChannelQCompression());
			CheckRadioButton(IDC_16BIT, IDC_8BIT, IDC_16BIT+2);//-pCDoc->GetChannelQnBytes());
		  if(pCDoc->GetChannelQCompression()){
				GetDlgItem(IDC_16BIT)->EnableWindow(TRUE);
				GetDlgItem(IDC_8BIT)->EnableWindow(TRUE);
		  }else{
			  GetDlgItem(IDC_16BIT)->EnableWindow(FALSE);
			  GetDlgItem(IDC_8BIT)->EnableWindow(FALSE);
		  }
	 }

    // Get a pointer to the main window
    CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
    if (pCMainWnd)
    {
        // Get a pointer to the graphic view pane
        CGraphicView *pCGraphicView = (CGraphicView *)pCMainWnd->GetPane (0, 1);
        if (pCGraphicView)
        {
            // Determine the current display speed
            float animationSpeed = pCGraphicView->GetAnimationSpeed ();

            // Convert the current display speed to a percentage
            m_iInitialPercent = int(animationSpeed*100.00F);
        }
    }

    // Set the range of the slider control
    m_speedSlider.SetRange (1, 200);

    // Set the initial pos of the slider control
    m_speedSlider.SetPos (m_iInitialPercent);
    return TRUE;
}

//////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
void
CAnimationSpeed::OnHScroll
(
    UINT nSBCode,
    UINT nPos,
    CScrollBar* pScrollBar
)
{
	// Get the current position of the slider control
    m_iInitialPercent = m_speedSlider.GetPos ();

    // Get a pointer to the main window
    CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
    if (pCMainWnd)
    {
        // Get a pointer to the graphic view pane
        CGraphicView *pCGraphicView = (CGraphicView *)pCMainWnd->GetPane (0, 1);
        if (pCGraphicView)
        {
            pCGraphicView->SetAnimationSpeed (((float)m_iInitialPercent) / (100.00F));
        }
    }

	// Allow the base class to process this message
    CDialog::OnHScroll (nSBCode, nPos, pScrollBar);
    return ;
}

//////////////////////////////////////////////////////////////
//
//  OnDestroy
//
void
CAnimationSpeed::OnDestroy (void)
{
    m_iInitialPercent = m_speedSlider.GetPos ();
	CDialog::OnDestroy();
    return ;
}

//////////////////////////////////////////////////////////////
//
//  OnBlend
//
void
CAnimationSpeed::OnBlend (void)
{
    // Get a pointer to the doc so we can get at the current scene
    // pointer.
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        // Turn on/off the blending option
        pCDoc->SetAnimationBlend (SendDlgItemMessage (IDC_BLEND, BM_GETCHECK));
    }

    return ;
}

void CAnimationSpeed::
OnCompressq(){
/*	CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	if(pCDoc){
		bool b_was_compressed = pCDoc->GetChannelQCompression();
		bool b_compress = IsDlgButtonChecked(IDC_COMPRESSQ) == BST_CHECKED;
		pCDoc->SetChannelQCompression(b_compress);
		//Enable/Disable
		if(b_compress){
				GetDlgItem(IDC_16BIT)->EnableWindow(TRUE);
				GetDlgItem(IDC_8BIT)->EnableWindow(TRUE);
		  }else{
			  GetDlgItem(IDC_16BIT)->EnableWindow(FALSE);
			  GetDlgItem(IDC_8BIT)->EnableWindow(FALSE);
		  }
		//Update
		  if(b_compress != b_was_compressed){
			int n_bytes = pCDoc->GetChannelQnBytes();
			CompressQ = b_compress;
			QnBytes = n_bytes;
		}
	}
*/
}

void CAnimationSpeed::
On16bit(){
/*
CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	pCDoc->SetChannelQnBytes(2);
	QnBytes = 2;
*/
}
void CAnimationSpeed::
On8bit(){
/*
	CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
	pCDoc->SetChannelQnBytes(1);
	QnBytes = 1;
*/
}
