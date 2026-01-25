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

// TeamObjectProperties.cpp
// Mike Lytle
// January, 2003
// (c) Electronic Arts 2003

#include "StdAfx.h"
#include "TeamObjectProperties.h"
#include "Common/MapObject.h"
#include "Common/WellKnownKeys.h"


/////////////////////////////////////////////////////////////////////////////
// TeamObjectProperties dialog

TeamObjectProperties::TeamObjectProperties(Dict* dictToEdit)
	: CPropertyPage(TeamObjectProperties::IDD),
	m_dictToEdit(dictToEdit)
{
	//{{AFX_DATA_INIT(TeamObjectProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

TeamObjectProperties::~TeamObjectProperties()
{
}

void TeamObjectProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TeamObjectProperties)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

#if 0
BEGIN_MESSAGE_MAP(TeamObjectProperties, CPropertyPage)
	//{{AFX_MSG_MAP(TeamObjectProperties)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_StartingHealth, _HealthToDict)
	ON_CBN_SELENDOK(IDC_MAPOBJECT_HitPoints, _HPsToDict)
	ON_CBN_KILLFOCUS(IDC_MAPOBJECT_HitPoints, _HPsToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Enabled, _EnabledToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Indestructible, _IndestructibleToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Unsellable, _UnsellableToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Powered, _PoweredToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Aggressiveness, _AggressivenessToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_VisionDistance, _VisibilityToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_ShroudClearingDistance, _ShroudClearingDistanceToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Veterancy, _VeterancyToDict)
 	ON_BN_CLICKED(IDC_MAPOBJECT_RecruitableAI, _RecruitableAIToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Selectable, _SelectableToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Weather, _WeatherToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Time, _TimeToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_StoppingDistance, _StoppingDistanceToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_StartingHealthEdit, _HealthToDict)
	ON_BN_CLICKED(IDC_UPDATE_TEAM_MEMBERS, _UpdateTeamMembers)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TeamObjectProperties message handlers

BOOL TeamObjectProperties::OnInitDialog()
{
	CDialog::OnInitDialog();
	updateTheUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TeamObjectProperties::OnOK()
{
	_PropertiesToDict();
}

void TeamObjectProperties::updateTheUI(void)
{
	_DictToHealth();
	_DictToHPs();
	_DictToEnabled();
	_DictToDestructible();
	_DictToPowered();
	_DictToUnsellable();
	_DictToAggressiveness();
	_DictToVisibilityRange();
	_DictToVeterancy();
	_DictToWeather();
	_DictToTime();
	_DictToShroudClearingDistance();
	_DictToRecruitableAI();
	_DictToSelectable();
	_DictToStoppingDistance();
}

void TeamObjectProperties::_DictToHealth(void)
{
	Int value = 100;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_teamObjectInitialHealth, &exists);
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_StartingHealth);
	CWnd* pItem2 = GetDlgItem(IDC_MAPOBJECT_StartingHealthEdit);
	if (pItem && pItem2) {
		if (value == 0) {
			pItem->SelectString(-1, "0%");
			pItem2->SetWindowText("");
			pItem2->EnableWindow(FALSE);
		} else if (value == 25) {
			pItem->SelectString(-1, "25%");
			pItem2->SetWindowText("");
			pItem2->EnableWindow(FALSE);
		} else if (value == 50) {
			pItem->SelectString(-1, "50%");
			pItem2->SetWindowText("");
			pItem2->EnableWindow(FALSE);
		} else if (value == 75) {
			pItem->SelectString(-1, "75%");
			pItem2->SetWindowText("");
			pItem2->EnableWindow(FALSE);
		} else if (value == 100) {
			pItem->SelectString(-1, "100%");
			pItem2->SetWindowText("");
			pItem2->EnableWindow(FALSE);
		} else {
			pItem->SelectString(-1, "Other");
			static char buff[12];
			snprintf(buff, ARRAY_SIZE(buff), "%d", value);
			pItem2->SetWindowText(buff);
			pItem2->EnableWindow(TRUE);
		}
	}
}

void TeamObjectProperties::_DictToHPs(void)
{
	Int value = -1;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_teamObjectMaxHPs, &exists);
		if (!exists)
			value = -1;
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_HitPoints);
	pItem->ResetContent();
	CString str;
	str.Format("Default For Unit");
	pItem->InsertString(-1, str);

	if (value != -1) {
		str.Format("%d", value);
		pItem->InsertString(-1, str);
		pItem->SetCurSel(1);
	} else {
		pItem->SetCurSel(0);
	}
}

