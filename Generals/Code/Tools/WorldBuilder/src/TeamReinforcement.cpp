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

// TeamReinforcement.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "TeamReinforcement.h"
#include "EditParameter.h"
#include "Common/WellKnownKeys.h"

/////////////////////////////////////////////////////////////////////////////
// TeamReinforcement dialog


TeamReinforcement::TeamReinforcement()
	: CPropertyPage(TeamReinforcement::IDD)	,
	m_teamDict(NULL)
{
	//{{AFX_DATA_INIT(TeamReinforcement)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TeamReinforcement::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TeamReinforcement)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TeamReinforcement, CPropertyPage)
	//{{AFX_MSG_MAP(TeamReinforcement)
	ON_BN_CLICKED(IDC_DEPLOY_BY, OnDeployBy)
	ON_BN_CLICKED(IDC_TEAM_STARTS_FULL, OnTeamStartsFull)
	ON_CBN_SELCHANGE(IDC_TRANSPORT_COMBO, OnSelchangeTransportCombo)
	ON_BN_CLICKED(IDC_TRANSPORTS_EXIT, OnTransportsExit)
	ON_CBN_SELCHANGE(IDC_VETERANCY, OnSelchangeVeterancy)
	ON_CBN_SELCHANGE(IDC_WAYPOINT_COMBO, OnSelchangeWaypointCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TeamReinforcement message handlers

BOOL TeamReinforcement::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	Bool exists;

	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_TRANSPORT_COMBO);
	EditParameter::loadTransports(pCombo);
	AsciiString transport = m_teamDict->getAsciiString(TheKey_teamTransport, &exists);
	Int stringNdx = pCombo->AddString(NONE_STRING);
	Int noneNdx = stringNdx;
	if (exists && !transport.isEmpty()) {
		Int ndx = pCombo->FindStringExact(-1, transport.str());
		if (ndx != CB_ERR) {
			stringNdx = ndx;
		}
	}
	pCombo->SetCurSel(stringNdx);

	CButton *pCheck = (CButton *) GetDlgItem(IDC_TEAM_STARTS_FULL);
	Bool teamStartsFull = m_teamDict->getBool(TheKey_teamStartsFull, &exists);
	pCheck->SetCheck(teamStartsFull?1:0);

	pCheck = (CButton *) GetDlgItem(IDC_TRANSPORTS_EXIT);
	Bool transportsExit = m_teamDict->getBool(TheKey_teamTransportsExit, &exists);
	pCheck->SetCheck(transportsExit?1:0);
	Bool enable = !(stringNdx==noneNdx);
	pCheck->EnableWindow(enable);
	pCombo->EnableWindow(enable);

	pCheck = (CButton *) GetDlgItem(IDC_DEPLOY_BY);
	pCheck->SetCheck( (stringNdx==noneNdx)?0:1);

	pCombo = (CComboBox *)GetDlgItem(IDC_VETERANCY);
	Int value = m_teamDict->getInt(TheKey_teamVeterancy, &exists);
	pCombo->SetCurSel(value);

	pCombo = (CComboBox *)GetDlgItem(IDC_WAYPOINT_COMBO);
	EditParameter::loadWaypoints(pCombo);
	AsciiString homeWaypoint = m_teamDict->getAsciiString(TheKey_teamReinforcementOrigin, &exists);
	stringNdx = pCombo->AddString(NONE_STRING);
	if (exists) {
		Int ndx = pCombo->FindStringExact(-1, homeWaypoint.str());
		if (ndx != CB_ERR) {
			stringNdx = ndx;
		}
	}
	pCombo->SetCurSel(stringNdx);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TeamReinforcement::OnDeployBy()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_DEPLOY_BY);
	if (pCheck->GetCheck()) {
		CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_TRANSPORT_COMBO);
		pCombo->SetCurSel(0);
		pCombo->EnableWindow();
		pCheck = (CButton *) GetDlgItem(IDC_TRANSPORTS_EXIT);
		pCheck->EnableWindow();
	}	else {
		CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_TRANSPORT_COMBO);
		pCombo->SetCurSel(-1);
		pCombo->EnableWindow(false);
		pCheck = (CButton *) GetDlgItem(IDC_TRANSPORTS_EXIT);
		pCheck->EnableWindow(false);
		m_teamDict->setAsciiString(TheKey_teamTransport, AsciiString::TheEmptyString);
	}
}

void TeamReinforcement::OnTeamStartsFull()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_TEAM_STARTS_FULL);
	Bool checked = 	pCheck->GetCheck()==1;
	m_teamDict->setBool(TheKey_teamStartsFull, checked);
}

void TeamReinforcement::OnSelchangeTransportCombo()
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_TRANSPORT_COMBO);
	CString txt;
	Int curSel = pCombo->GetCurSel();
	pCombo->GetLBText(curSel, txt);
	AsciiString comboText = AsciiString(txt);
	if (comboText == NONE_STRING) {
		comboText.clear();
	}
	m_teamDict->setAsciiString(TheKey_teamTransport, comboText);
}

void TeamReinforcement::OnTransportsExit()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_TRANSPORTS_EXIT);
	Bool checked = 	pCheck->GetCheck()==1;
	m_teamDict->setBool(TheKey_teamTransportsExit, checked);
}

void TeamReinforcement::OnSelchangeVeterancy()
{
	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_VETERANCY);
	static char buf[1024];
	int curSel = owner->GetCurSel();
	int value = 0;
	if (curSel >= 0) {
		value=curSel;
	}
	m_teamDict->setInt(TheKey_teamVeterancy, value);
}

void TeamReinforcement::OnSelchangeWaypointCombo()
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_WAYPOINT_COMBO);
	CString txt;
	Int curSel = pCombo->GetCurSel();
	if (curSel >= 0) {
		pCombo->GetLBText(curSel, txt);
	}
	AsciiString comboText = AsciiString(txt);
	if (comboText == NONE_STRING) {
		comboText.clear();
	}
	m_teamDict->setAsciiString(TheKey_teamReinforcementOrigin, comboText);
}

