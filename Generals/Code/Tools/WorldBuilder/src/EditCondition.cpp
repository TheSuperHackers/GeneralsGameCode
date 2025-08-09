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

// EditCondition.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "EditCondition.h"
#include "EditParameter.h"
#include "GameLogic/ScriptEngine.h"

/////////////////////////////////////////////////////////////////////////////
// EditCondition dialog


EditCondition::EditCondition(CWnd* pParent /*=NULL*/)
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

/////////////////////////////////////////////////////////////////////////////
// EditCondition message handlers


BOOL EditCondition::OnInitDialog()
{
	CDialog::OnInitDialog();


//	CDC *pDc =GetDC();

	CWnd *pWnd = GetDlgItem(IDC_RICH_EDIT_HERE);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_myEditCtrl.Create(WS_CHILD | ES_MULTILINE, rect, this, IDC_RICH_EDIT_HERE+1);
	m_myEditCtrl.ShowWindow(SW_SHOW);
	m_myEditCtrl.SetEventMask(m_myEditCtrl.GetEventMask() | ENM_LINK | ENM_SELCHANGE);

	CComboBox *pCmbo = (CComboBox *)GetDlgItem(IDC_CONDITION_TYPE);
	pCmbo->ResetContent();
	Int i;
	for (i=0; i<Condition::NUM_ITEMS; i++) {
		const ConditionTemplate *pTemplate = TheScriptEngine->getConditionTemplate(i);
		Int ndx = pCmbo->AddString(pTemplate->getName().str());
		if (i == m_condition->getConditionType()) {
			pCmbo->SetCurSel(ndx);
		}
	}
	m_condition->setWarnings(false);
	m_myEditCtrl.SetWindowText(m_condition->getUiText().str());
	m_myEditCtrl.SetSel(-1, -1);
	formatConditionText(-1);

	return TRUE;  // return TRUE unless you set the focus to a control
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
 	m_myEditCtrl.SetReadOnly();
	// Set up the links.
	cf.dwMask =  CFE_UNDERLINE | CFM_LINK | CFM_COLOR;

	cf.dwEffects = CFE_LINK | CFE_UNDERLINE;
	cf.crTextColor = RGB(0,0,255);

	AsciiString strings[MAX_PARMS];
	Int curChar = 0;
	Int numChars = 0;
	Int numStrings = m_condition->getUiStrings(strings);
	Int i;
	AsciiString warningText;
	for (i=0; i<MAX_PARMS; i++) {
		if (i<numStrings) {
			curChar += strings[i].getLength();
		}
		if (i<m_condition->getNumParameters()) {
			numChars = m_condition->getParameter(i)->getUiText().getLength();
			warningText.concat(EditParameter::getWarningText(m_condition->getParameter(i)));
			m_myEditCtrl.SetSel(curChar, curChar+numChars);
			if (numChars==0) continue;
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
		if (cstr.LoadString(IDS_SCRIPT_NOWARNINGS)) {
			GetDlgItem(IDC_WARNINGS_CAPTION)->SetWindowText(cstr);
		}
		GetDlgItem(IDC_WARNINGS_CAPTION)->EnableWindow(false);
		GetDlgItem(IDC_WARNINGS)->SetWindowText("");
	} else {
		if (cstr.LoadString(IDS_SCRIPT_WARNINGS)) {
			GetDlgItem(IDC_WARNINGS_CAPTION)->SetWindowText(cstr);
		}
		GetDlgItem(IDC_WARNINGS_CAPTION)->EnableWindow(true);
		GetDlgItem(IDC_WARNINGS)->SetWindowText(warningText.str());
	}

	m_modifiedTextColor = false;
	m_myEditCtrl.SetSel(startSel, endSel);
	m_updating = false;
}



BOOL EditCondition::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (LOWORD(wParam) == IDC_RICH_EDIT_HERE+1) {
		NMHDR *pHdr = (NMHDR *)lParam;
		if (pHdr->hwndFrom == m_myEditCtrl.m_hWnd && pHdr->code == EN_LINK) {
			ENLINK *pLink = (ENLINK *)pHdr;
			CHARRANGE chrg = pLink->chrg;
			if (pLink->msg == WM_LBUTTONDOWN) {
				// Determine which parameter.
				Int numChars = 0;
				Int curChar = 0;
				AsciiString strings[MAX_PARMS];
				Int numStrings = m_condition->getUiStrings(strings);
				Int i;
				for (i=0; i<MAX_PARMS; i++) {
					if (i<numStrings) {
						curChar += strings[i].getLength();
					}
					if (i<m_condition->getNumParameters()) {
						numChars = m_condition->getParameter(i)->getUiText().getLength();
						if (curChar == chrg.cpMin && curChar+numChars == chrg.cpMax) {
							if (IDOK==EditParameter::edit(m_condition->getParameter(i))) {
								m_myEditCtrl.SetWindowText(m_condition->getUiText().str());
								m_curEditParameter = i;
								this->PostMessage(WM_TIMER, 0, 0);
							}
							return true;
						}
						curChar += numChars;
					}
				}
			}
			CHARRANGE curChrg;
			m_myEditCtrl.GetSel(curChrg);
			if (curChrg.cpMin == chrg.cpMin && curChrg.cpMax == chrg.cpMax) {
				return true;
			}
			if (m_modifiedTextColor) {
				formatConditionText(-1);
			}
			m_curLinkChrg = chrg;
			m_myEditCtrl.SetSel(chrg.cpMin, chrg.cpMax);
			CHARFORMAT cf;
			memset(&cf, 0, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_COLOR;
			cf.crTextColor = RGB(0,0,0);
			m_myEditCtrl.SetSelectionCharFormat(cf);
			m_modifiedTextColor = true;
			return true;
		}	else 	if (pHdr->hwndFrom == m_myEditCtrl.m_hWnd && pHdr->code == EN_SELCHANGE) {
			if (m_updating) {
				return true;
			}
			CHARRANGE curChrg;
			m_myEditCtrl.GetSel(curChrg);
			if (curChrg.cpMin == m_curLinkChrg.cpMin && curChrg.cpMax == m_curLinkChrg.cpMax) {
				return true;
			}
			if (m_modifiedTextColor) {
				formatConditionText(-1);
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
	formatConditionText(-1);
}

/** Not actually a timer - just used to send a delayed message to self because rich
edit control is stupid.  jba. */
void EditCondition::OnTimer(UINT nIDEvent)
{
	formatConditionText(m_curEditParameter);
}
