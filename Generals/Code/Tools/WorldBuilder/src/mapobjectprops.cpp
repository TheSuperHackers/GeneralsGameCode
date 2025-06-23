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

// mapobjectprops.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "wbview.h"
#include "mapobjectprops.h"
#include "propedit.h"
#include "CUndoable.h"
#include "EditParameter.h"

#include "Common/ThingTemplate.h"
#include "Common/UnicodeString.h"
#include "Common/WellKnownKeys.h"

#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/Module/GenerateMinefieldBehavior.h"

const char* NEUTRAL_TEAM_UI_STR = "(neutral)";
const char* NEUTRAL_TEAM_INTERNAL_STR = "team";


/////////////////////////////////////////////////////////////////////////////
// MapObjectProps dialog

/*static*/ MapObjectProps *MapObjectProps::TheMapObjectProps = NULL;

void MapObjectProps::makeMain()
{
	DEBUG_ASSERTCRASH(TheMapObjectProps == NULL, ("already have a main props"));
	if (TheMapObjectProps == NULL)
		TheMapObjectProps = this;
}

MapObjectProps::MapObjectProps(Dict* dictToEdit, const char* title, CWnd* pParent /*=NULL*/) : 
	COptionsPanel(MapObjectProps::IDD, pParent),
	m_dictToEdit(dictToEdit),
	m_title(title),
	m_selectedObject(NULL)
{
	//{{AFX_DATA_INIT(MapObjectProps)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT	
}

MapObjectProps::~MapObjectProps()
{
	if (TheMapObjectProps == this)
		TheMapObjectProps = NULL;
}

void MapObjectProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MapObjectProps)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MapObjectProps, CDialog)
	//{{AFX_MSG_MAP(MapObjectProps)
	ON_LBN_SELCHANGE(IDC_PROPERTIES, OnSelchangeProperties)
	ON_BN_CLICKED(IDC_EDITPROP, OnEditprop)
	ON_BN_CLICKED(IDC_NEWPROP, OnNewprop)
	ON_BN_CLICKED(IDC_REMOVEPROP, OnRemoveprop)
	ON_LBN_DBLCLK(IDC_PROPERTIES, OnDblclkProperties)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Team, _TeamToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_Name, _NameToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_StartingHealth, _HealthToDict)
	ON_CBN_SELENDOK(IDC_MAPOBJECT_HitPoints, _HPsToDict)
	ON_CBN_KILLFOCUS(IDC_MAPOBJECT_HitPoints, _HPsToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Enabled, _EnabledToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Script, _ScriptToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Indestructible, _IndestructibleToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Unsellable, _UnsellableToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Targetable, _TargetableToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Powered, _PoweredToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Aggressiveness, _AggressivenessToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_VisionDistance, _VisibilityToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_ShroudClearingDistance, _ShroudClearingDistanceToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Veterancy, _VeterancyToDict)
 	ON_BN_CLICKED(IDC_MAPOBJECT_RecruitableAI, _RecruitableAIToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Selectable, _SelectableToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Weather, _WeatherToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Time, _TimeToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_ZOffset, SetZOffset)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_Angle, SetAngle)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_StoppingDistance, _StoppingDistanceToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_StartingHealthEdit, _HealthToDict)
	ON_LBN_SELCHANGE(IDC_MAPOBJECT_BuildWithUpgrades, _PrebuiltUpgradesToDict)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MapObjectProps message handlers

static AsciiString getNthKeyStr(const Dict* d, int i)
{
	NameKeyType k = d->getNthKey(i);
	AsciiString kstr = TheNameKeyGenerator->keyToName(k);
	return kstr;
}

