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
	ON_CBN_SELCHANGE(IDC_CONDITION_TYPE, OnSelchangeScriptActionType)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditAction message handlers


BOOL EditAction::OnInitDialog()
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
	for (i=0; i<ScriptAction::NUM_ITEMS; i++) {
		const ActionTemplate *pTemplate = TheScriptEngine->getActionTemplate(i);
		Int ndx = pCmbo->AddString(pTemplate->getName().str());
		if (i == m_action->getActionType()) {
			pCmbo->SetCurSel(ndx);
		}
	}
	m_action->setWarnings(false);
	m_myEditCtrl.SetWindowText(m_action->getUiText().str());
	m_myEditCtrl.SetSel(-1, -1);
	formatScriptActionText(-1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void EditAction::formatScriptActionText(Int parameterNdx)
{
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
	Int numStrings = m_action->getUiStrings(strings);
	AsciiString warningText;
	AsciiString informationText;
	Int i;
	for (i=0; i<MAX_PARMS; i++) {
		if (i<numStrings) {
			curChar += strings[i].getLength();
		}
		if (i<m_action->getNumParameters()) {
			warningText.concat(EditParameter::getWarningText(m_action->getParameter(i)));
			informationText.concat(EditParameter::getInfoText(m_action->getParameter(i)));
			numChars = m_action->getParameter(i)->getUiText().getLength();
			if (numChars==0) continue;
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
	m_updating = false;
}



BOOL EditAction::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (LOWORD(wParam) == IDC_RICH_EDIT_HERE+1)
	{
		NMHDR *pHdr = (NMHDR *)lParam;
		if (pHdr->hwndFrom == m_myEditCtrl.m_hWnd && pHdr->code == EN_LINK)
		{
			ENLINK *pLink = (ENLINK *)pHdr;
			CHARRANGE chrg = pLink->chrg;
			if (pLink->msg == WM_LBUTTONDOWN)
			{
				// Determine which parameter.
				Int numChars = 0;
				Int curChar = 0;
				AsciiString strings[MAX_PARMS];
				Int numStrings = m_action->getUiStrings(strings);
				Int i;
				for (i=0; i<MAX_PARMS; i++)
				{
					if (i<numStrings)
					{
						curChar += strings[i].getLength();
					}
					if (i<m_action->getNumParameters())
					{
						numChars = m_action->getParameter(i)->getUiText().getLength();
						if (curChar == chrg.cpMin && curChar+numChars == chrg.cpMax)
						{
							//Kris:
							//Before we edit the parameter, there is a new prerequisite for parameters
							//but only a few will ever care. We will store what we perceive as the unit,
							//and for simplicity, we'll store the first occurrence of the unit, although
							//this can change in the future should the need arise.
							AsciiString unitName;
							for( int j = 0; j < MAX_PARMS; j++ )
							{
								Parameter *parameter = m_action->getParameter( j );
								if( parameter && parameter->getParameterType() == Parameter::UNIT )
								{
									unitName = parameter->getString();
									break;
								}
							}

							if( EditParameter::edit( m_action->getParameter(i), unitName ) == IDOK )
							{
								m_myEditCtrl.SetWindowText(m_action->getUiText().str());
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
			if (curChrg.cpMin == chrg.cpMin && curChrg.cpMax == chrg.cpMax)
			{
				return true;
			}
			if (m_modifiedTextColor)
			{
				formatScriptActionText(-1);
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
		}
		else 	if (pHdr->hwndFrom == m_myEditCtrl.m_hWnd && pHdr->code == EN_SELCHANGE)
		{
			if (m_updating)
			{
				return true;
			}
			CHARRANGE curChrg;
			m_myEditCtrl.GetSel(curChrg);
			if (curChrg.cpMin == m_curLinkChrg.cpMin && curChrg.cpMax == m_curLinkChrg.cpMax)
			{
				return true;
			}
			if (m_modifiedTextColor)
			{
				formatScriptActionText(-1);
			}
		}
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void EditAction::OnSelchangeScriptActionType()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_CONDITION_TYPE);
	Int index = 0;
	CString str;
	pCombo->GetWindowText(str);
	Int i;
	for (i=0; i<ScriptAction::NUM_ITEMS; i++) {
		const ActionTemplate *pTemplate = TheScriptEngine->getActionTemplate(i);
		if (str == pTemplate->getName().str()) {
			index = i;
			break;
		}
	}

	m_action->setActionType((enum ScriptAction::ScriptActionType)i );
	m_myEditCtrl.SetWindowText(m_action->getUiText().str());
	formatScriptActionText(-1);
}

/** Not actually a timer - just used to send a delayed message to self because rich
edit control is stupid.  jba. */
void EditAction::OnTimer(UINT nIDEvent)
{
	formatScriptActionText(m_curEditParameter);
}
