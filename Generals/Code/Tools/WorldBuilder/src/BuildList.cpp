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

// BuildList.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "BuildList.h"
#include "BuildListTool.h"
#include "BaseBuildProps.h"
#include "CUndoable.h"
#include "PointerTool.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "GameLogic/SidesList.h"
#include "Common/PlayerTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/WellKnownKeys.h"
#include "wbview3d.h"

BuildList *BuildList::m_staticThis = NULL;
Bool BuildList::m_updating = false;


/////////////////////////////////////////////////////////////////////////////
// BuildList dialog


BuildList::BuildList(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(BuildList)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


BuildList::~BuildList(void)
{
}


void BuildList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BuildList)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BuildList, COptionsPanel)
	//{{AFX_MSG_MAP(BuildList)
	ON_CBN_SELCHANGE(IDC_SIDES_COMBO, OnSelchangeSidesCombo)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_ADD_BUILDING, OnAddBuilding)
	ON_LBN_SELCHANGE(IDC_BUILD_LIST, OnSelchangeBuildList)
	ON_BN_CLICKED(IDC_ALREADY_BUILD, OnAlreadyBuild)
	ON_BN_CLICKED(IDC_DELETE_BUILDING, OnDeleteBuilding)
	ON_CBN_SELENDOK(IDC_REBUILDS, OnSelendokRebuilds)
	ON_CBN_EDITCHANGE(IDC_REBUILDS, OnEditchangeRebuilds)
	ON_LBN_DBLCLK(IDC_BUILD_LIST, OnDblclkBuildList)
	ON_EN_CHANGE(IDC_MAPOBJECT_ZOffset, OnChangeZOffset)
	ON_EN_CHANGE(IDC_MAPOBJECT_Angle, OnChangeAngle)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BuildList data access method.



/////////////////////////////////////////////////////////////////////////////
// BuildList message handlers

/// Setup the controls in the dialog.
BOOL BuildList::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_heightSlider.SetupPopSliderButton(this, IDC_HEIGHT_POPUP, this);
	m_angleSlider.SetupPopSliderButton(this, IDC_ANGLE_POPUP, this);

	m_updating = true;
	loadSides();
	m_curSide = 0;
	updateCurSide();
	OnSelchangeBuildList();
	m_staticThis = this;
	m_updating = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/// Load the sides in the sides list.
void BuildList::loadSides(void) 
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_SIDES_COMBO);
	if (!pCombo) {
		DEBUG_LOG(("*** BuildList::loadSides Missing resource!!!"));
		return;
	}
	pCombo->ResetContent();
	Int i;
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		Dict *d = TheSidesList->getSideInfo(i)->getDict();
		AsciiString name = d->getAsciiString(TheKey_playerName);
		UnicodeString uni = d->getUnicodeString(TheKey_playerDisplayName);
		AsciiString fmt;
		if (name.isEmpty())
			fmt = "(neutral player, cannot be edited)";
		else
			fmt.format("%s=\"%ls\"",name.str(),uni.str());
		pCombo->AddString(fmt.str());
	}
	updateCurSide();
}

/// Updates the current side, loading it's build list.
void BuildList::updateCurSide(void) 
{
	if (TheSidesList->getNumSides() < 1)
		return;

	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_SIDES_COMBO);
	if (!pCombo) {
		DEBUG_LOG(("*** BuildList::updateCurSide Missing resource!!!"));
		return;
	}
	if (m_curSide<0 || m_curSide >= TheSidesList->getNumSides()) {
		m_curSide = 0;
	}
	if (pCombo->GetCurSel() != m_curSide) {
		pCombo->SetCurSel(m_curSide);
	}
	SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 

	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	if (!pList) {
		DEBUG_LOG(("*** BuildList::updateCurSide Missing resource!!! IDC_BUILD_LIST"));
		return;
	}
	pList->ResetContent();
	
	BuildListInfo *pBuild = pSide->getBuildList();
	while (pBuild) {
		const char *pName = pBuild->getTemplateName().str(); 
		pList->AddString(pName);
		pBuild = pBuild->getNext();
	}
	OnSelchangeBuildList();
}