void TeamObjectProperties::_DictToEnabled(void)
{
	Bool enabled = true;
	Bool exists;
	if (m_dictToEdit) {
		enabled  = m_dictToEdit->getBool(TheKey_teamObjectEnabled, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Enabled);
	if (pItem) {
		pItem->SetCheck(enabled);
	}
}

void TeamObjectProperties::_DictToDestructible(void)
{
	Bool destructible = true;
	Bool exists;
	if (m_dictToEdit) {
		destructible  = m_dictToEdit->getBool(TheKey_teamObjectIndestructible, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Indestructible);
	if (pItem) {
		pItem->SetCheck(destructible);
	}
}

void TeamObjectProperties::_DictToUnsellable(void)
{
	Bool unsellable = false;
	Bool exists;
	if (m_dictToEdit) {
		unsellable  = m_dictToEdit->getBool(TheKey_teamObjectUnsellable, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Unsellable);
	if (pItem) {
		pItem->SetCheck(unsellable);
	}
}

void TeamObjectProperties::_DictToPowered(void)
{
	Bool powered = true;
	Bool exists;
	if (m_dictToEdit) {
		powered  = m_dictToEdit->getBool(TheKey_teamObjectPowered, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Powered);
	if (pItem) {
		pItem->SetCheck(powered);
	}

}

void TeamObjectProperties::_DictToAggressiveness(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_teamObjectAggressiveness, &exists);
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_Aggressiveness);
	if (pItem) {
		if (value == -2) {
			pItem->SelectString(-1, "Sleep");
		} else if (value == -1) {
			pItem->SelectString(-1, "Passive");
		} else if (value == 0) {
			pItem->SelectString(-1, "Normal");
		} else if (value == 1) {
			pItem->SelectString(-1, "Alert");
		} else if (value == 2) {
			pItem->SelectString(-1, "Aggressive");
		}
	}
}

void TeamObjectProperties::_DictToVisibilityRange(void)
{
	Int distance = 0;
	Bool exists;
	if (m_dictToEdit) {
		distance = m_dictToEdit->getInt(TheKey_teamObjectVisualRange, &exists);
	}

	CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_VisionDistance);
	if (pItem) {
		static char buff[12];
		snprintf(buff, ARRAY_SIZE(buff), "%d", distance);
		if (distance == 0) {
			pItem->SetWindowText("");
		} else {
			pItem->SetWindowText(buff);
		}
	}
}

void TeamObjectProperties::_DictToVeterancy(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_teamObjectVeterancy, &exists);
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_Veterancy);
	if (pItem) {
		pItem->SetCurSel(value);
	}
}

void TeamObjectProperties::_DictToWeather(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_teamObjectWeather, &exists);
		if (!exists)
			value = 0;
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_Weather);
	pItem->SetCurSel(value);
}

void TeamObjectProperties::_DictToTime(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_teamObjectTime, &exists);
		if (!exists)
			value = 0;
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_Time);
	pItem->SetCurSel(value);
}

void TeamObjectProperties::_DictToShroudClearingDistance(void)
{
	Int distance = 0;
	Bool exists;
	if (m_dictToEdit) {
		distance = m_dictToEdit->getInt(TheKey_teamObjectShroudClearingDistance, &exists);
	}

	CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_ShroudClearingDistance);
	if (pItem) {
		static char buff[12];
		snprintf(buff, ARRAY_SIZE(buff), "%d", distance);
		if (distance == 0) {
			pItem->SetWindowText("");
		} else {
			pItem->SetWindowText(buff);
		}
	}
}

void TeamObjectProperties::_DictToRecruitableAI(void)
{
 	Bool recruitableAI = true;
 	Bool exists;
 	if (m_dictToEdit) {
		recruitableAI  = m_dictToEdit->getBool(TheKey_teamObjectRecruitableAI, &exists);
 	}

 	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_RecruitableAI);
 	if (pItem) {
		pItem->SetCheck(recruitableAI);
	}
}

void TeamObjectProperties::_DictToSelectable(void)
{
	Bool selectable = true;
	Bool exists;
	if (m_dictToEdit) {
		selectable  = m_dictToEdit->getBool(TheKey_teamObjectSelectable, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Selectable);
	if (pItem) {
		pItem->SetCheck(selectable);
	}
}

void TeamObjectProperties::_DictToStoppingDistance(void)
{
	Real stoppingDistance = 1.0f;
	Bool exists = false;
	if (m_dictToEdit) {
		stoppingDistance = m_dictToEdit->getReal(TheKey_teamObjectStoppingDistance, &exists);
	}

	CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_StoppingDistance);
	if (pItem) {
		static char buff[64];
		sprintf(buff, ARRAY_SIZE(buff), "%g", stoppingDistance);
		if (stoppingDistance == 0) {
			pItem->SetWindowText("");
		} else {
			pItem->SetWindowText(buff);
		}
	}
}

