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

// ScriptConditions.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "ScriptConditions.h"
#include "GameLogic/Scripts.h"
#include "EditCondition.h"
#include "ScriptDialog.h"

/////////////////////////////////////////////////////////////////////////////
// ScriptConditionsDlg property page

IMPLEMENT_DYNCREATE(ScriptConditionsDlg, CPropertyPage)

ScriptConditionsDlg::ScriptConditionsDlg() : CPropertyPage(ScriptConditionsDlg::IDD),
m_condition(NULL),
m_orCondition(NULL),
m_index(0)
{
	//{{AFX_DATA_INIT(ScriptConditionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ScriptConditionsDlg::~ScriptConditionsDlg()
{
}

void ScriptConditionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScriptConditionsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ScriptConditionsDlg, CPropertyPage)
	//{{AFX_MSG_MAP(ScriptConditionsDlg)
	ON_BN_CLICKED(IDC_EDIT_CONDITION, OnEditCondition)
	ON_LBN_SELCHANGE(IDC_CONDITION_LIST, OnSelchangeConditionList)
	ON_LBN_DBLCLK(IDC_CONDITION_LIST, OnDblclkConditionList)
	ON_BN_CLICKED(IDC_OR, OnOr)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_EN_CHANGE(IDC_EDIT_COMMENT, OnChangeEditComment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptConditionsDlg message handlers

BOOL ScriptConditionsDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	loadList();
	CWnd *pWnd = GetDlgItem(IDC_EDIT_COMMENT);
	pWnd->SetWindowText(m_script->getConditionComment().str());
	CListBox *pList = (CListBox *)GetDlgItem(IDC_CONDITION_LIST);
	pList->SetHorizontalExtent(10000);	// band-aid fix until more precise length can be determined
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ScriptConditionsDlg::loadList(void)
{
	Int count = 0;
	ScriptDialog::updateScriptWarning(m_script);
	CListBox *pList = (CListBox *)GetDlgItem(IDC_CONDITION_LIST);
	if (pList) {
		pList->ResetContent();
		OrCondition *pOr = m_script->getOrCondition();
		while (pOr) {
			if (count==0) {
				pList->AddString("*** IF ***");
			} else {
				pList->AddString("*** OR ***");
			}
			Condition *pCond = pOr->getFirstAndCondition();
			Bool first = true;
			while (pCond) {
				AsciiString label;
				if (first) label = "  "; 
				else label = "  *AND* ";
				first = false;
				label.concat(pCond->getUiText());
				pList->AddString(label.str());
				pCond = pCond->getNext();
			}
			count++;
			pOr = pOr->getNextOrCondition();
		}
		if (count>0) pList->SetCurSel(1);
		OnSelchangeConditionList();
	}	
}


void ScriptConditionsDlg::OnEditCondition() 
{
	CListBox *pList = (CListBox *)GetDlgItem(IDC_CONDITION_LIST);
	if (m_condition == NULL) {
		return;
	}
	EditCondition cDlg;
	cDlg.setCondition(m_condition);
	cDlg.DoModal();
	ScriptDialog::updateScriptWarning(m_script);
	pList->DeleteString(m_index);
	AsciiString label;
	Bool first;
	if (m_orCondition && m_orCondition->getFirstAndCondition() == m_condition) {
		first = true;
	}
	if (first) label = "  "; 
	else label = "    AND ";
	label.concat(m_condition->getUiText());
	pList->InsertString(m_index, label.str());
}

void ScriptConditionsDlg::enableUI() 
{
	CWnd *pWnd = GetDlgItem(IDC_EDIT_CONDITION);
	pWnd->EnableWindow(m_condition!=NULL);
	
	pWnd = GetDlgItem(IDC_COPY);
	pWnd->EnableWindow(m_condition!=NULL);

	pWnd = GetDlgItem(IDC_DELETE);
	pWnd->EnableWindow(m_condition || m_orCondition);
}

void ScriptConditionsDlg::setSel(OrCondition *pOr, Condition *pCond)
{
	m_orCondition = NULL;
	m_condition = NULL;
	CListBox *pList = (CListBox *)GetDlgItem(IDC_CONDITION_LIST);
	if (pList) {
		pList->SetCurSel(-1);
		Int count = 0;
		m_orCondition = m_script->getOrCondition();
		while (m_orCondition) {
			if (m_orCondition==pOr && pCond==NULL) {
				pList->SetCurSel(count);
				enableUI();
				return;
			}
			count++;
			m_condition = m_orCondition->getFirstAndCondition();
			while (m_condition) {
				if (m_condition == pCond) {
					pList->SetCurSel(count);
					enableUI();
					return;
				}
				count++;
				m_condition = m_condition->getNext();
			}
			m_orCondition = m_orCondition->getNextOrCondition();
		}
	}	
	enableUI();
}

void ScriptConditionsDlg::OnSelchangeConditionList() 
{
	m_orCondition = NULL;
	m_condition = NULL;
	CListBox *pList = (CListBox *)GetDlgItem(IDC_CONDITION_LIST);
	if (pList) {
		Int count = pList->GetCurSel();
		m_index = count;
		if (count<0) return;
		count+=1; 
		m_orCondition = m_script->getOrCondition();
		while (m_orCondition) {
			count--;
			if (count==0) {
				enableUI(); // Enable buttons based on selection.
				return;
			}
			m_condition = m_orCondition->getFirstAndCondition();
			while (m_condition) {
				count--;
				if (count==0) {
					enableUI(); // Enable buttons based on selection.
					return;
				}
				m_condition = m_condition->getNext();
			}
			m_orCondition = m_orCondition->getNextOrCondition();
		}
	}	
}

void ScriptConditionsDlg::OnDblclkConditionList() 
{
	OnEditCondition();
}


void ScriptConditionsDlg::OnOr() 
{
	OrCondition *pOr = newInstance( OrCondition);
	if (m_orCondition) {
		pOr->setNextOrCondition(m_orCondition->getNextOrCondition());
		m_orCondition->setNextOrCondition(pOr);
	} else {
		pOr->setNextOrCondition(m_script->getOrCondition());
		m_script->setOrCondition(pOr);
	}	
	loadList();
	setSel(pOr, NULL);
}

void ScriptConditionsDlg::OnNew() 
{
	Condition *pCond = newInstance( Condition)(Condition::CONDITION_TRUE);
	EditCondition cDlg;
	cDlg.setCondition(pCond);
	if (IDOK==cDlg.DoModal()) {
		OrCondition *pSavOr = m_orCondition;
		if (m_condition) {
			pCond->setNextCondition(m_condition->getNext());
			m_condition->setNextCondition(pCond);
		} else {
			if (m_orCondition == NULL) {
				OrCondition *pOr = newInstance( OrCondition);
				pOr->setNextOrCondition(m_script->getOrCondition());
				m_script->setOrCondition(pOr);
				m_orCondition = pOr;
			}
			pCond->setNextCondition(m_orCondition->getFirstAndCondition());
			m_orCondition->setFirstAndCondition(pCond);
		} 
		loadList();
		setSel(pSavOr, pCond);
	} else {
		MemoryPoolObject::deleteInstance(pCond);
	}
}

void ScriptConditionsDlg::OnDelete() 
{
	if (m_condition && m_orCondition) {
		m_orCondition->deleteCondition(m_condition);
		loadList();
	} else if (m_orCondition) {
		m_script->deleteOrCondition(m_orCondition);
		loadList();
	}
}

void ScriptConditionsDlg::OnCopy() 
{
	if (m_condition) {
		OrCondition *pSavOr = m_orCondition;
		Condition *pCopy = m_condition->duplicate();
		pCopy->setNextCondition(m_condition->getNext());
		m_condition->setNextCondition(pCopy);
		loadList();
		setSel(pSavOr, pCopy);
	}
}

Int ScriptConditionsDlg::doMoveDown( OrCondition **outWhichNow )
{
	(*outWhichNow) = m_orCondition;
	if (m_condition && m_orCondition) {
		Condition *pNext = m_condition->getNext();
		if (pNext==NULL) {
			OrCondition *pNOr = m_orCondition->getNextOrCondition();
			if (!pNOr) {
				pNOr = newInstance( OrCondition);
				m_orCondition->setNextOrCondition(pNOr);
			}
			Condition *newNext = pNOr->getFirstAndCondition();
			pNOr->setFirstAndCondition(m_condition);
			m_orCondition->removeCondition(m_condition);
			m_condition->setNextCondition(newNext);
			
			*outWhichNow = pNOr;
			return 2;	// we moved 2 indices, not just one.
		}

		Condition *pCur = m_orCondition->getFirstAndCondition();
		Condition *pPrev = NULL;
		while (pCur != m_condition) {
			pPrev = pCur;
			pCur = pCur->getNext();
		}
		DEBUG_ASSERTCRASH(pCur, ("Didn't find condition in list."));
		if (!pCur) return 0;
		if (pPrev) {
			pPrev->setNextCondition(pNext);
			pCur->setNextCondition(pNext->getNext());
			pNext->setNextCondition(pCur);
		} else {
			DEBUG_ASSERTCRASH(m_condition == m_orCondition->getFirstAndCondition(), ("Logic error."));
			pCur->setNextCondition(pNext->getNext());
			pNext->setNextCondition(pCur);
			m_orCondition->setFirstAndCondition(pNext);
		}
		return 1;
	} else if (m_orCondition) {
		OrCondition *pNext = m_orCondition->getNextOrCondition();
		if (pNext==NULL) return 0;
		OrCondition *pCur = m_script->getOrCondition();
		OrCondition *pPrev = NULL;
		while (pCur != m_orCondition) {
			pPrev = pCur;
			pCur = pCur->getNextOrCondition();
		}
		DEBUG_ASSERTCRASH(pCur, ("Didn't find Or in list."));
		if (!pCur) return 0;
		if (pPrev) {
			pPrev->setNextOrCondition(pNext);
			pCur->setNextOrCondition(pNext->getNextOrCondition());
			pNext->setNextOrCondition(pCur);
		} else {
			DEBUG_ASSERTCRASH(m_orCondition == m_script->getOrCondition(), ("Logic error."));
			pCur->setNextOrCondition(pNext->getNextOrCondition());
			pNext->setNextOrCondition(pCur);
			m_script->setOrCondition(pNext);
		}
		return 1;
	}
	return 0;
}

Int ScriptConditionsDlg::doMoveUp( OrCondition **outWhichNow )
{
	(*outWhichNow) = m_orCondition;
	if (m_condition && m_orCondition) {
		(*outWhichNow) = m_orCondition;
		Condition *pPrev = m_orCondition->findPreviousCondition(m_condition);
		if (pPrev == NULL) {
			OrCondition *pNOr = m_script->findPreviousOrCondition(m_orCondition);
			if (!pNOr) {
				pNOr = newInstance( OrCondition);
				pNOr->setNextOrCondition(m_orCondition);
				m_script->setOrCondition(pNOr);
			}
			Condition *previous = pNOr->findPreviousCondition(NULL);
			if (previous) {
				m_orCondition->removeCondition(m_condition);
				previous->setNextCondition(m_condition);
			} else {
				m_orCondition->removeCondition(m_condition);
				pNOr->setFirstAndCondition(m_condition);
			}
			
			(*outWhichNow) = pNOr;
			return 2;	// we moved 2 indices, not just one.
		}
		
		pPrev->setNextCondition(m_condition->getNext());
		m_condition->setNextCondition(pPrev);

		Condition *pPrevPrev = m_orCondition->findPreviousCondition(pPrev);
		if (pPrevPrev) {
			pPrevPrev->setNextCondition(m_condition);
		} else {
			m_orCondition->setFirstAndCondition(m_condition);
		}

		return 1;
	} else if (m_orCondition) {
		OrCondition *pOrPrev = m_script->findPreviousOrCondition(m_orCondition);
		if (!pOrPrev) {
			return 0;
		}

		pOrPrev->setNextOrCondition(m_orCondition->getNextOrCondition());
		m_orCondition->setNextOrCondition(pOrPrev);

		OrCondition *pOrPrevPrev = m_script->findPreviousOrCondition(pOrPrev);
		if (pOrPrevPrev) {
			pOrPrevPrev->setNextOrCondition(m_orCondition);
		} else {
			m_script->setOrCondition(m_orCondition);
		}
		return 1;
	}
	return 0;
}

void ScriptConditionsDlg::OnMoveDown() 
{
	Condition *pSav = m_condition;
	OrCondition *pSavOr;
	if (doMoveDown(&pSavOr) == 0) {
		return;
	}

	loadList();
	setSel(pSavOr, pSav);
}

void ScriptConditionsDlg::OnMoveUp() 
{
	Condition *pSav = m_condition;
	OrCondition *pSavOr;
	if (doMoveUp(&pSavOr) == 0) {
		return;
	}

	loadList();
	setSel(pSavOr, pSav);
}

void ScriptConditionsDlg::OnChangeEditComment() 
{
	CWnd *pWnd = GetDlgItem(IDC_EDIT_COMMENT);
	CString comment;
	pWnd->GetWindowText(comment);
	m_script->setConditionComment(AsciiString((LPCTSTR)comment));
}
