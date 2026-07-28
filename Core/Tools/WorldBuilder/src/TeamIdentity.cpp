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

// TeamIdentity.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "TeamIdentity.h"
#include "EditParameter.h"
#include "PickUnitDialog.h"
#include "Common/WellKnownKeys.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"
#include "GameLogic/SidesList.h"

static const char* NEUTRAL_NAME_STR = "(neutral)";

/////////////////////////////////////////////////////////////////////////////
// TeamIdentity dialog


TeamIdentity::TeamIdentity()
	: CPropertyPage(TeamIdentity::IDD)
{
	//{{AFX_DATA_INIT(TeamIdentity)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TeamIdentity::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TeamIdentity)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TeamIdentity, CPropertyPage)
	//{{AFX_MSG_MAP(TeamIdentity)
	ON_BN_CLICKED(IDC_AI_RECRUITABLE, OnAiRecruitable)
	ON_BN_CLICKED(IDC_AUTO_REINFORCE, OnAutoReinforce)
	ON_EN_CHANGE(IDC_DESCRIPTION, OnChangeDescription)
	ON_EN_CHANGE(IDC_MAX, OnChangeMax)
	ON_EN_CHANGE(IDC_PRIORITY_DECREASE, OnChangePriorityDecrease)
	ON_EN_CHANGE(IDC_PRIORITY_INCREASE, OnChangePriorityIncrease)
	ON_CBN_SELCHANGE(IDC_PRODUCTION_CONDITION, OnSelchangeProductionCondition)
	ON_EN_CHANGE(IDC_PRODUCTION_PRIORITY, OnChangeProductionPriority)
	ON_CBN_SELCHANGE(IDC_HOME_WAYPOINT, OnSelchangeHomeWaypoint)
	ON_BN_CLICKED(IDC_UNIT_TYPE1_BUTTON, OnUnitType1Button)
	ON_BN_CLICKED(IDC_UNIT_TYPE2_BUTTON, OnUnitType2Button)
	ON_BN_CLICKED(IDC_UNIT_TYPE3_BUTTON, OnUnitType3Button)
	ON_BN_CLICKED(IDC_UNIT_TYPE4_BUTTON, OnUnitType4Button)
	ON_BN_CLICKED(IDC_UNIT_TYPE5_BUTTON, OnUnitType5Button)
	ON_BN_CLICKED(IDC_UNIT_TYPE6_BUTTON, OnUnitType6Button)
	ON_BN_CLICKED(IDC_UNIT_TYPE7_BUTTON, OnUnitType7Button)
	ON_BN_CLICKED(IDC_PRODUCTION_EXECUTEACTIONS, OnExecuteActions)
	ON_EN_CHANGE(IDC_TEAM_NAME, OnChangeTeamName)
	ON_BN_CLICKED(IDC_TEAM_SINGLETON, OnTeamSingleton)
	ON_EN_KILLFOCUS(IDC_TEAM_NAME, OnKillfocusTeamName)
	ON_CBN_SELENDOK(IDC_TEAMOWNER, OnSelendokTeamowner)
	ON_EN_CHANGE(IDC_TEAM_BUILD_FRAMES, OnChangeTeamBuildFrames)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TeamIdentity message handlers

BOOL TeamIdentity::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	loadUnitsInfo(IDC_MIN_UNIT1, TheKey_teamUnitMinCount1,
								IDC_MAX_UNIT1, TheKey_teamUnitMaxCount1,
								IDC_UNIT_TYPE1, TheKey_teamUnitType1);

	loadUnitsInfo(IDC_MIN_UNIT2, TheKey_teamUnitMinCount2,
								IDC_MAX_UNIT2, TheKey_teamUnitMaxCount2,
								IDC_UNIT_TYPE2, TheKey_teamUnitType2);

	loadUnitsInfo(IDC_MIN_UNIT3, TheKey_teamUnitMinCount3,
								IDC_MAX_UNIT3, TheKey_teamUnitMaxCount3,
								IDC_UNIT_TYPE3, TheKey_teamUnitType3);

	loadUnitsInfo(IDC_MIN_UNIT4, TheKey_teamUnitMinCount4,
								IDC_MAX_UNIT4, TheKey_teamUnitMaxCount4,
								IDC_UNIT_TYPE4, TheKey_teamUnitType4);

	loadUnitsInfo(IDC_MIN_UNIT5, TheKey_teamUnitMinCount5,
								IDC_MAX_UNIT5, TheKey_teamUnitMaxCount5,
								IDC_UNIT_TYPE5, TheKey_teamUnitType5);

	loadUnitsInfo(IDC_MIN_UNIT6, TheKey_teamUnitMinCount6,
								IDC_MAX_UNIT6, TheKey_teamUnitMaxCount6,
								IDC_UNIT_TYPE6, TheKey_teamUnitType6);

	loadUnitsInfo(IDC_MIN_UNIT7, TheKey_teamUnitMinCount7,
								IDC_MAX_UNIT7, TheKey_teamUnitMaxCount7,
								IDC_UNIT_TYPE7, TheKey_teamUnitType7);

	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_HOME_WAYPOINT);
	Bool exists;
	EditParameter::loadWaypoints(pCombo);
	AsciiString homeWaypoint = m_teamDict->getAsciiString(TheKey_teamHome, &exists);
	Int stringNdx = pCombo->AddString(NONE_STRING);
	if (exists) {
		Int ndx = pCombo->FindStringExact(-1, homeWaypoint.str());
		if (ndx != CB_ERR) {
			stringNdx = ndx;
		}
	}
	pCombo->SetCurSel(stringNdx);

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_TEAMOWNER);
	owner->ResetContent();
	for (Int i = 0; i < TheSidesList->getNumSides(); i++)
	{
		AsciiString name = TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName);
		if (name.isEmpty())
			name = NEUTRAL_NAME_STR;
		owner->AddString(name.str());
	}

	// must re-find, since list is sorted
	AsciiString cur_oname = m_teamDict->getAsciiString(TheKey_teamOwner);
	Int myPlayerIndex = -1;
	TheSidesList->findSideInfo(cur_oname, &myPlayerIndex);
	AsciiString oname_ui = TheSidesList->getSideInfo(myPlayerIndex)->getDict()->getAsciiString(TheKey_playerName);
	int oindex_in_list = owner->FindStringExact(-1, oname_ui.str());
	DEBUG_ASSERTCRASH(oindex_in_list >= 0, ("hmm"));
	owner->SetCurSel(oindex_in_list);

	CButton *pCheck = (CButton *) GetDlgItem(IDC_AUTO_REINFORCE);
	Bool autoReinf = m_teamDict->getBool(TheKey_teamAutoReinforce, &exists);
	pCheck->SetCheck(autoReinf?1:0);

	pCheck = (CButton*)GetDlgItem(IDC_TEAM_SINGLETON);
	Bool singleton = m_teamDict->getBool(TheKey_teamIsSingleton, &exists);
	pCheck->SetCheck(singleton?1:0);

	pCheck = (CButton *) GetDlgItem(IDC_AI_RECRUITABLE);

	Bool aiRecruit = m_teamDict->getBool(TheKey_teamIsAIRecruitable, &exists);
	pCheck->SetCheck(aiRecruit?1:0);

	CWnd *pWnd = GetDlgItem(IDC_DESCRIPTION);
	AsciiString description = m_teamDict->getAsciiString(TheKey_teamDescription, &exists);
	pWnd->SetWindowText(description.str());

	pWnd = GetDlgItem(IDC_TEAM_NAME);
	description = m_teamDict->getAsciiString(TheKey_teamName, &exists);
	pWnd->SetWindowText(description.str());

	pWnd = GetDlgItem(IDC_MAX);
	Int maxInstances = m_teamDict->getInt(TheKey_teamMaxInstances, &exists);
	if (!exists) maxInstances = 1;
	description.format("%d", maxInstances);
	pWnd->SetWindowText(description.str());

	pWnd = GetDlgItem(IDC_PRODUCTION_PRIORITY);
	Int priority = m_teamDict->getInt(TheKey_teamProductionPriority, &exists);
	description.format("%d", priority);
	pWnd->SetWindowText(description.str());

	pWnd = GetDlgItem(IDC_PRIORITY_INCREASE);
	Int increase = m_teamDict->getInt(TheKey_teamProductionPrioritySuccessIncrease, &exists);
	description.format("%d", increase);
	pWnd->SetWindowText(description.str());

	pWnd = GetDlgItem(IDC_PRIORITY_DECREASE);
	Int decrease = m_teamDict->getInt(TheKey_teamProductionPriorityFailureDecrease, &exists);
	description.format("%d", decrease);
	pWnd->SetWindowText(description.str());

	pWnd = GetDlgItem(IDC_TEAM_BUILD_FRAMES);
	Int idleFrames = m_teamDict->getInt(TheKey_teamInitialIdleFrames, &exists);
	description.format("%d", idleFrames);
	pWnd->SetWindowText(description.str());

	pCombo = (CComboBox*)GetDlgItem(IDC_PRODUCTION_CONDITION);
	// Load the subroutine scripts into the combo box.
	EditParameter::loadScripts(pCombo, true);
	stringNdx = pCombo->AddString(NONE_STRING);
	pCombo->SetFocus();
	AsciiString script = m_teamDict->getAsciiString(TheKey_teamProductionCondition, &exists);
	if (exists) {
		Int ndx = pCombo->FindStringExact(-1, script.str());
		if (ndx != CB_ERR) {
			stringNdx = ndx;
		}
	}
	pCombo->SetCurSel(stringNdx);

	CButton *pButton = (CButton*) GetDlgItem(IDC_PRODUCTION_EXECUTEACTIONS);
	if (pButton) {
		Bool executeActions = m_teamDict->getBool(TheKey_teamExecutesActionsOnCreate, &exists);
		if (!exists) {
			m_teamDict->setBool(TheKey_teamExecutesActionsOnCreate, false);
			executeActions = false;
		}
		pButton->SetCheck(executeActions ? 1 : 0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TeamIdentity::loadUnitsInfo(int idcMinUnit, NameKeyType keyMinUnit,
								int idcMaxUnit, NameKeyType keyMaxUnit,
								int idcUnitType, NameKeyType keyUnitType)
{
	CEdit *pEdit = (CEdit *)GetDlgItem(idcMinUnit);
	CString text;
	Bool exists;
	text.Format("%d", m_teamDict->getInt(keyMinUnit, &exists));
	pEdit->SetWindowText(text);

	pEdit = (CEdit *)GetDlgItem(idcMaxUnit);
	text.Format("%d", m_teamDict->getInt(keyMaxUnit, &exists));
	pEdit->SetWindowText(text);

	AsciiString type = m_teamDict->getAsciiString(keyUnitType, &exists);
	if (type.isEmpty()) type = NONE_STRING;

	CComboBox *pCombo = (CComboBox *)GetDlgItem(idcUnitType);
	pCombo->ResetContent();

	Bool found = false;

	// add entries from the thing factory as the available UNITS to use
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
			 tTemplate;
			 tTemplate = tTemplate->friend_getNextTemplate() ) {

		// next tier uses the editor sorting bits that design can specify in the INI
		EditorSortingType sort = tTemplate->getEditorSorting();
		// TheSuperHackers @tweak DayV 20/07/2025 Allow structures in teams.
		if (sort != ES_VEHICLE && sort != ES_INFANTRY && sort != ES_STRUCTURE) {
			continue;
		}

		Int ndx = pCombo->AddString(tTemplate->getName().str());
		if (type == tTemplate->getName()) {
			found = true;
			pCombo->SetCurSel(ndx);
		}
	}

	Int ndx = pCombo->AddString(NONE_STRING);
	if (!found) {
		pCombo->SetCurSel(ndx);
	}

}


BOOL TeamIdentity::OnCommand(WPARAM wParam, LPARAM lParam)
{

	Int wNotifyCode = HIWORD(wParam); // notification code
	Int wID = LOWORD(wParam);         // item, control, or accelerator identifier
	NameKeyType key;
	if (wNotifyCode == EN_CHANGE) {
		Int editCtrl = wID;
		switch (editCtrl) {
			default: editCtrl = 0;
			case IDC_MIN_UNIT1: key = TheKey_teamUnitMinCount1; break;
			case IDC_MAX_UNIT1: key = TheKey_teamUnitMaxCount1; break;

			case IDC_MIN_UNIT2: key = TheKey_teamUnitMinCount2; break;
			case IDC_MAX_UNIT2: key = TheKey_teamUnitMaxCount2; break;

			case IDC_MIN_UNIT3: key = TheKey_teamUnitMinCount3; break;
			case IDC_MAX_UNIT3: key = TheKey_teamUnitMaxCount3; break;

			case IDC_MIN_UNIT4: key = TheKey_teamUnitMinCount4; break;
			case IDC_MAX_UNIT4: key = TheKey_teamUnitMaxCount4; break;

			case IDC_MIN_UNIT5: key = TheKey_teamUnitMinCount5; break;
			case IDC_MAX_UNIT5: key = TheKey_teamUnitMaxCount5; break;

			case IDC_MIN_UNIT6: key = TheKey_teamUnitMinCount6; break;
			case IDC_MAX_UNIT6: key = TheKey_teamUnitMaxCount6; break;

			case IDC_MIN_UNIT7: key = TheKey_teamUnitMinCount7; break;
			case IDC_MAX_UNIT7: key = TheKey_teamUnitMaxCount7; break;
		}
		if (editCtrl != 0) {
			CEdit *pEdit = (CEdit *)GetDlgItem(editCtrl);
			CString txt;
			pEdit->GetWindowText(txt);
			Int theInt;
			if (1==sscanf(txt, "%d", &theInt)) {
				m_teamDict->setInt(key, theInt);
			}
			return true;
		}
	}	else if (wNotifyCode == CBN_SELCHANGE) {
		Int cmboCtrl = wID;
		switch (cmboCtrl) {
			case IDC_UNIT_TYPE1: key = TheKey_teamUnitType1; break;
			case IDC_UNIT_TYPE2: key = TheKey_teamUnitType2; break;
			case IDC_UNIT_TYPE3: key = TheKey_teamUnitType3; break;
			case IDC_UNIT_TYPE4: key = TheKey_teamUnitType4; break;
			case IDC_UNIT_TYPE5: key = TheKey_teamUnitType5; break;
			case IDC_UNIT_TYPE6: key = TheKey_teamUnitType6; break;
			case IDC_UNIT_TYPE7: key = TheKey_teamUnitType7; break;
			default: cmboCtrl = 0;
		}
		if (cmboCtrl != 0) {
			CComboBox *pCombo = (CComboBox*)GetDlgItem(cmboCtrl);
			CString txt;
			pCombo->GetWindowText(txt);
			AsciiString comboText = AsciiString(txt);
			m_teamDict->setAsciiString(key, comboText);
			return true;
		}
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}

void TeamIdentity::OnAiRecruitable()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_AI_RECRUITABLE);
	Bool checked = 	pCheck->GetCheck()==1;
	m_teamDict->setBool(TheKey_teamIsAIRecruitable, checked);
}