static AsciiString getNthValueStr(const Dict* d, int i, Bool* enquote)
{
	*enquote = false;
	AsciiString vstr;
	switch(d->getNthType(i))
	{
		case Dict::DICT_BOOL:
			vstr.format("%s", d->getNthBool(i) ? "true" : "false");
			break;
		case Dict::DICT_INT:
			vstr.format("%d", d->getNthInt(i), d->getNthInt(i));
			break;
		case Dict::DICT_REAL:
			vstr.format("%f", d->getNthReal(i));
			break;
		case Dict::DICT_ASCIISTRING:
			vstr.format("%s", d->getNthAsciiString(i).str());
			*enquote = true;
			break;
		case Dict::DICT_UNICODESTRING:
			vstr.format("%ls", d->getNthUnicodeString(i).str());
			*enquote = true;
			break;
	}
	return vstr;
}

int MapObjectProps::getSel()
{
//	CListBox *list = (CListBox*)GetDlgItem(IDC_PROPERTIES);
//	int sel = list->GetCurSel();
//	if (!m_dictToEdit || sel == LB_ERR || sel < 0 || sel >= m_dictToEdit->getPairCount())
		return -1;
//	return sel;
}

void MapObjectProps::enableButtons()
{
	// do nothing
}

void MapObjectProps::OnSelchangeProperties() 
{
	
}

BOOL MapObjectProps::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_title)
		SetWindowText(m_title);

	m_heightSlider.SetupPopSliderButton(this, IDC_HEIGHT_POPUP, this);
	m_angleSlider.SetupPopSliderButton(this, IDC_ANGLE_POPUP, this);
	m_posUndoable = NULL;
	m_angle = 0;
	m_height = 0;

	updateTheUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*static*/ void MapObjectProps::update(void) 
{
	if (TheMapObjectProps) 
	{
		TheMapObjectProps->updateTheUI();
	}
}

void MapObjectProps::updateTheUI(void) 
{
	if (this != TheMapObjectProps) {
		return;
	}

	for (MapObject *pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (!pMapObj->isSelected() || pMapObj->isWaypoint() || pMapObj->isLight()) {
			continue;
		}

		m_dictToEdit = pMapObj ? pMapObj->getProperties() : NULL; 

		_DictToTeam();
		_DictToName();
		_DictToHealth();
		_DictToHPs();
		_DictToEnabled();
		_DictToScript();
		_DictToDestructible();
		_DictToPowered();
		_DictToAggressiveness();
		_DictToVisibilityRange();
		_DictToVeterancy();
		_DictToWeather();
		_DictToTime();
		_DictToShroudClearingDistance();
		_DictToRecruitableAI();
		_DictToSelectable();
		_DictToStoppingDistance();
		_DictToPrebuiltUpgrades();
		_DictToUnsellable();
		_DictToTargetable();
		ShowZOffset(pMapObj);
		ShowAngle(pMapObj);
		
		// simply break after the first one that's selected
		break;
	}
}

/*static*/ MapObject *MapObjectProps::getSingleSelectedMapObject(void)
{
	MapObject *pMapObj; 
	MapObject *theMapObj = NULL; 
//	Bool found = false;
	Int selCount=0;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isSelected()) {
			if (!pMapObj->isWaypoint() && !pMapObj->isLight()) 
			{
				theMapObj = pMapObj;
			}
			selCount++;
		}
	}
	if (selCount==1 && theMapObj) {
		return theMapObj;
	}
	return(NULL);
}

void MapObjectProps::OnEditprop() 
{
	int sel = getSel();
	if (sel == -1 || !m_dictToEdit)
		return;

	Bool enquote;
	AsciiString kstr = getNthKeyStr(m_dictToEdit, sel);
	AsciiString vstr = getNthValueStr(m_dictToEdit, sel, &enquote);
	Dict::DataType type = m_dictToEdit->getNthType(sel);

	PropEdit propDlg(&kstr, &type, &vstr, true, this);	
	if (propDlg.DoModal() == IDOK) 
	{
		Dict newDict = DictItemUndoable::buildSingleItemDict(kstr, type, vstr);
		if (this == TheMapObjectProps)
		{
			// it's the floating panel; use an undoable
			CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
			DictItemUndoable *pUndo = new DictItemUndoable(&m_dictToEdit, newDict, newDict.getNthKey(0));
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		} 
		else 
		{
			// we're running modal; just slam it in the dict
			m_dictToEdit->copyPairFrom(newDict, newDict.getNthKey(0));
		}
		updateTheUI();
	}
}

