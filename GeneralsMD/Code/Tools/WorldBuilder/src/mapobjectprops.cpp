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
#include "Common/AudioEventInfo.h"

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
	m_selectedObject(NULL),
  m_dictSource(NULL),
  m_scale( 1.0f ),
  m_height( 0 ),
  m_posUndoable( NULL ),
  m_angle( 0 ),
  m_defaultEntryIndex(0),
  m_defaultIsNone(true)
{


  //{{AFX_DATA_INIT(MapObjectProps)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

MapObjectProps::~MapObjectProps()
{
  if (TheMapObjectProps == this)
		TheMapObjectProps = NULL;

  if ( m_posUndoable != NULL )
  {
    REF_PTR_RELEASE( m_posUndoable );
  }

  if ( m_hWnd )
    DestroyWindow();
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
	ON_BN_CLICKED(IDC_CUSTOMIZE_CHECKBOX, customizeToDict)
	ON_BN_CLICKED(IDC_EDITPROP, OnEditprop)
	ON_BN_CLICKED(IDC_ENABLED_CHECKBOX, enabledToDict)
	ON_BN_CLICKED(IDC_LOOPING_CHECKBOX, loopingToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Enabled, _EnabledToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Indestructible, _IndestructibleToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Powered, _PoweredToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_RecruitableAI, _RecruitableAIToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Selectable, _SelectableToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Targetable, _TargetableToDict)
	ON_BN_CLICKED(IDC_MAPOBJECT_Unsellable, _UnsellableToDict)
	ON_BN_CLICKED(IDC_NEWPROP, OnNewprop)
	ON_BN_CLICKED(IDC_REMOVEPROP, OnRemoveprop)
	ON_BN_CLICKED(IDC_SCALE_OFF, OnScaleOff)
	ON_BN_CLICKED(IDC_SCALE_ON, OnScaleOn)
	ON_CBN_KILLFOCUS(IDC_MAPOBJECT_HitPoints, _HPsToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Aggressiveness, _AggressivenessToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Script, _ScriptToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_StartingHealth, _HealthToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Team, _TeamToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Time, _TimeToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Veterancy, _VeterancyToDict)
	ON_CBN_SELCHANGE(IDC_MAPOBJECT_Weather, _WeatherToDict)
	ON_CBN_SELCHANGE(IDC_PRIORITY_COMBO, priorityToDict)
	ON_CBN_SELCHANGE(IDC_SOUND_COMBO, attachedSoundToDict)
	ON_CBN_SELENDOK(IDC_MAPOBJECT_HitPoints, _HPsToDict)
	ON_EN_KILLFOCUS(IDC_LOOPCOUNT_EDIT, loopCountToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_Angle, SetAngle)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_Name, _NameToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_Scale, _ScaleToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_ShroudClearingDistance, _ShroudClearingDistanceToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_StartingHealthEdit, _HealthToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_StoppingDistance, _StoppingDistanceToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_VisionDistance, _VisibilityToDict)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_XYPosition, OnKillfocusMAPOBJECTXYPosition)
	ON_EN_KILLFOCUS(IDC_MAPOBJECT_ZOffset, SetZOffset)
	ON_EN_KILLFOCUS(IDC_MAX_RANGE_EDIT, maxRangeToDict)
	ON_EN_KILLFOCUS(IDC_MIN_RANGE_EDIT, minRangeToDict)
	ON_EN_KILLFOCUS(IDC_MIN_VOLUME_EDIT, minVolumeToDict)
	ON_EN_KILLFOCUS(IDC_VOLUME_EDIT, volumeToDict)
	ON_LBN_DBLCLK(IDC_PROPERTIES, OnDblclkProperties)
	ON_LBN_SELCHANGE(IDC_MAPOBJECT_BuildWithUpgrades, _PrebuiltUpgradesToDict)
	ON_LBN_SELCHANGE(IDC_PROPERTIES, OnSelchangeProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


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

/// Move data from object to dialog controls
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
		DEBUG_ASSERTLOG(i >= 0, ("missing team '%s'. Non-fatal (jkmcd)", name.str()));

  }
  owner->SetCurSel(i);
}

/// Move data from object to dialog controls
void MapObjectProps::_DictToName(void)
{
  AsciiString name = "";
  Bool exists;
  if (m_dictToEdit)
  {
    name = m_dictToEdit->getAsciiString(TheKey_objectName, &exists);
  }

  CWnd* pItem = GetDlgItem(IDC_MAPOBJECT_Name);
  if (pItem)
  {
    pItem->SetWindowText(name.str());
  }
}

