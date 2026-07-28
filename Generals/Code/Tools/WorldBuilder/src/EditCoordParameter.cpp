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

// EditCoordParameter.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "EditCoordParameter.h"
#include "GameLogic/Scripts.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/PolygonTrigger.h"

/////////////////////////////////////////////////////////////////////////////
// EditCoordParameter dialog


EditCoordParameter::EditCoordParameter(CWnd* pParent /*=nullptr*/)
	: CDialog(EditCoordParameter::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditCoordParameter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EditCoordParameter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditCoordParameter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditCoordParameter, CDialog)
	//{{AFX_MSG_MAP(EditCoordParameter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditCoordParameter message handlers

BOOL EditCoordParameter::OnInitDialog()
{
	CDialog::OnInitDialog();

	CEdit *pEditX = (CEdit *)GetDlgItem(IDC_EDIT_X);
	CEdit *pEditY = (CEdit *)GetDlgItem(IDC_EDIT_Y);
	CEdit *pEditZ = (CEdit *)GetDlgItem(IDC_EDIT_Z);

	m_parameter->getCoord3D(&m_coord);
	CString string;
	string.Format("%.2f", m_coord.x);
	pEditX->SetWindowText(string);
	string.Format("%.2f", m_coord.y);
	pEditY->SetWindowText(string);
	string.Format("%.2f", m_coord.z);
	pEditZ->SetWindowText(string);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void EditCoordParameter::OnOK()
{
	CEdit *pEditX = (CEdit *)GetDlgItem(IDC_EDIT_X);
	CEdit *pEditY = (CEdit *)GetDlgItem(IDC_EDIT_Y);
	CEdit *pEditZ = (CEdit *)GetDlgItem(IDC_EDIT_Z);
	CString txt;

	pEditX->GetWindowText(txt);
	Real theReal;
	if (1==sscanf(txt, "%f", &theReal)) {
		m_coord.x = theReal;
	} else {
		pEditX->SetFocus();
		::MessageBeep(MB_ICONEXCLAMATION);
		return;
	}
	pEditY->GetWindowText(txt);
	if (1==sscanf(txt, "%f", &theReal)) {
		m_coord.y = theReal;
	} else {
		pEditX->SetFocus();
		::MessageBeep(MB_ICONEXCLAMATION);
		return;
	}
	pEditZ->GetWindowText(txt);
	if (1==sscanf(txt, "%f", &theReal)) {
		m_coord.z = theReal;
	} else {
		pEditX->SetFocus();
		::MessageBeep(MB_ICONEXCLAMATION);
		return;
	}
	m_parameter->friend_setCoord3D(&m_coord);
	CDialog::OnOK();
}

void EditCoordParameter::OnCancel()
{

	CDialog::OnCancel();
}
