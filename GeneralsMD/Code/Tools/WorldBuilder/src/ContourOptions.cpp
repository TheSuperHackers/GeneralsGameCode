/*
**	Command & Conquer Generals Zero Hour(tm)
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

// ContourOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "ContourOptions.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"

Int ContourOptions::m_contourStep = 5;
Int ContourOptions::m_contourOffset = 0;
Int ContourOptions::m_contourWidth = 1;
/////////////////////////////////////////////////////////////////////////////
/// ContourOptions dialog trivial construstor - Create does the real work.


ContourOptions::ContourOptions(CWnd* pParent /*=NULL*/)
	: CDialog(ContourOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(ContourOptions) 
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/// Windows default stuff.
void ContourOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ContourOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}



/////////////////////////////////////////////////////////////////////////////
// ContourOptions message handlers

/// Dialog UI initialization.
/** Creates the slider controls, and sets the initial values for 
width and feather in the ui controls. */
BOOL ContourOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *pWnd = GetDlgItem(IDC_SLIDER1);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	pWnd->DestroyWindow();
	ScreenToClient(&rect);

	m_contourStepSlider.Create(TBS_HORZ|TBS_BOTTOM|TBS_AUTOTICKS, rect, this, IDC_SLIDER1);
	m_contourStepSlider.SetRange(MIN_CONTOUR_STEP,MAX_CONTOUR_STEP);
	m_contourStepSlider.SetTicFreq(1);
	m_contourStepSlider.SetPos(MIN_CONTOUR_STEP + MAX_CONTOUR_STEP-m_contourStep);
	m_contourStepSlider.ShowWindow(SW_SHOW);

	pWnd = GetDlgItem(IDC_SLIDER2);
	pWnd->GetWindowRect(&rect);

	pWnd->DestroyWindow();
	ScreenToClient(&rect);

	m_contourOffsetSlider.Create(TBS_HORZ|TBS_BOTTOM|TBS_AUTOTICKS, rect, this, IDC_SLIDER2);
	m_contourOffsetSlider.SetRange(MIN_CONTOUR_OFFSET,MAX_CONTOUR_OFFSET);
	m_contourOffsetSlider.SetTicFreq(1);
	m_contourOffsetSlider.SetPos(m_contourOffset);
	m_contourOffsetSlider.ShowWindow(SW_SHOW);

	pWnd = GetDlgItem(IDC_SLIDER3);
	pWnd->GetWindowRect(&rect);

	pWnd->DestroyWindow();
	ScreenToClient(&rect);

	m_contourWidthSlider.Create(TBS_HORZ|TBS_BOTTOM|TBS_AUTOTICKS, rect, this, IDC_SLIDER3);
	m_contourWidthSlider.SetRange(MIN_WIDTH,MAX_WIDTH);
	m_contourWidthSlider.SetTicFreq(1);
	m_contourWidthSlider.SetPos(m_contourWidth);
	m_contourWidthSlider.ShowWindow(SW_SHOW);

	CButton *pButton = (CButton*)GetDlgItem(IDC_SHOW_CONTOURS);
	CWorldBuilderView *pView = CWorldBuilderDoc::GetActive2DView();
	if (pButton && pView) pButton->SetCheck(pView->getShowContours()?1:0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/// Handles slider ui messages.
/** Gets the info, determines if it is the feather or width slider, 
		gets the new value, and updates the correspondig edit control
		and the brush tool. */
void ContourOptions::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CString str;
	m_updating = true;
	Bool step = (pScrollBar && pScrollBar->m_hWnd == m_contourStepSlider.m_hWnd);
	Bool offset = (pScrollBar && pScrollBar->m_hWnd == m_contourOffsetSlider.m_hWnd);
	if (step) {
		if (nSBCode != TB_THUMBTRACK) {
			nPos = m_contourStepSlider.GetPos();
		}
		m_contourStep = MIN_CONTOUR_STEP + MAX_CONTOUR_STEP - nPos;
	} else if (offset) { // width 
		if (nSBCode != TB_THUMBTRACK) {
			nPos = m_contourOffsetSlider.GetPos();
		}
		m_contourOffset = nPos;
	} else { // width 
		if (nSBCode != TB_THUMBTRACK) {
			nPos = m_contourWidthSlider.GetPos();
		}
		m_contourWidth = nPos;
	}
	if (nSBCode!=TB_THUMBTRACK) {
		CWorldBuilderView *pView = CWorldBuilderDoc::GetActive2DView();
		if (pView)
			pView->setCellSize(pView->getCellSize());
	}
	m_updating = false;
}


BEGIN_MESSAGE_MAP(ContourOptions, CDialog)
	//{{AFX_MSG_MAP(ContourOptions)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SHOW_CONTOURS, OnShowContours)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void ContourOptions::OnShowContours() 
{
	CWorldBuilderView *pView = CWorldBuilderDoc::GetActive2DView();
	if (pView) {
		Bool showContours = !pView->getShowContours();
		CButton *pButton = (CButton*)GetDlgItem(IDC_SHOW_CONTOURS);
		if (pButton) pButton->SetCheck(showContours?1:0);
		pView->setShowContours(showContours);
	}
}
