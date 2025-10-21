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

#if !defined(AFX_WDTVIEW_H__41C157F7_5631_11D1_8CDB_006097C6A583__INCLUDED_)
#define AFX_WDTVIEW_H__41C157F7_5631_11D1_8CDB_006097C6A583__INCLUDED_

#pragma once
// WDTView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWDumpTreeView view
class ChunkItem;
class CWDumpTreeView : public CTreeView
{
protected:
	CWDumpTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CWDumpTreeView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWDumpTreeView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CWDumpTreeView();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:

	enum SearchStateEnum {
		FIND_SELECTED_ITEM,
		FIND_STRING,
		SEARCH_WRAPPED
	};

	void		  InsertItem(ChunkItem *item, HTREEITEM Parent = TVI_ROOT);
	ChunkItem *FindChunkItem (ChunkItem *selectedchunkitem, ChunkItem *chunkitem, SearchStateEnum &searchstate);
	void		  SelectTreeItem (HTREEITEM treeitem, ChunkItem *chunkitem);

	//{{AFX_MSG(CWDumpTreeView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnToolsFind();
	afx_msg void OnToolsFindNext();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WDTVIEW_H__41C157F7_5631_11D1_8CDB_006097C6A583__INCLUDED_)
