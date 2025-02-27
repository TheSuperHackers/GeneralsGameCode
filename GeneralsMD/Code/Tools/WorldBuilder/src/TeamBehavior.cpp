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

// TeamBehavior.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "TeamBehavior.h"
#include "EditParameter.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/AI.h"

/////////////////////////////////////////////////////////////////////////////
// TeamBehavior dialog

TeamBehavior::TeamBehavior()
	: CPropertyPage(TeamBehavior::IDD)
{
	//{{AFX_DATA_INIT(TeamBehavior)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TeamBehavior::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TeamBehavior)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TeamBehavior, CPropertyPage)
	//{{AFX_MSG_MAP(TeamBehavior)
	ON_CBN_SELCHANGE(IDC_ON_CREATE_SCRIPT, OnSelchangeOnCreateScript)
	ON_BN_CLICKED(IDC_TRANSPORTS_RETURN, OnTransportsReturn)
	ON_BN_CLICKED(IDC_AVOID_THREATS, OnAvoidThreats)
	ON_CBN_SELCHANGE(IDC_ON_ENEMY_SIGHTED, OnSelchangeOnEnemySighted)
	ON_CBN_SELCHANGE(IDC_ON_DESTROYED, OnSelchangeOnDestroyed)
	ON_CBN_SELCHANGE(IDC_ON_UNIT_DESTROYED_SCRIPT, OnSelchangeOnUnitDestroyed)
	ON_BN_CLICKED(IDC_PERIMETER_DEFENSE, OnPerimeterDefense)
	ON_BN_CLICKED(IDC_BASE_DEFENSE, OnBaseDefense)
	ON_EN_CHANGE(IDC_PERCENT_DESTROYED, OnChangePercentDestroyed)
	ON_CBN_SELCHANGE(IDC_ENEMY_INTERACTIONS, OnSelchangeEnemyInteractions)
	ON_CBN_SELCHANGE(IDC_ON_ALL_CLEAR, OnSelchangeOnAllClear)
	ON_CBN_SELCHANGE(IDC_ON_IDLE_SCRIPT, OnSelchangeOnIdleScript)
	ON_BN_CLICKED(IDC_ATTACK_COMMON_TARGET, OnAttackCommonTarget)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TeamBehavior message handlers

void TeamBehavior::updateScript(NameKeyType keyScript, int idcScript)
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(idcScript);
	CString txt;
	Int curSel = pCombo->GetCurSel();
	pCombo->GetLBText(curSel, txt);
	AsciiString comboText = AsciiString(txt);
	if (comboText == NONE_STRING) {
		comboText.clear();
	}
	m_teamDict->setAsciiString(keyScript, comboText);
}

void TeamBehavior::setupScript(NameKeyType keyScript, int idcScript)
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(idcScript);
	// Load the subroutine scripts into the combo box.
	EditParameter::loadScripts(pCombo, true);
	Int stringNdx = pCombo->AddString(NONE_STRING);
	pCombo->SetFocus();
	Bool exists;
	AsciiString script = m_teamDict->getAsciiString(keyScript, &exists);
	if (exists && !script.isEmpty()) {
		Int ndx = pCombo->FindStringExact(-1, script.str());
		if (ndx != CB_ERR) {
			stringNdx = ndx;
		}	else {
			AsciiString badName = "*";
			badName.concat(script);
			stringNdx = pCombo->AddString(badName.str());
		}
	}
	pCombo->SetCurSel(stringNdx);
}


BOOL TeamBehavior::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	setupScript(TheKey_teamOnCreateScript, IDC_ON_CREATE_SCRIPT);
	setupScript(TheKey_teamOnIdleScript, IDC_ON_IDLE_SCRIPT);
	setupScript(TheKey_teamEnemySightedScript, IDC_ON_ENEMY_SIGHTED);
	setupScript(TheKey_teamOnDestroyedScript, IDC_ON_DESTROYED);
	setupScript(TheKey_teamAllClearScript, IDC_ON_ALL_CLEAR);
	setupScript(TheKey_teamOnUnitDestroyedScript, IDC_ON_UNIT_DESTROYED_SCRIPT);

	Bool exists;

	CButton *pCheck = (CButton *) GetDlgItem(IDC_TRANSPORTS_RETURN);
	Bool transportsReturn = m_teamDict->getBool(TheKey_teamTransportsReturn, &exists);
	pCheck->SetCheck(transportsReturn?1:0);

	pCheck = (CButton *) GetDlgItem(IDC_BASE_DEFENSE);
	if( pCheck )
	{
		Bool baseDef = m_teamDict->getBool(TheKey_teamIsBaseDefense, &exists);
		pCheck->SetCheck(baseDef?1:0);
	}

	pCheck = (CButton *) GetDlgItem(IDC_PERIMETER_DEFENSE);
	if( pCheck )
	{
		Bool perimeter = m_teamDict->getBool(TheKey_teamIsPerimeterDefense, &exists);
		pCheck->SetCheck(perimeter?1:0);
	}

	pCheck = (CButton *) GetDlgItem(IDC_AVOID_THREATS);
	Bool avoid = m_teamDict->getBool(TheKey_teamAvoidThreats, &exists);
	pCheck->SetCheck(avoid?1:0);

	pCheck = (CButton *) GetDlgItem(IDC_ATTACK_COMMON_TARGET);
	Bool attack = m_teamDict->getBool(TheKey_teamAttackCommonTarget, &exists);
	pCheck->SetCheck(attack?1:0);

	AsciiString description;

	CWnd *pWnd = GetDlgItem(IDC_PERCENT_DESTROYED);
	Real threshold = m_teamDict->getReal(TheKey_teamDestroyedThreshold, &exists);
	if (!exists) threshold = 0.5f;
	Int percent = floor((threshold*100)+0.5);
	description.format("%d", percent);
	pWnd->SetWindowText(description.str());

	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_ENEMY_INTERACTIONS);
	pCombo->SetCurSel(m_teamDict->getInt(TheKey_teamAggressiveness, &exists) - ATTITUDE_SLEEP);


	return FALSE;  // return TRUE unless you set the focus to a control
								// EXCEPTION: OCX Property Pages should return FALSE
}

