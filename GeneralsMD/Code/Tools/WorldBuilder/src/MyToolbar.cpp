/*
**	Command & Conquer Generals Zero Hour(tm)
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

// MyToolBar.cpp
// Class to handle cell size slider.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"

#include "Lib/BaseType.h"
#include "MyToolbar.h"
#include "resource.h"
#include "WorldBuilderView.h"
#include "WorldBuilderDoc.h"

BEGIN_MESSAGE_MAP(CellSizeToolBar, CDialogBar)
	//{{AFX_MSG_MAP(CellSizeToolBar)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define MAX_POS 7
#define MIN_POS 1

CellSizeToolBar *CellSizeToolBar::m_staticThis = NULL;

void CellSizeToolBar::CellSizeChanged(Int cellSize)
{
	Int newSize = 1;
	Int i;
	for (i=MIN_POS; i<MAX_POS; i++) {
		newSize *= 2;
		if (newSize >= cellSize) break;
	}
	i = MAX_POS - i + MIN_POS;  // Invert the range
	if (m_staticThis != NULL) {
		m_staticThis->m_cellSlider.SetPos(i);
	}
}

CellSizeToolBar::~CellSizeToolBar(void)
{
	m_staticThis = NULL;
}

void CellSizeToolBar::SetupSlider(void)
{
	CWnd *pWnd = GetDlgItem(ID_SLIDER);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	pWnd->DestroyWindow();
	ScreenToClient(&rect);

	m_cellSlider.Create(TBS_VERT|TBS_AUTOTICKS|TBS_RIGHT, rect, this, ID_SLIDER);
	m_cellSlider.SetRange(MIN_POS,MAX_POS);
	m_cellSlider.SetPos(3);
	m_cellSlider.ShowWindow(SW_SHOW);
	m_staticThis = this;
}

void CellSizeToolBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (nSBCode != TB_THUMBTRACK) {
		nPos = m_cellSlider.GetPos();
	}
	UnsignedInt i;
	// invert
	nPos = MAX_POS - nPos + MIN_POS;
	int newSize = 1;
	for (i=1; i<nPos; i++) {
		newSize *= 2;
	}
	if (newSize>64) return;

	CWorldBuilderView* pView = CWorldBuilderDoc::GetActive2DView();
	if (pView == NULL || newSize == pView->getCellSize()) return;
	pView->setCellSize(newSize);
}

LRESULT CellSizeToolBar::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	if (message == WM_VSCROLL) {
			int nScrollCode = (short)LOWORD(wParam);
			int nPos = (short)HIWORD(wParam);
			OnVScroll(nScrollCode, nPos, NULL);
			return(0);
	}
	return(CDialogBar::WindowProc(message, wParam, lParam));
}
