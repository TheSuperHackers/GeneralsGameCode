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
#include "brushoptions.h"
#include "WorldBuilderView.h"
#include "BrushTool.h"

BrushOptions *BrushOptions::m_staticThis = NULL;
Int BrushOptions::m_currentWidth = 0;
Int BrushOptions::m_currentHeight = 0;
Int BrushOptions::m_currentFeather = 0;
Int BrushOptions::m_currentRaiseLower = BrushTool::MIN_RAISE_LOWER;
Int BrushOptions::m_currentSmoothRadius = BrushTool::MIN_SMOOTH_RADIUS;
Int BrushOptions::m_currentSmoothRate = BrushTool::MIN_SMOOTH_RATE;
/////////////////////////////////////////////////////////////////////////////
/// BrushOptions dialog trivial construstor - Create does the real work.


BrushOptions::BrushOptions(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(BrushOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/// Windows default stuff.
void BrushOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BrushOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

/// Sets the feather value in the dialog.
/** Update the value in the edit control and the slider. */
void BrushOptions::setFeather(Int feather)
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
void BrushOptions::setWidth(Int width)
{
	CString buf;
	buf.Format("%d", width);
	m_currentWidth = width;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}

void BrushOptions::setHeight(Int height)
{
	char buffer[12];
	snprintf(buffer, ARRAY_SIZE(buffer), "%d", height);
	m_currentHeight = height;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
		if (pEdit) pEdit->SetWindowText(buffer);
	}
}

void BrushOptions::setRaiseLowerAmount(Int amount)
{
	char buffer[12];
	snprintf(buffer, ARRAY_SIZE(buffer), "%d", amount);
	m_currentRaiseLower = amount;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RAISELOWER_EDIT);
		if (pEdit) pEdit->SetWindowText(buffer);
		snprintf(buffer, ARRAY_SIZE(buffer), "%.1f FEET.", m_currentRaiseLower*MAP_HEIGHT_SCALE);
		pEdit = m_staticThis->GetDlgItem(IDC_RAISELOWER_LABEL);
		if (pEdit) pEdit->SetWindowText(buffer);
	}
}

void BrushOptions::setSmoothRadius(Int radius)
{
	CString buf;
	buf.Format("%d", radius);
	m_currentSmoothRadius = radius;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RADIUS_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}

void BrushOptions::setSmoothRate(Int rate)
{
	CString buf;
	buf.Format("%d", rate);
	m_currentSmoothRate = rate;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RATE_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}



/////////////////////////////////////////////////////////////////////////////
// BrushOptions message handlers

/// Dialog UI initialization.
/** Creates the slider controls, and sets the initial values for
width and feather in the ui controls. */
BOOL BrushOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_updating = true;
	m_brushWidthPopup.SetupPopSliderButton(this, IDC_SIZE_POPUP, this);
	m_brushFeatherPopup.SetupPopSliderButton(this, IDC_FEATHER_POPUP, this);
	m_brushHeightPopup.SetupPopSliderButton(this, IDC_HEIGHT_POPUP, this);
	m_raiseLowerPopup.SetupPopSliderButton(this, IDC_RAISELOWER_POPUP, this);
	m_smoothRadiusPopup.SetupPopSliderButton(this, IDC_RADIUS_POPUP, this);
	m_smoothRatePopup.SetupPopSliderButton(this, IDC_RATE_POPUP, this);


	m_staticThis = this;
	m_updating = false;
	setFeather(m_currentFeather);
	setWidth(m_currentWidth);
	setHeight(m_currentHeight);
	setRaiseLowerAmount(m_currentRaiseLower);
	setSmoothRadius(m_currentSmoothRadius);
	setSmoothRate(m_currentSmoothRate);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/// Handles feather edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void BrushOptions::OnChangeFeatherEdit()
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
				BrushTool::setFeather(m_currentFeather);
				snprintf(buffer, ARRAY_SIZE(buffer), "%.1f FEET.", m_currentFeather*MAP_XY_FACTOR);
				pEdit = m_staticThis->GetDlgItem(IDC_FEATHER_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void BrushOptions::OnChangeSizeEdit()
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
				BrushTool::setWidth(m_currentWidth);
				snprintf(buffer, ARRAY_SIZE(buffer), "%.1f FEET.", m_currentWidth*MAP_XY_FACTOR);
				pEdit = m_staticThis->GetDlgItem(IDC_WIDTH_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void BrushOptions::OnChangeHeightEdit()
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
				BrushTool::setHeight(m_currentHeight);
				snprintf(buffer, ARRAY_SIZE(buffer), "%.1f FEET.", m_currentHeight*MAP_HEIGHT_SCALE);
				pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

void BrushOptions::OnChangeRaiseLowerEdit()
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RAISELOWER_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int amount;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &amount)) {
				m_currentRaiseLower = amount;
				BrushTool::setRaiseLowerAmount(m_currentRaiseLower);
				snprintf(buffer, ARRAY_SIZE(buffer), "%.1f FEET.", m_currentRaiseLower*MAP_HEIGHT_SCALE);
				pEdit = m_staticThis->GetDlgItem(IDC_RAISELOWER_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

void BrushOptions::OnChangeSmoothRadiusEdit()
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RADIUS_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int radius;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &radius)) {
				m_currentSmoothRadius = radius;
				BrushTool::setSmoothRadius(m_currentSmoothRadius);
			}
			m_updating = false;
		}
}