void MapObjectProps::OnNewprop() 
{
	if (!m_dictToEdit)
		return;

	// TODO: Add your control notification handler code here
	static Dict::DataType lastNewType = Dict::DICT_BOOL;
	AsciiString key, value;

	PropEdit propDlg(&key, &lastNewType, &value, false, this);	
	if (propDlg.DoModal() == IDOK) 
	{
		Dict newDict = DictItemUndoable::buildSingleItemDict(key, lastNewType, value);
		if (this == TheMapObjectProps)
		{
			// it's the floating panel; use an undoable
			CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
			DictItemUndoable *pUndo = new DictItemUndoable(&m_dictToEdit, newDict, newDict.getNthKey(0));
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		}
		else 
		{
			// we're running modal; just slam it in the dict
			m_dictToEdit->copyPairFrom(newDict, newDict.getNthKey(0));
		}
		updateTheUI();
	}
}

void MapObjectProps::OnRemoveprop() 
{
	int sel = getSel();
	if (sel == -1)
		return;
	
	NameKeyType k = m_dictToEdit->getNthKey(sel);
	if (this == TheMapObjectProps)
	{
			// it's the floating panel; use an undoable
		CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
		Dict newDict;	// empty dict
		DictItemUndoable *pUndo = new DictItemUndoable(&m_dictToEdit, newDict, k);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	}
	else
	{
			// we're running modal; just slam it in the dict
		m_dictToEdit->remove(k);
	}
	updateTheUI();
}

void MapObjectProps::OnDblclkProperties() 
{
	OnEditprop();
}

void MapObjectProps::_DictToTeam(void)
{
	int i;

	AsciiString name;
	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Team);
	owner->ResetContent();
	for (i = 0; i < TheSidesList->getNumTeams(); i++)
	{
		name = TheSidesList->getTeamInfo(i)->getDict()->getAsciiString(TheKey_teamName);
		if (name == NEUTRAL_TEAM_INTERNAL_STR)
			name = NEUTRAL_TEAM_UI_STR;
		owner->AddString(name.str());
	}
	// re-find, since the list is sorted
	i = -1;
	if (m_dictToEdit)
	{
		name = m_dictToEdit->getAsciiString(TheKey_originalOwner);
		if (name == NEUTRAL_TEAM_INTERNAL_STR)
			name = NEUTRAL_TEAM_UI_STR;
		i = owner->FindStringExact(-1, name.str());
		DEBUG_ASSERTLOG(i >= 0, ("missing team '%s'. Non-fatal (jkmcd)\n", name.str()));
	}
	owner->SetCurSel(i);
}

void MapObjectProps::_DictToName(void)
{
	AsciiString name = "";
	Bool exists;
	if (m_dictToEdit) {
		name = m_dictToEdit->getAsciiString(TheKey_objectName, &exists);
	}

	CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_Name);
	if (pItem) {
		pItem->SetWindowText(name.str());
	}
}

void MapObjectProps::_DictToHealth(void)
{
	Int value = 100;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_objectInitialHealth, &exists);
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
			sprintf(buff, "%d", value);
			pItem2->SetWindowText(buff);
			pItem2->EnableWindow(TRUE);
		}
	}
}

void MapObjectProps::_DictToHPs(void)
{
	Int value = -1;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_objectMaxHPs, &exists);
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