/// Move data from object to dialog controls
void MapObjectProps::_DictToScript(void)
{
  if (!m_dictToEdit)
  {
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

/// Move data from dialog controls to object
void MapObjectProps::_TeamToDict(void)
{
  getAllSelectedDicts();

  CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Team);
  static char buf[1024];
  owner->GetWindowText(buf, sizeof(buf)-2);
  if (strcmp(buf, NEUTRAL_TEAM_UI_STR)==0)
    strcpy(buf, NEUTRAL_TEAM_INTERNAL_STR);

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc )
  {
    Dict newDict;
    newDict.setAsciiString(TheKey_originalOwner, AsciiString(buf));
    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_NameToDict(void)
{
  getAllSelectedDicts();

  CWnd *owner = GetDlgItem(IDC_MAPOBJECT_Name);
  CString cstr;
  owner->GetWindowText(cstr);

  // Note: when starting up, this is called before a document exists. Just ignore such
  // calls, since there can't be an object to affect.
  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc )
  {
    Dict newDict;
    newDict.setAsciiString(TheKey_objectName, cstr.GetBuffer(0));
    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_ScriptToDict(void)
{
  getAllSelectedDicts();

  CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Script);
  static char buf[1024];
  owner->GetWindowText(buf, sizeof(buf)-2);

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc )
  {
    Dict newDict;
    newDict.setAsciiString(TheKey_objectScriptAttachment, AsciiString(buf));
    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Do
  }
}


/// Move data from object to dialog controls
void MapObjectProps::_DictToScale(void)
{
// Currently not in the Mission Disk.
#if 0
  m_scale = 1;
  Bool exists;
  if (m_dictToEdit) {
    m_scale = m_dictToEdit->getReal(TheKey_objectPrototypeScale, &exists);
  }
  if (!exists) {
    m_scale = 1;
  }

  CWnd* edit = GetDlgItem(IDC_MAPOBJECT_Scale);
  CString cstr;
  cstr.Format("%.2f", m_scale);
  edit->SetWindowText(cstr);

  CButton *off = (CButton*) GetDlgItem(IDC_SCALE_OFF);
  CButton *on = (CButton*) GetDlgItem(IDC_SCALE_ON);
  if (!exists) {
    off->SetCheck(1);
    on->SetCheck(0);
    edit->EnableWindow(false);
    GetDlgItem(IDC_SCALE_POPUP)->EnableWindow(false);
  } else {
    off->SetCheck(0);
    on->SetCheck(1);
    edit->EnableWindow(TRUE);
    GetDlgItem(IDC_SCALE_POPUP)->EnableWindow(true);
  }
#endif

}

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from dialog controls to object
void MapObjectProps::_WeatherToDict(void)
{
  getAllSelectedDicts();

  CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Weather);
  static char buf[1024];
  int curSel = owner->GetCurSel();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc )
  {
    Dict newDict;
    newDict.setInt(TheKey_objectWeather, curSel);
    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_TimeToDict(void)
{
  getAllSelectedDicts();

  CComboBox *owner = (CComboBox*)GetDlgItem(IDC_MAPOBJECT_Time);
  static char buf[1024];
  int curSel = owner->GetCurSel();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc )
  {
    Dict newDict;
    newDict.setInt(TheKey_objectTime, curSel);
    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_ScaleToDict(void)
{
// Currently not in mission disk.
#if 0
  getAllSelectedDicts();

  Real value = 0.0f;
  CWnd* edit = GetDlgItem(IDC_MAPOBJECT_Scale);
  CString cstr;
  edit->GetWindowText(cstr);
  if (!cstr.IsEmpty()) {
    value = atof(cstr);
  }
  m_scale = value;

  CButton *off = (CButton*) GetDlgItem(IDC_SCALE_OFF);

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc )
  {
    Dict newDict;
    if (off->GetCheck() == 1) {
      newDict.remove(TheKey_objectPrototypeScale);
    } else {
      newDict.setReal(TheKey_objectPrototypeScale, m_scale);
    }
    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Do
  }
#endif
}

/// Move data from object to dialog controls
void MapObjectProps::ShowZOffset(MapObject *pMapObj)
{
  const Coord3D *loc = pMapObj->getLocation();
  static char buff[12];
  m_height = loc->z;
  sprintf(buff, "%0.2f", loc->z);
  CWnd* edit = GetDlgItem(IDC_MAPOBJECT_ZOffset);
  edit->SetWindowText(buff);
}

/// Move data from dialog controls to object
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
  if ( pDoc )
  {
    ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
    pDoc->AddAndDoUndoable(pUndo);
    pUndo->SetZOffset(value);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
  }
}

/// Move data from object to dialog controls
void MapObjectProps::ShowAngle(MapObject *pMapObj)
{
  m_angle = pMapObj->getAngle() * 180 / PI;
  static char buff[12];
  sprintf(buff, "%0.2f", m_angle);
  CWnd* edit = GetDlgItem(IDC_MAPOBJECT_Angle);
  edit->SetWindowText(buff);
  m_angle = atof(buff);
}

/// Move data from object to dialog controls
void MapObjectProps::ShowPosition(MapObject *pMapObj)
{
  m_position = *pMapObj->getLocation();
  static char buff[12];
  sprintf(buff, "%0.2f, %0.2f", m_position.x, m_position.y);
  CWnd* edit = GetDlgItem(IDC_MAPOBJECT_XYPosition);
  edit->SetWindowText(buff);
  sscanf(buff, "%f,%f", &m_position.x, &m_position.y);
}

/// Move data from dialog controls to object
void MapObjectProps::SetAngle(void)
{
  Real angle = m_angle;
  CWnd* edit = GetDlgItem(IDC_MAPOBJECT_Angle);
  CString cstr;
  edit->GetWindowText(cstr);
  if (!cstr.IsEmpty()) {
    angle = atof(cstr);
  }
  if (m_selectedObject==NULL) return;
  if (m_angle!=angle)
  {
    m_angle = angle;
    CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
    if ( pDoc )
    {
      ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
      pDoc->AddAndDoUndoable(pUndo);
      pUndo->RotateTo(angle * PI/180);
      REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    }
  }
}

/// Move data from dialog controls to object
void MapObjectProps::SetPosition(void)
{
  CWnd* edit = GetDlgItem(IDC_MAPOBJECT_XYPosition);
  CString cstr;
  edit->GetWindowText(cstr);
  Coord3D loc;
  loc = m_position;
  if (m_selectedObject==NULL) return;
  if (!cstr.IsEmpty()) {
    if (sscanf(cstr, "%f, %f", &loc.x, &loc.y)!=2)
      loc = m_position;
  }
  if (loc.x!=m_position.x || loc.y != m_position.y) {
    m_position = loc;
    CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
    if ( pDoc )
    {
      ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
      pDoc->AddAndDoUndoable(pUndo);
      if (m_selectedObject) {
        loc = *m_selectedObject->getLocation();
      }
      pUndo->SetOffset(m_position.x-loc.x, m_position.y-loc.y);
      REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    }
  }
}

/// Slider control
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

		case IDC_SCALE_POPUP:
			*pMin = 1;
			*pMax = 500;
			*pInitial = m_scale*100;
			*pLineSize = 1;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

