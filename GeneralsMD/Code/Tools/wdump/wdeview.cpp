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
#include "wdeview.h"
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
#endif //RTS_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWDumpEditView message handlers

void CWDumpEditView::OnInitialUpdate()
{
	SetFont(CFont::FromHandle((HFONT)::GetStockObject(ANSI_FIXED_FONT)));
	CEditView::OnInitialUpdate();
}

void CWDumpEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// TODO: Add your specialized code here and/or call the base class
	CEdit &edit = GetEditCtrl();
	edit.SetReadOnly(TRUE);
	CWdumpDoc *doc= (CWdumpDoc *) GetDocument();
	ChunkItem *item = doc->m_ChunkItem;

	if(item == nullptr) {
		edit.SetWindowText("Load a chunk file and select the chunk in the tree view to see it's hex data here.");
		return; // no selected chunk item, leave a clear screen.
	}
	if(item->Type != nullptr && item->Type->Wrapper) {
		CString text;
		text.Format("This chunk is a wrapper chunk for other chunks. Its total length is %d bytes.", item->Length);
		edit.SetWindowText(text);
		return;
	}
	if(item->Length == 0) {
		edit.SetWindowText("This chunk contains no data.");
		return;
	}
	char *text = Build_Hex_Text((unsigned char *) item->Data, item->Length);

	edit.SetWindowText(text);

	delete [] text;
}


char * CWDumpEditView::Build_Hex_Text(unsigned char * Source, int Length)
{
	// Limit the shown bytes so that the buffer size cannot overflow. Retail leaf chunks stay below 64 KiB.
	const int max_length = 1024 * 1024;
	const int hidden_length = Length > max_length ? Length - max_length : 0;
	Length -= hidden_length;

	const int max_per_line = 16;
	int per_line = max_per_line;
	int lines = (Length + per_line - 1) / per_line;
	const int reserved_length = (hidden_length > 0) ? 64 : 0;
	const size_t buf_size = (size_t)Length * 5 + per_line * 5 + reserved_length;

	char *buffer = new char[buf_size];
	char *dest = buffer;

	while(lines--) {
		if(lines == 0 && Length % per_line != 0) {
			per_line = Length % per_line;
		}

		// Print bytes every 3 characters.
		int counter = 0;
		do {
			sprintf(dest, "%02x ", Source[counter]);
			dest += 3;
		} while(++counter < per_line);

		if (per_line < max_per_line) {
			// Print spaces at left-over blank bytes in last row.
			const int blank_length = max_per_line - per_line;
			const int skip_char_count = blank_length * 3;
			memset(dest, ' ', skip_char_count);
			dest += skip_char_count;
		}

		// Print 2 spaces.
		*dest++ = ' ';
		*dest++ = ' ';

		// Print ascii characters.
		counter = 0;
		do {
			char c = Source[counter];
			if(c >= 32 && c <= 192)
				*dest++ = c;
			else
				*dest++ = '.';
		} while(++counter < per_line);

		// Print line break.
		*dest++ = '\r';
		*dest++ = '\n';

		Source += per_line;
	}

	if(hidden_length > 0) {
		dest += sprintf(dest, "... %d more bytes not shown", hidden_length);
	}

	*dest = 0;
	return buffer;
}
