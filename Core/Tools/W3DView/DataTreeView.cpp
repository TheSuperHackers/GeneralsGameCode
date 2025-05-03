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
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/DataTreeView.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/18/01 9:11a                                               $*
 *                                                                                             *
 *                    $Revision:: 35                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.H"
#include "W3DView.H"
#include "DataTreeView.H"
#include "RendObj.H"
#include "ViewerAssetMgr.H"
#include "Globals.h"
#include "W3DViewDoc.H"
#include "MainFrm.H"
#include "DistLod.H"
#include "AnimObj.H"
#include "HcAnim.H"
#include "AssetInfo.H"
#include "Utils.H"
#include "Vector.H"
#include "Part_Emt.H"
#include "Agg_Def.H"
#include "BMP2D.H"
#include "HLod.H"
#include "ViewerScene.h"
#include "texture.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


////////////////////////////////////////////////////////////////////////////
//	Local Prototypes
////////////////////////////////////////////////////////////////////////////
void Set_Highest_LOD (RenderObjClass *render_obj);


////////////////////////////////////////////////////////////////////////////
//	MFC Stuff
////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CDataTreeView, CTreeView)


////////////////////////////////////////////////////////////////////////////
//
//  CDataTreeView
//
////////////////////////////////////////////////////////////////////////////
CDataTreeView::CDataTreeView (void)
        : m_hMaterialsRoot (NULL),          
			 m_hMeshRoot  (NULL),
			 m_hMeshCollectionRoot (NULL),
			 m_hAggregateRoot (NULL),
			 m_hPrimitivesRoot (NULL),
			 m_hEmitterRoot (NULL),
          m_hLODRoot (NULL),
			 m_hSoundRoot (NULL),
			 m_iPrimitivesIcon (-1),
          m_iAnimationIcon (-1),
          m_iTCAnimationIcon(-1),
			 m_iADAnimationIcon(-1),
          m_iMeshIcon (-1),
          m_iMaterialIcon (-1),
          m_iLODIcon (-1),
			 m_iAggregateIcon (-1),
			 m_iEmitterIcon (-1),
			 m_iSoundIcon (-1),
			 m_RestrictAnims (true)