void MapObjectProps::_DictToEnabled(void)
{
	Bool enabled = true;
	Bool exists;
	if (m_dictToEdit) {
		enabled  = m_dictToEdit->getBool(TheKey_objectEnabled, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Enabled);
	if (pItem) {
		pItem->SetCheck(enabled);
	}	
}

void MapObjectProps::_DictToScript(void)
{
	if (!m_dictToEdit) {
		return;
	}
	
	Bool exists;
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Script);
	// Load the subroutine scripts into the combo box.
	EditParameter::loadScripts(pCombo, true);
	/*Int stringNdx =*/ pCombo->AddString("<none>");
	AsciiString script = m_dictToEdit->getAsciiString(TheKey_objectScriptAttachment, &exists);
	
	if (script.isEmpty()) {
		pCombo->SelectString(-1, "<none>");
	} else {
		pCombo->SelectString(-1, script.str());
	}
}

void MapObjectProps::_DictToDestructible(void)
{
	Bool destructible = true;
	Bool exists;
	if (m_dictToEdit) {
		destructible  = m_dictToEdit->getBool(TheKey_objectIndestructible, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Indestructible);
	if (pItem) {
		pItem->SetCheck(destructible);
	}	
}

void MapObjectProps::_DictToUnsellable(void)
{
	Bool unsellable = false;
	Bool exists;
	if (m_dictToEdit) {
		unsellable  = m_dictToEdit->getBool(TheKey_objectUnsellable, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Unsellable);
	if (pItem) {
		pItem->SetCheck(unsellable);
	}	
}

void MapObjectProps::_DictToTargetable()
{
	Bool targetable = false;
	Bool exists;
	if( m_dictToEdit ) 
	{
		targetable = m_dictToEdit->getBool( TheKey_objectTargetable, &exists );
	}

	CButton* pItem = (CButton*) GetDlgItem( IDC_MAPOBJECT_Targetable );
	if( pItem ) 
	{
		pItem->SetCheck( targetable );
	}	
}

void MapObjectProps::_DictToPowered(void)
{
	Bool powered = true;
	Bool exists;
	if (m_dictToEdit) {
		powered  = m_dictToEdit->getBool(TheKey_objectPowered, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Powered);
	if (pItem) {
		pItem->SetCheck(powered);
	}	
	
}

void MapObjectProps::_DictToAggressiveness(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_objectAggressiveness, &exists);
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

void MapObjectProps::_DictToVisibilityRange(void)
{
	Int distance = 0;
	Bool exists;
	if (m_dictToEdit) {
		distance = m_dictToEdit->getInt(TheKey_objectVisualRange, &exists);
	}

	CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_VisionDistance);
	if (pItem) {
		static char buff[12];
		sprintf(buff, "%d", distance);
		if (distance == 0) {
			pItem->SetWindowText(""); 
		} else {
			pItem->SetWindowText(buff);
		}
	}
}

void MapObjectProps::_DictToVeterancy(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_objectVeterancy, &exists);
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_Veterancy);
	if (pItem) {
		pItem->SetCurSel(value);
	}
}

void MapObjectProps::_DictToWeather(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_objectWeather, &exists);
		if (!exists)
			value = 0;
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_Weather);
	pItem->SetCurSel(value);
}

void MapObjectProps::_DictToTime(void)
{
	Int value = 0;
	Bool exists;
	if (m_dictToEdit) {
		value = m_dictToEdit->getInt(TheKey_objectTime, &exists);
		if (!exists)
			value = 0;
	}

	CComboBox* pItem = (CComboBox*) GetDlgItem(IDC_MAPOBJECT_Time);
	pItem->SetCurSel(value);
}

void MapObjectProps::_DictToShroudClearingDistance(void)
{
	Int distance = 0;
	Bool exists;
	if (m_dictToEdit) {
		distance = m_dictToEdit->getInt(TheKey_objectShroudClearingDistance, &exists);
	}

	CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_ShroudClearingDistance);
	if (pItem) {
		static char buff[12];
		sprintf(buff, "%d", distance);
		if (distance == 0) {
			pItem->SetWindowText(""); 
		} else {
			pItem->SetWindowText(buff);
		}
	}
}

