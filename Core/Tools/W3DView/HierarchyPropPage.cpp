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

// HierarchyPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "HierarchyPropPage.h"
#include "assetmgr.h"
#include "rendobj.h"
#include "AssetPropertySheet.h"
#include "MeshPropPage.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHierarchyPropPage property page

IMPLEMENT_DYNCREATE(CHierarchyPropPage, CPropertyPage)

////////////////////////////////////////////////////////////////
//
//  CHierarchyPropPage
//
CHierarchyPropPage::CHierarchyPropPage (const CString &stringHierarchyName)
    : CPropertyPage(CHierarchyPropPage::IDD)
{
	//{{AFX_DATA_INIT(CHierarchyPropPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_stringHierarchyName = stringHierarchyName;
    return ;
}

////////////////////////////////////////////////////////////////
//
//  CHierarchyPropPage
//
CHierarchyPropPage::~CHierarchyPropPage (void)
{
    return ;
}

////////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CHierarchyPropPage::DoDataExchange (CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHierarchyPropPage)
	DDX_Control(pDX, IDC_SUBOBJECT_LIST, m_subObjectListCtrl);
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CHierarchyPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CHierarchyPropPage)
	ON_NOTIFY(NM_DBLCLK, IDC_SUBOBJECT_LIST, OnDblclkSubObjectList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHierarchyPropPage message handlers

////////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CHierarchyPropPage::OnInitDialog (void)
{
	// Allow the base class to process this message
    CPropertyPage::OnInitDialog();

    if (m_stringHierarchyName.GetLength () > 0)
    {
        // Get a pointer to the hierarchy object from the asset manager
        RenderObjClass *pCHierarchy = WW3DAssetManager::Get_Instance()->Create_Render_Obj (m_stringHierarchyName);

        ASSERT (pCHierarchy);
        if (pCHierarchy)
        {
            CString stringDesc;
            stringDesc.Format (IDS_HIERARCHY_PROP_DESC, static_cast<const char*>(m_stringHierarchyName));

            // Put the description onto the dialog
            SetDlgItemText (IDC_DESCRIPTION, stringDesc);

            // Put the polygon count onto the dialog
            SetDlgItemInt (IDC_TOTAL_POLYGONS, pCHierarchy->Get_Num_Polys ());

            // Put the subobject count onto the dialog
            int iSubObjects = pCHierarchy->Get_Num_Sub_Objects ();
            SetDlgItemInt (IDC_SUBOBJECTS, iSubObjects);

            // Add the name column to the list control
            m_subObjectListCtrl.InsertColumn (0, "Name");

            // Loop through all the subobjects and add them to the list control
            for (int iObject = 0;
                 iObject < iSubObjects;
                 iObject ++)
            {
                // Get this subobject
                RenderObjClass *pCSubObject = pCHierarchy->Get_Sub_Object (iObject);
                if (pCSubObject)
                {
                    // Add this object to the list
                    m_subObjectListCtrl.InsertItem (0, pCSubObject->Get_Name ());

                    // Free this object
                    pCSubObject->Release_Ref ();
                    pCSubObject = nullptr;
                }
            }

            // Resize the column so it is wide enough to display the largest string
            m_subObjectListCtrl.SetColumnWidth (0, LVSCW_AUTOSIZE);

            // Free the object
            pCHierarchy->Release_Ref ();
            pCHierarchy = nullptr;
        }
	}

    GetParent ()->GetDlgItem (IDOK)->ShowWindow (SW_HIDE);
    GetParent ()->GetDlgItem (IDCANCEL)->SetWindowText ("Close");
    return TRUE;
}

////////////////////////////////////////////////////////////////
//
//  OnDblclkSubObjectList
//
void
CHierarchyPropPage::OnDblclkSubObjectList
(
    NMHDR* pNMHDR,
    LRESULT* pResult
)
{
    // Get the currently selected item
    int iIndex = m_subObjectListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
    if (iIndex != -1)
    {
        // Create a one-page property sheet that will display property information
        // for the mesh
        CMeshPropPage meshPropPage (m_subObjectListCtrl.GetItemText (iIndex, 0));
        CAssetPropertySheet propertySheet (IDS_MESH_PROP_TITLE, &meshPropPage, this);

        // Show the property sheet
        propertySheet.DoModal ();
    }

	(*pResult) = 0;
    return ;
}