void TeamBehavior::OnTransportsReturn()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_TRANSPORTS_RETURN);
	Bool checked = 	pCheck->GetCheck()==1;
	m_teamDict->setBool(TheKey_teamTransportsReturn, checked);
}

void TeamBehavior::OnAvoidThreats()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_AVOID_THREATS);
	Bool checked = 	pCheck->GetCheck()==1;
	m_teamDict->setBool(TheKey_teamAvoidThreats, checked);
}

void TeamBehavior::OnSelchangeOnEnemySighted()
{
	updateScript(TheKey_teamEnemySightedScript, IDC_ON_ENEMY_SIGHTED);
}

void TeamBehavior::OnSelchangeOnDestroyed()
{
	updateScript(TheKey_teamOnDestroyedScript, IDC_ON_DESTROYED);
}

void TeamBehavior::OnSelchangeOnUnitDestroyed()
{
	updateScript(TheKey_teamOnUnitDestroyedScript, IDC_ON_UNIT_DESTROYED_SCRIPT);
}

void TeamBehavior::OnSelchangeOnCreateScript()
{
	updateScript(TheKey_teamOnCreateScript, IDC_ON_CREATE_SCRIPT);
}

void TeamBehavior::OnPerimeterDefense()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_PERIMETER_DEFENSE);
	if( pCheck )
	{
		Bool checked = 	pCheck->GetCheck()==1;
		m_teamDict->setBool(TheKey_teamIsPerimeterDefense, checked);
		if (checked) {	// Can't be both base & perimeter defense.
			pCheck = (CButton *) GetDlgItem(IDC_BASE_DEFENSE);
			if( pCheck )
			{
				pCheck->SetCheck(0);
				m_teamDict->setBool(TheKey_teamIsBaseDefense, false);
			}
		}
	}
}

void TeamBehavior::OnBaseDefense()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_BASE_DEFENSE);
	if( pCheck )
	{
		Bool checked = 	pCheck->GetCheck()==1;
		m_teamDict->setBool(TheKey_teamIsBaseDefense, checked);
		if (checked) {	// Can't be both base & perimeter defense.
			pCheck = (CButton *) GetDlgItem(IDC_PERIMETER_DEFENSE);
			if( pCheck )
			{
				pCheck->SetCheck(0);
				m_teamDict->setBool(TheKey_teamIsPerimeterDefense, false);
			}
		}
	}
}

void TeamBehavior::OnChangePercentDestroyed()
{
	CWnd *pWnd = GetDlgItem(IDC_PERCENT_DESTROYED);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		Int percent = atoi(val);
		Real value = percent/100.0f;
		m_teamDict->setReal(TheKey_teamDestroyedThreshold, value);
	}
}

void TeamBehavior::OnSelchangeEnemyInteractions()
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_ENEMY_INTERACTIONS);
	Int mode = pCombo->GetCurSel();
	if (mode >= 0) {
		m_teamDict->setInt(TheKey_teamAggressiveness, mode + ATTITUDE_SLEEP);
	}
}

void TeamBehavior::OnSelchangeOnAllClear()
{
	updateScript(TheKey_teamAllClearScript, IDC_ON_ALL_CLEAR);
}

void TeamBehavior::OnSelchangeOnIdleScript()
{
	updateScript(TheKey_teamOnIdleScript, IDC_ON_IDLE_SCRIPT);
}

void TeamBehavior::OnAttackCommonTarget()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_ATTACK_COMMON_TARGET);
	Bool checked = 	pCheck->GetCheck()==1;
	m_teamDict->setBool(TheKey_teamAttackCommonTarget, checked);
}
