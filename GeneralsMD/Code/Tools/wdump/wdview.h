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

// wdumpView.h : interface of the CWdumpView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WDUMPVIEW_H__41C157EF_5631_11D1_8CDB_006097C6A583__INCLUDED_)
#define AFX_WDUMPVIEW_H__41C157EF_5631_11D1_8CDB_006097C6A583__INCLUDED_

#pragma once

class CWdumpView : public CView
{
protected: // create from serialization only
	CWdumpView();
	DECLARE_DYNCREATE(CWdumpView)

// Attributes
public:
	CWdumpDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWdumpView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWdumpView();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWdumpView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef RTS_DEBUG  // debug version in wdumpView.cpp
inline CWdumpDoc* CWdumpView::GetDocument()
   { return (CWdumpDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WDUMPVIEW_H__41C157EF_5631_11D1_8CDB_006097C6A583__INCLUDED_)
