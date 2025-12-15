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

// EditCondition.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "EditCondition.h"
#include "EditParameter.h"
#include "GameLogic/ScriptEngine.h"


LRESULT CMyTreeCtrl::WindowProc(	UINT message, WPARAM wParam, LPARAM lParam )
{
	if (message==WM_KEYDOWN) {
		Int nVirtKey = (int) wParam;    // virtual-key code
		if (nVirtKey == ' ') {
			return 0;
		}
	}
	return CTreeCtrl::WindowProc(message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// EditCondition dialog


EditCondition::EditCondition(CWnd* pParent /*=nullptr*/)
	: CDialog(EditCondition::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditCondition)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EditCondition::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditCondition)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditCondition, CDialog)
	//{{AFX_MSG_MAP(EditCondition)
	ON_CBN_SELCHANGE(IDC_CONDITION_TYPE, OnSelchangeConditionType)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
static HTREEITEM findOrAdd(CTreeCtrl *tree, HTREEITEM parent, const char *pLabel)
{
	TVINSERTSTRUCT ins;
	char buffer[_MAX_PATH];
	::memset(&ins, 0, sizeof(ins));
	HTREEITEM child = tree->GetChildItem(parent);
	while (child != nullptr) {
		ins.item.mask = TVIF_HANDLE|TVIF_TEXT;
		ins.item.hItem = child;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = sizeof(buffer)-2;
		tree->GetItem(&ins.item);
		if (strcmp(buffer, pLabel) == 0) {
			return(child);
		}
		child = tree->GetNextSiblingItem(child);
	}

	// not found, so add it.
	::memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_SORT;
	ins.item.mask = TVIF_PARAM|TVIF_TEXT;
	ins.item.lParam = -1;
	ins.item.pszText = (char*)pLabel;
	ins.item.cchTextMax = strlen(pLabel);
	child = tree->InsertItem(&ins);
	return(child);
}

/////////////////////////////////////////////////////////////////////////////
// EditCondition message handlers