void MapObjectProps::_DictToRecruitableAI(void)
{
 	Bool recruitableAI = true;
 	Bool exists;
 	if (m_dictToEdit) {
		recruitableAI  = m_dictToEdit->getBool(TheKey_objectRecruitableAI, &exists);
 	}
	
 	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_RecruitableAI);
 	if (pItem) {
		pItem->SetCheck(recruitableAI);
	}	
}

void MapObjectProps::_DictToSelectable(void)
{
	Bool selectable = true;
	Bool exists;
	if (m_dictToEdit) {
		selectable  = m_dictToEdit->getBool(TheKey_objectSelectable, &exists);
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Selectable);
	if (pItem) {
		pItem->SetCheck(selectable);
	}	
}

void MapObjectProps::_DictToStoppingDistance(void)
{
	Real stoppingDistance = 1.0f;
	Bool exists = false;
	if (m_dictToEdit) {
		stoppingDistance = m_dictToEdit->getReal(TheKey_objectStoppingDistance, &exists);
	}

	CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_StoppingDistance);
	if (pItem) {
		static char buff[12];
		sprintf(buff, "%g", stoppingDistance);
		pItem->SetWindowText(buff);
	}	
}

void MapObjectProps::_DictToPrebuiltUpgrades(void)
{
	getAllSelectedDicts();

	CListBox *pBox = (CListBox*) GetDlgItem(IDC_MAPOBJECT_BuildWithUpgrades);
	if (!pBox) {
		return;
	}

	// First, clear out the list
	pBox->ResetContent();
	CString cstr;

	// Then, if there's multiple units selected, add the Single Selection Only string
	if (m_allSelectedDicts.size() > 1) {
		cstr.LoadString(IDS_SINGLE_SELECTION_ONLY);
		pBox->AddString(cstr);
		return;
	}

	if (m_selectedObject == NULL) {
		return;
	}

	// Otherwise, fill it with the upgrades available for this unit
	const ThingTemplate *tt = m_selectedObject->getThingTemplate();
	if (tt == NULL) {
		// This is valid. For instance, Scorch marks do not have thing templates.
		return;
	}

	Bool noUpgrades = false;

	// Now do any behaviors that are also upgrades.
	const ModuleInfo& mi = tt->getBehaviorModuleInfo();
	if (mi.getCount() == 0) {
		if (noUpgrades) {
			cstr.LoadString(IDS_NO_UPGRADES);
			pBox->AddString(cstr);
			return;
		}
	} else {
		Int numBehaviorModules = mi.getCount();

		Int numBehaviorsWithUpgrades = 0;

		for (int i = 0; i < numBehaviorModules; ++i) {
			if (mi.getNthName(i).compareNoCase("GenerateMinefieldBehavior") == 0) {
				const GenerateMinefieldBehaviorModuleData *gmbmd = (const GenerateMinefieldBehaviorModuleData *)mi.getNthData(i);
				if (!gmbmd) {
					continue;
				}
				if (gmbmd->m_upgradeMuxData.m_activationUpgradeNames.size() > 0) {
					cstr = gmbmd->m_upgradeMuxData.m_activationUpgradeNames[0].str();
					if (pBox->FindString(-1, cstr) == LB_ERR) {
						pBox->AddString(cstr);
						++numBehaviorsWithUpgrades;
					}
				}
			}
		}

		if (numBehaviorsWithUpgrades == 0) {
			if (noUpgrades) {
				cstr.LoadString(IDS_NO_UPGRADES);
				pBox->AddString(cstr);
				return;
			}
		}
	}

	// Finally, walk through the upgrades that he already has, and select the appropriate members
	// from the list
	
	Bool exists;
	int upgradeNum = 0;
	AsciiString upgradeString;

	do {
		AsciiString keyName;
		keyName.format("%s%d", TheNameKeyGenerator->keyToName(TheKey_objectGrantUpgrade).str(), upgradeNum);
		upgradeString = m_dictToEdit->getAsciiString(NAMEKEY(keyName), &exists);

		if (exists) {
			Int selNdx = pBox->FindStringExact(-1, upgradeString.str());
			if (selNdx == LB_ERR) {
				DEBUG_CRASH(("Object claims '%s', but it wasn't found in the list of possible upgrades.", upgradeString.str()));
				++upgradeNum;
				continue;
			}
			pBox->SetSel(selNdx);

		} else {
			upgradeString.clear();
		}

		++upgradeNum;
	} while (!upgradeString.isEmpty());
}