/// Slider control
void MapObjectProps::PopSliderChanged(const long sliderID, long theVal)
{
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc == NULL )
    return;

	CWnd* edit;
	static char buff[36];
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

		case IDC_SCALE_POPUP:
			m_scale = theVal/100.0f;
			sprintf(buff, "%0.2f", m_scale);
			edit = GetDlgItem(IDC_MAPOBJECT_Scale);
			edit->SetWindowText(buff);
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

/// Slider control
void MapObjectProps::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_HEIGHT_POPUP:
		case IDC_ANGLE_POPUP:
      if ( m_posUndoable != NULL )
      {
  			REF_PTR_RELEASE(m_posUndoable); // belongs to pDoc now.
      }
			m_posUndoable = NULL;
			break;

		case IDC_SCALE_POPUP:
			_ScaleToDict();
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch

}

/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, NAMEKEY_INVALID, m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
  }
}

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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


/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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


/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
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

/// Move data from object to dialog controls
void MapObjectProps::_DictToSelectable(void)
{
	Int selectable = true;
	Bool exists=false;
	if (m_dictToEdit) {
		selectable = m_dictToEdit->getBool(TheKey_objectSelectable, &exists);
	}
	if (!exists) {
		selectable = 2;
	}

	CButton* pItem = (CButton*) GetDlgItem(IDC_MAPOBJECT_Selectable);
	if (pItem) {
		pItem->SetCheck(selectable);
	}
}

/// Move data from object to dialog controls
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


/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setInt(TheKey_objectInitialHealth, value);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}


/// Move data from dialog controls to object
void MapObjectProps::_EnabledToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Enabled);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setBool(TheKey_objectEnabled, isChecked);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_IndestructibleToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Indestructible);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setBool(TheKey_objectIndestructible, isChecked);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_UnsellableToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Unsellable);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setBool(TheKey_objectUnsellable, isChecked);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_TargetableToDict()
{
	getAllSelectedDicts();

	CButton *owner = (CButton*)GetDlgItem( IDC_MAPOBJECT_Targetable );
	Bool isChecked = owner->GetCheck() != 0;

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setBool( TheKey_objectTargetable, isChecked );
	  DictItemUndoable *pUndo = new DictItemUndoable( getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size() );
	  pDoc->AddAndDoUndoable( pUndo );
	  REF_PTR_RELEASE( pUndo ); // belongs to pDoc now.
	  // Update is called by Do
  }
}


/// Move data from dialog controls to object
void MapObjectProps::_PoweredToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Powered);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setBool(TheKey_objectPowered, isChecked);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setInt(TheKey_objectAggressiveness, value);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  if (value != -1) {
		  newDict.setInt(TheKey_objectVisualRange, value);
	  }
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, TheKey_objectVisualRange, m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setInt(TheKey_objectVeterancy, value);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  if (value != -1) {
		  newDict.setInt(TheKey_objectShroudClearingDistance, value);
	  }
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, TheKey_objectShroudClearingDistance, m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_RecruitableAIToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_RecruitableAI);
	Bool isChecked = (owner->GetCheck() != 0);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setBool(TheKey_objectRecruitableAI, isChecked);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
void MapObjectProps::_SelectableToDict(void)
{
	getAllSelectedDicts();

	CButton *owner = (CButton*) GetDlgItem(IDC_MAPOBJECT_Selectable);
	Bool isChecked = (owner->GetCheck() == 1);
	Bool isTristate = (owner->GetCheck() == 2);

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  if (isTristate) {
		  newDict.remove(TheKey_objectSelectable);
	  } else {
		  newDict.setBool(TheKey_objectSelectable, isChecked);
	  }
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Do
  }
}

/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
    Dict newDict;

	  newDict.setInt(TheKey_objectMaxHPs, value);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
  }
}

/// Move data from dialog controls to object
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
  if ( pDoc != NULL )
  {
	  Dict newDict;
	  newDict.setReal(TheKey_objectStoppingDistance, value);
	  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
	  pDoc->AddAndDoUndoable(pUndo);
	  REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	  // Update is called by Doc
  }
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


static const Char NO_SOUND_STRING[] = "(None)";
static const Char BASE_DEFAULT_STRING[] = "Default";

void MapObjectProps::OnSelchangeProperties()
{

}

/// Initialize dialog, especially drop down lists
BOOL MapObjectProps::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_title)
		SetWindowText(m_title);

  m_heightSlider.SetupPopSliderButton(this, IDC_HEIGHT_POPUP, this);
  m_angleSlider.SetupPopSliderButton(this, IDC_ANGLE_POPUP, this);
  m_scaleSlider.SetupPopSliderButton(this, IDC_SCALE_POPUP, this);
	m_posUndoable = NULL;
	m_angle = 0;
	m_height = 0;
	m_scale = 1.0f;

	InitSound();
	updateTheUI();

  return TRUE;
}

//---------------------------------------------------------------------------------------------------
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

    m_dictSource = pMapObj;
    // Select correct dictionary
		m_dictToEdit = m_dictSource ? m_dictSource->getProperties() : NULL;

		updateTheUI(m_dictSource);

		// simply break after the first one that's selected
		break;
	}
}

/// Move *all* data from object to dialog controls
void MapObjectProps::updateTheUI(MapObject *pMapObj)
{
  _DictToName();
  _DictToTeam();
  _DictToScript();
  _DictToWeather();
  _DictToTime();
  _DictToScale();
  _DictToPrebuiltUpgrades();
	_DictToHealth();
  _DictToHPs();
  _DictToEnabled();
  _DictToDestructible();
  _DictToPowered();
  _DictToAggressiveness();
  _DictToVisibilityRange();
  _DictToVeterancy();
  _DictToShroudClearingDistance();
  _DictToRecruitableAI();
  _DictToSelectable();
  _DictToStoppingDistance();
  _DictToUnsellable();
  _DictToTargetable();
	ShowZOffset(pMapObj);
  ShowAngle(pMapObj);
  ShowPosition(pMapObj);

  // Warning: order is important. dictToAttachedSound() must come before dictToCustomize(),
  // dictToCustomize() must come before any of the customization controls, dictToLooping()
  // must come before dictToLoopCount(), and dictToLooping() and dictToLoopCount() must
  // come before dictToEnabled().
  dictToAttachedSound();
  dictToCustomize();
  dictToLooping();
  dictToLoopCount();
  dictToEnabled();
  dictToMinVolume();
  dictToVolume();
  dictToMinRange();
  dictToMaxRange();
  dictToPriority();
}

