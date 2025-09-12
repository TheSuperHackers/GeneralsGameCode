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

// RoadOptions.cpp : implementation file
//


#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "RoadOptions.h"
#include "CUndoable.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/FileSystem.h"
#include "GameClient/TerrainRoads.h"

#include <list>

RoadOptions *RoadOptions::m_staticThis = NULL;
Bool RoadOptions::m_updating = false;
AsciiString RoadOptions::m_currentRoadName;
Int RoadOptions::m_currentRoadIndex=0;
Int RoadOptions::m_numberOfRoads=0;
Int RoadOptions::m_numberOfBridges=0;

Bool RoadOptions::m_angleCorners = false; ///<angled or curved.
Bool RoadOptions::m_tightCurve = false;		///< Broad curve false, tight curve true.
Bool RoadOptions::m_doJoin = false;		///< Is a join to different road type.

/////////////////////////////////////////////////////////////////////////////
// RoadOptions dialog


RoadOptions::RoadOptions(CWnd* pParent /*=NULL*/)
{
	m_currentRoadName = "Road";
	//{{AFX_DATA_INIT(RoadOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


RoadOptions::~RoadOptions(void)
{
	m_currentRoadName.clear();
}


void RoadOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RoadOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RoadOptions, COptionsPanel)
	//{{AFX_MSG_MAP(RoadOptions)
	ON_BN_CLICKED(IDC_TIGHT_CURVE, OnTightCurve)
	ON_BN_CLICKED(IDC_ANGLED, OnAngled)
	ON_BN_CLICKED(IDC_BROAD_CURVE, OnBroadCurve)
	ON_BN_CLICKED(IDC_JOIN, OnJoin)
	ON_BN_CLICKED(IDC_APPLY_ROAD, OnApplyRoad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RoadOptions data access method.


void RoadOptions::updateLabel(void)
{
	const char *tName = getCurRoadName().str();

	CWnd *pLabel = GetDlgItem(IDC_ROAD_NAME);
	if (pLabel) {
		pLabel->SetWindowText(tName);
	}
}

/** Returns true if only one or more roads is selected. */
Bool RoadOptions::selectionIsRoadsOnly(void)
{
//	MapObject *theMapObj = NULL;
	Bool foundRoad = false;
	Bool foundAnythingElse = false;
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isSelected()) {
			if (pMapObj->getFlag(FLAG_ROAD_FLAGS)) {
				foundRoad = true;
			}	else {
				foundAnythingElse = true;
			}
		}
	}
	return (foundRoad && (!foundAnythingElse));
}

/** Returns true if only one or more roads is selected. */
void RoadOptions::updateSelection(void)
{
//	MapObject *theMapObj = NULL;
	Int angled = 0;
	Int tight = 0;
	Int broad = 0;
	Int join = 0;
	AsciiString roadName;
	Bool multipleNames = false;

	if (!m_staticThis) return;
	MapObject *pMapObj;

	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isSelected() && pMapObj->getFlag(FLAG_ROAD_FLAGS)) {
			if (roadName.isEmpty()) {
				roadName = pMapObj->getName();
			} else {
				if (!(roadName==pMapObj->getName())) {
					multipleNames = true;
				}
			}
			if (pMapObj->getFlag(FLAG_ROAD_CORNER_ANGLED)) {
				angled = 1;
			}	else if (pMapObj->getFlag(FLAG_ROAD_CORNER_TIGHT)) {
				tight = 1;
			}	else {
				broad = 1;
			}
			if (pMapObj->getFlag(FLAG_ROAD_JOIN)) {
				join = 1;
			}
		}
	}
	if (!roadName.isEmpty() && !multipleNames) {
		CWnd *pLabel = m_staticThis->GetDlgItem(IDC_ROAD_NAME);
		if (pLabel) {
			pLabel->SetWindowText(roadName.str());
			m_staticThis->findAndSelect(TVI_ROOT, roadName);
		}
	} else {
		m_staticThis->setRoadTreeViewSelection(TVI_ROOT, m_currentRoadIndex);
	}
	if (angled+broad+tight==0) {
		// nothing selected
		if (m_angleCorners) {
			angled = 1;
		}	 else if (m_tightCurve) {
			tight = 1;
		} else {
			broad = 1;
		}
		if (m_doJoin) {
			join = 1;
		}
	} else if (angled+broad+tight==1) {
		// One type selected
	} else {
		angled = tight = broad = join = 0;
	}
	if (m_staticThis) {
		CButton *pButton = (CButton *)m_staticThis->GetDlgItem(IDC_TIGHT_CURVE);
		pButton->SetCheck(tight);
		pButton = (CButton *)m_staticThis->GetDlgItem(IDC_BROAD_CURVE);
		pButton->SetCheck(broad);
		pButton = (CButton *)m_staticThis->GetDlgItem(IDC_ANGLED);
		pButton->SetCheck(angled);
		pButton = (CButton *)m_staticThis->GetDlgItem(IDC_JOIN);
		pButton->SetCheck(join);
	}
}

