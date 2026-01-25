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

// EditAction.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "EditAction.h"
#include "EditParameter.h"
#include "GameLogic/ScriptEngine.h"


/////////////////////////////////////////////////////////////////////////////
// EditAction dialog


EditAction::EditAction(CWnd* pParent /*=nullptr*/)
	: CDialog(EditAction::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditAction)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EditAction::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditAction)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditAction, CDialog)
	//{{AFX_MSG_MAP(EditAction)
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
// EditAction message handlers


BOOL EditAction::OnInitDialog()
{
	CDialog::OnInitDialog();


//	CDC *pDc =GetDC();
	CRect rect;

	CTreeCtrl *pTree = (CTreeCtrl *)GetDlgItem(IDC_ACTION_TREE);
	pTree->GetWindowRect(&rect);

	ScreenToClient(&rect);
	m_actionTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP|WS_TABSTOP, rect, this, IDC_ACTION_TREE);
	m_actionTreeView.ShowWindow(SW_SHOW);
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
	for (i=0; i<ScriptAction::NUM_ITEMS; i++) {
		const ActionTemplate *pTemplate = TheScriptEngine->getActionTemplate(i);
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
				parent = findOrAdd(&m_actionTreeView, parent, prefix);
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
		HTREEITEM item = m_actionTreeView.InsertItem(&ins);
		if (i == m_action->getActionType()) {
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
				parent = findOrAdd(&m_actionTreeView, parent, prefix);
			}
		} while (count>0);

		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_SORT;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = i;
		ins.item.pszText = (char*)name;
		ins.item.cchTextMax = 0;
		m_actionTreeView.InsertItem(&ins);
	}
	m_actionTreeView.Select(selItem, TVGN_FIRSTVISIBLE);
	m_actionTreeView.SelectItem(selItem);
	m_action->setWarnings(false);
	m_myEditCtrl.SetWindowText(m_action->getUiText().str());
	m_myEditCtrl.SetSel(-1, -1);
	formatScriptActionText(0);
	m_actionTreeView.SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void EditAction::formatScriptActionText(Int parameterNdx) {
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
	Int numStrings = m_action->getUiStrings(strings);
	AsciiString warningText;
	AsciiString informationText;
	Int i;
	for (i=0; i<MAX_PARMS; i++) {
		if (i<numStrings) {
			curChar += strings[i].getLength();
		}
		if (i<m_action->getNumParameters()) {
			warningText.concat(EditParameter::getWarningText(m_action->getParameter(i), false));
			informationText.concat(EditParameter::getInfoText(m_action->getParameter(i)));
			numChars = m_action->getParameter(i)->getUiText().getLength();
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



BOOL EditAction::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;

	// Handle events from the tree control.
	if (pHdr->hdr.idFrom == IDC_ACTION_TREE) {
		if (pHdr->hdr.code == TVN_SELCHANGED) {
			char buffer[_MAX_PATH];
			HTREEITEM hItem = m_actionTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
			item.hItem = hItem;
			item.pszText = buffer;
			item.cchTextMax = sizeof(buffer)-2;
			m_actionTreeView.GetItem(&item);
			if (item.lParam >= 0) {
				enum ScriptAction::ScriptActionType actionType = (enum ScriptAction::ScriptActionType)item.lParam;
				if (m_action->getActionType() != actionType) {
					m_action->setActionType( actionType );
					m_myEditCtrl.SetWindowText(m_action->getUiText().str());
					formatScriptActionText(0);
				}
			}
		} else if (pHdr->hdr.code == TVN_KEYDOWN) {
			NMTVKEYDOWN	*pKey = (NMTVKEYDOWN*)lParam;
			Int key = pKey->wVKey;
			if (key==VK_SHIFT || key==VK_SPACE) {
				HTREEITEM hItem = m_actionTreeView.GetSelectedItem();
				if (!m_actionTreeView.ItemHasChildren(hItem)) {
					hItem = m_actionTreeView.GetParentItem(hItem);
				}
				m_actionTreeView.Expand(hItem, TVE_TOGGLE);
				return 0;
			}
			return 0;
		}
		return TRUE;
	}

	// Handle events from the rich edit control containing the action pieces.
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
				Int numStrings = m_action->getUiStrings(strings);
				Int i;
 				Bool match = false;
				for (i=0; i<MAX_PARMS; i++) {
					if (i<numStrings) {
						curChar += strings[i].getLength();
					}
					if (i<m_action->getNumParameters()) {
						numChars = m_action->getParameter(i)->getUiText().getLength();
						match = (curChar+numChars/2 > chrg.cpMin && curChar+numChars/2 < chrg.cpMax);
						if (match) {
							m_curEditParameter = i;
							break;
						}
						curChar += numChars;
					}
				}
				if (pLink->msg == WM_LBUTTONDOWN)
				{
					// Determine which parameter.
					if (match)
					{
						if( m_action->getParameter( m_curEditParameter )->getParameterType() == Parameter::COMMANDBUTTON_ABILITY )
						{
							EditParameter::edit(m_action->getParameter(m_curEditParameter), 0, m_action->getParameter(0)->getString() );
						}
						else
						{
							EditParameter::edit(m_action->getParameter(m_curEditParameter), 0 );
						}
						m_myEditCtrl.SetWindowText(m_action->getUiText().str());
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
					formatScriptActionText(m_curEditParameter);
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
						if (m_curEditParameter >= m_action->getNumParameters()) {
							m_curEditParameter = 0;
							return 1;
						}
						this->PostMessage(WM_TIMER, 0, 0);
						return 0;
					}
					EditParameter::edit(m_action->getParameter(m_curEditParameter), keyPressed);
					m_curEditParameter++;
					if (m_curEditParameter >= m_action->getNumParameters()) {
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

/** Not actually a timer - just used to send a delayed message to self because rich
edit control is stupid.  jba. */
void EditAction::OnTimer(UINT nIDEvent)
{
	m_myEditCtrl.SetWindowText(m_action->getUiText().str());
	formatScriptActionText(m_curEditParameter);
}
