/*
**	Command & Conquer Generals(tm)
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

// OpenMap.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "OpenMap.h"
#include "Common/GlobalData.h"

/////////////////////////////////////////////////////////////////////////////
// OpenMap dialog


OpenMap::OpenMap(TOpenMapInfo *pInfo, CWnd* pParent /*=NULL*/)
	: CDialog(OpenMap::IDD, pParent),
	m_pInfo(pInfo)
{
	m_pInfo->browse = false;
	m_usingSystemDir = ::AfxGetApp()->GetProfileInt(MAP_OPENSAVE_PANEL_SECTION, "UseSystemDir", TRUE);

	//{{AFX_DATA_INIT(OpenMap)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void OpenMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OpenMap)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OpenMap, CDialog)
	//{{AFX_MSG_MAP(OpenMap)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_SYSTEMMAPS, OnSystemMaps)
	ON_BN_CLICKED(IDC_USERMAPS, OnUserMaps)
	ON_LBN_DBLCLK(IDC_OPEN_LIST, OnDblclkOpenList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OpenMap message handlers

void OpenMap::OnSystemMaps()
{
	populateMapListbox( TRUE );
}

void OpenMap::OnUserMaps()
{
	populateMapListbox( FALSE );
}

void OpenMap::OnBrowse() 
{
	m_pInfo->browse = true;
	OnOK();
}

void OpenMap::OnOK() 
{
	CListBox *pList = (CListBox *)this->GetDlgItem(IDC_OPEN_LIST);
	if (pList == NULL) {
		OnCancel();
		return;
	}
	
	Int sel = pList->GetCurSel();
	if (sel == LB_ERR) {
		m_pInfo->browse = true;
	} else {
		CString newName;
		pList->GetText(sel, newName );
		if (m_usingSystemDir)
			m_pInfo->filename = ".\\Maps\\" + newName + "\\" + newName + ".map";
		else
		{
			m_pInfo->filename = TheGlobalData->getPath_UserData().str();
			m_pInfo->filename = m_pInfo->filename + "Maps\\" + newName + "\\" + newName + ".map";
		}
	}
	CDialog::OnOK();
}

void OpenMap::populateMapListbox( Bool systemMaps )
{
	m_usingSystemDir = systemMaps;
	::AfxGetApp()->WriteProfileInt(MAP_OPENSAVE_PANEL_SECTION, "UseSystemDir", m_usingSystemDir);

	HANDLE			hFindFile = 0;
	WIN32_FIND_DATA			findData;
	char				dirBuf[_MAX_PATH];
	char				findBuf[_MAX_PATH];
	char				fileBuf[_MAX_PATH];

	if (systemMaps)
		strcpy(dirBuf, "Maps\\");
	else
	{
		strcpy(dirBuf, TheGlobalData->getPath_UserData().str());
		strcat(dirBuf, "Maps\\");
	}

	int len = strlen(dirBuf);

	if (len > 0 && dirBuf[len - 1] != '\\') {
		dirBuf[len++] = '\\';
		dirBuf[len] = 0;
	}
	CListBox *pList = (CListBox *)this->GetDlgItem(IDC_OPEN_LIST);
	if (pList == NULL) return;
	pList->ResetContent();
	strcpy(findBuf, dirBuf);
	strcat(findBuf, "*.*");

	Bool found = false;

	hFindFile = FindFirstFile(findBuf, &findData); 
	if (hFindFile != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
				continue;
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				continue;
			}

			strcpy(fileBuf, dirBuf);
			strcat(fileBuf, findData.cFileName);
			strcat(fileBuf, "\\");
			strcat(fileBuf, findData.cFileName);
			strcat(fileBuf, ".map");
			try {
				CFileStatus status;
				if (CFile::GetStatus(fileBuf, status)) {
					if (!(status.m_attribute & CFile::directory)) {
						pList->AddString(findData.cFileName);
						found = true;
					};
				}
			} catch(...) {}

		} while (FindNextFile(hFindFile, &findData));

		if (hFindFile) FindClose(hFindFile);
 	}
	if (found) {
		pList->SetCurSel(0);
	} else {
		CWnd *pOk = GetDlgItem(IDOK);
		if (pOk) pOk->EnableWindow(false);
	}
}

BOOL OpenMap::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CButton *pSystemMaps = (CButton *)this->GetDlgItem(IDC_SYSTEMMAPS);
	if (pSystemMaps != NULL)
		pSystemMaps->SetCheck( m_usingSystemDir );

	CButton *pUserMaps = (CButton *)this->GetDlgItem(IDC_USERMAPS);
	if (pUserMaps != NULL)
		pUserMaps->SetCheck( !m_usingSystemDir );

	// TheSuperHackers @tweak Originally World Builder has hidden the System Maps tab button in Release builds,
	// perhaps with the intention to only show User Maps to community users. However, World Builder did release
	// as a Debug build and always had the System Maps tab, therefore this now shows it always for simplicity.

	populateMapListbox( m_usingSystemDir );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void OpenMap::OnDblclkOpenList() 
{
	OnOK();
}