void TeamIdentity::OnAutoReinforce()
{
	CButton *pCheck = (CButton *) GetDlgItem(IDC_AUTO_REINFORCE);
	Bool checked = 	pCheck->GetCheck()==1;
	m_teamDict->setBool(TheKey_teamAutoReinforce, checked);
}


void TeamIdentity::OnChangeDescription()
{
	CWnd *pWnd = GetDlgItem(IDC_DESCRIPTION);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		AsciiString des = static_cast<LPCSTR>(val);
		m_teamDict->setAsciiString(TheKey_teamDescription, des);
	}
}

void TeamIdentity::OnChangeMax()
{
	CWnd *pWnd = GetDlgItem(IDC_MAX);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		Int maxInstances = atoi(val);
		m_teamDict->setInt(TheKey_teamMaxInstances, maxInstances);
	}
}

void TeamIdentity::OnChangePriorityDecrease()
{
	CWnd *pWnd = GetDlgItem(IDC_PRIORITY_DECREASE);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		Int decrease = atoi(val);
		m_teamDict->setInt(TheKey_teamProductionPriorityFailureDecrease, decrease);
	}
}

void TeamIdentity::OnChangePriorityIncrease()
{
	CWnd *pWnd = GetDlgItem(IDC_PRIORITY_INCREASE);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		Int increase = atoi(val);
		m_teamDict->setInt(TheKey_teamProductionPrioritySuccessIncrease, increase);
	}
}

