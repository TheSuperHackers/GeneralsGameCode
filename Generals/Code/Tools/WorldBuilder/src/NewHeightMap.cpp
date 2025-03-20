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

// NewHeightMap.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "NewHeightMap.h"

/////////////////////////////////////////////////////////////////////////////
// CNewHeightMap dialog


CNewHeightMap::CNewHeightMap(TNewHeightInfo *hiP,  const char *label, CWnd* pParent /*=NULL*/)
	: CDialog(CNewHeightMap::IDD, pParent)
{
	mHeightInfo = *hiP;
	m_label = label;
	//{{AFX_DATA_INIT(CNewHeightMap)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNewHeightMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewHeightMap)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewHeightMap, CDialog)
	//{{AFX_MSG_MAP(CNewHeightMap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewHeightMap message handlers

BOOL CNewHeightMap::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *edit = GetDlgItem(IDC_INITIAL_HEIGHT);
	CString val;
	val.Format("%d", mHeightInfo.initialHeight);
	if (edit) edit->SetWindowText(val);

	edit = GetDlgItem(IDC_X_SIZE);
	val.Format("%d", mHeightInfo.xExtent);
	if (edit) edit->SetWindowText(val);

	edit = GetDlgItem(IDC_BORDER_SIZE);
	val.Format("%d", mHeightInfo.borderWidth);
	if (edit) edit->SetWindowText(val);

	edit = GetDlgItem(IDC_Y_SIZE);
	val.Format("%d", mHeightInfo.yExtent);
	if (edit) edit->SetWindowText(val);
	if (m_label) SetWindowText(m_label);


	CButton *pButton;
	if (mHeightInfo.forResize) {
		pButton = (CButton*)GetDlgItem(IDC_CENTER);
		pButton->SetCheck(1);
		mHeightInfo.anchorBottom = false;
		mHeightInfo.anchorTop = false;
		mHeightInfo.anchorLeft = false;
		mHeightInfo.anchorRight = false;
		doAnchorButton(IDC_CENTER);
	} else {
		pButton = (CButton*)GetDlgItem(IDC_CENTER);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_TOP_LEFT);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_TOP);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_TOP_RIGHT);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_CENTER_LEFT);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_CENTER_RIGHT);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_BOTTOM);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_BOTTOM_LEFT);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_BOTTOM_RIGHT);
		pButton->ShowWindow(SW_HIDE);
		pButton = (CButton*)GetDlgItem(IDC_ANCHOR_LABEL);
		pButton->ShowWindow(SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Handles the button press for this button. */
Bool CNewHeightMap::doAnchorButton(Int buttonID)
{
	TNewHeightInfo heightInfo = mHeightInfo;
	Bool processed = true;
	heightInfo.anchorBottom = false;
	heightInfo.anchorTop = false;
	heightInfo.anchorLeft = false;
	heightInfo.anchorRight = false;
	switch(buttonID) {
		case IDC_CENTER: 
		break;
		case IDC_TOP_LEFT: 
			heightInfo.anchorTop = true;
			heightInfo.anchorLeft = true;
		break;
		case IDC_TOP: 
			heightInfo.anchorTop = true;
		break;
		case IDC_TOP_RIGHT: 
			heightInfo.anchorTop = true;
			heightInfo.anchorRight = true;
		break;
		case IDC_CENTER_LEFT: 
			heightInfo.anchorLeft = true;
		break;
		case IDC_CENTER_RIGHT: 
			heightInfo.anchorRight = true;
		break;
		case IDC_BOTTOM: 
			heightInfo.anchorBottom = true;
		break;
		case IDC_BOTTOM_LEFT: 
			heightInfo.anchorBottom = true;
			heightInfo.anchorLeft = true;
		break;
		case IDC_BOTTOM_RIGHT: 
			heightInfo.anchorBottom = true;
			heightInfo.anchorRight = true;
		break;
		default:
			processed = false;
		break;
	}
	if (!processed) return(false);
	mHeightInfo = heightInfo;
	CButton *pButton;
	pButton = (CButton*)GetDlgItem(IDC_CENTER);
	pButton->SetCheck(buttonID==IDC_CENTER?1:0);
	pButton = (CButton*)GetDlgItem(IDC_TOP_LEFT);
	pButton->SetCheck(buttonID==IDC_TOP_LEFT?1:0);
	pButton = (CButton*)GetDlgItem(IDC_TOP);
	pButton->SetCheck(buttonID==IDC_TOP?1:0);
	pButton = (CButton*)GetDlgItem(IDC_TOP_RIGHT);
	pButton->SetCheck(buttonID==IDC_TOP_RIGHT?1:0);
	pButton = (CButton*)GetDlgItem(IDC_CENTER_LEFT);
	pButton->SetCheck(buttonID==IDC_CENTER_LEFT?1:0);
	pButton = (CButton*)GetDlgItem(IDC_CENTER_RIGHT);
	pButton->SetCheck(buttonID==IDC_CENTER_RIGHT?1:0);
	pButton = (CButton*)GetDlgItem(IDC_BOTTOM);
	pButton->SetCheck(buttonID==IDC_BOTTOM?1:0);
	pButton = (CButton*)GetDlgItem(IDC_BOTTOM_LEFT);
	pButton->SetCheck(buttonID==IDC_BOTTOM_LEFT?1:0);
	pButton = (CButton*)GetDlgItem(IDC_BOTTOM_RIGHT);
	pButton->SetCheck(buttonID==IDC_BOTTOM_RIGHT?1:0);
	pButton = (CButton*)GetDlgItem(IDC_ANCHOR_LABEL);
	pButton->SetCheck(buttonID==IDC_ANCHOR_LABEL?1:0);
	return true;

}


void CNewHeightMap::OnOK() 
{
	CWnd *edit = GetDlgItem(IDC_INITIAL_HEIGHT);
	CString val;
	if (edit) {
		edit->GetWindowText(val);
		mHeightInfo.initialHeight = atoi(val);
	}
	edit = GetDlgItem(IDC_X_SIZE);
	if (edit) {
		edit->GetWindowText(val);
		mHeightInfo.xExtent = atoi(val);
	}
	edit = GetDlgItem(IDC_Y_SIZE);
	if (edit) {
		edit->GetWindowText(val);
		mHeightInfo.yExtent = atoi(val);
	}
	edit = GetDlgItem(IDC_BORDER_SIZE);
	if (edit) {
		edit->GetWindowText(val);
		mHeightInfo.borderWidth = atoi(val);
	}
	CDialog::OnOK();
}


BOOL CNewHeightMap::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	Int cmd = HIWORD(wParam);
	if (cmd == BN_CLICKED) {
		Int idButton = (int) LOWORD(wParam);    // identifier of button 
		if (doAnchorButton(idButton)) {
			return true;
		}
	}
 	
	return CDialog::OnCommand(wParam, lParam);
}
