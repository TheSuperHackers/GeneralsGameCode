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

// SaveMap.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "SaveMap.h"
#include "Common/GlobalData.h"

/////////////////////////////////////////////////////////////////////////////
// SaveMap dialog


SaveMap::SaveMap(TSaveMapInfo *pInfo, CWnd* pParent /*=NULL*/)
	: CDialog(SaveMap::IDD, pParent),
	m_pInfo(pInfo)
{
#if defined(RTS_DEBUG)
	m_pInfo->usingSystemDir = m_usingSystemDir = ::AfxGetApp()->GetProfileInt(MAP_OPENSAVE_PANEL_SECTION, "UseSystemDir", TRUE);
#else
	m_pInfo->usingSystemDir = m_usingSystemDir = FALSE;
#endif

	//{{AFX_DATA_INIT(SaveMap)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SaveMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SaveMap)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SaveMap, CDialog)
	//{{AFX_MSG_MAP(SaveMap)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_SYSTEMMAPS, OnSystemMaps)
	ON_BN_CLICKED(IDC_USERMAPS, OnUserMaps)
	ON_LBN_SELCHANGE(IDC_SAVE_LIST, OnSelchangeSaveList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SaveMap message handlers

void SaveMap::OnSystemMaps()
{
	populateMapListbox( TRUE );
}

void SaveMap::OnUserMaps()
{
	populateMapListbox( FALSE );
}

void SaveMap::OnOK() 
{
	CWnd *pEdit = GetDlgItem(IDC_SAVE_MAP_EDIT);
	if (pEdit == NULL) {
		DEBUG_CRASH(("Bad resources."));
		OnCancel();
		return;
	}
	pEdit->GetWindowText(m_pInfo->filename);
	m_pInfo->browse = false;
	// Construct file name of .\Maps\mapname\mapname.map
	CString testName;
	if (m_usingSystemDir)
		testName = ".\\Maps\\";
	else
	{
		testName = TheGlobalData->getPath_UserData().str();
		testName = testName + "Maps\\";
	}
	testName += m_pInfo->filename + "\\" + m_pInfo->filename + ".map"	;
	CFileStatus status;
	if (CFile::GetStatus(testName, status)) {
		CString warn;
		warn.Format(IDS_REPLACEFILE, LPCTSTR(testName));
		Int ret = ::AfxMessageBox(warn, MB_YESNO);
		if (ret == IDNO) {
			return;
		}
	}
	CDialog::OnOK();
}

void SaveMap::OnCancel() 
{
	CDialog::OnCancel();
}

void SaveMap::OnBrowse() 
{
	m_pInfo->browse = true;
	CDialog::OnOK();
}

void SaveMap::populateMapListbox( Bool systemMaps )
{
	m_pInfo->usingSystemDir = m_usingSystemDir = systemMaps;
#if defined(RTS_DEBUG)
	::AfxGetApp()->WriteProfileInt(MAP_OPENSAVE_PANEL_SECTION, "UseSystemDir", m_usingSystemDir);
#endif

	HANDLE			hFindFile = 0;
	WIN32_FIND_DATA			findData; 
	char				dirBuf[_MAX_PATH];
	char				findBuf[_MAX_PATH];
	char				fileBuf[_MAX_PATH];

	if (systemMaps)
		strcpy(dirBuf, ".\\Maps\\");
	else
		sprintf(dirBuf, "%sMaps\\", TheGlobalData->getPath_UserData().str());
	int len = strlen(dirBuf);

	if (len > 0 && dirBuf[len - 1] != '\\') {
		dirBuf[len++] = '\\';
		dirBuf[len] = 0;
	}
	CListBox *pList = (CListBox *)this->GetDlgItem(IDC_SAVE_LIST);
	if (pList == NULL) return;
	pList->ResetContent();
	strcpy(findBuf, dirBuf);
	strcat(findBuf, "*.*");

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
					};
				}
			} catch(...) {}

		} while (FindNextFile(hFindFile, &findData));

		if (hFindFile) FindClose(hFindFile);
 	}
	CEdit *pEdit = (CEdit*)GetDlgItem(IDC_SAVE_MAP_EDIT);
	if (pEdit != NULL) {
		strcpy(fileBuf, m_pInfo->filename);
		Int len = strlen(fileBuf);
		if (len>4 && stricmp(".map", fileBuf+(len-4)) == 0) {
			// strip of the .map
			fileBuf[len-4] = 0;
		}
		while (len>0) {
			if (fileBuf[len] == '\\') {
				len++;
				break;
			}
			len--;
		}
		pEdit->SetWindowText(&fileBuf[len]);
		pEdit->SetSel(0, 1000, true);
		pEdit->SetFocus();
	}
}

void SaveMap::OnSelchangeSaveList() 
{
	CListBox *pList = (CListBox *)this->GetDlgItem(IDC_SAVE_LIST);
	if (pList == NULL) {
		return;
	}
	
	Int sel = pList->GetCurSel();
	CString filename;
	if (sel != LB_ERR) {
		pList->GetText(sel, filename);
	}
	CWnd *pEdit = GetDlgItem(IDC_SAVE_MAP_EDIT);
	if (pEdit == NULL) {
		return;
	}
	pEdit->SetWindowText(filename);
}

BOOL SaveMap::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CButton *pSystemMaps = (CButton *)this->GetDlgItem(IDC_SYSTEMMAPS);
	if (pSystemMaps != NULL)
		pSystemMaps->SetCheck( m_usingSystemDir );

	CButton *pUserMaps = (CButton *)this->GetDlgItem(IDC_USERMAPS);
	if (pUserMaps != NULL)
		pUserMaps->SetCheck( !m_usingSystemDir );

#if !defined(RTS_DEBUG)
	if (pSystemMaps)
		pSystemMaps->ShowWindow( FALSE );
	if (pUserMaps)
		pUserMaps->ShowWindow( FALSE );
#endif

	populateMapListbox( m_usingSystemDir );

	return FALSE;  // return TRUE unless you set the focus to a control
	              
}
