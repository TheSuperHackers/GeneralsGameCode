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

// ParticleFrameKeyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "ParticleFrameKeyDialog.h"
#include "Utils.H"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ParticleFrameKeyDialogClass dialog


ParticleFrameKeyDialogClass::ParticleFrameKeyDialogClass(float frame,CWnd* pParent /*=NULL*/) : 
	CDialog(ParticleFrameKeyDialogClass::IDD, pParent),
	m_Frame(frame)
{
	//{{AFX_DATA_INIT(ParticleFrameKeyDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ParticleFrameKeyDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ParticleFrameKeyDialogClass)
	DDX_Control(pDX, IDC_FRAME_SPIN, m_FrameSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ParticleFrameKeyDialogClass, CDialog)
	//{{AFX_MSG_MAP(ParticleFrameKeyDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ParticleFrameKeyDialogClass message handlers


BOOL ParticleFrameKeyDialogClass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	Initialize_Spinner (m_FrameSpin, m_Frame, -1024, 1024);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void ParticleFrameKeyDialogClass::OnOK() 
{
	m_Frame = GetDlgItemFloat(m_hWnd,IDC_FRAME_EDIT);
	CDialog::OnOK();
}

BOOL ParticleFrameKeyDialogClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
