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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/TextureMgrDialog.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/01/01 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.H"
#include "W3DView.H"
#include "TextureMgrDialog.H"
#include "Mesh.H"
#include "MatInfo.H"
#include "TextureSettingsDialog.H"
#include "AssetMgr.H"
#include "texture.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef WW3D_DX8

/////////////////////////////////////////////////////////////////////////////
//
// Local constants
//
const int TEXTURE_THUMB_X				= 96;
const int TEXTURE_THUMB_Y				= 96;
const int TEXTURE_THUMBSMALL_X		= 48;
const int TEXTURE_THUMBSMALL_Y		= 48;

const int COL_NAME						= 0;
const int COL_TEXTURES					= 1;
const int COL_DIMENSIONS				= 1;
const int COL_TEXTURE_TYPE				= 2;


/////////////////////////////////////////////////////////////////////////////
//
// TextureMgrDialogClass
//
TextureMgrDialogClass::TextureMgrDialogClass
(
	RenderObjClass *pbase_model,
	CWnd *pParent
)
	: m_pBaseModel (pbase_model),
	  m_bContainsMeshes (true),
	  m_pImageList (NULL),
	  m_pImageListSmall (NULL),
	  m_pTextureImageList (NULL),
	  m_pTextureImageListSmall (NULL),
	  CDialog (TextureMgrDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(TextureMgrDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
TextureMgrDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TextureMgrDialogClass)
	DDX_Control(pDX, IDC_MESH_TEXTURE_LIST_CTRL, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(TextureMgrDialogClass, CDialog)
	//{{AFX_MSG_MAP(TextureMgrDialogClass)
	ON_NOTIFY(NM_DBLCLK, IDC_MESH_TEXTURE_LIST_CTRL, OnDblclkMeshTextureListCtrl)
	ON_NOTIFY(LVN_KEYDOWN, IDC_MESH_TEXTURE_LIST_CTRL, OnKeydownMeshTextureListCtrl)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_BACK, OnBack)
	ON_COMMAND(IDC_DETAILS, OnDetails)
	ON_COMMAND(IDC_LARGE, OnLarge)
	ON_COMMAND(IDC_LIST, OnList)
	ON_COMMAND(IDC_SMALL, OnSmall)
	ON_COMMAND(IDC_PROPAGATE, OnPropagate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// Fill_List_Ctrl_With_Meshes
//
void
TextureMgrDialogClass::Fill_List_Ctrl_With_Meshes (void)
{
	m_ListCtrl.DeleteAllItems ();
	m_ListCtrl.SetImageList (m_pImageList, LVSIL_NORMAL);
	m_ListCtrl.SetImageList (m_pImageListSmall, LVSIL_SMALL);	
	m_ListCtrl.DeleteColumn (COL_TEXTURE_TYPE);	
	m_ListCtrl.DeleteColumn (COL_DIMENSIONS);

	// Add the textures column
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	m_ListCtrl.InsertColumn (COL_TEXTURES, "Textures", LVCFMT_LEFT, (rect.Width () / 2) - 20);

	// Loop through the list of mesh's and add them to the list control
	for (int index = 0; index < m_NodeList.Count (); index ++) {
		
		// Add this mesh to the list control
		TextureListNodeClass *pnode = m_NodeList[index];
		int list_index = m_ListCtrl.InsertItem (0, pnode->Get_Name (), pnode->Get_Icon_Index ());
		if (list_index != -1) {
			
			// Insert the texture count in the second column
			CString texture_string;
			texture_string.Format ("%d textures", pnode->Get_Subobj_List ().Count ());
			if (pnode->Get_Subobj_List ().Count () == 1) {
				texture_string = "1 texture";
			}
			m_ListCtrl.SetItemText (list_index, COL_TEXTURES, texture_string);

			// Associate the node with the list entry
			m_ListCtrl.SetItemData (list_index, (DWORD)pnode);
		}
	}

	m_bContainsMeshes = true;
	m_Toolbar.Enable_Button (IDC_BACK, false);
	m_Toolbar.Enable_Button (IDC_PROPAGATE, true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_List_Ctrl_With_Textures
//
void
TextureMgrDialogClass::Fill_List_Ctrl_With_Textures (TextureListNodeClass &parent_node)
{	
	m_ListCtrl.DeleteAllItems ();
	m_ListCtrl.SetImageList (m_pTextureImageList, LVSIL_NORMAL);
	m_ListCtrl.SetImageList (m_pTextureImageListSmall, LVSIL_SMALL);
	m_ListCtrl.DeleteColumn (COL_TEXTURES);

	// Add the textures column
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	m_ListCtrl.InsertColumn (COL_DIMENSIONS, "Dimensions", LVCFMT_LEFT, (rect.Width () / 4) - 10);
	m_ListCtrl.InsertColumn (COL_TEXTURE_TYPE, "Texture Type", LVCFMT_LEFT, (rect.Width () / 4) - 10);	

	// Loop through the list of textures and add them to the list control
	TEXTURE_NODE_LIST &node_list = parent_node.Get_Subobj_List ();
	for (int index = 0; index < node_list.Count (); index ++) {
		
		// Add this mesh to the list control
		TextureListNodeClass *pnode = node_list[index];
		int list_index = m_ListCtrl.InsertItem (0, pnode->Get_Name (), pnode->Get_Icon_Index ());
		Insert_Texture_Details (pnode, list_index);
	}

	m_bContainsMeshes = false;
	m_Toolbar.Enable_Button (IDC_BACK, true);
	m_Toolbar.Enable_Button (IDC_PROPAGATE, false);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
BOOL
TextureMgrDialogClass::OnInitDialog (void)
{
	CWaitCursor wait_cursor;	

	// Allow the base class to process this message
	CDialog::OnInitDialog ();
	m_ListCtrl.SetExtendedStyle (LVS_EX_FULLROWSELECT);

	m_Toolbar.CreateEx (this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, CRect(0, 0, 0, 0), 101);
	m_Toolbar.SetOwner (this);
	m_Toolbar.LoadToolBar (IDR_INSTANCES_TOOLBAR);
	m_Toolbar.SetBarStyle (m_Toolbar.GetBarStyle () | CBRS_TOOLTIPS | CBRS_FLYBY);

	// Get the bounding rectangle of the form window
	CRect rect;
	::GetWindowRect (::GetDlgItem (m_hWnd, IDC_TOOLBAR_SLOT), &rect);
	ScreenToClient (&rect);
	m_Toolbar.SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), SWP_NOZORDER);

	ASSERT (m_pBaseModel != NULL);

	// Create an icon imagelist for the tree control
	m_pImageList = new CImageList;
	m_pImageListSmall = new CImageList;
	m_pTextureImageList = new CImageList;
	m_pTextureImageListSmall = new CImageList;
	m_pImageList->Create (32, 32, ILC_COLOR | ILC_MASK, 1, 2);
	m_pImageListSmall->Create (16, 16, ILC_COLOR | ILC_MASK, 1, 2);
	m_pTextureImageList->Create (TEXTURE_THUMB_X, TEXTURE_THUMB_Y, ILC_COLOR24, 10, 20);
	m_pTextureImageListSmall->Create (TEXTURE_THUMBSMALL_X, TEXTURE_THUMBSMALL_Y, ILC_COLOR24, 10, 20);

	// Load this icon and add it to our imagelist
	m_pImageList->Add ((HICON)::LoadImage (::AfxGetResourceHandle (),
														MAKEINTRESOURCE (IDI_MESH_FOLDER),
														IMAGE_ICON,
														32,
														32,
														LR_SHARED));

	// Load this icon and add it to our imagelist
	m_pImageListSmall->Add ((HICON)::LoadImage (::AfxGetResourceHandle (),
															  MAKEINTRESOURCE (IDI_MESH_FOLDER),
															  IMAGE_ICON,
															  16,
															  16,
															  LR_SHARED));
	
	// Add the name column to the list control
	m_ListCtrl.GetClientRect (&rect);
	m_ListCtrl.InsertColumn (COL_NAME, "Name", LVCFMT_LEFT, (rect.Width () / 2) - 20);
	
	// Build a list of mesh's and textures
	Add_Subobjs_To_List (m_pBaseModel);
	Fill_List_Ctrl_With_Meshes ();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Subobjs_To_List
//
void
TextureMgrDialogClass::Add_Subobjs_To_List (RenderObjClass *prender_obj)
{
	// Loop through all the subobjs in this render object
	int subobj_count = prender_obj->Get_Num_Sub_Objects ();
	for (int index = 0; index < subobj_count; index ++) {
		
		// Get a pointer to this subobject
		RenderObjClass *psubobj = prender_obj->Get_Sub_Object (index);
		if (psubobj != NULL) {
			
			// Recursively add subobjs to the list
			Add_Subobjs_To_List (psubobj);
			MEMBER_RELEASE (psubobj);
		}
	}

	// If this is a mesh, then add it to the list
	if (prender_obj->Class_ID () == RenderObjClass::CLASSID_MESH) {
		
		// Create a new node and add it to our list
		TextureListNodeClass *pnode = new TextureListNodeClass (prender_obj->Get_Name ());
		m_NodeList.Add (pnode);

		// Add all the mesh's textures to this list
		Add_Textures_To_Node ((MeshClass *)prender_obj, pnode);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Textures_To_Node
//
void
TextureMgrDialogClass::Add_Textures_To_Node
(
	MeshClass *pmesh,
	TextureListNodeClass *pmesh_node
)
{
	MaterialInfoClass *pmat_info = pmesh->Get_Material_Info ();
	if (pmat_info != NULL) {
		
		// Loop through all the textures and add them as subobjs
		for (int index = 0; index < pmat_info->Texture_Count (); index ++) {
			TextureClass *ptexture = pmat_info->Get_Texture (index);
			if (ptexture != NULL) {

				// Create a node from this texture and add it to the mesh
				TextureListNodeClass *pnode = new TextureListNodeClass (ptexture, ::Get_Texture_Name (*ptexture));
				pmesh_node->Add_Subobj (pnode);
				pnode->Set_Parent (pmesh_node);
				pnode->Set_Texture_Index (index);

				// Give this texture a thumbnail
				pnode->Set_Icon_Index (Get_Thumbnail (ptexture));

				// Release our hold on this pointer
				REF_PTR_RELEASE (ptexture);				
			}
		}
		
		// Release our hold on this pointer
		MEMBER_RELEASE (pmat_info);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Texture_Thumbnail
//
int
TextureMgrDialogClass::Find_Texture_Thumbnail (LPCTSTR name)
{
	int icon_index = -1;

	// Loop through the names in our list and if we find the texture
	// name, return its index
	for (int index = 0; (index < m_TextureNames.Count ()) && (icon_index == -1); index ++) {
		if (::lstrcmpi (name, m_TextureNames[index]) == 0) {
			icon_index = index;
		}
	}

	// Return the icon index (-1 if not found)
	return icon_index;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
void
TextureMgrDialogClass::OnOK (void)
{
	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
void
TextureMgrDialogClass::OnCancel (void) 
{
	// Allow the base class to process this message
	CDialog::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkMeshTextureListCtrl
//
void
TextureMgrDialogClass::OnDblclkMeshTextureListCtrl
(
	NMHDR *pNMHDR,
	LRESULT *pResult
)
{
	// Determine which item is selected
	int index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (index != -1) {
		
		// Get the node associated with this entry
		TextureListNodeClass *pnode = (TextureListNodeClass *)m_ListCtrl.GetItemData (index);
		if (pnode != NULL) {
			
			// Is this a mesh or a texture?
			if (pnode->Get_Type () == TextureListNodeClass::TYPE_MESH) {
				Fill_List_Ctrl_With_Textures (*pnode);
			} else {
				
				// Is this a texture the user can modify?
				TextureClass *ptexture = pnode->Peek_Texture ();
				if (ptexture->getClassID () != ID_INDIRECT_TEXTURE_CLASS) {
					MessageBox ("This texture can not be edited.  Only indirect textures are modifiable", "Invalid Type", MB_OK | MB_ICONEXCLAMATION);
				} else {

					// Get an original instance of this model so we can compare textures to
					// try and find an original texture...
					TextureListNodeClass *pmesh_node = pnode->Get_Parent ();
					RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance ()->Create_Render_Obj (pmesh_node->Get_Name ());
					TextureClass *poriginal_texture = NULL;
					if (prender_obj != NULL) {
						
						// Get the material information for this render object
						MaterialInfoClass *pmat_info = prender_obj->Get_Material_Info ();
						if (pmat_info != NULL) {
							
							// Attempt to find the original texture
							poriginal_texture = pmat_info->Get_Texture (pnode->Get_Texture_Index ());
							if (poriginal_texture->getClassID () != ID_INDIRECT_TEXTURE_CLASS) {
								SR_RELEASE (poriginal_texture);
							}

							MEMBER_RELEASE (pmat_info);
						}
					}
					
					// Show the user a dialog containing the texture's properties
					TextureSettingsDialogClass dialog ((IndirectTextureClass *)ptexture,
																  (IndirectTextureClass *)poriginal_texture,
																  this);
					dialog.DoModal ();

					// If the settings we modified, then update the list control information
					if (dialog.Were_Settings_Modified ()) {
						
						// Recreate the thumbnail (if necessary)
						pnode->Set_Icon_Index (Get_Thumbnail (ptexture));
						pnode->Set_Name (::Get_Texture_Name (*ptexture));
						
						// Update the list control with the new settings
						Insert_Texture_Details (pnode, index);
						m_ListCtrl.Update (index);
					}

					// Release our hold on the originaltexture
					SR_RELEASE (poriginal_texture);
				}
			}
		}
	}

	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKeydownMeshTextureListCtrl
//
void
TextureMgrDialogClass::OnKeydownMeshTextureListCtrl
(
	NMHDR* pNMHDR,
	LRESULT* pResult
)
{
	// Did the user press the backspace key?
	LV_KEYDOWN *pLVKeyDown = (LV_KEYDOWN *)pNMHDR;
	if (pLVKeyDown && (pLVKeyDown->wVKey == VK_BACK)) {
		
		// Display the mesh list
		if (m_bContainsMeshes == false) {
			Fill_List_Ctrl_With_Meshes ();
		}
	}
	
	(*pResult) = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
void
TextureMgrDialogClass::OnDestroy (void) 
{
	// Free the state image list we associated with the control
	m_ListCtrl.SetImageList (NULL, LVSIL_NORMAL);
	m_ListCtrl.SetImageList (NULL, LVSIL_SMALL);
	SAFE_DELETE (m_pImageList);
	SAFE_DELETE (m_pImageListSmall);
	SAFE_DELETE (m_pTextureImageList);	
	SAFE_DELETE (m_pTextureImageListSmall);	
	m_TextureNames.Delete_All ();

	// Loop through the list of nodes and free them
	for (int index = 0; index < m_NodeList.Count (); index ++) {
		TextureListNodeClass *pnode = m_NodeList[index];
		SAFE_DELETE (pnode);
	}

	// Remove all the entries from the list
	m_NodeList.Delete_All ();
	
	// Allow the base class to process this message
	CDialog::OnDestroy ();
	return ; 
}


/////////////////////////////////////////////////////////////////////////////
//
// OnBack
//
void
TextureMgrDialogClass::OnBack (void)
{
	// Display the mesh list
	if (m_bContainsMeshes == false) {
		Fill_List_Ctrl_With_Meshes ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDetails
//
void
TextureMgrDialogClass::OnDetails (void)
{
	LONG style = ::GetWindowLong (m_ListCtrl, GWL_STYLE);
	SetWindowLong (m_ListCtrl, GWL_STYLE, (style & (~LVS_TYPEMASK)) | LVS_REPORT);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLarge
//
void
TextureMgrDialogClass::OnLarge (void)
{
	LONG style = ::GetWindowLong (m_ListCtrl, GWL_STYLE);
	SetWindowLong (m_ListCtrl, GWL_STYLE, (style & (~LVS_TYPEMASK)) | LVS_ICON);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnList
//
void
TextureMgrDialogClass::OnList (void)
{
	LONG style = ::GetWindowLong (m_ListCtrl, GWL_STYLE);
	SetWindowLong (m_ListCtrl, GWL_STYLE, (style & (~LVS_TYPEMASK)) | LVS_LIST);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSmall
//
void
TextureMgrDialogClass::OnSmall (void)
{
	LONG style = ::GetWindowLong (m_ListCtrl, GWL_STYLE);
	SetWindowLong (m_ListCtrl, GWL_STYLE, (style & (~LVS_TYPEMASK)) | LVS_SMALLICON);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Thumbnail
//
int
TextureMgrDialogClass::Get_Thumbnail (srTextureIFace *ptexture)
{
	int icon_index = 0;

	// See if this texture already has a thumbnail
	icon_index = Find_Texture_Thumbnail (::Get_Texture_Name (*ptexture));
	if (icon_index == -1) {

		// Create a windows bitmap from this texture
		HBITMAP hbmp = ::Make_Bitmap_From_Texture (*ptexture, TEXTURE_THUMB_X, TEXTURE_THUMB_Y);
		if (hbmp != NULL) {
			
			// Insert this bitmap into our imagelist
			CBitmap temp_obj;
			temp_obj.Attach (hbmp);
			icon_index = m_pTextureImageList->Add (&temp_obj, (CBitmap *)NULL);

			// Create a smaller bitmap and insert it into the other imagelist
			HBITMAP hsmall_bitmap = (HBITMAP)::CopyImage (hbmp, IMAGE_BITMAP, TEXTURE_THUMBSMALL_X, TEXTURE_THUMBSMALL_Y, 0);
			CBitmap small_obj;
			small_obj.Attach (hsmall_bitmap);
			m_pTextureImageListSmall->Add (&small_obj, (CBitmap *)NULL);

			// Add a name to our list to represent this texture
			m_TextureNames.Add (::Get_Texture_Name (*ptexture));
		}	
	}

	// Return the icon index
	return icon_index;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Texture_Details
//
void
TextureMgrDialogClass::Insert_Texture_Details
(
	TextureListNodeClass *pnode,
	int index
)
{
	TextureClass *ptexture = pnode->Peek_Texture ();
	if ((index != -1) && (ptexture != NULL)) {

		// Get the name of the texture (mark it differently if its an editable texture)
		CString texture_name = ::Get_Texture_Name (*ptexture);
		if (ptexture->getClassID () == ID_INDIRECT_TEXTURE_CLASS) {
			texture_name += " *";
		}
		
		// Update the texture's icon and name in the list control
		m_ListCtrl.SetItem (index,
								  COL_NAME,
								  LVIF_IMAGE | LVIF_TEXT,
								  texture_name,
								  pnode->Get_Icon_Index (),
								  0, 0, 0);

		// Insert the texture dimensions in the second column		
		SurfaceClass::SurfaceDescription surface_desc;
		ptexture->Get_Level_Description(surface_desc);
		CString dimension_string;
		dimension_string.Format ("(%dx%d)", surface_desc.Width, surface_desc.Height);
		m_ListCtrl.SetItemText (index, COL_DIMENSIONS, dimension_string);

		// Determine what type the texture is
		CString type_string;
		switch (ptexture->getClassID ())
		{
			case srClass::ID_TEXTURE_FILE:
				type_string = "Simple file";
				break;

			case ID_MANUAL_ANIM_TEXTURE_INSTANCE_CLASS:
			case ID_TIME_ANIM_TEXTURE_INSTANCE_CLASS:
				type_string = "Animated";
				break;

			case ID_RESIZEABLE_TEXTURE_INSTANCE_CLASS:
				type_string = "Resizeable";
				break;

			case ID_INDIRECT_TEXTURE_CLASS:
				type_string = "Indirect (editable)";
				break;

			default:
				ASSERT (0);
				break;
		}

		// Set the texture type information
		m_ListCtrl.SetItemText (index, COL_TEXTURE_TYPE, type_string);

		// Associate the node with the list entry
		m_ListCtrl.SetItemData (index, (DWORD)pnode);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Propogate_Changes
//
/////////////////////////////////////////////////////////////////////////////
void
TextureMgrDialogClass::OnPropagate (void)
{	
	//
	//	Determine the currently selected item
	//
	int sel_index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (m_bContainsMeshes == false || sel_index == -1) {
		return ;
	}

	if (MessageBox ("Warning, this operation will copy the texture settings from the currently selected mesh into all meshes with the same texture count.  Do you wish to continue?", "Texture Propagate", MB_ICONQUESTION | MB_YESNO) == IDNO) {
		return ;
	}

	TextureListNodeClass *src_node		= (TextureListNodeClass *)m_ListCtrl.GetItemData (sel_index);
	TEXTURE_NODE_LIST &src_texture_list	= src_node->Get_Subobj_List ();
	int src_texture_count					= src_texture_list.Count ();

	//
	//	Loop over all the other meshes in the list
	//
	int counter = m_ListCtrl.GetItemCount ();
	while (counter --) {
		
		//
		//	Does this node have the same number of replaceable textures as the src node?
		//
		TextureListNodeClass *curr_node = (TextureListNodeClass *)m_ListCtrl.GetItemData (counter);		
		if (	(curr_node != NULL) &&
				(curr_node->Get_Subobj_List ().Count () == src_texture_count))
		{
			TEXTURE_NODE_LIST &curr_texture_list = curr_node->Get_Subobj_List ();

			//
			//	Copy the textures...
			//
			int texture_counter = src_texture_count;
			while (texture_counter --) {
				
				TextureListNodeClass *curr_texture_node	= curr_texture_list[texture_counter];
				TextureListNodeClass *src_texture_node		= src_texture_list[texture_counter];
				if (curr_texture_node != NULL && src_texture_node != NULL) {
					
					TextureClass *curr_texture	= curr_texture_node->Peek_Texture ();
					TextureClass *src_texture	= src_texture_node->Peek_Texture ();

					//
					//	Are the textures both indirect textures?
					//
					if (	curr_texture != NULL && src_texture != NULL &&
							curr_texture->getClassID () == ID_INDIRECT_TEXTURE_CLASS &&
							src_texture->getClassID () == ID_INDIRECT_TEXTURE_CLASS)
					{
						TextureClass *real_texture = ((IndirectTextureClass *)src_texture)->Peek_Texture ();
						((IndirectTextureClass *)curr_texture)->Set_Texture (real_texture);					
					}
				}
			}
		}
	}

	return ;
}


#endif // WW3D_DX8
