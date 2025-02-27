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

// BaseBuildProps.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "BaseBuildProps.h"
#include "EditParameter.h"

/////////////////////////////////////////////////////////////////////////////
// BaseBuildProps dialog


BaseBuildProps::BaseBuildProps(CWnd* pParent /*=NULL*/)
	: CDialog(BaseBuildProps::IDD, pParent)
{
	//{{AFX_DATA_INIT(BaseBuildProps)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void BaseBuildProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BaseBuildProps)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BOOL BaseBuildProps::OnInitDialog()
{
	// add name
	CWnd* pName = GetDlgItem(IDC_MAPOBJECT_Name);
	if (pName) {
		pName->SetWindowText(m_name.str());
	}

	// add script
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Script);
	EditParameter::loadScripts(pCombo, true);
	pCombo->AddString("<none>");
	if (m_script.isEmpty()) {
		pCombo->SelectString(-1, "<none>");
	} else {
		pCombo->SelectString(-1, m_script.str());
	}

	// add health
	CWnd* pHealth = GetDlgItem(IDC_MAPOBJECT_StartingHealthEdit);
	static char buff[12];
	snprintf(buff, ARRAY_SIZE(buff), "%d", m_health);
	pHealth->SetWindowText(buff);

	CButton* pItem;
	pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Unsellable);
	if (pItem) {
		pItem->SetCheck(m_unsellable);
	}

	return TRUE;
}


BEGIN_MESSAGE_MAP(BaseBuildProps, CDialog)
	//{{AFX_MSG_MAP(BaseBuildProps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BaseBuildProps message handlers
void BaseBuildProps::setProps(AsciiString name, AsciiString script, Int health, Bool unsellable)
{
	m_name = name;
	m_script = script;
	m_health = health;
	m_unsellable = unsellable;
}

void BaseBuildProps::OnOK()
{
	CComboBox *combo;
	CWnd* edit;
	CString cstr;
	static char buf[1024];

	edit = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Name);
	edit->GetWindowText(buf, sizeof(buf)-2);
	m_name = AsciiString(buf);

	combo = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Script);
	combo->GetWindowText(buf, sizeof(buf)-2);
	m_script = AsciiString(buf);

	edit = GetDlgItem(IDC_MAPOBJECT_StartingHealthEdit);
	edit->GetWindowText(cstr);
	if (cstr.IsEmpty()) {
		m_health = 100;
	} else {
		m_health = atoi(cstr.GetBuffer(0));
		if (m_health < 0)
			m_health = 0;
	}

	CButton *check;
	check = (CButton*) GetDlgItem(IDC_MAPOBJECT_Unsellable);
	m_unsellable = (check->GetCheck() != 0);

	CDialog::OnOK();
}
