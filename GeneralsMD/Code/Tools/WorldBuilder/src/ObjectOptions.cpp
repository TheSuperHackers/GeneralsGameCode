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

// ObjectOptions.cpp : implementation file
//

#define DEFINE_EDITOR_SORTING_NAMES

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "ObjectOptions.h"
#include "WHeightMapEdit.h"
#include "addplayerdialog.h"
#include "WorldBuilderDoc.h"
#include "CUndoable.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/WellKnownKeys.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"
#include "Common/PlayerTemplate.h"
#include "Common/FileSystem.h" // for LOAD_TEST_ASSETS
#include "GameLogic/SidesList.h"
#include "GameClient/Color.h"

#include "wbview3d.h"

// This is used to allow sounds to be played via PlaySound
#include <mmsystem.h>
#include <list>

#define OBJECT_OPTION_PANEL "ObjectOptionPanel"

// bool ObjectOptions::m_isObjectOptsWindowOpen  = false;
ObjectOptions *ObjectOptions::m_staticThis = NULL;
Bool ObjectOptions::m_updating = false;
char ObjectOptions::m_currentObjectName[NAME_MAX_LEN];
Int ObjectOptions::m_currentObjectIndex=-1;
AsciiString ObjectOptions::m_curOwnerName;

/////////////////////////////////////////////////////////////////////////////
// ObjectOptions dialog


ObjectOptions::ObjectOptions(CWnd* pParent /*=NULL*/)
{
	m_objectsList = NULL;
	m_objectsListModified = false;
	strcpy(m_currentObjectName, "No Selection");
	m_curOwnerName.clear();
	m_bPreviewAmbient = false;
	m_isObjectOptsWindowOpen  = false;
	//{{AFX_DATA_INIT(ObjectOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


ObjectOptions::~ObjectOptions(void)
{
	if (m_objectsList) {
		m_objectsList->deleteInstance();
	}
	m_objectsList = NULL;
}


void ObjectOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ObjectOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ObjectOptions, COptionsPanel)
	//{{AFX_MSG_MAP(ObjectOptions)
	ON_CBN_EDITCHANGE(IDC_OWNINGTEAM, OnEditchangeOwningteam)
	ON_CBN_CLOSEUP(IDC_OWNINGTEAM, OnCloseupOwningteam)
	ON_CBN_SELCHANGE(IDC_OWNINGTEAM, OnSelchangeOwningteam)
	ON_BN_CLICKED(IDC_OBJECT_SEARCH_BUTTON, OnSearch)
	ON_BN_CLICKED(IDC_OBJECT_SEARCH_RESET_BTN, OnReset)
	ON_BN_CLICKED(IDC_TOGGLE_PREVIEW_SOUND, OnPreviewAmbientSound)
	ON_BN_CLICKED(IDC_TOGGLE_PREV_FEEDBACK, OnPreviewBuildZone)
	ON_BN_CLICKED(IDC_TOGGLE_WATER_HEIGHT, OnUseWaterHeight)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void ObjectOptions::OnUseWaterHeight()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_TOGGLE_WATER_HEIGHT);
	m_bUseWaterHeight = (pButton->GetCheck() == 1);
	::AfxGetApp()->WriteProfileInt(OBJECT_OPTION_PANEL, "UseWaterHeight", m_bUseWaterHeight ? 1 : 0);
}

void ObjectOptions::OnPreviewBuildZone()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_TOGGLE_PREV_FEEDBACK);
	m_bPreviewBuildZone = (pButton->GetCheck() == 1);
	::AfxGetApp()->WriteProfileInt(OBJECT_OPTION_PANEL, "PreviewBuildZone", m_bPreviewBuildZone ? 1 : 0);

	// CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	// if (pDoc==NULL) return;
	// WbView3d *p3View = pDoc->GetActive3DView();
	// p3View->setShowBuildZoneFeedBack(m_bPreviewBuildZone);
}

void ObjectOptions::OnPreviewAmbientSound()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_TOGGLE_PREVIEW_SOUND);
	m_bPreviewAmbient = (pButton->GetCheck() == 1);
	::AfxGetApp()->WriteProfileInt(OBJECT_OPTION_PANEL, "PreviewSound", m_bPreviewAmbient ? 1 : 0);
}

