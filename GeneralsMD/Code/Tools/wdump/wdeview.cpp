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

// WDView.cpp : implementation file
//

#include "stdafx.h"
#include "wdump.h"
#include "WDEView.h"
#include "wdumpdoc.h"
#include "chunk_d.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWDumpEditView

IMPLEMENT_DYNCREATE(CWDumpEditView, CEditView)

CWDumpEditView::CWDumpEditView()
{
}

CWDumpEditView::~CWDumpEditView()
{
}


BEGIN_MESSAGE_MAP(CWDumpEditView, CEditView)
	//{{AFX_MSG_MAP(CWDumpEditView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWDumpEditView drawing

void CWDumpEditView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CWDumpEditView diagnostics

#ifdef RTS_DEBUG
void CWDumpEditView::AssertValid() const
{
	CEditView::AssertValid();
}

void CWDumpEditView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWDumpEditView message handlers

void CWDumpEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	CEdit &edit = GetEditCtrl();
	edit.SetReadOnly(TRUE);
	CWdumpDoc *doc= (CWdumpDoc *) GetDocument();
	ChunkItem *item = doc->m_ChunkItem;

	if(item == 0) {
		edit.SetWindowText("Load a chunk file and select the chunk in the tree view to see it's hex data here.");
		return; // no selected chunk item, leave a clear screen.
	}
	char *text = Build_Hex_Text((unsigned char *) item->Data, item->Length);

	edit.SetWindowText(text);

	delete text;
}


char * CWDumpEditView::Build_Hex_Text(unsigned char * Source, int Length)
{
	if(Source == 0) {
			char *c = new char[256];
			sprintf(c, "This chunk is a wrapper chunk for other chunks. It's total length is %d", Length);
			return c;
	}
	int per_line = 16;

	int lines = Length / per_line;
	int buf_size = Length * 5 + per_line * 5;
	
	char *buffer = new char[buf_size];
	char *dest = buffer;

	while(lines--) {
		if(lines == 0) {
			per_line = Length % per_line;
		}
		int counter = 0;
		do {
			sprintf(dest, "%02x ", Source[counter]);
			dest += 3;
		} while(++counter < per_line);

		*dest++ = ' ';
		*dest++ = ' ';

		counter = 0;
		do {
			char c = Source[counter];
			if(c >= 32 && c <= 192) 
				*dest++ = c;
			else 
				*dest++ = '.';
		} while(++counter < per_line);

		*dest++ = '\r';
		*dest++ = '\n';
		Source += per_line;
	}
	*dest = 0;
	return buffer;
}