void MapObjectProps::_TeamToDict(void)
{
	getAllSelectedDicts();

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Team);
	static char buf[1024];
	owner->GetWindowText(buf, sizeof(buf)-2);
	if (strcmp(buf, NEUTRAL_TEAM_UI_STR)==0)
		strcpy(buf, NEUTRAL_TEAM_INTERNAL_STR);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setAsciiString(TheKey_originalOwner, AsciiString(buf));
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_NameToDict(void)
{
	getAllSelectedDicts();

	CWnd *owner = GetDlgItem(IDC_MAPOBJECT_Name);
	CString cstr;
	owner->GetWindowText(cstr);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setAsciiString(TheKey_objectName, cstr.GetBuffer(0));
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_HealthToDict(void)
{
	getAllSelectedDicts();

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
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setInt(TheKey_objectInitialHealth, value);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_PrebuiltUpgradesToDict(void)
{
	getAllSelectedDicts();
	
	CListBox *pBox = (CListBox *) GetDlgItem(IDC_MAPOBJECT_BuildWithUpgrades);
	if (!pBox) {
		return;
	}

	// We only work for single selections
	if (m_allSelectedDicts.size() != 1) {
		return;
	}

	if (m_selectedObject)	{
		if ( !m_selectedObject->getThingTemplate() ) {
			return;
		}
	}
	
	Bool exists;
	int upgradeNum = 0;
	AsciiString upgradeString;

	// We're going to sub this entire dict for the existing entire dict.
	Dict newDict = *m_allSelectedDicts[0];

	// First, clear out any existing notions of what we should upgrade.
	do {
		AsciiString keyName;
		keyName.format("%s%d", TheNameKeyGenerator->keyToName(TheKey_objectGrantUpgrade).str(), upgradeNum);
		upgradeString = newDict.getAsciiString(NAMEKEY(keyName), &exists);

		if (exists) {
			newDict.remove(NAMEKEY(keyName));
		}

		++upgradeNum;
	} while (!upgradeString.isEmpty());

	upgradeNum = 0;
	// We've now removed them, so lets add the ones that are selected now.
	Int countOfItems = pBox->GetCount();
	for (Int i = 0; i < countOfItems; ++i) {
		if (pBox->GetSel(i) > 0) {
			CString selTxt;
			// This thing is selected. Get its text, and add it to the dict.
			pBox->GetText(i, selTxt);

			AsciiString keyName;
			keyName.format("%s%d", TheNameKeyGenerator->keyToName(TheKey_objectGrantUpgrade).str(), upgradeNum);
			newDict.setAsciiString(NAMEKEY(keyName), AsciiString(selTxt.GetBuffer(0)));
			++upgradeNum;
		}
	}

	// Now, do the Undoable
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, NAMEKEY_INVALID, m_allSelectedDicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

void MapObjectProps::_EnabledToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Enabled);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setBool(TheKey_objectEnabled, isChecked);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_ScriptToDict(void)
{
	getAllSelectedDicts();

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Script);
	static char buf[1024];
	owner->GetWindowText(buf, sizeof(buf)-2);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setAsciiString(TheKey_objectScriptAttachment, AsciiString(buf));
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do	
}

void MapObjectProps::_IndestructibleToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Indestructible);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setBool(TheKey_objectIndestructible, isChecked);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do	
}