void TeamIdentity::OnSelchangeProductionCondition()
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_PRODUCTION_CONDITION);
	CString txt;
	Int curSel = pCombo->GetCurSel();
	pCombo->GetLBText(curSel, txt);
	AsciiString comboText = AsciiString(txt);
	if (comboText == NONE_STRING) {
		comboText.clear();
	}
	m_teamDict->setAsciiString(TheKey_teamProductionCondition, comboText);
}

void TeamIdentity::OnChangeProductionPriority()
{
	CWnd *pWnd = GetDlgItem(IDC_PRODUCTION_PRIORITY);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		Int priority = atoi(val);
		m_teamDict->setInt(TheKey_teamProductionPriority, priority);
	}
}

void TeamIdentity::OnSelchangeHomeWaypoint()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_HOME_WAYPOINT);
	CString txt;
	Int curSel = pCombo->GetCurSel();
	pCombo->GetLBText(curSel, txt);
	AsciiString comboText = AsciiString(txt);
	if (comboText == NONE_STRING) {
		comboText.clear();
	}
	m_teamDict->setAsciiString(TheKey_teamHome, comboText);
}

void TeamIdentity::OnUnitTypeButton(Int idcUnitType)
{
	PickUnitDialog dlg;
	dlg.SetAllowableType(ES_VEHICLE);
	dlg.SetAllowableType(ES_INFANTRY);
	// TheSuperHackers @tweak DayV 20/07/2025 Allow structures in teams.
	dlg.SetAllowableType(ES_STRUCTURE);
	if (dlg.DoModal() == IDOK) {
		AsciiString unit = dlg.getPickedUnit();
		NameKeyType keyUnitType;

		switch (idcUnitType)
		{
			case IDC_UNIT_TYPE1:
				keyUnitType = TheKey_teamUnitType1;
				break;
			case IDC_UNIT_TYPE2:
				keyUnitType = TheKey_teamUnitType2;
				break;
			case IDC_UNIT_TYPE3:
				keyUnitType = TheKey_teamUnitType3;
				break;
			case IDC_UNIT_TYPE4:
				keyUnitType = TheKey_teamUnitType4;
				break;
			case IDC_UNIT_TYPE5:
				keyUnitType = TheKey_teamUnitType5;
				break;
			case IDC_UNIT_TYPE6:
				keyUnitType = TheKey_teamUnitType6;
				break;
			case IDC_UNIT_TYPE7:
				keyUnitType = TheKey_teamUnitType7;
				break;
		}
		m_teamDict->setAsciiString(keyUnitType, unit);
		CComboBox *pCombo = (CComboBox *)GetDlgItem(idcUnitType);
		pCombo->SelectString(-1, unit.str());
	}

}

