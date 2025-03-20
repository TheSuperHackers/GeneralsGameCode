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

// SelectMacrotexture.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "SelectMacrotexture.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/HeightMap.h"

/////////////////////////////////////////////////////////////////////////////
// SelectMacrotexture dialog


SelectMacrotexture::SelectMacrotexture(CWnd* pParent /*=NULL*/)
	: CDialog(SelectMacrotexture::IDD, pParent)
{
	//{{AFX_DATA_INIT(SelectMacrotexture)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SelectMacrotexture::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectMacrotexture)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SelectMacrotexture, CDialog)
	//{{AFX_MSG_MAP(SelectMacrotexture)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define DEFAULT "***Default"

/////////////////////////////////////////////////////////////////////////////
// SelectMacrotexture message handlers

BOOL SelectMacrotexture::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *pWnd = GetDlgItem(IDC_TEXTURE_TREEVIEW);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_textureTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, rect, this, IDC_TERRAIN_TREEVIEW);
	m_textureTreeView.ShowWindow(SW_SHOW);

	{
		char				dirBuf[_MAX_PATH];
		char				findBuf[_MAX_PATH];
		char				fileBuf[_MAX_PATH];

		strcpy(dirBuf, "..\\TestArt");
		int len = strlen(dirBuf);

		if (len > 0 && dirBuf[len - 1] != '\\') {
			dirBuf[len++] = '\\';
			dirBuf[len] = 0;
		}
		strcpy(findBuf, dirBuf);

		FilenameList filenameList;
		TheFileSystem->getFileListInDirectory(AsciiString(findBuf), AsciiString("*.tga"), filenameList, FALSE);

		if (filenameList.size() > 0) {
			TVINSERTSTRUCT ins;
			HTREEITEM child = NULL;
			FilenameList::iterator it = filenameList.begin();
			do {
				AsciiString filename = *it;
				len = filename.getLength();
				if (len<5) continue;
				strcpy(fileBuf, filename.str());
					::memset(&ins, 0, sizeof(ins));
					ins.hParent = TVI_ROOT;
					ins.hInsertAfter = TVI_SORT;
					ins.item.mask = TVIF_PARAM|TVIF_TEXT;
					ins.item.lParam = -1;
					ins.item.pszText = fileBuf;
					ins.item.cchTextMax = strlen(fileBuf);				
					child = m_textureTreeView.InsertItem(&ins);
				++it;
			} while (it != filenameList.end());

			::memset(&ins, 0, sizeof(ins));
			ins.hParent = TVI_ROOT;
			ins.hInsertAfter = TVI_SORT;
			ins.item.mask = TVIF_PARAM|TVIF_TEXT;
			ins.item.lParam = -1;
			ins.item.pszText = DEFAULT;
			ins.item.cchTextMax = strlen(DEFAULT);				
			child = m_textureTreeView.InsertItem(&ins);

 		}
	}

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL SelectMacrotexture::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;
	if (pHdr->hdr.hwndFrom == m_textureTreeView.m_hWnd) {
		if (pHdr->hdr.code == TVN_SELCHANGED) {
			char buffer[_MAX_PATH];
			HTREEITEM hItem = m_textureTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
			item.hItem = hItem;
			item.pszText = buffer;
			item.cchTextMax = sizeof(buffer)-2;				
			m_textureTreeView.GetItem(&item);
			if (0==strcmp(buffer, DEFAULT)) {
				TheTerrainRenderObject->updateMacroTexture(AsciiString(""));
			} else {
				TheTerrainRenderObject->updateMacroTexture(AsciiString(buffer));
			}
			TheWritableGlobalData->m_useLightMap = true;
		}
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}