/////////////////////////////////////////////////////////////////////////////
static Int findSideListEntryWithPlayerOfSide(AsciiString side)
{
	for (int i = 0; i < TheSidesList->getNumSides(); i++)
	{
		AsciiString ptname = TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerFaction);
		const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(ptname));
		if (pt && pt->getSide() == side)
		{
			return i;
		}
	}

	// DEBUG_CRASH(("no SideList entry found for %s!\n",side.str()));
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// ObjectOptions data access method.

/*static*/ void ObjectOptions::update()
{
	if (m_staticThis)
		m_staticThis->updateLabel();
}


void ObjectOptions::updateLabel()
{
	CWnd *pLabel = GetDlgItem(IDC_OBJECT_NAME);
	if (pLabel) {
		pLabel->SetWindowText(m_currentObjectName);
	}

	CComboBox *list = (CComboBox*)GetDlgItem(IDC_OWNINGTEAM);
	list->ResetContent();

	Bool found = false;
	AsciiString defTeamName;
	MapObject *pCur = getCurMapObject();
	if (!pCur) {
		// No valid selection. Just return.
		return;
	}
	if (pCur)
	{
		const ThingTemplate* tt = pCur->getThingTemplate();
		if (tt)
		{
			AsciiString defPlayerSide = tt->getDefaultOwningSide();
			Int i = findSideListEntryWithPlayerOfSide(defPlayerSide);
			if (i >= 0)
			{
				defTeamName.set("team");
				defTeamName.concat(TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName));
				found = true;
			}
		}
		else
		{
			// put it on neutral
			defTeamName.set("team");
			found = true;
		}
	}

	// flag the preview for redraw also
	if (pCur && pCur->getThingTemplate())
	{
		m_objectPreview.SetThingTemplate(pCur->getThingTemplate());
	}
	else
	{
		m_objectPreview.SetThingTemplate(NULL);
	}
	m_objectPreview.Invalidate();

	Int neutral = -1;
	Int sel = -1;
	for (int i = 0; i < TheSidesList->getNumTeams(); i++)
	{
		Dict *d = TheSidesList->getTeamInfo(i)->getDict();
		AsciiString name = d->getAsciiString(TheKey_teamName);
		DEBUG_ASSERTCRASH(!name.isEmpty(),("bad"));

		if (name == defTeamName)
			sel = i;

		if (name == "team")
		{
			name = "(neutral)";	// aka, neutral
			neutral = i;
		}
		list->AddString(name.str());
	}
	DEBUG_ASSERTCRASH(TheSidesList->getNumTeams() == 0 || neutral != -1, ("must have a neutral"));
	if (sel == -1)
	{
		DEBUG_ASSERTCRASH(defTeamName.isEmpty(), ("owning team not found, using neutral"));
		sel = neutral;
	}
	if (sel == -1)
	{
		DEBUG_ASSERTCRASH(!pCur || TheSidesList->getNumTeams()==0,("hmm, should not happen"));
		m_curOwnerName.clear();
	}
	else
	{
		DEBUG_ASSERTCRASH(pCur,("hmm, should not happen"));
		m_curOwnerName = TheSidesList->getTeamInfo(sel)->getDict()->getAsciiString(TheKey_teamName);
	}
	list->SetCurSel(sel);
}

#if 0
/////////////////////////////////////////////////////////////////////////////
static const PlayerTemplate* findFirstPlayerTemplateOnSide(AsciiString side)
{
	if (side.isEmpty())
		return NULL;	// neutral, this is ok

	for (int i = 0; i < ThePlayerTemplateStore->getPlayerTemplateCount(); i++)
	{
		const PlayerTemplate* pt = ThePlayerTemplateStore->getNthPlayerTemplate(i);
		if (pt && pt->getSide() == side)
		{
			return pt;
		}
	}

	DEBUG_CRASH(("no player found for %s!\n",side.str()));
	return NULL;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// ObjectOptions message handlers

/// Setup the controls in the dialog.
BOOL ObjectOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_updating = true;
	
//	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();


	// add entries from the thing factory as the available objects to use
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
			 tTemplate;
			 tTemplate = tTemplate->friend_getNextTemplate() )
	{
		Coord3D loc = { 0, 0, 0 };
		MapObject *pMap;

		// create new map object
		pMap = newInstance( MapObject)( loc, tTemplate->getName(), 0.0f, 0, NULL, tTemplate );
		pMap->setNextMap( m_objectsList );
		m_objectsList = pMap;
			
		// get display color for the editor
		Color cc = tTemplate->getDisplayColor();
		pMap->setColor(cc);

	}  // end for tTemplate