/** Applies road corner flags and road type to selection. */
void RoadOptions::applyToSelection(void)
{
	Int flagMask = FLAG_ROAD_CORNER_ANGLED | FLAG_ROAD_CORNER_TIGHT;
	Int flagVal = 0;
	if (m_angleCorners) {
		flagVal = FLAG_ROAD_CORNER_ANGLED;
	} else if (m_tightCurve) {
		flagVal = FLAG_ROAD_CORNER_TIGHT;
	}
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	ModifyFlagsUndoable *pUndo = new ModifyFlagsUndoable(pDoc, flagMask, flagVal);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}


/////////////////////////////////////////////////////////////////////////////
// RoadOptions message handlers

/// Setup the controls in the dialog.
BOOL RoadOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_updating = true;

	CWnd *pWnd = GetDlgItem(IDC_ROAD_TREEVIEW);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_roadTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, rect, this, IDC_ROAD_TREEVIEW);
	m_roadTreeView.ShowWindow(SW_SHOW);

	Int index = 0;
	m_numberOfRoads = 0;

	// load roads from INI
	TerrainRoadType *road;
	for( road = TheTerrainRoads->firstRoad(); road; road = TheTerrainRoads->nextRoad( road ) )
	{

		addRoad( (char *)road->getName().str(), index, TVI_ROOT );
		index++;
		m_numberOfRoads++;

	}

	// load roads from test assets
#ifdef LOAD_TEST_ASSETS
	{
		char				dirBuf[_MAX_PATH];
		char				findBuf[_MAX_PATH];
		char				fileBuf[_MAX_PATH];

		strcpy(dirBuf, ROAD_DIRECTORY);
		int len = strlen(dirBuf);

		strcpy(findBuf, dirBuf);

		FilenameList filenameList;
		TheFileSystem->getFileListInDirectory(AsciiString(findBuf), AsciiString("*.tga"), filenameList, FALSE);

		if (filenameList.size() > 0) {
			FilenameList::iterator it = filenameList.begin();
			do {
				AsciiString	filename = *it;

				if ((filename.compare(".") == 0) || (filename.compare("..") == 0)) {
					++it;
					continue;
				}
				len = filename.getLength();
				if (len<5) {
					++it;
					continue;
				}
				// only do .tga files
				if (!(filename.endsWith(".tga"))) {
					++it;
					continue;
				}
				strcpy(fileBuf, TEST_STRING);
				strcat(fileBuf, "\\");
				strcat(fileBuf, filename.str());
				addRoad(fileBuf, index, TVI_ROOT);
				index++;
				m_numberOfRoads++;
				++it;
			} while (it != filenameList.end());

 		}
	}
#endif

	m_numberOfBridges = 0;
	// add bridge defs from INI
	TerrainRoadType *bridge;
	for( bridge = TheTerrainRoads->firstBridge();
			 bridge;
			 bridge = TheTerrainRoads->nextBridge( bridge ) )
	{

		addRoad( (char *)bridge->getName().str(), index, TVI_ROOT );
		index++;
		m_numberOfBridges++;

	}

	m_currentRoadIndex = 1;
	setRoadTreeViewSelection(TVI_ROOT, m_currentRoadIndex);
	updateLabel();
	m_staticThis = this;
	updateSelection();
	m_updating = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM RoadOptions::findOrAdd(HTREEITEM parent, const char *pLabel)
{
	TVINSERTSTRUCT ins;
	char buffer[_MAX_PATH];
	::memset(&ins, 0, sizeof(ins));
	HTREEITEM child = m_roadTreeView.GetChildItem(parent);
	while (child != NULL) {
		ins.item.mask = TVIF_HANDLE|TVIF_TEXT;
		ins.item.hItem = child;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = sizeof(buffer)-2;
		m_roadTreeView.GetItem(&ins.item);
		if (strcmp(buffer, pLabel) == 0) {
			return(child);
		}
		child = m_roadTreeView.GetNextSiblingItem(child);
	}

	// not found, so add it.
	::memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_LAST;
	ins.item.mask = TVIF_PARAM|TVIF_TEXT;
	ins.item.lParam = -1;
	ins.item.pszText = const_cast<char*>(pLabel);
	ins.item.cchTextMax = strlen(pLabel);
	child = m_roadTreeView.InsertItem(&ins);
	return(child);
}

