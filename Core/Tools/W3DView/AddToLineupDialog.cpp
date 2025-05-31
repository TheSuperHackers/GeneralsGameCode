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

// AddToLineupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "AddToLineupDialog.h"

#include "ViewerScene.h"
#include <rendobj.h>
#include <assetmgr.h>


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddToLineupDialog dialog


CAddToLineupDialog::CAddToLineupDialog(ViewerSceneClass *scene, CWnd* pParent /*=NULL*/)
:	CDialog(CAddToLineupDialog::IDD, pParent),
	m_pCScene(scene)
{
	//{{AFX_DATA_INIT(CAddToLineupDialog)
	m_Object = _T("");
	//}}AFX_DATA_INIT
}


void CAddToLineupDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddToLineupDialog)
	DDX_CBString(pDX, IDC_OBJECT, m_Object);
	DDV_MaxChars(pDX, m_Object, 64);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddToLineupDialog, CDialog)
	//{{AFX_MSG_MAP(CAddToLineupDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddToLineupDialog message handlers

BOOL CAddToLineupDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_pCScene)
	{
		// Get a pointer to the combo box control.
		CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_OBJECT);
		ASSERT_VALID(pCombo);

		// Populate the combo box with the names of the objects that
		// can be added to the lineup.
		WW3DAssetManager *assets = WW3DAssetManager::Get_Instance();
		ASSERT(assets != NULL);
		RenderObjIterator *it = assets->Create_Render_Obj_Iterator();
		ASSERT(it != NULL);
		for (; !it->Is_Done(); it->Next())
		{
			if (m_pCScene->Can_Line_Up(it->Current_Item_Class_ID()))
				pCombo->AddString(it->Current_Item_Name());
		}
		assets->Release_Render_Obj_Iterator(it);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddToLineupDialog::OnOK() 
{
	// Make sure the user actually chose a name.
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_OBJECT);
	ASSERT_VALID(pCombo);
	CString text;
	pCombo->GetWindowText(text);
	if (text.IsEmpty())
	{
		::AfxMessageBox("Please select an object, or type in an object name.");
		return;
	}
	
	CDialog::OnOK();
}
