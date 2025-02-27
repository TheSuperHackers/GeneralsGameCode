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

// Mike Lytle
// 01/07/03
// RulerOptions.cpp

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "RulerOptions.h"
#include "WorldBuilderView.h"
#include "RulerTool.h"

RulerOptions*	RulerOptions::m_staticThis = NULL;

/////////////////////////////////////////////////////////////////////////////
RulerOptions::RulerOptions(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(RulerOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/// Windows default stuff.
void RulerOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RulerOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


/** Update the value in the edit control. */
void RulerOptions::setWidth(Real width)
{
	CString buf;
	// Multiply by 2 because we are changing from radius to diameter.
	buf.Format("%f", width * 2.0f);
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RULER_WIDTH);
		if (pEdit && pEdit->IsWindowEnabled()) {
			pEdit->SetWindowText(buf);
		}
	}
}


/// Dialog UI initialization.
BOOL RulerOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_staticThis = this;
	m_updating = false;
	if (RulerTool::getType() != RULER_CIRCLE) {
		CWnd *pEdit = GetDlgItem(IDC_RULER_WIDTH);
		if (pEdit) {
			// Disable the edit box since the ruler isn't a circle.
			pEdit->EnableWindow(false);
		}
	} else {
		// Only the circle rulers use the edit box to change the size.
		setWidth(RulerTool::getLength());
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void RulerOptions::OnChangeWidthEdit()
{
	if (m_updating) return;
	CWnd *pEdit = GetDlgItem(IDC_RULER_WIDTH);
	char buffer[_MAX_PATH];
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		float width;
		m_updating = true;
		// Pull out the length from the text.
		if (1 == sscanf(buffer, "%f", &width)) {
			// Change from diameter to radius.
			RulerTool::setLength(width / 2.0f);
		}
		m_updating = false;
	}
}

void RulerOptions::OnChangeCheckRuler()
{
	if (m_updating) return;
	CWnd *pCheck = GetDlgItem(IDC_CHECK_RULER);
	if (pCheck) {
		if (RulerTool::switchType()) {
			CWnd *pEdit = GetDlgItem(IDC_RULER_WIDTH);
			if (pEdit) {
				// The edit box is disabled when the ruler is a line.
				pEdit->EnableWindow(!pEdit->IsWindowEnabled());
				// Make sure that the length is updated.
				if (pEdit->IsWindowEnabled()) {
					setWidth(RulerTool::getLength());
				}
			}
		}
	}
}




BEGIN_MESSAGE_MAP(RulerOptions, COptionsPanel)
	//{{AFX_MSG_MAP(RulerOptions)
	ON_EN_CHANGE(IDC_RULER_WIDTH, OnChangeWidthEdit)
	ON_BN_CLICKED(IDC_CHECK_RULER, OnChangeCheckRuler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