/** Locate and select child item in tree item parent with name pLabel.  If not
found, false.  If found, return true. */
Bool RoadOptions::findAndSelect(HTREEITEM parent, AsciiString label)
{
	TVINSERTSTRUCT ins;
	char buffer[_MAX_PATH];
	::memset(&ins, 0, sizeof(ins));
	HTREEITEM child = m_roadTreeView.GetChildItem(parent);
	while (child != NULL) {
		ins.item.mask = TVIF_HANDLE|TVIF_TEXT;
		ins.item.hItem = child;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = sizeof(buffer)-2;
		m_roadTreeView.GetItem(&ins.item);
		if (label.compare(buffer) == 0) {
			m_roadTreeView.SelectItem(child);
			return(true);
		}
		if (findAndSelect(child, label)) {
			return true;
		}
		child = m_roadTreeView.GetNextSiblingItem(child);
	}
	return false;
}

/** Add the road hierarchy paths to the tree view. */
void RoadOptions::addRoad(char *pPath, Int terrainNdx, HTREEITEM parent)
{
	TerrainRoadType *road;
	char buffer[_MAX_PATH];
	Bool doAdd = FALSE;

	// try to find the road in our INI definition
	road = TheTerrainRoads->findRoadOrBridge( AsciiString( pPath ) );
	if( road )
	{

		// roads go in a road tree, bridges in their own tree
		if( road->isBridge() == TRUE )
			parent = findOrAdd( parent, "Bridges" );
		else
			parent = findOrAdd( parent, "Roads" );

		// set the name to place as the name of the road entry in INI
		strcpy( buffer, road->getName().str() );

		// do the add
		doAdd = TRUE;

	}

#ifdef LOAD_TEST_ASSETS
	if (!doAdd && !strncmp(TEST_STRING, pPath, strlen(TEST_STRING))) {
		parent = findOrAdd(parent, TEST_STRING);
		strcpy(buffer, pPath + strlen(TEST_STRING) + 1);
		doAdd = true;
	}
#endif

	if( doAdd )
	{
		TVINSERTSTRUCT ins;

		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_LAST;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = terrainNdx;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = strlen(buffer)+2;
		m_roadTreeView.InsertItem(&ins);
	}

}

/// Set the selected road in the tree view.
Bool RoadOptions::setRoadTreeViewSelection(HTREEITEM parent, Int selection)
{
	TVITEM item;
	char buffer[NAME_MAX_LEN];
	::memset(&item, 0, sizeof(item));
	HTREEITEM child = m_roadTreeView.GetChildItem(parent);
	while (child != NULL) {
		item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
		item.hItem = child;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer)-2;
		m_roadTreeView.GetItem(&item);
		if (item.lParam == selection) {
			m_roadTreeView.SelectItem(child);
			m_currentRoadName = buffer;
			return(true);
		}
		if (setRoadTreeViewSelection(child, selection)) {
			return(true);
		}
		child = m_roadTreeView.GetNextSiblingItem(child);
	}
	return(false);
}