void MapObjectProps::_UnsellableToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Unsellable);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setBool(TheKey_objectUnsellable, isChecked);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
	
}

void MapObjectProps::_TargetableToDict()
{
	getAllSelectedDicts();

	CButton *owner = (CButton*)GetDlgItem( IDC_MAPOBJECT_Targetable );
	Bool isChecked = owner->GetCheck() != 0;

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setBool( TheKey_objectTargetable, isChecked );
	DictItemUndoable *pUndo = new DictItemUndoable( getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size() );
	pDoc->AddAndDoUndoable( pUndo );
	REF_PTR_RELEASE( pUndo ); // belongs to pDoc now.
	// Update is called by Do
}


void MapObjectProps::_PoweredToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Powered);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setBool(TheKey_objectPowered, isChecked);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do

}

void MapObjectProps::_AggressivenessToDict(void)
{
	getAllSelectedDicts();

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

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setInt(TheKey_objectAggressiveness, value);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do		
}

void MapObjectProps::_VisibilityToDict(void)
{
	getAllSelectedDicts();

	int value = -1;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_VisionDistance);
	edit->EnableWindow(TRUE);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		value = atoi(cstr.GetBuffer(0));
	}

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	if (value != -1) {
		newDict.setInt(TheKey_objectVisualRange, value);
	}
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, TheKey_objectVisualRange, m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_VeterancyToDict(void)
{
	getAllSelectedDicts();

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Veterancy);
	static char buf[1024];
	int curSel = owner->GetCurSel();
	int value = 0;	
	if (curSel >= 0) {
		value=curSel;
	}

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setInt(TheKey_objectVeterancy, value);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do	

}

void MapObjectProps::_WeatherToDict(void)
{
	getAllSelectedDicts();

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Weather);
	static char buf[1024];
	int curSel = owner->GetCurSel();

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setInt(TheKey_objectWeather, curSel);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do	

}

void MapObjectProps::_TimeToDict(void)
{
	getAllSelectedDicts();

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Time);
	static char buf[1024];
	int curSel = owner->GetCurSel();

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setInt(TheKey_objectTime, curSel);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do	

}

void MapObjectProps::_ShroudClearingDistanceToDict(void)
{
	getAllSelectedDicts();
	
	int value = -1;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_ShroudClearingDistance);
	edit->EnableWindow(TRUE);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		value = atoi(cstr.GetBuffer(0));
	}

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	if (value != -1) {
		newDict.setInt(TheKey_objectShroudClearingDistance, value);
	}
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, TheKey_objectShroudClearingDistance, m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_RecruitableAIToDict(void)
{
	getAllSelectedDicts();
	
	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_RecruitableAI);
	Bool isChecked = (owner->GetCheck() != 0);
	
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setBool(TheKey_objectRecruitableAI, isChecked);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_SelectableToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Selectable);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setBool(TheKey_objectSelectable, isChecked);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::_HPsToDict() 
{
	getAllSelectedDicts();
	
	Int value;
	static char buf[1024];

	CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_HitPoints);
	owner->GetWindowText(buf, sizeof(buf)-2);
	value = atoi(buf);
	if (value == 0)
		value = -1;

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;

	newDict.setInt(TheKey_objectMaxHPs, value);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

void MapObjectProps::_StoppingDistanceToDict(void)
{
	getAllSelectedDicts();

	Real value;
	static char buf[1024];

	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_StoppingDistance);
	CString cstr;
	edit->GetWindowText(cstr);
	if (cstr.IsEmpty()) {
		return;
	}
	value = atof(cstr.GetBuffer(0));

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	Dict newDict;
	newDict.setReal(TheKey_objectStoppingDistance, value);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	// Update is called by Do
}

void MapObjectProps::OnOK()
{

}

void MapObjectProps::OnCancel()
{

}


