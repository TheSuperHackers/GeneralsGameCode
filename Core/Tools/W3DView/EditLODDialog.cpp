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

// EditLODDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "EditLODDialog.h"
#include "distlod.h"
#include "Utils.h"
#include "rendobj.h"
#include "W3DViewDoc.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////
//
//  Local Constants
//
const int COL_NAME          = 0;
const int COL_SWITCH_UP     = 1;
const int COL_SWITCH_DN     = 2;


/////////////////////////////////////////////////////////////
//
//  CEditLODDialog
//
CEditLODDialog::CEditLODDialog(CWnd* pParent /*=NULL*/)
	: m_spinIncrement (0.5F),
      CDialog(CEditLODDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditLODDialog)
	//}}AFX_DATA_INIT
    return ;
}

/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CEditLODDialog::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
    CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CEditLODDialog)
	DDX_Control(pDX, IDC_SWITCH_UP_SPIN, m_switchUpSpin);
	DDX_Control(pDX, IDC_SWITCH_DN_SPIN, m_switchDownSpin);
	DDX_Control(pDX, IDC_HIERARCHY_LIST, m_hierarchyListCtrl);
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CEditLODDialog, CDialog)
	//{{AFX_MSG_MAP(CEditLODDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SWITCH_UP_SPIN, OnDeltaposSwitchUpSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SWITCH_DN_SPIN, OnDeltaposSwitchDnSpin)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_HIERARCHY_LIST, OnItemChangedHierarchyList)
	ON_EN_UPDATE(IDC_SWITCH_DN_EDIT, OnUpdateSwitchDnEdit)
	ON_EN_UPDATE(IDC_SWITCH_UP_EDIT, OnUpdateSwitchUpEdit)
	ON_BN_CLICKED(IDC_RECALC, OnRecalc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CEditLODDialog::OnInitDialog (void) 
{
	// Allow the base class to process this message
    CDialog::OnInitDialog ();

    // Center the dialog around the data tree view instead
    // of the direct center of the screen
    ::CenterDialogAroundTreeView (m_hWnd);

    // Get a pointer to the doc
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        // Get the current LOD from the doc
        DistLODClass *pLOD = (DistLODClass *)pCDoc->GetDisplayedObject ();
        ASSERT (pLOD);
        if (pLOD)
        {
            int iSubObjects = pLOD->Get_Num_Sub_Objects ();

            // Add the columns to the list control
            m_hierarchyListCtrl.InsertColumn (COL_NAME, "Name");
            m_hierarchyListCtrl.InsertColumn (COL_SWITCH_UP, "Switch Up");
            m_hierarchyListCtrl.InsertColumn (COL_SWITCH_DN, "Switch Down");

            RenderObjClass *pfirst_subobj = pLOD->Get_Sub_Object (0);
				if (pfirst_subobj != NULL) {
					m_spinIncrement = pfirst_subobj->Get_Bounding_Sphere ().Radius / 5.0F;
					MEMBER_RELEASE (pfirst_subobj);
				}
            
            // Loop through all the subobjects
            for (int iObject = 0;
                 (iObject < iSubObjects);
                 iObject ++)
            {
                // Get this subobject
                RenderObjClass *pCSubObject = pLOD->Get_Sub_Object (iObject);
                if (pCSubObject)
                {
                    // Add this object to the list
                    int iIndex = m_hierarchyListCtrl.InsertItem (COL_NAME, pCSubObject->Get_Name ());

                    CString stringTemp;
                    stringTemp.Format ("%.2f", pLOD->Get_Switch_Up_Dist (iObject));
                    m_hierarchyListCtrl.SetItemText (iIndex, COL_SWITCH_UP, stringTemp);

                    stringTemp.Format ("%.2f", pLOD->Get_Switch_Down_Dist (iObject));
                    m_hierarchyListCtrl.SetItemText (iIndex, COL_SWITCH_DN, stringTemp);

                    // Free this object
						  MEMBER_RELEASE (pCSubObject);
                }
            }

            m_switchUpSpin.SetRange (-20, UD_MAXVAL-20);
            m_switchDownSpin.SetRange (-20, UD_MAXVAL-20);
            
            // Resize the columns so they are wide enough to display the largest string
            m_hierarchyListCtrl.SetColumnWidth (COL_NAME, LVSCW_AUTOSIZE);
            m_hierarchyListCtrl.SetColumnWidth (COL_SWITCH_UP, LVSCW_AUTOSIZE_USEHEADER);
            m_hierarchyListCtrl.SetColumnWidth (COL_SWITCH_DN, LVSCW_AUTOSIZE_USEHEADER);

            // Select the first item in the list
            m_hierarchyListCtrl.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED);
        }
    }
	
	return TRUE;
}

