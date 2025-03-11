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

// ScriptActionsFalse.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "ScriptActionsFalse.h"
#include "GameLogic/Scripts.h"
#include "EditAction.h"	
#include "ScriptDialog.h"

/////////////////////////////////////////////////////////////////////////////
// ScriptActionsFalse property page

IMPLEMENT_DYNCREATE(ScriptActionsFalse, CPropertyPage)

ScriptActionsFalse::ScriptActionsFalse() : CPropertyPage(ScriptActionsFalse::IDD),
m_falseAction(NULL),
m_index(0)
{
	//{{AFX_DATA_INIT(ScriptActionsFalse)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ScriptActionsFalse::~ScriptActionsFalse()
{
}

void ScriptActionsFalse::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScriptActionsFalse)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ScriptActionsFalse, CPropertyPage)
	//{{AFX_MSG_MAP(ScriptActionsFalse)
	ON_BN_CLICKED(IDC_EDIT, OnEditAction)
	ON_LBN_SELCHANGE(IDC_ACTION_LIST, OnSelchangeActionList)
	ON_LBN_DBLCLK(IDC_ACTION_LIST, OnDblclkActionList)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_EN_CHANGE(IDC_EDIT_COMMENT, OnChangeEditComment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptActionsFalse message handlers

BOOL ScriptActionsFalse::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	CWnd *pWnd = GetDlgItem(IDC_EDIT_COMMENT);
	pWnd->SetWindowText(m_script->getActionComment().str());
	loadList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ScriptActionsFalse::loadList(void)
{
	m_falseAction = NULL;
	ScriptDialog::updateScriptWarning(m_script);
	CListBox *pList = (CListBox *)GetDlgItem(IDC_ACTION_LIST);
	Int count = 0;
	if (pList) {
		pList->ResetContent();
		ScriptAction *pAction = m_script->getFalseAction();
		while (pAction) {
			AsciiString astr = pAction->getUiText();
			if (astr.isEmpty()) {
				astr = "Invalid Action";
			}

			pList->AddString(astr.str());
			pAction = pAction->getNext();
			count++;
		}
		if (count>0 && count<=m_index) {
			m_index = count-1;
		}
		pList->SetCurSel(m_index);
		OnSelchangeActionList();
	}	
}


void ScriptActionsFalse::OnEditAction() 
{
	CListBox *pList = (CListBox *)GetDlgItem(IDC_ACTION_LIST);
	if (m_falseAction == NULL) {
		return;
	}
	EditAction cDlg;
	cDlg.setAction(m_falseAction);
	cDlg.DoModal();
	ScriptDialog::updateScriptWarning(m_script);
	pList->DeleteString(m_index);
	pList->InsertString(m_index, m_falseAction->getUiText().str());
	pList->SetCurSel(m_index);
}

void ScriptActionsFalse::enableUI() 
{
	CWnd *pWnd = GetDlgItem(IDC_EDIT);
	pWnd->EnableWindow(m_falseAction!=NULL);
	
	pWnd = GetDlgItem(IDC_COPY);
	pWnd->EnableWindow(m_falseAction!=NULL);

	pWnd = GetDlgItem(IDC_DELETE);
	pWnd->EnableWindow(m_falseAction!=NULL);
	
	pWnd = GetDlgItem(IDC_MOVE_DOWN);
	pWnd->EnableWindow(m_falseAction && m_falseAction->getNext());

	pWnd = GetDlgItem(IDC_MOVE_UP);
	pWnd->EnableWindow(m_falseAction && m_index>0);
	
}

void ScriptActionsFalse::OnSelchangeActionList() 
{
	m_falseAction = NULL;
	CListBox *pList = (CListBox *)GetDlgItem(IDC_ACTION_LIST);
	if (pList) {
		Int count = pList->GetCurSel();
		m_index = count;
		if (count<0) {
			enableUI();
			return;
		}
		count++;
		m_falseAction = m_script->getFalseAction();
		while (m_falseAction) {
			count--;
			if (count==0) {
				enableUI(); // Enable buttons based on selection.
				return;
			}
			m_falseAction = m_falseAction->getNext();
		}
	}	
	enableUI(); // Enable buttons based on selection.
}

void ScriptActionsFalse::OnDblclkActionList() 
{
	OnEditAction();
}



void ScriptActionsFalse::OnNew() 
{
	ScriptAction *pAct = newInstance( ScriptAction)(ScriptAction::DEBUG_MESSAGE_BOX);
	EditAction aDlg;
	aDlg.setAction(pAct);
	if (IDOK==aDlg.DoModal()) {
		if (m_falseAction) {
			pAct->setNextAction(m_falseAction->getNext());
			m_falseAction->setNextAction(pAct);
		} else {
			pAct->setNextAction(m_script->getFalseAction());
			m_script->setFalseAction(pAct);
		} 
		m_index++;
		loadList();
	} else {
		pAct->deleteInstance();
	}
}

void ScriptActionsFalse::OnDelete() 
{
	if (m_falseAction) {
		m_script->deleteFalseAction(m_falseAction);
		loadList();
	}
}

void ScriptActionsFalse::OnCopy() 
{
	if (m_falseAction) {
		ScriptAction *pCopy = m_falseAction->duplicate();
		pCopy->setNextAction(m_falseAction->getNext());
		m_falseAction->setNextAction(pCopy);
		m_index++;
		loadList();
	}
}

Bool ScriptActionsFalse::doMoveDown() 
{
	if (m_falseAction && m_falseAction->getNext()) {
		ScriptAction *pNext = m_falseAction->getNext();
		ScriptAction *pCur = m_script->getFalseAction();
		ScriptAction *pPrev = NULL;
		while (pCur != m_falseAction) {
			pPrev = pCur;
			pCur = pCur->getNext();
		}
		DEBUG_ASSERTCRASH(pCur, ("Didn't find action in list."));
		if (!pCur) return false;
		if (pPrev) {
			pPrev->setNextAction(pNext);
			pCur->setNextAction(pNext->getNext());
			pNext->setNextAction(pCur);
		} else {
			DEBUG_ASSERTCRASH(m_falseAction == m_script->getFalseAction(), ("Logic error."));
			pCur->setNextAction(pNext->getNext());
			pNext->setNextAction(pCur);
			m_script->setFalseAction(pNext);
		}
		return true;
	}
	return false;
}

void ScriptActionsFalse::OnMoveDown() 
{
	if (doMoveDown()) {
		m_index++;
		loadList();
	}
}

void ScriptActionsFalse::OnMoveUp() 
{
	if (m_falseAction && m_index>0) {
//		ScriptAction *pNext = m_falseAction;
		ScriptAction *pPrev = m_script->getFalseAction();
		while (pPrev->getNext() != m_falseAction) {
			pPrev = pPrev->getNext();
		}
		if (pPrev) {
			m_falseAction = pPrev;
			m_index--;
			if (doMoveDown()) {
				loadList();
			}
		}
	}
}

void ScriptActionsFalse::OnChangeEditComment() 
{
	CWnd *pWnd = GetDlgItem(IDC_EDIT_COMMENT);
	CString comment;
	pWnd->GetWindowText(comment);
	m_script->setActionComment(AsciiString((LPCTSTR)comment));
}
