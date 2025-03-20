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

#include "StdAfx.h"
#include "resource.h"

#include "TeamGeneric.h"
#include "EditParameter.h"
#include "WorldBuilder.h"

#include "Common/AsciiString.h"
#include "Common/Dict.h"
#include "Common/WellKnownKeys.h"

static const UINT s_allControls[][2] = 
{
	{ IDC_SCRIPT_PREFIX1,		IDC_TeamGeneric_Script1,	},
	{ IDC_SCRIPT_PREFIX2,		IDC_TeamGeneric_Script2,	},
	{ IDC_SCRIPT_PREFIX3,		IDC_TeamGeneric_Script3,	},
	{ IDC_SCRIPT_PREFIX4,		IDC_TeamGeneric_Script4,	},
	{ IDC_SCRIPT_PREFIX5,		IDC_TeamGeneric_Script5,	},
	{ IDC_SCRIPT_PREFIX6,		IDC_TeamGeneric_Script6,	},
	{ IDC_SCRIPT_PREFIX7,		IDC_TeamGeneric_Script7,	},
	{ IDC_SCRIPT_PREFIX8,		IDC_TeamGeneric_Script8,	},
	{ IDC_SCRIPT_PREFIX9,		IDC_TeamGeneric_Script9,	},
	{ IDC_SCRIPT_PREFIX10,	IDC_TeamGeneric_Script10, },
	{ IDC_SCRIPT_PREFIX11,	IDC_TeamGeneric_Script11, },
	{ IDC_SCRIPT_PREFIX12,	IDC_TeamGeneric_Script12, },
	{ IDC_SCRIPT_PREFIX13,	IDC_TeamGeneric_Script13, },
	{ IDC_SCRIPT_PREFIX14,	IDC_TeamGeneric_Script14, },
	{ IDC_SCRIPT_PREFIX15,	IDC_TeamGeneric_Script15, },
	{ IDC_SCRIPT_PREFIX16,	IDC_TeamGeneric_Script16, },
	{ 0,0, },
};

TeamGeneric::TeamGeneric() : CPropertyPage(TeamGeneric::IDD)
{
	
}

BOOL TeamGeneric::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	// Fill all the combo boxes with the scripts we have.
	_fillComboBoxesWithScripts();

	// Set up the dialog as appropriate.
	_dictToScripts();

	return FALSE;
}

void TeamGeneric::_fillComboBoxesWithScripts()
{
	int i = 0;
	while (s_allControls[i][1]) {
		CComboBox *pCombo = (CComboBox*) GetDlgItem(s_allControls[i][1]);
		if (!pCombo) {
			continue;
		}

		// Load all the scripts, then add the NONE string.
		EditParameter::loadScripts(pCombo, true);
		pCombo->InsertString(0, NONE_STRING);
		++i;
	}
}

void TeamGeneric::_dictToScripts()
{
	CWnd *pText = NULL;
	CComboBox *pCombo = NULL;

	if (!m_teamDict) {
		return;
	}

	int i = 0;
	while (s_allControls[i][1]) {

		pText = GetDlgItem(s_allControls[i][0]);
		if (!pText) {
			continue;
		}

		pCombo = (CComboBox*) GetDlgItem(s_allControls[i][1]);
		if (!pCombo) {
			continue;
		}

		Bool exists;
		AsciiString scriptString;
		AsciiString keyName;
		keyName.format("%s%d", TheNameKeyGenerator->keyToName(TheKey_teamGenericScriptHook).str(), i);
		scriptString = m_teamDict->getAsciiString(NAMEKEY(keyName), &exists);

		pText->ShowWindow(SW_SHOW);
		pCombo->ShowWindow(SW_SHOW);

		if (exists) {
			Int selNdx = pCombo->FindStringExact(-1, scriptString.str());
			if (selNdx == LB_ERR) {
				pCombo->SetCurSel(0);
			} else {
				pCombo->SetCurSel(selNdx);
			}
		} else {
			break;
		}

		++i;
	}

	if (!s_allControls[i][1]) {
		// We filled everything.
		return;
	}

	if (!pCombo) {
		// We filled nothing, or there was an error.
		return;
	}

	pCombo->SetCurSel(0);

	++i;
	while (s_allControls[i][1]) {
		pText = GetDlgItem(s_allControls[i][0]);
		if (!pText) {
			continue;
		}

		pCombo = (CComboBox*) GetDlgItem(s_allControls[i][1]);
		if (!pCombo) {
			continue;
		}

		pText->ShowWindow(SW_HIDE);
		pCombo->ShowWindow(SW_HIDE);
		++i;
	}
}

void TeamGeneric::_scriptsToDict()
{
	if (!m_teamDict) {
		return;
	}

	CWnd *pText = NULL;
	CComboBox *pCombo = NULL;

	int scriptNum = 0;

	int i = 0;
	while (s_allControls[i][1]) {

		pText = GetDlgItem(s_allControls[i][0]);
		if (!pText) {
			continue;
		}

		pCombo = (CComboBox*) GetDlgItem(s_allControls[i][1]);
		if (!pCombo) {
			continue;
		}

		// i should always be incremented, so just do it here.
		++i;

		AsciiString keyName;
		keyName.format("%s%d", TheNameKeyGenerator->keyToName(TheKey_teamGenericScriptHook).str(), scriptNum);

		int curSel = pCombo->GetCurSel();
		if (curSel == CB_ERR || curSel == 0) {
			if (m_teamDict->known(NAMEKEY(keyName), Dict::DICT_ASCIISTRING)) {
				// remove it if we know it.
				m_teamDict->remove(NAMEKEY(keyName));
			}

			continue;
		}

		CString cstr;
		pCombo->GetLBText(curSel, cstr);

		AsciiString scriptString = cstr;
		m_teamDict->setAsciiString(NAMEKEY(keyName), scriptString);
		++scriptNum;
	}

	for ( ; s_allControls[scriptNum][1]; ++scriptNum ) {
		AsciiString keyName;
		keyName.format("%s%d", TheNameKeyGenerator->keyToName(TheKey_teamGenericScriptHook).str(), scriptNum);

		if (m_teamDict->known(NAMEKEY(keyName), Dict::DICT_ASCIISTRING))  {
			m_teamDict->remove(NAMEKEY(keyName));
		}
	}
}

void TeamGeneric::OnScriptAdjust()
{
	_scriptsToDict();
	_dictToScripts();
}

BEGIN_MESSAGE_MAP(TeamGeneric, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script1, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script2, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script3, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script4, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script5, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script6, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script7, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script8, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script9, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script10, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script11, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script12, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script13, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script14, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script15, OnScriptAdjust)
	ON_CBN_SELCHANGE(IDC_TeamGeneric_Script16, OnScriptAdjust)
END_MESSAGE_MAP()