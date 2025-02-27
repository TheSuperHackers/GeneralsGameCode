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

#pragma once

// DataTreeView.h : header file
//

#include "AfxCView.h"
#include "AssetTypes.h"
#include "Vector.h"

// Forward declarations
class RenderObjClass;
class AssetInfoClass;


/////////////////////////////////////////////////////////////////////////////
//
// CDataTreeView view
//
class CDataTreeView : public CTreeView
{
protected:
	CDataTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDataTreeView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CDataTreeView();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CDataTreeView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		/////////////////////////////////////////////////////////////////////
		//	Public methods
		/////////////////////////////////////////////////////////////////////

		//
		//	Asset insertion methods
		//
		bool					Add_Asset_To_Tree (LPCTSTR name, ASSET_TYPE type, bool bselect);
		void					LoadAssetsIntoTree (void);
		void					Refresh_Asset (LPCTSTR new_name, LPCTSTR old_name, ASSET_TYPE type);

		//
		//	Animation insertion methods
		//
		void					LoadAnimationsIntoTree (void);
		void					LoadAnimationsIntoTree (HTREEITEM hItem);

	  bool					Are_Anims_Restricted (void) const			{ return m_RestrictAnims; }
	  void					Restrict_Anims (bool onoff);

		//
		//	Texture insertion methods
		//
		void					Load_Materials_Into_Tree (void);

		//
		//	Display methods
		//
		void					Display_Asset (HTREEITEM htree_item = NULL);
		void					Select_Next (void);
		void					Select_Prev (void);
		void					Reload_Lightmap_Models (void);

		//
		// Information methods
		//
		RenderObjClass *	Get_Current_Render_Obj (void) const;
		AssetInfoClass *	Get_Current_Asset_Info (void) const;
		LPCTSTR				GetCurrentSelectionName (void);
		ASSET_TYPE			GetCurrentSelectionType (void);
		HTREEITEM			FindChildItem (HTREEITEM hParentItem, LPCTSTR pszChildItemName);
		HTREEITEM			FindChildItem (HTREEITEM hParentItem, RenderObjClass *prender_obj);
		HTREEITEM			FindFirstChildItemBasedOnHierarchyName (HTREEITEM hParentItem, LPCTSTR pszHierarchyName);
		HTREEITEM			FindSiblingItemBasedOnHierarchyName (HTREEITEM hCurrentItem, LPCTSTR pszHierarchyName);
		void					Build_Render_Object_List (DynamicVectorClass <CString> &asset_list, HTREEITEM hparent = TVI_ROOT);

		//
		//	Initialization methods
		//
		void					CreateRootNodes (void);

	protected:

		///////////////////////////////////////////////////////////////////////
		//	Protected methods
		///////////////////////////////////////////////////////////////////////
		ASSET_TYPE			Determine_Tree_Location (RenderObjClass &render_obj, HTREEITEM &hroot, int &icon_index);
		void					Determine_Tree_Location (ASSET_TYPE type, HTREEITEM &hroot, int &icon_index);
		RenderObjClass *	Create_Render_Obj_To_Display (HTREEITEM htree_item);
		void					Add_Emitters_To_Menu (HMENU hmenu, RenderObjClass &render_obj);
		void					Free_Child_Models (HTREEITEM parent_item);

	private:

		///////////////////////////////////////////////////////
		//
		//	Private member data
		//
		HTREEITEM	m_hMaterialsRoot;
		HTREEITEM	m_hMeshRoot;
		HTREEITEM	m_hAggregateRoot;
		HTREEITEM	m_hLODRoot;
		HTREEITEM	m_hMeshCollectionRoot;
		HTREEITEM	m_hEmitterRoot;
		HTREEITEM	m_hPrimitivesRoot;
		HTREEITEM	m_hHierarchyRoot;
		HTREEITEM	m_hSoundRoot;
		int			m_iAnimationIcon;
		int			m_iTCAnimationIcon;
		int			m_iADAnimationIcon;
		int			m_iMeshIcon;
		int			m_iMaterialIcon;
		int			m_iLODIcon;
		int			m_iEmitterIcon;
		int			m_iPrimitivesIcon;
		int			m_iAggregateIcon;
		int			m_iHierarchyIcon;
		int			m_iSoundIcon;
		bool			m_RestrictAnims;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
