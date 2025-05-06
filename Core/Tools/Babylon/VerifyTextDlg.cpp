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

// VerifyTextDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Babylon.h"
#include "VerifyTextDlg.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVerifyTextDlg dialog


CVerifyTextDlg::CVerifyTextDlg( char *trans, char *orig, CWnd* pParent /*=NULL*/)
	: CDialog(CVerifyTextDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVerifyTextDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_trans = trans;
	m_orig = orig;
}


void CVerifyTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVerifyTextDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVerifyTextDlg, CDialog)
	//{{AFX_MSG_MAP(CVerifyTextDlg)
	ON_BN_CLICKED(IDC_NOMATCH, OnNomatch)
	ON_BN_CLICKED(IDC_MATCH, OnMatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVerifyTextDlg message handlers

void CVerifyTextDlg::OnNomatch() 
{

	EndDialog ( IDNO );
	
}

void CVerifyTextDlg::OnMatch() 
{

	EndDialog ( IDYES );
	
}

BOOL CVerifyTextDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetDlgItemText ( IDC_TRANS, m_trans );
	SetDlgItemText ( IDC_ORIG, m_orig );
		
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
