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

// MeshPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "MeshPropPage.h"
#include "rendobj.h"
#include "assetmgr.h"
#include "mesh.h"
#include "meshmdl.h"
#include "w3d_file.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMeshPropPage property page

IMPLEMENT_DYNCREATE(CMeshPropPage, CPropertyPage)


////////////////////////////////////////////////////////////////
//
//  CMeshPropPage
//
CMeshPropPage::CMeshPropPage (const CString &stringMeshName)
    : CPropertyPage(CMeshPropPage::IDD)
{
	//{{AFX_DATA_INIT(CMeshPropPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_stringMeshName = stringMeshName;
    return ;
}

////////////////////////////////////////////////////////////////
//
//  ~CMeshPropPage
//
CMeshPropPage::~CMeshPropPage (void)
{
    return ;
}

////////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CMeshPropPage::DoDataExchange (CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMeshPropPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CMeshPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMeshPropPage)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeshPropPage message handlers

////////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CMeshPropPage::OnInitDialog (void)
{
	// Allow the base class to process this message
    CPropertyPage::OnInitDialog();
	
    if (m_stringMeshName.GetLength () > 0)
    {
        // Get a pointer to the mesh object from the asset manager
        MeshClass *pCMesh = (MeshClass *)WW3DAssetManager::Get_Instance()->Create_Render_Obj (m_stringMeshName);
        
        ASSERT (pCMesh);
        if (pCMesh)
        {
            CString stringDesc;
            stringDesc.Format (IDS_MESH_PROP_DESC, static_cast<const char*>(m_stringMeshName));

            // Put the description onto the dialog
            SetDlgItemText (IDC_DESCRIPTION, stringDesc);

            // Put the polygon count onto the dialog
            SetDlgItemInt (IDC_POLYGON_COUNT, pCMesh->Get_Num_Polys ());

            MeshModelClass *pmeshmodel = pCMesh->Get_Model ();
            ASSERT (pmeshmodel);
            if (pmeshmodel)
            {
                // Put the vertex count onto the dialog
                SetDlgItemInt (IDC_VERTEX_COUNT, pmeshmodel->Get_Vertex_Count ());
            }

            // Put the user text onto the dialog
            SetDlgItemText (IDC_USER_TEXT, pCMesh->Get_User_Text ());

            // Get the flags for the mesh
            DWORD dwFlags = pCMesh->Get_W3D_Flags ();

            // Determine what type of mesh this is
            if ((dwFlags & W3D_MESH_FLAG_COLLISION_BOX) == W3D_MESH_FLAG_COLLISION_BOX)
            {
                SendDlgItemMessage (IDC_MESH_TYPE_COLLISION_BOX, BM_SETCHECK, (WPARAM)TRUE);
            }
            else if ((dwFlags & W3D_MESH_FLAG_SKIN) == W3D_MESH_FLAG_SKIN)
            {
                SendDlgItemMessage (IDC_MESH_TYPE_SKIN, BM_SETCHECK, (WPARAM)TRUE);
            }
            else if ((dwFlags & W3D_MESH_FLAG_SHADOW) == W3D_MESH_FLAG_SHADOW)
            {
                SendDlgItemMessage (IDC_MESH_TYPE_SHADOW, BM_SETCHECK, (WPARAM)TRUE);
            }
            else
            {
                SendDlgItemMessage (IDC_MESH_TYPE_NORMAL, BM_SETCHECK, (WPARAM)TRUE);
            }


            // Is this collision type physical?
            DWORD dwCollisionFlags = dwFlags & W3D_MESH_FLAG_COLLISION_TYPE_MASK;
            if ((dwCollisionFlags & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) == W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL)
            {
                SendDlgItemMessage (IDC_COLLISION_TYPE_PHYSICAL, BM_SETCHECK, (WPARAM)TRUE);
            }
            
            // Is this collision type projectile?
            if ((dwCollisionFlags & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) == W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE)
            {
                SendDlgItemMessage (IDC_COLLISION_TYPE_PROJECTILE, BM_SETCHECK, (WPARAM)TRUE);
            }

            // Is this a hidden mesh?
            if ((dwFlags & W3D_MESH_FLAG_HIDDEN) == W3D_MESH_FLAG_HIDDEN)
            {
                SendDlgItemMessage (IDC_HIDDEN, BM_SETCHECK, (WPARAM)TRUE);
            }

            // Free the object
            pCMesh->Release_Ref ();
            pCMesh = NULL;
        }
    }

    GetParent ()->GetDlgItem (IDOK)->ShowWindow (SW_HIDE);
    GetParent ()->GetDlgItem (IDCANCEL)->SetWindowText ("Close");	
	return TRUE;
}


void CMeshPropPage::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CPropertyPage::OnClose();
    return ;
}