void BuildList::OnSelchangeSidesCombo() 
{
	if (TheSidesList->getNumSides() < 1)
		return;

	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_SIDES_COMBO);
	if (!pCombo) {
		DEBUG_LOG(("*** BuildList::OnSelchangeSidesCombo Missing resource!!!"));
		return;
	}

	if (pCombo->GetCurSel() != m_curSide) {
		m_curSide = pCombo->GetCurSel();
		updateCurSide();
		CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
		if (pList) {
			pList->SetCurSel(0);
		}
		OnSelchangeBuildList();
	}
	
}

void BuildList::OnMoveUp() 
{
	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	if (!pList) {
		DEBUG_LOG(("*** BuildList::updateCurSide Missing resource!!! IDC_BUILD_LIST"));
		return;
	}
	m_curBuildList = pList->GetCurSel();
	if (m_curBuildList < 1) return;
	SidesList	sides;
	sides = *TheSidesList;

	SidesInfo *pSide = sides.getSideInfo(m_curSide);
	Int count = m_curBuildList;

	BuildListInfo *pBuildInfo = pSide->getBuildList();
	while (count) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
		if (pBuildInfo == NULL) return;
	}
	Int newSel = m_curBuildList-1;
	pSide->reorderInBuildList(pBuildInfo, newSel); 
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	SidesListUndoable *pUndo = new SidesListUndoable(sides, pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.

	updateCurSide();
	pList->SetCurSel(newSel);
	OnSelchangeBuildList();
}

void BuildList::OnMoveDown() 
{
	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	if (!pList) {
		DEBUG_LOG(("*** BuildList::updateCurSide Missing resource!!! IDC_BUILD_LIST"));
		return;
	}
	if (m_curBuildList < 0) return;
	SidesList	sides;
	sides = *TheSidesList;

	SidesInfo *pSide = sides.getSideInfo(m_curSide);
	m_curBuildList = pList->GetCurSel();
	Int count = m_curBuildList;
	BuildListInfo *pBuildInfo = pSide->getBuildList();
	while (count) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
		if (pBuildInfo == NULL) return;
	}
	if (pBuildInfo->getNext() == NULL) {
		// there isn't one to move down after. 
		return;
	}
	Int newSel = m_curBuildList+1;

	pSide->reorderInBuildList(pBuildInfo, newSel); 
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	SidesListUndoable *pUndo = new SidesListUndoable(sides, pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	updateCurSide();
	pList->SetCurSel(newSel);
	OnSelchangeBuildList();
}

void BuildList::OnAddBuilding() 
{
	BuildListTool::addBuilding();
}