#if 0		// Lights are not working right now.
	{
		Coord3D pt = {0,0,0};
		char base[1024] = "*Lights/Light";
		MapObject *pMap = newInstance(MapObject)(pt, AsciiString(base), 0.0f, 0, NULL, NULL );
		pMap->setIsLight();
		
		Dict *props = pMap->getProperties();
		RGBColor tmp;
		tmp.red = tmp.green = tmp.blue = 1.0f;		
		Int tmpi = tmp.getAsInt();
		props->setReal(TheKey_lightHeightAboveTerrain, 0.0f);
		props->setReal(TheKey_lightInnerRadius, 15.0f);
		props->setReal(TheKey_lightOuterRadius, 25.0f);
		props->setInt(TheKey_lightAmbientColor, tmpi);
		props->setInt(TheKey_lightDiffuseColor, tmpi);

		pMap->setNextMap(m_objectsList);
		m_objectsList = pMap;
	}
#endif 
#ifdef LOAD_TEST_ASSETS
	{
		char				dirBuf[_MAX_PATH];
		char				findBuf[_MAX_PATH];
		char				fileBuf[_MAX_PATH];
		Int					i;

		strcpy(dirBuf, TEST_W3D_DIR_PATH);
		int len = strlen(dirBuf);

		if (len > 0 && dirBuf[len - 1] != '\\' && dirBuf[len-1] != '/') {
			dirBuf[len++] = '\\';
			dirBuf[len] = 0;
		}
		strcpy(findBuf, dirBuf);
		strcat(findBuf, "*.*");

		FilenameList filenameList;
		TheFileSystem->getFileListInDirectory(AsciiString(dirBuf), AsciiString("*.w3d"), filenameList, FALSE);

		if (filenameList.size() > 0) {
			FilenameList::iterator it = filenameList.begin();
			do {
				AsciiString filename = *it;
				len = filename.getLength();
				if (len<5) continue;
				// only do .w3d files

				// strip out the path, and just keep the filename itself.
				AsciiString token;
				while (filename.getLength() > 0) {
					filename.nextToken(&token, "\\/");
				}

				strcpy(fileBuf, TEST_STRING);
				strcat(fileBuf, "/");
				strcat(fileBuf, token.str());
				for (i=strlen(fileBuf)-1; i>0; i--) {
					if (fileBuf[i] == '.') {
						// strip off .w3d file extension.
						fileBuf[i] = 0;
						break; // we stripped it already, we don't need to go any further.
					}
				}
				Coord3D pt = {0,0,0};
				MapObject *pMap = newInstance(MapObject)(pt, AsciiString(fileBuf), 0.0f, 0, NULL, NULL );
				pMap->setNextMap(m_objectsList);
				m_objectsList = pMap;

				++it;
			} while (it != filenameList.end());
 		}
	}