//---------------------------------------------------------------------------------------------------

void MapObjectProps::InitSound(void)
{
  CComboBox * priorityComboBox = (CComboBox *)GetDlgItem(IDC_PRIORITY_COMBO);
  DEBUG_ASSERTCRASH( priorityComboBox != NULL, ("Cannot find sound priority combobox" ) );

  if ( priorityComboBox != NULL )
  {
    Int i;
    for ( i = 0; i <= AP_CRITICAL; i++ )
    {
      Int index = priorityComboBox->InsertString( i,theAudioPriorityNames[i] );
      (void)index;
      DEBUG_ASSERTCRASH( index == i, ("insert string returned %d, expected %d", index, i ) );
    }
  }

  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  DEBUG_ASSERTCRASH( soundComboBox != NULL, ("Cannot find sound combobox" ) );
  m_defaultEntryIndex = 0;
  m_defaultIsNone = true;

  // Load up combobox
  if ( soundComboBox != NULL )
  {
    // Add all the sound names in order. Since the combobox has the SORTED style,
    // we can just add the strings in and let the combo box sort them
    const AudioEventInfoHash & audioEventHash = TheAudio->getAllAudioEvents();

    AudioEventInfoHash::const_iterator it;
    for ( it = audioEventHash.begin(); it != audioEventHash.end(); it++ )
    {
      if ( it->second->m_soundType == AT_SoundEffect )
      {
        // Hmm, should we be filtering the list in any other way? Oh well.
        soundComboBox->AddString( it->second->m_audioName.str() );
      }
    }

    m_defaultEntryIndex = soundComboBox->InsertString( 0, BASE_DEFAULT_STRING );
    m_defaultEntryName = NO_SOUND_STRING;
    m_defaultIsNone = true;

    soundComboBox->InsertString( 1, NO_SOUND_STRING );
  }

} // end InitSound


// Adds a series of Undoable's to the given MultipleUndoable which clears the
// objectSoundAmbientCustomized flag and all the customization flags out of
// all the selected dictionaries.
// NOTE: assumes getAllSelectedDicts() has already been called
void MapObjectProps::clearCustomizeFlag( CWorldBuilderDoc* pDoc, MultipleUndoable * ownerUndoable )
{
  Dict empty;

  DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientCustomized, m_allSelectedDicts.size(), pDoc, true);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.

  // Note: we can set all the undoes in between the first and last undo to not invalidate,
  // since the first (last for doing) and last invalidate after these changes anyway
  pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientMaxRange, m_allSelectedDicts.size(), pDoc, false);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.

  pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientMinVolume, m_allSelectedDicts.size(), pDoc, false);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.

  pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientVolume, m_allSelectedDicts.size(), pDoc, false);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.

  pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientMinRange, m_allSelectedDicts.size(), pDoc, false);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.

  pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientLooping, m_allSelectedDicts.size(), pDoc, false);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.

  pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientLoopCount, m_allSelectedDicts.size(), pDoc, false);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.

  pUndo = new DictItemUndoable(getAllSelectedDictsData(), empty, TheKey_objectSoundAmbientPriority, m_allSelectedDicts.size(), pDoc, true);
  ownerUndoable->addUndoable(pUndo);
  REF_PTR_RELEASE(pUndo); // belongs to ownerUndoable now.
}

