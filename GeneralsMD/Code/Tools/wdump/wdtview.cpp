/*
**	Command & Conquer Renegade(tm)
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

// WDTView.cpp : implementation file
//

#include "stdafx.h"
#include "wdump.h"
#include "WDTView.h"
#include "wdumpdoc.h"
#include "chunk_d.h"
#include "finddialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWDumpTreeView

IMPLEMENT_DYNCREATE(CWDumpTreeView, CTreeView)

CWDumpTreeView::CWDumpTreeView()
{
}

CWDumpTreeView::~CWDumpTreeView()
{
}


BEGIN_MESSAGE_MAP(CWDumpTreeView, CTreeView)
	//{{AFX_MSG_MAP(CWDumpTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_COMMAND(IDM_TOOLS_FIND, OnToolsFind)
	ON_COMMAND(IDM_TOOLS_FIND_NEXT, OnToolsFindNext)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWDumpTreeView drawing

void CWDumpTreeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CWDumpTreeView diagnostics

#ifdef RTS_DEBUG
void CWDumpTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CWDumpTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWDumpTreeView message handlers

void CWDumpTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// add all the chunk items to the view
	CTreeCtrl &tree = GetTreeCtrl();
	tree.DeleteAllItems();
	long flags = tree.GetStyle();
	flags |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP;
	SetWindowLong(tree.GetSafeHwnd(), GWL_STYLE, flags);

	CWdumpDoc *doc= (CWdumpDoc *) GetDocument();
	ChunkData *data = &doc->m_ChunkData;
	
	POSITION p = data->Chunks.GetHeadPosition();
	while(p) {
		ChunkItem *item = data->Chunks.GetNext(p);
		InsertItem(item);
	}
}


void CWDumpTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	CWdumpDoc *doc= (CWdumpDoc *) GetDocument();
	doc->m_ChunkItem = (ChunkItem *) pNMTreeView->itemNew.lParam;
	doc->UpdateAllViews(this);

	*pResult = 0;
}


void CWDumpTreeView::InsertItem(ChunkItem * item, HTREEITEM Parent)
{
	const char *name;

	if(item->Type) 
		name = item->Type->Name;
	else {
		static char _buf[256];
		sprintf(_buf,"Unknown: id=0x%X",item->ID);
		name = _buf;
	}

	CTreeCtrl &tree = GetTreeCtrl();
	HTREEITEM tree_item = tree.InsertItem(name, Parent);
	tree.SetItem(tree_item, TVIF_PARAM,0,0,0,0,0, (long) item);

	POSITION p = item->Chunks.GetHeadPosition();
	while(p != 0) {
		ChunkItem *subitem = item->Chunks.GetNext(p); 
		InsertItem(subitem, tree_item);
	}
}


void CWDumpTreeView::OnToolsFind() 
{
	FindDialog finder;

	if (finder.DoModal() == IDOK) {

		// If there is a string go find it.
		if (strlen (FindDialog::String()) > 0) {
			OnToolsFindNext();
		}
	}
}


void CWDumpTreeView::OnToolsFindNext() 
{
	ChunkItem *matchedchunkitem;
	
	// If no string go request one.
	if (strlen (FindDialog::String()) == 0) {

		OnToolsFind();
	
	} else {

		FindDialog::Found (false);

		// Iterate over all chunks in the hierarchy. If a match is found select the tree
		// item that corresponds to the matched chunk item.
		{
			CWaitCursor			waitcursor;
			HTREEITEM			selectedtreeitem;						
			ChunkItem		  *selectedchunkitem;
			SearchStateEnum	searchstate;
			CWdumpDoc		  *doc	= (CWdumpDoc *) GetDocument();
			ChunkData		  *data	= &doc->m_ChunkData;
			POSITION				p;

			// Get the currently selected chunk item.
			selectedtreeitem = GetTreeCtrl().GetSelectedItem();
			if (selectedtreeitem != NULL) {
				selectedchunkitem = (ChunkItem*) GetTreeCtrl().GetItemData (selectedtreeitem);
				searchstate = FIND_SELECTED_ITEM;
			} else {
				selectedchunkitem = NULL;
				searchstate = FIND_STRING;
			}

			p = 0;
			matchedchunkitem = NULL;
			while (true) {

				ChunkItem *chunkitem;

				// Get the root chunk item.
				if (p == 0) {
					p = data->Chunks.GetHeadPosition();
					if (p == 0) break;
				}

				chunkitem = data->Chunks.GetNext (p);
				matchedchunkitem = FindChunkItem (selectedchunkitem, chunkitem, searchstate);
				if ((matchedchunkitem != NULL) || (searchstate == SEARCH_WRAPPED)) break;
			}
		}

		// Was a match found?
		if (matchedchunkitem != NULL) {
			SelectTreeItem (GetTreeCtrl().GetRootItem(), matchedchunkitem);
		} else {

			const char *controlstring  = "Cannot find \"%s\".";

			char *message;

			message = new char [strlen (controlstring) + strlen (FindDialog::String())];
			ASSERT (message != NULL);
			sprintf (message, controlstring, FindDialog::String());
			MessageBox (message, "Find String", MB_OK | MB_ICONEXCLAMATION);
			delete [] message;
		}
	}
}

ChunkItem *CWDumpTreeView::FindChunkItem (ChunkItem *selectedchunkitem, ChunkItem *chunkitem, SearchStateEnum &searchstate)
{
	// Searching for the currently selected item or looking for a match?
	switch (searchstate) {

		case FIND_SELECTED_ITEM:
		
			// Searching for the currently selected chunk item.
			if (chunkitem == selectedchunkitem) {
				searchstate = FIND_STRING;
			}
			break;

		case FIND_STRING:

			// Searching for a string associated with the chunk item. 
			if (chunkitem == selectedchunkitem) {
				searchstate = SEARCH_WRAPPED;
				return (NULL);
			} else {
		  		if ((chunkitem != 0) && (chunkitem->Type != 0) && (chunkitem->Type->Callback != 0)) {
					(*chunkitem->Type->Callback)(chunkitem, NULL);
				}
				if (FindDialog::Found()) return (chunkitem);
			}
			break;

		case SEARCH_WRAPPED:

			// This case should never occur at this point. As soon as it has been detected
			// that the search has wrapped the stack should unwind immediately.
			ASSERT (FALSE);
			return (NULL);
			break;
	}

	// Iterate over all chunks in the hierarchy. Return immediately if a match is found or if the search has wrapped.
	POSITION p = chunkitem->Chunks.GetHeadPosition();
	while (p != 0) {

		ChunkItem *subchunkitem, *matchedchunkitem;

		subchunkitem = chunkitem->Chunks.GetNext (p);
		matchedchunkitem = FindChunkItem (selectedchunkitem, subchunkitem, searchstate);
		if ((matchedchunkitem != NULL) || (searchstate == SEARCH_WRAPPED)) return (matchedchunkitem);
	}

	// No match found.
	return (NULL);
}


void CWDumpTreeView::SelectTreeItem (HTREEITEM treeitem, ChunkItem *chunkitem)
{
	CTreeCtrl &tree = GetTreeCtrl();

	// Select a tree item that matches the given chunk item. Recurse if necessary.
	while (treeitem != NULL) {
		
		HTREEITEM subtreeitem;

		if (tree.GetItemData (treeitem) == (DWORD) chunkitem) {
			tree.SelectItem (treeitem);
		}
		subtreeitem = tree.GetChildItem (treeitem);
		if (subtreeitem != NULL) {
			SelectTreeItem (subtreeitem, chunkitem);
		}
		treeitem = tree.GetNextSiblingItem (treeitem);
	}
}