#endif

	CWnd *pWnd = GetDlgItem(IDC_OBJECT_HEIGHT_EDIT);
	if (pWnd) {
		CString s;
		s.Format("%d",MAGIC_GROUND_Z);
		pWnd->SetWindowText(s);
	}

	pWnd = GetDlgItem(IDC_TERRAIN_TREEVIEW);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_objectTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, rect, this, IDC_TERRAIN_TREEVIEW);
	m_objectTreeView.ShowWindow(SW_SHOW);

	pWnd = GetDlgItem(IDC_TERRAIN_SWATCHES);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_objectPreview.Create(NULL, "", WS_CHILD, rect, this, IDC_TERRAIN_SWATCHES);
	m_objectPreview.ShowWindow(SW_SHOW);

	MapObject *pMap =  m_objectsList;
	Int index = 0;
	while (pMap) {
		addObject( pMap, pMap->getName().str(), index, TVI_ROOT);
		index++;
		pMap = pMap->getNext();
	}

	CButton *pButton = (CButton*)GetDlgItem(IDC_TOGGLE_PREVIEW_SOUND);
	m_bPreviewAmbient=::AfxGetApp()->GetProfileInt(OBJECT_OPTION_PANEL, "PreviewSound", 1);
	pButton->SetCheck(m_bPreviewAmbient ? 1:0);
	OnPreviewAmbientSound();

	pButton = (CButton*)GetDlgItem(IDC_TOGGLE_PREV_FEEDBACK);
	m_bPreviewBuildZone=::AfxGetApp()->GetProfileInt(OBJECT_OPTION_PANEL, "PreviewBuildZone", 0);
	pButton->SetCheck(m_bPreviewBuildZone ? 1:0);
	OnPreviewBuildZone();

	pButton = (CButton*)GetDlgItem(IDC_TOGGLE_WATER_HEIGHT);
	m_bUseWaterHeight=::AfxGetApp()->GetProfileInt(OBJECT_OPTION_PANEL, "UseWaterHeight", 0);
	pButton->SetCheck(m_bUseWaterHeight ? 1:0);
	OnUseWaterHeight();

	m_staticThis = this;
	m_updating = false;

	updateLabel();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM ObjectOptions::findOrAdd(HTREEITEM parent, const char *pLabel)
{
	TVINSERTSTRUCT ins;
	char buffer[_MAX_PATH];
	::memset(&ins, 0, sizeof(ins));
	HTREEITEM child = m_objectTreeView.GetChildItem(parent);
	while (child != NULL) {
		ins.item.mask = TVIF_HANDLE|TVIF_TEXT;
		ins.item.hItem = child;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = sizeof(buffer)-2;				
		m_objectTreeView.GetItem(&ins.item);
		if (strcmp(buffer, pLabel) == 0) {
			return(child);
		}
		child = m_objectTreeView.GetNextSiblingItem(child);
	}

	// not found, so add it.
	::memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_SORT;
	ins.item.mask = TVIF_PARAM|TVIF_TEXT;
	ins.item.lParam = -1;
	ins.item.pszText = (char*)pLabel;
	ins.item.cchTextMax = strlen(pLabel);				
	child = m_objectTreeView.InsertItem(&ins);
	return(child);
}

HTREEITEM ObjectOptions::findOrDont(const char *pLabel)
{
	return _FindOrDont(pLabel, m_objectTreeView.GetRootItem());
}

HTREEITEM ObjectOptions::_FindOrDont(const char* pLabel, HTREEITEM startPoint)
{
	std::list<HTREEITEM> itemsToEx;
	itemsToEx.push_back(startPoint);
	
	while (itemsToEx.front()) {
		char buffer[_MAX_PATH];
		HTREEITEM hItem = itemsToEx.front();
		itemsToEx.pop_front();

		if (!m_objectTreeView.ItemHasChildren(hItem)) {
			TVITEM item;
			memset(&item, 0, sizeof(item));

			item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
			item.hItem = hItem;
			item.pszText = buffer;
			item.cchTextMax = sizeof(buffer)-2;				
			m_objectTreeView.GetItem(&item);

			const char* strToTest = strrchr(pLabel, '/');
//		if (strstr((strToTest ? strToTest : pLabel), buffer)) 
			if (strcmp((strToTest ? strToTest : pLabel), buffer) == 0) 
			{
				return hItem;
			}
		} else {
			// add the first child, the others will be caught by the adding of the siblings
			itemsToEx.push_back(m_objectTreeView.GetChildItem(hItem));
		} // Always add the first sibling, if it exists

		if (m_objectTreeView.GetNextSiblingItem(hItem)) {
			itemsToEx.push_back(m_objectTreeView.GetNextSiblingItem(hItem));
		}
	}
	return NULL;
}


