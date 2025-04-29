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

// ScaleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "ScaleDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ScaleDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ScaleDialogClass::ScaleDialogClass (float scale, CWnd* pParent,
												const char *prompt_string)
	:	m_Scale (scale),
		m_Prompt(prompt_string),
		CDialog(ScaleDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ScaleDialogClass)
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
ScaleDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScaleDialogClass)
	DDX_Control(pDX, IDC_SIZE_SPIN, m_ScaleSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ScaleDialogClass, CDialog)
	//{{AFX_MSG_MAP(ScaleDialogClass)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ScaleDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();

	m_ScaleSpin.SetRange (1, 10000);
	m_ScaleSpin.SetPos (int(m_Scale * 100.0F));

	// If we were given a different prompt upon creation, apply it now.
	if (!m_Prompt.IsEmpty())
		SetDlgItemText(IDC_PROMPT, m_Prompt);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ScaleDialogClass::OnOK (void)
{
	int pos = m_ScaleSpin.GetPos();
	if (pos & 0xffff0000) {
		// Error condition. Most likely the value is out of range.
		MessageBox("Invalid scale value. Please enter a number between 1 and 10,000", 
			"Invalid Scale", MB_OK | MB_ICONINFORMATION);
		return;
	}
	m_Scale = ((float)pos) / 100.0F;

	// We cannot accept this value if it is less than or equal to zero.
	if (m_Scale <= 0.0f) {
		MessageBox("Scale must be a value greater than zero!", "Invalid Scale",
			MB_OK | MB_ICONINFORMATION);
		return;
	}

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNotify
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ScaleDialogClass::OnNotify
(	
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
) 
{
	//
	//	Update the spinner control if necessary
	//
	NMHDR *header = (NMHDR *)lParam;
	if ((header != NULL) && (header->code == UDN_DELTAPOS)) {
		//LPNMUPDOWN updown = (LPNMUPDOWN)lParam;		
		//::Update_Spinner_Buddy (header->hwndFrom, updown->iDelta);
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}