{
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  ~CDataTreeView
//
CDataTreeView::~CDataTreeView (void)
{
    return ;
}


BEGIN_MESSAGE_MAP(CDataTreeView, CTreeView)
	//{{AFX_MSG_MAP(CDataTreeView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////
//
//  OnDraw
//
void
CDataTreeView::OnDraw (CDC *pDC)
{
	return ;
}

/////////////////////////////////////////////////////////////////////////////
// CDataTreeView diagnostics

#ifdef RTS_DEBUG
void CDataTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CDataTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDataTreeView message handlers


////////////////////////////////////////////////////////////////////////////
//
//  PreCreateWindow
//
BOOL
CDataTreeView::PreCreateWindow (CREATESTRUCT& cs) 
{
    // Modify the style bits for the window so it will
    // have buttons and lines between nodes.
    cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;

    // Allow the base class to process this message
	return CTreeView::PreCreateWindow(cs);
}

////////////////////////////////////////////////////////////////////////////
//
//  OnInitialUpdate
//
void
CDataTreeView::OnInitialUpdate (void)
{
	// Allow the base class to process this message
    CTreeView::OnInitialUpdate ();
	
	// TODO: Add your specialized code here and/or call the base class
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateRootNodes
//
void
CDataTreeView::CreateRootNodes (void) 
{
	// Insert all the root nodes
	m_hMaterialsRoot		= GetTreeCtrl ().InsertItem ("Materials", m_iMaterialIcon, m_iMaterialIcon);	
	m_hMeshRoot				= GetTreeCtrl ().InsertItem ("Mesh", m_iMeshIcon, m_iMeshIcon);
	m_hHierarchyRoot		= GetTreeCtrl ().InsertItem ("Hierarchy", m_iHierarchyIcon, m_iHierarchyIcon);
	m_hLODRoot				= GetTreeCtrl ().InsertItem ("H-LOD", m_iLODIcon, m_iLODIcon);
	m_hMeshCollectionRoot = GetTreeCtrl ().InsertItem ("Mesh Collection", m_iMeshIcon, m_iMeshIcon);
	m_hAggregateRoot		= GetTreeCtrl ().InsertItem ("Aggregate", m_iAggregateIcon, m_iAggregateIcon);    
	m_hEmitterRoot			= GetTreeCtrl ().InsertItem ("Emitter", m_iEmitterIcon, m_iEmitterIcon);	
	m_hPrimitivesRoot		= GetTreeCtrl ().InsertItem ("Primitives", m_iPrimitivesIcon, m_iPrimitivesIcon);
	m_hSoundRoot			= GetTreeCtrl ().InsertItem ("Sounds", m_iSoundIcon, m_iSoundIcon);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnCreate
//
int
CDataTreeView::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;

    CImageList imageList;
    imageList.Create (16, 18, ILC_COLOR | ILC_MASK, 5, 10);
    
    // Add the icons to the imagelist
    m_iAnimationIcon		= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_ANIMATION)));
	 m_iTCAnimationIcon	= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_ANIMATION_COMPRESSED)));
	 m_iADAnimationIcon  = imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_ANIMATION_COMPRESSED_DELTA)));
    m_iMeshIcon			= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_MESH)));
    m_iMaterialIcon		= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_MATERIAL)));
    m_iLODIcon				= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_LOD)));
	 m_iAggregateIcon		= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_HIERARCHY)));
	 m_iEmitterIcon		= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_HIERARCHY)));
	 m_iHierarchyIcon		= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_HIERARCHY)));
	 m_iPrimitivesIcon	= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_PRIMITIVES)));
	 m_iSoundIcon			= imageList.Add (::LoadIcon (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDI_SOUND)));

    // Pass the imagelist onto the tree control
    GetTreeCtrl ().SetImageList (&imageList, TVSIL_NORMAL);
    imageList.Detach ();

    // Create the root nodes that will hold the contain
    // asset types.
    CreateRootNodes ();
	return 0;
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_Materials_Into_Tree
//
void
CDataTreeView::Load_Materials_Into_Tree (void) 
{
	// Get an iterator from the asset manager that we can
	// use to enumerate the currently loaded textures
	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
		
	// Loop through all the textures in the manager
	for (ite.First ();
		  !ite.Is_Done ();
		  ite.Next ()) {

		// Get the current texture name
		TextureClass* ptexture=ite.Peek_Value();
		LPCTSTR texture_name = ptexture->Get_Texture_Name();

		if ((ptexture != NULL) &&
			 FindChildItem (m_hMaterialsRoot, texture_name) == NULL) {

			// Add this entry to the tree
			HTREEITEM tree_item = GetTreeCtrl ().InsertItem (texture_name, m_iMaterialIcon, m_iMaterialIcon, m_hMaterialsRoot, TVI_SORT);
			ASSERT (tree_item != NULL);

			// Allocate a new asset information class to associate with this entry
			ptexture->Add_Ref ();
			AssetInfoClass *asset_info = new AssetInfoClass (texture_name, TypeMaterial, NULL, (DWORD)ptexture);
			GetTreeCtrl ().SetItemData (tree_item, (ULONG)asset_info);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  LoadAssetsIntoTree
//
void
CDataTreeView::LoadAssetsIntoTree (void) 
{
	// Turn off repainting
	GetTreeCtrl ().SetRedraw (FALSE);

	DynamicVectorClass<CString> dist_lod_list;

	// Get an iterator from the asset manager that we can
	// use to enumerate the currently loaded assets
	RenderObjIterator *pObjEnum = WW3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator ();
	ASSERT (pObjEnum);
	if (pObjEnum) {

		// Loop through all the assets in the manager
		for (pObjEnum->First ();
			  pObjEnum->Is_Done () == FALSE;
			  pObjEnum->Next ()) {

			// Does this render obj really exist?
			LPCTSTR pszItemName = pObjEnum->Current_Item_Name ();
			if (WW3DAssetManager::Get_Instance()->Render_Obj_Exists (pszItemName)) {
				
				BOOL bInsert = FALSE;
				HTREEITEM hParentNode = NULL;
				ASSET_TYPE assetType = TypeUnknown;
				int iIconIndex = -1;

				// What type of asset is this?
				switch (pObjEnum->Current_Item_Class_ID ()) {

					case RenderObjClass::CLASSID_COLLECTION:						
						// This is a 'mesh collection', we want to add this under the 'collection' node.
						bInsert = TRUE;
						hParentNode = m_hMeshCollectionRoot;
						assetType = TypeMesh;
						iIconIndex = m_iMeshIcon;
						break;

					//
					// This is a mesh render object, we want to add this under the mesh node.
					//
					case RenderObjClass::CLASSID_MESH:						
						bInsert			= TRUE;
						hParentNode		= m_hMeshRoot;
						assetType		= TypeMesh;
						iIconIndex		= m_iMeshIcon;
						break;

					//
					// This is a sound render obj, we want to add this under the sound node.
					//					
					case RenderObjClass::CLASSID_SOUND:
						bInsert			= TRUE;
						hParentNode		= m_hSoundRoot;
						assetType		= TypeSound;
						iIconIndex		= m_iSoundIcon;						
						break;

					case RenderObjClass::CLASSID_HMODEL:
						// Shouldn't happen
						ASSERT (0);
						break;

					case RenderObjClass::CLASSID_PARTICLEEMITTER:							
						// This is an 'emitter', we want to add this under the 'emitter' node.
						bInsert = TRUE;
						hParentNode = m_hEmitterRoot;
						assetType = TypeEmitter;
						iIconIndex = m_iEmitterIcon;
						break;

					case RenderObjClass::CLASSID_SPHERE:
					case RenderObjClass::CLASSID_RING:
						bInsert		= TRUE;
						hParentNode = m_hPrimitivesRoot;
						assetType	= TypePrimitives;
						iIconIndex	= m_iPrimitivesIcon;
						break;

					case RenderObjClass::CLASSID_DISTLOD:
						dist_lod_list.Add (pszItemName);
					case RenderObjClass::CLASSID_HLOD:
						// Assume this is a simple hierarchy LOD, we want to add this under the hierarchy node.
						bInsert = TRUE;
						hParentNode = m_hHierarchyRoot;
						assetType = TypeHierarchy;
						iIconIndex = m_iHierarchyIcon;

						// Test this HLOD to see if its a true LOD or a simple hierarchy
						if (::Is_Real_LOD (pszItemName)) {
							hParentNode = m_hLODRoot;
							assetType = TypeLOD;
							iIconIndex = m_iLODIcon;
						}
						break;
				}

				if (bInsert) {

					// Check to see if this object is an aggregate
					if (::Is_Aggregate (pszItemName)) {
						hParentNode = m_hAggregateRoot;
						assetType = TypeAggregate;
						iIconIndex = m_iAggregateIcon;
					}

					// If this object isn't already in the tree then add it
					if (FindChildItem (hParentNode, pszItemName) == NULL) {

						// Add this entry to the tree
						HTREEITEM hItem = GetTreeCtrl ().InsertItem (pszItemName, iIconIndex, iIconIndex, hParentNode, TVI_SORT);
						ASSERT (hItem != NULL);

						// Allocate a new asset information class to associate with this entry
						AssetInfoClass *asset_info = new AssetInfoClass (pszItemName, assetType);
						GetTreeCtrl ().SetItemData (hItem, (ULONG)asset_info);
					}
				}
			}
		}

		// Free the enumerator object we created earlier
		SAFE_DELETE (pObjEnum);
	}

	// Loop through all the old-style dist lod's and convert their prototypes
	// to the new HLOD format
	for (int index = 0; index < dist_lod_list.Count (); index ++) {
		HLodClass *plod = (HLodClass *)WW3DAssetManager::Get_Instance ()->Create_Render_Obj (dist_lod_list[index]);
		if (plod != NULL) {
			HLodDefClass *definition = new HLodDefClass (*plod);
			HLodPrototypeClass *prototype = new HLodPrototypeClass (definition);
			WW3DAssetManager::Get_Instance ()->Remove_Prototype (dist_lod_list[index]);
			WW3DAssetManager::Get_Instance ()->Add_Prototype (prototype);
		}
	}

	// Now that we've added all the hierarchies to the tree, add their animations
	// as well.
	LoadAnimationsIntoTree ();
	Load_Materials_Into_Tree ();

	// Turn;repainting back on
	GetTreeCtrl ().SetRedraw (TRUE);

	// Force the window to be repainted
	Invalidate (FALSE);
	UpdateWindow ();
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  LoadAnimationsIntoTree
//
void
CDataTreeView::LoadAnimationsIntoTree (void)
{
    // Get an iterator from the asset manager that we can
    // use to enumerate the currently loaded assets
    AssetIterator *pAnimEnum = WW3DAssetManager::Get_Instance()->Create_HAnim_Iterator ();
    ASSERT (pAnimEnum);
    if (pAnimEnum)
    {
        // Loop through all the animations in the manager
        for (pAnimEnum->First ();
             (pAnimEnum->Is_Done () == FALSE);
             pAnimEnum->Next ())
        {
            LPCTSTR pszAnimName = pAnimEnum->Current_Item_Name ();

            // Get an instance of the animation object
            HAnimClass *pHierarchyAnim = WW3DAssetManager::Get_Instance()->Get_HAnim (pszAnimName);
            
            ASSERT (pHierarchyAnim);
            if (pHierarchyAnim)
            {
                // Get the name of the hierarchy that this animation belongs to
                LPCTSTR pszHierarchyName = pHierarchyAnim->Get_HName ();
                HTREEITEM hNode;
                // Loop through all the hierarchies and add this animation to any pertinent ones
                for (hNode = FindFirstChildItemBasedOnHierarchyName (m_hHierarchyRoot, pszHierarchyName);
                     (hNode != NULL);
                     hNode = FindSiblingItemBasedOnHierarchyName (hNode, pszHierarchyName))
                {
                    // Is this animation already loaded into the tree?
                    HTREEITEM hAnimationNode = FindChildItem (hNode, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = GetTreeCtrl ().InsertItem (pszAnimName, m_iAnimationIcon, m_iAnimationIcon, hNode, TVI_SORT);
                        ASSERT (hAnimationNode != NULL);
                                    
                        // Associate the items name with its entry
                        GetTreeCtrl ().SetItemData (hAnimationNode, (ULONG)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Loop through all the aggregates and add this animation to any pertinent ones
                for (hNode = FindFirstChildItemBasedOnHierarchyName (m_hAggregateRoot, pszHierarchyName);
                     (hNode != NULL);
                     hNode = FindSiblingItemBasedOnHierarchyName (hNode, pszHierarchyName))
                {
                    // Is this animation already loaded into the tree?
                    HTREEITEM hAnimationNode = FindChildItem (hNode, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = GetTreeCtrl ().InsertItem (pszAnimName, m_iAnimationIcon, m_iAnimationIcon, hNode, TVI_SORT);
                        ASSERT (hAnimationNode != NULL);
                                    
                        // Associate the items name with its entry
                        GetTreeCtrl ().SetItemData (hAnimationNode, (ULONG)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Loop through all the hierarchies and add this animation to any pertinent ones
                for (hNode = FindFirstChildItemBasedOnHierarchyName (m_hLODRoot, pszHierarchyName);
                     (hNode != NULL);
                     hNode = FindSiblingItemBasedOnHierarchyName (hNode, pszHierarchyName))
                {
                    // Is this animation already loaded into the tree?
                    HTREEITEM hAnimationNode = FindChildItem (hNode, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = GetTreeCtrl ().InsertItem (pszAnimName, m_iAnimationIcon, m_iAnimationIcon, hNode, TVI_SORT);
                        ASSERT (hAnimationNode != NULL);
                                    
                        // Associate the items name with its entry
                        GetTreeCtrl ().SetItemData (hAnimationNode, (ULONG)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Release our hold on this animation...
					 MEMBER_RELEASE (pHierarchyAnim);
            }							
        }
        
        // Free the object
        delete pAnimEnum;
        pAnimEnum = NULL;
    }
    
    return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  LoadAnimationsIntoTree
//
void
CDataTreeView::LoadAnimationsIntoTree (HTREEITEM hItem)
{
    // Get the data associated with this item
    AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (hItem);
    ASSERT (asset_info != NULL);

    // Get an iterator from the asset manager that we can
    // use to enumerate the currently loaded assets
    AssetIterator *pAnimEnum = WW3DAssetManager::Get_Instance()->Create_HAnim_Iterator ();
    ASSERT (pAnimEnum);
    if (pAnimEnum)
    {
        // Loop through all the animations in the manager
        for (pAnimEnum->First ();
             (pAnimEnum->Is_Done () == FALSE);
             pAnimEnum->Next ())
        {
            LPCTSTR pszAnimName = pAnimEnum->Current_Item_Name ();

            // Get an instance of the animation object
            HAnimClass *pHierarchyAnim = WW3DAssetManager::Get_Instance()->Get_HAnim (pszAnimName);
            
            ASSERT (pHierarchyAnim);
            if (pHierarchyAnim)
            {
                // Get the name of the hierarchy that this animation belongs to
                LPCTSTR pszHierarchyName = pHierarchyAnim->Get_HName ();

                // Does the item match the hierarchy name?
                if (::lstrcmp (asset_info->Get_Hierarchy_Name (), pszHierarchyName) == 0)
                {
                    // Is this animation already loaded into the tree?
                    HTREEITEM hAnimationNode = FindChildItem (hItem, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = GetTreeCtrl ().InsertItem (pszAnimName, m_iAnimationIcon, m_iAnimationIcon, hItem, TVI_SORT);
                        ASSERT (hAnimationNode != NULL);
                                    
                        // Associate the items name with its entry
                        GetTreeCtrl ().SetItemData (hAnimationNode, (ULONG)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Release our hold on the animation object
                MEMBER_RELEASE (pHierarchyAnim);
            }				
        }
        
        // Free the object
        delete pAnimEnum;
        pAnimEnum = NULL;
    }
    
    return ;
}
   

////////////////////////////////////////////////////////////////////////////
//
//  Determine_Tree_Location
//
ASSET_TYPE
CDataTreeView::Determine_Tree_Location
(
	RenderObjClass &render_obj,
	HTREEITEM &hroot,
	int &icon_index
)
{
	ASSET_TYPE type = TypeUnknown;

	// What class does this render object belong to?
	switch (render_obj.Class_ID ()) {
		
		case RenderObjClass::CLASSID_COLLECTION:
			hroot				= m_hMeshCollectionRoot;
			type				= TypeMesh;
			icon_index		= m_iMeshIcon;
			break;

		case RenderObjClass::CLASSID_MESH:
			hroot = m_hMeshRoot;
			type = TypeMesh;
			icon_index = m_iMeshIcon;
			break;						  
		
		case RenderObjClass::CLASSID_SOUND:
			hroot				= m_hSoundRoot;
			type				= TypeSound;
			icon_index		= m_iSoundIcon;
			break;						  		

		case RenderObjClass::CLASSID_HMODEL:			
			// Shouldn't happen
			ASSERT (0);
			break;

		case RenderObjClass::CLASSID_PARTICLEEMITTER:
			hroot				= m_hEmitterRoot;
			type				= TypeEmitter;
			icon_index		= m_iEmitterIcon;
			break;

		case RenderObjClass::CLASSID_SPHERE:
		case RenderObjClass::CLASSID_RING:
			hroot				= m_hPrimitivesRoot;
			type				= TypePrimitives;
			icon_index		= m_iPrimitivesIcon;
			break;
	
		case RenderObjClass::CLASSID_DISTLOD:
		case RenderObjClass::CLASSID_HLOD:
         hroot				= m_hHierarchyRoot;
         type				= TypeHierarchy;
         icon_index		= m_iHierarchyIcon;

			//
			// Determine if this is a true LOD or a simple hierarchy
			//
			if (((HLodClass &)render_obj).Get_LOD_Count () > 1) {
				hroot			= m_hLODRoot;
				type			= TypeLOD;
				icon_index	= m_iLODIcon;
			}
			break;
	}

	//
	// Is this an aggregate?
	//
	if (render_obj.Get_Base_Model_Name () != NULL) {
		hroot			= m_hAggregateRoot;
		type			= TypeAggregate;
		icon_index	= m_iAggregateIcon;
	}

	// Return the type of node
	return type;
}



////////////////////////////////////////////////////////////////////////////
//
//  Determine_Tree_Location
//
void
CDataTreeView::Determine_Tree_Location
(
	ASSET_TYPE type,
	HTREEITEM &hroot,
	int &icon_index
)
{
	// What type of asset is this?
	switch (type) {

		case TypeMesh:
			hroot			= m_hMeshRoot;
			icon_index	= m_iMeshIcon;
			break;

		case TypeSound:
			hroot			= m_hSoundRoot;
			icon_index	= m_iSoundIcon;
			break;

		case TypeAggregate:
			hroot			= m_hAggregateRoot;
			icon_index	= m_iAggregateIcon;
			break;

		case TypeHierarchy:
			// Shouldn't happen
			ASSERT (0);
			break;

		case TypeEmitter:
			hroot			= m_hEmitterRoot;
			icon_index	= m_iEmitterIcon;
			break;

		case TypePrimitives:
			hroot			= m_hPrimitivesRoot;
			icon_index	= m_iPrimitivesIcon;
			break;

		case TypeLOD:
			hroot			= m_hLODRoot;
			icon_index	= m_iLODIcon;
			break;
	}

	return ;
}



////////////////////////////////////////////////////////////////////////////
//
//  Add_Asset_To_Tree
//
bool
CDataTreeView::Add_Asset_To_Tree
(
	LPCTSTR name,
	ASSET_TYPE type,
	bool bselect
)
{
	// Assume failure
	bool retval = false;

	// Param OK?
	ASSERT (name != NULL);
	if (name != NULL) {

		// Turn off repainting
		GetTreeCtrl ().SetRedraw (FALSE);
		
		// Determime where this asset should go
		HTREEITEM hparent = NULL;
		int icon_index = 0;
		Determine_Tree_Location (type, hparent, icon_index);

		// Is this asset already in the tree?
		HTREEITEM htree_item = FindChildItem (hparent, name);
		if (htree_item == NULL) {

			// Add this object to the tree
			htree_item = GetTreeCtrl ().InsertItem (name,
																 icon_index,
																 icon_index,
																 hparent,
																 TVI_SORT);
			
			// Associate the render object with its entry in the tree
			AssetInfoClass *asset_info = new AssetInfoClass (name, type);
			GetTreeCtrl ().SetItemData (htree_item, (ULONG)asset_info);

			// Load the object's animations into the tree (if necessary)
			if (asset_info->Can_Asset_Have_Animations ()) {
				LoadAnimationsIntoTree (htree_item);
			}

			// Success!
			retval = (htree_item != NULL);
		}

		// Select the instance (if requested)
		if (bselect) {
			GetTreeCtrl ().SelectItem (htree_item);
			GetTreeCtrl ().EnsureVisible (htree_item);
		}

		// Turn painting back on
		GetTreeCtrl ().SetRedraw (TRUE);

		// Force the window to be repainted
		Invalidate (FALSE);
		UpdateWindow ();
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  FindChildItem
//
HTREEITEM
CDataTreeView::FindChildItem
(
	HTREEITEM hParentItem,
	RenderObjClass *prender_obj
)
{
	// Assume we won't find the item
	HTREEITEM hchild_item = NULL;

	// Loop through all the children of this node
	for (HTREEITEM htree_item = GetTreeCtrl ().GetChildItem (hParentItem);
		  (htree_item != NULL) && (hchild_item == NULL);
		  htree_item = GetTreeCtrl ().GetNextSiblingItem (htree_item)) {

		// Get the data associated with this item
		AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);

		// Is this the item we were looking for?
		if (asset_info &&
			 asset_info->Peek_Render_Obj () == prender_obj) {

			// This was the item we were looking for, return
			// its handle to the caller
			hchild_item = htree_item;
		}
	}

	// Return the child item handle
	return hchild_item;
}


////////////////////////////////////////////////////////////////////////////
//
//  FindChildItem
//
HTREEITEM
CDataTreeView::FindChildItem
(
    HTREEITEM hParentItem,
    LPCTSTR pszChildItemName
)
{
    // Assume we won't find the item
    HTREEITEM hChildItem = NULL;

    // Loop through all the children of this node
    for (HTREEITEM hTreeItem = GetTreeCtrl ().GetChildItem (hParentItem);
         (hTreeItem != NULL) && (hChildItem == NULL);
         hTreeItem = GetTreeCtrl ().GetNextSiblingItem (hTreeItem))
    {
        // Is this the child item we were looking for?
        if (::lstrcmp (GetTreeCtrl ().GetItemText (hTreeItem), pszChildItemName) == 0)
        {
            // This was the child item we were looking for, return
            // its handle to the caller
            hChildItem = hTreeItem;
        }
    }

    // Return the child item handle
    return hChildItem;
}

////////////////////////////////////////////////////////////////////////////
//
//  FindSiblingItemBasedOnHierarchyName
//
HTREEITEM
CDataTreeView::FindSiblingItemBasedOnHierarchyName
(
    HTREEITEM hCurrentItem,
    LPCTSTR pszHierarchyName
)
{
    // Assume we won't find the item
    HTREEITEM hSiblingItem = NULL;

    // Loop through all the siblings of this node
    HTREEITEM hTreeItem = hCurrentItem;
    while (((hTreeItem = GetTreeCtrl ().GetNextSiblingItem (hTreeItem)) != NULL) &&
           (hSiblingItem == NULL))
    {
        // Get the data associated with this item
        AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (hTreeItem);

        // Is this the item we were looking for?
        if (m_RestrictAnims == false ||
				(asset_info && ::lstrcmp (asset_info->Get_Hierarchy_Name (), pszHierarchyName) == 0))
        {
            // This was the item we were looking for, return
            // its handle to the caller
            hSiblingItem = hTreeItem;
        }        
    }

    // Return the sibling item handle
    return hSiblingItem;
}

////////////////////////////////////////////////////////////////////////////
//
//  FindFirstChildItemBasedOnHierarchyName
//
HTREEITEM
CDataTreeView::FindFirstChildItemBasedOnHierarchyName
(
    HTREEITEM hParentItem,
    LPCTSTR pszHierarchyName
)
{
    // Assume we won't find the item
    HTREEITEM hChildItem = NULL;

    // Loop through all the children of this node
    for (HTREEITEM hTreeItem = GetTreeCtrl ().GetChildItem (hParentItem);
         (hTreeItem != NULL) && (hChildItem == NULL);
         hTreeItem = GetTreeCtrl ().GetNextSiblingItem (hTreeItem))
    {
        // Get the data associated with this item
        AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (hTreeItem);

        //
		  // Is this the item we were looking for?
		  //
        if (m_RestrictAnims == false ||
				(asset_info && ::lstrcmp (asset_info->Get_Hierarchy_Name (), pszHierarchyName) == 0))
        {
            // This was the child item we were looking for, return
            // its handle to the caller
            hChildItem = hTreeItem;
        }        
    }

    // Return the child item handle
    return hChildItem;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSelChanged
//
void
CDataTreeView::OnSelChanged
(
    NMHDR* pNMHDR,
    LRESULT* pResult
)
{
	// Display the new selection
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	Display_Asset (pNMTreeView->itemNew.hItem);
	(*pResult) = 0;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Display_Asset
//
void
CDataTreeView::Display_Asset (HTREEITEM htree_item)
{
	if (htree_item == NULL) {
		htree_item = GetTreeCtrl ().GetSelectedItem ();
	}

	//
	// Get the object associated with this entry
	//
	AssetInfoClass *asset_info = NULL;	
	if (htree_item != NULL) {
		asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
	}

	if (asset_info != NULL) {

		// Get the current document, so we can get a pointer to the scene
		CW3DViewDoc *pdoc = (CW3DViewDoc *)GetDocument ();
		ASSERT (pdoc != NULL);
		if (pdoc != NULL) {

			// What type of asset is it?
			switch (asset_info->Get_Type ())
			{
				case TypeCompressedAnimation:
				case TypeAnimation:
				{
					HTREEITEM hParentItem = GetTreeCtrl ().GetParentItem (htree_item);
					if (hParentItem != NULL) {

						// Ask the document to start playing the animation for this object
						RenderObjClass *prender_obj = Create_Render_Obj_To_Display (hParentItem);                         
						pdoc->PlayAnimation (prender_obj,
						asset_info->Get_Name ());
						MEMBER_RELEASE (prender_obj);
					}
				}
				break;

				case TypeEmitter:
				{
					// Ask the document to display this object
					ParticleEmitterClass *emitter = (ParticleEmitterClass *)Create_Render_Obj_To_Display (htree_item);
					pdoc->Display_Emitter (emitter);
					MEMBER_RELEASE (emitter);
				}
				break;

				case TypeHierarchy:
				{
					// If the advanced animation option is turned on, display a dialog
					// where the user can choose which animations to play together.

				}

				default:
				{
					// Ask the document to display this object
					RenderObjClass *prender_obj = Create_Render_Obj_To_Display (htree_item);
					pdoc->DisplayObject (prender_obj);
					MEMBER_RELEASE (prender_obj);
				}
				break;
			}

			// Get the main window of our app
			CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
			if (pCMainWnd != NULL) {

				// Let the main window know our selection type has changed
				pCMainWnd->OnSelectionChanged (asset_info->Get_Type ());

				if (asset_info->Get_Type () == TypeAggregate) {
					pCMainWnd->Update_Emitters_List ();
				} else {

					::EnableMenuItem (::GetSubMenu (::GetMenu (*pCMainWnd), 3), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
					HMENU hsub_menu = pCMainWnd->Get_Emitters_List_Menu ();
					int index = 0;
					while (::RemoveMenu (hsub_menu, index, MF_BYPOSITION)) {
						//index ++;
					}
				}
			}
		}
	} else {

		// Reset the display
		CW3DViewDoc* pdoc = (CW3DViewDoc *)GetDocument ();
		ASSERT (pdoc != NULL);
		if (pdoc != NULL) {
			pdoc->DisplayObject ((RenderObjClass *)NULL);
			
			CMainFrame *main_wnd = (CMainFrame *)::AfxGetMainWnd ();
			if (main_wnd != NULL) {
				main_wnd->OnSelectionChanged (TypeUnknown);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDeleteItem
//
void
CDataTreeView::OnDeleteItem
(
	NMHDR *pNMHDR,
	LRESULT *pResult
) 
{
	// Get the information object for this asset
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;	
	AssetInfoClass *asset_info = (AssetInfoClass *)pNMTreeView->itemOld.lParam;
	
	// If this is a texture, then free our hold on its interface
	if (asset_info && (asset_info->Get_Type () == TypeMaterial)) {
		TextureClass *ptexture = (TextureClass *)asset_info->Get_User_Number ();
		REF_PTR_RELEASE (ptexture);
	}

	// Free the asset information object
	SAFE_DELETE (asset_info);

	// Reset the data associated with this entry
	GetTreeCtrl ().SetItemData (pNMTreeView->itemOld.hItem, NULL);
	(*pResult) = 0;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Current_Asset_Info
//
AssetInfoClass *
CDataTreeView::Get_Current_Asset_Info (void) const
{
	AssetInfoClass *asset_info = NULL;

	// Get the currently selected node from the tree control
	HTREEITEM htree_item = GetTreeCtrl ().GetSelectedItem ();
	if (htree_item != NULL) {

		// Get the data associated with this item
		asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
	}

	// Return the asset information associated with the current item
	return asset_info;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Current_Render_Obj
//
RenderObjClass *
CDataTreeView::Get_Current_Render_Obj (void) const
{
	RenderObjClass *prender_obj = NULL;

	// Get the currently selected node from the tree control
	HTREEITEM htree_item = GetTreeCtrl ().GetSelectedItem ();
	if (htree_item != NULL) {

		// Get the data associated with this item
		AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
		if (asset_info != NULL) {

			// Return the render object pointer
			prender_obj = asset_info->Peek_Render_Obj ();
		}
	}

	// Return the render object pointer
	return prender_obj;
}


////////////////////////////////////////////////////////////////////////////
//
//  GetCurrentSelectionName
//
LPCTSTR
CDataTreeView::GetCurrentSelectionName (void)
{
	LPCTSTR pname = NULL;

	// Get the currently selected node from the tree control
	HTREEITEM htree_item = GetTreeCtrl ().GetSelectedItem ();
	if (htree_item != NULL) {

		// Get the data associated with this item
		AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
		if (asset_info != NULL) {

			// Return the name of the asset to the caller
			pname = asset_info->Get_Name ();
		}
	}

	// Return the name of the selected node
	return pname;
}

////////////////////////////////////////////////////////////////////////////
//
//  GetCurrentSelectionType
//
ASSET_TYPE
CDataTreeView::GetCurrentSelectionType (void)
{
	ASSET_TYPE type = TypeUnknown;

	// Get the currently selected node from the tree control
	HTREEITEM htree_item = GetTreeCtrl ().GetSelectedItem ();
	if (htree_item != NULL) {
		
		// Get the associated asset information for this node.
		AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
		if (asset_info != NULL) {
			type = asset_info->Get_Type ();
		}
	}

	// Return the type of the selected node
	return type;
}

////////////////////////////////////////////////////////////////////////////
//
//  OnDblclk
//
void
CDataTreeView::OnDblclk
(
    NMHDR* pNMHDR,
    LRESULT* pResult
)
{
    // Get the main window of our app
    CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
    if (pCMainWnd != NULL)
    {
        // Display the properties for the currently selected object
        //pCMainWnd->ShowObjectProperties ();
    }

	(*pResult) = 0;
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_Render_Object_List
//
void
CDataTreeView::Build_Render_Object_List
(
	DynamicVectorClass <CString> &asset_list,
	HTREEITEM hparent
)
{
	// Loop through all the children of this node
	for (HTREEITEM htree_item = GetTreeCtrl ().GetChildItem (hparent);
		  (htree_item != NULL);
		  htree_item = GetTreeCtrl ().GetNextSiblingItem (htree_item)) {		

		// Determine if this is an asset type we want to add to the list
		AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
		if ((asset_info != NULL) &&
			 (asset_info->Get_Type () != TypeAnimation) &&
			 (asset_info->Get_Type () != TypeCompressedAnimation) &&
			 (asset_info->Get_Type () != TypeMaterial))
		{
			asset_list.Add (GetTreeCtrl ().GetItemText (htree_item));
		}

		// If this item has children, then add the children recursively
		if (GetTreeCtrl ().ItemHasChildren (htree_item)) {
			Build_Render_Object_List (asset_list, htree_item);
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Create_Render_Obj_To_Display
//
////////////////////////////////////////////////////////////////////////////
RenderObjClass *
CDataTreeView::Create_Render_Obj_To_Display (HTREEITEM htree_item)
{
	// Lookup the information object associated with this asset
	RenderObjClass *render_obj = NULL;
	AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
	if (asset_info != NULL) {
		
		// Use the asset's instance if there is one, otherwise attempt to create one
		render_obj = asset_info->Get_Render_Obj ();
		if (render_obj == NULL) {

			// If this is a texture, then create a special BMP obj from it
			if (asset_info->Get_Type () == TypeMaterial) {
				TextureClass *ptexture = (TextureClass *)asset_info->Get_User_Number ();
				if (ptexture != NULL) {
					render_obj = new Bitmap2DObjClass (ptexture, 0.5F, 0.5F, true, false, false, true);
				}
			}

			//
			// Finally, if we aren't successful, create a new instance based on its name
			//
			if (render_obj == NULL) {
				render_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (asset_info->Get_Name ());				
			}
		}
	}

	//
	//	Force the highest level LOD
	//
	if (	render_obj != NULL &&
			::GetCurrentDocument ()->GetScene ()->Are_LODs_Switching () == false)
	{
		Set_Highest_LOD (render_obj);
	}
	
	return render_obj;
}


////////////////////////////////////////////////////////////////////////////
//
//  Refresh_Asset
//
void
CDataTreeView::Refresh_Asset
(
	LPCTSTR new_name,
	LPCTSTR old_name,
	ASSET_TYPE type
)
{
	// Params OK?
	if ((new_name != NULL) && (old_name != NULL)) {

		// Turn off repainting
		GetTreeCtrl ().SetRedraw (FALSE);
		
		// Determime where this asset should go
		HTREEITEM hparent = NULL;
		int icon_index = 0;
		Determine_Tree_Location (type, hparent, icon_index);

		// Can we find the item we are supposed to refresh?
		HTREEITEM htree_item = FindChildItem (hparent, old_name);
		if (htree_item != NULL) {

			// Refresh the item's text in the tree control
			GetTreeCtrl ().SetItemText (htree_item, new_name);

			// Refresh the associated asset info structure
			AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (htree_item);
			if (asset_info != NULL) {
				asset_info->Set_Name (new_name);
			}		
		} else {

			// This asset wasn't already in the tree, so add it...
			Add_Asset_To_Tree (new_name, type, true);
		}

		// Turn painting back on
		GetTreeCtrl ().SetRedraw (TRUE);

		// Force the window to be repainted
		Invalidate (FALSE);
		UpdateWindow ();	
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Select_Next
//
void
CDataTreeView::Select_Next (void)
{
	//
	//	Get the selected entry in the tree control
	//
	HTREEITEM hselected = GetTreeCtrl ().GetSelectedItem ();
	if (hselected != NULL) {

		//
		//	Select the item that follows the currently selected item
		//
		HTREEITEM hitem = GetTreeCtrl ().GetNextItem (hselected, TVGN_NEXT);
		if (hitem != NULL) {
			GetTreeCtrl ().SelectItem (hitem);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Select_Prev
//
void
CDataTreeView::Select_Prev (void)
{
	//
	//	Get the selected entry in the tree control
	//
	HTREEITEM hselected = GetTreeCtrl ().GetSelectedItem ();
	if (hselected != NULL) {

		//
		//	Select the item that follows the currently selected item
		//
		HTREEITEM hitem = GetTreeCtrl ().GetNextItem (hselected, TVGN_PREVIOUS);
		if (hitem != NULL) {
			GetTreeCtrl ().SelectItem (hitem);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reload_Lightmap_Models
//
////////////////////////////////////////////////////////////////////////////
void
CDataTreeView::Reload_Lightmap_Models (void)
{
	Free_Child_Models (m_hMeshCollectionRoot);
	Free_Child_Models (m_hHierarchyRoot);
	Free_Child_Models (m_hMeshRoot);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Free_Child_Models
//
////////////////////////////////////////////////////////////////////////////
void
CDataTreeView::Free_Child_Models (HTREEITEM parent_item)
{
	//
	// Loop through all the children of this node
	//
	for (	HTREEITEM tree_item = GetTreeCtrl ().GetChildItem (parent_item);
			tree_item != NULL;
			tree_item = GetTreeCtrl ().GetNextSiblingItem (tree_item))
	{
		//
		// Get the data associated with this item
		//
		AssetInfoClass *asset_info = (AssetInfoClass *)GetTreeCtrl ().GetItemData (tree_item);
		if (asset_info  != NULL) {
			WW3DAssetManager::Get_Instance ()->Remove_Prototype (asset_info->Get_Name ());
		}        
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Highest_LOD
//
////////////////////////////////////////////////////////////////////////////
void
Set_Highest_LOD (RenderObjClass *render_obj) 
{
	if (render_obj != NULL) {
		for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
			RenderObjClass *sub_obj = render_obj->Get_Sub_Object (index);
			if (sub_obj != NULL) {
				Set_Highest_LOD (sub_obj);
			}
			MEMBER_RELEASE (sub_obj);
		}

		//
		// Switcht this LOD to its highest level
		//
		if (render_obj->Class_ID () == RenderObjClass::CLASSID_HLOD) {
			((HLodClass *)render_obj)->Set_LOD_Level (((HLodClass *)render_obj)->Get_Lod_Count () - 1);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Restrict_Anims
//
////////////////////////////////////////////////////////////////////////////
void
CDataTreeView::Restrict_Anims (bool onoff)
{
	if (m_RestrictAnims != onoff) {
		m_RestrictAnims = onoff;
		
		//
		//	Reload the tree
		//
		GetTreeCtrl ().DeleteAllItems ();
		CreateRootNodes ();
		LoadAssetsIntoTree ();
	}

	return ;
}