//-------------------------------------------------------------------------------------------------
/** Add the object hierarchy paths to the tree view. */
//-------------------------------------------------------------------------------------------------
void ObjectOptions::addObject( MapObject *mapObject, const char *pPath, 
															 Int terrainNdx, HTREEITEM parent )
{
	char buffer[ _MAX_PATH ];
	const char *leafName = NULL;

	// sanity
	if( mapObject == NULL )
		return;

	//
	// if we have an thing template in mapObject, we've read it from the new INI database,
	// we will sort those items into the tree based on properties of the template that
	// make it easier for us to browse when building levels 
	//
	// Feel free to reorganize how this tree is constructed from the template
	// data at will, whatever makes it easier for design
	//
	const ThingTemplate *thingTemplate = mapObject->getThingTemplate();
	if( thingTemplate )
	{

		// first check for test sorted objects
		if( thingTemplate->getEditorSorting() == ES_TEST )
			parent = findOrAdd( parent, "TEST" );
	
		// first sort by side, either create or find the tree item with matching side name
		AsciiString side = thingTemplate->getDefaultOwningSide();
		DEBUG_ASSERTCRASH( !side.isEmpty(), ("NULL default side in template\n") );
		strcpy( buffer, side.str() );
		parent = findOrAdd( parent, buffer );

		// next tier uses the editor sorting that design can specify in the INI
		EditorSortingType i = ES_FIRST;
		for( ;
				 i < ES_NUM_SORTING_TYPES;
				 i = (EditorSortingType)(i + 1) )
		{

			if( thingTemplate->getEditorSorting() == i )
			{

				parent = findOrAdd( parent, EditorSortingNames[ i ] );
				break;  // exit for

			}  // end if

		}  // end for i

		if( i == ES_NUM_SORTING_TYPES )
			parent = findOrAdd( parent, "UNSORTED" );

		// the leaf name is the name of the template
		leafName = thingTemplate->getName().str();

	}  // end if
	else 
	{
	
		// all these old entries we will put in a tree for legacy GDF items
		parent = findOrAdd( parent, "**TEST MODELS" );

		Int i=0;
		leafName = pPath;
		while (pPath[i] && i<sizeof(buffer)) {
			if (pPath[i] == 0) {
				return;
			}
			if (pPath[i] == '/') {
				pPath+= i+1;
				i = 0;			
			}
			buffer[i] = pPath[i];
			i++;
		}

		if( i > 0 )
		{
			buffer[ i ] = 0;
			leafName = buffer;

		}  // end if

	}  // end else if

	// add to the tree view
	if( leafName )
	{
		TVINSERTSTRUCT ins;

		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_SORT;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = terrainNdx;
		ins.item.pszText = (char*)leafName;
		ins.item.cchTextMax = strlen(leafName)+2;				
		m_objectTreeView.InsertItem(&ins);

	}

}

/// Set the selected object in the tree view.
Bool ObjectOptions::setObjectTreeViewSelection(HTREEITEM parent, Int selection)
{
	TVITEM item;
	char buffer[NAME_MAX_LEN];
	::memset(&item, 0, sizeof(item));
	HTREEITEM child = m_objectTreeView.GetChildItem(parent);
	while (child != NULL) {
		item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
		item.hItem = child;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer)-2;				
		m_objectTreeView.GetItem(&item);
		if (item.lParam == selection) {
			// m_isObjectOptsWindowOpen  = true;
			m_objectTreeView.SelectItem(child);
			// m_isObjectOptsWindowOpen  = false;
			return(true);
		}
		if (setObjectTreeViewSelection(child, selection)) 
		{
			strcpy(m_currentObjectName, buffer);
			updateLabel();
			return(true);
		}
		child = m_objectTreeView.GetNextSiblingItem(child);
	}
	return(false);
}