void MapObjectProps::ShowZOffset(MapObject *pMapObj)
{
	const Coord3D *loc = pMapObj->getLocation();
	static char buff[12];
	m_height = loc->z;
	sprintf(buff, "%0.2f", loc->z);
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_ZOffset);
	edit->SetWindowText(buff);
}

void MapObjectProps::SetZOffset(void)
{
	Real value = 0.0f;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_ZOffset);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		value = atof(cstr);
	}
	m_height = value;
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	pUndo->SetZOffset(value);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

void MapObjectProps::ShowAngle(MapObject *pMapObj)
{
	m_angle = pMapObj->getAngle() * 180 / PI;
	static char buff[12];
	sprintf(buff, "%0.2f", m_angle);
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_Angle);
	edit->SetWindowText(buff);

}

void MapObjectProps::SetAngle(void)
{
	Real angle = 0.0f;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_Angle);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		angle = atof(cstr);
	}
	m_angle = angle;
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	pUndo->RotateTo(angle * PI/180);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

void MapObjectProps::getAllSelectedDicts(void)
{
	m_allSelectedDicts.clear();

	for (MapObject *pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {

		/* If a bridge exists, and either end is selected, do both ends. */
		if (pMapObj->getFlag(FLAG_BRIDGE_POINT1)) {
			MapObject *pMapObj2 = pMapObj->getNext();
			if (pMapObj2 && pMapObj2->getFlag(FLAG_BRIDGE_POINT2)) {
				// Got a bridge.
				if (pMapObj->isSelected() || pMapObj2->isSelected()) {
					m_allSelectedDicts.push_back(pMapObj->getProperties());
					m_allSelectedDicts.push_back(pMapObj2->getProperties());
					pMapObj = pMapObj2;
					continue;
				}
			}
		}


		if (!pMapObj->isSelected() || pMapObj->isWaypoint() || pMapObj->isLight()) {
			continue;
		}
		m_allSelectedDicts.push_back(pMapObj->getProperties());
		m_selectedObject = pMapObj;
	}
}

Dict** MapObjectProps::getAllSelectedDictsData()
{
#if defined(USING_STLPORT) || __cplusplus < 201103L
	return !m_allSelectedDicts.empty() ? &m_allSelectedDicts.front() : NULL;
#else
	return m_allSelectedDicts.data();
#endif
}

void MapObjectProps::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {

		case IDC_HEIGHT_POPUP:
			*pMin = -50;
			*pMax = 50;
			*pInitial = m_height;
			*pLineSize = 1;
			break;

		case IDC_ANGLE_POPUP:
			*pMin = 0;
			*pMax = 360;
			*pInitial = m_angle;
			*pLineSize = 1;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void MapObjectProps::PopSliderChanged(const long sliderID, long theVal)
{
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	CWnd* edit;
	static char buff[12];
	switch (sliderID) {
		case IDC_HEIGHT_POPUP:
			if (!m_posUndoable) {
				m_posUndoable = new ModifyObjectUndoable(pDoc);
				pDoc->AddAndDoUndoable(m_posUndoable);
			}
			m_posUndoable->SetZOffset(theVal);
			m_height = theVal;
			sprintf(buff, "%0.2f", m_height);
			edit = GetDlgItem(IDC_MAPOBJECT_ZOffset);
			edit->SetWindowText(buff);
			break;

		case IDC_ANGLE_POPUP:
			if (!m_posUndoable) {
				m_posUndoable = new ModifyObjectUndoable(pDoc);
				pDoc->AddAndDoUndoable(m_posUndoable);
			}
			m_posUndoable->RotateTo(theVal * PI/180);
			m_angle = theVal;
			sprintf(buff, "%0.2f", m_angle);
			edit = GetDlgItem(IDC_MAPOBJECT_Angle);
			edit->SetWindowText(buff);
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void MapObjectProps::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_HEIGHT_POPUP:
		case IDC_ANGLE_POPUP:
			REF_PTR_RELEASE(m_posUndoable); // belongs to pDoc now.
			m_posUndoable = NULL;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch

}