void BuildList::addBuilding(Coord3D loc, Real angle, AsciiString name)
{
	if (!m_staticThis) {
		return;
	}
	BuildListInfo *pBuildInfo = newInstance( BuildListInfo);
	pBuildInfo->setAngle(angle);
	pBuildInfo->setTemplateName(name);
	pBuildInfo->setLocation(loc);

	SidesList	sides;
	sides = *TheSidesList;

	SidesInfo *pSide = sides.getSideInfo(m_staticThis->m_curSide);
	pSide->addToBuildList(pBuildInfo, 1000); // add at end.
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	SidesListUndoable *pUndo = new SidesListUndoable(sides, pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	m_staticThis->updateCurSide();
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->invalBuildListItemInView(pBuildInfo);
};


void BuildList::setSelectedBuildList(BuildListInfo *pInfo)
{
	if (!m_staticThis) {
		return;
	}
	Int i;
 	for (i=0; i<TheSidesList->getNumSides(); i++) {
		SidesInfo *pSide = TheSidesList->getSideInfo(i); 
		Int listSel = 0;
		for (BuildListInfo *pBuild = pSide->getBuildList(); pBuild; pBuild = pBuild->getNext()) {
			if (pInfo == pBuild) {
				pBuild->setSelected(true);
//				CComboBox *pCombo = (CComboBox*)m_staticThis->GetDlgItem(IDC_SIDES_COMBO);
				if (m_staticThis->m_curSide != i) {
					m_staticThis->m_curSide = i;
					m_staticThis->updateCurSide();
				}
				CListBox *pList = (CListBox*)m_staticThis->GetDlgItem(IDC_BUILD_LIST);
				pList->SetCurSel(listSel);
				m_staticThis->OnSelchangeBuildList();
			} else {
				pBuild->setSelected(false);
			}
			listSel++;
		}
	}
};


void BuildList::OnSelchangeBuildList() 
{
	if (TheSidesList->getNumSides() < 1)
		return;

	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	if (!pList) {
		DEBUG_LOG(("*** BuildList::updateCurSide Missing resource!!! IDC_BUILD_LIST"));
		return;
	}
	m_curBuildList = pList->GetCurSel();
	Int numBL = pList->GetCount();
	
	SidesInfo *pSide = NULL;
	if (TheSidesList) {
		pSide = TheSidesList->getSideInfo(m_curSide);
	}
	Int count = m_curBuildList;
	BuildListInfo *pBuildInfo = NULL;
	if (pSide) {
		pBuildInfo = pSide->getBuildList();
	}
	if (count<0) pBuildInfo = NULL;
	while (count && pBuildInfo) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
	}

	Bool enableUp=false;
	Bool enableDown=false;
	Bool enableAttrs = true;

	Int energyConsumption = 0;
	Int energyProduction = 0;
	Real energyUsed = 0.0f;
	const Dict *playerDict = pSide->getDict();
	AsciiString playerName = playerDict->getAsciiString(TheKey_playerName);

	// add up energy consumed/produced by objects on the map
	const ThingTemplate *thingTemplate;
	for (MapObject *pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) 
	{
		// get thing template based from map object name
		thingTemplate = pMapObj->getThingTemplate();
		if (!thingTemplate)
			continue;
		Dict *d = pMapObj->getProperties();
		Bool exists;
		AsciiString objectTeamName = d->getAsciiString(TheKey_originalOwner, &exists);
		TeamsInfo *teamInfo = TheSidesList->findTeamInfo(objectTeamName);
		Dict *teamDict = (teamInfo)?teamInfo->getDict():NULL;
		AsciiString objectOwnerName = (teamDict)?teamDict->getAsciiString(TheKey_teamOwner):AsciiString::TheEmptyString;

		Int energy = 0;
		if (objectOwnerName.compareNoCase(playerName) == 0)
			energy = thingTemplate->getEnergyProduction();

		if (energy > 0)
			energyProduction += energy;
		else
			energyConsumption -= energy; // keep it positive
	}

	// add up energy consumed/produced by objects in the build list
	{
		count = m_curBuildList;
		BuildListInfo *pBuildInfo = pSide->getBuildList();
		if (count<0) pBuildInfo = NULL;
		while (count>=0 && pBuildInfo) {
			AsciiString tName = pBuildInfo->getTemplateName();
			const ThingTemplate *templ = TheThingFactory->findTemplate(tName);
			if (!templ)
				break;

			Int energy = templ->getEnergyProduction();

			if (energy > 0)
				energyProduction += energy;
			else
				energyConsumption -= energy; // keep it positive

			count--;
			pBuildInfo = pBuildInfo->getNext();
		}
	}

	if (energyProduction)
	{
		energyUsed = (Real)energyConsumption/(Real)energyProduction;
		energyUsed = min(1.0f, energyUsed);
		energyUsed = max(0.0f, energyUsed);
	}
	else if (energyConsumption)
	{
		energyUsed = 1.0f;
	}
	//DEBUG_LOG(("Energy: %d/%d - %g", energyConsumption, energyProduction, energyUsed));
	CProgressCtrl *progressWnd = (CProgressCtrl *)GetDlgItem(IDC_POWER);
	if (progressWnd)
	{
		progressWnd->EnableWindow(true);
		progressWnd->SetPos((Int)((1.0f-energyUsed)*100));
	}

	if (pBuildInfo==NULL) {
		enableAttrs = false;
	}
	if (m_curBuildList > 0) {
		enableUp = true;
	}
	if (m_curBuildList >= 0 && m_curBuildList < numBL-1) {
		enableDown = true;
	}
	CWnd *pWnd = GetDlgItem(IDC_MOVE_UP);
	if (pWnd) pWnd->EnableWindow(enableUp);
	pWnd = GetDlgItem(IDC_MOVE_DOWN);
	if (pWnd) pWnd->EnableWindow(enableDown);
	pWnd = GetDlgItem(IDC_ALREADY_BUILD);
	if (pWnd) pWnd->EnableWindow(enableAttrs);
	pWnd = GetDlgItem(IDC_REBUILDS);
	if (pWnd) pWnd->EnableWindow(enableAttrs);
	pWnd = GetDlgItem(IDC_DELETE_BUILDING);
	if (pWnd) pWnd->EnableWindow(enableAttrs);
	PointerTool::clearSelection(); // unselect other stuff.
	if (pBuildInfo) {
		CWnd *edit;
		static char buff[12];
		pBuildInfo->setSelected(true);

		m_angle = pBuildInfo->getAngle() * 180/PI;
		sprintf(buff, "%0.2f", m_angle);
		edit = GetDlgItem(IDC_MAPOBJECT_Angle);
		edit->SetWindowText(buff);

		m_height = pBuildInfo->getLocation()->z;
		sprintf(buff, "%0.2f", m_height);
		edit = GetDlgItem(IDC_MAPOBJECT_ZOffset);
		edit->SetWindowText(buff);

		CButton *pBtn = (CButton *)GetDlgItem(IDC_ALREADY_BUILD);
		if (pBtn) pBtn->SetCheck(pBuildInfo->isInitiallyBuilt()?1:0);
		CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_REBUILDS);
		if (pCombo==NULL) return;
		UnsignedInt nr = pBuildInfo->getNumRebuilds();
		if (nr == BuildListInfo::UNLIMITED_REBUILDS) {
			pCombo->SetCurSel(6);
		} else if (nr<6) {
			pCombo->SetCurSel(nr);
		} else {
			CString str;
			str.Format("%d", nr);
			pCombo->SetWindowText(str);
		}
	}


}