BOOL ObjectOptions::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;
	if (pHdr->hdr.hwndFrom == m_objectTreeView.m_hWnd) {

		if (pHdr->hdr.code == TVN_ITEMEXPANDED) {
			if (pHdr->action == TVE_COLLAPSE) {
				TVITEM item;
				::memset(&item, 0, sizeof(item));
				item.mask = TVIF_STATE;
				item.hItem = pHdr->itemOld.hItem;
				m_objectTreeView.GetItem(&item);
				item.state &= (~TVIS_EXPANDEDONCE);
				item.mask = TVIF_STATE;
				m_objectTreeView.SetItem(&item);
			}
		}
		if (pHdr->hdr.code == TVN_SELCHANGED) {
			char buffer[NAME_MAX_LEN];
			HTREEITEM hItem = m_objectTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
			item.hItem = hItem;
			item.pszText = buffer;
			item.cchTextMax = sizeof(buffer)-2;				
			m_objectTreeView.GetItem(&item);
			if (item.lParam >= 0) {
				m_currentObjectIndex = item.lParam;
				strcpy(m_currentObjectName, buffer);
			
				MapObject* obj = getCurMapObject();
				if (obj && m_isObjectOptsWindowOpen  && m_bPreviewAmbient) {
					const ThingTemplate* thingTemplate = obj->getThingTemplate();
					if (thingTemplate) {
						const AudioEventRTS* event = thingTemplate->getSoundAmbient();
			
						if (event) {
							const AudioEventInfo* audioInfo = event->getAudioEventInfo();
			
							if (!audioInfo && TheAudio) {
								audioInfo = TheAudio->findAudioEventInfo(event->getEventName());
							}
			
							if (audioInfo) {
								AudioEventRTS eventToPlay;
								eventToPlay.setEventName(event->getEventName());
								eventToPlay.setAudioEventInfo(audioInfo);
								eventToPlay.generateFilename();

								DEBUG_LOG(("String Event %s\n",eventToPlay.getFilename().str()));
			
								if (!eventToPlay.getFilename().isEmpty()) {
									PlaySound(eventToPlay.getFilename().str(), NULL, SND_ASYNC | SND_FILENAME | SND_PURGE);
								}
							}
						}
					}
				}
			} else if (m_objectTreeView.ItemHasChildren(item.hItem)) {
				strcpy(m_currentObjectName, "No Selection");
				m_currentObjectIndex = -1;
			}
			updateLabel();
		}
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}


MapObject *ObjectOptions::getCurMapObject(void)
{
	if (m_staticThis && m_currentObjectIndex >= 0) {
		MapObject *pObj = m_staticThis->m_objectsList;
		int count = 0;
		while (pObj) {
			if (count == m_currentObjectIndex) {
				return(pObj);
			}
			count++;
			pObj = pObj->getNext();
		}
	}
	return(NULL);
}

AsciiString ObjectOptions::getCurGdfName(void)
{
	MapObject *pCur = getCurMapObject();
	if (pCur) {
		return pCur->getName();
	}
	return AsciiString::TheEmptyString;
}

MapObject *ObjectOptions::duplicateCurMapObjectForPlace(const Coord3D* loc, Real angle, Bool checkPlayers)
{
	MapObject *pCur = getCurMapObject();
	if (pCur)
	{
		
		Bool found = false;
		const ThingTemplate* tt = pCur->getThingTemplate();
		if (checkPlayers) 
		{
			AsciiString defaultTeam("team");
			
			AsciiString objectTeamName = m_curOwnerName;
			if (objectTeamName != defaultTeam) {
				TeamsInfo *teamInfo = TheSidesList->findTeamInfo(objectTeamName);
				if (teamInfo) {
					AsciiString teamOwner = teamInfo->getDict()->getAsciiString(TheKey_teamOwner);
					SidesInfo* pSide = TheSidesList->findSideInfo(teamOwner);
					if (pSide) {
						found = true;
					}
				}
			}
			if (!found && tt)
			{
				AsciiString defPlayerSide = tt->getDefaultOwningSide();
				Int si = findSideListEntryWithPlayerOfSide(defPlayerSide);

				found = (si >= 0);
				if (!found)
				{
					AddPlayerDialog addPlyr(pCur->getThingTemplate()->getDefaultOwningSide());
					if (addPlyr.DoModal() == IDOK) 
					{
						for (int i = 0; i < TheSidesList->getNumSides(); i++)
						{
							AsciiString playerTmplName = TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerFaction);
							if (playerTmplName == addPlyr.getAddedSide())
							{
								m_curOwnerName.set("team");
								m_curOwnerName.concat(TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName));
								found = true;
								break;
							}
						}
					}
				}
			}
			else
			{
				if (!found) {
					// neutral
					m_curOwnerName.set("team");
					found = true;
				}
			}
		} else {
			found = true;
		}
		if (found)
		{
			MapObject *pNew = newInstance(MapObject)( *loc, pCur->getName(), angle, 
																			 pCur->getFlags(), pCur->getProperties(), 
																			 pCur->getThingTemplate() );
			pNew->getProperties()->setAsciiString(TheKey_originalOwner, m_curOwnerName);
			pNew->setColor(pCur->getColor());
			return pNew;
		}
	}
	AfxMessageBox("Unable to add object.");
	return(NULL);
}

