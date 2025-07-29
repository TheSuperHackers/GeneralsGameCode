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

// wdumpDoc.cpp : implementation of the CWdumpDoc class
//

#include "stdafx.h"
#include "wdump.h"
#include "wdumpdoc.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWdumpDoc

IMPLEMENT_DYNCREATE(CWdumpDoc, CDocument)

BEGIN_MESSAGE_MAP(CWdumpDoc, CDocument)
	//{{AFX_MSG_MAP(CWdumpDoc)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWdumpDoc construction/destruction

CWdumpDoc::CWdumpDoc()
{
	// TODO: add one-time construction code here
	m_ChunkItem = 0;
}

CWdumpDoc::~CWdumpDoc()
{
}

BOOL CWdumpDoc::OnNewDocument()
{
	m_ChunkItem = 0;

	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CWdumpDoc serialization

void CWdumpDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		Read_File(ar.m_strFileName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWdumpDoc diagnostics

#ifdef RTS_DEBUG
void CWdumpDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWdumpDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //RTS_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWdumpDoc commands

void CWdumpDoc::OnFileOpen() 
{
	static char szFilter[] = "W3D Files (*.w3d)|*.w3d|WLT Files (*.wlt)|*.wlt|WHT Files (*.wht)|*.wht|WHA Files (*.wha)|*.wha|WTM Files (*.wtm)|*.wtm|All Files (*.*)|*.*||";
	
	CFileDialog f(	true, 
						NULL, 
						NULL, 
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						szFilter);
	
	if(f.DoModal() != IDOK) return;

	// Add this filename to recent file list (MRU).
	// NOTE: This call is not made by the framework.

	//Moumine 1/2/2002    1:10:02 PM -- Project W3DShellExt needs to leave this out
#if ! defined _W3DSHELLEXT 
	theApp.AddToRecentFileList (f.m_ofn.lpstrFile);
#endif
	m_ChunkItem = 0;
	UpdateAllViews(0);
	Read_File(f.m_ofn.lpstrFile);
}


void CWdumpDoc::Read_File(const char *filename)
{
	m_ChunkData.Load(filename);
	m_ChunkItem = 0;
	UpdateAllViews(0);
}