void BuildList::OnAlreadyBuild() 
{
	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	if (!pList) {
		DEBUG_LOG(("*** BuildList::updateCurSide Missing resource!!! IDC_BUILD_LIST"));
		return;
	}
	SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 
	m_curBuildList = pList->GetCurSel();
	if (m_curBuildList < 0) return;
	Int count = m_curBuildList;
	BuildListInfo *pBuildInfo = pSide->getBuildList();
	while (count) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
		if (pBuildInfo == NULL) return;
	}
	CButton *pBtn = (CButton *)GetDlgItem(IDC_ALREADY_BUILD);
	if (pBtn) {
		pBuildInfo->setInitiallyBuilt(pBtn->GetCheck()==1);
	}
}

void BuildList::OnDeleteBuilding() 
{
	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	if (!pList) {
		DEBUG_LOG(("*** BuildList::updateCurSide Missing resource!!! IDC_BUILD_LIST"));
		return;
	}
	m_curBuildList = pList->GetCurSel();
	if (m_curBuildList < 0) return;
	SidesList	sides;
	sides = *TheSidesList;

	SidesInfo *pSide = sides.getSideInfo(m_curSide);
	Int count = m_curBuildList;
	BuildListInfo *pBuildInfo = pSide->getBuildList();
	while (count) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
		if (pBuildInfo == NULL) return;
	}
	pSide->removeFromBuildList(pBuildInfo); 

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	SidesListUndoable *pUndo = new SidesListUndoable(sides, pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	updateCurSide();
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->invalBuildListItemInView(pBuildInfo);
	pList->SetCurSel(-1);
}

