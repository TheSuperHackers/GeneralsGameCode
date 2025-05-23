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

// PickUnitDialog.cpp : implementation file
//

#define DEFINE_EDITOR_SORTING_NAMES

#include "StdAfx.h"
#include "resource.h"
#include "PickUnitDialog.h"

#include "Common/ThingTemplate.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"

/////////////////////////////////////////////////////////////////////////////
// PickUnitDialog dialog


ReplaceUnitDialog::ReplaceUnitDialog(CWnd* pParent /*=NULL*/)
	: PickUnitDialog(IDD, pParent)
{
	m_objectsList = NULL;
	m_currentObjectIndex = -1;
	m_currentObjectName[0] = 0;
	for (int i = ES_FIRST; i<ES_NUM_SORTING_TYPES; i++)	{
		m_allowable[i] = false;
	}
	//{{AFX_DATA_INIT(PickUnitDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BOOL ReplaceUnitDialog::OnInitDialog() 
{
	PickUnitDialog::OnInitDialog();
	CWnd *pWnd = GetDlgItem(IDC_MISSINGLABEL);
	if (pWnd) 
		pWnd->SetWindowText(m_missingName.str());

	return TRUE;
}

BEGIN_MESSAGE_MAP(ReplaceUnitDialog, CDialog)
	//{{AFX_MSG_MAP(ReplaceUnitDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

PickUnitDialog::PickUnitDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	m_objectsList = NULL;
	m_currentObjectIndex = -1;
	m_currentObjectName[0] = 0;
	for (int i = ES_FIRST; i<ES_NUM_SORTING_TYPES; i++)	{
		m_allowable[i] = false;
	}
	//{{AFX_DATA_INIT(PickUnitDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

PickUnitDialog::PickUnitDialog(UINT id, CWnd* pParent /*=NULL*/)
	: CDialog(id, pParent)
{
	m_objectsList = NULL;
	m_currentObjectIndex = -1;
	m_currentObjectName[0] = 0;
	for (int i = ES_FIRST; i<ES_NUM_SORTING_TYPES; i++)	{
		m_allowable[i] = false;
	}
	//{{AFX_DATA_INIT(PickUnitDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

PickUnitDialog::~PickUnitDialog()
{
	if (m_objectsList) {
		deleteInstance(m_objectsList);
	}
	m_objectsList = NULL;
}

void PickUnitDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PickUnitDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PickUnitDialog, CDialog)
	//{{AFX_MSG_MAP(PickUnitDialog)
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void PickUnitDialog::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(BUILD_PICK_PANEL_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(BUILD_PICK_PANEL_SECTION, "Left", frameRect.left);
	}
	
}

Bool PickUnitDialog::IsAllowableType(EditorSortingType sort, Bool isBuildable)
{
	if (m_factionOnly && !isBuildable) 
	{
		return false;
	}
	return (m_allowable[sort]);
}

void PickUnitDialog::SetAllowableType(EditorSortingType sort)
{
	m_allowable[sort] = true;
}

void PickUnitDialog::SetupAsPanel(void)
{
	CWnd *pWnd = GetDlgItem(IDCANCEL);
	if (pWnd) {
		pWnd->ShowWindow(SW_HIDE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// PickUnitDialog message handlers

BOOL PickUnitDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();

	// add entries from the thing factory as the available objects to use
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
			 tTemplate;
			 tTemplate = tTemplate->friend_getNextTemplate() )
	{
		Coord3D loc = { 0, 0, 0 };
		MapObject *pMap;

		EditorSortingType sort = tTemplate->getEditorSorting();
		if (!IsAllowableType(sort, tTemplate->isBuildableItem())) continue;

		// create new map object
		pMap = newInstance(MapObject)( loc, tTemplate->getName(), 0.0f, 0, NULL, tTemplate );
		pMap->setNextMap( m_objectsList );
		m_objectsList = pMap;
	}  // end for tTemplate

	CWnd *pWnd = GetDlgItem(IDC_OBJECT_HEIGHT_EDIT);
	if (pWnd) {
		CString s;
		s.Format("%d",MAGIC_GROUND_Z);
		pWnd->SetWindowText(s);
	}

	pWnd = GetDlgItem(IDC_OBJECT_TREEVIEW);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_objectTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, rect, this, IDC_TERRAIN_TREEVIEW);
	m_objectTreeView.ShowWindow(SW_SHOW);

	MapObject *pMap =  m_objectsList;
	Int index = 0;
	while (pMap) {
		addObject( pMap, pMap->getName().str(), index, TVI_ROOT);
		index++;
		pMap = pMap->getNext();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM PickUnitDialog::findOrAdd(HTREEITEM parent, const char *pLabel)
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

BOOL PickUnitDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
			}	else if (m_objectTreeView.ItemHasChildren(item.hItem)) {
				strcpy(m_currentObjectName, "");
				m_currentObjectIndex = -1;
			}
		}
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

//-------------------------------------------------------------------------------------------------
/** Add the object hierarchy paths to the tree view. */
//-------------------------------------------------------------------------------------------------
void PickUnitDialog::addObject( MapObject *mapObject, const char *pPath, Int index, HTREEITEM parent )
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
		ins.item.lParam = index;
		ins.item.pszText = (char*)leafName;
		ins.item.cchTextMax = strlen(leafName)+2;				
		m_objectTreeView.InsertItem(&ins);

	}

}

AsciiString PickUnitDialog::getPickedUnit(void)
{
	if (m_currentObjectIndex >= 0) {
		AsciiString retval(m_currentObjectName);
		return retval;
	}
	return AsciiString::TheEmptyString;
}

const ThingTemplate* PickUnitDialog::getPickedThing(void)
{
	if (m_currentObjectIndex >= 0) {
		const ThingTemplate *tTemplate;
		for( tTemplate = TheThingFactory->firstTemplate();
			tTemplate;
			tTemplate = tTemplate->friend_getNextTemplate() )
		{
			if (m_currentObjectName == tTemplate->getName())
				return tTemplate;
		}
	}
	return NULL;
}
