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

// ParticleSizeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "ParticleSizeDialog.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ParticleSizeDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ParticleSizeDialogClass::ParticleSizeDialogClass (float size, CWnd *pParent)
	:	m_Size (size),
		CDialog(ParticleSizeDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ParticleSizeDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
ParticleSizeDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ParticleSizeDialogClass)
	DDX_Control(pDX, IDC_SIZE_SPIN, m_SizeSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ParticleSizeDialogClass, CDialog)
	//{{AFX_MSG_MAP(ParticleSizeDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ParticleSizeDialogClass message handlers


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ParticleSizeDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();

	Initialize_Spinner (m_SizeSpin, m_Size, 0, 10000);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ParticleSizeDialogClass::OnOK (void)
{
	m_Size = GetDlgItemFloat (m_hWnd, IDC_SIZE_EDIT);
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNotify
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ParticleSizeDialogClass::OnNotify
(	
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
) 
{
	//
	//	Update the spinner control if necessary
	//
	NMHDR *pheader = (NMHDR *)lParam;
	if ((pheader != NULL) && (pheader->code == UDN_DELTAPOS)) {
		LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;		
		::Update_Spinner_Buddy (pheader->hwndFrom, pupdown->iDelta);
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}
