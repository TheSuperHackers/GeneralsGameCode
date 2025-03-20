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

// MoundOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "MoundOptions.h"
#include "WorldBuilderView.h"
#include "MoundTool.h"

MoundOptions *MoundOptions::m_staticThis = NULL;
Int MoundOptions::m_currentWidth = 0;
Int MoundOptions::m_currentHeight = 0;
Int MoundOptions::m_currentFeather = 0;
/////////////////////////////////////////////////////////////////////////////
/// MoundOptions dialog trivial construstor - Create does the real work.


MoundOptions::MoundOptions(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(MoundOptions) 
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/// Windows default stuff.
void MoundOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MoundOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

/// Sets the feather value in the dialog.
/** Update the value in the edit control and the slider. */
void MoundOptions::setFeather(Int feather) 
{ 
	CString buf;
	buf.Format("%d", feather);
	m_currentFeather = feather;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_FEATHER_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}

/// Sets the brush width value in the dialog.
/** Update the value in the edit control and the slider. */
void MoundOptions::setWidth(Int width) 
{ 
	CString buf;
	buf.Format("%d", width);
	m_currentWidth = width;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}

void MoundOptions::setHeight(Int height) 
{ 
	char buffer[50];
	sprintf(buffer, "%d", height);
	m_currentHeight = height;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
		if (pEdit) pEdit->SetWindowText(buffer);
	}
}



/////////////////////////////////////////////////////////////////////////////
// MoundOptions message handlers

/// Dialog UI initialization.
/** Creates the slider controls, and sets the initial values for 
width and feather in the ui controls. */
BOOL MoundOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_updating = true;
	m_brushWidthPopup.SetupPopSliderButton(this, IDC_SIZE_POPUP, this);
	m_brushFeatherPopup.SetupPopSliderButton(this, IDC_FEATHER_POPUP, this);
	m_brushHeightPopup.SetupPopSliderButton(this, IDC_HEIGHT_POPUP, this);


	m_staticThis = this;
	m_updating = false;
	setFeather(m_currentFeather);
	setWidth(m_currentWidth);
	setHeight(m_currentHeight);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/// Handles feather edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void MoundOptions::OnChangeFeatherEdit() 
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_FEATHER_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int feather;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &feather)) {
				m_currentFeather = feather;
				MoundTool::setFeather(m_currentFeather);
				sprintf(buffer, "%.1f FEET.", m_currentFeather*MAP_XY_FACTOR);
				pEdit = m_staticThis->GetDlgItem(IDC_FEATHER_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void MoundOptions::OnChangeSizeEdit() 
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int width;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &width)) {
				m_currentWidth = width;
				MoundTool::setWidth(m_currentWidth);
				sprintf(buffer, "%.1f FEET.", m_currentWidth*MAP_XY_FACTOR);
				pEdit = m_staticThis->GetDlgItem(IDC_WIDTH_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void MoundOptions::OnChangeHeightEdit() 
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int height;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &height)) {
				m_currentHeight = height;
				MoundTool::setMoundHeight(m_currentHeight);
				sprintf(buffer, "%.1f FEET.", m_currentHeight*MAP_HEIGHT_SCALE);
				pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

void MoundOptions::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			*pMin = MIN_BRUSH_SIZE;
			*pMax = MAX_BRUSH_SIZE;
			*pInitial = m_currentWidth;
			*pLineSize = 1;
			break;

		case IDC_HEIGHT_POPUP:
			*pMin = MIN_MOUND_HEIGHT;
			*pMax = MAX_MOUND_HEIGHT;
			*pInitial = m_currentHeight;
			*pLineSize = 1;
			break;

		case IDC_FEATHER_POPUP:
			*pMin = MIN_FEATHER;
			*pMax = MAX_FEATHER;
			*pInitial = m_currentFeather;
			*pLineSize = 1;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void MoundOptions::PopSliderChanged(const long sliderID, long theVal)
{
	CString str;
	CWnd *pEdit;
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			m_currentWidth = theVal;
			str.Format("%d",m_currentWidth);
			pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			MoundTool::setWidth(m_currentWidth);
			break;

		case IDC_HEIGHT_POPUP:
			m_currentHeight = theVal;
			str.Format("%d",m_currentHeight);
			pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			MoundTool::setMoundHeight(m_currentHeight);
			break;

		case IDC_FEATHER_POPUP:
			m_currentFeather = theVal;
			str.Format("%d",m_currentFeather);
			pEdit = m_staticThis->GetDlgItem(IDC_FEATHER_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			MoundTool::setFeather(m_currentFeather);
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void MoundOptions::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_SIZE_POPUP:
			break;
		case IDC_HEIGHT_POPUP:
			break;
		case IDC_FEATHER_POPUP:
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch

}


BEGIN_MESSAGE_MAP(MoundOptions, COptionsPanel)
	//{{AFX_MSG_MAP(MoundOptions)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_FEATHER_EDIT, OnChangeFeatherEdit)
	ON_EN_CHANGE(IDC_SIZE_EDIT, OnChangeSizeEdit)
	ON_EN_CHANGE(IDC_HEIGHT_EDIT, OnChangeHeightEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

