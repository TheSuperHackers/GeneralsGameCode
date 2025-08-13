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

// wdumpDoc.h : interface of the CWdumpDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WDUMPDOC_H__41C157ED_5631_11D1_8CDB_006097C6A583__INCLUDED_)
#define AFX_WDUMPDOC_H__41C157ED_5631_11D1_8CDB_006097C6A583__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "chunk_d.h"

class CWdumpDoc : public CDocument
{
//Moumine 1/2/2002    11:12:25 AM ---Need this as public in project W3dShellExt
#if ! defined _W3DSHELLEXT
protected: // create from serialization only
#else
public:
#endif
	CWdumpDoc();
protected:
	DECLARE_DYNCREATE(CWdumpDoc)
	void Read_File(const char *filename);

// Attributes
public:
	ChunkData m_ChunkData;
	ChunkItem *m_ChunkItem;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWdumpDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWdumpDoc();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWdumpDoc)
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WDUMPDOC_H__41C157ED_5631_11D1_8CDB_006097C6A583__INCLUDED_)
