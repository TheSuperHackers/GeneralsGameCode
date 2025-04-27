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

// RestrictedFileDialog.cpp : implementation file
//

#include "StdAfx.H"
#include "W3DView.H"
#include "RestrictedFileDialog.H"
#include "Utils.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RestrictedFileDialogClass

IMPLEMENT_DYNAMIC(RestrictedFileDialogClass, CFileDialog)

RestrictedFileDialogClass::RestrictedFileDialogClass(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ExpectedFilename = lpszFileName;
	return ;
}


BEGIN_MESSAGE_MAP(RestrictedFileDialogClass, CFileDialog)
	//{{AFX_MSG_MAP(RestrictedFileDialogClass)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////
//
//	OnFileNameChange
//
void
RestrictedFileDialogClass::OnFileNameChange (void)
{
	// Force the original filename into the filename control
	CommDlg_OpenSave_SetControlText (::GetParent (m_hWnd), 0x480, (LPCTSTR)m_ExpectedFilename);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	OnFileNameOK
//
BOOL
RestrictedFileDialogClass::OnFileNameOK (void)
{
	// Force the original filename into the filename control
	CommDlg_OpenSave_SetControlText (::GetParent (m_hWnd), 0x480, (LPCTSTR)m_ExpectedFilename);
		
	// Fill the filename fields of the OPENFILESTRUCT structure with the
	// original filename and the new path
	CString path = ::Strip_Filename_From_Path (m_ofn.lpstrFile);
	path += "\\";
	path += m_ExpectedFilename;
	::lstrcpy (m_ofn.lpstrFile, path);
	::lstrcpy (m_ofn.lpstrFileTitle, m_ExpectedFilename);
	return FALSE;
}


/////////////////////////////////////////////////////////////////
//
//	OnInitDone
//
void
RestrictedFileDialogClass::OnInitDone (void)
{
	// Disable the controls we don't want the user to change
	::EnableWindow (::GetDlgItem (::GetParent (m_hWnd), 0x480), FALSE);
	::EnableWindow (::GetDlgItem (::GetParent (m_hWnd), 0x470), FALSE);
	return ;
}