/////////////////////////////////////////////////////////////
//
//  OnOK
//
void
CEditLODDialog::OnOK (void) 
{

    // Get a pointer to the doc
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        // Get the current LOD from the doc
        DistLODClass *pLOD = (DistLODClass *)pCDoc->GetDisplayedObject ();
        ASSERT (pLOD);
        if (pLOD)
        {
            int iSubObjects = pLOD->Get_Num_Sub_Objects ();

            // Loop through all the subobjects and add them to the list control
            for (int iObject = 0;
                 (iObject < iSubObjects);
                 iObject ++)
            {
                // Get the switch up distance from the list control
                CString stringTemp = m_hierarchyListCtrl.GetItemText (iObject, COL_SWITCH_UP);
                
                // Convert the string to a float and pass this value
                // onto the LOD manager
                float switchDistance = ::atof (stringTemp);
                pLOD->Set_Switch_Up_Dist (iObject, switchDistance);

                // Get the switch down distance from the list control
                stringTemp = m_hierarchyListCtrl.GetItemText (iObject, COL_SWITCH_DN);
                
                // Convert the string to a float and pass this value
                // onto the LOD manager
                switchDistance = ::atof (stringTemp);
                pLOD->Set_Switch_Down_Dist (iObject, switchDistance);
            }
        }
    }    

	// Allow the base class to process this message
    CDialog::OnOK ();
    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnCancel
//
void
CEditLODDialog::OnCancel (void) 
{
	// Allow the base class to process this message
    CDialog::OnCancel ();
    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnDeltaposSwitchUpSpin
//
void
CEditLODDialog::OnDeltaposSwitchUpSpin
(
    NMHDR* pNMHDR,
    LRESULT* pResult
)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
    if (pNMUpDown)
    {
        float newVal = float(pNMUpDown->iPos) / 10.00F;

        // Change the switching distance in the edit control
        CString stringTemp;
        stringTemp.Format ("%.2f", newVal);
        SetDlgItemText (IDC_SWITCH_UP_EDIT, stringTemp);

        // Find the selected item in the list control
        int iIndex = m_hierarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
        if (iIndex != -1)
        {
            // Change the switching distance in the list control
            m_hierarchyListCtrl.SetItemText (iIndex, COL_SWITCH_UP, stringTemp);
        }
    }
	
	*pResult = 0;
    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnDeltaposSwitchDnSpin
//
void
CEditLODDialog::OnDeltaposSwitchDnSpin
(
    NMHDR* pNMHDR,
    LRESULT* pResult
)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
    if (pNMUpDown)
    {
        float newVal = float(pNMUpDown->iPos) / 10.00F;

        // Change the switching distance in the edit control
        CString stringTemp;
        stringTemp.Format ("%.2f", newVal);
        SetDlgItemText (IDC_SWITCH_DN_EDIT, stringTemp);

        // Find the selected item in the list control
        int iIndex = m_hierarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
        if (iIndex != -1)
        {
            // Change the switching distance in the list control
            m_hierarchyListCtrl.SetItemText (iIndex, COL_SWITCH_DN, stringTemp);
        }
    }
	
	*pResult = 0;
    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnItemChangedHierarchyList
//
void
CEditLODDialog::OnItemChangedHierarchyList
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
               
        if ((pNMListView->uNewState & LVIS_SELECTED) != LVIS_SELECTED)
        {
            // Is there a selected item in the list control?
            if (m_hierarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED) == -1)
            {
                // Disabled the edit and spin controls
                EnableControls (FALSE);                
            }
        }
        else
        {
            // Enable the edit and spin controls
            EnableControls (TRUE);

            // Load the control with data for the selected item.            
            ResetControls (pNMListView->iItem);            
        }
    }
	
	*pResult = 0;
    return ;
}

/////////////////////////////////////////////////////////////
//
//  ResetControls
//
void
CEditLODDialog::ResetControls (int iIndex)
{
    //
    // Set the text of the group box
    //
    CString stringTemp = m_hierarchyListCtrl.GetItemText (iIndex, COL_NAME);

    // Set the text of the group box
    CString stringTitle;
    stringTitle.Format ("Settings (%s)", (LPCTSTR)stringTemp);
    SetDlgItemText (IDC_SETTINGS_GROUP, stringTitle);

    //
    // Reset the switch up controls
    //

    // Get the switch up distance from the list control
    stringTemp = m_hierarchyListCtrl.GetItemText (iIndex, COL_SWITCH_UP);

    // Set the text of the edit control to reflect the switching distance
    SetDlgItemText (IDC_SWITCH_UP_EDIT, stringTemp);
    
    // Set the current position of the spin control
    float switchDistance = ::atof (stringTemp);
    m_switchUpSpin.SetPos (int(switchDistance * 10.00F));

    //
    // Reset the switch down controls
    //

    // Get the switch down distance from the list control
    stringTemp = m_hierarchyListCtrl.GetItemText (iIndex, COL_SWITCH_DN);

    // Set the text of the edit control to reflect the switching distance
    SetDlgItemText (IDC_SWITCH_DN_EDIT, stringTemp);

    // Set the current position of the spin control
    switchDistance = ::atof (stringTemp);
    m_switchDownSpin.SetPos (int(switchDistance * 10.00F));
    return ;
}

