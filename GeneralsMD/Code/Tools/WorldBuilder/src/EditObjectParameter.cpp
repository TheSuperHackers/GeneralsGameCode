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

// EditObjectParameter.cpp : implementation file
//

#define DEFINE_EDITOR_SORTING_NAMES

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "EditObjectParameter.h"
#include "EditParameter.h"


#include "GameLogic/Scripts.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/PolygonTrigger.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"

/////////////////////////////////////////////////////////////////////////////
// EditObjectParameter dialog


EditObjectParameter::EditObjectParameter(CWnd* pParent /*=nullptr*/)
	: CDialog(EditObjectParameter::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditObjectParameter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EditObjectParameter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditObjectParameter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditObjectParameter, CDialog)
	//{{AFX_MSG_MAP(EditObjectParameter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditObjectParameter message handlers

BOOL EditObjectParameter::OnInitDialog()
{
	CDialog::OnInitDialog();


	CWnd *pWnd = GetDlgItem(IDC_OBJECT_TREEVIEW);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_objectTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, rect, this, IDC_TERRAIN_TREEVIEW);
	m_objectTreeView.ShowWindow(SW_SHOW);

	// add entries from the thing factory as the available objects to use
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
			 tTemplate;
			 tTemplate = tTemplate->friend_getNextTemplate() )
	{
		addObject(tTemplate);

	}


	addObjectLists();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//-------------------------------------------------------------------------------------------------
/** Add the object hierarchy paths to the tree view. */
//-------------------------------------------------------------------------------------------------
void EditObjectParameter::addObject( const ThingTemplate *thingTemplate  )
{
	HTREEITEM parent = TVI_ROOT;
	const char *leafName;
	//
	// if we have an thing template in mapObject, we've read it from the new INI database,
	// we will sort those items into the tree based on properties of the template that
	// make it easier for us to browse when building levels
	//
	// Feel free to reorganize how this tree is constructed from the template
	// data at will, whatever makes it easier for design
	//
	if( thingTemplate )
	{

		// first check for test sorted objects
		if( thingTemplate->getEditorSorting() == ES_TEST )
			parent = findOrAdd( parent, "TEST" );

		// first sort by Side, either create or find the tree item with matching side name
		AsciiString side = thingTemplate->getDefaultOwningSide();
		DEBUG_ASSERTCRASH(!side.isEmpty(), ("null default side in template") );
		parent = findOrAdd( parent, side.str());

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

			}

		}

		if( i == ES_NUM_SORTING_TYPES )
			parent = findOrAdd( parent, "UNSORTED" );

		// the leaf name is the name of the template
		leafName = thingTemplate->getName().str();

	}

	// add to the tree view
	if( leafName )
	{
		TVINSERTSTRUCT ins;

		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_SORT;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = 0;
		ins.item.pszText = (char*)leafName;
		ins.item.cchTextMax = strlen(leafName)+2;
		m_objectTreeView.InsertItem(&ins);

	}

}

/**

*/
void EditObjectParameter::addObjectLists( )
{
	HTREEITEM parent = TVI_ROOT;
	const char *leafName;

	parent = findOrAdd(parent, "Object Lists");

	std::vector<AsciiString> strings;
	EditParameter::loadObjectTypeList(nullptr, &strings);

	Int numItems = strings.size();

	for (Int i = 0; i < numItems; ++i) {
		// add to the tree view

		leafName = strings[i].str();

		if( leafName )
		{
			TVINSERTSTRUCT ins;

			::memset(&ins, 0, sizeof(ins));
			ins.hParent = parent;
			ins.hInsertAfter = TVI_SORT;
			ins.item.mask = TVIF_PARAM|TVIF_TEXT;
			ins.item.lParam = 0;
			ins.item.pszText = (char*)leafName;
			ins.item.cchTextMax = strlen(leafName)+2;
			m_objectTreeView.InsertItem(&ins);
		}
	}
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM EditObjectParameter::findOrAdd(HTREEITEM parent, const char *pLabel)
{
	TVINSERTSTRUCT ins;
	char buffer[_MAX_PATH];
	::memset(&ins, 0, sizeof(ins));
	HTREEITEM child = m_objectTreeView.GetChildItem(parent);
	while (child != nullptr) {
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

BOOL EditObjectParameter::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;

	// Handle events from the tree control.
	if (pHdr->hdr.idFrom == IDC_TERRAIN_TREEVIEW) {
		if (pHdr->hdr.code == TVN_KEYDOWN) {
			NMTVKEYDOWN	*pKey = (NMTVKEYDOWN*)lParam;
			Int key = pKey->wVKey;
			if (key==VK_SHIFT || key==VK_SPACE) {
				HTREEITEM hItem = m_objectTreeView.GetSelectedItem();
				if (!m_objectTreeView.ItemHasChildren(hItem)) {
					hItem = m_objectTreeView.GetParentItem(hItem);
				}
				m_objectTreeView.Expand(hItem, TVE_TOGGLE);
				return 0;
			}
			return 0;
		}
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void EditObjectParameter::OnOK()
{
	char buffer[_MAX_PATH];
	HTREEITEM hItem = m_objectTreeView.GetSelectedItem();
	if (!hItem) {
		::MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	TVITEM item;
	::memset(&item, 0, sizeof(item));
	item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
	item.hItem = hItem;
	item.pszText = buffer;
	item.cchTextMax = sizeof(buffer)-2;
	m_objectTreeView.GetItem(&item);
	AsciiString objName = buffer;
	// We used to try to find the TT here, but now we don't because we
	// use object lists as well.
	m_parameter->friend_setString(objName);
	CDialog::OnOK();
}

void EditObjectParameter::OnCancel()
{

	CDialog::OnCancel();
}