Real ObjectOptions::getCurObjectHeight(void)
{
	if (m_staticThis) {
		CWnd *pWnd = m_staticThis->GetDlgItem(IDC_OBJECT_HEIGHT_EDIT);
		if (pWnd) {
			CString val;
			pWnd->GetWindowText(val);
			Real height = atoi(val);
			if (height>0) {
				height *= MAP_HEIGHT_SCALE;
			}
			return(height);
		}
	}
	return(MAGIC_GROUND_Z);
}

MapObject *ObjectOptions::getObjectNamed(AsciiString name)
{
	if (m_staticThis) {
		MapObject *pObj = m_staticThis->m_objectsList;
//		int count = 0;
		while (pObj) {
			if (name == pObj->getName()) {
				return(pObj);
			}
			const char *curName = pObj->getName().str();
			const char *leaf = curName;
			while (*curName) {
				if (*curName == '/') {
					leaf = curName+1;
				}
				curName++;
			}
			if (0==strcmp(leaf, name.str())) {
				return(pObj);
			}
			pObj = pObj->getNext();
		}
	}
	return(NULL);
}

Int ObjectOptions::getObjectNamedIndex(const AsciiString& name)
{
	if (m_staticThis) {
		MapObject *pObj = m_staticThis->m_objectsList;
		int count = 0;
		while (pObj) {
			if (name == pObj->getName()) {
				return(count);
			}
			const char *curName = pObj->getName().str();
			const char *leaf = curName;
			while (*curName) {
				if (*curName == '/') {
					leaf = curName+1;
				}
				curName++;
			}
			if (0==strcmp(leaf, name.str())) {
				return(count);
			}
			pObj = pObj->getNext();
		}
	}
	return(NULL);
}

void ObjectOptions::OnOK()
{
    OnSearch(); 
}

// Adriane [Deathscythe] : This function is mainly used for loading new objects from map.ini
void ObjectOptions::reprocessObjectList()
{
	if (!m_staticThis)
		return;

	// Clear current tree
	m_staticThis->m_objectTreeView.DeleteAllItems(); 
	m_staticThis->m_objectsList = NULL;
	
	const ThingTemplate *tTemplate;
	for (tTemplate = TheThingFactory->firstTemplate();
		tTemplate;
		tTemplate = tTemplate->friend_getNextTemplate())
	{
		Coord3D loc = { 0, 0, 0 };
		MapObject *pMap;

		DEBUG_LOG(("Adding Object to ObjectList: '%s'\n", tTemplate->getName().str()));

		// create new map object
		pMap = newInstance(MapObject)(loc, tTemplate->getName(), 0.0f, 0, NULL, tTemplate);
		pMap->setNextMap(m_staticThis->m_objectsList);
		m_staticThis->m_objectsList = pMap;

		// get display color for the editor
		Color cc = tTemplate->getDisplayColor();
		pMap->setColor(cc);
	}

	// Repopulate list
	MapObject* pMap = m_staticThis->m_objectsList;
	Int index = 0;
	while (pMap)
	{
		m_staticThis->addObject(pMap, pMap->getName().str(), index, TVI_ROOT);
		index++;
		pMap = pMap->getNext();
	}
}

// Add the function that handles the search button click
void ObjectOptions::OnReset()
{
	m_objectTreeView.DeleteAllItems(); // Clear current tree

	// Repopulate list
	MapObject* pMap = m_objectsList;
	Int index = 0;
	while (pMap)
	{
		addObject(pMap, pMap->getName().str(), index, TVI_ROOT);
		index++;
		pMap = pMap->getNext();
	}

	m_objectsListModified = false;
}