/////////////////////////////////////////////////////////////
//
//  EnableControls
//
void
CEditLODDialog::EnableControls (BOOL bEnable)
{    
    // Enable or disable the windows
    ::EnableWindow (::GetDlgItem (m_hWnd, IDC_SETTINGS_GROUP), bEnable);
    ::EnableWindow (::GetDlgItem (m_hWnd, IDC_SWITCH_UP_SPIN), bEnable);
    ::EnableWindow (::GetDlgItem (m_hWnd, IDC_SWITCH_UP_EDIT), bEnable);
    ::EnableWindow (::GetDlgItem (m_hWnd, IDC_SWITCH_DN_SPIN), bEnable);
    ::EnableWindow (::GetDlgItem (m_hWnd, IDC_SWITCH_DN_EDIT), bEnable);    
    ::EnableWindow (::GetDlgItem (m_hWnd, IDC_RECALC), bEnable); 
    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnUpdateSwitchDnEdit
//
void
CEditLODDialog::OnUpdateSwitchDnEdit (void) 
{    
    // Get the switching distance from the edit control
    CString stringTemp;
    GetDlgItemText (IDC_SWITCH_DN_EDIT, stringTemp);
    float newVal = ::atof (stringTemp);

    // Change the switching distance in the spin control
    m_switchDownSpin.SetPos (int(newVal * 10.00F));

    // Find the selected item in the list control
    int iIndex = m_hierarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
    if (iIndex != -1)
    {
        // Change the switching distance in the list control
        m_hierarchyListCtrl.SetItemText (iIndex, COL_SWITCH_DN, stringTemp);
    }

	return ;
}

/////////////////////////////////////////////////////////////
//
//  OnUpdateSwitchUpEdit
//
void CEditLODDialog::OnUpdateSwitchUpEdit (void)
{
    // Get the switching distance from the edit control
    CString stringTemp;
    GetDlgItemText (IDC_SWITCH_UP_EDIT, stringTemp);
    float newVal = ::atof (stringTemp);

    // Change the switching distance in the spin control
    m_switchUpSpin.SetPos (int(newVal * 10.00F));    

    // Find the selected item in the list control
    int iIndex = m_hierarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
    if (iIndex != -1)
    {
        // Change the switching distance in the list control
        m_hierarchyListCtrl.SetItemText (iIndex, COL_SWITCH_UP, stringTemp);
    }

	return ;
}

/////////////////////////////////////////////////////////////
//
//  OnRecalc
//
void CEditLODDialog::OnRecalc (void)
{
    // Get the up switching distance from the edit control
    CString stringTemp;
    GetDlgItemText (IDC_SWITCH_UP_EDIT, stringTemp);
    float switchUp = ::atof (stringTemp);

    // Get the down switching distance from the edit control
    stringTemp;
    GetDlgItemText (IDC_SWITCH_DN_EDIT, stringTemp);
    float switchDown = ::atof (stringTemp);

    if (switchUp < switchDown)
    {
        // Calculate the current range
        float switchDelta = switchDown - switchUp;
        float switchOverlap = switchDelta * 0.1F;

        // Get a pointer to the doc
        CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
        if (pCDoc)
        {
            // Get the current LOD from the doc
            DistLODClass *pLOD = (DistLODClass *)pCDoc->GetDisplayedObject ();
            ASSERT (pLOD);
            if (pLOD)
            {
                int iSubObjects = pLOD->Get_Num_Sub_Objects ();
                switchUp = switchDown - switchOverlap;

                // Loop through all the subobjects (following the highlighted one)
                for (int iObject = m_hierarchyListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED) + 1;
                     (iObject < iSubObjects);
                     iObject ++)
                {
                    // Set the text of the switch up column
                    CString stringTemp;
                    stringTemp.Format ("%.2f", switchUp);
                    m_hierarchyListCtrl.SetItemText (iObject, COL_SWITCH_UP, stringTemp);

                    // Set the text of the switch dn column
                    stringTemp.Format ("%.2f", switchUp+switchDelta);
                    m_hierarchyListCtrl.SetItemText (iObject, COL_SWITCH_DN, stringTemp);

                    // Add the range to the switch up distance
                    switchUp += switchDelta-switchOverlap;
                }
            }
        }
    }

    return ;
}
