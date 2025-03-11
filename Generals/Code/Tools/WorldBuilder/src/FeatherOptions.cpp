/*
**	Command & Conquer Generals(tm)
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

// brushoptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "FeatherOptions.h"
#include "WorldBuilderView.h"
#include "FeatherTool.h"

FeatherOptions *FeatherOptions::m_staticThis = NULL;
Int FeatherOptions::m_currentFeather = 0;
Int FeatherOptions::m_currentRate = 3;
Int FeatherOptions::m_currentRadius = 1;
/////////////////////////////////////////////////////////////////////////////
/// FeatherOptions dialog trivial construstor - Create does the real work.


FeatherOptions::FeatherOptions(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(FeatherOptions) 
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/// Windows default stuff.
void FeatherOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FeatherOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

/// Sets the feather value in the dialog.
/** Update the value in the edit control and the slider. */
void FeatherOptions::setFeather(Int feather) 
{ 
	CString buf;
	buf.Format("%d", feather);
	m_currentFeather = feather;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}


/// Sets the rate value in the dialog.
/** Update the value in the edit control and the slider. */
void FeatherOptions::setRate(Int rate) 
{ 
	CString buf;
	buf.Format("%d", rate);
	m_currentRate = rate;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RATE_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}


/// Sets the radius value in the dialog.
/** Update the value in the edit control and the slider. */
void FeatherOptions::setRadius(Int radius) 
{ 
	CString buf;
	buf.Format("%d", radius);
	m_currentRadius = radius;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RADIUS_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}



/////////////////////////////////////////////////////////////////////////////
// FeatherOptions message handlers

/// Dialog UI initialization.
/** Creates the slider controls, and sets the initial values for 
width and feather in the ui controls. */
BOOL FeatherOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_updating = true;
	m_featherPopup.SetupPopSliderButton(this, IDC_SIZE_POPUP, this);
	m_radiusPopup.SetupPopSliderButton(this, IDC_RADIUS_POPUP, this);
	m_ratePopup.SetupPopSliderButton(this, IDC_RATE_POPUP, this);


	m_staticThis = this;
	m_updating = false;
	setFeather(m_currentFeather);
	setRate(m_currentRate);
	setRadius(m_currentRadius);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void FeatherOptions::OnChangeSizeEdit() 
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int width;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &width)) {
				m_currentFeather = width;
				FeatherTool::setFeather(m_currentFeather);
				sprintf(buffer, "%.1f FEET.", m_currentFeather*MAP_XY_FACTOR);
				pEdit = m_staticThis->GetDlgItem(IDC_WIDTH_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}


void FeatherOptions::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			*pMin = MIN_FEATHER_SIZE;
			*pMax = MAX_FEATHER_SIZE;
			*pInitial = m_currentFeather;
			*pLineSize = 1;
			break;

		case IDC_RADIUS_POPUP:
			*pMin = MIN_RADIUS;
			*pMax = MAX_RADIUS;
			*pInitial = m_currentRadius;
			*pLineSize = 1;
			break;

		case IDC_RATE_POPUP:
			*pMin = MIN_RATE;
			*pMax = MAX_RATE;
			*pInitial = m_currentRate;
			*pLineSize = 1;
			break;


		default:
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void FeatherOptions::PopSliderChanged(const long sliderID, long theVal)
{
	CString str;
	CWnd *pEdit;
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			m_currentFeather = theVal;
			str.Format("%d",m_currentFeather);
			pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			FeatherTool::setFeather(m_currentFeather);
			break;

		case IDC_RADIUS_POPUP:
			m_currentRadius = theVal;
			str.Format("%d",m_currentRadius);
			pEdit = m_staticThis->GetDlgItem(IDC_RADIUS_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			FeatherTool::setRadius(m_currentRadius);
			break;

		case IDC_RATE_POPUP:
			m_currentRate = theVal;
			str.Format("%d",m_currentRate);
			pEdit = m_staticThis->GetDlgItem(IDC_RATE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			FeatherTool::setRate(m_currentRate);
			break;

		default:
			break;
	}	// switch
}

void FeatherOptions::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_SIZE_POPUP:
		case IDC_RADIUS_POPUP:
		case IDC_RATE_POPUP:
			break;

		default:
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch

}


BEGIN_MESSAGE_MAP(FeatherOptions, COptionsPanel)
	//{{AFX_MSG_MAP(FeatherOptions)
	ON_EN_CHANGE(IDC_SIZE_EDIT, OnChangeSizeEdit)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


