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

// ParticleRotationKeyDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "ParticleRotationKeyDialog.h"
#include "Utils.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// ParticleRotationKeyDialogClass constructor
//
/////////////////////////////////////////////////////////////////////////////
ParticleRotationKeyDialogClass::ParticleRotationKeyDialogClass(float rotation,CWnd* pParent /*=nullptr*/) :
	CDialog(ParticleRotationKeyDialogClass::IDD, pParent),
	m_Rotation(rotation)
{
	//{{AFX_DATA_INIT(ParticleRotationKeyDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
//
// ParticleRotationKeyDialogClass destructor
//
/////////////////////////////////////////////////////////////////////////////
void ParticleRotationKeyDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ParticleRotationKeyDialogClass)
	DDX_Control(pDX, IDC_ROTATION_SPIN, m_RotationSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ParticleRotationKeyDialogClass, CDialog)
	//{{AFX_MSG_MAP(ParticleRotationKeyDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ParticleRotationKeyDialogClass message handlers


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ParticleRotationKeyDialogClass::OnInitDialog()
{
	CDialog::OnInitDialog();

	Initialize_Spinner (m_RotationSpin, m_Rotation, -10000, 10000);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
//
// OnOk
//
/////////////////////////////////////////////////////////////////////////////
void
ParticleRotationKeyDialogClass::OnOK()
{
	m_Rotation = GetDlgItemFloat (m_hWnd, IDC_ROTATION_EDIT);
	CDialog::OnOK();
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNotify
//
/////////////////////////////////////////////////////////////////////////////
BOOL ParticleRotationKeyDialogClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	//
	//	Update the spinner control if necessary
	//
	NMHDR *pheader = (NMHDR *)lParam;
	if ((pheader != nullptr) && (pheader->code == UDN_DELTAPOS)) {
		LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;
		::Update_Spinner_Buddy (pheader->hwndFrom, pupdown->iDelta);
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}