/// Move data from dialog controls to object(s)
void MapObjectProps::attachedSoundToDict(void)
{
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    MultipleUndoable *pUndo = new MultipleUndoable;

    Dict newDict;

    // For the default, erase the objectSoundAmbient key
    if ( soundComboBox->GetCurSel() != m_defaultEntryIndex )
    {
      CString selectionText;
      soundComboBox->GetLBText( soundComboBox->GetCurSel(), selectionText );
      if ( selectionText == NO_SOUND_STRING )
      {
        newDict.setAsciiString(TheKey_objectSoundAmbient, "");
        // Can't customize the null sound
        clearCustomizeFlag( pDoc, pUndo );
      }
      else
      {
        newDict.setAsciiString(TheKey_objectSoundAmbient, static_cast< const char * >( selectionText) );
      }
    }
    else if ( m_defaultIsNone )
    {
      // Can't customize the null sound
      clearCustomizeFlag( pDoc, pUndo );
    }

    DictItemUndoable *pDictUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, TheKey_objectSoundAmbient, m_allSelectedDicts.size(), pDoc, true);
    pUndo->addUndoable( pDictUndo );
    pDoc->AddAndDoUndoable( pUndo );
    REF_PTR_RELEASE( pDictUndo ); // belongs to pUndo
    REF_PTR_RELEASE( pUndo ); // belongs to pDoc now.
    // Update is called by Doc
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::customizeToDict(void)
{
  CButton * customizeCheckbox = (CButton *)GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    // For space reasons, erase key instead of added "false" key. Should be treated the same
    if ( customizeCheckbox->GetCheck() == 1 )
    {
      newDict.setBool( TheKey_objectSoundAmbientCustomized, true );
      DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, TheKey_objectSoundAmbientCustomized, m_allSelectedDicts.size(), pDoc, true);
      pDoc->AddAndDoUndoable(pUndo);
      REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
      // Update is called by Doc
    }
    else
    {
      MultipleUndoable *pUndo = new MultipleUndoable;
      clearCustomizeFlag( pDoc, pUndo );
      pDoc->AddAndDoUndoable( pUndo );
      REF_PTR_RELEASE( pUndo ); // belongs to pDoc now.
      // Update is called by Doc
    }
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::enabledToDict(void)
{
  CButton * enabledCheckbox = (CButton *)GetDlgItem(IDC_ENABLED_CHECKBOX);
  if ( enabledCheckbox == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    newDict.setBool( TheKey_objectSoundAmbientEnabled, enabledCheckbox->GetCheck() != 0 );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::loopingToDict(void)
{
  CButton * loopingCheckbox = (CButton *)GetDlgItem(IDC_LOOPING_CHECKBOX);
  if ( loopingCheckbox == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    newDict.setBool( TheKey_objectSoundAmbientLooping, loopingCheckbox->GetCheck() != 0 );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::loopCountToDict(void)
{
  CEdit * loopCountEdit = (CEdit *)GetDlgItem(IDC_LOOPCOUNT_EDIT);
  if ( loopCountEdit == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    CString loopCountString;
    loopCountEdit->GetWindowText(loopCountString);

    newDict.setInt( TheKey_objectSoundAmbientLoopCount, atoi( loopCountString ) );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::minVolumeToDict(void)
{
  CEdit * minVolumeEdit = (CEdit *)GetDlgItem(IDC_MIN_VOLUME_EDIT);
  if ( minVolumeEdit == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    CString minVolumeString;
    minVolumeEdit->GetWindowText(minVolumeString);

    // Note: min volume is stored as Real between 0.0 and 1.0, but displayed as a percentage from 0 to 100
    newDict.setReal( TheKey_objectSoundAmbientMinVolume, INT_TO_REAL( atoi( minVolumeString ) ) / 100.0f );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::volumeToDict(void)
{
  CEdit * volumeEdit = (CEdit *)GetDlgItem(IDC_VOLUME_EDIT);
  if ( volumeEdit == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    CString volumeString;
    volumeEdit->GetWindowText(volumeString);

    // Note: volume is stored as Real between 0.0 and 1.0, but displayed as a percentage from 0 to 100
    newDict.setReal( TheKey_objectSoundAmbientVolume, INT_TO_REAL( atoi( volumeString ) ) / 100.0f );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}


/// Move data from dialog controls to object(s)
void MapObjectProps::minRangeToDict(void)
{
  CEdit * minRangeEdit = (CEdit *)GetDlgItem(IDC_MIN_RANGE_EDIT);
  if ( minRangeEdit == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    CString minRangeString;
    minRangeEdit->GetWindowText(minRangeString);

    newDict.setReal( TheKey_objectSoundAmbientMinRange, INT_TO_REAL( atoi( minRangeString ) ) );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::maxRangeToDict(void)
{
  CEdit * maxRangeEdit = (CEdit *)GetDlgItem(IDC_MAX_RANGE_EDIT);
  if ( maxRangeEdit == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    CString maxRangeString;
    maxRangeEdit->GetWindowText(maxRangeString);

    newDict.setReal( TheKey_objectSoundAmbientMaxRange, INT_TO_REAL( atoi( maxRangeString ) ) );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}

/// Move data from dialog controls to object(s)
void MapObjectProps::priorityToDict(void)
{
  CComboBox * priorityComboBox = (CComboBox *)GetDlgItem(IDC_PRIORITY_COMBO);
  if ( priorityComboBox == NULL )
    return;

  getAllSelectedDicts();

  CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
  if ( pDoc != NULL )
  {
    Dict newDict;

    newDict.setInt( TheKey_objectSoundAmbientPriority, priorityComboBox->GetCurSel() );

    DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size(), pDoc, true);
    pDoc->AddAndDoUndoable(pUndo);
    REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
    // Update is called by Doc
  }
}


/// Move data from object to dialog controls
void MapObjectProps::dictToAttachedSound()
{
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
    return;

  // Update the string for the "default" entry
  m_defaultIsNone = true;
  m_defaultEntryName = NO_SOUND_STRING;
  soundComboBox->DeleteString(m_defaultEntryIndex);
  const ThingTemplate * thingTemplate = NULL;
  if (m_dictSource)
  {
    thingTemplate = m_dictSource->getThingTemplate();
  }

  if ( thingTemplate )
  {
    AsciiString string( BASE_DEFAULT_STRING );

    const AudioEventRTS * defaultAudioEvent;

    // Note: getSoundAmbient will return a non-NULL pointer even if there is no real sound attached to the object
    if ( thingTemplate->hasSoundAmbient() )
    {
      defaultAudioEvent = thingTemplate->getSoundAmbient();
    }
    else
    {
      defaultAudioEvent = NULL;
    }

    if ( defaultAudioEvent == NULL || defaultAudioEvent == TheAudio->getValidSilentAudioEvent() )
    {
      string.concat( " <None>" );
    }
    else
    {
      string.concat( " <" );
      string.concat( defaultAudioEvent->getEventName() );
      string.concat( '>' );
      m_defaultEntryName = defaultAudioEvent->getEventName();
      m_defaultIsNone = false;
    }
    m_defaultEntryIndex = soundComboBox->InsertString(0, string.str());
  }
  else
  {
    m_defaultEntryIndex = soundComboBox->InsertString(0, BASE_DEFAULT_STRING);
  }

  // Now select the correct entry in the list box
  AsciiString sound;
  Bool exists = false;
  if (m_dictToEdit)
  {
    sound = m_dictToEdit->getAsciiString(TheKey_objectSoundAmbient, &exists);
  }

  if ( !exists )
  {
    // Use the "default" entry
    soundComboBox->SetCurSel( m_defaultEntryIndex );
    return;
  }

  if ( sound.isEmpty() )
  {
    // Use the entry "(None)"
    sound = NO_SOUND_STRING;
  }

  // Note: better than SelectString because we want the exact string not a prefix
  Int index = soundComboBox->FindStringExact( -1, sound.str() );
  if ( index > -1 )
  {
    soundComboBox->SetCurSel( index );
  }
  else
  {
    DEBUG_CRASH( ("Could not find existing sound's name %s in combo box", sound.str() ) );
    soundComboBox->SetCurSel( m_defaultEntryIndex );
  }
}

/// Move data from object to dialog controls
void MapObjectProps::dictToCustomize()
{
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL )
    return;

  // If the current sound is "none", disable the customize button
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
    return;

  int index = soundComboBox->GetCurSel();
  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING ||
       ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    customizeCheckbox->SetCheck( 0 );
    customizeCheckbox->EnableWindow( false );
    return;
  }

  // Sound is not "none", so we can customize if we wish
  customizeCheckbox->EnableWindow( true );

  Bool exists = false;
  Bool customized = false;
  if (m_dictToEdit)
  {
    customized = m_dictToEdit->getBool(TheKey_objectSoundAmbientCustomized, &exists);
  }

  customizeCheckbox->SetCheck( exists && customized ? 1 : 0 );
}


/// Move data from object to dialog controls
void MapObjectProps::dictToLooping()
{
  CButton * loopingCheckbox = ( CButton * )GetDlgItem(IDC_LOOPING_CHECKBOX);
  if ( loopingCheckbox == NULL )
    return;

  // If the customized checkbox is off, all customization controls are disabled
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL || customizeCheckbox->GetCheck() == 0 )
  {
    loopingCheckbox->EnableWindow( false );
  }
  else
  {
    loopingCheckbox->EnableWindow( true );

    // If customization is enabled, look to see if the looping flag has already been
    // customized
    Bool exists = false;
    Bool looping = false;
    if (m_dictToEdit)
    {
      looping = m_dictToEdit->getBool(TheKey_objectSoundAmbientLooping, &exists);
    }

    if ( exists )
    {
      loopingCheckbox->SetCheck( looping ? 1 : 0 );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the looping checkbox

  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
  {
    loopingCheckbox->SetCheck( 0 );
    return;
  }
  Int index = soundComboBox->GetCurSel();

  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING || ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    loopingCheckbox->SetCheck( 0 );
    return;
  }

  if ( index == m_defaultEntryIndex )
  {
    // Correct the current string e.g. remove "Default <" and ">"
    currentString = m_defaultEntryName.str();
  }

  AudioEventInfo * audioEventInfo = TheAudio->findAudioEventInfo(static_cast< const char * >( currentString ) );

  loopingCheckbox->SetCheck( ( audioEventInfo && ( audioEventInfo->m_control & AC_LOOP ) ) ? 1 : 0 );
}

/// Move data from object to dialog controls
void MapObjectProps::dictToLoopCount()
{
  CEdit * loopCountEdit = ( CEdit * )GetDlgItem(IDC_LOOPCOUNT_EDIT);
  if ( loopCountEdit == NULL )
    return;

  // If the customized checkbox is off, all customization controls are disabled
  // If the looping checkbox is off, the loop count control is disabled
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  CButton * loopingCheckbox = ( CButton * )GetDlgItem(IDC_LOOPING_CHECKBOX);
  if ( customizeCheckbox == NULL || customizeCheckbox->GetCheck() == 0 ||
       loopingCheckbox == NULL || loopingCheckbox->GetCheck() == 0 )
  {
    loopCountEdit->EnableWindow( false );
  }
  else
  {
    loopCountEdit->EnableWindow( true );

    // If customization is enabled, look to see if the loop count has already been
    // customized
    Bool exists = false;
    Int loopCount = 0;
    if (m_dictToEdit)
    {
      loopCount = m_dictToEdit->getInt(TheKey_objectSoundAmbientLoopCount, &exists);
    }

    if ( exists )
    {
      CString loopCountText;
      loopCountText.Format( "%d", loopCount );
      loopCountEdit->SetWindowText( loopCountText );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the loop count

  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
  {
    loopCountEdit->SetWindowText( "0" );
    return;
  }

  Int index = soundComboBox->GetCurSel();

  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING || ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    loopCountEdit->SetWindowText( "0" );
    return;
  }

  if ( index == m_defaultEntryIndex )
  {
    // Correct the current string e.g. remove "Default <" and ">"
    currentString = m_defaultEntryName.str();
  }

  AudioEventInfo * audioEventInfo = TheAudio->findAudioEventInfo(static_cast< const char * >( currentString ) );

  if ( audioEventInfo == NULL )
  {
    loopCountEdit->SetWindowText( "0" );
    return;
  }

  CString loopCountText;
  loopCountText.Format( "%d", audioEventInfo->m_loopCount );
  loopCountEdit->SetWindowText( loopCountText );
}

/// Move data from object to dialog controls
void MapObjectProps::dictToEnabled()
{
  CButton * enableCheckbox = ( CButton * )GetDlgItem(IDC_ENABLED_CHECKBOX);
  if ( enableCheckbox == NULL )
    return;

  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);

  // If we don't have a sound, we can't enable it
  CString currentString;
  if ( soundComboBox == NULL )
  {
    enableCheckbox->EnableWindow( false );
    enableCheckbox->SetCheck( 0 );
    return;
  }

  Int soundIndex = soundComboBox->GetCurSel();
  soundComboBox->GetLBText( soundIndex, currentString );
  if ( currentString == NO_SOUND_STRING ||
      ( soundIndex == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    enableCheckbox->SetCheck( 0 );
    enableCheckbox->EnableWindow( false );
    return;
  }
  else
  {
    enableCheckbox->EnableWindow( true );

    // Look to see if the enabled flag has already been customized
    Bool exists = false;
    Bool enabled = false;
    if (m_dictToEdit)
    {
      enabled = m_dictToEdit->getBool(TheKey_objectSoundAmbientEnabled, &exists);
    }

    if ( exists )
    {
      enableCheckbox->SetCheck( enabled ? 1 : 0 );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the enabled checkbox

  // Look up sound and see if it's looping. Looping-forever sounds start enabled;
  // non-looping ones don't.
  // Actually, by now, dictToLooping and dictToLoopCount will have looked up the
  // info we need for us
  // NOTE: This test should match the tests done in Object::updateObjValuesFromMapProperties()
  // when it decided whether or not to enable a customized sound
  CButton * loopingCheckbox = (CButton *)GetDlgItem(IDC_LOOPING_CHECKBOX);
  CEdit * loopCountEdit = (CEdit *)GetDlgItem(IDC_LOOPCOUNT_EDIT);
  if ( loopingCheckbox && loopCountEdit && loopingCheckbox->GetCheck() == 1 )
  {
    CString loopCount;
    loopCountEdit->GetWindowText( loopCount );

    if ( atoi(loopCount) == 0 )
    {
      enableCheckbox->SetCheck( 1 );
      return;
    }
  }

  enableCheckbox->SetCheck( 0 );
}


/// Move data from object to dialog controls
void MapObjectProps::dictToMinVolume()
{
  CEdit * minVolumeEdit = ( CEdit * )GetDlgItem(IDC_MIN_VOLUME_EDIT);
  if ( minVolumeEdit == NULL )
    return;

  // If the customized checkbox is off, all customization controls are disabled
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL || customizeCheckbox->GetCheck() == 0 )
  {
    minVolumeEdit->EnableWindow( false );
  }
  else
  {
    minVolumeEdit->EnableWindow( true );

    // If customization is enabled, look to see if the minimum volume has already been
    // customized
    Bool exists = false;
    Real minVolume = 0.0f;
    if (m_dictToEdit)
    {
      minVolume = m_dictToEdit->getReal(TheKey_objectSoundAmbientMinVolume, &exists);
    }

    if ( exists )
    {
      // Note: min volume is stored as Real between 0.0 and 1.0, but displayed as a percentage from 0 to 100
      CString minVolumeText;
      minVolumeText.Format( "%d", REAL_TO_INT( ( minVolume * 100.0f ) + 0.5 ) );
      minVolumeEdit->SetWindowText( minVolumeText );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the minimum volume
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
  {
    minVolumeEdit->SetWindowText( "40" );
    return;
  }

  Int index = soundComboBox->GetCurSel();

  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING || ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    minVolumeEdit->SetWindowText( "40" );
    return;
  }

  if ( index == m_defaultEntryIndex )
  {
    // Correct the current string e.g. remove "Default <" and ">"
    currentString = m_defaultEntryName.str();
  }

  AudioEventInfo * audioEventInfo = TheAudio->findAudioEventInfo(static_cast< const char * >( currentString ) );

  if ( audioEventInfo == NULL )
  {
    minVolumeEdit->SetWindowText( "40" );
    return;
  }

  // Note: min volume is stored as Real between 0.0 and 1.0, but displayed as a percentage from 0 to 100
  CString minVolumeText;
  minVolumeText.Format( "%d", REAL_TO_INT( ( audioEventInfo->m_minVolume * 100.0f ) + 0.5 ) );
  minVolumeEdit->SetWindowText( minVolumeText );
}

/// Move data from object to dialog controls
void MapObjectProps::dictToVolume()
{
  CEdit * volumeEdit = ( CEdit * )GetDlgItem(IDC_VOLUME_EDIT);
  if ( volumeEdit == NULL )
    return;

  // If the customized checkbox is off, all customization controls are disabled
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL || customizeCheckbox->GetCheck() == 0 )
  {
    volumeEdit->EnableWindow( false );
  }
  else
  {
    volumeEdit->EnableWindow( true );

    // If customization is enabled, look to see if the volume has already been
    // customized
    Bool exists = false;
    Real volume = 0.0f;
    if (m_dictToEdit)
    {
      volume = m_dictToEdit->getReal(TheKey_objectSoundAmbientVolume, &exists);
    }

    if ( exists )
    {
      // Note: min volume is stored as Real between 0.0 and 1.0, but displayed as a percentage from 0 to 100
      CString volumeText;
      volumeText.Format( "%d", REAL_TO_INT( ( volume * 100.0f ) + 0.5 ) );
      volumeEdit->SetWindowText( volumeText );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the volume
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
  {
    volumeEdit->SetWindowText( "100" );
    return;
  }

  Int index = soundComboBox->GetCurSel();

  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING || ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    volumeEdit->SetWindowText( "100" );
    return;
  }

  if ( index == m_defaultEntryIndex )
  {
    // Correct the current string e.g. remove "Default <" and ">"
    currentString = m_defaultEntryName.str();
  }

  AudioEventInfo * audioEventInfo = TheAudio->findAudioEventInfo(static_cast< const char * >( currentString ) );

  if ( audioEventInfo == NULL )
  {
    volumeEdit->SetWindowText( "100" );
    return;
  }

  // Note: volume is stored as Real between 0.0 and 1.0, but displayed as a percentage from 0 to 100
  CString volumeText;
  volumeText.Format( "%d", REAL_TO_INT( ( audioEventInfo->m_volume * 100.0f ) + 0.5 ) );
  volumeEdit->SetWindowText( volumeText );
}


/// Move data from object to dialog controls
void MapObjectProps::dictToMinRange()
{
  CEdit * minRangeEdit = ( CEdit * )GetDlgItem(IDC_MIN_RANGE_EDIT);
  if ( minRangeEdit == NULL )
    return;

  // If the customized checkbox is off, all customization controls are disabled
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL || customizeCheckbox->GetCheck() == 0 )
  {
    minRangeEdit->EnableWindow( false );
  }
  else
  {
    minRangeEdit->EnableWindow( true );

    // If customization is enabled, look to see if the minimum range has already been
    // customized
    Bool exists = false;
    Real minRange = 0.0f;
    if (m_dictToEdit)
    {
      minRange = m_dictToEdit->getReal(TheKey_objectSoundAmbientMinRange, &exists);
    }

    if ( exists )
    {
      CString minRangeText;
      minRangeText.Format( "%d", REAL_TO_INT(minRange) );
      minRangeEdit->SetWindowText( minRangeText );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the minimum range
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
  {
    minRangeEdit->SetWindowText( "175" );
    return;
  }

  Int index = soundComboBox->GetCurSel();

  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING || ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    minRangeEdit->SetWindowText( "175" );
    return;
  }

  if ( index == m_defaultEntryIndex )
  {
    // Correct the current string e.g. remove "Default <" and ">"
    currentString = m_defaultEntryName.str();
  }

  AudioEventInfo * audioEventInfo = TheAudio->findAudioEventInfo(static_cast< const char * >( currentString ) );

  if ( audioEventInfo == NULL )
  {
    minRangeEdit->SetWindowText( "175" );
    return;
  }

  CString minRangeText;
  minRangeText.Format( "%d", REAL_TO_INT( audioEventInfo->m_minDistance ) );
  minRangeEdit->SetWindowText( minRangeText );
}


/// Move data from object to dialog controls
void MapObjectProps::dictToMaxRange()
{
  CEdit * maxRangeEdit = ( CEdit * )GetDlgItem(IDC_MAX_RANGE_EDIT);
  if ( maxRangeEdit == NULL )
    return;

  // If the customized checkbox is off, all customization controls are disabled
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL || customizeCheckbox->GetCheck() == 0 )
  {
    maxRangeEdit->EnableWindow( false );
  }
  else
  {
    maxRangeEdit->EnableWindow( true );

    // If customization is enabled, look to see if the maximum range has already been
    // customized
    Bool exists = false;
    Real maxRange = 0.0f;
    if (m_dictToEdit)
    {
      maxRange = m_dictToEdit->getReal(TheKey_objectSoundAmbientMaxRange, &exists);
    }

    if ( exists )
    {
      CString maxRangeText;
      maxRangeText.Format( "%d", REAL_TO_INT( maxRange ) );
      maxRangeEdit->SetWindowText( maxRangeText );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the minimum range
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
  {
    maxRangeEdit->SetWindowText( "600" );
    return;
  }

  Int index = soundComboBox->GetCurSel();

  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING || ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    maxRangeEdit->SetWindowText( "600" );
    return;
  }

  if ( index == m_defaultEntryIndex )
  {
    // Correct the current string e.g. remove "Default <" and ">"
    currentString = m_defaultEntryName.str();
  }

  AudioEventInfo * audioEventInfo = TheAudio->findAudioEventInfo(static_cast< const char * >( currentString ) );

  if ( audioEventInfo == NULL )
  {
    maxRangeEdit->SetWindowText( "600" );
    return;
  }

  CString maxRangeText;
  maxRangeText.Format( "%d", REAL_TO_INT( audioEventInfo->m_maxDistance ) );
  maxRangeEdit->SetWindowText( maxRangeText );
}

/// Move data from object to dialog controls
void MapObjectProps::dictToPriority()
{
  CComboBox * priorityComboBox = ( CComboBox * )GetDlgItem(IDC_PRIORITY_COMBO);
  if ( priorityComboBox == NULL )
    return;

  // If the customized checkbox is off, all customization controls are disabled
  CButton * customizeCheckbox = ( CButton * )GetDlgItem(IDC_CUSTOMIZE_CHECKBOX);
  if ( customizeCheckbox == NULL || customizeCheckbox->GetCheck() == 0 )
  {
    priorityComboBox->EnableWindow( false );
  }
  else
  {
    priorityComboBox->EnableWindow( true );

    // If customization is enabled, look to see if the maximum range has already been
    // customized
    Bool exists = false;
    Int priorityEnum = AP_LOWEST;
    if (m_dictToEdit)
    {
      priorityEnum = m_dictToEdit->getInt(TheKey_objectSoundAmbientPriority, &exists);
    }

    if ( exists )
    {
      if ( priorityEnum < 0 || priorityEnum > AP_CRITICAL )
      {
        DEBUG_CRASH( ("Bad soundAmbientPriority key %d", priorityEnum ) );
        priorityEnum = AP_LOWEST;
      }

      // Note: indexes of priority combobox map to priority enum
      priorityComboBox->SetCurSel( priorityEnum );

      // Don't use defaults, just return
      return;
    }
  }

  // If we get here, we need the default for the priority
  CComboBox * soundComboBox = (CComboBox *)GetDlgItem(IDC_SOUND_COMBO);
  if ( soundComboBox == NULL )
  {
    priorityComboBox->SetCurSel( AP_LOWEST );
    return;
  }

  Int index = soundComboBox->GetCurSel();

  CString currentString;
  soundComboBox->GetLBText( index, currentString );
  if ( currentString == NO_SOUND_STRING || ( index == m_defaultEntryIndex && m_defaultIsNone ) )
  {
    priorityComboBox->SetCurSel( AP_LOWEST );
    return;
  }

  if ( index == m_defaultEntryIndex )
  {
    // Correct the current string e.g. remove "Default <" and ">"
    currentString = m_defaultEntryName.str();
  }

  AudioEventInfo * audioEventInfo = TheAudio->findAudioEventInfo(static_cast< const char * >( currentString ) );

  if ( audioEventInfo == NULL )
  {
    priorityComboBox->SetCurSel( AP_LOWEST );
    return;
  }

  priorityComboBox->SetCurSel( audioEventInfo->m_priority );
}


// Special message to handle pages being resized to new sheet size
// see knowledge base article Q143291
const Int RESIZE_PAGE_MESSAGE = WM_USER + 117;


/////////////////////////////////////////////////////////////////////////////
// MapObjectProps message handlers



void MapObjectProps::enableButtons()
{
	// do nothing
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





void MapObjectProps::OnOK()
{
  // Make sure CPropertySheet functions don't close the window
}

void MapObjectProps::OnCancel()
{
  // Make sure CPropertySheet functions don't close the window
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

/// Move data from dialog controls to object
void MapObjectProps::OnScaleOn()
{
  _ScaleToDict();
}

/// Move data from dialog controls to object
void MapObjectProps::OnScaleOff()
{
  _ScaleToDict();
}

/// Move data from dialog controls to object
void MapObjectProps::OnKillfocusMAPOBJECTXYPosition()
{
  SetPosition();
}

