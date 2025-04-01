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
 *                     $Archive:: /Commando/Code/Tools/W3DView/AggregateNameDialog.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.H"
#include "W3DView.H"
#include "AggregateNameDialog.H"
#include "W3D_File.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
//	AggregateNameDialogClass
//
AggregateNameDialogClass::AggregateNameDialogClass (CWnd* pParent /*=NULL*/)
	: CDialog(AggregateNameDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(AggregateNameDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	AggregateNameDialogClass
//
AggregateNameDialogClass::AggregateNameDialogClass
(
	UINT resource_id,
	const CString &def_name,
	CWnd *pParent
)
	: m_Name (def_name),
	  CDialog (resource_id, pParent)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	AggregateNameDialogClass
//
void
AggregateNameDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange (pDX);
	//{{AFX_DATA_MAP(AggregateNameDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(AggregateNameDialogClass, CDialog)
	//{{AFX_MSG_MAP(AggregateNameDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
void
AggregateNameDialogClass::OnOK (void)
{
	GetDlgItemText (IDC_AGGREGATE_NAME, m_Name);
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
BOOL
AggregateNameDialogClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();

	// Restrict the amount of text a user can type into the control
	SendDlgItemMessage (IDC_AGGREGATE_NAME, EM_LIMITTEXT, (WPARAM)W3D_NAME_LEN-1);
	SetDlgItemText (IDC_AGGREGATE_NAME, m_Name);
	return TRUE;
}