void BuildList::OnSelendokRebuilds() 
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_REBUILDS);
	if (pCombo==NULL) return;
	Int sel = pCombo->GetCurSel();
	if (sel<0) return; // no selection.
	UnsignedInt nr;
	if (sel == 6) {
		nr = BuildListInfo::UNLIMITED_REBUILDS;
	} else if (sel<6) {
		nr = sel;
	} 
	SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 
	if (m_curBuildList < 0) return;
	Int count = m_curBuildList;
	BuildListInfo *pBuildInfo = pSide->getBuildList();
	while (count) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
		if (pBuildInfo == NULL) return;
	}
	pBuildInfo->setNumRebuilds(nr);
}

void BuildList::OnEditchangeRebuilds() 
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_REBUILDS);
	if (pCombo==NULL) return;
	Int sel = pCombo->GetCurSel();
	if (sel>=0) return; // An entry is selected, and handled by OnSelendokRebuilds..
	char buffer[_MAX_PATH];
	if (pCombo) {
		pCombo->GetWindowText(buffer, sizeof(buffer));
		Int nr;
		if (1==sscanf(buffer, "%d", &nr)) {
		}
			SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 
			if (m_curBuildList < 0) return;
			Int count = m_curBuildList;
			BuildListInfo *pBuildInfo = pSide->getBuildList();
			while (count) {
				count--;
				pBuildInfo = pBuildInfo->getNext();
				if (pBuildInfo == NULL) return;
			}
			pBuildInfo->setNumRebuilds(nr);
	}
}

void BuildList::OnDblclkBuildList() 
{
	SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 
	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	m_curBuildList = pList->GetCurSel();
	if (m_curBuildList < 0) return;
	Int count = m_curBuildList;
	BuildListInfo *pBI = pSide->getBuildList();
	while (count) {
		count--;
		pBI = pBI->getNext();
		if (pBI == NULL) return;
	}

	BaseBuildProps dlg;
	dlg.setProps(pBI->getBuildingName(), pBI->getScript(), pBI->getHealth(), pBI->getUnsellable());
	if (dlg.DoModal() == IDOK) {
		pBI->setBuildingName(dlg.getName());
		pBI->setScript(dlg.getScript());
		pBI->setHealth(dlg.getHealth());
		pBI->setUnsellable(dlg.getUnsellable());
	}
}

void BuildList::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
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

void BuildList::PopSliderChanged(const long sliderID, long theVal)
{
//	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	CWnd* edit;
	static char buff[12];
	switch (sliderID) {
		case IDC_HEIGHT_POPUP:
			m_height = theVal;
			sprintf(buff, "%0.2f", m_height);
			edit = GetDlgItem(IDC_MAPOBJECT_ZOffset);
			edit->SetWindowText(buff);
			OnChangeZOffset();
			break;

		case IDC_ANGLE_POPUP:
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

void BuildList::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_HEIGHT_POPUP:
		case IDC_ANGLE_POPUP:
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch

}

void BuildList::OnChangeZOffset() 
{
	Real value = 0.0f;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_ZOffset);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		value = atof(cstr);
	}
	m_height = value;

	SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 
	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	m_curBuildList = pList->GetCurSel();
	if (m_curBuildList < 0) return;
	Int count = m_curBuildList;
	BuildListInfo *pBuildInfo = pSide->getBuildList();
	while (count) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
		if (pBuildInfo == NULL) return;
	}
	Coord3D loc = *pBuildInfo->getLocation();
	loc.z = m_height;
	pBuildInfo->setLocation(loc);
	WbView3d *p3View = CWorldBuilderDoc::GetActiveDoc()->GetActive3DView();
	p3View->invalBuildListItemInView(pBuildInfo);
}

