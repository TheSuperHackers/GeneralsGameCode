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

// WDLView.cpp : implementation file
//

#include "stdafx.h"
#include "wdump.h"
#include "WDLView.h"
#include "WDumpDoc.h"
#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWDumpListView

IMPLEMENT_DYNCREATE(CWDumpListView, CListView)

CWDumpListView::CWDumpListView()
{
}

CWDumpListView::~CWDumpListView()
{
}


BEGIN_MESSAGE_MAP(CWDumpListView, CListView)
	//{{AFX_MSG_MAP(CWDumpListView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWDumpListView drawing

void CWDumpListView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CWDumpListView diagnostics

#ifdef RTS_DEBUG
void CWDumpListView::AssertValid() const
{
	CListView::AssertValid();
}

void CWDumpListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //RTS_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWDumpListView message handlers

void CWDumpListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CListCtrl &list = GetListCtrl();
	CWdumpDoc *doc= (CWdumpDoc *) GetDocument();
	ChunkItem *item = doc->m_ChunkItem;
	list.DeleteAllItems();	


	if((item != 0) && (item->Type != 0) && (item->Type->Callback != 0)) {
		(*item->Type->Callback)(item, &list);
	}
}

void CWDumpListView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	CListCtrl &list = GetListCtrl();
	long flags = list.GetStyle();
	flags |= LVS_REPORT;
	SetWindowLong(list.GetSafeHwnd(), GWL_STYLE, flags);

	static LV_COLUMN Name_Column = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 230, const_cast<LPSTR>("Name"), 0,0 };
	static LV_COLUMN Type_Column = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 70, const_cast<LPSTR>("Type"), 0,0 };
	static LV_COLUMN Value_Column = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 300, const_cast<LPSTR>("Value"), 0,0 };

	list.InsertColumn(0, &Name_Column);
	list.InsertColumn(1, &Type_Column);
	list.InsertColumn(2, &Value_Column);
}
