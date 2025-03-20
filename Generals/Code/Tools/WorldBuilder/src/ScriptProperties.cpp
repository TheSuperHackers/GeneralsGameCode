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

// ScriptProperties.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "ScriptProperties.h"
#include "GameLogic/Scripts.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma message("************************************** WARNING, optimization disabled for debugging purposes")
#endif
/////////////////////////////////////////////////////////////////////////////
// ScriptProperties property page

IMPLEMENT_DYNCREATE(ScriptProperties, CPropertyPage)

ScriptProperties::ScriptProperties() : m_updating(false), m_script(NULL),
CPropertyPage(ScriptProperties::IDD)
{
	//{{AFX_DATA_INIT(ScriptProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ScriptProperties::~ScriptProperties()
{
}

void ScriptProperties::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScriptProperties)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ScriptProperties, CPropertyPage)
	//{{AFX_MSG_MAP(ScriptProperties)
	ON_EN_CHANGE(IDC_SCRIPT_COMMENT, OnChangeScriptComment)
	ON_EN_CHANGE(IDC_SCRIPT_NAME, OnChangeScriptName)
	ON_BN_CLICKED(IDC_SCRIPT_ACTIVE, OnScriptActive)
	ON_BN_CLICKED(IDC_ONE_SHOT, OnOneShot)
	ON_BN_CLICKED(IDC_EASY, OnEasy)
	ON_BN_CLICKED(IDC_HARD, OnHard)
	ON_BN_CLICKED(IDC_NORMAL, OnNormal)
	ON_BN_CLICKED(IDC_SCRIPT_SUBROUTINE, OnScriptSubroutine)
	ON_BN_CLICKED(IDC_EVERY_FRAME, OnEveryFrame)
	ON_BN_CLICKED(IDC_EVERY_SECOND, OnEverySecond)
	ON_EN_CHANGE(IDC_SECONDS_EDIT, OnChangeSecondsEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptProperties message handlers

BOOL ScriptProperties::OnSetActive() 
{
	if ( CPropertyPage::OnSetActive()) {
		CWnd *pWnd = GetDlgItem(IDC_SCRIPT_COMMENT);
		pWnd->SetWindowText(m_script->getComment().str());
		
		pWnd = GetDlgItem(IDC_SCRIPT_NAME);
		pWnd->SetWindowText(m_script->getName().str());
		enableControls();
		return true;
	}
	return false;
}

void ScriptProperties::enableControls() 
{
	Bool isSubroutine = m_script->isSubroutine();
	CButton *pButton = (CButton*)GetDlgItem(IDC_SCRIPT_SUBROUTINE);
	pButton->SetCheck(isSubroutine ? 1:0);
	pButton = (CButton*)GetDlgItem(IDC_SCRIPT_ACTIVE);
	pButton->SetCheck(m_script->isActive() ? 1:0);
	//pButton->EnableWindow(!isSubroutine);
	pButton = (CButton*)GetDlgItem(IDC_ONE_SHOT);
	pButton->SetCheck(m_script->isOneShot() ? 1:0);
	//pButton->EnableWindow(!isSubroutine);
	pButton = (CButton*)GetDlgItem(IDC_EASY);
	pButton->SetCheck(m_script->isEasy() ? 1:0);
	//pButton->EnableWindow(!isSubroutine);
	pButton = (CButton*)GetDlgItem(IDC_NORMAL);
	pButton->SetCheck(m_script->isNormal() ? 1:0);
	//pButton->EnableWindow(!isSubroutine);
	pButton = (CButton*)GetDlgItem(IDC_HARD);
	pButton->SetCheck(m_script->isHard() ? 1:0);
	//pButton->EnableWindow(!isSubroutine);

	m_updating = true;
	Int delay = m_script->getDelayEvalSeconds();
	pButton = (CButton*)GetDlgItem(IDC_EVERY_SECOND);
	pButton->SetCheck(delay>0);
	pButton = (CButton*)GetDlgItem(IDC_EVERY_FRAME);
	pButton->SetCheck(delay==0);
	CString text="";
	if (delay>0) text.Format("%d", delay);
	GetDlgItem(IDC_SECONDS_EDIT)->SetWindowText(text);
	m_updating = false;
}

void ScriptProperties::OnChangeScriptComment() 
{
	CWnd *pWnd = GetDlgItem(IDC_SCRIPT_COMMENT);
	CString comment;
	pWnd->GetWindowText(comment);
	m_script->setComment(AsciiString((LPCTSTR)comment));
}

void ScriptProperties::OnChangeScriptName() 
{
	CWnd *pWnd = GetDlgItem(IDC_SCRIPT_NAME);
	CString name;
	pWnd->GetWindowText(name);
	m_script->setName(AsciiString((LPCTSTR)name));
	GetParent()->SetWindowText(name);
}

void ScriptProperties::OnScriptActive() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_SCRIPT_ACTIVE);
	m_script->setActive(pButton->GetCheck()==1);
}

void ScriptProperties::OnOneShot() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_ONE_SHOT);
	m_script->setOneShot(pButton->GetCheck()==1);
}

void ScriptProperties::OnEasy() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_EASY);
	m_script->setEasy(pButton->GetCheck()==1);
}

void ScriptProperties::OnHard() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_HARD);
	m_script->setHard(pButton->GetCheck()==1);
}

void ScriptProperties::OnNormal() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_NORMAL);
	m_script->setNormal(pButton->GetCheck()==1);
}

void ScriptProperties::OnScriptSubroutine() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_SCRIPT_SUBROUTINE);
	Bool isSubroutine = pButton->GetCheck()==1;
	m_script->setSubroutine(isSubroutine);

	enableControls();

}

void ScriptProperties::OnEveryFrame() 
{
	m_updating = true;
	CButton *pButton = (CButton*)GetDlgItem(IDC_EVERY_SECOND);
	pButton->SetCheck(0);
	pButton = (CButton*)GetDlgItem(IDC_EVERY_FRAME);
	pButton->SetCheck(1);
	GetDlgItem(IDC_SECONDS_EDIT)->SetWindowText("");
	m_script->setDelayEvalSeconds(0);
	m_updating = false;
}

void ScriptProperties::OnEverySecond() 
{
	m_updating = true;
	CButton *pButton = (CButton*)GetDlgItem(IDC_EVERY_SECOND);
	pButton->SetCheck(1);
	pButton = (CButton*)GetDlgItem(IDC_EVERY_FRAME);
	pButton->SetCheck(0);
	GetDlgItem(IDC_SECONDS_EDIT)->SetWindowText("1");
	m_script->setDelayEvalSeconds(1);
	m_updating = false;
}

void ScriptProperties::OnChangeSecondsEdit() 
{
	if (m_updating) return;
	CButton *pButton = (CButton*)GetDlgItem(IDC_EVERY_SECOND);
	pButton->SetCheck(1);
	pButton = (CButton*)GetDlgItem(IDC_EVERY_FRAME);
	pButton->SetCheck(0);
	CWnd *pEdit = GetDlgItem(IDC_SECONDS_EDIT);
	CString text;
	pEdit->GetWindowText(text);
	Int theInt;
	if (1==sscanf(text, "%d", &theInt)) {
		m_script->setDelayEvalSeconds(theInt);
	}
}
