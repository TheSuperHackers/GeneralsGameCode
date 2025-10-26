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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/OpacitySettingsDialog.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "W3DView.h"
#include "OpacitySettingsDialog.h"
#include "ColorBar.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// OpacitySettingsDialogClass
//
/////////////////////////////////////////////////////////////////////////////
OpacitySettingsDialogClass::OpacitySettingsDialogClass (float opacity, CWnd *pParent)
	:	m_OpacityBar (NULL),
		m_Opacity (opacity),
		CDialog(OpacitySettingsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(OpacitySettingsDialogClass)
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
OpacitySettingsDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OpacitySettingsDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(OpacitySettingsDialogClass, CDialog)
	//{{AFX_MSG_MAP(OpacitySettingsDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
OpacitySettingsDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog();

	m_OpacityBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_OPACITY_BAR));
	ASSERT (m_OpacityBar);

	//
	// Setup the opacity bar
	//
	m_OpacityBar->Set_Range (0, 1);
	m_OpacityBar->Modify_Point (0, 0, 0, 0, 0);
	m_OpacityBar->Insert_Point (1, 1, 255, 255, 255);
	m_OpacityBar->Set_Selection_Pos (m_Opacity);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
OpacitySettingsDialogClass::OnOK (void)
{
	m_Opacity = m_OpacityBar->Get_Selection_Pos ();
	CDialog::OnOK ();
	return ;
}
