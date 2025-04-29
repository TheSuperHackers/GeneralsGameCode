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

// W3DViewView.cpp : implementation of the CW3DViewView class
//

#include "stdafx.h"
#include "W3DView.h"

#include "W3DViewDoc.h"
#include "W3DViewView.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CW3DViewView

IMPLEMENT_DYNCREATE(CW3DViewView, CView)

BEGIN_MESSAGE_MAP(CW3DViewView, CView)
	//{{AFX_MSG_MAP(CW3DViewView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CW3DViewView construction/destruction

CW3DViewView::CW3DViewView()
{
	// TODO: add construction code here

}

CW3DViewView::~CW3DViewView()
{
}

BOOL CW3DViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CW3DViewView drawing

void CW3DViewView::OnDraw(CDC* pDC)
{
	CW3DViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CW3DViewView diagnostics

#ifdef RTS_DEBUG
void CW3DViewView::AssertValid() const
{
	CView::AssertValid();
}

void CW3DViewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CW3DViewDoc* CW3DViewView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CW3DViewDoc)));
	return (CW3DViewDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CW3DViewView message handlers