void BuildList::OnChangeAngle() 
{
	Real angle = 0.0f;
	CWnd* edit = GetDlgItem(IDC_MAPOBJECT_Angle);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		angle = atof(cstr);
	}
	m_angle = angle;
	SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 
	CListBox *pList = (CListBox*)GetDlgItem(IDC_BUILD_LIST);
	m_curBuildList = pList->GetCurSel();
	if (m_curBuildList < 0) return;
	Int count = m_curBuildList;
	BuildListInfo *pBuildInfo = pSide->getBuildList();
	while (count) {
		count--;
		pBuildInfo = pBuildInfo->getNext();
		if (pBuildInfo == NULL) return;
	}
	pBuildInfo->setAngle(m_angle * PI/180);
	WbView3d *p3View = CWorldBuilderDoc::GetActiveDoc()->GetActive3DView();
	p3View->invalBuildListItemInView(pBuildInfo);
}

void BuildList::OnExport() 
{
	static FILE *theLogFile = NULL;
	Bool open = false;
	try {
		char dirbuf[ _MAX_PATH ];
		::GetModuleFileName( NULL, dirbuf, sizeof( dirbuf ) );
		char *pEnd = dirbuf + strlen( dirbuf );
		while( pEnd != dirbuf ) 
		{
			if( *pEnd == '\\' ) 
			{
				*(pEnd + 1) = 0;
				break;
			}
			pEnd--;
		}

		char curbuf[ _MAX_PATH ];

		strcpy(curbuf, dirbuf);
		SidesInfo *pSide = TheSidesList->getSideInfo(m_curSide); 
		Dict *d = TheSidesList->getSideInfo(m_curSide)->getDict();
		AsciiString name = d->getAsciiString(TheKey_playerName);
		strcat(curbuf, name.str());
		strcat(curbuf, "_BuildList");
		strcat(curbuf, ".ini");

		theLogFile = fopen(curbuf, "w");
		if (theLogFile == NULL)
			throw;

		AsciiString tmplname = d->getAsciiString(TheKey_playerFaction);
		const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(tmplname));
		DEBUG_ASSERTCRASH(pt != NULL, ("PlayerTemplate %s not found -- this is an obsolete map (please open and resave in WB)",tmplname.str()));
		
		fprintf(theLogFile, ";Skirmish AI Build List\n");
		fprintf(theLogFile, "SkirmishBuildList %s\n", pt->getSide().str());

		open = true;
		
		BuildListInfo *pBuildInfo = pSide->getBuildList();
		while (pBuildInfo) {
			fprintf(theLogFile, "  Structure %s\n", pBuildInfo->getTemplateName().str());
			if (!pBuildInfo->getBuildingName().isEmpty()) {
				fprintf(theLogFile, "    Name = %s\n", pBuildInfo->getBuildingName().str());
			}
			fprintf(theLogFile, "    Location = X:%.2f Y:%.2f\n", pBuildInfo->getLocation()->x, pBuildInfo->getLocation()->y);
			fprintf(theLogFile, "    Rebuilds = %d\n", pBuildInfo->getNumRebuilds());
			fprintf(theLogFile, "    Angle = %.2f\n", pBuildInfo->getAngle()*180/PI);
			fprintf(theLogFile, "    InitiallyBuilt = %s\n", pBuildInfo->isInitiallyBuilt()?"Yes":"No");
			fprintf(theLogFile, "    AutomaticallyBuild = %s\n", false?"Yes":"No");
			fprintf(theLogFile, "  END ;Structure %s\n", pBuildInfo->getTemplateName().str());
			pBuildInfo = pBuildInfo->getNext();
		}
		fprintf(theLogFile, "END ;SkirmishBuildList %s\n", tmplname.str());
		fclose(theLogFile);
		open = false;
	} catch (...) {
		if (open) {
			fclose(theLogFile);
		}
	}
	
}