void TeamObjectProperties::_HealthToDict(void)
{
	Int value;
	static char buf[1024];

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_StartingHealth);
	owner->GetWindowText(buf, sizeof(buf)-2);
	if (strcmp(buf, "Dead") == 0) {
		value = 0;
	} else if (strcmp(buf, "25%") == 0) {
		value = 25;
	} else if (strcmp(buf, "50%") == 0) {
		value = 50;
	} else if (strcmp(buf, "75%") == 0) {
		value = 75;
	} else if (strcmp(buf, "100%") == 0) {
		value = 100;
	} else if (strcmp(buf, "Other") == 0) {
		CWnd* edit = GetDlgItem(IDC_MAPOBJECT_StartingHealthEdit);
		edit->EnableWindow(TRUE);
		CString cstr;
		edit->GetWindowText(cstr);
		if (cstr.IsEmpty()) {
			return;
		}
		value = atoi(cstr.GetBuffer(0));
	}

	m_dictToEdit->setInt(TheKey_teamObjectInitialHealth, value);
}

void TeamObjectProperties::_EnabledToDict(void)
{
	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Enabled);
	Bool isChecked = (owner->GetCheck() != 0);

	m_dictToEdit->setBool(TheKey_teamObjectEnabled, isChecked);
}


void TeamObjectProperties::_IndestructibleToDict(void)
{
	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Indestructible);
	Bool isChecked = (owner->GetCheck() != 0);

	m_dictToEdit->setBool(TheKey_teamObjectIndestructible, isChecked);
}

void TeamObjectProperties::_UnsellableToDict(void)
{
	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Unsellable);
	Bool isChecked = (owner->GetCheck() != 0);

	m_dictToEdit->setBool(TheKey_teamObjectUnsellable, isChecked);
}

void TeamObjectProperties::_PoweredToDict(void)
{
	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Powered);
	Bool isChecked = (owner->GetCheck() != 0);

	m_dictToEdit->setBool(TheKey_teamObjectPowered, isChecked);
}

void TeamObjectProperties::_AggressivenessToDict(void)
{
	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Aggressiveness);
	static char buf[1024];
	owner->GetWindowText(buf, sizeof(buf)-2);
	int value = 0;

	if (strcmp(buf, "Sleep") == 0) {
		value = -2;
	} else if (strcmp(buf, "Passive") == 0) {
		value = -1;
	} else if (strcmp(buf, "Normal") == 0) {
		value = 0;
	} else if (strcmp(buf, "Alert") == 0) {
		value = 1;
	} else if (strcmp(buf, "Aggressive") == 0) {
		value = 2;
	}

	m_dictToEdit->setInt(TheKey_teamObjectAggressiveness, value);
}

void TeamObjectProperties::_VisibilityToDict(void)
{
	int value = -1;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_VisionDistance);
	edit->EnableWindow(TRUE);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		value = atoi(cstr.GetBuffer(0));
	}

	if (value != -1) {
		m_dictToEdit->setInt(TheKey_teamObjectVisualRange, value);
	}
}

void TeamObjectProperties::_VeterancyToDict(void)
{
	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Veterancy);
	static char buf[1024];
	int curSel = owner->GetCurSel();
	int value = 0;
	if (curSel >= 0) {
		value=curSel;
	}

	m_dictToEdit->setInt(TheKey_teamObjectVeterancy, value);
}

void TeamObjectProperties::_WeatherToDict(void)
{
	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Weather);
	static char buf[1024];
	int curSel = owner->GetCurSel();

	m_dictToEdit->setInt(TheKey_teamObjectWeather, curSel);
}

void TeamObjectProperties::_TimeToDict(void)
{
	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Time);
	static char buf[1024];
	int curSel = owner->GetCurSel();

	m_dictToEdit->setInt(TheKey_teamObjectTime, curSel);
}

void TeamObjectProperties::_ShroudClearingDistanceToDict(void)
{
	int value = -1;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_ShroudClearingDistance);
	edit->EnableWindow(TRUE);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		value = atoi(cstr.GetBuffer(0));
	}

	if (value != -1) {
		m_dictToEdit->setInt(TheKey_teamObjectShroudClearingDistance, value);
	}
}

void TeamObjectProperties::_RecruitableAIToDict(void)
{
	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_RecruitableAI);
	Bool isChecked = (owner->GetCheck() != 0);

	m_dictToEdit->setBool(TheKey_teamObjectRecruitableAI, isChecked);
}

void TeamObjectProperties::_SelectableToDict(void)
{
	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Selectable);
	Bool isChecked = (owner->GetCheck() != 0);

	m_dictToEdit->setBool(TheKey_teamObjectSelectable, isChecked);
}

