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

// BoneMgrDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "BoneMgrDialog.h"
#include "htree.h"
#include "assetmgr.h"
#include "Utils.h"
#include "MainFrm.h"
#include "W3DViewDoc.h"
#include "DataTreeView.h"
//#include "HModel.H"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////
//
//  BoneMgrDialogClass
//
BoneMgrDialogClass::BoneMgrDialogClass
(
	RenderObjClass *prender_obj,
	CWnd *pparent
)
	: m_pBaseModel (prender_obj),
	  m_pBackupModel (NULL),
	  m_bAttach (true),
	  CDialog (BoneMgrDialogClass::IDD, pparent)
{
	//{{AFX_DATA_INIT(BoneMgrDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


///////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
BoneMgrDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BoneMgrDialogClass)
	DDX_Control(pDX, IDC_OBJECT_COMBO, m_ObjectCombo);
	DDX_Control(pDX, IDC_BONE_TREE, m_BoneTree);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(BoneMgrDialogClass, CDialog)
	//{{AFX_MSG_MAP(BoneMgrDialogClass)
	ON_NOTIFY(TVN_SELCHANGED, IDC_BONE_TREE, OnSelchangedBoneTree)
	ON_CBN_SELCHANGE(IDC_OBJECT_COMBO, OnSelchangeObjectCombo)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ATTACH_BUTTON, OnAttachButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
BoneMgrDialogClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();

	// Make a backup of this hierarchy in case we need to restore it.
	m_pBackupModel = m_pBaseModel->Clone ();

	// Create an icon imagelist for the tree control
	CImageList *pimagelist = new CImageList;
	pimagelist->Create (16, 16, ILC_COLOR | ILC_MASK, 2, 2);

	// Load this icon and add it to our imagelist
	pimagelist->Add ((HICON)::LoadImage (::AfxGetResourceHandle (),
													 MAKEINTRESOURCE (IDI_FOLDER),
													 IMAGE_ICON,
													 16,
													 16,
													 LR_SHARED));

	pimagelist->Add ((HICON)::LoadImage (::AfxGetResourceHandle (),
													 MAKEINTRESOURCE (IDI_OBJECT),
													 IMAGE_ICON,
													 16,
													 16,
													 LR_SHARED));

	m_BoneTree.SetImageList (pimagelist, TVSIL_NORMAL);

	// Get the hierarchy tree for this model so we can enumerate bone's
	// and subobjects.
	HTREEITEM hfirst_item = NULL;

	// Loop through all the bones in this model
	int bone_count = m_pBaseModel->Get_Num_Bones ();
	int index = 0;
	for (; index < bone_count; index ++) {			
		const char *pbone_name = m_pBaseModel->Get_Bone_Name (index);

		// Add this bone to the tree control
		HTREEITEM hbone_item = m_BoneTree.InsertItem (pbone_name, 0, 0);
		Fill_Bone_Item (hbone_item, index);			

		// Is this the first item we've added to the tree?
		if (hfirst_item == NULL) {
			hfirst_item = hbone_item;
		}
	}

	//
	//	Sort the tree control
	//
	m_BoneTree.SortChildren (TVI_ROOT);

	// Build a list of all the render objects currently loaded
	CW3DViewDoc *pdoc = (CW3DViewDoc *)((CMainFrame *)::AfxGetMainWnd())->GetActiveDocument ();
	CDataTreeView *pdata_tree = pdoc->GetDataTreeView ();	
	DynamicVectorClass <CString> asset_list;
	pdata_tree->Build_Render_Object_List (asset_list);

	// Add this render object list to the combobox
	for (index = 0; index < asset_list.Count (); index ++) {
		m_ObjectCombo.AddString (asset_list[index]);
	}

	// Select the first entry in the combobox
	m_ObjectCombo.SetCurSel (0);
	OnSelchangeObjectCombo ();	

	// Select the first item in the tree
	m_BoneTree.SelectItem (hfirst_item);
	return TRUE;
}