BOOL EditCondition::OnInitDialog()
{
	CDialog::OnInitDialog();


//	CDC *pDc =GetDC();
	CRect rect;

	CTreeCtrl *pTree = (CTreeCtrl *)GetDlgItem(IDC_CONDITION_TREE);
	pTree->GetWindowRect(&rect);

	ScreenToClient(&rect);
	m_conditionTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP|WS_TABSTOP, rect, this, IDC_CONDITION_TREE);
	m_conditionTreeView.ShowWindow(SW_SHOW);
	pTree->DestroyWindow();

	CWnd *pWnd = GetDlgItem(IDC_RICH_EDIT_HERE);
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_myEditCtrl.Create(WS_CHILD | WS_TABSTOP | ES_MULTILINE, rect, this, IDC_RICH_EDIT_HERE+1);
	m_myEditCtrl.ShowWindow(SW_SHOW);
	m_myEditCtrl.SetEventMask(m_myEditCtrl.GetEventMask() | ENM_LINK | ENM_SELCHANGE | ENM_KEYEVENTS);

	Int i;
	HTREEITEM selItem = nullptr;
	for (i=0; i<Condition::NUM_ITEMS; i++) {
		const ConditionTemplate *pTemplate = TheScriptEngine->getConditionTemplate(i);
		char prefix[_MAX_PATH];
		const char *name = pTemplate->getName().str();

		Int count = 0;
		HTREEITEM parent = TVI_ROOT;
		do {
			count = 0;
			const char *nameStart = name;
			while (*name && *name != '/') {
				count++;
				name++;
			}
			if (*name=='/') {
				count++;
				name++;
			} else {
				name = nameStart;
				count = 0;
			}
			if (count>0) {
				strlcpy(prefix, nameStart, count);
				parent = findOrAdd(&m_conditionTreeView, parent, prefix);
			}
		} while (count>0);

		TVINSERTSTRUCT ins;
		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_SORT;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = i;
		ins.item.pszText = (char*)name;
		ins.item.cchTextMax = 0;
		HTREEITEM item = m_conditionTreeView.InsertItem(&ins);
		if (i == m_condition->getConditionType()) {
			selItem = item;
		}

		name = pTemplate->getName2().str();
		count = 0;
		if (pTemplate->getName2().isEmpty()) continue;
		parent = TVI_ROOT;
		do {
			count = 0;
			const char *nameStart = name;
			while (*name && *name != '/') {
				count++;
				name++;
			}
			if (*name=='/') {
				count++;
				name++;
			} else {
				name = nameStart;
				count = 0;
			}
			if (count>0) {
				strlcpy(prefix, nameStart, count);
				parent = findOrAdd(&m_conditionTreeView, parent, prefix);
			}
		} while (count>0);

		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_SORT;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = i;
		ins.item.pszText = (char*)name;
		ins.item.cchTextMax = 0;
		m_conditionTreeView.InsertItem(&ins);
	}
	m_conditionTreeView.Select(selItem, TVGN_FIRSTVISIBLE);
	m_conditionTreeView.SelectItem(selItem);
	m_condition->setWarnings(false);
	m_myEditCtrl.SetWindowText(m_condition->getUiText().str());
	m_myEditCtrl.SetSel(-1, -1);
	formatConditionText(0);
	m_conditionTreeView.SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void EditCondition::formatConditionText(Int parameterNdx) {
	CHARFORMAT2 cf;
	m_updating = true;
	long startSel, endSel;
	m_myEditCtrl.GetSel(startSel, endSel);
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE | CFM_SIZE |CFM_CHARSET | CFM_BOLD | CFM_LINK;
	cf.bCharSet = DEFAULT_CHARSET;
	cf.yHeight = 14;
	cf.bPitchAndFamily = FF_DONTCARE;
	strcpy(cf.szFaceName, "MS Sans Serif");
	cf.dwEffects = 0;
	m_myEditCtrl.SetDefaultCharFormat(cf);

	m_myEditCtrl.SetSel(0, 1000);
	m_myEditCtrl.SetSelectionCharFormat(cf);
 	//m_myEditCtrl.SetReadOnly();
	// Set up the links.
	cf.dwMask =  CFE_UNDERLINE | CFM_LINK | CFM_COLOR;

	cf.dwEffects = CFE_LINK | CFE_UNDERLINE;
	cf.crTextColor = RGB(0,0,255);
	m_curEditParameter = parameterNdx;

	AsciiString strings[MAX_PARMS];
	Int curChar = 0;
	Int numChars = 0;
	Int numStrings = m_condition->getUiStrings(strings);
	AsciiString warningText;
	AsciiString informationText;
	Int i;
	for (i=0; i<MAX_PARMS; i++) {
		if (i<numStrings) {
			curChar += strings[i].getLength();
		}
		if (i<m_condition->getNumParameters()) {
			warningText.concat(EditParameter::getWarningText(m_condition->getParameter(i), false));
			informationText.concat(EditParameter::getInfoText(m_condition->getParameter(i)));
			numChars = m_condition->getParameter(i)->getUiText().getLength();
			if (curChar==0) {
				curChar++;
				numChars--;
			}
			m_myEditCtrl.SetSel(curChar, curChar+numChars);
			if (i==parameterNdx) {
				startSel = curChar;
				endSel = curChar+numChars;
				cf.crTextColor = RGB(0,0,0); //black
			}	else {
				cf.crTextColor = RGB(0,0,255); //blue
			}
			m_myEditCtrl.SetSelectionCharFormat(cf);
			curChar += numChars;
		}
	}

	CString cstr;
	if (warningText.isEmpty()) {
		if (informationText.isEmpty()) {
			if (cstr.LoadString(IDS_SCRIPT_NOWARNINGS)) {
				GetDlgItem(IDC_WARNINGS_CAPTION)->SetWindowText(cstr);
			}
			GetDlgItem(IDC_WARNINGS_CAPTION)->EnableWindow(false);
			GetDlgItem(IDC_WARNINGS)->SetWindowText("");
		} else {
			if (cstr.LoadString(IDS_SCRIPT_INFORMATION)) {
				GetDlgItem(IDC_WARNINGS_CAPTION)->SetWindowText(cstr);
			}
			GetDlgItem(IDC_WARNINGS_CAPTION)->EnableWindow(true);
			GetDlgItem(IDC_WARNINGS)->SetWindowText(informationText.str());
		}
	} else {
		if (cstr.LoadString(IDS_SCRIPT_WARNINGS)) {
			GetDlgItem(IDC_WARNINGS_CAPTION)->SetWindowText(cstr);
		}
		GetDlgItem(IDC_WARNINGS_CAPTION)->EnableWindow(true);
		GetDlgItem(IDC_WARNINGS)->SetWindowText(warningText.str());
	}

	m_modifiedTextColor = false;
	m_myEditCtrl.SetSel(startSel, endSel);
	m_curLinkChrg.cpMax = endSel;
	m_curLinkChrg.cpMin = startSel;
	m_updating = false;
}