void TeamIdentity::OnUnitType1Button()
{
	OnUnitTypeButton(IDC_UNIT_TYPE1);
}

void TeamIdentity::OnUnitType2Button()
{
	OnUnitTypeButton(IDC_UNIT_TYPE2);
}

void TeamIdentity::OnUnitType3Button()
{
	OnUnitTypeButton(IDC_UNIT_TYPE3);
}

void TeamIdentity::OnUnitType4Button()
{
	OnUnitTypeButton(IDC_UNIT_TYPE4);
}

void TeamIdentity::OnUnitType5Button()
{
	OnUnitTypeButton(IDC_UNIT_TYPE5);
}

void TeamIdentity::OnUnitType6Button()
{
	OnUnitTypeButton(IDC_UNIT_TYPE6);
}

void TeamIdentity::OnUnitType7Button()
{
	OnUnitTypeButton(IDC_UNIT_TYPE7);
}

void TeamIdentity::OnExecuteActions()
{
	CButton *pButton = (CButton*) GetDlgItem(IDC_PRODUCTION_EXECUTEACTIONS);
	if (!pButton) {
		return;
	}

	m_teamDict->setBool(TheKey_teamExecutesActionsOnCreate, pButton->GetCheck() ? true : false);
}

void TeamIdentity::OnChangeTeamName()
{

}