// Add the function that handles the search button click
void ObjectOptions::OnSearch()
{
    UpdateData(TRUE);

    CString searchText;
    GetDlgItemText(IDC_OBJECT_SEARCH_EDIT, searchText);
    searchText.MakeLower();

    m_objectTreeView.DeleteAllItems(); // Clear current tree

    if (searchText.IsEmpty())
    {
		::MessageBeep(MB_ICONEXCLAMATION);
        // Repopulate full list if search is empty
        MapObject* pMap = m_objectsList;
		Int index = 0;
		while (pMap)
		{
			addObject(pMap, pMap->getName().str(), index, TVI_ROOT);
			index++;
			pMap = pMap->getNext();
		}

		m_objectsListModified = false;

        return;
    }

    MapObject* pMap = m_objectsList;
    int index = 0;
    int matchCount = 0;

    while (pMap)
    {
        CString name = pMap->getName().str();
        CString lowerName = name;
        lowerName.MakeLower();

        if (lowerName.Find(searchText) != -1)
        {
            addObject(pMap, name, index, TVI_ROOT);
            matchCount++;
        }

        pMap = pMap->getNext();
        index++;
    }

    if (matchCount == 0)
    {
        MessageBox("No matches found.", "Search", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        // Expand all items in the tree
        HTREEITEM hRoot = m_objectTreeView.GetRootItem();
        if (hRoot)
        {
            ExpandAllItems(m_objectTreeView, hRoot);
        }

		m_objectsListModified = true;
    }
}


void ObjectOptions::ExpandAllItems(CTreeCtrl& treeCtrl, HTREEITEM hItem)
{
    while (hItem)
    {
        treeCtrl.Expand(hItem, TVE_EXPAND);
        HTREEITEM hChild = treeCtrl.GetChildItem(hItem);
        if (hChild)
            ExpandAllItems(treeCtrl, hChild);

        hItem = treeCtrl.GetNextSiblingItem(hItem);
    }
}

// Old Logic -- Unused
// HTREEITEM ObjectOptions::getNextItem(HTREEITEM hItem)
// {
//     if (m_objectTreeView.GetChildItem(hItem))
//         return m_objectTreeView.GetChildItem(hItem);

//     while (hItem)
//     {
//         if (m_objectTreeView.GetNextSiblingItem(hItem))
//             return m_objectTreeView.GetNextSiblingItem(hItem);

//         hItem = m_objectTreeView.GetParentItem(hItem);
//     }

//     return NULL;
// }

void ObjectOptions::OnEditchangeOwningteam() 
{
	CComboBox *list = (CComboBox*)GetDlgItem(IDC_OWNINGTEAM);
	Int sel = list->GetCurSel();

	m_curOwnerName.clear();
	if (sel >= 0)
	{
		// note, get playername from the dicts, NOT from the popup.
		Dict *d = TheSidesList->getTeamInfo(sel)->getDict();
		m_curOwnerName = d->getAsciiString(TheKey_teamName);
	}
	// no, do NOT call this; it'll reset back to default
	//updateLabel();
}

void ObjectOptions::selectObject(const MapObject* pObj)
{
	if (m_staticThis) {

		char buffer[NAME_MAX_LEN];
		HTREEITEM objToSel = m_staticThis->findOrDont(pObj->getName().str());
		if (objToSel == NULL) {
			return;
		}

		TVITEM item;
		::memset(&item, 0, sizeof(item));
		item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
		item.hItem = objToSel;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer)-2;				
		m_staticThis->m_objectTreeView.GetItem(&item);

		if (m_staticThis->m_objectTreeView.SelectItem(objToSel)) {
			m_staticThis->m_currentObjectIndex = item.lParam;
			strcpy(m_staticThis->m_currentObjectName, buffer);
		}
	}
}

void ObjectOptions::OnCloseupOwningteam() 
{
	OnEditchangeOwningteam();
}

void ObjectOptions::OnSelchangeOwningteam() 
{
	OnEditchangeOwningteam();
}

void ObjectOptions::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	/**
	 * Adriane [Deathscythe]
	 * Call reset when we modify the object list from a search and the dialog is about to commit seppuku.
	 * For some shitty reason, the object list here is tied to the Fence Tool — make sure to reset it.
	 */
	m_isObjectOptsWindowOpen = bShow;

	if (!bShow && m_objectsListModified)
	{
		OnReset();
	}
}