///////////////////////////////////////////////////////////////
//
//  Fill_Bone_Item
//
void
BoneMgrDialogClass::Fill_Bone_Item
(
	HTREEITEM hbone_item,
	int bone_index
)
{
	// Create a new instance of the hmodel which we can use
	// to compare with the supplied hmodel and determine
	// which 'bones-models' are new.
	const char *orig_model_name = m_pBaseModel->Get_Base_Model_Name ();
	orig_model_name = (orig_model_name == NULL) ? m_pBaseModel->Get_Name () : orig_model_name;
	RenderObjClass *porig_model = WW3DAssetManager::Get_Instance()->Create_Render_Obj (orig_model_name);

	// Build a list of nodes that are contained in the vanilla model
	DynamicVectorClass <RenderObjClass *> orig_node_list;
	int index = 0;
	for (;
		  index < porig_model->Get_Num_Sub_Objects_On_Bone (bone_index);
		  index ++) {
		RenderObjClass *psubobj = porig_model->Get_Sub_Object_On_Bone (index, bone_index);
		if (psubobj != NULL) {
			orig_node_list.Add (psubobj);
		}
	}

	// Build a list of nodes that are contained in this bone
	DynamicVectorClass <RenderObjClass *> node_list;
	for (index = 0;
		  index < m_pBaseModel->Get_Num_Sub_Objects_On_Bone (bone_index);
		  index ++) {
		RenderObjClass *psubobj = m_pBaseModel->Get_Sub_Object_On_Bone (index, bone_index);
		if (psubobj != NULL) {
			node_list.Add (psubobj);
		}
	}

	if (node_list.Count () > 0) {
		
		// Add the subobjects to the tree control
		for (int node_index = 0; node_index < node_list.Count (); node_index ++) {
			RenderObjClass *psubobject = node_list[node_index];
			ASSERT (psubobject != NULL);
			
			// Is this subobject new?  (i.e. not in a 'vanilla' instance?)
			if (psubobject != NULL &&
				 (Is_Object_In_List (psubobject->Get_Name (), orig_node_list) == false)) {				
				m_BoneTree.InsertItem (psubobject->Get_Name (), 1, 1, hbone_item);
			}
		}
	}

	// Free our hold on the render objs in the original node list
	for (index = 0; index < orig_node_list.Count (); index ++) {
		MEMBER_RELEASE (orig_node_list[index]);
	}

	// Free our hold on the render objs in the node list
	for (index = 0; index < node_list.Count (); index ++) {
		MEMBER_RELEASE (node_list[index]);
	}
	
	MEMBER_RELEASE (porig_model);
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Is_Object_In_List
//
bool
BoneMgrDialogClass::Is_Object_In_List
(
	const char *passet_name,
	DynamicVectorClass <RenderObjClass *> &node_list
)
{
	// Assume failure
	bool retval = false;

	// Loop through the nodes in the list until we've found the one
	// were are looking for.
	for (int node_index = 0; (node_index < node_list.Count ()) && (retval == false); node_index ++) {
		RenderObjClass *prender_obj = node_list[node_index];
		
		// Is this the render object we were looking for?
		if (prender_obj != NULL &&
		    ::lstrcmpi (prender_obj->Get_Name (), passet_name) == 0) {
			retval = true;
		}
	}	

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	OnSelchangedBoneTree
//
void
BoneMgrDialogClass::OnSelchangedBoneTree
(
	NMHDR *pNMHDR,
	LRESULT *pResult
)
{
	// Make the dialog controls reflect the new selection
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	Update_Controls (pNMTreeView->itemNew.hItem);

	(*pResult) = 0;
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	OnSelchangeObjectCombo
//
void
BoneMgrDialogClass::OnSelchangeObjectCombo (void)
{
	// Get the name of the currently selected render object
	CString name;
	int index = m_ObjectCombo.GetCurSel ();
	m_ObjectCombo.GetLBText (index, name);

	// Change the text of the 'Attach' button based on whether or not
	// this render object is already attached to the current bone.
	if (Is_Render_Obj_Already_Attached (name)) {
		SetDlgItemText (IDC_ATTACH_BUTTON, "&Remove");
		m_bAttach = false;
	} else {
		SetDlgItemText (IDC_ATTACH_BUTTON, "&Attach");
		m_bAttach = true;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Is_Render_Obj_Already_Attached
//
bool
BoneMgrDialogClass::Is_Render_Obj_Already_Attached (const CString &name)
{
	// Assume not already attached
	bool retval = false;

	HTREEITEM htree_item = m_BoneTree.GetSelectedItem ();
	HTREEITEM hparent_item = m_BoneTree.GetParentItem (htree_item);
	htree_item = (hparent_item != NULL) ? hparent_item : htree_item;
	if (htree_item != NULL) {
		
		// Loop through all the children of this bone
		for (HTREEITEM hchild_item = m_BoneTree.GetChildItem (htree_item);
			  (hchild_item != NULL) && (retval == false);
			  hchild_item = m_BoneTree.GetNextSiblingItem (hchild_item)) {
			
			// Is this the render object we were looking for?
			CString child_name = m_BoneTree.GetItemText (hchild_item);
			if (name.CompareNoCase (child_name) == 0) {
				retval = true;
			}
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Update_Controls
//
void
BoneMgrDialogClass::Update_Controls (HTREEITEM selected_item)
{
	// Get the name of the currently selected item
	CString name = m_BoneTree.GetItemText (selected_item);
	bool bis_bone = (m_BoneTree.GetParentItem (selected_item) == NULL);

	// Did the user select a bone name?
	if (bis_bone) {
		m_BoneName = name;
	} else {

		// Select this render object in the combobox
		int index = m_ObjectCombo.FindStringExact (-1, name);
		m_ObjectCombo.SetCurSel ((index != CB_ERR) ? index : 0);		

		// The bone name is the name of the parent item of the selected item.
		m_BoneName = m_BoneTree.GetItemText (m_BoneTree.GetParentItem (selected_item));
	}

	OnSelchangeObjectCombo ();

	// Change the text of the group box to reflect the bone name
	CString text;
	text.Format ("Bone: %s", static_cast<const char*>(m_BoneName));
	SetDlgItemText (IDC_BONE_GROUPBOX, text);
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	OnDestroy
//
void
BoneMgrDialogClass::OnDestroy (void)
{		
	// Free the state image list we associated with the control
	CImageList *pimagelist = m_BoneTree.GetImageList (TVSIL_NORMAL);
	m_BoneTree.SetImageList (NULL, TVSIL_NORMAL);
	SAFE_DELETE (pimagelist);
	
	// Allow the base class to process this message
	CDialog::OnDestroy ();
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
void
BoneMgrDialogClass::OnOK (void)
{
	// Simply forget about the backup we made
	MEMBER_RELEASE (m_pBackupModel);
	
	// Update the hierarchy's cached information to reflect the new settings
	CW3DViewDoc *pdoc = (CW3DViewDoc *)((CMainFrame *)::AfxGetMainWnd())->GetActiveDocument ();
	pdoc->Update_Aggregate_Prototype (*m_pBaseModel);

	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	OnCancel
//
void
BoneMgrDialogClass::OnCancel (void)
{
	CWaitCursor wait_cursor;

	// Display the backup hierarchy
	CW3DViewDoc *pdoc = (CW3DViewDoc *)((CMainFrame *)::AfxGetMainWnd())->GetActiveDocument ();
	pdoc->DisplayObject (m_pBackupModel, false, false);

	// Allow the base class to process this message
	CDialog::OnCancel ();
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	OnAttachButton
//
void
BoneMgrDialogClass::OnAttachButton (void) 
{
	// Get the name of the currently selected render object
	CString name;
	int index = m_ObjectCombo.GetCurSel ();
	m_ObjectCombo.GetLBText (index, name);

	// Lookup the currently selected bone item
	HTREEITEM hbone_item = Get_Current_Bone_Item ();

	// Should we attach or remove the render object?
	if (m_bAttach) {
		
		// Create an instance of the render object and attach it to the bone
		RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (name);
		if (prender_obj != NULL) {
			m_pBaseModel->Add_Sub_Object_To_Bone (prender_obj, m_BoneName);
			m_BoneTree.InsertItem (name, 1, 1, hbone_item);
			MEMBER_RELEASE (prender_obj);
		}

	} else {

		// Loop through all the subobjects on the bone
		bool found = false;
		int bone_index = m_pBaseModel->Get_Bone_Index (m_BoneName);
		int count = m_pBaseModel->Get_Num_Sub_Objects_On_Bone (bone_index);
		for (int index = 0; (index < count) && !found; index ++) {
			
			// Is this the subobject we were looking for?
			RenderObjClass *psub_obj = m_pBaseModel->Get_Sub_Object_On_Bone (index, bone_index);
			if ((psub_obj != NULL) &&
				 (::lstrcmpi (psub_obj->Get_Name (), name) == 0)) {
				
				// Remove this subobject from the bone
				m_pBaseModel->Remove_Sub_Object (psub_obj);
				found = true;
			}

			// Release our hold on this pointer
			MEMBER_RELEASE (psub_obj);
		}
		
		// Remove the object from our UI
		Remove_Object_From_Bone (hbone_item, name);
	}

	// Refresh the UI state
	m_BoneTree.InvalidateRect (NULL, TRUE);
	m_BoneTree.UpdateWindow ();
	Update_Controls (hbone_item);
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Current_Bone_Item
//
HTREEITEM
BoneMgrDialogClass::Get_Current_Bone_Item (void)
{
	// Get the currently selected item and its parent
	HTREEITEM htree_item = m_BoneTree.GetSelectedItem ();
	HTREEITEM hparent_item = m_BoneTree.GetParentItem (htree_item);
	
	// Return the bone item
	return (hparent_item != NULL) ? hparent_item : htree_item;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Remove_Object_From_Bone
//
void
BoneMgrDialogClass::Remove_Object_From_Bone
(
	HTREEITEM bone_item,
	const CString &name
)
{
	// Loop through all the children of this bone
	for (HTREEITEM hchild_item = m_BoneTree.GetChildItem (bone_item);
		  (hchild_item != NULL);
		  hchild_item = m_BoneTree.GetNextSiblingItem (hchild_item)) {
		
		// Is this the render object we were looking for?
		CString child_name = m_BoneTree.GetItemText (hchild_item);
		if (name.CompareNoCase (child_name) == 0) {
			m_BoneTree.DeleteItem (hchild_item);
			break ;
		}		
	}
		
	return ;
}

