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

// FenceOptions.cpp : implementation file
//

/** The fence options is essentially a front for the object options panel.
The	fence options panel has a subset of the objects available, and when one is 
selected, makes the current object in the object options panel match this object.
Then the new object is created by the object options panel, so team parenting and 
so forth is all handled in the object options panel.  jba. */

#define DEFINE_EDITOR_SORTING_NAMES

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "FenceOptions.h"
#include "ObjectOptions.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "CUndoable.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/WellKnownKeys.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"
#include "GameLogic/SidesList.h"
#include "GameClient/Color.h"

#include <list>

FenceOptions *FenceOptions::m_staticThis = NULL;
Bool FenceOptions::m_updating = false;
Int FenceOptions::m_currentObjectIndex=-1;
Real FenceOptions::m_fenceSpacing=1;
Real FenceOptions::m_fenceOffset=0;
// static Bool g_freshWarning_fenceopts = true;
										 
/////////////////////////////////////////////////////////////////////////////
// FenceOptions dialog


FenceOptions::FenceOptions(CWnd* pParent /*=NULL*/)
{
	m_objectsList = NULL;
	m_customSpacing = false;
	m_showAllObjectTypes = false;
	//{{AFX_DATA_INIT(FenceOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


FenceOptions::~FenceOptions(void)
{
	if (m_objectsList) {
		m_objectsList->deleteInstance();
	}
	m_objectsList = NULL;
}


void FenceOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FenceOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(FenceOptions, COptionsPanel)
	//{{AFX_MSG_MAP(FenceOptions)
	ON_EN_CHANGE(IDC_FENCE_SPACING_EDIT, OnChangeFenceSpacingEdit)
	ON_BN_CLICKED(IDC_OBJECT_SEARCH_BUTTON, OnSearch)
	ON_BN_CLICKED(IDC_OBJECT_SEARCH_RESET_BTN, OnReset)
	ON_BN_CLICKED(IDC_FENCE_ONLY, OnCheckFenceOnly)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FenceOptions data access method.

/*static*/ void FenceOptions::update()
{
	if (m_staticThis) {
		m_staticThis->updateObjectOptions();
	}
}

void FenceOptions::updateObjectOptions()
{
	MapObject *pObj = m_objectsList;
	if (m_currentObjectIndex >= 0) {
		int count = 0;
		int visibleIndex = -1; // Track index of visible fence objects
		while (pObj) {
			if (!m_showAllObjectTypes && pObj->getThingTemplate()->getFenceWidth() == 0) {
				// Skip non-fence objects when not showing all types
				pObj = pObj->getNext();
				continue;
			}
			
			// If the current object is visible, update the index
			if (count == m_currentObjectIndex) {
				visibleIndex = count;
				ObjectOptions::selectObject(pObj);
				const ThingTemplate *t = pObj->getThingTemplate();
				if (t && !m_customSpacing) {
					Real fenceWidth = t->getFenceWidth();
					if (fenceWidth) {
						m_fenceSpacing = fenceWidth;
					} else {
						Real scaledRadius = t->getTemplateGeometryInfo().getMajorRadius() * 2.18f;
						m_fenceSpacing = (scaledRadius > 10.0f) ? scaledRadius : 10.0f;
					}
					m_fenceOffset = t->getFenceXOffset();
				}				
				break;
			}
			count++;
			pObj = pObj->getNext();
		}

		// If the object is not found or it's filtered out, reset selection
		if (visibleIndex == -1) {
			m_currentObjectIndex = -1;
			return; // Or handle this case as needed
		}
	}

	CWnd *pWnd = GetDlgItem(IDC_FENCE_SPACING_EDIT);
	if (pWnd) {
		CString s;
		s.Format("%f",m_fenceSpacing);
		pWnd->SetWindowText(s);
	}

	/**
	 * Adriane [Deathscythe] --
	 * Support for the Object Preview
	 */
	if (pObj && pObj->getThingTemplate()) {
		m_objectPreview.SetThingTemplate(pObj->getThingTemplate());
	} else {
		m_objectPreview.SetThingTemplate(NULL);
	}
	m_objectPreview.Invalidate();
}


/////////////////////////////////////////////////////////////////////////////
// FenceOptions message handlers

/// Setup the controls in the dialog.
BOOL FenceOptions::OnInitDialog() 
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

		// Only add fence type objects by default. -- we disable this coz this will be the base list before filtering..
		// if (tTemplate->getFenceWidth() == 0) continue; 

		// create new map object
		pMap = newInstance( MapObject)( loc, tTemplate->getName(), 0.0f, 0, NULL, tTemplate );
		pMap->setNextMap( m_objectsList );
		m_objectsList = pMap;

		// get display color for the editor
		Color cc = tTemplate->getDisplayColor();
		pMap->setColor(cc);

	}  // end for tTemplate


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

		if (!m_showAllObjectTypes && pMap->getThingTemplate()->getFenceWidth() == 0) {
			pMap = pMap->getNext();
			continue;
		}

		addObject( pMap, pMap->getName().str(), "", index, TVI_ROOT);
		index++;
		pMap = pMap->getNext();
	}


	m_staticThis = this;
	m_updating = false;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM FenceOptions::findOrAdd(HTREEITEM parent, const char *pLabel)
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