void RoadOptions::SelectConnected(void)
{
	std::list<MapObject*> roadSegs;
	std::list<MapObject*> connectedSegs;
	for (MapObject* pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		if (pMapObj->getFlag(FLAG_ROAD_POINT1))
		{
			if (pMapObj->isSelected() || pMapObj->getNext() && pMapObj->getNext()->isSelected())
			{
				connectedSegs.push_back(pMapObj);
			}
			else
			{
				roadSegs.push_back(pMapObj);
			}
		}
	}
	Bool changed = true;
	while (changed)
	{
		changed = false;
		for (std::list<MapObject*>::iterator it = roadSegs.begin(); it != roadSegs.end(); ++it)
		{
			MapObject* o = *it;
			const Coord3D *oLoc = o->getLocation();
			const Coord3D *onLoc = o->getNext()->getLocation();
			for (std::list<MapObject*>::iterator connected = connectedSegs.begin(); connected != connectedSegs.end(); ++connected)
			{
				MapObject* p = *connected;
				const Coord3D *pLoc = p->getLocation();
				const Coord3D *pnLoc = p->getNext()->getLocation();

				Real dx1 = oLoc->x - pLoc->x;
				Real dy1 = oLoc->y - pLoc->y;
				dx1 = abs(dx1);
				dy1 = abs(dy1);
				Real qd1 = max(dx1, dy1);
				//Real dist1 = sqrt(dx1*dx1+dy1*dy1);

				Real dx2 = oLoc->x - pnLoc->x;
				Real dy2 = oLoc->y - pnLoc->y;
				dx2 = abs(dx2);
				dy2 = abs(dy2);
				Real qd2 = max(dx2, dy2);
				//Real dist2 = sqrt(dx2*dx2+dy2*dy2);

				Real dx3 = onLoc->x - pLoc->x;
				Real dy3 = onLoc->y - pLoc->y;
				dx3 = abs(dx3);
				dy3 = abs(dy3);
				Real qd3 = max(dx3, dy3);
				//Real dist3 = sqrt(dx3*dx3+dy3*dy3);

				Real dx4 = onLoc->x - pnLoc->x;
				Real dy4 = onLoc->y - pnLoc->y;
				dx4 = abs(dx4);
				dy4 = abs(dy4);
				Real qd4 = max(dx4, dy4);
				//Real dist4 = sqrt(dx4*dx4+dy4*dy4);

				if (qd1 < MAP_XY_FACTOR/100 || qd2 < MAP_XY_FACTOR/100 || qd3 < MAP_XY_FACTOR/100 || qd4 < MAP_XY_FACTOR/100) {
					connectedSegs.push_back(o);
					roadSegs.erase(it);
					changed = true;
					break;
				}
			}
		}
	}

	for (std::list<MapObject*>::iterator connected = connectedSegs.begin(); connected != connectedSegs.end(); ++connected)
	{
		MapObject* p = *connected;
		if (p) {
			p->setSelected(true);
			p->getNext()->setSelected(true);
		}
	}
}

void RoadOptions::ChangeRoadType(AsciiString newRoad)
{
	SelectConnected();
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	pUndo->SetName(newRoad);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

BOOL RoadOptions::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;
	if (pHdr->hdr.hwndFrom == m_roadTreeView.m_hWnd) {
		if (pHdr->hdr.code == TVN_SELCHANGED) {
			char buffer[NAME_MAX_LEN];
			HTREEITEM hItem = m_roadTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
			item.hItem = hItem;
			item.pszText = buffer;
			item.cchTextMax = sizeof(buffer)-2;
			m_roadTreeView.GetItem(&item);
			if (item.lParam >= 0) {
				m_currentRoadIndex = item.lParam;
				m_currentRoadName = buffer;
				updateLabel();
			}	else if (!(item.state & TVIS_EXPANDEDONCE) ) {
				HTREEITEM child = m_roadTreeView.GetChildItem(hItem);
				while (child != NULL) {
					hItem = child;
					child = m_roadTreeView.GetChildItem(hItem);
				}
				if (hItem != m_roadTreeView.GetSelectedItem()) {
					m_roadTreeView.SelectItem(hItem);
					updateSelection();
				}
			}
		}
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}


void RoadOptions::OnTightCurve()
{
	m_angleCorners = false;
	m_tightCurve = true;
	applyToSelection();
}

void RoadOptions::OnAngled()
{
	m_angleCorners = true;
	m_tightCurve = false;
	applyToSelection();
}

void RoadOptions::OnBroadCurve()
{
	m_angleCorners = false;
	m_tightCurve = false;
	applyToSelection();
}

void RoadOptions::OnJoin()
{
	CButton *pButton = (CButton *)m_staticThis->GetDlgItem(IDC_JOIN);
	m_doJoin = pButton->GetCheck()==1;
	Int flagMask = FLAG_ROAD_JOIN;
	Int flagVal = 0;
	if (m_doJoin) {
		flagVal = FLAG_ROAD_JOIN;
	}
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	ModifyFlagsUndoable *pUndo = new ModifyFlagsUndoable(pDoc, flagMask, flagVal);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

void RoadOptions::OnApplyRoad()
{
	if (m_currentRoadName != AsciiString::TheEmptyString)
	{
		ChangeRoadType(m_currentRoadName);
	}
}