void BrushOptions::OnChangeSmoothRateEdit()
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RATE_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int rate;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &rate)) {
				m_currentSmoothRate = rate;
				BrushTool::setSmoothRate(m_currentSmoothRate);
			}
			m_updating = false;
		}
}

void BrushOptions::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			*pMin = 1;
			*pMax = 30;
			*pInitial = m_currentWidth;
			*pLineSize = 1;
			break;

		case IDC_HEIGHT_POPUP:
			*pMin = 0;
			*pMax = 255;
			*pInitial = m_currentHeight;
			*pLineSize = 1;
			break;

		case IDC_FEATHER_POPUP:
			*pMin = 0;
			*pMax = 20;
			*pInitial = m_currentFeather;
			*pLineSize = 1;
			break;

		case IDC_RAISELOWER_POPUP:
			*pMin = BrushTool::MIN_RAISE_LOWER;
			*pMax = BrushTool::MAX_RAISE_LOWER;
			*pInitial = m_currentRaiseLower;
			*pLineSize = 1;
			break;

		case IDC_RADIUS_POPUP:
			*pMin = BrushTool::MIN_SMOOTH_RADIUS;
			*pMax = BrushTool::MAX_SMOOTH_RADIUS;
			*pInitial = m_currentSmoothRadius;
			*pLineSize = 1;
			break;

		case IDC_RATE_POPUP:
			*pMin = BrushTool::MIN_SMOOTH_RATE;
			*pMax = BrushTool::MAX_SMOOTH_RATE;
			*pInitial = m_currentSmoothRate;
			*pLineSize = 1;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}
}

void BrushOptions::PopSliderChanged(const long sliderID, long theVal)
{
	CString str;
	CWnd *pEdit;
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			m_currentWidth = theVal;
			str.Format("%d",m_currentWidth);
			pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			BrushTool::setWidth(m_currentWidth);
			break;

		case IDC_HEIGHT_POPUP:
			m_currentHeight = theVal;
			str.Format("%d",m_currentHeight);
			pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			BrushTool::setHeight(m_currentHeight);
			break;

		case IDC_FEATHER_POPUP:
			m_currentFeather = theVal;
			str.Format("%d",m_currentFeather);
			pEdit = m_staticThis->GetDlgItem(IDC_FEATHER_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			BrushTool::setFeather(m_currentFeather);
			break;

		case IDC_RAISELOWER_POPUP:
			m_currentRaiseLower = theVal;
			str.Format("%d", m_currentRaiseLower);
			pEdit = m_staticThis->GetDlgItem(IDC_RAISELOWER_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			BrushTool::setRaiseLowerAmount(m_currentRaiseLower);
			break;

		case IDC_RADIUS_POPUP:
			m_currentSmoothRadius = theVal;
			str.Format("%d", m_currentSmoothRadius);
			pEdit = m_staticThis->GetDlgItem(IDC_RADIUS_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			BrushTool::setSmoothRadius(m_currentSmoothRadius);
			break;

		case IDC_RATE_POPUP:
			m_currentSmoothRate = theVal;
			str.Format("%d", m_currentSmoothRate);
			pEdit = m_staticThis->GetDlgItem(IDC_RATE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			BrushTool::setSmoothRate(m_currentSmoothRate);
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}
}

void BrushOptions::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_SIZE_POPUP:
			break;
		case IDC_HEIGHT_POPUP:
			break;
		case IDC_FEATHER_POPUP:
			break;
	case IDC_RAISELOWER_POPUP:
			break;
	case IDC_RADIUS_POPUP:
			break;
	case IDC_RATE_POPUP:
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}

}


BEGIN_MESSAGE_MAP(BrushOptions, COptionsPanel)
	//{{AFX_MSG_MAP(BrushOptions)
	ON_EN_CHANGE(IDC_FEATHER_EDIT, OnChangeFeatherEdit)
	ON_EN_CHANGE(IDC_SIZE_EDIT, OnChangeSizeEdit)
	ON_EN_CHANGE(IDC_HEIGHT_EDIT, OnChangeHeightEdit)
	ON_EN_CHANGE(IDC_RAISELOWER_EDIT, OnChangeRaiseLowerEdit)
	ON_EN_CHANGE(IDC_RADIUS_EDIT, OnChangeSmoothRadiusEdit)
	ON_EN_CHANGE(IDC_RATE_EDIT, OnChangeSmoothRateEdit)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