//-------------------------------------------------------------------------------------------------
/** Add the object hierarchy paths to the tree view. */
//-------------------------------------------------------------------------------------------------
void FenceOptions::addObject( MapObject *mapObject, const char *pPath, const char *name, 
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

Bool FenceOptions::hasSelectedObject(void)
{
	// If we have no selected object, return false.
	if (m_currentObjectIndex==-1) return false;
	// If the objects panel has no selected object, return false.
	if (ObjectOptions::getCurGdfName() == AsciiString::TheEmptyString) return false;
	// If our selection is valid, return true.
	if (m_staticThis && m_currentObjectIndex >= 0) {
		MapObject *pObj = m_staticThis->m_objectsList;
		int count = 0;
		while (pObj) {
			if (count == m_currentObjectIndex) {
				return(true);
			}
			count++;
			pObj = pObj->getNext();
		}
	}
	return false;
}

/// Set the selected object in the tree view.
Bool FenceOptions::setObjectTreeViewSelection(HTREEITEM parent, Int selection)
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
			m_objectTreeView.SelectItem(child);
			return(true);
		}
		if (setObjectTreeViewSelection(child, selection)) 
		{
			updateObjectOptions();
			return(true);
		}
		child = m_objectTreeView.GetNextSiblingItem(child);
	}
	return(false);
}


BOOL FenceOptions::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
				m_customSpacing = false;
			}	else if (m_objectTreeView.ItemHasChildren(item.hItem)) {
				m_currentObjectIndex = -1;
			}
			updateObjectOptions();
		}
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}




void FenceOptions::OnChangeFenceSpacingEdit() 
{
	CWnd *pWnd = m_staticThis->GetDlgItem(IDC_FENCE_SPACING_EDIT);
	if (pWnd) {
		CString val;
		pWnd->GetWindowText(val);
		m_fenceSpacing = atof(val);
		m_customSpacing = true;
	}
}

/**
 * Adriane [Deathscythe]
 * Start the long code of pain 
 * 
 * Changes include preview support, non fencetype objects and search.
 */

void FenceOptions::OnOK()
{
    OnSearch(); 
}

// Add the function that handles the search button click
void FenceOptions::OnReset()
{
	m_objectTreeView.DeleteAllItems(); // Clear current tree

	// Repopulate list
	MapObject *pMap =  m_objectsList;
	Int index = 0;
	while (pMap) {

		// Skip non-fence objects if checkbox is unchecked (m_showAllObjectTypes is false)
		if (!m_showAllObjectTypes && pMap->getThingTemplate()->getFenceWidth() == 0) {
			pMap = pMap->getNext();
			continue;
		}

		addObject( pMap, pMap->getName().str(), "", index, TVI_ROOT);
		index++;
		pMap = pMap->getNext();
	}
}

// Add the function that handles the search button click
void FenceOptions::OnSearch()
{
    UpdateData(TRUE);

    CString searchText;
    GetDlgItemText(IDC_OBJECT_SEARCH_EDIT, searchText);
    searchText.MakeLower();

    m_objectTreeView.DeleteAllItems(); // Clear current tree

    if (searchText.IsEmpty())
    {
        // Repopulate full list if search is empty
		MapObject *pMap =  m_objectsList;
		Int index = 0;
		while (pMap) {

			// Skip non-fence objects if checkbox is checked
			if (!m_showAllObjectTypes && pMap->getThingTemplate()->getFenceWidth() == 0) {
				pMap = pMap->getNext();
				continue;
			}
	
			addObject( pMap, pMap->getName().str(), "", index, TVI_ROOT);
			index++;
			pMap = pMap->getNext();
		}

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

			// Skip non-fence objects if checkbox is checked
			if (!m_showAllObjectTypes && pMap->getThingTemplate()->getFenceWidth() == 0) {
				pMap = pMap->getNext();
				continue;
			}

            addObject( pMap, pMap->getName().str(), "", index, TVI_ROOT);
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
    }
}


void FenceOptions::ExpandAllItems(CTreeCtrl& treeCtrl, HTREEITEM hItem)
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

void FenceOptions::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (!bShow)
	{
		/** We are required to do a reset everytime we hide this window or 
		 * 	else there will be a fuck up with object options
		*/
		OnReset();
	}
}

void FenceOptions::OnCheckFenceOnly()
{
    m_showAllObjectTypes = ((CButton*)GetDlgItem(IDC_FENCE_ONLY))->GetCheck();

	/** 
	 * Adriane [Deathscythe]
	 * Only enable search when non-fence types are allowed -- 
	 * I had to do this since the search for non fence type is bugged and fucked -- cant figure out how to fix it nor chatgpt it hallucinating...
	*/
	if (m_showAllObjectTypes)
    {
        GetDlgItem(IDC_OBJECT_SEARCH_EDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_OBJECT_SEARCH_BUTTON)->EnableWindow(TRUE);
        GetDlgItem(IDC_OBJECT_SEARCH_RESET_BTN)->EnableWindow(TRUE);
    } else {
		GetDlgItem(IDC_OBJECT_SEARCH_EDIT)->SetWindowText(_T(""));
		GetDlgItem(IDC_OBJECT_SEARCH_EDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_OBJECT_SEARCH_BUTTON)->EnableWindow(FALSE);
        GetDlgItem(IDC_OBJECT_SEARCH_RESET_BTN)->EnableWindow(FALSE);
		OnReset();
	}

    // if (g_freshWarning_fenceopts) {
    //     AfxMessageBox(
	// 		_T("Warning: Non-fence object types do not have a fence spacing value and will therefore use a default value of 50.0. This may not be accurate for objects with a larger width. You can use the Shift key to dynamically set the fence spacing.\n\n- Adriane [Deathscythe]"),
    //         MB_ICONWARNING | MB_OK
    //     );
	// 	g_freshWarning_fenceopts = false;
    // }
	OnSearch();
}


