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

// ScriptActionsTrue.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "ScriptActionsTrue.h"
#include "GameLogic/Scripts.h"
#include "EditAction.h"
#include "ScriptDialog.h"

/////////////////////////////////////////////////////////////////////////////
// ScriptActionsTrue property page

IMPLEMENT_DYNCREATE(ScriptActionsTrue, CPropertyPage)

ScriptActionsTrue::ScriptActionsTrue() : CPropertyPage(ScriptActionsTrue::IDD),
m_action(NULL),
m_index(0)
{
	//{{AFX_DATA_INIT(ScriptActionsTrue)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ScriptActionsTrue::~ScriptActionsTrue()
{
}

void ScriptActionsTrue::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScriptActionsTrue)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ScriptActionsTrue, CPropertyPage)
	//{{AFX_MSG_MAP(ScriptActionsTrue)
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
// ScriptActionsTrue message handlers

BOOL ScriptActionsTrue::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	CWnd *pWnd = GetDlgItem(IDC_EDIT_COMMENT);
	pWnd->SetWindowText(m_script->getActionComment().str());
	loadList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ScriptActionsTrue::loadList(void)
{
	m_action = NULL;
	ScriptDialog::updateScriptWarning(m_script);
	CListBox *pList = (CListBox *)GetDlgItem(IDC_ACTION_LIST);
	Int count = 0;
	if (pList) {
		pList->ResetContent();
		ScriptAction *pAction = m_script->getAction();
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


void ScriptActionsTrue::OnEditAction()
{
	CListBox *pList = (CListBox *)GetDlgItem(IDC_ACTION_LIST);
	if (m_action == NULL) {
		return;
	}
	EditAction cDlg;
	cDlg.setAction(m_action);
	if (cDlg.DoModal() == IDOK)
	{
		ScriptDialog::updateScriptWarning(m_script);
		pList->DeleteString(m_index);
		pList->InsertString(m_index, m_action->getUiText().str());
		pList->SetCurSel(m_index);
	}
}

void ScriptActionsTrue::enableUI()
{
	CWnd *pWnd = GetDlgItem(IDC_EDIT);
	pWnd->EnableWindow(m_action!=NULL);

	pWnd = GetDlgItem(IDC_COPY);
	pWnd->EnableWindow(m_action!=NULL);

	pWnd = GetDlgItem(IDC_DELETE);
	pWnd->EnableWindow(m_action!=NULL);

	pWnd = GetDlgItem(IDC_MOVE_DOWN);
	pWnd->EnableWindow(m_action && m_action->getNext());

	pWnd = GetDlgItem(IDC_MOVE_UP);
	pWnd->EnableWindow(m_action && m_index>0);

}

void ScriptActionsTrue::OnSelchangeActionList()
{
	m_action = NULL;
	CListBox *pList = (CListBox *)GetDlgItem(IDC_ACTION_LIST);
	if (pList) {
		Int count = pList->GetCurSel();
		m_index = count;
		if (count<0) {
			enableUI();
			return;
		}
		count++;
		m_action = m_script->getAction();
		while (m_action) {
			count--;
			if (count==0) {
				enableUI(); // Enable buttons based on selection.
				return;
			}
			m_action = m_action->getNext();
		}
	}
	enableUI(); // Enable buttons based on selection.
}

void ScriptActionsTrue::OnDblclkActionList()
{
	OnEditAction();
}



void ScriptActionsTrue::OnNew()
{
	ScriptAction *pAct = newInstance( ScriptAction)(ScriptAction::DEBUG_MESSAGE_BOX);
	EditAction aDlg;
	aDlg.setAction(pAct);
	if (IDOK==aDlg.DoModal()) {
		if (m_action) {
			pAct->setNextAction(m_action->getNext());
			m_action->setNextAction(pAct);
		} else {
			pAct->setNextAction(m_script->getAction());
			m_script->setAction(pAct);
		}
		m_index++;
		loadList();
	} else {
		deleteInstance(pAct);
	}
}

void ScriptActionsTrue::OnDelete()
{
	if (m_action) {
		m_script->deleteAction(m_action);
		loadList();
	}
}

void ScriptActionsTrue::OnCopy()
{
	if (m_action) {
		ScriptAction *pCopy = m_action->duplicate();
		pCopy->setNextAction(m_action->getNext());
		m_action->setNextAction(pCopy);
		m_index++;
		loadList();
	}
}

Bool ScriptActionsTrue::doMoveDown()
{
	if (m_action && m_action->getNext()) {
		ScriptAction *pNext = m_action->getNext();
		ScriptAction *pCur = m_script->getAction();
		ScriptAction *pPrev = NULL;
		while (pCur != m_action) {
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
			DEBUG_ASSERTCRASH(m_action == m_script->getAction(), ("Logic error."));
			pCur->setNextAction(pNext->getNext());
			pNext->setNextAction(pCur);
			m_script->setAction(pNext);
		}
		return true;
	}
	return false;
}

void ScriptActionsTrue::OnMoveDown()
{
	if (doMoveDown()) {
		m_index++;
		loadList();
	}
}

void ScriptActionsTrue::OnMoveUp()
{
	if (m_action && m_index>0) {
//		ScriptAction *pNext = m_action;
		ScriptAction *pPrev = m_script->getAction();
		while (pPrev->getNext() != m_action) {
			pPrev = pPrev->getNext();
		}
		if (pPrev) {
			m_action = pPrev;
			m_index--;
			if (doMoveDown()) {
				loadList();
			}
		}
	}
}

void ScriptActionsTrue::OnChangeEditComment()
{
	CWnd *pWnd = GetDlgItem(IDC_EDIT_COMMENT);
	CString comment;
	pWnd->GetWindowText(comment);
	m_script->setActionComment(AsciiString((LPCTSTR)comment));
}
