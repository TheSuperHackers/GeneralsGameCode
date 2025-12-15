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

// CameraDistanceDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "CameraDistanceDialog.h"
#include "Utils.h"
#include "GraphicView.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CameraDistanceDialogClass
//
/////////////////////////////////////////////////////////////////////////////
CameraDistanceDialogClass::CameraDistanceDialogClass(CWnd* pParent /*=nullptr*/)
	: CDialog(CameraDistanceDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CameraDistanceDialogClass)
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
CameraDistanceDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CameraDistanceDialogClass)
	DDX_Control(pDX, IDC_DISTANCE_SPIN, m_DistanceSpinCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(CameraDistanceDialogClass, CDialog)
	//{{AFX_MSG_MAP(CameraDistanceDialogClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CameraDistanceDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	CGraphicView *graphic_view = ::Get_Graphic_View ();

	::Initialize_Spinner (m_DistanceSpinCtrl, graphic_view->Get_Camera_Distance (), 0, 25000.0F);
	::SetDlgItemFloat (m_hWnd, IDC_DISTANCE_EDIT, graphic_view->Get_Camera_Distance ());
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
CameraDistanceDialogClass::OnOK (void)
{
	CDialog::OnOK ();

	float distance = ::GetDlgItemFloat (m_hWnd, IDC_DISTANCE_EDIT);
	CGraphicView *graphic_view = ::Get_Graphic_View ();
	graphic_view->Set_Camera_Distance (distance);
	return ;
}


////////////////////////////////////////////////////////////////////
//
//	OnNotify
//
////////////////////////////////////////////////////////////////////
BOOL
CameraDistanceDialogClass::OnNotify
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
	if ((pheader != nullptr) && (pheader->code == UDN_DELTAPOS)) {
		LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;
		::Update_Spinner_Buddy (pheader->hwndFrom, pupdown->iDelta);
	}

	// Allow the base class to process this message
	return CDialog::OnNotify (wParam, lParam, pResult);
}