BOOL EditCondition::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;

	// Handle events from the tree control.
	if (pHdr->hdr.idFrom == IDC_CONDITION_TREE) {
		if (pHdr->hdr.code == TVN_SELCHANGED) {
			char buffer[_MAX_PATH];
			HTREEITEM hItem = m_conditionTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
			item.hItem = hItem;
			item.pszText = buffer;
			item.cchTextMax = sizeof(buffer)-2;
			m_conditionTreeView.GetItem(&item);
			if (item.lParam >= 0) {
				enum Condition::ConditionType conditionType = (enum Condition::ConditionType)item.lParam;
				if (m_condition->getConditionType() != conditionType) {
					m_condition->setConditionType( conditionType );
					m_myEditCtrl.SetWindowText(m_condition->getUiText().str());
					formatConditionText(0);
				}
			}
		} else if (pHdr->hdr.code == TVN_KEYDOWN) {
			NMTVKEYDOWN	*pKey = (NMTVKEYDOWN*)lParam;
			Int key = pKey->wVKey;
			if (key==VK_SHIFT || key==VK_SPACE) {
				HTREEITEM hItem = m_conditionTreeView.GetSelectedItem();
				if (!m_conditionTreeView.ItemHasChildren(hItem)) {
					hItem = m_conditionTreeView.GetParentItem(hItem);
				}
				m_conditionTreeView.Expand(hItem, TVE_TOGGLE);
				return 0;
			}
			return 0;
		}
		return TRUE;
	}

	// Handle events from the rich edit control containg the condition pieces.
	if (LOWORD(wParam) == IDC_RICH_EDIT_HERE+1) {
		NMHDR *pHdr = (NMHDR *)lParam;
		if (pHdr->hwndFrom == m_myEditCtrl.m_hWnd) {
			if (pHdr->code == EN_LINK) {
				ENLINK *pLink = (ENLINK *)pHdr;
				CHARRANGE chrg = pLink->chrg;
				// Determine which parameter.
				Int numChars = 0;
				Int curChar = 0;
				AsciiString strings[MAX_PARMS];
				Int numStrings = m_condition->getUiStrings(strings);
				Int i;
 				Bool match = false;
				for (i=0; i<MAX_PARMS; i++) {
					if (i<numStrings) {
						curChar += strings[i].getLength();
					}
					if (i<m_condition->getNumParameters()) {
						numChars = m_condition->getParameter(i)->getUiText().getLength();
						match = (curChar+numChars/2 > chrg.cpMin && curChar+numChars/2 < chrg.cpMax);
						if (match) {
							m_curEditParameter = i;
							break;
						}
						curChar += numChars;
					}
				}
				if (pLink->msg == WM_LBUTTONDOWN) {
					// Determine which parameter.
					if (match) {
						EditParameter::edit(m_condition->getParameter(m_curEditParameter), 0);
						m_myEditCtrl.SetWindowText(m_condition->getUiText().str());
						this->PostMessage(WM_TIMER, 0, 0);
						return true;
					}
				}
				CHARRANGE curChrg;
				m_myEditCtrl.GetSel(curChrg);
				if (curChrg.cpMin == chrg.cpMin && curChrg.cpMax == chrg.cpMax) {
					return true;
				}
				if (m_modifiedTextColor) {
					formatConditionText(m_curEditParameter);
				}
				m_curLinkChrg = chrg;
				m_myEditCtrl.SetSel(chrg.cpMin, chrg.cpMax);
				m_myEditCtrl.SetFocus();
				CHARFORMAT cf;
				memset(&cf, 0, sizeof(cf));
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_COLOR;
				cf.crTextColor = RGB(0,0,0);
				m_myEditCtrl.SetSelectionCharFormat(cf);
				m_modifiedTextColor = true;
				return true;
			}	else 	if (pHdr->code == EN_SETFOCUS) {
				this->PostMessage(WM_TIMER, 0, 0);
			}	else 	if (pHdr->code == EN_SELCHANGE) {
				if (m_updating) {
					return true;
				}
				CHARRANGE curChrg;
				m_myEditCtrl.GetSel(curChrg);
				if (curChrg.cpMin == m_curLinkChrg.cpMin && curChrg.cpMax == m_curLinkChrg.cpMax) {
					return true;
				}
				m_myEditCtrl.SetSel(m_curLinkChrg.cpMin, m_curLinkChrg.cpMax);
				if (m_modifiedTextColor) {
					this->PostMessage(WM_TIMER, 0, 0);
				}
			} else if (pHdr->code == EN_MSGFILTER) {
				MSGFILTER *pFilter = (MSGFILTER *)pHdr;
				if (pFilter->msg == WM_CHAR) {
					Int keyPressed = pFilter->wParam;
					if (keyPressed==VK_RETURN) {
						return 1;
					}
					if (keyPressed==VK_RETURN || keyPressed == VK_TAB) {
						m_curEditParameter++;
						if (m_curEditParameter >= m_condition->getNumParameters()) {
							m_curEditParameter = 0;
							return 1;
						}
						this->PostMessage(WM_TIMER, 0, 0);
						return 0;
					}
					EditParameter::edit(m_condition->getParameter(m_curEditParameter), keyPressed);
					m_curEditParameter++;
					if (m_curEditParameter >= m_condition->getNumParameters()) {
						m_curEditParameter = 0;
					}
					this->PostMessage(WM_TIMER, 0, 0);
					return 0;
				}
				return 1;
			}
		}
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void EditCondition::OnSelchangeConditionType()
{
	CComboBox *pCmbo = (CComboBox *)GetDlgItem(IDC_CONDITION_TYPE);
	Int index = 0;
	CString str;
	pCmbo->GetWindowText(str);
	Int i;
	for (i=0; i<ScriptAction::NUM_ITEMS; i++) {
		const ConditionTemplate *pTemplate = TheScriptEngine->getConditionTemplate(i);
		if (str == pTemplate->getName().str()) {
			index = i;
			break;
		}
	}
	m_condition->setConditionType((enum Condition::ConditionType)index);
	m_myEditCtrl.SetWindowText(m_condition->getUiText().str());
	formatConditionText(m_curEditParameter);
}

/** Not actually a timer - just used to send a delayed message to self because rich
edit control is stupid.  jba. */
void EditCondition::OnTimer(UINT nIDEvent)
{
	m_myEditCtrl.SetWindowText(m_condition->getUiText().str());
	formatConditionText(m_curEditParameter);
}
