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

// BackgroundObjectDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "BackgroundObjectDialog.h"
#include "assetmgr.h"
#include "Utils.h"
#include "W3DViewDoc.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////
//
//  CBackgroundObjectDialog
//
CBackgroundObjectDialog::CBackgroundObjectDialog (CWnd* pParent /*=NULL*/)
	: CDialog(CBackgroundObjectDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBackgroundObjectDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    return ;
}

/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CBackgroundObjectDialog::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBackgroundObjectDialog)
	DDX_Control(pDX, IDC_HIERARCHY_LIST, m_heirarchyListCtrl);
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CBackgroundObjectDialog, CDialog)
	//{{AFX_MSG_MAP(CBackgroundObjectDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_HIERARCHY_LIST, OnItemChangedHierarchyList)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBackgroundObjectDialog message handlers

/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CBackgroundObjectDialog::OnInitDialog (void)
{
    // Allow the base class to process this message
	CDialog::OnInitDialog ();
	
    // Center the dialog around the data tree view instead
    // of the direct center of the screen
    ::CenterDialogAroundTreeView (m_hWnd);

    m_heirarchyListCtrl.InsertColumn (0, "Name");
    //m_heirarchyListCtrl.InsertColumn (1, "Subobjects");

    // Get an iterator from the asset manager that we can
    // use to enumerate the currently loaded assets
    RenderObjIterator *pObjEnum = WW3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator ();

    ASSERT (pObjEnum);
    if (pObjEnum)
    {
        // Loop through all the assets in the manager
        for (pObjEnum->First ();
             pObjEnum->Is_Done () == FALSE;
             pObjEnum->Next ())
        {
            LPCTSTR pszItemName = pObjEnum->Current_Item_Name ();

            // Is this a hierarchy?
            if (WW3DAssetManager::Get_Instance()->Render_Obj_Exists (pszItemName) &&
                (pObjEnum->Current_Item_Class_ID () == RenderObjClass::CLASSID_HMODEL))
            {
                // Add this hierarchy to the list
                m_heirarchyListCtrl.InsertItem (0, pszItemName);
            }
        }

        // Free the enumerator object we created earlier
        delete pObjEnum;
        pObjEnum = NULL;
    }

    // Get a pointer to the doc
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        // Get the name of the current background object
        CString stringCurrObject = pCDoc->GetBackgroundObjectName ();
        
        LV_FINDINFO findInfo = { 0 };
        findInfo.flags = LVFI_STRING;
        findInfo.psz = (LPCTSTR)stringCurrObject;

        // Attempt to find this item in the list control
        int iIndex = m_heirarchyListCtrl.FindItem (&findInfo);
        if (iIndex != -1)
        {
            // Select the item in the list control
            m_heirarchyListCtrl.SetItemState (iIndex, LVNI_SELECTED, LVNI_SELECTED);
            SetDlgItemText (IDC_CURR_OBJ, m_heirarchyListCtrl.GetItemText (iIndex, 0));
        }
        else
        {
            // Select the first item in the list control
            m_heirarchyListCtrl.SetItemState (0, LVNI_SELECTED, LVNI_SELECTED);
            SetDlgItemText (IDC_CURR_OBJ, m_heirarchyListCtrl.GetItemText (0, 0));
        }
    }    

    // Size the columns so they are large enough to display their contents
    m_heirarchyListCtrl.SetColumnWidth (0, LVSCW_AUTOSIZE);
    //m_heirarchyListCtrl.SetColumnWidth (1, LVSCW_AUTOSIZE_USEHEADER);
	return TRUE;
}

/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
void
CBackgroundObjectDialog::OnOK (void)
{
    // Get a pointer to the doc
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        // Get the index of the currently selected item.
        int iIndex = m_heirarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
        if (iIndex != -1)
        {
            // Get the name of the item from the list control
            CString stringItemName = m_heirarchyListCtrl.GetItemText (iIndex, 0);

            // Ask the doc to load the background object and display it
            pCDoc->SetBackgroundObject (stringItemName);
        }
        else
        {
            // Ask the doc to clear the background object
            pCDoc->SetBackgroundObject (NULL);
        }

	    // Allow the base class to process this message
        CDialog::OnOK ();
    }

    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnItemChangedHierarchyList
//
void
CBackgroundObjectDialog::OnItemChangedHierarchyList
(
    NMHDR* pNMHDR,
    LRESULT* pResult
)
{
	// Did the 'state' of the entry change?
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if (pNMListView &&
        (pNMListView->uChanged & LVIF_STATE) == LVIF_STATE)
    {
        // Is the new state a selected state?
        if ((pNMListView->uNewState & LVIS_SELECTED) != LVIS_SELECTED)
        {
            // Is there a selected item in the list control?
            if (m_heirarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED) == -1)
            {
                // Clear the text of the current object static text control
                SetDlgItemText (IDC_CURR_OBJ, "");
            }
        }
        else
        {
            // Set the text of the current object static text control
            SetDlgItemText (IDC_CURR_OBJ, m_heirarchyListCtrl.GetItemText (pNMListView->iItem, 0));
        }
    }
	
	*pResult = 0;
    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnClear
//
void
CBackgroundObjectDialog::OnClear (void) 
{
    // Get the current selection (if any)
    int iIndex = m_heirarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
    if (iIndex != -1)
    {
        // Clear the selection state from this entry
        m_heirarchyListCtrl.SetItemState (iIndex, 0, LVIS_SELECTED);
    }

	return ;
}