void TeamIdentity::OnTeamSingleton()
{
	CButton *singleton = (CButton*)GetDlgItem(IDC_TEAM_SINGLETON);
	m_teamDict->setBool(TheKey_teamIsSingleton, singleton->GetCheck() != 0);
}

void TeamIdentity::OnKillfocusTeamName()
{
	CWnd *pWnd = GetDlgItem(IDC_TEAM_NAME);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		AsciiString tnamenew = static_cast<LPCSTR>(val);
		AsciiString tnamecur = m_teamDict->getAsciiString(TheKey_teamName);
		Bool set = true;
		if (tnamecur != tnamenew) {
			if (m_sides->findTeamInfo(tnamenew) || m_sides->findSideInfo(tnamenew))
			{
				::AfxMessageBox(IDS_NAME_IN_USE);
				set = false;
			}
			else
			{
				Int count = MapObject::countMapObjectsWithOwner(tnamecur);
				if (count > 0)
				{
					set = false;
					CString msg;
					msg.Format(IDS_RENAMING_INUSE_TEAM, count);
					if (::AfxMessageBox(msg, MB_YESNO) == IDYES)
						set = true;
				}
			}
		}
		if (set) {
			m_teamDict->setAsciiString(TheKey_teamName, tnamenew);
		} else {
			Bool exists;
			AsciiString description = m_teamDict->getAsciiString(TheKey_teamName, &exists);
			pWnd->SetWindowText(description.str());
		}
	}
}

void TeamIdentity::OnSelendokTeamowner()
{
	CWnd *pWnd = GetDlgItem(IDC_TEAMOWNER);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		AsciiString des = static_cast<LPCSTR>(val);
		m_teamDict->setAsciiString(TheKey_teamOwner, des);
	}
}

void TeamIdentity::OnChangeTeamBuildFrames()
{
	CWnd *pWnd = GetDlgItem(IDC_TEAM_BUILD_FRAMES);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		Int idleFrames = atoi(val);
		m_teamDict->setInt(TheKey_teamInitialIdleFrames, idleFrames);
	}

}
