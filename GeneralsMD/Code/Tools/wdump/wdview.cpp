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

// wdumpView.cpp : implementation of the CWdumpView class
//

#include "stdafx.h"
#include "wdump.h"

#include "wdumpdoc.h"
#include "wdview.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWdumpView

IMPLEMENT_DYNCREATE(CWdumpView, CView)

BEGIN_MESSAGE_MAP(CWdumpView, CView)
	//{{AFX_MSG_MAP(CWdumpView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWdumpView construction/destruction

CWdumpView::CWdumpView()
{
	// TODO: add construction code here

}

CWdumpView::~CWdumpView()
{
}

BOOL CWdumpView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CWdumpView drawing

void CWdumpView::OnDraw(CDC* pDC)
{
	CWdumpDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CWdumpView diagnostics

#ifdef RTS_DEBUG
void CWdumpView::AssertValid() const
{
	CView::AssertValid();
}

void CWdumpView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWdumpDoc* CWdumpView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWdumpDoc)));
	return (CWdumpDoc*)m_pDocument;
}
#endif //RTS_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWdumpView message handlers
