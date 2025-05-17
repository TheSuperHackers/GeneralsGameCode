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

// AnimationPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "AnimationPropPage.h"
#include "RendObj.H"
#include "AssetMgr.H"
#include "Mesh.H"
#include "W3DViewDoc.H"
#include "Utils.H"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimationPropPage property page

IMPLEMENT_DYNCREATE(CAnimationPropPage, CPropertyPage)


////////////////////////////////////////////////////////////////
//
//  CAnimationPropPage
//
CAnimationPropPage::CAnimationPropPage (void)
    : CPropertyPage(CAnimationPropPage::IDD)
{
	//{{AFX_DATA_INIT(CAnimationPropPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    return ;
}

////////////////////////////////////////////////////////////////
//
//  CAnimationPropPage
//
CAnimationPropPage::~CAnimationPropPage (void)
{
    return ;
}

////////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CAnimationPropPage::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
    CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationPropPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CAnimationPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAnimationPropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CAnimationPropPage::OnInitDialog (void) 
{
	// Allow the base class to process this message
    CPropertyPage::OnInitDialog ();

    // Get a pointer to the doc so we can get at the current animation object
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc && pCDoc->GetCurrentAnimation ())
    {
        HAnimClass *pCAnimation = pCDoc->GetCurrentAnimation ();
        
        // Set the description text at the top of the dialog
        CString stringTemp;
        stringTemp.Format (IDS_ANI_PROP_DESC, pCAnimation->Get_Name ());
        SetDlgItemText (IDC_DESCRIPTION, stringTemp);
        
        // Fill in the number of frames
        SetDlgItemInt (IDC_FRAME_COUNT, pCAnimation->Get_Num_Frames ());
        
        // Fill in the frame rate of the animation
        stringTemp.Format ("%.2f fps", pCAnimation->Get_Frame_Rate ());
        SetDlgItemText (IDC_FRAME_RATE, stringTemp);

        // Fill in the total time taken by the animation
        stringTemp.Format ("%.3f seconds", pCAnimation->Get_Total_Time ());
        SetDlgItemText (IDC_TOTAL_TIME, stringTemp);

        // Set the name of the hierarchy this animation belongs to.
        SetDlgItemText (IDC_HIERARCHY_NAME, pCAnimation->Get_HName ());

    }
	
	return TRUE;
}