void TeamObjectProperties::_HPsToDict()
{
	Int value;
	static char buf[1024];

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_HitPoints);
	owner->GetWindowText(buf, sizeof(buf)-2);
	value = atoi(buf);
	if (value == 0)
		value = -1;

	m_dictToEdit->setInt(TheKey_teamObjectMaxHPs, value);
}

void TeamObjectProperties::_StoppingDistanceToDict(void)
{
	Real value;
	static char buf[1024];

	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_StoppingDistance);
	CString cstr;
	edit->GetWindowText(cstr);
	if (cstr.IsEmpty()) {
		return;
	}
	value = atof(cstr.GetBuffer(0));

	m_dictToEdit->setReal(TheKey_teamObjectStoppingDistance, value);
}

void TeamObjectProperties::_UpdateTeamMembers()
{
	CString msg;
	msg.Format("Are you sure you want to change the team members?");
	if (::AfxMessageBox(msg, MB_YESNO) == IDNO) {
		return;
	}

	AsciiString teamName = m_dictToEdit->getAsciiString(TheKey_teamName);

	MapObject *pObj;
	for (pObj=MapObject::getFirstMapObject(); pObj; pObj=pObj->getNext()) {
		Dict* objectDict = pObj->getProperties();
		DEBUG_ASSERTCRASH(objectDict, ("objectDict shouldn't be null"));

		AsciiString objectsTeam = objectDict->getAsciiString(TheKey_originalOwner);

		if (teamName == objectsTeam) {
			DEBUG_ASSERTCRASH(m_dictToEdit, ("m_dictToEdit shouldn't be null"));
			Bool exists;

			Int value = m_dictToEdit->getInt(TheKey_teamObjectInitialHealth, &exists);
			objectDict->setInt(TheKey_objectInitialHealth, value);

			value = m_dictToEdit->getInt(TheKey_teamObjectMaxHPs, &exists);
			objectDict->setInt(TheKey_objectMaxHPs, value);

			Bool enabled = m_dictToEdit->getBool(TheKey_teamObjectEnabled, &exists);
			objectDict->setBool(TheKey_objectEnabled, enabled);

			enabled = m_dictToEdit->getBool(TheKey_teamObjectIndestructible, &exists);
			objectDict->setBool(TheKey_objectIndestructible, enabled);

			enabled = m_dictToEdit->getBool(TheKey_teamObjectUnsellable, &exists);
			objectDict->setBool(TheKey_objectUnsellable, enabled);

			enabled = m_dictToEdit->getBool(TheKey_teamObjectPowered, &exists);
			objectDict->setBool(TheKey_objectPowered, enabled);

			value = m_dictToEdit->getInt(TheKey_teamObjectAggressiveness, &exists);
			objectDict->setInt(TheKey_objectAggressiveness, value);

			value = m_dictToEdit->getInt(TheKey_teamObjectVisualRange, &exists);
			objectDict->setInt(TheKey_objectVisualRange, value);

			value = m_dictToEdit->getInt(TheKey_teamObjectVeterancy, &exists);
			objectDict->setInt(TheKey_objectVeterancy, value);

			value = m_dictToEdit->getInt(TheKey_teamObjectWeather, &exists);
			objectDict->setInt(TheKey_objectWeather, value);

			value = m_dictToEdit->getInt(TheKey_teamObjectTime, &exists);
			objectDict->setInt(TheKey_objectTime, value);

			value = m_dictToEdit->getInt(TheKey_teamObjectShroudClearingDistance, &exists);
			objectDict->setInt(TheKey_objectShroudClearingDistance, value);

			enabled = m_dictToEdit->getBool(TheKey_teamObjectRecruitableAI, &exists);
			objectDict->setBool(TheKey_objectRecruitableAI, enabled);

			enabled = m_dictToEdit->getBool(TheKey_teamObjectSelectable, &exists);
			objectDict->setBool(TheKey_objectSelectable, enabled);

			Real dist = m_dictToEdit->getReal(TheKey_teamObjectStoppingDistance, &exists);
			objectDict->setReal(TheKey_objectStoppingDistance, dist);

		}
	}
}


void TeamObjectProperties::_PropertiesToDict()
{
	// Make sure that the attributes are in the dictionary.
	_HealthToDict();
	_HPsToDict();
	_EnabledToDict();
	_IndestructibleToDict();
	_UnsellableToDict();
	_PoweredToDict();
	_AggressivenessToDict();
	_VisibilityToDict();
	_ShroudClearingDistanceToDict();
	_VeterancyToDict();
	_RecruitableAIToDict();
	_SelectableToDict();
	_WeatherToDict();
	_TimeToDict();
	_StoppingDistanceToDict();
	_HealthToDict();
}

#endif

BOOL TeamObjectProperties::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return CPropertyPage::OnCommand(wParam, lParam);
}

